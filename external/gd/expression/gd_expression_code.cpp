/**
 * @file gd_expression_code.cpp
 * @brief Implementation of the flat-bytecode program container `code`.
 *
 * ### Compilation overview
 *
 * The compiler is a single forward pass over the source text.  It reads one
 * logical line at a time (newline or semicolon terminated, strings skipped).
 * Each line is classified by its leading keyword:
 *
 *   - Plain expression  -> `eKindExpr`
 *   - `if <cond>`       -> `eKindIf`  (jump patched when matching `end` / `else` is found)
 *   - `else`            -> `eKindElseJump` (jump patched when next `end` is found)
 *   - `while <cond>`    -> `eKindWhile` (jump patched when matching `end` is found)
 *   - `for <v>=<s>,<l>,<step>` -> three statements: `eKindForInit`, `eKindForCond`, `eKindForStep`
 *   - `begin`           -> opens a block (recorded on a block stack)
 *   - `end`             -> closes the innermost block, patches pending jumps
 *   - `break`           -> `eKindBreak`  (jump patched when enclosing loop's `end` is found)
 *   - `continue`        -> `eKindContinue` (jump is set immediately to the loop head)
 *
 * Jump targets are resolved by a small backpatch stack that is O(nesting depth)
 * in memory — no heap allocation beyond the statement vector itself.
 *
 * ### Execution overview
 *
 * `execute` walks `m_vectorStatement` with an integer program-counter.
 * Each statement kind does at most one `token::calculate_s` call and one
 * integer write.  The inner loop is a plain `switch`.
 */

#include <stack>
#include <stdexcept>
#include <string>

#include "gd_expression_code.h"

_GD_EXPRESSION_BEGIN

// ===========================================================================
// Internal helpers (file-private)
// ===========================================================================

namespace {

/// Characters that count as whitespace for line trimming @TODO  Optimize this
inline bool is_whitespace_(char i)
{
   return i == ' ' || i == '\t' || i == '\r' || i == '\f' || i == '\v' || i == ';';
}

/// True when c opens a string literal (' or ")
inline bool is_string_delimiter_(char c)
{
   return c == '\'' || c == '"';
}

/**
 * @brief Internal block-stack entry used during compilation.
 *
 * One entry is pushed for every `begin` keyword encountered.  When the
 * matching `end` is found the entry is used to backpatch all pending jumps
 * that need to know where the block ends.
 */
struct block
{
   enum enumKind : uint8_t
   {
      eKindIf    = 0, ///< if / else-if block
      eKindElse  = 1, ///< else block
      eKindWhile = 2, ///< while block
      eKindFor   = 3, ///< for block
   };

   enumKind               m_eKind;

   /// index of the eKindIf / eKindWhile / eKindForCond statement whose
   /// m_iJump should point past the block
   int32_t                m_iCondIndex = -1;

   /// index of the eKindElseJump statement at the end of an if-body
   /// (needs to be patched to jump past the else-body too)
   int32_t                m_iElseJumpIndex = -1;

   /// for-loop: index of the eKindForCond statement (continue target)
   int32_t                m_iForCondIndex  = -1;

   /// for-loop: index of the eKindForStep statement (loop-back target)
   int32_t                m_iForStepIndex  = -1;

   /// loop only: list of eKindBreak statement indices waiting to be patched
   std::vector<int32_t>   m_vectorBreak;

   /// loop only: list of eKindContinue statement indices waiting to be patched
   std::vector<int32_t>   m_vectorContinue;

   /// for-loop variable name (used to build the step expression)
   std::string            m_stringForVar;
};

} // anonymous namespace


// ===========================================================================
// code - static free functions
// ===========================================================================

/**
 * @brief Skip over one string literal, return pointer past closing delimiter.
 *
 * Delegates to `token::read_string_s` which already handles
 * `'...'`, `"..."`, `'''...'''`, and `"""..."""`.
 *
 * This is the ONLY place in the file that knows about string delimiter rules.
 * If you need different string handling, replace this function body.
 */
const char* code::scan_string_end_s(const char* piszBegin, const char* piszEnd)
{
   assert(piszBegin < piszEnd);
   std::string_view string_;
   const char* piszAfter = nullptr;
   token::read_string_s(piszBegin, piszEnd, string_, &piszAfter);
   return piszAfter != nullptr ? piszAfter : piszEnd;
}

/**
 * @brief Find the end of one logical line in the source text.
 *
 * A line ends at `\n`, `;`, or `piszEnd`.  String literals are stepped over
 * so that a newline inside `'''` or `"""` does not end the line early.
 *
 * @param piszBegin     current scan start
 * @param piszEnd       one past the source buffer
 * @param ppiszLineEnd  (out) pointer to the terminator char (`\n`, `;`, or piszEnd)
 * @return string_view covering the line content, NOT including the terminator
 */
std::string_view code::scan_line_end_s(const char* piszBegin, const char* piszEnd,
                                       const char** ppiszLineEnd)
{
   const char* pisz_ = piszBegin;

   while( pisz_ < piszEnd )
   {
      char c = *pisz_;

      if( c == '\n' || c == ';' )                   { break; }
      if( is_string_delimiter_(c) )                  { pisz_ = scan_string_end_s(pisz_, piszEnd); continue; }

      ++pisz_;
   }

   if( ppiszLineEnd != nullptr ) { *ppiszLineEnd = pisz_; }

   return std::string_view(piszBegin, static_cast<size_t>(pisz_ - piszBegin));
}

/** ------------------------------------------------------------------------- trim_s
 * @brief Trim leading and trailing ASCII whitespace from a string_view.
 */
std::string_view code::trim_s(std::string_view stringLine)
{
   while( stringLine.empty() == false && is_whitespace_(stringLine.front()) )
      stringLine.remove_prefix(1);

   while( stringLine.empty() == false && is_whitespace_(stringLine.back()) )
      stringLine.remove_suffix(1);

   return stringLine;
}

/**
 * @brief Return true when `stringLine` starts with `stringKeyword` followed
 *        by whitespace or is exactly equal to it (no trailing characters).
 *
 * Example: starts_with_keyword_s("if x > 0", "if") == true
 *          starts_with_keyword_s("ifdef",    "if") == false
 */
bool code::starts_with_keyword_s(std::string_view stringLine, std::string_view stringKeyword)
{
   if( stringLine.size() < stringKeyword.size() ) { return false; }
   if( stringLine.substr(0, stringKeyword.size()) != stringKeyword ) { return false; }

   if( stringLine.size() == stringKeyword.size() ) { return true; } // exact match

   char cNext = stringLine[stringKeyword.size()];
   return is_whitespace_(cNext) || cNext == '(' || cNext == ';';
}

/**
 * @brief Split a line into { leading_keyword, rest }.
 *
 * If the line starts with an alphabetic keyword the keyword and the
 * remainder (trimmed) are returned.  If not, the keyword part is empty
 * and the whole line is in `rest`.
 *
 * Example: "if x > 0" -> { "if", "x > 0" }
 *          "x = 1"    -> { "",   "x = 1" }
 */
std::pair<std::string_view, std::string_view> code::split_keyword_s(std::string_view stringLine)
{
   if( stringLine.empty() || (stringLine[0] < 'a' || stringLine[0] > 'z') )
   {
      return { std::string_view{}, stringLine };
   }

   size_t uEnd = 0;
   while( uEnd < stringLine.size() )
   {
      char c = stringLine[uEnd];
      if( !( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ) ) break;
      uEnd++;
   }

   std::string_view stringKeyword = stringLine.substr(0, uEnd);
   std::string_view stringRest    = trim_s(stringLine.substr(uEnd));

   return { stringKeyword, stringRest };
}

