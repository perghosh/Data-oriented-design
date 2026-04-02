/**
 * @file gd_expression_code.h
 * @TAG #gd::expression
 *
 * @brief Provides a lightweight flat-bytecode program container that adds
 *        control-flow (if/else/while/for) on top of the existing expression
 *        evaluator without modifying it.
 *
 * ## Design
 *
 * A `code` object holds a flat `vector<statement>`.  Each statement owns its
 * pre-compiled postfix token list (ready to hand straight to
 * `token::calculate_s`) plus a jump index that the executor uses to branch.
 * There are no heap-allocated nodes and no virtual calls; branching is a
 * single integer assignment to a program-counter variable.
 *
 * ## Syntax accepted by `code::compile_s`
 *
 * The compiler expects a line-oriented script where each logical line ends
 * with a newline or a semicolon (`;`).  Block structure is delimited by the
 * keywords `begin` / `end` (chosen because they are already in `enumKeyword`
 * and are trivial to parse without nesting ambiguity).
 *
 * ```
 * x = 1
 * if x > 0
 * begin
 *     y = x * 2
 * end
 * else
 * begin
 *     y = 0
 * end
 *
 * while x < 10
 * begin
 *     x = x + 1
 * end
 *
 * for i = 1, 10, 1
 * begin
 *     x = x + i
 * end
 * ```
 *
 * `for` syntax: `for <var> = <start>, <limit>, <step>`
 * The three parts after `=` are plain expressions separated by commas.
 * Inside a for-body `break` and `continue` are supported.
 *
 * ## Strings
 *
 * String literals are passed through to `token::read_string_s` which already
 * handles `'...'`, `"..."`, `'''...'''`, and `"""..."""`.  The scanner in this
 * file only needs to know where a string starts and ends so it can skip over
 * it without treating its content as syntax (newlines inside a triple-quoted
 * string are not statement terminators).
 *
 * | Area             | Methods                                                       | Description                                     |
 * |------------------|---------------------------------------------------------------|-------------------------------------------------|
 * | statement        | (struct)                                                      | One compiled unit: kind + jump + token list.    |
 * | Construction     | code(), code(const code&), code(code&&)                       | Standard construction / copy / move.            |
 * | Compile          | compile_s(string_view, runtime&)                              | Parse source text into statements.              |
 * | Execute          | execute(runtime&, value*)                                     | Run all statements, optionally return last val. |
 * | Utility          | clear(), statement_count(), dump()                            | Inspection / reset helpers.                     |
 * | String scanning  | scan_string_end_s(begin,end)                                  | Skip over any string literal (replaceable).     |
 * | Line scanning    | scan_line_end_s(begin,end)                                    | Find end of logical line, respecting strings.   |
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd_expression.h"
#include "gd_expression_value.h"
#include "gd_expression_runtime.h"
#include "gd_expression_token.h"

#ifndef _GD_EXPRESSION_BEGIN
#  define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#  define _GD_EXPRESSION_END   } }
#endif

_GD_EXPRESSION_BEGIN

/**
 * @brief One compiled unit of execution inside a `code` program.
 *
 * A statement is the atom of the flat bytecode.  The executor walks a
 * `vector<statement>` sequentially and only jumps when the kind demands it.
 *
 * ### Jump semantics
 *
 * | Kind            | Condition token list | m_iJump meaning                          |
 * |-----------------|----------------------|------------------------------------------|
 * | eKindExpr       | expression to run    | unused (-1)                              |
 * | eKindIf         | condition            | index to jump to when condition is false |
 * | eKindElseJump   | empty                | unconditional jump past else-body        |
 * | eKindWhile      | condition            | index to jump to when condition is false |
 * | eKindLoopBack   | empty                | unconditional jump back to while/for     |
 * | eKindForInit    | init expression      | unused (-1)                              |
 * | eKindForCond    | condition            | index to jump past loop when false       |
 * | eKindForStep    | step expression      | m_iJump = index of eKindForCond          |
 * | eKindBreak      | empty                | index past enclosing loop end            |
 * | eKindContinue   | empty                | index of loop step / condition           |
 */
