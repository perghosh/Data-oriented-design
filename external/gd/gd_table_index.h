// @FILE [tag: table, index] [description: Indexing table data for fast searches and lookup. Indexes may be used as helpers in table objects] [type: header] [name: gd_table_index.h]

#pragma once

#include <vector>

#include "gd_types.h"
#include "gd_variant_view.h"
#include "gd_table.h"

_GD_TABLE_BEGIN

class index_base
{
};

/** ===========================================================================
 * \brief index to manage int64 values (works with integer values that is able to convert to int64)
 *
 *
 *
 \code
 \endcode
 */
class index_int64 : public index_base
{
// ## construction -------------------------------------------------------------
public:
   index_int64() {}
   index_int64( size_t uCount ) { m_vectorIndex.reserve( uCount ); }
   // copy
   index_int64( const index_int64& o ) { common_construct( o ); }
   index_int64( index_int64&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   index_int64& operator=( const index_int64& o ) { common_construct( o ); return *this; }
   index_int64& operator=( index_int64&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~index_int64() {}
private:
   // common copy
   void common_construct( const index_int64& o ) {
      m_iValue = o.m_iValue; m_vectorIndex = o.m_vectorIndex;
   }
   void common_construct( index_int64&& o ) noexcept {
      m_iValue = o.m_iValue; m_vectorIndex = std::move( o.m_vectorIndex );
   }

// ## operator -----------------------------------------------------------------
public:
   operator uint64_t() const { return find( m_iValue ).second; }
   index_int64& operator()( int64_t iValue ) { m_iValue = iValue; return *this; }

// ## methods ------------------------------------------------------------------
public:
/** \name OPERATION
*///@{
   /// add value to be indexed
   void add( const gd::variant_view& variantviewValue, uint64_t uRow );
   /// sort values to make index work (no sorting and index do not work)
   void sort();
   /// find row value based on sorted value
   std::pair<bool, uint64_t> find( int64_t iFindValue ) const noexcept;
//@}

// ## attributes ----------------------------------------------------------------
public:
   int64_t m_iValue; ///< value to search for
   std::vector< std::pair< int64_t, uint64_t > > m_vectorIndex; ///< sorted vector used as index
   


// ## free functions ------------------------------------------------------------
public:
   /// get the type value index is able to work with
   inline static gd::types::enumType type_s() { return gd::types::eTypeInt64; }
};


/** ===========================================================================
 * \brief index to manage string values (works with byte string values)
 *
 *
 *
 \code
 \endcode
 */
class index_string : public index_base
{
// ## construction -------------------------------------------------------------
public:
   index_string() {}
   index_string( size_t uCount ) { m_vectorIndex.reserve( uCount ); }
   // copy
   index_string( const index_string& o ) { common_construct( o ); }
   index_string( index_string&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   index_string& operator=( const index_string& o ) { common_construct( o ); return *this; }
   index_string& operator=( index_string&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~index_string() {}
private:
   // common copy
   void common_construct( const index_string& o ) {
      m_stringValue = o.m_stringValue; m_vectorIndex = o.m_vectorIndex;
   }
   void common_construct( index_string&& o ) noexcept {
      m_stringValue = o.m_stringValue; m_vectorIndex = std::move( o.m_vectorIndex );
   }

// ## operator -----------------------------------------------------------------
public:
   operator uint64_t() const { return find( m_stringValue ).second; }
   index_string& operator()( const std::string_view& stringValue ) { m_stringValue = stringValue; return *this; }

// ## methods ------------------------------------------------------------------
public:
/** \name OPERATION
*///@{
   /// add value to be indexed
   void add( const gd::variant_view& variantviewValue, uint64_t uRow );
   /// sort values to make index work (no sorting and index do not work)
   void sort();
   /// find row value based on sorted value
   std::pair<bool, uint64_t> find( const std::string_view& stringValue ) const noexcept;
//@}

// ## attributes ----------------------------------------------------------------
public:
   std::string_view m_stringValue; ///< value to search for
   std::vector< std::pair< std::string_view, uint64_t > > m_vectorIndex; ///< sorted vector used as index
   


// ## free functions ------------------------------------------------------------
public:
   /// get the type value index is able to work with
   inline static gd::types::enumType type_s() { return gd::types::eTypeString; }
};






template<typename INDEX, typename TABlE>
INDEX create_index_g( const TABlE& table, unsigned uColumn ) {
   auto uRowCount = table.get_row_count();
   INDEX index_( table.get_row_count() );
   for( decltype(uRowCount) uRow = 0; uRow < uRowCount; uRow++ ) {
      index_.add( table.cell_get_variant_view( uRow, uColumn ), uRow );
   }

   index_.sort();
   return index_;
}

_GD_TABLE_END