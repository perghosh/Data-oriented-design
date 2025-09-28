#include <algorithm>

#include "gd_parse_match_pattern.h"

_GD_PARSE_BEGIN

/// @brief Construct patterns with a vector of strings
patterns::patterns(const std::vector<std::string>& vectorPattern)
{
   m_arrayMarkerHint = { 0 };
   m_vectorPattern.reserve(vectorPattern.size());
   for( const auto& it : vectorPattern )
   {
      add( std::string_view( it ) );
   }
}

/// sort vector with patterns based on length and start with the longest pattern
void patterns::sort()
{
   std::sort(m_vectorPattern.begin(), m_vectorPattern.end(), [](const pattern& a_, const pattern& b_) {
      return a_.get_pattern().length() > b_.get_pattern().length();
   });
}

/// prepare marker hint array based on patterns in vector
void patterns::prepare()
{
   m_arrayMarkerHint = { 0 };
   for( const auto& it : m_vectorPattern )
   {
      if( it.is_ignore_case() == true )
      {
         // add both upper and lower case characters to marker hint
         char iChar = it.get_pattern()[0];
         auto bIsAlpha = std::isalpha(static_cast<unsigned char>( iChar )) != 0;
         if( bIsAlpha == false ) 
         {
            add_marker_hint(it.get_pattern()[0]);
            continue; // not an alpha character, just add it once
         }

         auto iMarker = static_cast<char>( std::tolower(iChar) );
         add_marker_hint( iMarker );
         iMarker = static_cast<char>( std::toupper(iChar) );
         add_marker_hint( iMarker );
      }
      else
      {
         add_marker_hint(it.get_pattern()[0]);
      }
   }
}

/// Finds first pattern in list of internal patterns and if any is found return index to that pattern
int patterns::find_pattern(const char* piText, size_t uLength, uint64_t* puOffset ) const
{
   decltype( piText ) piTextEnd = piText + uLength;
   for( const auto* p_ = piText; p_ != piTextEnd; p_++ )
   {
      if( m_arrayMarkerHint[static_cast<uint8_t>( *p_ )] == 0 ) continue;     // no match found
      int iIndex;
      iIndex = find_( p_, piText, piTextEnd );
      if( iIndex != -1 )                                                      // pattern found
      {
         if( puOffset != nullptr ) *puOffset = p_ - piText;                   // set offset to start of pattern
         return iIndex;                                       
      }
   }

   return -1;
}

/// Finds first pattern in list of internal patterns and if any is found return index to that pattern
int patterns::find_pattern(const char* piText, size_t uLength, uint64_t uOffset, uint64_t* puOffset ) const
{                                                                                                  assert(uLength >= uOffset);
   const char* piPosition = piText + uOffset;
   decltype( piPosition ) piTextEnd = piText + uLength;
   for( const auto* p_ = piPosition; p_ != piTextEnd; p_++ )
   {
      if( m_arrayMarkerHint[static_cast<uint8_t>( *p_ )] == 0 ) continue;     // no match found
      int iIndex = find_( (const uint8_t*)p_, (const uint8_t*)piText, (const uint8_t*)piTextEnd );
      if( iIndex != -1 )                                                      // pattern found
      {
         if( puOffset != nullptr ) *puOffset = p_ - piText;                   // set offset to start of pattern
         return iIndex;                                       
      }
   }

   return -1;
}


/// Finds first pattern in list of internal patterns and if any is found return index to that pattern
int patterns::find_(const uint8_t* puBegin, const uint8_t* puEnd ) const
{
   int iIndex = -1;
   size_t uLength = puEnd - puBegin;
   for( const auto& it : m_vectorPattern )
   {
      iIndex++;
      const auto& stringPattern = it.get_pattern();
      if( stringPattern.length() > uLength ) continue;                         // pattern is longer than text

      if( std::memcmp(puBegin, stringPattern.data(), stringPattern.length()) == 0 )
      {
         return iIndex;                                                        // pattern found
      }

   }

   return -1;
}

/// Finds first pattern in list of internal patterns and if any is found return index to that pattern
/// This function is optimized to use the pattern's compare method for matching, which considers case sensitivity and word boundaries.
int patterns::find_(const uint8_t* puPosition, const uint8_t* puBegin, const uint8_t* puEnd ) const
{
   int iIndex = -1;
   size_t uLength = puEnd - puPosition;
   for( const auto& it : m_vectorPattern )
   {
      iIndex++;
      const auto& stringPattern = it.get_pattern();
      if( stringPattern.length() > uLength ) continue;                         // pattern is longer than text
      if( it.compare( (const char*)puPosition, (const char*)puBegin ) == true )
      {
         return iIndex;                                                        // pattern found
      }
   }
   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief Compares a substring of text to the stored pattern, considering case sensitivity and word boundaries as specified by the pattern's flags.
 * @param piText Pointer to the start of the substring in the text to compare against the pattern.
 * @param piTextStart Pointer to the start of the full text buffer, used for word boundary checks. Can be nullptr if not needed.
 * @return Returns true if the substring matches the pattern according to the current flags and word boundary requirements; otherwise, returns false.
 */
bool patterns::pattern::compare(const char* piText, const char* piTextStart) const 
{	                                                                                                assert(m_stringPattern.empty() == false && "pattern string is empty");
   const size_t uPatternLength = m_stringPattern.length();

   // ## Fast path: exact case-sensitive comparison when no flags are set ....
   if(m_uFlags == 0) { return std::strncmp(piText, m_stringPattern.c_str(), uPatternLength) == 0; }

   // ## Check basic string match (case-sensitive or case-insensitive) .......
   bool bMatches;
   if(is_ignore_case() == true) 
   {
      // Case-insensitive comparison - use platform-specific function
#ifdef _WIN32
      bMatches = _strnicmp(piText, m_stringPattern.c_str(), uPatternLength) == 0;
#else
      bMatches = strncasecmp(piText, m_stringPattern.c_str(), uPatternLength) == 0;
#endif
   }
   else 
   {
      // Case-sensitive comparison
      bMatches = std::strncmp(piText, m_stringPattern.c_str(), uPatternLength) == 0;
   }

   if(bMatches == false) return false;


	// ## Check word boundaries if required ...................................
   if(is_word() == true)                                                      // If word matching is required, check word boundaries 
   {
      if(piTextStart != nullptr && piText > piTextStart)                      // Check character before the match (should be non-alphanumeric or start of string)
      {
         if(std::isalnum(static_cast<unsigned char>(*(piText - 1))) || *(piText - 1) == '_') 
         {
            return false;
         }
      }

      // Check character after the match (should be non-alphanumeric or end of string)
      char iCharAfter = piText[uPatternLength];
      if(iCharAfter != '\0' && (std::isalnum(static_cast<unsigned char>(iCharAfter)) || iCharAfter == '_'))  { return false; }
   }

   return true;
}


_GD_PARSE_END