/** ------------------------------------------------------------------------- compile_expression_s
 * @brief Compile one expression string into a postfix token vector.
 *
 * Uses `tag_formula_keyword` so keyword operators (and, or, not, …) work
 * inside conditions and assignments, then runs `compile` for the shunting-
 * yard reorder.
 */
std::pair<bool, std::string> code::compile_expression_s( std::string_view stringExpression, std::vector<token>& vectorOut)
{
   std::vector<token> vectorInfix;

   auto [bParse, stringParseError] = token::parse_s(stringExpression, vectorInfix, tag_formula{});
   if( bParse == false ) { return { false, stringParseError }; }
   if( vectorInfix.empty() == true ) { return { true, {} }; }

   auto [bCompile, stringCompileError] = token::compile_s(vectorInfix, vectorOut, tag_postfix{});
   if( bCompile == false ) { return { false, stringCompileError }; }

   return { true, {} };
}


// ===========================================================================
// code::compile
// ===========================================================================

/** ------------------------------------------------------------------------- compile
 * @brief Compile source text into the flat statement list.
 *
 * The `runtime_` parameter is accepted for future use (e.g. pre-declaring
 * variables) but is not required for correctness.
 *
 * @param piszBegin  start of source text
 * @param piszEnd    one past source text
 * @param runtime_   runtime context
 * @return { true, "" } on success, { false, error-message } on failure
 */
