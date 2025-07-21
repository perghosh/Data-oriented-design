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

   bool is_quote(uint8_t uChar) const { return m_stringQuoteChars.find(uChar) != std::string::npos; }

   bool is_open_scope(uint8_t uChar) const { return m_uOpenBracket == uChar; }
   bool is_close_scope(uint8_t uChar) const { return m_uCloseBracket == uChar; }

   /// Skip quoted section, returning pointer past closing quote
   const uint8_t* skip_quoted(const uint8_t* puPosition, const uint8_t* puEnd) const { assert( is_quote(*puPosition) == true );
      unsigned uQuoteCount = 1;
      const uint8_t uQuote = *puPosition;
      ++puPosition; // Skip opening quote

      while( *puPosition == uQuote && puPosition < puEnd ) { puPosition++, uQuoteCount++; }

      if( uQuoteCount % 2 == 0 ) return (puPosition + 1);                      // Even number of quotes, return position after closing quote

      while(puPosition < puEnd) 
      {
         if( *puPosition == uQuote ) 
         {
            if( uQuoteCount == 1 )
            {
               // Check for escaped quote (double quote)
               if( puPosition + 1 < puEnd && *(puPosition + 1) == uQuote)
               {
                  puPosition += 2; // Skip both quotes
               } 
               else 
               {
                  return puPosition + 1; // Return pointer past closing quote
               }
            }
            else
            {
               // ## Match number of quotes text start with
               auto uCount = uQuoteCount;
               while( puPosition < puEnd && uCount > 0  && *puPosition == uQuote )
               {
                  ++puPosition;
                  --uCount;
               }

               if( uCount == 0 ) return puPosition;                           // Return position after closing quote
            }
         } 
         else 
         {
            ++puPosition;
         }
      }

      return puPosition; // Unterminated quote
   }

   const char* skip_quoted(const char* piPosition, const char* piEnd) const { return (const char*)skip_quoted(reinterpret_cast<const uint8_t*>( piPosition ), reinterpret_cast<const uint8_t*>( piEnd )); }



// ## attributes ---------------------------------------------------------------
   unsigned m_uOptions;           ///< flag options for parsing behavior
   uint8_t m_uOpenBracket;        ///< opening bracket character
   uint8_t m_uCloseBracket;       ///< closing bracket character
   uint8_t m_uKeySeparator;       ///< key-value separator character
   std::string m_stringQuoteChars;    ///< supported quote characters
};

/** ---------------------------------------------------------------------------
 * @brief Find a substring in a text block with optional scope handling
 * @param pbszBegin start of text to search within
 * @param pbszEnd end of text
 * @param pbszFind substring to find
 * @param uLength length of the substring to find
 * @param code parsing rules for handling scopes and quotes
 * @param bScope if true, considers scope characters for searching
 * @return pointer to found substring or nullptr if not found
 */
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const code& code, bool bScope )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   //if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;


   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find
   unsigned uScope = 1;                    // scope of search, how many characters to compare after first character has been found

   if( bScope == true ) uScope = 0;                                            // if scope is true than we need to find scope character, otherwise we just compare first character
   
   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( bScope == true )
      {
         if( code.is_open_scope(*pbszPosition) == true ) uScope++;
         else if( code.is_close_scope(*pbszPosition) == true )
         {
            if( uScope > 0 ) uScope--;
         }
      }

      if( uScope == 0 )
      {
         pbszPosition++;
         continue;
      }
      
      if( *pbszPosition != chFind )
      {
         if( code.is_quote( *pbszPosition ) == false )
         {
            pbszPosition++;
            continue;
         }
         else
         {
            pbszPosition = code.skip_quoted(pbszPosition, pbszEnd );
         }
      }
      else
      {
         if( uLength == 1 || memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 )
         {
            return pbszPosition;                                              // found text? return pointer to text
         }

         pbszPosition++;
      }
   }

   return nullptr;
}




TEST_CASE("[table] custom columns", "[strstr]") 
{
   std::string stringText = "This is a test string with key3 some patterns to find: [key1: `value1 key3`] [key2: \"value2 key3\"] [key3: value3]";
   const char* begin = stringText.c_str();
   const char* piEnd = begin + stringText.size();
   const char* find = "key3";
   unsigned uLength = (unsigned)strlen(find);
   
   const code codeParser('[', ']');
   const char* result = strstr(begin, piEnd, find, uLength, codeParser, true);
   
   REQUIRE(result != nullptr);
   REQUIRE(std::string(result, uLength) == find);
}