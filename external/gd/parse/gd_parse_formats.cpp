/**
 * @file gd_parse_formats.cpp
 */

#include "gd/gd_parse.h"

#include "gd_parse_formats.h"

_GD_PARSE_BEGIN




/** ---------------------------------------------------------------------------
 * @brief Skips a quoted section in the input, handling escaped quotes and multi-quote delimiters.
 * 
 * Advances the pointer past a quoted value, supporting both single and multiple quote characters
 * (e.g., "", '', """ 0 1 2 """,  ``). Handles escaped quotes (e.g., "" inside "") and sets the value range if requested.
 * Returns a pointer to the position after the closing quote(s), or to puEnd if unterminated.
 * 
 * @param puPosition Pointer to the current position (must be at a quote character).
 * @param puEnd Pointer to the end of the input.
 * @param ppairValue Optional output pair to receive the start and length of the quoted value.
 * @return Pointer to the position after the closing quote(s), or puEnd if unterminated.
 * 
 * @code
 * // Example usage:
 * const code codeParser();
 * const uint8_t* input = ...;
 * const uint8_t* end = ...;
 * std::pair<const uint8_t*, size_t> value;
 * const uint8_t* result = codeParser.skip_quoted(input, end, &value);
 * 
 * // Handle the result...
 * @endcode
 */
const uint8_t* code::skip_quoted(const uint8_t* puPosition, const uint8_t* puEnd, std::pair<const uint8_t*, size_t>* ppairValue) const 
{                                                                                                  assert( is_quote(*puPosition) == true );
   auto trim_ = [](auto* ppairValue) -> void {
      const uint8_t* puStart = ppairValue->first;
      const uint8_t* puEnd = ppairValue->first + ppairValue->second;

      while(puStart < puEnd && isspace(*puStart)) ++puStart;
      while( puEnd > puStart && isspace(*(puEnd - 1))) --puEnd;

      ppairValue->first = puStart;
      ppairValue->second = puEnd - puStart;
   };

   unsigned uQuoteCount = 1;
   const uint8_t uQuote = *puPosition;
   ++puPosition; // Skip opening quote

   while( *puPosition == uQuote && puPosition < puEnd ) { puPosition++, uQuoteCount++; }

   if( uQuoteCount % 2 == 0 ) 
   {
      if(ppairValue != nullptr) 
      { 
         ppairValue->first = puPosition - (uQuoteCount >> 1);
         ppairValue->second = 0;
      }

      return puPosition;                                                      // Return position after closing quote
   }

   if( ppairValue != nullptr ) { ppairValue->first = puPosition; }            // Set value start position

   while(puPosition < puEnd) 
   {
      if( *puPosition == uQuote ) 
      {
         if( uQuoteCount == 1 )                                               // check for single quote, this may be escaped with double quote
         {
            // Check for escaped quote (double quote)
            if( puPosition + 1 < puEnd && *(puPosition + 1) == uQuote)
            {
               puPosition += 2; // Skip both quotes
            } 
            else 
            {
               if( ppairValue != nullptr ) 
               {
                  const auto* puValueStart = ppairValue->first;
                  size_t uLength = puPosition - puValueStart;    // Calculate length of value
                  ppairValue->second = uLength; 
                  if( is_trim() == true ) { trim_( ppairValue ); }            // Trim whitespace if enabled
               }

               return puPosition + 1; // Return pointer past closing quote
            }
         }
         else                                                                 // tripple or more quotes
         {
            // ## Match number of quotes text start with
            auto uCount = uQuoteCount;
            while( puPosition < puEnd && uCount > 0  && *puPosition == uQuote )
            {
               ++puPosition;
               --uCount;
            }

            if( ppairValue != nullptr ) 
            {
               const auto* puValueStart = ppairValue->first;
               size_t uLength = puPosition - puValueStart - uQuoteCount;      // Calculate length of value
               ppairValue->second = uLength; 
            }
            if( uCount == 0 ) return puPosition;                              // Return position after closing quote
         }
      } 
      else 
      {
         ++puPosition;
      }
   }

   if( ppairValue != nullptr ) { *ppairValue = { nullptr, 0 }; }              // No valid value found

   return puPosition; // Unterminated quote
}