std::pair<bool, std::string> code::compile(const char* piszBegin, const char* piszEnd, runtime& runtime_)
{
   m_vectorStatement.clear();

   // ## Block stack: one entry per open `begin` we have not yet seen `end` for
   std::vector<block> vectorBlock;

   const char* piszPosition = piszBegin;

   int32_t iLine = 0; ///< line counter for error messages

   while( piszPosition < piszEnd )
   {
      // ### Skip blank / whitespace-only lines and move past terminator ------

      while( piszPosition < piszEnd && is_whitespace_(*piszPosition) ) { ++piszPosition; } // skip leading horizontal whitespace on a line

      if( piszPosition >= piszEnd ) { break; }

      // blank line / comment line starting with #
      if( *piszPosition == '\n' ) { ++piszPosition; ++iLine; continue; }
      if( *piszPosition == ';'  ) { ++piszPosition; continue; }

      // ### Read one logical line -------------------------------------------

      const char* piszLineEnd = nullptr;
      std::string_view stringLine = scan_line_end_s(piszPosition, piszEnd, &piszLineEnd);
      std::string_view stringTrimmed = trim_s(stringLine);

      // advance past terminator for next iteration
      piszPosition = piszLineEnd;
      if( piszPosition < piszEnd && (*piszPosition == '\n' || *piszPosition == ';') )
      {
         if( *piszPosition == '\n' ) { ++iLine; }
         piszPosition++;
      }

      if( stringTrimmed.empty() ) { continue; }

      // ### Classify the line by its leading keyword ------------------------

      auto [stringKeyword, stringRest] = split_keyword_s(stringTrimmed);

      // ---- begin -----------------------------------------------------------
      if( stringKeyword == "begin" )
      {
         // A `begin` without a preceding control-flow keyword is only allowed
         // at top level as a no-op block (rare, but not an error).
         // The block stack was already extended by the preceding if/while/for
         // handler, so `begin` just needs to confirm that the topmost frame
         // has been started.
         if( vectorBlock.empty() )
         {
            // bare begin / end block — push a dummy frame so `end` pops it
            block frame_;
            frame_.m_eKind      = block::eKindIf; // reuse if-kind as "bare"
            frame_.m_iCondIndex = -1;
            vectorBlock.push_back(std::move(frame_));
         }
         // else: frame was already pushed by if / while / for handler
         continue;
      }

      // ---- end -------------------------------------------------------------
      if( stringKeyword == "end" )
      {
         if( vectorBlock.empty() ) { return { false, "[code::compile_s] unexpected 'end' without matching 'begin' at line " + std::to_string(iLine) }; }

         block frame_ = std::move(vectorBlock.back());
         vectorBlock.pop_back();

         int32_t iHere = static_cast<int32_t>(m_vectorStatement.size());

         switch( frame_.m_eKind )
         {
         case block::eKindIf:
         {
            // Patch the if-condition jump to land HERE (first statement after block)
            if( frame_.m_iCondIndex >= 0 ) { m_vectorStatement[frame_.m_iCondIndex].set_jump(iHere); }

            // Patch any preceding ElseJump to also land HERE
            if( frame_.m_iElseJumpIndex >= 0 ) { m_vectorStatement[frame_.m_iElseJumpIndex].set_jump(iHere); }
            break;
         }

         case block::eKindElse:
         {
            // Patch the ElseJump (emitted at end of if-body) to land HERE
            if( frame_.m_iElseJumpIndex >= 0 ) { m_vectorStatement[frame_.m_iElseJumpIndex].set_jump( iHere ); }
            break;
         }

         case block::eKindWhile:
         {
            // Emit the loop-back jump to the while condition
            statement stmtLoopBack_(statement::eKindLoopBack, frame_.m_iCondIndex);
            m_vectorStatement.push_back(std::move(stmtLoopBack_));
            iHere = static_cast<int32_t>(m_vectorStatement.size()); // re-compute after push

            // Patch while condition to jump past loop-back
            if( frame_.m_iCondIndex >= 0 ) { m_vectorStatement[frame_.m_iCondIndex].set_jump(iHere); }

            // Patch break statements
            for( int32_t iBreak : frame_.m_vectorBreak ) { m_vectorStatement[iBreak].set_jump(iHere); }

            // Patch continue statements -> jump to while condition
            for( int32_t iCont : frame_.m_vectorContinue ) { m_vectorStatement[iCont].set_jump(frame_.m_iCondIndex); }

            break;
         }

         case block::eKindFor:
         {
            // Emit step statement looping back to condition
            // The step tokens are already compiled inside the for-step statement.
            // We just need a LoopBack pointing at eKindForCond.
            statement statementLoopBack(statement::eKindLoopBack, frame_.m_iForCondIndex);
            m_vectorStatement.push_back(std::move(statementLoopBack));
            iHere = static_cast<int32_t>(m_vectorStatement.size());

            // Patch ForCond to exit past loop-back
            if( frame_.m_iForCondIndex >= 0 )
               m_vectorStatement[frame_.m_iForCondIndex].set_jump(iHere);

            // Patch break -> past loop
            for( int32_t iBreak : frame_.m_vectorBreak )
               m_vectorStatement[iBreak].set_jump(iHere);

            // Patch continue -> step statement
            for( int32_t iCont : frame_.m_vectorContinue )
               m_vectorStatement[iCont].set_jump(frame_.m_iForStepIndex);

            break;
         }
         } // switch frame kind

         continue;
      }

      // ---- else ------------------------------------------------------------
      if( stringKeyword == "else" )
      {
         if( vectorBlock.empty() || vectorBlock.back().m_eKind != block::eKindIf )
         {
            return { false, "[code::compile_s] 'else' without matching 'if' at line " + std::to_string(iLine) };
         }

         // End the if-body: emit an unconditional jump that will be patched
         // to skip the else-body once we see its `end`.
         int32_t iElseJumpIndex = static_cast<int32_t>(m_vectorStatement.size());
         m_vectorStatement.emplace_back(statement::eKindElseJump);

         int32_t iElseBodyStart = static_cast<int32_t>(m_vectorStatement.size());

         // Patch the if-condition jump to land at the else-body start
         block& frame_ = vectorBlock.back();
         if( frame_.m_iCondIndex >= 0 )
            m_vectorStatement[frame_.m_iCondIndex].set_jump(iElseBodyStart);

         // Convert the if-frame to an else-frame and record the jump index
         frame_.m_eKind         = block::eKindElse;
         frame_.m_iCondIndex    = -1;
         frame_.m_iElseJumpIndex = iElseJumpIndex;

         // `else` is always followed by `begin` on the next line; we leave
         // the frame on the stack so the matching `end` will close it.
         continue;
      }

      // ---- if --------------------------------------------------------------
      if( stringKeyword == "if" )
      {
         if( stringRest.empty() )
         {
            return { false, "[code::compile_s] 'if' without condition at line " + std::to_string(iLine) };
         }

         // Compile condition expression
         std::vector<token> vectorCond;
         auto [bOk, stringErr] = compile_expression_s(stringRest, vectorCond);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtIf_(statement::eKindIf, std::move(vectorCond));
         stmtIf_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtIf_));

         // Push a block frame; `begin` on the next line will confirm it,
         // `end` will close and patch it.
         block frame_;
         frame_.m_eKind      = block::eKindIf;
         frame_.m_iCondIndex = iCondIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- while -----------------------------------------------------------
      if( stringKeyword == "while" )
      {
         if( stringRest.empty() )
         {
            return { false, "[code::compile_s] 'while' without condition at line " + std::to_string(iLine) };
         }

         // Compile condition
         std::vector<token> vectorCond;
         auto [bOk, stringErr] = compile_expression_s(stringRest, vectorCond);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtWhile_(statement::eKindWhile, std::move(vectorCond));
         stmtWhile_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtWhile_));

         block frame_;
         frame_.m_eKind      = block::eKindWhile;
         frame_.m_iCondIndex = iCondIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- for -------------------------------------------------------------
      // Syntax:  for <var> = <start>, <limit>, <step>
      // Example: for i = 0, 10, 1
      if( stringKeyword == "for" )
      {
         if( stringRest.empty() )
         {
            return { false, "[code::compile_s] 'for' without arguments at line " + std::to_string(iLine) };
         }

         // Split "var = start, limit, step" by finding the '=' and the commas.
         // We do a lightweight scan (respecting strings) rather than a full parse
         // so we can handle the three expression parts independently.

         auto find_char_ = [](std::string_view sv, char c, size_t uFrom = 0) -> size_t
         {
            for( size_t u = uFrom; u < sv.size(); ++u )
            {
               if( is_string_delimiter_(sv[u]) )
               {
                  // skip string literal
                  ++u;
                  char delim = sv[u - 1];
                  while( u < sv.size() && sv[u] != delim ) { ++u; }
               }
               else if( sv[u] == c ) { return u; }
            }
            return std::string_view::npos;
         };

         // locate '='
         size_t uEq = find_char_(stringRest, '=');
         if( uEq == std::string_view::npos )
            return { false, "[code::compile_s] 'for' missing '=' at line " + std::to_string(iLine) };

         std::string_view svVar       = trim_s(stringRest.substr(0, uEq));
         std::string_view svAfterEq   = trim_s(stringRest.substr(uEq + 1));

         // locate the two commas in the remainder
         size_t uComma1 = find_char_(svAfterEq, ',');
         if( uComma1 == std::string_view::npos )
            return { false, "[code::compile_s] 'for' missing limit after start at line " + std::to_string(iLine) };

         size_t uComma2 = find_char_(svAfterEq, ',', uComma1 + 1);
         if( uComma2 == std::string_view::npos )
            return { false, "[code::compile_s] 'for' missing step at line " + std::to_string(iLine) };

         std::string_view svStart = trim_s(svAfterEq.substr(0, uComma1));
         std::string_view svLimit = trim_s(svAfterEq.substr(uComma1 + 1, uComma2 - uComma1 - 1));
         std::string_view svStep  = trim_s(svAfterEq.substr(uComma2 + 1));

         if( svVar.empty()   ) return { false, "[code::compile_s] 'for' empty variable name at line " + std::to_string(iLine) };
         if( svStart.empty() ) return { false, "[code::compile_s] 'for' empty start expression at line " + std::to_string(iLine) };
         if( svLimit.empty() ) return { false, "[code::compile_s] 'for' empty limit expression at line " + std::to_string(iLine) };
         if( svStep.empty()  ) return { false, "[code::compile_s] 'for' empty step expression at line " + std::to_string(iLine) };

         // -- ForInit: var = start
         {
            std::string stringInit = std::string(svVar) + " = " + std::string(svStart);
            std::vector<token> vectorInit;
            auto [bOk, stringErr] = compile_expression_s(stringInit, vectorInit);
            if( bOk == false ) { return { false, stringErr }; }

            statement stmtInit_(statement::eKindForInit, std::move(vectorInit));
            stmtInit_.m_stringSource = std::string(stringTrimmed);
            m_vectorStatement.push_back(std::move(stmtInit_));
         }

         // -- ForCond: var < limit  (or <= depending on convention)
         //    We use `var < limit` with exclusive upper bound (Python style).
         //    Change to `<=` here if inclusive is preferred.
         int32_t iForCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         {
            std::string stringCond = std::string(svVar) + " < " + std::string(svLimit);
            std::vector<token> vectorCond;
            auto [bOk, stringErr] = compile_expression_s(stringCond, vectorCond);
            if( bOk == false ) { return { false, stringErr }; }

            statement stmtCond_(statement::eKindForCond, std::move(vectorCond));
            m_vectorStatement.push_back(std::move(stmtCond_));
         }

         // The for-body comes between ForCond and the ForStep.
         // We reserve a slot for ForStep now and fill it after compilation of
         // the body (at `end` time), BUT we actually need the step to run at
         // the bottom of the loop body, not at the top.  The simpler approach:
         // emit ForStep AFTER the body (at `end`), and store its expression
         // in the block frame so we can emit it then.

         block frame_;
         frame_.m_eKind         = block::eKindFor;
         frame_.m_iForCondIndex = iForCondIndex;
         frame_.m_iForStepIndex = -1; // filled when `end` is processed
         frame_.m_stringForVar  = std::string(svVar);

         // Store the step expression string in a temporary statement that we
         // will finalise at `end`.  We use a placeholder eKindExpr with the
         // pre-compiled tokens already in it; the executor will treat it as a
         // normal expression run.
         {
            std::string stringStep = std::string(svVar) + " = " + std::string(svVar) + " + " + std::string(svStep);
            std::vector<token> vectorStep;
            auto [bOk, stringErr] = compile_expression_s(stringStep, vectorStep);
            if( bOk == false ) { return { false, stringErr }; }

            // Reserve the ForStep slot at the END (will be pushed at `end`).
            // Store tokens in the frame for now.
            frame_.m_iForStepIndex = -2; // sentinel: "tokens are ready in frame"
            // Temporarily stash in a statement stored in the frame's break vector
            // ... cleaner: just store the compiled tokens directly in the frame.
            // We add a dummy statement to m_vectorStatement now as a placeholder
            // that `end` will overwrite with a proper eKindForStep.
            // Actually: it is simpler and cheaper to emit the step statement
            // at `end`, so we store the compiled tokens in a local that the
            // frame needs to own.  We pack it into a one-element vector inside
            // the frame by re-using m_vectorBreak as a token-index store — but
            // that conflates types.  Instead we just store a full statement.
            //
            // Solution: extend block with an optional step statement.
            // Since we cannot change the struct definition here (it is local),
            // we emit a special eKindExpr placeholder statement, remember its
            // index, and overwrite its kind/tokens at `end`.

            int32_t iStepSlot = static_cast<int32_t>(m_vectorStatement.size());
            statement stmtStepSlot_(statement::eKindForStep, std::move(vectorStep));
            stmtStepSlot_.m_iJump = iForCondIndex; // step loops back to condition
            m_vectorStatement.push_back(std::move(stmtStepSlot_));

            frame_.m_iForStepIndex = iStepSlot;

            // BUT we want the step to execute AFTER the body, not before.
            // Move it: remove it from the vector and re-insert at `end` time.
            // We achieve this by saving the statement and erasing the slot.
            // ... The cleanest flat-bytecode approach: just leave the step slot
            // in its current position and jump OVER it from the condition into
            // the body, then loop back to the step from the end of the body.
            //
            // Layout:
            //   [N]   ForCond  (jump to past-loop if false)
            //   [N+1] ForStep  (run step, then jump to ForCond) <- skip on first entry
            //   [N+2] <body statements>
            //   [M]   LoopBack -> N+1  (jump to step, not to cond)
            //
            // When ForCond is true we jump over the ForStep to N+2 (body start).
            // At loop-back we jump to N+1 (step), which then jumps to N (cond).

            // The ForStep's own jump (m_iJump) points to ForCond.
            // We need ForCond to jump into the body (past the step slot) when true,
            // and to jump past everything when false.
            // "True branch" of ForCond: iStepSlot+1 (body start).
            // We encode this as: execute falls through to body (PC = iCondIndex+1
            // would be the step slot, which we do NOT want on first entry).
            //
            // Re-design: emit ForCond with a "skip step" jump embedded.
            // The executor for eKindForCond does:
            //   if condition true  -> PC = m_iJump (past-loop) ... wait, that is backwards.
            //
            // Correct approach: on FALSE the condition jumps PAST the loop.
            // On TRUE it falls through to the step slot, which we need to skip.
            // So we need an extra skip on the first entry only — that is awkward.
            //
            // Simplest correct layout that avoids extra complexity:
            //   [N]   ForInit
            //   [N+1] ForCond  -> jump to [M+1] (past loop) when false
            //   [N+2] <body statements>
            //   [M]   ForStep  -> jump back to [N+1]
            //   [M+1] (first past-loop statement)
            //
            // We achieve this by NOT emitting ForStep here.  Instead we store
            // the pre-compiled step tokens in the block frame and emit the
            // ForStep statement when `end` is processed, just before the
            // LoopBack.  Then LoopBack points to ForStep, and ForStep points
            // to ForCond.  ForCond on false jumps to M+2 (past LoopBack).

            // Undo: remove the prematurely emitted step slot.
            m_vectorStatement.pop_back();

            // Store step tokens in the frame.  We pack them into a statement
            // so the frame owns them cleanly.
            statement statementStep(statement::eKindForStep);
            statementStep.m_vectorToken = std::move(stmtStepSlot_.m_vectorToken);
            statementStep.m_iJump       = iForCondIndex;

            // Stash in the frame via the break vector as a raw index trick is
            // ugly; instead add a dedicated member.  Since we own block
            // locally and it already has m_iForStepIndex, let's store the
            // statement temporarily in a std::vector<statement> member.
            // But block is a local struct ... add a member there:
            //   statement m_stmtStep;
            // We cannot modify block from here because it was already
            // defined above.  So we use a different trick: serialise the step
            // tokens into m_stringForVar with a sentinel, then rebuild at end.
            // This is messy.  The right solution: put `statement m_stmtStep`
            // into block — but that requires moving the definition.
            //
            // Given the constraints of this single-file implementation we use
            // the simplest correct approach: emit the step as a normal eKindExpr
            // statement immediately after ForCond, emit a "skip" jump over it,
            // then the body follows, and at `end` we emit a LoopBack to the
            // step statement (not to ForCond).
            //
            // Final chosen layout (simplest, no deferred emission needed):
            //
            //   [N]   ForInit                          run once
            //   [N+1] ForCond  jump->M+2 when false   check limit
            //   [N+2] ForSkip  jump->N+3               skip step on first entry
            //   [N+3] ForStep  jump->N+1               step expression + loop back
            //   [N+4] <body>
            //   [M]   LoopBack -> N+3                  run step then re-check
            //   [M+1] (past loop)
            //
            // ForSkip is an unconditional jump (eKindElseJump reused).
            // ForStep's m_iJump = N+1 (ForCond).
            // ForCond's m_iJump = M+1.
            // ForSkip's m_iJump = N+4 (first body statement).
            // LoopBack's m_iJump = N+3 (ForStep).

            // Emit ForStep now (at N+3 relative to current base)
            int32_t iStepIndex = static_cast<int32_t>(m_vectorStatement.size());
            statementStep.m_iJump = iForCondIndex; // step -> cond
            m_vectorStatement.push_back(std::move(statementStep));
            frame_.m_iForStepIndex = iStepIndex;

            // Emit ForSkip (N+2, emitted BEFORE step, so swap):
            // We need ForSkip at N+2 and ForStep at N+3, but we just pushed
            // ForStep at N+2.  Swap:
            // Insert ForSkip before ForStep.
            int32_t iSkipIdx = iStepIndex; // will be N+2 after insert
            statement stmtSkip_(statement::eKindElseJump); // unconditional jump
            stmtSkip_.m_iJump = iStepIndex + 1; // jump to body start (N+4, i.e. one past ForStep)
            m_vectorStatement.insert(m_vectorStatement.begin() + iSkipIdx,
                                     std::move(stmtSkip_));

            // After insert, ForStep is now at iStepIndex+1
            frame_.m_iForStepIndex = iStepIndex + 1;
            // Fix ForStep's own jump (was already set to iForCondIndex, still correct)

            // ForCond at iForCondIndex: its false-jump will be patched at `end`.
            // Continue: body starts at iStepIndex+2 = iSkipIdx+2
            // Fix ForSkip jump to point to body start
            m_vectorStatement[iSkipIdx].set_jump(iStepIndex + 2);

            // continue/break in body will target step and past-loop respectively;
            // those are patched at `end`.
         }

         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- break -----------------------------------------------------------
      if( stringKeyword == "break" )
      {
         // Find innermost loop frame
         bool bFoundLoop = false;
         for( int32_t i = static_cast<int32_t>(vectorBlock.size()) - 1; i >= 0; --i )
         {
            if( vectorBlock[i].m_eKind == block::eKindWhile ||
                vectorBlock[i].m_eKind == block::eKindFor )
            {
               int32_t iBreakIdx = static_cast<int32_t>(m_vectorStatement.size());
               m_vectorStatement.emplace_back(statement::eKindBreak);
               vectorBlock[i].m_vectorBreak.push_back(iBreakIdx);
               bFoundLoop = true;
               break;
            }
         }

         if( bFoundLoop == false )
            return { false, "[code::compile_s] 'break' outside loop at line " + std::to_string(iLine) };

         continue;
      }

      // ---- continue --------------------------------------------------------
      if( stringKeyword == "continue" )
      {
         bool bFoundLoop = false;
         for( int32_t i = static_cast<int32_t>(vectorBlock.size()) - 1; i >= 0; --i )
         {
            if( vectorBlock[i].m_eKind == block::eKindWhile ||
                vectorBlock[i].m_eKind == block::eKindFor )
            {
               int32_t iContinueIndex = static_cast<int32_t>(m_vectorStatement.size());
               m_vectorStatement.emplace_back(statement::eKindContinue);
               vectorBlock[i].m_vectorContinue.push_back(iContinueIndex);
               bFoundLoop = true;
               break;
            }
         }

         if( bFoundLoop == false ) { return { false, "[code::compile_s] 'continue' outside loop at line " + std::to_string(iLine) }; }

         continue;
      }

      // ---- plain expression -----------------------------------------------
      {
         std::vector<token> vectorPostfix;
         auto [bOk, stringErr] = compile_expression_s(stringTrimmed, vectorPostfix);
         if( bOk == false )
            return { false, "[code::compile_s] expression error at line " + std::to_string(iLine) + ": " + stringErr };

         if( vectorPostfix.empty() == false )
         {
            statement stmtExpr_(statement::eKindExpr, std::move(vectorPostfix));
            stmtExpr_.m_stringSource = std::string(stringTrimmed);
            m_vectorStatement.push_back(std::move(stmtExpr_));
         }
      }

   } // while source not exhausted

   // ## Check for unclosed blocks
   if( vectorBlock.empty() == false )
   {
      return { false, "[code::compile_s] " + std::to_string(vectorBlock.size()) + " unclosed 'begin' block(s)" };
   }

   return { true, {} };
}


