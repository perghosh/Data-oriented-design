#include <regex>

#include "gd_types.h"
#include "gd_utf8_2.h"

namespace gd {
   namespace ascii {
      /** ---------------------------------------------------------------------
       * @brief compare two ascii stings using wildcard
       * @param pbszText string to compare with
       * @param pbszWildcard string to compare with
       * @return 0 if string is equal, negative if first string is less than second string, positive if first string is greater than second string
       */
      int strcmp( const char* pbszText, const char* pbszWildcard, utf8::tag_wildcard )
      {
         unsigned uPosition = 0;
         while( pbszText[uPosition] != '\0' && pbszWildcard[uPosition] != '\0' )
         {
            if( (pbszText[uPosition] - pbszWildcard[uPosition] == 0) || pbszWildcard[uPosition] == '?' )
            {
               uPosition++;
               continue;
            }

            if( pbszWildcard[uPosition] == '*' ) return 0;

            break;
         }

         return (pbszText[uPosition] - pbszWildcard[uPosition]);
      }

      /*----------------------------------------------------------------------------- strcmp
       * @brief   Compare strings with support for wildcard characters
       * 
       * This function compares two strings where the pattern string can contain wildcard
       * characters:
       * - '?' matches any single character
       * - '*' matches zero or more characters of any type
       * 
       * The algorithm uses a backtracking approach to handle the '*' wildcard, which allows
       * it to correctly match complex patterns like "a*b*c" against various inputs.
       * 
       * Example usage:
       * @code
       *   // Simple file pattern matching
       *   const char* piFilename = "document.pdf";
       *   const char* piPattern = "*.pdf";
       *   bool bMatches = strcmp(piFilename, strlen(piFilename), 
       *                         piPattern, strlen(piPattern), 
       *                         utf8::tag_wildcard());
       *   // bMatches will be true
       *   
       *   // More complex pattern matching
       *   bool bMatches2 = strcmp("abcxyz", 6, "a?c*z", 5, utf8::tag_wildcard());
       *   // bMatches2 will be true
       *   
       *   // Pattern that doesn't match
       *   bool bMatches3 = strcmp("sample.txt", 10, "*.doc", 5, utf8::tag_wildcard());
       *   // bMatches3 will be false
       * @endcode
       * 
       * @param piText         Pointer to the text to be compared
       * @param uTextLength    Length of the text in characters
       * @param piPattern      Pointer to the pattern with possible wildcards
       * @param uPatternLength Length of the pattern in characters
       * @param tag_wildcard   Tag dispatch parameter to select wildcard comparison
       * 
       * @return bool          true if the text matches the pattern, false otherwise
       */
      bool strcmp(const char* piText, size_t uTextLength, const char* piPattern, size_t uPatternLength, utf8::tag_wildcard)
      {
         size_t uTextPosition = 0;
         size_t uPatternPosition = 0;

         // Variables for backtracking when '*' is found
         size_t uStarPatternPosition = std::string::npos;
         size_t uStarTextPosition = std::string::npos;

         while(uTextPosition < uTextLength)                                    // Compare characters until end of text
         {
            // ## If current characters match or pattern has '?'
            if(uPatternPosition < uPatternLength && (piText[uTextPosition] == piPattern[uPatternPosition] || piPattern[uPatternPosition] == '?'))
            {
               ++uTextPosition;
               ++uPatternPosition;
            }
            // ## If pattern has '*'
            else if(uPatternPosition < uPatternLength && piPattern[uPatternPosition] == '*')
            {
               // Save positions for backtracking
               uStarPatternPosition = uPatternPosition;
               uStarTextPosition = uTextPosition;

               // Skip consecutive '*' characters
               while(uPatternPosition < uPatternLength && piPattern[uPatternPosition] == '*')
               {
                  ++uPatternPosition;
               }
            }
            // No match but we have a '*' to backtrack to
            else if(uStarPatternPosition != std::string::npos)
            {
               // Try next position in text, but keep the pattern just after '*'
               uPatternPosition = uStarPatternPosition + 1;
               ++uStarTextPosition;
               uTextPosition = uStarTextPosition;
            }
            // No match and no '*' to backtrack to
            else
            {
               return false;
            }
         }

         // Skip any remaining '*' characters in pattern
         while(uPatternPosition < uPatternLength && piPattern[uPatternPosition] == '*')
         {
            ++uPatternPosition;
         }

         // If we've consumed the entire pattern, it's a match
         return (uPatternPosition == uPatternLength);      
      }