/** ---------------------------------------------------------------------------
 * @brief Reads a value from the input, handling quoted and unquoted values.
 * 
 * This function reads a value from the input, skipping whitespace and handling
 * quoted values. If the value is quoted, it skips to the closing quote and returns
 * the value range. If unquoted, it reads until the end of the value or until a
 * specified closing character (if applicable).
 * 
 * Sample usage: it tries to move to value and return pair with value start and length.
 * verbatim
   [key=value]
    ^...^
   [key: value]
    ^....^
 * endverbatim
 * 
 * When separator is set, it expects a key-value pair format and skips until the separator.
 * No separator means it reads until non space value is found or newline that means no value.
 * 
 * @param puPosition Pointer to the current position in the input.
 * @param puEnd Pointer to the end of the input.
 * @return A pair containing a pointer to the start of the value and its length.
 */
std::pair<const uint8_t*, size_t> code::read_value(const uint8_t* puPosition, const uint8_t* puEnd) const
{                                                                                                  assert( puPosition <= puEnd );
   std::pair<const uint8_t*, size_t> pairValue = { nullptr, 0 }; // Initialize value pair

   // ## move to value
   puPosition = (const uint8_t*)next_non_space_g(reinterpret_cast<const char*>(puPosition), reinterpret_cast<const char*>(puEnd) );

   if( is_separator() == true )                                                // is separator is used to split between key and value
   {
      // ## Skip key and move to separator
      while( puPosition < puEnd && is_separator(*puPosition) == false ) { puPosition++; }

      if(is_separator(*puPosition) == true )
      {
         ++puPosition;
         puPosition = (const uint8_t*)next_non_space_g(reinterpret_cast<const char*>(puPosition), reinterpret_cast<const char*>(puEnd));
      }
      else
      {
         // ## No separator found, return empty value
         return { nullptr, 0 }; // Skip empty key-value pairs
      }
   }
   else
   {
      // ## Skip whitespace until separator
      while( puPosition < puEnd && isspace(*puPosition) == 0 ) { ++puPosition; }

      // Special case: if we find a newline or end of input, we assume _no_ value is present
      while( puPosition < puEnd && isspace(*puPosition) != 0 )
      {
         if( *puPosition == '\n' || *puPosition == '\0' )
         {
            return { nullptr, 0 }; // Skip empty key-value pairs
         }
         puPosition++;
      }
   }

   if( is_quote(*puPosition) )
   {
      puPosition = skip_quoted(puPosition, puEnd, &pairValue );
   }
   else
   {
      // ## Unquoted value
      const uint8_t* puValueStart = puPosition;
      if( m_uCloseBracket != 0 )
      {
         while( puPosition < puEnd && *puPosition != m_uCloseBracket ) puPosition++;
      }
      else
      {
         // End value with newline or double space
         int iSpacePrevious = 0; // Previous character space state
         while( puPosition < puEnd && *puPosition != '\n' )
         {
            auto iSpace = isspace(*puPosition);
            if( iSpace != 0 && iSpacePrevious != 0 ) { break; }               // Stop reading value on double space
            iSpacePrevious = iSpace;                                          // Update previous space state
            puPosition++; // Find end of value
         }
      }

      pairValue.first = puValueStart;
      pairValue.second = puPosition - puValueStart;                           // Calculate length of value

   }

   return pairValue; 
}



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


/** ---------------------------------------------------------------------------
 * @brief Reads a value from the input, handling quoted and unquoted values.
 * 
 * This function reads a value from the input, skipping whitespace and handling
 * quoted values. If the value is quoted, it skips to the closing quote and returns
 * the value range. If unquoted, it reads until the end of the value or until a
 * specified closing character (if applicable).
 * 
 * @param piBegin Pointer to the current position in the input.
 * @param piEnd Pointer to the end of the input.
 * @param piFind Pointer to the substring to find.
 * @param uLength Length of the substring to find.
 * @param code Parsing rules for handling scopes and quotes.
 * @param bScope If true, considers scope characters for searching.
 * @param puLength Optional output for length of found value.
 * @return A pair containing a boolean indicating success and a pointer to the position after the value.
 */
std::pair<bool, const char*> read_value_g(const char* piBegin, const char* piEnd, const char* piFind, unsigned uLength, const code& code, bool bScope, size_t* puLength)
{                                                                                                  assert( piBegin <= piEnd );
   // ## Find value
   const char* piPosition = strstr(piBegin, piEnd, piFind, uLength, code, bScope); // Find key-value separator
   if( piPosition == nullptr ) return { false, piEnd }; // Not found

   // ## Go to value and return information about value length and position

                              
   return { true, piPosition }; // Return position after value
}




_GD_PARSE_END