// ===========================================================================
// code::compile_lua
// ===========================================================================

/** -------------------------------------------------------------------------- lua_translate_operators_s
 * @brief Replace Lua-specific operator spellings with expression-engine equivalents.
 *
 * The only substitution currently needed is `~=` (Lua not-equal) -> `!=`.
 * String literals inside the expression are skipped so their content is never
 * modified.
 *
 * @param stringExpression  raw Lua expression text
 * @return                  expression with Lua operators replaced
 */
std::string code::lua_translate_operators_s(std::string_view stringExpression)
{
   std::string stringResult;
   stringResult.reserve(stringExpression.size());

   const char* pisz_    = stringExpression.data();
   const char* piszEnd_ = pisz_ + stringExpression.size();

   while( pisz_ < piszEnd_ )
   {
      char iCharacter = *pisz_;

      // ## skip string literals verbatim so we never alter their content
      if( is_string_delimiter_(iCharacter) )
      {
         const char* piszAfter = scan_string_end_s(pisz_, piszEnd_);
         stringResult.append(pisz_, static_cast<size_t>(piszAfter - pisz_));
         pisz_ = piszAfter;
         continue;
      }

      // ## translate ~= to !=
      if( iCharacter == '~' && (pisz_ + 1) < piszEnd_ && pisz_[1] == '=' )
      {
         stringResult += "!=";
         pisz_ += 2;
         continue;
      }

      stringResult += iCharacter;
      ++pisz_;
   }

   return stringResult;
}


