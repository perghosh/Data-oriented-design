// $TAG #play #ini #xml #history

#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

struct code
{
   enum enumOptions
   {
      eTrimWhitespace = 1 << 0,     ///< Trim whitespace from keys and values
      eAllowUnquoted = 1 << 1,      ///< Allow unquoted values
      eStrictQuoting = 1 << 2,      ///< Require matching quote types
      eSkipEmpty = 1 << 3,          ///< Skip empty key-value pairs
      eOptionsMAX = 1 << 4,
   };

   code(uint8_t uOpenBracket, uint8_t uCloseBracket) : m_uOpenBracket(uOpenBracket), m_uCloseBracket(uCloseBracket), m_uKeySeparator(':'), m_stringQuoteChars("\"'`"), m_uOptions(eTrimWhitespace | eAllowUnquoted) {}
   code(uint8_t uOpenBracket, uint8_t uCloseBracket, uint8_t uKeySeparator) : m_uOpenBracket(uOpenBracket), m_uCloseBracket(uCloseBracket), m_uKeySeparator(uKeySeparator), m_stringQuoteChars("\"'`"), m_uOptions(eTrimWhitespace | eAllowUnquoted) {}

   bool is_quote(uint8_t uChar) const { return m_uQuotes.find(uChar) != std::string::npos; }

   /// Skip quoted section, returning pointer past closing quote
   const uint8_t* skip_quoted(const uint8_t* puPosition, const uint8_t* puEnd) const { assert( is_quote(*puPosition) == true );
      unsigned uQuoteCount = 1;
      const uint8_t uQuote = *puPosition;
      ++puPosition; // Skip opening quote

      while( *puPosition == uQuote && puPosition < puEnd ) puPosition++;

      uQuoteCount



      while (puPosition < puEnd) {
         if (*puPosition == uQuote) {
            // Check for escaped quote (double quote)
            if (puPosition + 1 < puEnd && *(puPosition + 1) == uQuote) {
               puPosition += 2; // Skip both quotes
            } else {
               return puPosition + 1; // Return pointer past closing quote
            }
         } else {
            ++puPosition;
         }
      }

      return puPosition; // Unterminated quote
   }



// ## attributes ---------------------------------------------------------------
   unsigned m_uOptions;           ///< flag options for parsing behavior
   uint8_t m_uOpenBracket;        ///< opening bracket character
   uint8_t m_uCloseBracket;       ///< closing bracket character
   uint8_t m_uKeySeparator;       ///< key-value separator character
   std::string m_stringQuoteChars;    ///< supported quote characters
};

/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of the byte string needle in the byte string pointed to by haystack.
 * @param pbszBegin start of text (haystack) to find string (needle) in 
 * @param pbszEnd end of text
 * @param pbszFind pointer to string that is searched for within text
 * @param uLength length of string
 * @param csv parse rules, adapted to sql query format
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to string in text if found, nullptr if not found
*/
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const code& code )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   //if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find
   
   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         //if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
         if( code.is )
         {
            pbszPosition++;
            continue;
         }
         else
         {
            if( csv.is_quote( *pbszPosition ) == true )
            {
               // ## found quote, text within quote is skipped
               pbszPosition = skip_quoted_g( pbszPosition );
            }
            else
            {
               pbszPosition++;
               continue;
            }
         }
      }
      else
      {
         if( uLength == 1 || memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 ) return pbszPosition; // found text? return pointer to text

         pbszPosition++;
      }
   }

   return nullptr;
}




TEST_CASE("[table] custom columns", "[strstr]") 
{
}