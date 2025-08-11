/**
* @file gd_parse_formats.h
* @brief Parse logic for patterns that isn't as standard but useful in different contexts.
* 
* Parse logic for know formats
*/


#pragma once

#include "gd/gd_compiler.h"

#include <array>
#include <cassert>
#include <cstring>
#if GD_COMPILER_HAS_CPP20_SUPPORT
#include <span>
#endif
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_BEGIN
#  define _GD_PARSE_BEGIN namespace gd { namespace parse {
#  define _GD_PARSE_END } }
#endif

_GD_PARSE_BEGIN

/// tag dispatcher for code specific overloads
struct tag_code {};


/**
 * @brief Parse logic for key-value pairs and other formats.
 *
 * This structure defines the parsing options and methods for handling key-value pairs, and this parsing logic
 * tries to be as flexible as possible to allow for different formats and styles.
 * 
 * If only one quote character is used text is trimmed, but more than one quote character is used, then the text is not trimmed.
 * If separator is not set, then it assumes that value starts just after the key, so no separator is used.
 * When brackets are used, it assumes that the key-value pairs are enclosed within those brackets.
 * 
 * *sample*
 * 
 */
struct code
{
   enum enumOptions
   {
      eTrim = 1 << 0,     ///< Trim whitespace from keys and values
      eAllowUnquoted = 1 << 1,      ///< Allow unquoted values
      eStrictQuoting = 1 << 2,      ///< Require matching quote types
      eSkipEmpty = 1 << 3,          ///< Skip empty key-value pairs
      eOptionsMAX = 1 << 4,
   };

   code() : m_uOpenBracket(0), m_uCloseBracket(0), m_uKeySeparator(0), m_stringQuoteChars("\"'"), m_uOptions(eTrim | eAllowUnquoted) {}
   /// Construct with custom brackets
   code(uint8_t uOpenBracket, uint8_t uCloseBracket) : m_uOpenBracket(uOpenBracket), m_uCloseBracket(uCloseBracket), m_uKeySeparator(':'), m_stringQuoteChars("\"'`"), m_uOptions(eTrim | eAllowUnquoted) {}
   /// Construct with custom brackets and key separator
   code(uint8_t uOpenBracket, uint8_t uCloseBracket, uint8_t uKeySeparator) : m_uOpenBracket(uOpenBracket), m_uCloseBracket(uCloseBracket), m_uKeySeparator(uKeySeparator), m_stringQuoteChars("\"'`"), m_uOptions(eTrim | eAllowUnquoted) {}
   /// Construct with custom brackets
   code( std::string_view stringBracket, std::string_view stringQuoteChars = "\"'", unsigned uOptions = eTrim | eAllowUnquoted )
      : m_uOpenBracket( stringBracket.empty() ? 0 : static_cast<uint8_t>(stringBracket[0]) ),
        m_uCloseBracket( stringBracket.length() < 2 ? 0 : static_cast<uint8_t>(stringBracket[1]) ),
        m_uKeySeparator(stringBracket.length() < 3 ? 0 : static_cast<uint8_t>( stringBracket[2] )),
      m_stringQuoteChars(stringQuoteChars), m_uOptions(uOptions) { if( m_uOpenBracket != 0 && m_uCloseBracket == 0 ) { std::swap( m_uCloseBracket, m_uKeySeparator ); } }

   code(const code& o): m_uOptions(o.m_uOptions), m_uOpenBracket(o.m_uOpenBracket), m_uCloseBracket(o.m_uCloseBracket), m_uKeySeparator(o.m_uKeySeparator), m_stringQuoteChars(o.m_stringQuoteChars) {}

   bool is_trim() const { return ( m_uOptions & eTrim ) != 0; }

   bool is_quote(uint8_t uChar) const { return m_stringQuoteChars.find(uChar) != std::string::npos; }

   bool is_scope() const { return m_uOpenBracket != 0 && m_uCloseBracket != 0; }
   bool is_bracket() const { return is_scope(); }
   bool is_open_scope(uint8_t uChar) const { return m_uOpenBracket == uChar; }
   bool is_close_scope(uint8_t uChar) const { return m_uCloseBracket == uChar; }
   bool is_separator() const { return m_uKeySeparator != 0; }
   bool is_separator(uint8_t uChar) const { return m_uKeySeparator == uChar; }