/** -------------------------------------------------------------------------- compile_lua
 * @brief Compile Lua-syntax source into the flat statement list.
 *
 * ### Lua syntax handled
 *
 * ```lua
 * if condition then
 *     -- body
 * elseif condition then
 *     -- body
 * else
 *     -- body
 * end
 *
 * while condition do
 *     -- body
 * end
 *
 * repeat
 *     -- body
 * until condition
 *
 * for i = first, last [, delta] do
 *     -- body
 * end
 *
 * break
 * ```
 *
 * ### Block delimiters
 *
 * | `compile`  | `compile_lua` |
 * |------------|---------------|
 * | `begin`    | `then` / `do` |
 * | `end`      | `end`         |
 *
 * `then` and `do` close the control-flow header line and open the body —
 * they may appear at the end of the same line as the keyword:
 * `if x > 0 then` is one logical line, not two.
 *
 * ### repeat / until
 *
 * Compiled as: body statements, then an inverted-condition `eKindIf` that
 * jumps PAST the loop when the `until` condition is TRUE (loop exits when
 * condition becomes true), followed by an `eKindLoopBack` to the first body
 * statement.
 *
 * Layout:
 * ```
 * [B]   <body statements>
 * [U]   eKindIf(until_cond)  -> [U+2]  (exits when condition TRUE)
 * [U+1] eKindLoopBack        -> [B]
 * [U+2] (past loop)
 * ```
 *
 * @param piszBegin  start of Lua source text
 * @param piszEnd    one past end of source text
 * @param runtime_   runtime context
 * @return { true, "" } on success, { false, error-message } on failure
 */