      /**
       * @brief Compares two std::string_view objects lexicographically, ignoring case.
       *
       * This function performs a case-insensitive comparison of two string views.
       * It returns an integer indicating their relative order:
       * - 0 if the strings are equal (ignoring case),
       * - a negative value if the first string is lexicographically less than the second,
       * - a positive value if the first string is lexicographically greater than the second.
       *
       * @param string1_ The first string view to compare.
       * @param string2_ The second string view to compare.
       * @return int 0 if equal, < 0 if string1_ < string2_, > 0 if string1_ > string2_ (case-insensitive).
       * @note This function uses std::tolower, which supports ASCII case conversion only.
       *       For Unicode case-insensitive comparison, consider a library like ICU.
       */
      int stricmp( std::string_view string1_, std::string_view string2_ )
      {
         size_t uLength1 = string1_.length();
         size_t uLength2 = string2_.length();
         size_t uShortestLength = (uLength1 < uLength2) ? uLength1 : uLength2;

         // ## Compare characters up to the shorter length
         for(size_t u = 0; u < uShortestLength; ++u) 
         {
            unsigned char uChar1 = static_cast<unsigned char>(string1_[u]);
            unsigned char uChar2 = static_cast<unsigned char>(string2_[u]);
            int iDifference = std::tolower(uChar1) - std::tolower(uChar2);
            if( iDifference != 0 ) { return iDifference; }
         }

         // If all characters match up to min_len, compare lengths
         if( uLength1 < uLength2 ) return -1;
         if( uLength1 > uLength2 ) return 1;
         return 0;
      }
   }
} // gd


_GD_UTF8_BEGIN

