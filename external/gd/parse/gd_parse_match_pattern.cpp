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

/// Finds first pattern in list of internal patterns and if any is found return index to that pattern
int patterns::find_pattern(const char* piText, size_t uLength, uint64_t* puOffset ) const
{
   decltype( piText ) piTextEnd = piText + uLength;
   for( const auto* p_ = piText; p_ != piTextEnd; p_++ )
   {
      if( m_arrayMarkerHint[static_cast<uint8_t>( *p_ )] == 0 ) continue;     // no match found
      int iIndex = find_( p_, piTextEnd );
      if( puOffset != nullptr ) *puOffset = p_ - piText;                      // set offset to start of pattern
      if( iIndex != -1 ) return iIndex;                                       // pattern found
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


_GD_PARSE_END