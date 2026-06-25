// @FILE [tag: table, index] [description: Indexing table data for fast searches and lookup. Indexes may be used as helpers in table objects] [type: header] [name: gd_table_index.h]

#pragma once

#include <algorithm>
#include <tuple>
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
   /// compact the index by removing duplicate integer values (only first value with lowest row is kept)
   void compact();

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
 * \code
 * // Full example showing declaration, index creation, compaction, and lookup
 * class metadata_cache
 * {
 * public:
 *    const gd::table::table* m_ptableColumn = nullptr;    ///< non-owning metadata table source
 *    gd::table::index_string m_indexstringTable;          ///< index for fast lookup by `table` name
 *
 *    void BuildTableIndex()
 *    {
 *       auto uColumnTable = m_ptableColumn->column_find_index( "table" ); 
 *
 *       m_indexstringTable = gd::table::create_index_g<gd::table::index_string>( *m_ptableColumn, uColumnTable );
 *       m_indexstringTable.compact();
 *    }
 *
 *    std::pair<bool, uint64_t> FindTableRow( std::string_view stringTableName ) const
 *    {
 *       return m_indexstringTable.find( stringTableName );
 *    }
 * };
 * \endcode
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
   /// compact the index by removing duplicate string values (only first value with lowest row is kept)
   void compact();
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


template<typename TYPE1, typename TYPE2>
class index_composite : public index_base
{
public:
   index_composite() {}
   index_composite(size_t uCount) { m_vectorIndex.reserve(uCount); }

   index_composite(const index_composite& o) { common_construct(o); }
   index_composite(index_composite&& o) noexcept { common_construct(std::move(o)); }
   index_composite& operator=(const index_composite& o) { common_construct(o); return *this; }
   index_composite& operator=(index_composite&& o) noexcept { common_construct(std::move(o)); return *this; }

private:
   void common_construct(const index_composite& o) {
      m_pairValue = o.m_pairValue; m_vectorIndex = o.m_vectorIndex;
   }
   void common_construct(index_composite&& o) noexcept {
      m_pairValue = std::move(o.m_pairValue); m_vectorIndex = std::move(o.m_vectorIndex);
   }

public:
   operator uint64_t() const { return find(m_pairValue.first, m_pairValue.second).second; }
   index_composite& operator()(const TYPE1& v1, const TYPE2& v2) {
      m_pairValue = { v1, v2 }; return *this;
   }

public:
   // NOTE: add() has to be specialized or use 'if constexpr' for variant extraction!
   void add(const gd::variant_view& variantviewValue1, const gd::variant_view& variantviewValue2, uint64_t uRow);

   void sort() {
      std::sort(std::begin(m_vectorIndex), std::end(m_vectorIndex), [](const auto& v1, const auto& v2) {
         if(std::get<0>(v1) != std::get<0>(v2)) return std::get<0>(v1) < std::get<0>(v2);
         return std::get<1>(v1) < std::get<1>(v2);
      });
   }

   std::pair<bool, uint64_t> find(const TYPE1& find1, const TYPE2& find2) const noexcept {
      auto itEnd = std::end(m_vectorIndex);
      auto itFind = std::lower_bound(std::begin(m_vectorIndex), itEnd, std::make_pair(find1, find2), [](const auto& v1, const auto& pairFind) {
         if(std::get<0>(v1) != pairFind.first) return std::get<0>(v1) < pairFind.first;
         return std::get<1>(v1) < pairFind.second;
         });

      if(itFind != itEnd && std::get<0>(*itFind) == find1 && std::get<1>(*itFind) == find2) {
         return { true, std::get<2>(*itFind) };
      }
      return { false, (uint64_t)-1 };
   }

   void compact() {
      if(m_vectorIndex.empty()) return;
      auto it = m_vectorIndex.begin();
      auto itTail = std::next(m_vectorIndex.begin());

      for(; itTail != m_vectorIndex.end(); ++itTail) {
         if(std::get<0>(*itTail) != std::get<0>(*it) || std::get<1>(*itTail) != std::get<1>(*it)) {
            ++it;
            *it = *itTail;
         }
         else {
            if(std::get<2>(*itTail) < std::get<2>(*it)) {
               std::get<2>(*it) = std::get<2>(*itTail);
            }
         }
      }
      m_vectorIndex.erase(std::next(it), m_vectorIndex.end());
      m_vectorIndex.shrink_to_fit();
   }

public:
   std::pair<TYPE1, TYPE2> m_pairValue; ///< value to search for
   std::vector< std::tuple< TYPE1, TYPE2, uint64_t > > m_vectorIndex; ///< sorted vector used as index
};

/// Using a type alias to match your naming convention
using index_string_string = index_composite<std::string_view, std::string_view>;

template<typename TYPE1, typename TYPE2>
void index_composite<TYPE1, TYPE2>::add(const gd::variant_view& v1, const gd::variant_view& v2, uint64_t uRow)
{
   TYPE1 key1_;
   TYPE2 key2_;

   if constexpr(std::is_same_v<TYPE1, std::string_view>) key1_ = v1.get_string_view();
   else                                                  key1_ = static_cast<TYPE1>(v1.cast_as_int64());

   if constexpr(std::is_same_v<TYPE2, std::string_view>) key2_ = v2.get_string_view();
   else                                                  key2_ = static_cast<TYPE2>(v2.cast_as_int64());

   m_vectorIndex.emplace_back(key1_, key2_, uRow);
}


/// Create and index for a table based on a specific column, the index will be sorted and ready for searching
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

/// Create and index for a table based on a specific column, the index will be sorted and ready for searching
template<typename INDEX, typename TABlE>
INDEX create_index_g(const TABlE& table, unsigned uColumn1, unsigned uColumn2) {
   auto uRowCount = table.get_row_count();
   INDEX index_(table.get_row_count());
   for(decltype(uRowCount) uRow = 0; uRow < uRowCount; uRow++) {
      index_.add(table.cell_get_variant_view(uRow, uColumn1), table.cell_get_variant_view(uRow, uColumn2), uRow);
   }

   index_.sort();
   return index_;
}


/// Create and index for a table based on a specific column name, the index will be sorted and ready for searching
template<typename INDEX, typename TABlE>
INDEX create_index_g(const TABlE& table, std::string_view stringColumn) {
   auto uColumn = table.column_find_index(stringColumn);
   if(uColumn == (unsigned)-1) { return INDEX(); }
   return create_index_g<INDEX>(table, uColumn);
}






_GD_TABLE_END
