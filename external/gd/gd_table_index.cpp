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

/** --------------------------------------------------------------------------
 * @brief compact the index by removing duplicate int64 values (only first value with lowest row is kept)
 *
 * Note that this function assumes that the index is already sorted other wise the result will be undefined
 */
void index_int64::compact()
{
   if(m_vectorIndex.empty()) return;

   auto it = m_vectorIndex.begin(); // start writing from the first element
   auto itTail = std::next(m_vectorIndex.begin()); // start reading from the second element

   for(; itTail != m_vectorIndex.end(); ++itTail)
   {
      if(itTail->first != it->first)
      {  // ## We have a unique int64 value, we are keeping the lowest row value
         ++it;                                                                // move to the next unique element
         *it = *itTail;                                                       // copy the unique element to the next position
      }
      else
      {  // ## We have a duplicate int64 value, we are keeping the lowest row value
         if(itTail->second < it->second)
         {
            it->second = itTail->second;                                      // update the row value to the lowest one
         }
      }
   }

   m_vectorIndex.erase(std::next(it), m_vectorIndex.end());                   // erase the duplicates after the last unique element
   m_vectorIndex.shrink_to_fit();
}


// ----------------------------------------------------------------------------
// --------------------------------------------------------------- index_string
// ----------------------------------------------------------------------------

void index_string::add( const gd::variant_view& variantviewValue, uint64_t uRow )
{
   if( variantviewValue.is_string() )
   {
      std::string_view stringValue = variantviewValue.get_string_view();
      m_vectorIndex.push_back( std::make_pair(stringValue, uRow ) );
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

/** --------------------------------------------------------------------------
 * @brief compact the index by removing duplicate string values (only first value with lowest row is kept)
 * 
 * Note that this function assumes that the index is already sorted other wise the result will be undefined
 */
void index_string::compact()
{
  if(m_vectorIndex.empty()) return;

   auto it = m_vectorIndex.begin(); // start writing from the first element
   auto itTail = std::next(m_vectorIndex.begin()); // start reading from the second element

   for(; itTail != m_vectorIndex.end(); ++itTail)
   {
      if(itTail->first != it->first)
      {  // ## We have a unique int64 value, we are keeping the lowest row value
         ++it;                                                                // move to the next unique element
         *it = *itTail;                                                       // copy the unique element to the next position
      }
      else
      {  // ## We have a duplicate int64 value, we are keeping the lowest row value
         if(itTail->second < it->second)
         {
            it->second = itTail->second;                                      // update the row value to the lowest one
         }
      }
   }

   m_vectorIndex.erase(std::next(it), m_vectorIndex.end());                   // erase the duplicates after the last unique element
   m_vectorIndex.shrink_to_fit();
}



_GD_TABLE_END