   /// Skip quoted section, returning pointer past closing quote
   const uint8_t* skip_quoted(const uint8_t* puPosition, const uint8_t* puEnd, std::pair<const uint8_t*, size_t>* ppairValue = nullptr ) const;
   const char* skip_quoted(const char* piPosition, const char* piEnd) const { return (const char*)skip_quoted(reinterpret_cast<const uint8_t*>( piPosition ), reinterpret_cast<const uint8_t*>( piEnd )); }

   std::pair<const uint8_t*, size_t> read_value(const uint8_t* puPosition, const uint8_t* puEnd) const;
   std::pair<const char*, size_t> read_value(const char* piPosition, const char* piEnd) const { 
      auto pairValue = read_value(reinterpret_cast<const uint8_t*>(piPosition), reinterpret_cast<const uint8_t*>(piEnd)); 
      return { reinterpret_cast<const char*>(pairValue.first), pairValue.second }; 
   }
   std::string_view read_value(const std::string_view& stringText) const {
      auto pairValue = read_value(reinterpret_cast<const uint8_t*>(stringText.data()), reinterpret_cast<const uint8_t*>(stringText.data() + stringText.size()));
      return std::string_view(reinterpret_cast<const char*>(pairValue.first), pairValue.second);
   }
   std::string_view read_value(const char* piText, const char* piEnd, gd::types::tag_view ) const {
      auto pairValue = read_value(reinterpret_cast<const uint8_t*>(piText), reinterpret_cast<const uint8_t*>(piEnd));
      return std::string_view(reinterpret_cast<const char*>(pairValue.first), pairValue.second);
   }
   std::string_view read_value(const char* piText, size_t uLength) const {
      auto pairValue = read_value(reinterpret_cast<const uint8_t*>(piText), reinterpret_cast<const uint8_t*>(piText + uLength));
      return std::string_view(reinterpret_cast<const char*>(pairValue.first), pairValue.second);
   }

// ## attributes ---------------------------------------------------------------
   unsigned m_uOptions;           ///< flag options for parsing behavior
   uint8_t m_uOpenBracket;        ///< opening bracket character
   uint8_t m_uCloseBracket;       ///< closing bracket character
   uint8_t m_uKeySeparator;       ///< key-value separator character
   std::string m_stringQuoteChars;    ///< supported quote characters
};


const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const code& code, bool bScope );
inline const char* strstr( const char* pbszBegin, const char* pbszEnd, const std::string_view& stringFind, const code& code, bool bScope ) { return strstr( pbszBegin, pbszEnd, stringFind.data(), (unsigned)stringFind.length(), code, bScope ); }
inline const char* strstr(const std::string_view& stringText, const std::string_view& stringFind, const code& code, bool bScope) { return strstr(stringText.data(), stringText.data() + stringText.length(), stringFind.data(), (unsigned)stringFind.length(), code, bScope); }
inline const char* strstr(const char* pbszBegin, const char* pbszEnd, char iCharacter, const code& code, bool bScope) { 
   char piFind[2] = { iCharacter, '\0' };
   return strstr(pbszBegin, pbszEnd, piFind, 1, code, bScope);
}

const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const code& code, bool bScope, gd::types::tag_name );
inline const char* strstr( const char* pbszBegin, const char* pbszEnd, const std::string_view& stringFind, const code& code, bool bScope, gd::types::tag_name ) { return strstr( pbszBegin, pbszEnd, stringFind.data(), (unsigned)stringFind.length(), code, bScope, gd::types::tag_name{}); }
inline const char* strstr(const std::string_view& stringText, const std::string_view& stringFind, const code& code, bool bScope, gd::types::tag_name) { return strstr(stringText.data(), stringText.data() + stringText.length(), stringFind.data(), (unsigned)stringFind.length(), code, bScope, gd::types::tag_name{}); }
inline const char* strstr(const char* pbszBegin, const char* pbszEnd, char iCharacter, const code& code, bool bScope, gd::types::tag_name) { 
   char piFind[2] = { iCharacter, '\0' };
   return strstr(pbszBegin, pbszEnd, piFind, 1, code, bScope, gd::types::tag_name{});
}

std::pair<bool, const char*> read_value_g( const char* piBegin, const char* piEnd, const char* piFind, unsigned uLength, const code& code, bool bScope, size_t* puLength );


_GD_PARSE_END



