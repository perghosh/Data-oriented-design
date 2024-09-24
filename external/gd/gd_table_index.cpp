#include "gd_table_index.h"
#include <algorithm>

#include "gd_table_index.h"
#include "gd_variant.h"
#include "gd_table_index.h"

_GD_TABLE_BEGIN

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- index_int64
// ----------------------------------------------------------------------------


void index_int64::add( const gd::variant_view& variantviewValue, uint64_t uRow )
{
   if( variantviewValue.is_64() )
   {
      m_vectorIndex.push_back( std::make_pair( variantviewValue.cast_as_int64(), uRow ) );
   }
   else
   {
      auto v_ = variantviewValue.convert_to( type_s() );
      m_vectorIndex.push_back( std::make_pair( (int64_t)v_, uRow ) );
   }
}

void index_int64::sort()
{
   std::sort( std::begin( m_vectorIndex ), std::end( m_vectorIndex ), []( const auto& v1, const auto& v2 ) {
      return v1.first < v2.first;
   });
}

std::pair<bool, uint64_t> index_int64::find( int64_t iFindValue ) const noexcept
{
   auto itEnd = std::end( m_vectorIndex );
   auto itFind = std::lower_bound( std::begin( m_vectorIndex ), itEnd, iFindValue, []( const auto& v1, int64_t iFind ) {
      return v1.first < iFind;
   });

   if( itFind != itEnd ) return { true, itFind->second };                      // found value? then return index to value

   return { false, ( uint64_t )-1 };                                           // value not found, return invalid position
}

// ----------------------------------------------------------------------------
// --------------------------------------------------------------- index_string
// ----------------------------------------------------------------------------

void index_string::add( const gd::variant_view& variantviewValue, uint64_t uRow )
{
   if( variantviewValue.is_string() )
   {
      m_vectorIndex.push_back( std::make_pair( variantviewValue.get_string_view(), uRow ) );
   }
   else
   {                                                                                               assert( false );
                        
   }
}

void index_string::sort()
{
   std::sort( std::begin( m_vectorIndex ), std::end( m_vectorIndex ), []( const auto& v1, const auto& v2 ) {
      return v1.first < v2.first;
   });
}

std::pair<bool, uint64_t> index_string::find( const std::string_view& stringFindValue ) const noexcept
{
   auto itEnd = std::end( m_vectorIndex );
   auto itFind = std::lower_bound( std::begin( m_vectorIndex ), itEnd, stringFindValue, []( const auto& v1, const std::string_view& stringFindValue ) {
      return v1.first < stringFindValue;
   });

   if( itFind != itEnd ) return { true, itFind->second };                      // found value? then return index to value

   return { false, ( uint64_t )-1 };                                           // value not found, return invalid position
}



_GD_TABLE_END