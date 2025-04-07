#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
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
struct tag_expression {}; ///< tag for expression
struct tag_postfix {}; ///< tag for postfix

enum enumParseValue
{
   eParseValueNone = 0x0000,
   eParseValueInteger = 0x0001,
   eParseValueDecimal = 0x0002,
   eParseValueString = 0x0003,
   eParseValueBoolean = 0x0004,
   eParseValueNull = 0x0005,
   eParseValueFunction = 0x0006,
   eParseValueVariable = 0x0007,
};

enum enumTokenType
{
   eTokenTypeNone = 0,
   eTokenTypeKeyword = 1,
   eTokenTypeOperator = 2,
   eTokenTypeSeparator = 3,
   eTokenTypeStringDelimiter = 4,
   eTokenTypeSpecialChar = 5,
   eTokenTypeValue = 6,
   eTokenTypeIdentifier = 7,
   eTokenTypeFormula = 8,
   eTokenTypeFunction = 9,
   eTokenTypeVariable = 10,
   eTokenTypeLabel = 11,
   eTokenTypeMember = 12,
   eTokenTypeEnd = 13,
};



enum enumKeyword
{
   eKeywordIf        = 0x0001,
   eKeywordElse      = 0x0002,
   eKeywordWhile     = 0x0003,
   eKeywordFor       = 0x0004,
   eKeywordSwitch    = 0x0005,
   eKeywordDo        = 0x0006,
   eKeywordReturn    = 0x0007,
   eKeywordBreak     = 0x0008,
   eKeywordContinue  = 0x0009,
   eKeywordCase      = 0x000A,
   eKeywordDefault   = 0x000B,
   eKeywordGoto      = 0x000C,
   eKeywordThis      = 0x000D,

   eKeywordBegin     = 0x0010,
   eKeywordEnd       = 0x0011,
};

enum enumValueType
{
   eValueTypeNone    = 0,
   eValueTypeBoolean = 1, 
   eValueTypeInteger = 8,
   eValueTypeDecimal = 11,
   eValueTypePointer = 12,
   eValueTypeString  = 14,
   eValueTypeUtf8    = 15,
   eValueTypeBinary  = 18,
};

enum enumTokenPart
{
   eTokenPartToken = 0,
   eTokenPartType = 1,
};

/**
 * \brief
 *
 *
 */
struct token 
{
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
   static const char* skip_whitespace_s(const char* piszBegin, const char* piszEnd);
   static std::pair<bool, std::string> parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_formula);
   static std::pair<bool, std::string> parse_s(const std::string_view& stringExpression, std::vector<token>& vectorToken, tag_formula);
   static std::pair<bool, std::string> compile_s(const std::vector<token>& vectorIn, std::vector<token>& vectorOut, tag_postfix);
   static std::pair<bool, std::string> calculate_s( const std::vector<token>& vectorToken, value* pvalueResult );
   static std::pair<bool, std::string> calculate_s(const std::vector<token>& vectorToken, value* pvalueResult, runtime& runtime_);

   static uint32_t read_number_s(const char* piszBegin, const char* piszEnd, std::string_view& string_); 
   static uint32_t read_string_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char**  ppiszReadTo );
   static std::pair<uint32_t, enumTokenType> read_variable_and_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char**  ppiszReadTo); 



   static uint32_t type_s( uint32_t uType, enumTokenPart eTokenPart );
   static uint32_t to_type_s( uint32_t uType, enumTokenPart eTokenPart );

   static constexpr uint32_t token_type_s(const std::string_view& s_);
   

};

inline std::pair<bool, std::string> token::parse_s(const std::string_view& stringExpression, std::vector<token>& vectorToken, tag_formula) {
   return parse_s(stringExpression.data(), stringExpression.data() + stringExpression.length(), vectorToken, tag_formula());
}

inline std::pair<bool, std::string> token::calculate_s(const std::vector<token>& vectorToken, value* pvalueResult) {
   runtime runtime_;
   return calculate_s(vectorToken, pvalueResult, runtime_);
}

/// \brief get type information from token type
inline uint32_t token::type_s(uint32_t uType, enumTokenPart eTokenPart) {
   return (uType >> (static_cast<unsigned>(eTokenPart) * 8)) & 0xFF;
}

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