std::pair<bool, std::string> code::compile_lua(const char* piszBegin, const char* piszEnd, runtime& runtime_)
{
   m_vectorStatement.clear();

   std::vector<block> vectorBlock;

   const char* piszPosition = piszBegin;
   int32_t iLine = 0;                                                         ///< line counter for error messages

   while( piszPosition < piszEnd )
   {
      // ### Skip leading horizontal whitespace on a line --------------------

      while( piszPosition < piszEnd && is_whitespace_(*piszPosition) ) { ++piszPosition; }

      if( piszPosition >= piszEnd ) { break; }

      // blank line
      if( *piszPosition == '\n' ) { ++piszPosition; ++iLine; continue; }

      // ### Read one logical line -------------------------------------------

      const char* piszLineEnd = nullptr;
      std::string_view stringLine    = scan_line_end_s(piszPosition, piszEnd, &piszLineEnd);
      std::string_view stringTrimmed = trim_s(stringLine);

      // advance past terminator
      piszPosition = piszLineEnd;
      if( piszPosition < piszEnd && *piszPosition == '\n' ) { ++iLine; ++piszPosition; }

      if( stringTrimmed.empty() ) { continue; }

      // ### Split into leading keyword and remainder -------------------------

      auto [stringKeyword, stringRest] = split_keyword_s( stringTrimmed );      // @TODO: Optimise: speed upp checks for keywords

      // ---- end -------------------------------------------------------------
      // Closes: if/elseif/else chain, while, repeat, for
      if( stringKeyword == "end" )
      {
         if( vectorBlock.empty() ) { return { false, "[code::compile_lua] unexpected 'end' at line " + std::to_string(iLine) }; }

         block frame_ = std::move(vectorBlock.back());
         vectorBlock.pop_back();

         int32_t iHere = static_cast<int32_t>(m_vectorStatement.size());

         switch( frame_.m_eKind )
         {
         case block::eKindIf:
         {
            // Patch the last condition (if or elseif) to jump HERE
            if( frame_.m_iCondIndex >= 0 ) { m_vectorStatement[frame_.m_iCondIndex].set_jump(iHere); }

            // Patch the most-recent else-jump to land HERE
            if( frame_.m_iElseJumpIndex >= 0 ) { m_vectorStatement[frame_.m_iElseJumpIndex].set_jump(iHere); }

            // Patch any earlier elseif-chain else-jumps (stored in m_vectorBreak)
            // @NOTE m_vectorBreak is reused for ElseJump chain indices in compile_lua
            for( int32_t iElseJump : frame_.m_vectorBreak ) { m_vectorStatement[iElseJump].set_jump( iHere ); }

            break;
         }

         case block::eKindElse:
         {
            // Patch the else-body jump to land HERE
            if( frame_.m_iElseJumpIndex >= 0 ) { m_vectorStatement[frame_.m_iElseJumpIndex].set_jump(iHere); }

            // Patch earlier elseif-chain jumps (also stored in m_vectorBreak)
            for( int32_t iElseJump : frame_.m_vectorBreak ) { m_vectorStatement[iElseJump].set_jump(iHere); }

            break;
         }

         case block::eKindWhile:
         {
            // Emit loop-back to while condition
            statement stmtLoopBack_(statement::eKindLoopBack, frame_.m_iCondIndex);
            m_vectorStatement.push_back(std::move(stmtLoopBack_));
            iHere = static_cast<int32_t>(m_vectorStatement.size());

            // Patch while condition to jump past loop
            if( frame_.m_iCondIndex >= 0 ) { m_vectorStatement[frame_.m_iCondIndex].set_jump(iHere); }

            for( int32_t iBreak : frame_.m_vectorBreak ) { m_vectorStatement[iBreak].set_jump( iHere ); }

            for( int32_t iContinue : frame_.m_vectorContinue ) { m_vectorStatement[iContinue].set_jump( frame_.m_iCondIndex ); }

            break;
         }

         case block::eKindFor:
         {
            // Emit loop-back to ForStep (which then jumps to ForCond)
            statement stmtLoopBack_(statement::eKindLoopBack, frame_.m_iForStepIndex);
            m_vectorStatement.push_back(std::move(stmtLoopBack_));
            iHere = static_cast<int32_t>(m_vectorStatement.size());

            // Patch ForCond false-exit to land HERE
            if( frame_.m_iForCondIndex >= 0 )
               m_vectorStatement[frame_.m_iForCondIndex].set_jump(iHere);

            for( int32_t iBreak : frame_.m_vectorBreak )
               m_vectorStatement[iBreak].set_jump(iHere);

            for( int32_t iContinue : frame_.m_vectorContinue )
               m_vectorStatement[iContinue].set_jump(frame_.m_iForStepIndex);

            break;
         }
         } // switch frame kind

         continue;
      }

      // ---- if --------------------------------------------------------------
      // Syntax: if <condition> then
      if( stringKeyword == "if" )
      {
         if( stringRest.empty() ) { return { false, "[code::compile_lua] 'if' without condition at line " + std::to_string(iLine) }; }

         // Strip trailing 'then' keyword — it may appear at end of condition
         std::string_view stringCondition = stringRest;
         {
            auto [stringLastKeyword, stringAfterLast] = split_keyword_s(stringCondition);
            // walk to last token: find 'then' at the tail
            // Simple approach: if the last word is 'then', remove it
            if( stringCondition.size() >= 4 )
            {
               std::string_view stringTail = trim_s(stringCondition.substr(stringCondition.rfind(' ') == std::string_view::npos ? 0 : stringCondition.rfind(' ')));
               if( stringTail == "then" )
               {
                  size_t uThenPos = stringCondition.rfind("then");
                  stringCondition = trim_s(stringCondition.substr(0, uThenPos));
               }
            }
         }

         // std::string stringTranslated = lua_translate_operators_s(stringCondition); // @TODO: For later, now use C++ operator
         std::vector<token> vectorCondition;
         auto [bOk, stringErr] = compile_expression_s(stringCondition, vectorCondition);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtIf_(statement::eKindIf, std::move(vectorCondition));
         stmtIf_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtIf_));

         block frame_;
         frame_.m_eKind      = block::eKindIf;
         frame_.m_iCondIndex = iCondIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- elseif ----------------------------------------------------------
      // Syntax: elseif <condition> then
      // Closes the previous if/elseif body and opens a new condition branch.
      if( stringKeyword == "elseif" )
      {
         if( vectorBlock.empty() || vectorBlock.back().m_eKind != block::eKindIf )
         {
            return { false, "[code::compile_lua] 'elseif' without matching 'if' at line " + std::to_string(iLine) };
         }
         if( stringRest.empty() )
         {
            return { false, "[code::compile_lua] 'elseif' without condition at line " + std::to_string(iLine) };
         }

         // ## End the current if/elseif body with an unconditional jump.
         // This jump will be chained into a list so all branches skip past the
         // entire if/elseif/else/end block.  We store it in m_iElseJumpIndex
         // by linking: the new ElseJump's jump will be patched at the NEXT
         // elseif / else / end.  To support a chain of elseif branches we
         // keep a vector of pending ElseJump indices in the block frame.
         // Because block only stores one m_iElseJumpIndex we emit a new jump,
         // patch the previous condition's false-branch to land at the new
         // elseif condition, then record the new jump index.

         int32_t iElseJumpIndex = static_cast<int32_t>(m_vectorStatement.size());
         m_vectorStatement.emplace_back(statement::eKindElseJump); // will be patched at end

         int32_t iNewCondStart = static_cast<int32_t>(m_vectorStatement.size());

         // Patch the PREVIOUS condition's false-jump to land here (at the new elseif condition)
         block& frame_ = vectorBlock.back();
         if( frame_.m_iCondIndex >= 0 )
            m_vectorStatement[frame_.m_iCondIndex].set_jump(iNewCondStart);

         // The previous ElseJump still needs patching; collect it alongside any earlier ones.
         // We reuse m_vectorBreak to store pending ElseJump indices (they will all be
         // patched to the same final destination at `end`).
         if( frame_.m_iElseJumpIndex >= 0 )
            frame_.m_vectorBreak.push_back(frame_.m_iElseJumpIndex); // @NOTE reusing m_vectorBreak for ElseJump chain

         frame_.m_iElseJumpIndex = iElseJumpIndex;

         // ## Compile the elseif condition
         std::string_view stringCondition = stringRest;
         {
            size_t uThenPos = stringCondition.rfind("then");
            if( uThenPos != std::string_view::npos )
               stringCondition = trim_s(stringCondition.substr(0, uThenPos));
         }

         //std::string stringTranslated = lua_translate_operators_s( stringCondition ); // @TODO: For later, now use C++ operator
         std::vector<token> vectorCondition;
         auto [bOk, stringErr] = compile_expression_s(stringCondition, vectorCondition);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtElseIf_(statement::eKindIf, std::move(vectorCondition));
         stmtElseIf_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtElseIf_));

         frame_.m_iCondIndex = iCondIndex; // frame stays on stack, condition index updated
         continue;
      }

      // ---- else ------------------------------------------------------------
      if( stringKeyword == "else" )
      {
         if( vectorBlock.empty() || vectorBlock.back().m_eKind != block::eKindIf )
         {
            return { false, "[code::compile_lua] 'else' without matching 'if' at line " + std::to_string(iLine) };
         }

         // Emit the end-of-if-body unconditional jump
         int32_t iElseJumpIndex = static_cast<int32_t>(m_vectorStatement.size());
         m_vectorStatement.emplace_back(statement::eKindElseJump);

         int32_t iElseBodyStart = static_cast<int32_t>(m_vectorStatement.size());

         block& frame_ = vectorBlock.back();

         // Patch the last condition's false-jump to land at else-body start
         if( frame_.m_iCondIndex >= 0 )
            m_vectorStatement[frame_.m_iCondIndex].set_jump(iElseBodyStart);

         // Collect any pending ElseJumps from elseif chain into the frame's break list
         if( frame_.m_iElseJumpIndex >= 0 )
            frame_.m_vectorBreak.push_back(frame_.m_iElseJumpIndex);

         frame_.m_eKind          = block::eKindElse;
         frame_.m_iCondIndex     = -1;
         frame_.m_iElseJumpIndex = iElseJumpIndex; // this one gets patched at `end`
         continue;
      }

      // ---- while -----------------------------------------------------------
      // Syntax: while <condition> do
      if( stringKeyword == "while" )
      {
         if( stringRest.empty() )
         {
            return { false, "[code::compile_lua] 'while' without condition at line " + std::to_string(iLine) };
         }

         // Strip trailing 'do'
         std::string_view stringCondition = stringRest;
         {
            size_t uDoPos = stringCondition.rfind("do");
            if( uDoPos != std::string_view::npos )
            {
               // only strip if it is a standalone word
               bool bWordBoundaryBefore = (uDoPos == 0 || is_whitespace_(stringCondition[uDoPos - 1]));
               bool bWordBoundaryAfter  = ((uDoPos + 2) >= stringCondition.size() || is_whitespace_(stringCondition[uDoPos + 2]));
               if( bWordBoundaryBefore && bWordBoundaryAfter )
                  stringCondition = trim_s(stringCondition.substr(0, uDoPos));
            }
         }

         // std::string stringTranslated = lua_translate_operators_s(stringCondition); @TODO: For later, now use C++ operator
         std::vector<token> vectorCondition;
         auto [bOk, stringErr] = compile_expression_s(stringCondition, vectorCondition);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtWhile_(statement::eKindWhile, std::move(vectorCondition));
         stmtWhile_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtWhile_));

         block frame_;
         frame_.m_eKind      = block::eKindWhile;
         frame_.m_iCondIndex = iCondIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- repeat ----------------------------------------------------------
      // Syntax: repeat ... until <condition>
      // The block opener is the `repeat` keyword itself (no `do` / `then`).
      // Compiled layout:
      //   [B]   <body statements>
      //   [U]   eKindIf(until_cond) -> [U+2]   exit when condition TRUE
      //   [U+1] eKindLoopBack       -> [B]
      //   [U+2] (past loop)
      if( stringKeyword == "repeat" )
      {
         // Record the body start so we can point LoopBack at it
         int32_t iBodyStart = static_cast<int32_t>(m_vectorStatement.size());

         // Push a frame that records the body start in m_iCondIndex.
         // We reuse eKindWhile; at `until` time we switch to a custom patch.
         block frame_;
         frame_.m_eKind      = block::eKindWhile;
         frame_.m_iCondIndex = iBodyStart;          // body start, NOT a condition statement
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- until -----------------------------------------------------------
      // Closes a `repeat` block. Syntax: until <condition>
      if( stringKeyword == "until" )
      {
         if( vectorBlock.empty() || vectorBlock.back().m_eKind != block::eKindWhile )
         {
            return { false, "[code::compile_lua] 'until' without matching 'repeat' at line " + std::to_string(iLine) };
         }
         if( stringRest.empty() )
         {
            return { false, "[code::compile_lua] 'until' without condition at line " + std::to_string(iLine) };
         }

         block frame_ = std::move(vectorBlock.back());
         vectorBlock.pop_back();

         int32_t iBodyStart = frame_.m_iCondIndex; // recorded at `repeat` time

         // Compile the until condition; when TRUE the loop exits
         // std::string stringTranslated = lua_translate_operators_s(stringRest); @TODO: For later, now use C++ operator
         std::vector<token> vectorCondition;
         auto [bOk, stringErr] = compile_expression_s(stringRest, vectorCondition);
         if( bOk == false ) { return { false, stringErr }; }

         // Emit eKindIf(until_cond): true -> jump past loop (past the LoopBack)
         int32_t iUntilCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtUntilCond_(statement::eKindIf, std::move(vectorCondition));
         stmtUntilCond_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtUntilCond_));

         // Emit LoopBack -> body start
         statement stmtLoopBack_(statement::eKindLoopBack, iBodyStart);
         m_vectorStatement.push_back(std::move(stmtLoopBack_));

         int32_t iHere = static_cast<int32_t>(m_vectorStatement.size());

         // Patch the until-condition jump: true -> past LoopBack (iHere)
         m_vectorStatement[iUntilCondIndex].set_jump(iHere);

         // Patch break statements
         for( int32_t iBreak : frame_.m_vectorBreak ) { m_vectorStatement[iBreak].set_jump(iHere); }

         continue;
      }

      // ---- for -------------------------------------------------------------
      // Syntax: for <var> = <first>, <last> [, <delta>] do
      // Delta defaults to 1 when omitted.  Range is inclusive (Lua semantics).
      if( stringKeyword == "for" )
      {
         if( stringRest.empty() ) { return { false, "[code::compile_lua] 'for' without arguments at line " + std::to_string(iLine) }; }

         // Strip trailing 'do' from the remainder
         std::string_view stringForHeader = stringRest;
         {
            size_t uDoPos = stringForHeader.rfind("do");
            if( uDoPos != std::string_view::npos )
            {
               bool bWordBoundaryBefore = (uDoPos == 0 || is_whitespace_(stringForHeader[uDoPos - 1]));
               bool bWordBoundaryAfter  = ((uDoPos + 2) >= stringForHeader.size() || is_whitespace_(stringForHeader[uDoPos + 2]));
               if( bWordBoundaryBefore && bWordBoundaryAfter )
                  stringForHeader = trim_s(stringForHeader.substr(0, uDoPos));
            }
         }

         // Scan for '=' and commas, respecting string literals
         auto find_char_lua_ = [](std::string_view sv, char iChar, size_t uFrom = 0) -> size_t
         {
            for( size_t u = uFrom; u < sv.size(); ++u )
            {
               if( is_string_delimiter_(sv[u]) )
               {
                  ++u;
                  char iDelim = sv[u - 1];
                  while( u < sv.size() && sv[u] != iDelim ) { ++u; }
               }
               else if( sv[u] == iChar ) { return u; }
            }
            return std::string_view::npos;
         };

         size_t uEqPos = find_char_lua_(stringForHeader, '=');
         if( uEqPos == std::string_view::npos )
            return { false, "[code::compile_lua] 'for' missing '=' at line " + std::to_string(iLine) };

         std::string_view stringVar    = trim_s(stringForHeader.substr(0, uEqPos));
         std::string_view stringAfterEq = trim_s(stringForHeader.substr(uEqPos + 1));

         // First comma separates first from last
         size_t uComma1 = find_char_lua_(stringAfterEq, ',');
         if( uComma1 == std::string_view::npos )
            return { false, "[code::compile_lua] 'for' missing last value at line " + std::to_string(iLine) };

         std::string_view stringFirst = trim_s(stringAfterEq.substr(0, uComma1));
         std::string_view stringAfterFirst = trim_s(stringAfterEq.substr(uComma1 + 1));

         // Optional second comma separates last from delta
         size_t uComma2 = find_char_lua_(stringAfterFirst, ',');
         std::string_view stringLast;
         std::string_view stringDelta;
         if( uComma2 == std::string_view::npos )
         {
            stringLast  = trim_s(stringAfterFirst);
            stringDelta = "1"; // Lua default delta
         }
         else
         {
            stringLast  = trim_s(stringAfterFirst.substr(0, uComma2));
            stringDelta = trim_s(stringAfterFirst.substr(uComma2 + 1));
         }

         if( stringVar.empty()   ) return { false, "[code::compile_lua] 'for' empty variable at line " + std::to_string(iLine) };
         if( stringFirst.empty() ) return { false, "[code::compile_lua] 'for' empty first value at line " + std::to_string(iLine) };
         if( stringLast.empty()  ) return { false, "[code::compile_lua] 'for' empty last value at line " + std::to_string(iLine) };

         // -- ForInit: var = first
         {
            std::string stringInit = std::string(stringVar) + " = " + std::string(stringFirst);
            std::vector<token> vectorInit;
            auto [bOk, stringErr] = compile_expression_s(stringInit, vectorInit);
            if( bOk == false ) { return { false, stringErr }; }

            statement stmtInit_(statement::eKindForInit, std::move(vectorInit));
            stmtInit_.m_stringSource = std::string(stringTrimmed);
            m_vectorStatement.push_back(std::move(stmtInit_));
         }

         // -- ForCond: var <= last  (inclusive, matching Lua semantics)
         int32_t iForCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         {
            std::string stringCond = std::string(stringVar) + " <= " + std::string(stringLast);
            std::vector<token> vectorCondition;
            auto [bOk, stringErr] = compile_expression_s(stringCond, vectorCondition);
            if( bOk == false ) { return { false, stringErr }; }

            statement stmtCond_(statement::eKindForCond, std::move(vectorCondition));
            m_vectorStatement.push_back(std::move(stmtCond_));
         }

         // -- ForStep: emitted NOW so we can record its index.
         // Layout:
         //   [ForCond]   condition  -> past loop when false
         //   [ForSkip]   ElseJump   -> body start  (skips step on first entry)
         //   [ForStep]   step expr  -> ForCond
         //   [body...]
         //   [LoopBack]             -> ForStep
         //   [past loop]
         int32_t iForStepIndex = static_cast<int32_t>(m_vectorStatement.size()) + 1; // step is at +1 after the skip

         // Emit ForSkip (unconditional jump over ForStep to body start)
         statement stmtSkip_(statement::eKindElseJump);
         stmtSkip_.m_iJump = iForStepIndex + 1; // body starts at step+1
         m_vectorStatement.push_back(std::move(stmtSkip_));

         // Emit ForStep: var = var + delta
         {
            std::string stringStep = std::string(stringVar) + " = " + std::string(stringVar) + " + " + std::string(stringDelta);
            std::vector<token> vectorStep;
            auto [bOk, stringErr] = compile_expression_s(stringStep, vectorStep);
            if( bOk == false ) { return { false, stringErr }; }

            statement stmtStep_(statement::eKindForStep, std::move(vectorStep));
            stmtStep_.m_iJump = iForCondIndex; // step -> cond
            m_vectorStatement.push_back(std::move(stmtStep_));
         }                                                                     assert( static_cast<int32_t>(m_vectorStatement.size()) - 1 == iForStepIndex );

         block frame_;
         frame_.m_eKind         = block::eKindFor;
         frame_.m_iForCondIndex = iForCondIndex;
         frame_.m_iForStepIndex = iForStepIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- break -----------------------------------------------------------
      if( stringKeyword == "break" )
      {
         bool bFoundLoop = false;
         for( int32_t i = static_cast<int32_t>(vectorBlock.size()) - 1; i >= 0; --i )
         {
            if( vectorBlock[i].m_eKind == block::eKindWhile ||
                vectorBlock[i].m_eKind == block::eKindFor )
            {
               int32_t iBreakIndex = static_cast<int32_t>(m_vectorStatement.size());
               m_vectorStatement.emplace_back(statement::eKindBreak);
               vectorBlock[i].m_vectorBreak.push_back(iBreakIndex);
               bFoundLoop = true;
               break;
            }
         }

         if( bFoundLoop == false )
            return { false, "[code::compile_lua] 'break' outside loop at line " + std::to_string(iLine) };

         continue;
      }

      // ---- plain expression -----------------------------------------------
      {
         // std::string stringTranslated = lua_translate_operators_s(stringTrimmed); // @NOTE: keep for later decision on whether to translate operators in expression statements (currently only ~= -> !=)
         std::vector<token> vectorPostfix;
         // auto [bOk, stringErr] = compile_expression_s(stringTranslated, vectorPostfix); 

         auto [bOk, stringErr] = compile_expression_s(stringTrimmed, vectorPostfix);
         if( bOk == false ) { return { false, "[code::compile_lua] expression error at line " + std::to_string(iLine) + ": " + stringErr }; }

         if( vectorPostfix.empty() == false )
         {
            statement stmtExpr_(statement::eKindExpr, std::move(vectorPostfix));
            stmtExpr_.m_stringSource = std::string(stringTrimmed);
            m_vectorStatement.push_back(std::move(stmtExpr_));
         }
      }

   } // while source not exhausted

   // ## patch all pending ElseJump indices in elseif chains
   // (they are collected in m_vectorBreak of closed if-frames; any still on
   //  the stack here indicate an unclosed block)

   if( vectorBlock.empty() == false ) { return { false, "[code::compile_lua] " + std::to_string(vectorBlock.size()) + " unclosed block(s) at end of source" }; }

   return { true, {} };
}