struct statement
{
   enum enumKind : uint8_t
   {
      eKindExpr      = 0,  ///< plain expression, always executed
      eKindIf        = 1,  ///< evaluate condition; false -> jump to m_iJump
      eKindElseJump  = 2,  ///< end-of-if-body: unconditional jump past else
      eKindWhile     = 3,  ///< evaluate condition; false -> jump to m_iJump
      eKindLoopBack  = 4,  ///< end-of-loop-body: jump back (m_iJump = head)
      eKindForInit   = 5,  ///< for: init assignment, run once
      eKindForCond   = 6,  ///< for: condition check; false -> jump past loop
      eKindForStep   = 7,  ///< for: step expression; then jump to eKindForCond
      eKindBreak     = 8,  ///< jump to first statement past enclosing loop
      eKindContinue  = 9,  ///< jump to loop step / condition
   };

   // ## construction ----------------------------------------------------------

   statement() {}

   explicit statement(enumKind eKind) : m_eKind(eKind) {}

   statement(enumKind eKind, int32_t iJump) : m_eKind(eKind), m_iJump(iJump) {}

   statement(enumKind eKind, std::vector<token>&& vectorToken_)
      : m_eKind(eKind), m_vectorToken(std::move(vectorToken_)) {}

   // copy
   statement(const statement& o) { common_construct(o); }
   statement(statement&& o) noexcept { common_construct(std::move(o)); }

   // assign
   statement& operator=(const statement& o)      { common_construct(o);            return *this; }
   statement& operator=(statement&& o) noexcept  { common_construct(std::move(o)); return *this; }

   ~statement() {}

   void common_construct(const statement& o)
   {
      m_eKind       = o.m_eKind;
      m_iJump       = o.m_iJump;
      m_vectorToken = o.m_vectorToken;
      m_stringSource = o.m_stringSource;
   }

   void common_construct(statement&& o) noexcept
   {
      m_eKind        = o.m_eKind;
      m_iJump        = o.m_iJump;
      m_vectorToken  = std::move(o.m_vectorToken);
      m_stringSource = std::move(o.m_stringSource);
   }

   // ## get/set ---------------------------------------------------------------

   enumKind               get_kind()  const { return m_eKind; }
   int32_t                get_jump()  const { return m_iJump; }
   const std::vector<token>& get_tokens() const { return m_vectorToken; }

   void set_jump(int32_t iJump) { m_iJump = iJump; }

   bool has_tokens() const { return m_vectorToken.empty() == false; }

   // ## attributes ------------------------------------------------------------

   enumKind            m_eKind        = eKindExpr; ///< what this statement does
   int32_t             m_iJump        = -1;         ///< branch target index, -1 = none
   std::vector<token>  m_vectorToken;               ///< pre-compiled postfix tokens
   std::string         m_stringSource;              ///< original source text (debug / error messages)
};


/**
 * @brief Flat-bytecode program container with if / else / while / for support.
 *
 * A `code` object is the single addition needed to give the expression engine
 * control-flow.  It reuses all existing types (`token`, `value`, `runtime`)
 * without modification.
 *
 * ### Lifecycle
 *
 * ```cpp
 * gd::expression::runtime runtime_;
 * runtime_.add("x", int64_t(0));
 *
 * gd::expression::code code_;
 * auto [bOk, stringError] = code_.compile_s(R"(
 *     x = 1
 *     if x > 0
 *     begin
 *         x = x * 10
 *     end
 * )", runtime_);
 *
 * if( bOk )
 * {
 *     gd::expression::value result_;
 *     code_.execute(runtime_, &result_);
 * }
 * ```
 */
struct code
{
   // ## construction ----------------------------------------------------------

   code() {}

   // copy
   code(const code& o) { common_construct(o); }
   code(code&& o) noexcept { common_construct(std::move(o)); }

   // assign
   code& operator=(const code& o)      { common_construct(o);            return *this; }
   code& operator=(code&& o) noexcept  { common_construct(std::move(o)); return *this; }

   ~code() {}

   void common_construct(const code& o)      { m_vectorStatement = o.m_vectorStatement; }
   void common_construct(code&& o) noexcept  { m_vectorStatement = std::move(o.m_vectorStatement); }

   // ## methods ---------------------------------------------------------------

   /// @brief number of compiled statements
   size_t statement_count() const { return m_vectorStatement.size(); }

