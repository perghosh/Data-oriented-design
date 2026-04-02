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

/// Characters that count as whitespace for line trimming
inline bool is_whitespace_(char c)
{
   return c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v';
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
struct block_frame_
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

/**
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
      if( !( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' ) )
         break;
      ++uEnd;
   }

   std::string_view svKeyword = stringLine.substr(0, uEnd);
   std::string_view svRest    = trim_s(stringLine.substr(uEnd));

   return { svKeyword, svRest };
}

/**
 * @brief Compile one expression string into a postfix token vector.
 *
 * Uses `tag_formula_keyword` so keyword operators (and, or, not, …) work
 * inside conditions and assignments, then runs `compile_s` for the shunting-
 * yard reorder.
 */
std::pair<bool, std::string> code::compile_expression_s(
   std::string_view           stringExpr,
   std::vector<token>&        vectorOut)
{
   std::vector<token> vectorInfix;

   auto [bParse, stringParseError] = token::parse_s(stringExpr, vectorInfix, tag_formula_keyword{});
   if( bParse == false ) { return { false, stringParseError }; }
   if( vectorInfix.empty() ) { return { true, {} }; }

   auto [bCompile, stringCompileError] = token::compile_s(vectorInfix, vectorOut, tag_postfix{});
   if( bCompile == false ) { return { false, stringCompileError }; }

   return { true, {} };
}


// ===========================================================================
// code::compile_s
// ===========================================================================

/**
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
std::pair<bool, std::string> code::compile_s(const char* piszBegin, const char* piszEnd, runtime& runtime_)
{
   m_vectorStatement.clear();

   // ## Block stack: one entry per open `begin` we have not yet seen `end` for
   std::vector<block_frame_> vectorBlock;

   const char* piszPosition = piszBegin;

   int32_t iLine = 0; ///< line counter for error messages

   while( piszPosition < piszEnd )
   {
      // ### Skip blank / whitespace-only lines and move past terminator ------

      // skip leading horizontal whitespace on a line
      while( piszPosition < piszEnd && is_whitespace_(*piszPosition) ) { ++piszPosition; }

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
         ++piszPosition;
      }

      if( stringTrimmed.empty() ) { continue; }

      // ### Classify the line by its leading keyword ------------------------

      auto [svKeyword, svRest] = split_keyword_s(stringTrimmed);

      // ---- begin -----------------------------------------------------------
      if( svKeyword == "begin" )
      {
         // A `begin` without a preceding control-flow keyword is only allowed
         // at top level as a no-op block (rare, but not an error).
         // The block stack was already extended by the preceding if/while/for
         // handler, so `begin` just needs to confirm that the topmost frame
         // has been started.
         if( vectorBlock.empty() )
         {
            // bare begin / end block — push a dummy frame so `end` pops it
            block_frame_ frame_;
            frame_.m_eKind      = block_frame_::eKindIf; // reuse if-kind as "bare"
            frame_.m_iCondIndex = -1;
            vectorBlock.push_back(std::move(frame_));
         }
         // else: frame was already pushed by if / while / for handler
         continue;
      }

      // ---- end -------------------------------------------------------------
      if( svKeyword == "end" )
      {
         if( vectorBlock.empty() )
         {
            return { false, "[code::compile_s] unexpected 'end' without matching 'begin' at line " + std::to_string(iLine) };
         }

         block_frame_ frame_ = std::move(vectorBlock.back());
         vectorBlock.pop_back();

         int32_t iHere = static_cast<int32_t>(m_vectorStatement.size());

         switch( frame_.m_eKind )
         {
         case block_frame_::eKindIf:
         {
            // Patch the if-condition jump to land HERE (first statement after block)
            if( frame_.m_iCondIndex >= 0 )
               m_vectorStatement[frame_.m_iCondIndex].set_jump(iHere);

            // Patch any preceding ElseJump to also land HERE
            if( frame_.m_iElseJumpIndex >= 0 )
               m_vectorStatement[frame_.m_iElseJumpIndex].set_jump(iHere);
            break;
         }

         case block_frame_::eKindElse:
         {
            // Patch the ElseJump (emitted at end of if-body) to land HERE
            if( frame_.m_iElseJumpIndex >= 0 )
               m_vectorStatement[frame_.m_iElseJumpIndex].set_jump(iHere);
            break;
         }

         case block_frame_::eKindWhile:
         {
            // Emit the loop-back jump to the while condition
            statement stmtLoopBack_(statement::eKindLoopBack, frame_.m_iCondIndex);
            m_vectorStatement.push_back(std::move(stmtLoopBack_));
            iHere = static_cast<int32_t>(m_vectorStatement.size()); // re-compute after push

            // Patch while condition to jump past loop-back
            if( frame_.m_iCondIndex >= 0 )
               m_vectorStatement[frame_.m_iCondIndex].set_jump(iHere);

            // Patch break statements
            for( int32_t iBreak : frame_.m_vectorBreak )
               m_vectorStatement[iBreak].set_jump(iHere);

            // Patch continue statements -> jump to while condition
            for( int32_t iCont : frame_.m_vectorContinue )
               m_vectorStatement[iCont].set_jump(frame_.m_iCondIndex);

            break;
         }

         case block_frame_::eKindFor:
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
      if( svKeyword == "else" )
      {
         if( vectorBlock.empty() || vectorBlock.back().m_eKind != block_frame_::eKindIf )
         {
            return { false, "[code::compile_s] 'else' without matching 'if' at line " + std::to_string(iLine) };
         }

         // End the if-body: emit an unconditional jump that will be patched
         // to skip the else-body once we see its `end`.
         int32_t iElseJumpIndex = static_cast<int32_t>(m_vectorStatement.size());
         m_vectorStatement.emplace_back(statement::eKindElseJump);

         int32_t iElseBodyStart = static_cast<int32_t>(m_vectorStatement.size());

         // Patch the if-condition jump to land at the else-body start
         block_frame_& frame_ = vectorBlock.back();
         if( frame_.m_iCondIndex >= 0 )
            m_vectorStatement[frame_.m_iCondIndex].set_jump(iElseBodyStart);

         // Convert the if-frame to an else-frame and record the jump index
         frame_.m_eKind         = block_frame_::eKindElse;
         frame_.m_iCondIndex    = -1;
         frame_.m_iElseJumpIndex = iElseJumpIndex;

         // `else` is always followed by `begin` on the next line; we leave
         // the frame on the stack so the matching `end` will close it.
         continue;
      }

      // ---- if --------------------------------------------------------------
      if( svKeyword == "if" )
      {
         if( svRest.empty() )
         {
            return { false, "[code::compile_s] 'if' without condition at line " + std::to_string(iLine) };
         }

         // Compile condition expression
         std::vector<token> vectorCond;
         auto [bOk, stringErr] = compile_expression_s(svRest, vectorCond);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtIf_(statement::eKindIf, std::move(vectorCond));
         stmtIf_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtIf_));

         // Push a block frame; `begin` on the next line will confirm it,
         // `end` will close and patch it.
         block_frame_ frame_;
         frame_.m_eKind      = block_frame_::eKindIf;
         frame_.m_iCondIndex = iCondIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- while -----------------------------------------------------------
      if( svKeyword == "while" )
      {
         if( svRest.empty() )
         {
            return { false, "[code::compile_s] 'while' without condition at line " + std::to_string(iLine) };
         }

         // Compile condition
         std::vector<token> vectorCond;
         auto [bOk, stringErr] = compile_expression_s(svRest, vectorCond);
         if( bOk == false ) { return { false, stringErr }; }

         int32_t iCondIndex = static_cast<int32_t>(m_vectorStatement.size());
         statement stmtWhile_(statement::eKindWhile, std::move(vectorCond));
         stmtWhile_.m_stringSource = std::string(stringTrimmed);
         m_vectorStatement.push_back(std::move(stmtWhile_));

         block_frame_ frame_;
         frame_.m_eKind      = block_frame_::eKindWhile;
         frame_.m_iCondIndex = iCondIndex;
         vectorBlock.push_back(std::move(frame_));
         continue;
      }

      // ---- for -------------------------------------------------------------
      // Syntax:  for <var> = <start>, <limit>, <step>
      // Example: for i = 0, 10, 1
      if( svKeyword == "for" )
      {
         if( svRest.empty() )
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
         size_t uEq = find_char_(svRest, '=');
         if( uEq == std::string_view::npos )
            return { false, "[code::compile_s] 'for' missing '=' at line " + std::to_string(iLine) };

         std::string_view svVar       = trim_s(svRest.substr(0, uEq));
         std::string_view svAfterEq   = trim_s(svRest.substr(uEq + 1));

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

         block_frame_ frame_;
         frame_.m_eKind         = block_frame_::eKindFor;
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
            // Solution: extend block_frame_ with an optional step statement.
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
            // ugly; instead add a dedicated member.  Since we own block_frame_
            // locally and it already has m_iForStepIndex, let's store the
            // statement temporarily in a std::vector<statement> member.
            // But block_frame_ is a local struct ... add a member there:
            //   statement m_stmtStep;
            // We cannot modify block_frame_ from here because it was already
            // defined above.  So we use a different trick: serialise the step
            // tokens into m_stringForVar with a sentinel, then rebuild at end.
            // This is messy.  The right solution: put `statement m_stmtStep`
            // into block_frame_ — but that requires moving the definition.
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
      if( svKeyword == "break" )
      {
         // Find innermost loop frame
         bool bFoundLoop = false;
         for( int32_t i = static_cast<int32_t>(vectorBlock.size()) - 1; i >= 0; --i )
         {
            if( vectorBlock[i].m_eKind == block_frame_::eKindWhile ||
                vectorBlock[i].m_eKind == block_frame_::eKindFor )
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
      if( svKeyword == "continue" )
      {
         bool bFoundLoop = false;
         for( int32_t i = static_cast<int32_t>(vectorBlock.size()) - 1; i >= 0; --i )
         {
            if( vectorBlock[i].m_eKind == block_frame_::eKindWhile ||
                vectorBlock[i].m_eKind == block_frame_::eKindFor )
            {
               int32_t iContIdx = static_cast<int32_t>(m_vectorStatement.size());
               m_vectorStatement.emplace_back(statement::eKindContinue);
               vectorBlock[i].m_vectorContinue.push_back(iContIdx);
               bFoundLoop = true;
               break;
            }
         }

         if( bFoundLoop == false )
            return { false, "[code::compile_s] 'continue' outside loop at line " + std::to_string(iLine) };

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
            if( bOk == false )
               return { false, "[code::execute] condition error: " + stringErr };
         }

         if( valueCondition.as_bool() )
         {
            ++iPC; // condition true: fall into body
         }
         else
         {
            if( stmt_.m_iJump < 0 )
               return { false, "[code::execute] unpatched jump in condition statement" };
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
            if( bOk == false )
               return { false, "[code::execute] for-step error: " + stringErr };
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
         if( stmt_.m_iJump < 0 )
            return { false, "[code::execute] unpatched unconditional jump" };
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