// ===========================================================================
// code::execute
// ===========================================================================

/**
 * @brief Execute all compiled statements.
 *
 * The executor is a plain integer loop with a switch.  No virtual calls.
 * `token::calculate_s` is called at most once per statement.
 *
 * @param runtime_  runtime context (variables, methods, globals)
 * @param pResult   if non-null, receives the value produced by the last
 *                  expression statement that was actually executed
 * @return { true, "" } on success, { false, error-message } on first error
 */
std::pair<bool, std::string> code::execute(runtime& runtime_, value* pResult) const
{
   const int32_t iCount = static_cast<int32_t>(m_vectorStatement.size());
   int32_t iPC = 0;
   value valueLast_;

   while( iPC < iCount )
   {
      const statement& stmt_ = m_vectorStatement[static_cast<size_t>(iPC)];

      switch( stmt_.get_kind() )
      {
      // -- plain expression -------------------------------------------------
      case statement::eKindExpr:
      case statement::eKindForInit:
      {
         if( stmt_.has_tokens() )
         {
            auto [bOk, stringError] = token::calculate_s(stmt_.m_vectorToken, &valueLast_, runtime_);
            if( bOk == false ) return { false, "[code::execute] " + stringError + " in: " + stmt_.m_stringSource };
         }
         ++iPC;
         break;
      }

      // -- if / while / for-cond: evaluate condition, branch on false --------
      case statement::eKindIf:
      case statement::eKindWhile:
      case statement::eKindForCond:
      {
         value valueCondition;
         if( stmt_.has_tokens() )
         {
            auto [bOk, stringErr] = token::calculate_s(stmt_.m_vectorToken, &valueCondition, runtime_);
            if( bOk == false ) { return { false, "[code::execute] condition error: " + stringErr }; }
         }

         if( valueCondition.as_bool() )
         {
            ++iPC; // condition true: fall into body
         }
         else
         {
            if( stmt_.m_iJump < 0 ) { return { false, "[code::execute] unpatched jump in condition statement" }; }
            iPC = stmt_.m_iJump; // condition false: jump past block
         }
         break;
      }

      // -- for-step: run step expression then jump back to condition ---------
      case statement::eKindForStep:
      {
         if( stmt_.has_tokens() )
         {
            auto [bOk, stringErr] = token::calculate_s(stmt_.m_vectorToken, &valueLast_, runtime_);
            if( bOk == false ) { return { false, "[code::execute] for-step error: " + stringErr }; }
         }

         if( stmt_.m_iJump < 0 )
            return { false, "[code::execute] unpatched jump in for-step statement" };
         iPC = stmt_.m_iJump; // jump back to ForCond
         break;
      }

      // -- unconditional jumps (else-skip, loop-back, break, continue) -------
      case statement::eKindElseJump:
      case statement::eKindLoopBack:
      case statement::eKindBreak:
      case statement::eKindContinue:
      {
         if( stmt_.m_iJump < 0 ) { return { false, "[code::execute] unpatched unconditional jump" }; }
         iPC = stmt_.m_iJump;
         break;
      }

      default:
      {
         return { false, "[code::execute] unknown statement kind" };
      }
      } // switch
   } // while

   if( pResult != nullptr ) { *pResult = valueLast_; }

   return { true, {} };
}


