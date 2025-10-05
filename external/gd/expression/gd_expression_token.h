/**
 * @file gd_expression_token.h
 * @brief Token item struct, definitions for expression parsing and evaluation using tokens
 * 
 * | Area                | Methods (Examples)                                                                 | Description                                                                                   |
 * |---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 * | Construction        | token(), token(uint32_t, const std::string_view&), operator=(...), common_construct(...) | Constructors and assignment operators for creating and copying token objects.                 |
 * | Value Management    | set_type(...), set_name(...)                                                       | Methods for setting token type and name.                                                      |
 * | Access/Query        | get_token_type(), get_value_type(), get_function_type(), get_token_part(), get_token_group(), get_type(), get_name() | Methods for retrieving token type, value type, function type, part, group, and name.          |
 * | Conversion          | as_value()                                                                         | Converts token to value type for evaluation.                                                  |
 * | Parsing/Compilation | parse_s(...), compile_s(...)                                                       | Methods for parsing expressions into tokens and compiling to postfix notation.                |
 * | Calculation         | calculate_s(...)                                                                   | Methods for evaluating tokenized expressions and returning results.                           |
 * | Utility             | skip_whitespace_s(...), read_number_s(...), read_string_s(...), read_variable_and_s(...), type_s(...), to_type_s(...), token_type_s(...) | Utility methods for whitespace skipping, reading numbers/strings/variables, and type handling.|
 */

#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <variant>

#include "gd_expression.h"
#include "gd_expression_value.h"
#include "gd_expression_runtime.h"

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 

struct tag_formula {}; ///< tag for formula
struct tag_formula_keyword {}; ///< tag for formula with keywords
struct tag_expression {}; ///< tag for expression
struct tag_postfix {}; ///< tag for postfix

enum enumTokenType
{
   eTokenTypeNone            = 0,
   eTokenTypeKeyword         = 1,
   eTokenTypeOperator        = 2,
   eTokenTypeSeparator       = 3,
   eTokenTypeStringDelimiter = 4,
   eTokenTypeSpecialChar     = 5,
   eTokenTypeValue           = 6,
   eTokenTypeIdentifier      = 7,
   eTokenTypeFormula         = 8,
   eTokenTypeFunction        = 9,
   eTokenTypeVariable        = 10,
   eTokenTypeLabel           = 11,
   eTokenTypeMember          = 12,
   eTokenTypeEnd             = 13,

   eTokenTypeBlock           = 14,
   eTokenTypeStatement       = 15,
};



enum enumKeyword
{
   eKeywordIf                = 0x0001,
   eKeywordElse              = 0x0002,
   eKeywordElseIf            = 0x0003,
   eKeywordWhile             = 0x0004,
   eKeywordFor               = 0x0005,
   eKeywordSwitch            = 0x0006,
   eKeywordDo                = 0x0007,
   eKeywordReturn            = 0x0008,
   eKeywordBreak             = 0x0009,
   eKeywordContinue          = 0x000A,
   eKeywordCase              = 0x000B,
   eKeywordDefault           = 0x000C,
   eKeywordGoto              = 0x000D,
   eKeywordThis              = 0x000E,

   eKeywordBegin             = 0x0010,
   eKeywordEnd               = 0x0011,
};

enum enumFunction
{
   eFunctionNamespace        = 0x0100,
};

enum enumValueType
{
   eValueTypeNone            = 0,
   eValueTypeBoolean         = 1, 
   eValueTypeInteger         = 8,
   eValueTypeDecimal         = 11,
   eValueTypePointer         = 12,
   eValueTypeString          = 14,
   eValueTypeUtf8            = 15,
   eValueTypeBinary          = 18,
};

enum enumTokenPart
{
   eTokenPartToken           = 0,
   eTokenPartType            = 1,
};

/**
 * @brief Represents a token in an expression.
 *
 * A token is a fundamental unit of an expression, such as an operator, operand, or identifier.
 * Each token has a type and a name, which help in parsing and evaluating expressions.
 * 
 * The name in this token is a string view that references the original expression string, avoiding unnecessary copies
 * or using the list of operators that exists as from the static member method called operator_s
 */
