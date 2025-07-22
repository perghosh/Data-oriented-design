/**
 * @file gd_parse_formats.cpp
 */

#include "gd/gd_parse.h"

#include "gd_parse_formats.h"

_GD_PARSE_BEGIN

const uint8_t* code::skip_quoted(const uint8_t* puPosition, const uint8_t* puEnd, std::pair<const uint8_t*, size_t>* ppairValue) const 
{                                                                                                  assert( is_quote(*puPosition) == true );
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
         if( uQuoteCount == 1 )
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
                  size_t uLength = puPosition - puValueStart - uQuoteCount;    // Calculate length of value
                  ppairValue->second = uLength; 
               }

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

std::pair<const uint8_t*, size_t> code::read_value(const uint8_t* puPosition, const uint8_t* puEnd) const
{                                                                                                  assert( puPosition <= puEnd );
   std::pair<const uint8_t*, size_t> pairValue = { nullptr, 0 }; // Initialize value pair

   // ## move to value
   puPosition = (const uint8_t*)next_non_space_g(reinterpret_cast<const char*>(puPosition), reinterpret_cast<const char*>(puEnd) );

   if( is_separator() == true )
   {
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
         while( puPosition < puEnd && *puPosition != '\0' && isspace( *puPosition ) == 0 ) puPosition++;  // Find end of value
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


std::pair<bool, const char*> read_value_g(const char* piBegin, const char* piEnd, const char* piFind, unsigned uLength, const code& code, bool bScope, size_t* puLength)
{                                                                                                  assert( piBegin <= piEnd );
   // ## Find value
   const char* piPosition = strstr(piBegin, piEnd, piFind, uLength, code, bScope); // Find key-value separator
   if( piPosition == nullptr ) return { false, piEnd }; // Not found

   // ## Go to value and return information about value length and position

                              
   return { true, piPosition }; // Return position after value
}




_GD_PARSE_END