// ===========================================================================
// code::dump  (debug helper)
// ===========================================================================

/**
 * @brief Return a human-readable representation of the compiled statements.
 *
 * Useful for verifying compiler output and debugging jump targets.
 */
std::string code::dump() const
{
   static const char* KIND_NAME[] =
   {
      "Expr     ",
      "If       ",
      "ElseJump ",
      "While    ",
      "LoopBack ",
      "ForInit  ",
      "ForCond  ",
      "ForStep  ",
      "Break    ",
      "Continue ",
   };

   std::string stringOut;
   stringOut.reserve(m_vectorStatement.size() * 48);

   for( size_t u = 0; u < m_vectorStatement.size(); ++u )
   {
      const statement& stmt_ = m_vectorStatement[u];
      uint8_t uKind = static_cast<uint8_t>(stmt_.m_eKind);

      // index
      stringOut += '[';
      stringOut += std::to_string(u);
      stringOut += ']';
      if( u < 10  ) { stringOut += ' '; }
      if( u < 100 ) { stringOut += ' '; }
      stringOut += ' ';

      // kind
      if( uKind < sizeof(KIND_NAME) / sizeof(KIND_NAME[0]) )
         stringOut += KIND_NAME[uKind];
      else
         stringOut += "Unknown  ";

      // jump
      if( stmt_.m_iJump >= 0 )
      {
         stringOut += "-> ";
         stringOut += std::to_string(stmt_.m_iJump);
         stringOut += ' ';
      }
      else
      {
         stringOut += "         ";
      }

      // source
      if( stmt_.m_stringSource.empty() == false )
      {
         stringOut += '`';
         stringOut += stmt_.m_stringSource.substr(0, 60);
         if( stmt_.m_stringSource.size() > 60 ) stringOut += "...";
         stringOut += '`';
      }

      // token count
      if( stmt_.m_vectorToken.empty() == false )
      {
         stringOut += "  (";
         stringOut += std::to_string(stmt_.m_vectorToken.size());
         stringOut += " tokens)";
      }

      stringOut += '\n';
   }

   return stringOut;
}

_GD_EXPRESSION_END