struct token 
{
   /// @brief constant values for operator types
   enum enumOperator
   {
      eOperatorNone              = 0,   //  0: no operator
      eOperatorAdd               = 1,   //  1: +
      eOperatorSubtract          = 2,   //  2: -
      eOperatorAddAssign         = 3,   //  3: +=
      eOperatorAssign            = 4,   //  4: =
      eOperatorBitwiseAnd        = 5,   //  5: &
      eOperatorBitwiseAndAssign  = 6,   //  6: &=
      eOperatorBitwiseNot        = 7,   //  7: ~
      eOperatorBitwiseOr         = 8,   //  8: |
      eOperatorBitwiseOrAssign   = 9,   //  9: |=
      eOperatorBitwiseXor        = 10,  // 10: ^
      eOperatorBitwiseXorAssign  = 11,  // 11: ^=
      eOperatorComma             = 12,  // 12: ,
      eOperatorDecrement         = 13,  // 13: --
      eOperatorDivide            = 14,  // 14: /
      eOperatorDivideAssign      = 15,  // 15: /=
      eOperatorEqual             = 16,  // 16: ==
      eOperatorGreaterThan       = 17,  // 17: >
      eOperatorGreaterThanEqual  = 18,  // 18: >=
      eOperatorIncrement         = 19,  // 19: ++
      eOperatorLeftShift         = 20,  // 20: <<
      eOperatorLeftShiftAssign   = 21,  // 21: <<=
      eOperatorLessThan          = 22,  // 22: <
      eOperatorLessThanEqual     = 23,  // 23: <=
      eOperatorLogicalAnd        = 24,  // 24: &&
      eOperatorLogicalNot        = 25,  // 25: !
      eOperatorLogicalOr         = 26,  // 26: ||
      eOperatorModulus           = 27,  // 27: %
      eOperatorModulusAssign     = 28,  // 28: %=
      eOperatorMultiply          = 29,  // 29: *
      eOperatorMultiplyAssign    = 30,  // 30: *=
      eOperatorNotEqual          = 31,  // 31: !=
      eOperatorRightShift        = 32,  // 32: >>
      eOperatorRightShiftAssign  = 33   // 33: >>=
   };

// ## construction ------------------------------------------------------------
   token() : m_uType(0), m_stringName("") {}
   token(uint32_t uType, const std::string_view& string_) : m_uType(uType), m_stringName(string_) {}
   // copy
   token(const token& o) { common_construct(o); }
   token(token&& o) noexcept { common_construct(std::move(o)); }
   // assign
   token& operator=(const token& o) { common_construct(o); return *this; }
   token& operator=(token&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~token() {}
   // common copy
   void common_construct(const token& o) {
       m_uType = o.m_uType;
       m_stringName = o.m_stringName;
   }
   void common_construct(token&& o) noexcept {
       m_uType = o.m_uType;
       m_stringName = std::move(o.m_stringName);
   }

// ## get/set -----------------------------------------------------------------
   uint32_t get_token_type() const { return m_uType & 0xFF; }
   uint32_t get_value_type() const { return ( m_uType >> 8 ) & 0xFF; }
   uint32_t get_function_type() const { return ( m_uType & 0xFF00 ); }
   uint32_t get_token_part() const { return ( m_uType >> 16 ) & 0xFF; }
   uint32_t get_token_group() const { return ( m_uType >> 24 ) & 0xFF; }

// ## methods -----------------------------------------------------------------
   void set_type(uint32_t type) { m_uType = type; }
   uint32_t get_type() const { return m_uType; }

   void set_name(const std::string_view& name) { m_stringName = name; }
   std::string_view get_name() const { return m_stringName; }

   value as_value() const;

/** \name DEBUG
*///@{
//@}

// ## attributes --------------------------------------------------------------
   uint32_t m_uType;
   std::string_view m_stringName;

// ## free functions ----------------------------------------------------------

   // ## move methods

   static const char* skip_whitespace_s(const char* piszBegin, const char* piszEnd);

   // ## parse methods

   static std::pair<bool, std::string> parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_formula);
   static std::pair<bool, std::string> parse_s(const std::string_view& stringExpression, std::vector<token>& vectorToken, tag_formula);
   static std::pair<bool, std::string> parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_formula_keyword);
   static std::pair<bool, std::string> parse_s(const std::string_view& stringExpression, std::vector<token>& vectorToken, tag_formula_keyword);

	// ## compile and calculate methods

   static std::pair<bool, std::string> compile_s(const std::vector<token>& vectorIn, std::vector<token>& vectorOut, tag_postfix);
   static std::pair<bool, std::string> calculate_s( const std::vector<token>& vectorToken, value* pvalueResult );
   static std::pair<bool, std::string> calculate_s(const std::vector<token>& vectorToken, value* pvalueResult, runtime& runtime_);
   static std::pair<bool, std::string> calculate_s(const std::vector<token>& vectorToken, std::vector<value>* pvectorReturn, runtime& runtime_);
   /// @brief calculate_s that is simplified for one liner expression that returns single value, make sure that the formula is correct, only compile time checks and twrows
   static value calculate_s( const std::string_view& stringExpression, const std::vector< std::pair<std::string, value::variant_t>>& vectorVariable );
   /// @brief calculate_s that is simplified for one liner expression and this also take a callback function that will be called when the runtime is created, this is useful for setting up the runtime with variables and other information
   static value calculate_s( const std::string_view& stringExpression, const std::vector< std::pair<std::string, value::variant_t>>& vectorVariable, std::function< void( runtime& runtime )> callback_ );
   static value calculate_s( const std::string_view& stringExpression, runtime& runtime_ );
   static value calculate_s( const std::string_view& stringExpression, std::unique_ptr<runtime>& pruntime );
   static value calculate_s( const std::string_view& stringExpression );

	// ## read methods

   static uint32_t read_number_s(const char* piszBegin, const char* piszEnd, std::string_view& string_); 
   static uint32_t read_string_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char**  ppiszReadTo );
   static std::pair<uint32_t, uint32_t> read_variable_and_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char**  ppiszReadTo); 

   static uint32_t type_s( uint32_t uType, enumTokenPart eTokenPart );
   static uint32_t to_type_s( uint32_t uType, enumTokenPart eTokenPart );

   static constexpr uint32_t token_type_s(const std::string_view& s_);

   /// @brief read keyword operator from string, returns true if a keyword was found
   static bool operator_read_keyword_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char** ppiszReadTo);

   // ## operator methods

   static std::string_view operator_s(uint32_t uOperator);
   static uint32_t operator_s(std::string_view stringOperator);
};

