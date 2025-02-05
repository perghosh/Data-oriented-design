#pragma once

#include <cstdint>


extern const uint8_t pbToken_s[256];

// Define the token groups as constant bit flags.
enum class enumTokenGroup : uint8_t {
   eTokenGroupNumber   = 0x01, // digits '0'-'9'
   eTokenGroupOperator = 0x02, // arithmetic operators (e.g. +, -, *, /, %, =, ^)
   eTokenGroupSymbol   = 0x04, // punctuation and miscellaneous symbols
   eTokenGroupLetter   = 0x08, // letters A-Z, a-z (and underscore as part of identifiers)
   eTokenGroupSpace    = 0x10  // whitespace (space; you could add tab, newline, etc.)
};


/**
* @brief Token type definitions for the scripting language.
*
* Define the enumeration `enumToken`, which assigns unique
* constant values to the various token types used by the scripting language's
* lexical analyzer. Each token value represents a distinct category of input,
* facilitating the parsing process.
*/
enum class enumToken : uint32_t
{
   eTokenUnknown         = 0,  // Unknown or uninitialized token.
   eTokenNumber          = 1,  // Numeric literal (e.g., 123, 3.14).
   eTokenString          = 2,  // String literal (e.g., "hello world").
   eTokenIdentifier      = 3,  // Identifier (variable or function names).
   eTokenOperator        = 4,  // Operator (e.g., +, -, *, /, etc.).
   eTokenDelimiter       = 5,  // Delimiter (e.g., comma, semicolon).
   eTokenKeyword         = 6,  // Keyword (e.g., if, else, while).
   eTokenComment         = 7,  // Comment (inline or block comments).
   eTokenWhitespace      = 8,  // Whitespace (spaces, tabs, newlines).
   eTokenOpenParen       = 9,  // Open parenthesis '('.
   eTokenCloseParen      = 10, // Close parenthesis ')'.
   eTokenOpenBrace       = 11, // Open brace '{'.
   eTokenCloseBrace      = 12, // Close brace '}'.
   eTokenOpenBracket     = 13, // Open bracket '['.
   eTokenCloseBracket    = 14, // Close bracket ']'.
   eTokenEndOfFile       = 15  // End-of-file (EOF) marker.
};