/**
 * @brief 
 * 0x01 = integer
 * 0x02 = decimal
*/
enum enumNumberType : uint8_t { eString = 0x00, eInteger = 0x01, eDecimal = 0x02, };
static const uint8_t pIsNumber_s[0x100] =
{
 //0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,0, /* 0x20-0x2F */
   1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0, /* 0x30-0x3F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xC0-0xCF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xD0-0xDF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xE0-0xEF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
};


static uint8_t get_number_type_s( const std::string_view& stringCheckType )
{
   uint8_t uType = 0;
   for( auto it = std::begin( stringCheckType ), itEnd = std::end( stringCheckType ); it != itEnd; it++ )
   {
      uint8_t uCharType = pIsNumber_s[*it];
      if( uCharType == 0 ) return 0;                                           // found 0 number that means that this has to be a string
      uType |= uCharType;
   }
   return uType;
}

/** -------------------------------------------------------------------
 * @brief Split string into multiple strings and store them in vector, string is split where char is found
 * @param stringText text to split into multiple parts
 * @param stringSplit character sequence that marks where to split text
 * @param vectorPart vector where strings are stored
*/
void split( const std::string_view& stringText, const std::string_view& stringSplit, std::vector<gd::variant>& vectorPart, gd::variant_type::enumType eDefaultType )
{
   if( stringSplit.empty() == true ) return;

   gd::variant_type::enumType eType = eDefaultType;
   std::string stringPart;                // Store string parts added to vector
   auto uLength = stringSplit.length();   // Split string length
   const uint8_t* pubszSplitWith = reinterpret_cast<const uint8_t*>( stringSplit.data() ); // help compiler to optimize ?

   const uint8_t* pubszPosition = reinterpret_cast<const uint8_t*>( stringText.data() ); // start of text
   const uint8_t* pubszTextEnd = reinterpret_cast<const uint8_t*>( stringText.data() + stringText.length() ); // end of text

   while( pubszPosition != pubszTextEnd )
   {                                                                                               assert( pubszPosition < pubszTextEnd ); assert( *pubszPosition != 0 );
      // ## Compare if split text sequence is found, then add value to vector based on found type
      if( *pubszPosition == *pubszSplitWith && memcmp( ( void* )pubszPosition, ( void* )pubszSplitWith, uLength ) == 0 )
      {
         uint8_t uType = get_number_type_s( stringPart );
         if( uType == eString ) vectorPart.emplace_back( gd::variant( stringPart, eDefaultType ) );
         else
         {
            gd::variant value_;
            if( eDecimal & uType ) 
            {
               double dValue = std::stold( stringPart );
               value_ = dValue;
            }
            else
            {
               int64_t iValue = std::atoll( stringPart.c_str() );
               value_ = iValue;
            }

            vectorPart.emplace_back( value_ );
         }

         stringPart.clear();
         pubszPosition += uLength;                                             // Move past split sequence
         eType = eDefaultType;                                                 // Set to default type for new value
         continue;
      }

      stringPart += (char)*pubszPosition;
      pubszPosition++;                                                         // next character
   }

   // add final part
   if( stringPart.empty() == false )
   {
      uint8_t uType = get_number_type_s( stringPart );
      if( uType == eString ) vectorPart.emplace_back( gd::variant( stringPart, eDefaultType ) );
      else
      {
         if( eDecimal & uType ) vectorPart.emplace_back( gd::variant( (double)std::stold( stringPart ) ) );
         else                   vectorPart.emplace_back( gd::variant( (int64_t)std::atoll( stringPart.c_str() ) ) );
      }
   }
}

std::vector<std::string> split(const std::string_view& stringText, const std::string_view& stringSplit)
{
   std::vector<std::string> vectorString;
   std::vector<gd::variant> vectorPart;

   split( stringText, stringSplit, vectorPart );

   for(auto it : vectorPart)
   {
      vectorString.push_back( it.as_string() );
   }


   return vectorString;
}

/** ---------------------------------------------------------------------------
 * @brief Split string into parts divided by sent character
 * @param stringText text that is split into parts and placed in vectorPart
 * @param chSplit character used to split text
 * @param vectorPart 
*/
/*
void split( const std::string_view& stringText, char chSplit, std::vector<std::string_view>& vectorPart )
{
   const char* pbszBegin = stringText.data();
   const char* pbszEnd = pbszBegin + stringText.length();
   const char* pbszPosition = pbszBegin;
   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chSplit ) pbszPosition++;
      else
      {
         vectorPart.push_back( std::string_view( pbszBegin, pbszPosition ) );
         pbszPosition++;
         pbszBegin = pbszPosition;
      }
   }

   if( *pbszPosition != chSplit ) vectorPart.push_back( std::string_view( pbszBegin, pbszPosition ) );
}
*/


_GD_UTF8_END


_GD_UTF8_BEGIN

namespace regex {
   int64_t find( const std::string_view& stringText, const std::string_view& stringFind )
   {
      std::string text_( stringText );
      std::regex regexMatch( stringFind.data() );
      std::smatch smatch_;

      if( std::regex_search(text_, smatch_, regexMatch) == true )
      {
         auto iPosition = smatch_.position();
         return iPosition;
      }

      return -1;
   }


   void replace(std::string& stringText, const std::string_view& stringMatch, const std::string_view& stringInsert)
   {
      //std::string stringNew;
      std::regex regexMatch( stringMatch.data() );

      stringText = std::regex_replace(stringText, regexMatch, stringInsert.data() );
   }

}
_GD_UTF8_END