/// @brief parse expression string to tokens
inline std::pair<bool, std::string> token::parse_s(const std::string_view& stringExpression, std::vector<token>& vectorToken, tag_formula) {
   return parse_s(stringExpression.data(), stringExpression.data() + stringExpression.length(), vectorToken, tag_formula());
}

/// @brief parse expression string to tokens
inline std::pair<bool, std::string> token::parse_s(const std::string_view& stringExpression, std::vector<token>& vectorToken, tag_formula_keyword) {
   return parse_s(stringExpression.data(), stringExpression.data() + stringExpression.length(), vectorToken, tag_formula_keyword());
}


/// @brief calculate_s that is simplified  where this version removes then need for runtime
inline std::pair<bool, std::string> token::calculate_s(const std::vector<token>& vectorToken, value* pvalueResult) {
   runtime runtime_;
   return calculate_s(vectorToken, pvalueResult, runtime_);
}

/// \brief get type information from token type
inline uint32_t token::type_s(uint32_t uType, enumTokenPart eTokenPart) {
   return (uType >> (static_cast<unsigned>(eTokenPart) * 8)) & 0xFF;
}

/// @brief get byte part from type, conviniense method to get byte part from type
inline uint32_t token::to_type_s(uint32_t uType, enumTokenPart eTokenPart) {
   return uType << (static_cast<unsigned>(eTokenPart) * 8);
}


constexpr uint32_t token::token_type_s(const std::string_view& s_) {
   if(s_ == "NONE") return eTokenTypeNone;
   if(s_ == "KEYWORD") return eTokenTypeKeyword;
   if(s_ == "OPERATOR") return eTokenTypeOperator;
   if(s_ == "SEPARATOR") return eTokenTypeSeparator;
   if(s_ == "STRING_DELIMITER") return eTokenTypeStringDelimiter;
   if(s_ == "SPECIAL_CHAR") return eTokenTypeSpecialChar;
   if(s_ == "VALUE") return eTokenTypeValue;
   if(s_ == "IDENTIFIER") return eTokenTypeIdentifier;
   if(s_ == "FORMULA") return eTokenTypeFormula;
   if(s_ == "FUNCTION") return eTokenTypeFunction;
   if(s_ == "VARIABLE") return eTokenTypeVariable;
   if(s_ == "END") return eTokenTypeEnd;
   throw std::invalid_argument("Invalid token type string");
}

_GD_EXPRESSION_END