   /// @brief true if no statements have been compiled yet
   bool empty() const { return m_vectorStatement.empty(); }

   /// @brief remove all compiled statements, ready for a new compile
   void clear() { m_vectorStatement.clear(); }

   // ## compile ---------------------------------------------------------------

   /// @brief compile source text into statements; runtime is used only for
   ///        variable lookup during expression pre-compilation (not execution)
   std::pair<bool, std::string> compile(const std::string_view& stringSource, runtime& runtime_);

   /// @brief compile from raw char pointers
   std::pair<bool, std::string> compile(const char* piszBegin, const char* piszEnd, runtime& runtime_);

   /// @brief compile from null-terminated C string (convenience overload)
   std::pair<bool, std::string> compile(const char* piszSource, runtime& runtime_);

   // ## execute ---------------------------------------------------------------

   /// @brief execute all compiled statements
   /// @param runtime_   runtime context (variables, methods)
   /// @param pResult    if non-null receives the value of the last expression
   /// @return { true, "" } on success, { false, message } on error
   std::pair<bool, std::string> execute(runtime& runtime_, value* pResult = nullptr) const;

   /** @name DEBUG
    *///@{
   std::string dump() const;
   //@}

   // ## attributes ------------------------------------------------------------

   std::vector<statement> m_vectorStatement; ///< flat list of compiled statements

   // ## free (static) functions -----------------------------------------------

   /// @brief skip over a string literal, returning pointer past closing delimiter
   ///
   /// Delegates to `token::read_string_s` so the behaviour is identical to the
   /// rest of the expression system.  Replace this function if you integrate
   /// a different string reader.
   ///
   /// @param piszBegin  points at the opening quote character (' or ")
   /// @param piszEnd    one past the end of the source buffer
   /// @return pointer past the closing delimiter, or piszEnd on unterminated string
   static const char* scan_string_end_s(const char* piszBegin, const char* piszEnd);

   /// @brief find the end of one logical line in the source text
   ///
   /// A logical line ends at `\n`, `;`, or `piszEnd`.  String literals are
   /// skipped so that newlines inside `'''` / `"""` do not terminate the line.
   ///
   /// @param piszBegin  start of current scan position
   /// @param piszEnd    one past the end of the source buffer
   /// @param ppiszLineEnd  out-param: pointer to the terminating character
   ///                      (the `\n`, `;`, or piszEnd itself)
   /// @return string_view covering the line content (without the terminator)
   static std::string_view scan_line_end_s(const char* piszBegin, const char* piszEnd,
                                           const char** ppiszLineEnd = nullptr);

   /// @brief trim leading and trailing whitespace from a string_view
   static std::string_view trim_s(std::string_view stringLine);

   /// @brief check whether the trimmed line starts with a given keyword
   ///        (case-sensitive, must be followed by whitespace or end-of-string)
   static bool starts_with_keyword_s(std::string_view stringLine, std::string_view stringKeyword);

   /// @brief extract the keyword at the front of a line and return the remainder
   ///        e.g. "if x > 0" -> keyword="if", rest="x > 0"
   ///        returns { keyword, rest } where keyword is empty if no keyword found
   static std::pair<std::string_view, std::string_view> split_keyword_s(std::string_view stringLine);

   /// @brief compile one assignment / plain expression line (uses tag_formula so
   ///        a lone '=' is treated as assignment, not equality)
   static std::pair<bool, std::string> compile_expression_s( std::string_view stringExpr, std::vector<token>& vectorOut);

   /// @brief compile a condition expression (uses tag_formula_keyword so
   ///        keyword operators like 'and', 'or', 'not' are recognised and
   ///        a lone '=' is treated as '==')
   static std::pair<bool, std::string> compile_condition_s( std::string_view stringExpr, std::vector<token>& vectorOut);
};

// ---------------------------------------------------------------------------
// inline helpers
// ---------------------------------------------------------------------------

/// @brief compile from std::string_view - thin redirect to pointer overload
inline std::pair<bool, std::string> code::compile(const std::string_view& stringSource, runtime& runtime_)
{
   return compile(stringSource.data(), stringSource.data() + stringSource.size(), runtime_);
}

_GD_EXPRESSION_END
