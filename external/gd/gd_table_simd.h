// @FILE [tag: table, simd] [description: Table that works well as data transfer object and to temporary storage] [type: header] [name: gd_table_simd.h]

/**
 * \file gd_table_simd.h
 *
 * @brief Table used to transfer/move data, Use `gd::table::simd::table` class that is optimized for data transfer.
 *
 | Area                | Methods (Examples)                                                                 | Description                                                                                   |
 |---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | table_column_buffer(...), common_construct(...)                                    | Constructors for various ways to create and copy table buffers, including from other tables.   |
 | Column Management   | column_add(...), column_rename(...), column_exists(...), column_get_index(...)     | Methods for adding, renaming, finding, and managing columns and their metadata.                |
 | Row Management      | row_add(...), row_set(...), row_get_variant_view(...), row_reserve_add(...)        | Methods for adding, setting, retrieving, and reserving rows and their values.                  |
 | Cell Access         | cell_get(...), cell_set(...), cell_get_variant_view(...), cell_get_length(...)     | Methods for accessing and modifying individual cell values, including type conversion.         |
 | Data Operations     | append(...), harvest(...), plant(...), swap(...), erase(...), split(...)           | Methods for copying, merging, splitting, swapping, and erasing data between tables.            |
 | Searching/Sorting   | find(...), find_variant_view(...), sort(...), sort_null(...)                       | Methods for searching for values and sorting rows by column values, including null handling.   |
 | Iteration/ForEach   | column_for_each(...), row_for_each(...)                                            | Methods for iterating over columns and rows with callback functions.                           |
 | Debug/Printing      | debug::print(...), debug::print_row(...), debug::print_column(...)                 | Methods for printing table, row, and column information for debugging purposes.                |
 | Utility/Meta        | clear(), count_used_rows(), count_free_rows(), column_match_s(...), to_columns(...)| Utility methods for clearing, counting, matching, and converting table/column metadata.        |
 *
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "gd_arguments.h"
#include "gd_table.h"
#include "gd_table_column.h"
#include "gd_types.h"
#include "gd_variant_view.h"

#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreorder-ctor"
#pragma clang diagnostic ignored "-Wunused-variable"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
#pragma GCC diagnostic push
#elif defined( _MSC_VER )
#pragma warning(push)
#pragma warning( disable : 26495 )
#endif



#ifndef _GD_TABLE_SIMD_BEGIN
#  define _GD_TABLE_SIMD_BEGIN namespace gd { namespace table { namespace simd {
#  define _GD_TABLE_SIMD_END } } }
#endif

_GD_TABLE_SIMD_BEGIN

class table_base
{
public:
   /**
    * \brief constant numbers used in table or items used in table
    *
    * ## Column states
    * - eColumnStateLength: column value begins with length
    * - eColumnStateReference: column value is stored in reference object
    * - eColumnStateKey: column acts as key column
    *
    * ## Row states
    * - eRowStateUse: row flag marking that row is in use
    * - eRowStateDeleted: row flag marking that row is deleted
    *
    * ## Table flags
    * - eTableFlagNull32: reserve 32 bit for each row to mark null for column if no value (max 32 columns)
    * - eTableFlagNull64: reserve 64 bit for each row to mark null for column if no value (max 64 columns)
    * - eTableFlagRowStatus: enable row status (if row is valid, modified, deleted)
    * - eTableFlagDuplicateStrings: enable duplicate strings, strings are not checked for duplicates
    *
    * ## Size information
    * - eSpaceNull32Columns: space marking null columns
    * - eSpaceNull64Columns: space marking null columns
    * - eSpaceRowState: space where row state data is placed
    * - eSpaceRowGrowBy: default number of rows to grow by
    * - eSpaceFirstAllocate: number of rows to allocate before any values is added
    */                                                                        // ## @API [tag: constant] [description: constant values used in table]
   enum
   {
      // ## column flags marking column states, how column behaves/works
      eColumnStateLength = 0x01,                                              ///< column flag marking that value begins with length
      eColumnStateReference = 0x02,                                           ///< column flag marking that value is stored in reference object
      eColumnStateKey = 0x04,                                                 ///< column acts as key column

      // ## row state flags
      eRowStateUse = 0x01,                                                    ///< row flag marking that row is in use
      eRowStateDeleted = 0x02,                                                ///< row flag marking that row is deleted

      // ## table flags marking table states, how table behaves
      eTableFlagNull32 = 0x0001,                                              ///< reserve 32 bit for each row to mark null for column if no value
      eTableFlagNull64 = 0x0002,                                              ///< reserve 64 bit for each row to mark null for column if no value
      eTableFlagRowStatus = 0x0004,                                           ///< enable row status (if row is valid, modified, deleted)
      eTableFlagDuplicateStrings = 0x0008,                                    ///< enable duplicate strings, reference string are not checked for duplicates
      eTableFlagMAX = 0x000f,                                                 ///< max flag value
      eTableFlagMASK = 0x000f,                                                ///< mask for table state values

      // ## size information used to calculate space needed by table
      eSpaceNull32Columns = sizeof(uint32_t),                                 ///< space marking null columns
      eSpaceNull64Columns = sizeof(uint64_t),                                 ///< space marking null columns
      eSpaceRowState = sizeof(uint32_t),                                      ///< space where row state data is placed
      eSpaceRowGrowBy = 10,                                                   ///< default number of rows to grow by
      eSpaceFirstAllocate = 10,                                               ///< number of rows to allocate before any values is added
   };

public:
   /**
    * @brief iterator to move trough rows in table
    */
   struct iterator_row                                                        // ## @API [tag: iterator] [description: row iterator for table]
   {
      iterator_row(): m_uRow(0), m_ptable(nullptr) {}
      iterator_row( uint64_t uRow, table_base* ptable ): m_uRow(uRow), m_ptable(ptable ) {}

      auto operator*() const { return gd::table::row<table_base>( m_ptable, m_uRow ); }

      bool operator==( const iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow == m_uRow; }
      bool operator!=( const iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow != m_uRow; }

      uint64_t get_row() const noexcept { return m_uRow; }
      int64_t get_irow() const noexcept { return (int64_t)m_uRow; }

      const uint8_t* get( tag_raw ) const { return m_ptable->row_get(m_uRow); }

      iterator_row& operator++() { m_uRow++; return *this; }
      iterator_row operator++(int) { iterator_row it_ = *this; ++(*this); return it_; }
      iterator_row& operator--() { m_uRow--; return *this; }
      iterator_row operator--(int) { iterator_row it_ = *this; --(*this); return it_; }

      auto operator+( std::ptrdiff_t iDistance ) { return iterator_row( (std::ptrdiff_t)m_uRow + iDistance, m_ptable ); }
      auto operator-( std::ptrdiff_t iDistance ) { return iterator_row( (std::ptrdiff_t)m_uRow - iDistance, m_ptable ); }

      gd::variant_view cell_get_variant_view( unsigned uIndex ) const { return m_ptable->cell_get_variant_view( m_uRow, uIndex ); }
      gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const { return m_ptable->cell_get_variant_view( m_uRow, stringName ); }
      std::vector< gd::variant_view > cell_get_variant_view() const { return m_ptable->cell_get_variant_view( m_uRow ); }
      std::vector< gd::variant_view > get_variant_view( const std::vector<unsigned>& vectorColumn ) const { return m_ptable->row_get_variant_view( m_uRow, vectorColumn ); }

      gd::argument::arguments get_arguments() const { return m_ptable->row_get_arguments( m_uRow ); } 
      void get_arguments( gd::argument::arguments& argumentsRow ) const { m_ptable->row_get_arguments( m_uRow, argumentsRow ); }

      void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, uColumn, variantviewValue ); }
      void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, stringName, variantviewValue ); }
      void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert ) { m_ptable->cell_set( m_uRow, uColumn, variantviewValue, tag_convert{} ); }
      void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert ) { m_ptable->cell_set( m_uRow, stringName, variantviewValue, tag_convert{} ); }

      uint64_t m_uRow;     ///< active row index
      table_base* m_ptable; ///< pointer to table that owns the iterator
   };

   struct const_iterator_row
   {
      const_iterator_row(): m_uRow(0), m_ptable(nullptr) {}
      const_iterator_row( uint64_t uRow, const table_base* ptable ): m_uRow(uRow), m_ptable(ptable) {}
      const_iterator_row( int64_t iRow, const table_base* ptable ): m_uRow((uint64_t)iRow), m_ptable(ptable) {}

      bool operator==( const const_iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow == m_uRow; }
      bool operator!=( const const_iterator_row& o ) const { assert( o.m_ptable == m_ptable ); return o.m_uRow != m_uRow; }

      uint64_t get_row() const noexcept { return m_uRow; }
      int64_t get_irow() const noexcept { return (int64_t)m_uRow; }

      const uint8_t* get( tag_raw ) const { return m_ptable->row_get(m_uRow); }

      gd::variant_view cell_get_variant_view( unsigned uIndex ) const { return m_ptable->cell_get_variant_view( m_uRow, uIndex ); }
      gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const { return m_ptable->cell_get_variant_view( m_uRow, stringName ); }
      std::vector< gd::variant_view > cell_get_variant_view() const { return m_ptable->cell_get_variant_view( m_uRow ); }
      std::vector< gd::variant_view > get_variant_view( const std::vector<unsigned>& vectorColumn ) const { return m_ptable->row_get_variant_view( m_uRow, vectorColumn ); }

      const_iterator_row& operator++() { m_uRow++; return *this; }
      const_iterator_row operator++(int) { const_iterator_row it_ = *this; ++(*this); return it_; }
      const_iterator_row& operator--() { m_uRow--; return *this; }
      const_iterator_row operator--(int) { const_iterator_row it_ = *this; --(*this); return it_; }

      const_iterator_row operator+( int64_t iDistance ) { return const_iterator_row( m_uRow + iDistance, m_ptable ); }
      const_iterator_row operator-( int64_t iDistance ) { return const_iterator_row( m_uRow - iDistance, m_ptable ); }

      uint64_t m_uRow;
      const table_base* m_ptable;
   };


public:

// ## @API [tag: construct] [description: table construction, lots of constructors to simplify how to create new tables]
public:
   table_base() : m_uFlags(0), m_uRowSize(0), m_uRowCount(0), m_uPackCount(0), m_uRowGrowBy(0) {}
   table_base(unsigned uRowCount) : m_uFlags(0), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(uRowCount), m_uRowGrowBy(0) {}
   table_base(unsigned uRowCount, unsigned uFlags) : m_uFlags(uFlags), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(uRowCount), m_uRowGrowBy(0) { assert(m_uFlags < eTableFlagMAX); }
   table_base(unsigned uRowCount, unsigned uFlags, unsigned uGrowBy) : m_uFlags(uFlags), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(uRowCount), m_uRowGrowBy(uGrowBy) { assert(m_uFlags < eTableFlagMAX); }
   table_base(tag_null) : m_uFlags(eTableFlagNull64), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(0) { assert(m_uFlags < eTableFlagMAX); }
   table_base(tag_full_meta) : m_uFlags(eTableFlagNull64 | eTableFlagRowStatus), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(0) { assert(m_uFlags < eTableFlagMAX); }

   // ## @API [tag: operator] [description: table operators]
public:
   std::vector<gd::variant_view> operator[](uint64_t uRow) const { return row_get_variant_view(uRow); }

   //table& operator+=(const table& o) { append(o); return *this; }

   gd::variant_view operator()(uint64_t uRow, unsigned uColumn) const { return cell_get_variant_view(uRow, uColumn); }
   gd::variant_view operator()(uint64_t uRow, const std::string_view& stringName) const { return cell_get_variant_view(uRow, stringName); }

   cell<table_base> operator[](const std::pair<uint64_t, unsigned>& pairCell) noexcept { return cell<table_base>(this, pairCell.first, pairCell.second); }
   cell<table_base> operator[](const std::pair<uint64_t, unsigned>& pairCell) const noexcept { return cell<table_base>(const_cast<table_base*>(this), pairCell.first, pairCell.second); }
   cell<table_base> operator[](const std::pair<uint64_t, std::string_view>& pairCell) noexcept {
      auto column_ = column_get_index(pairCell.second);
      return cell<table_base>(this, pairCell.first, column_);
   }
   cell<table_base> operator[](const std::pair<uint64_t, std::string_view>& pairCell) const noexcept {
      auto column_ = column_get_index(pairCell.second);
      return cell<table_base>(const_cast<table_base*>(this), pairCell.first, column_);
   }

#if defined( GD_COMPILER_HAS_CPP23_SUPPORT )
   // ## multidimensional subscript operator used to access or set cell values in table, it is used like table( row, column ) or table( row, "column_name" )
   cell<table_base> operator[](uint64_t uRow, unsigned uColumn) noexcept { return cell<table_base>(this, uRow, uColumn); }
   cell<table_base> operator[](uint64_t uRow, unsigned uColumn) const noexcept { return cell<table_base>(const_cast<table_base*>(this), uRow, uColumn); }

   cell<table_base> operator[](uint64_t uRow, std::string_view stringColumnName) noexcept {
      auto column_ = column_get_index(stringColumnName);
      return cell<table_base>(this, uRow, column_);
   }
   cell<table_base> operator[](uint64_t uRow, std::string_view stringColumnName) const noexcept {
      auto column_ = column_get_index(stringColumnName);
      return cell<table_base>(const_cast<table_base*>(this), uRow, column_);
   }
#endif   


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{
   unsigned get_flags() const noexcept { return m_uFlags; }
   void set_state [[deprecated]] (uint32_t uFlags) noexcept { m_uFlags = uFlags; }
   void set_flags(uint32_t uFlags) noexcept { m_uFlags = uFlags; }
   void set_state [[deprecated]] (tag_full_meta) noexcept { m_uFlags = eTableFlagRowStatus | eTableFlagNull64; }
   void set_flags(tag_full_meta) noexcept { m_uFlags = eTableFlagRowStatus | eTableFlagNull64; }
   void set_flags(uint32_t uSet, uint32_t uClear) noexcept { m_uFlags |= uSet; m_uFlags &= ~uClear; }
   unsigned get_column_count() const noexcept { return (unsigned)m_pcolumns->size(); }
   /// Get number of rows with values
   uint64_t get_row_count() const noexcept { return m_uRowCount; }

   unsigned size_row() const noexcept { return m_uRowSize; }
   unsigned size_row_meta() const noexcept;
   /// get meta block size
   uint64_t size_meta_total() const noexcept { return size_row_meta() * m_uRowReservedPackCount; }
   /// get meta block size for rows
   uint64_t size_meta_total(uint64_t uRowCount) const noexcept { return size_row_meta() * uRowCount; }
   /// calc and return total allocated memory size
   uint64_t size_reserved_total() const noexcept { return (m_uRowSize + size_row_meta()) * m_uRowReservedPackCount; }
   /// calc and return total allocated memory size for rows
   uint64_t size_reserved_total(uint64_t uRowCount) const noexcept { return (m_uRowSize * uRowCount) + (size_row_meta() * uRowCount); }

   bool is_null() const { return m_uFlags & (eTableFlagNull32 | eTableFlagNull64); }
   bool is_null32() const { return m_uFlags & eTableFlagNull32; }
   bool is_null64() const { return m_uFlags & eTableFlagNull64; }
   bool is_rowstatus() const { return m_uFlags & eTableFlagRowStatus; }
   bool is_duplicated_strings() const { return m_uFlags & eTableFlagDuplicateStrings; }
   bool is_rowmeta() const { return m_puMetaData != nullptr; }



   // ## @API [tag: column] [description: column management methods]

/// prepare to store column information
   void column_prepare() { if(m_pcolumns == nullptr) m_pcolumns = new_columns_s(); }

   /// @name column_add
   /// Add columns to table, this is typically done before adding values to table. Remember to call @see prepare before adding data
   /// Parameters:
   /// - `uColumnType` type of column to add @see: gd::types::enumType
   /// - `stringType` type of columns as string name, will be converted to the type number
   /// - `uSize` if type do not have a fixed size then size will have the maximum length for text
   /// - `columnToAdd` has all column properties for column to add
   /// - `stringName` name for column
   /// - `stringAlias` alias name for column (column can have both name and alias)
   ///@{
   table_base& column_add(const detail::column& columnToAdd) { m_pcolumns->add(columnToAdd); return *this; }
   table_base& column_add(unsigned uColumnType, const std::string_view& stringName) { return column_add(uColumnType, 0, stringName); }
   table_base& column_add(unsigned uColumnType, unsigned uSize);
   table_base& column_add(unsigned uColumnType, unsigned uSize, std::string_view stringName, std::string_view stringAlias);
   table_base& column_add(unsigned uColumnType, unsigned uSize, std::string_view stringName) { return column_add(uColumnType, uSize, stringName, std::string_view{}); }
   table_base& column_add(unsigned uColumnType, unsigned uSize, std::string_view stringAlias, tag_alias) { return column_add(uColumnType, uSize, std::string_view{}, stringAlias); }
   table_base& column_add(const std::vector< std::tuple< unsigned, unsigned, std::string_view > >& vectorColumn);
   table_base& column_add(std::string_view stringType) { return column_add(detail::column((unsigned)gd::types::type_g(stringType))); }
   table_base& column_add(std::string_view stringType, std::string_view stringName) { return column_add((unsigned)gd::types::type_g(stringType), 0, stringName, std::string_view{}); }
   table_base& column_add(std::string_view stringType, unsigned uSize) { return column_add((unsigned)gd::types::type_g(stringType), uSize); }
   table_base& column_add(std::string_view stringType, unsigned uSize, std::string_view stringName) { return column_add((unsigned)gd::types::type_g(stringType), uSize, stringName, std::string_view{}); }
   table_base& column_add(std::string_view stringType, unsigned uSize, std::string_view stringAlias, tag_alias) { return column_add((unsigned)gd::types::type_g(stringType), uSize, std::string_view{}, stringAlias); }
   table_base& column_add(std::string_view stringType, unsigned uSize, std::string_view stringName, std::string_view stringAlias) { return column_add((unsigned)gd::types::type_g(stringType), uSize, stringName, stringAlias); }

   table_base& column_add(const std::initializer_list<std::pair<std::string_view, unsigned>>& listType, tag_type_name);
   table_base& column_add(const std::initializer_list<std::tuple<std::string_view, unsigned, std::string_view>>& listType, tag_type_name);

   table_base& column_add(const std::vector< std::pair< std::string_view, unsigned > >& vectorType, tag_type_name);
   table_base& column_add(const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorType, tag_type_name);
   table_base& column_add(const std::vector< std::tuple< std::string_view, unsigned, std::string_view, std::string_view > >& vectorType, tag_type_name);
   table_base& column_add(const std::initializer_list< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name);
   table_base& column_add(const std::vector< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name);
   table_base& column_add(const std::vector< std::pair< unsigned, unsigned > >& vectorType, tag_type_constant);
   table_base& column_add(const table_base* p_);
   ///@}

   /// @brief find column index for column name
   int column_find_index(std::string_view stringName) const noexcept;
   int column_find_index(std::string_view stringAlias, tag_alias) const noexcept;
   /// @brief find column index for column name with wildcard, wildcars like ? and * are supported
   int column_find_index(const std::string_view& stringWildcard, tag_wildcard) const noexcept;
   /// @brief get column index for column name, asserts if not found
   unsigned column_get_index(const std::string_view& stringName) const noexcept;
   unsigned column_get_index(const std::string_view& stringAlias, tag_alias) const noexcept;
   /// @brief get column index for column name with wildcard, wildcars like ? and * are supported, asserts if not found
   unsigned column_get_index(const std::string_view& stringName, tag_wildcard) const noexcept;

   auto column_begin() { return m_pcolumns->begin(); }
   auto column_end() { return m_pcolumns->end(); }
   auto column_begin() const { return m_pcolumns->begin(); }
   auto column_end() const { return m_pcolumns->end(); }
   auto column_cbegin() const { return m_pcolumns->cbegin(); }
   auto column_cend() const { return m_pcolumns->cend(); }



   std::pair<bool, std::string> prepare( unsigned uValueSize, unsigned uStride );

   /// return pointer to section holding null column information
   uint8_t* row_get_null(uint64_t uRow) const noexcept;


   uint8_t* row_get( uint64_t uRow ) const noexcept {                                              assert(m_uPackCount == 4 || m_uPackCount == 8 && "Pack size must be 4 or 8");
      uint64_t uMemoryRow = uRow / count_pack();                               // get memory row index, each row holds array of values, each array is called pack, each pack holds 4 or 8 rows
      return m_puData + uMemoryRow * m_uRowSize; 
   }

   void row_get_arguments(uint64_t uRow, gd::argument::arguments& argumentsValue) const;
   gd::argument::arguments row_get_arguments(uint64_t uRow) const { gd::argument::arguments a_; row_get_arguments(uRow, a_); return a_; }
   gd::argument::arguments row_get_arguments(uint64_t uRow, const unsigned* puIndex, unsigned uSize) const;
   gd::argument::arguments row_get_arguments(uint64_t uRow, const std::vector<unsigned>& vectorIndex) const { return row_get_arguments(uRow, vectorIndex.data(), (unsigned)vectorIndex.size()); }


   std::vector<gd::variant_view> row_get_variant_view(uint64_t uRow) const;
   std::vector<gd::variant_view> row_get_variant_view(uint64_t uRow, const unsigned* puIndex, unsigned uSize) const;
   std::vector<gd::variant_view> row_get_variant_view(uint64_t uRow, const std::vector<unsigned>& vectorIndex) const { return row_get_variant_view(uRow, vectorIndex.data(), (unsigned)vectorIndex.size()); }
   void row_get_variant_view(uint64_t uRow, std::vector<gd::variant_view>& vectorValue) const;
   void row_get_variant_view(uint64_t uRow, unsigned uOffset, std::vector<gd::variant_view>& vectorValue) const;
   void row_get_variant_view(uint64_t uRow, const unsigned* puIndex, unsigned uSize, std::vector<gd::variant_view>& vectorValue) const;
   void row_get_variant_view(uint64_t uRow, const std::vector<unsigned>& vectorIndex, std::vector<gd::variant_view>& vectorValue) const { row_get_variant_view(uRow, vectorIndex.data(), (unsigned)vectorIndex.size(), vectorValue); }

   void row_add(uint64_t uCount);
   /// Simple add one row to table that is safe (if table have null values these are automatically set to null)
   uint64_t row_add_one() { row_add(1); return m_uRowCount - 1; }

   void row_reserve_add(uint64_t uCount);
   void row_reserve_add() { row_reserve_add(1); }

   bool cell_is_null(uint64_t uRow, unsigned uColumn) const noexcept;
   bool cell_is_null(uint64_t uRow, std::string_view stringName) const noexcept { return cell_is_null(uRow, column_get_index(stringName)); }
   const reference* cell_get_reference(uint64_t uRow, unsigned uColumn) const noexcept;

   uint32_t cell_get_value32(uint64_t uRow, unsigned uColumn) const noexcept;
   uint64_t cell_get_value64(uint64_t uRow, unsigned uColumn) const noexcept;

   gd::variant_view cell_get_variant_view(uint64_t uRow, unsigned uColumn) const noexcept;
   gd::variant_view cell_get_variant_view(uint64_t uRow, const std::string_view& stringName) const noexcept;
   std::vector< gd::variant_view > cell_get_variant_view(uint64_t uRow, unsigned uFromColumn, unsigned uToColumn) const;
   std::vector< gd::variant_view > cell_get_variant_view(uint64_t uRow) const { return cell_get_variant_view(uRow, 0, get_column_count()); }


   void cell_set_value(uint64_t uRow, unsigned uColumn, uint32_t uValue);
   void cell_set_value(uint64_t uRow, unsigned uColumn, uint64_t uValue);

   void cell_set(uint64_t uRow, unsigned uColumn, gd::variant_view variantviewValue);
   void cell_set(uint64_t uRow, unsigned uColumn, gd::variant_view variantviewValue, tag_convert);
   void cell_set(uint64_t uRow, const std::string_view& stringName, gd::variant_view variantviewValue);
   void cell_set(uint64_t uRow, const std::string_view& stringAlias, gd::variant_view variantviewValue, tag_alias);
   void cell_set(uint64_t uRow, const std::string_view& stringName, gd::variant_view variantviewValue, tag_convert);
   void cell_set(uint64_t uRow, std::string_view stringName, gd::variant_view variantviewValue, tag_adjust);
   void cell_set(uint64_t uRow, const std::string_view& stringAlias, gd::variant_view variantviewValue, tag_convert, tag_alias);
   void cell_set(uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue);
   void cell_set(uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue, tag_convert);


   void cell_set_null(uint64_t uRow, unsigned uColumn);
   void cell_set_null(uint64_t uRow, std::string_view stringName);
   void cell_set_not_null(uint64_t uRow, unsigned uColumn);

protected:
   /// Counts number of values in each array (simd block)
   unsigned count_pack() const noexcept { return m_uPackCount; }
   /// Counts number of rows that can be stored in allocated memory, this is the number of reserved rows
   uint64_t count_reserved_row() const noexcept { return m_uRowReservedPackCount * count_pack(); }
   /// Size of each value in bytes, all values in table have this size
   unsigned size_value() const noexcept { return m_uValueSize; }
   /// Size of each block/array in bytes
   unsigned size_pack() const noexcept { return m_uPackCount * size_value(); }
   /// Offset position for column in row. Use this to get the specific column value in row and array for AoSoA (Array of Structure of Arrays) layout
   unsigned offset(uint64_t uRow, unsigned uColumn, tag_column) const noexcept { return uColumn * size_pack() + (uRow % count_pack()) * size_value(); }


// ## @API [tag: attribute, member] [description: member data to table]
public:
   uint8_t* m_puData = nullptr;        ///< data to hold values in table
   uint8_t* m_puMetaData = nullptr;    ///< data block with row meta information
   unsigned m_uFlags;                  ///< state information for table (eTableFlagNull32, eTableFlagNull64, eTableFlagRowStatus, eTableFlagUniqueStrings )
   unsigned m_uRowSize;                ///< row size in bytes
   unsigned m_uRowMetaSize;            ///< meta data size in bytes for each row
   unsigned m_uRowGrowBy = eSpaceRowGrowBy;///< if table needs more space, this holds number of rows to grow by
   unsigned m_uValueSize;              ///< size of each value in bytes, used to calculate row size
   unsigned m_uPackCount;               ///< number of values for each block/array, in AoSoA (Array of Structure of Arrays) layout
   uint64_t m_uRowCount;               ///< row count (row count * row size = total amount of bytes allocated)
   uint64_t m_uRowReservedPackCount = eSpaceFirstAllocate; ///< reserved row block count, max number of row blocks that can be placed in allocated memory
   gd::argument::arguments m_argumentsProperty; ///< table properties
   references m_references;            ///< Stores blob data
   names m_namesColumn;                ///< names for columns in table. this works like a data store for const text values used to store column names and aliases
   detail::columns* m_pcolumns{nullptr};

// ## @API [tag: static] [description: static member methods]
public:
   /// Create columns object on heap
   static detail::columns* new_columns_s();



};

/** ---------------------------------------------------------------------------
 * @brief Get number of bytes used to store meta information for each row
 * *Calculation steps to find out meta size needed for each row*
 * - null flags for each columnd, 32 or 64 bits (4 or 8 bytes)
 * - row state, 4 bytes
 * - arguments object size
 * @return unsigned bytes needed to store meta information for row
*/
inline unsigned table_base::size_row_meta() const noexcept {
   unsigned uMetaDataSize = 0;
   if(is_null32() == true)      uMetaDataSize += eSpaceNull32Columns;
   else if(is_null64() == true) uMetaDataSize += eSpaceNull64Columns;
   if(is_rowstatus() == true)   uMetaDataSize += eSpaceRowState;

   return uMetaDataSize;
}

/** ---------------------------------------------------------------------------
 * @brief Return pointer to row null value section (flags in metadata marking cell null values)
 * This is the first part of meta data for each row, if table is created to store null values for each column
 * @param uRow index for row null value is returned for
 * @return uint8_t* pointer to row null value section
*/
inline uint8_t* table_base::row_get_null( uint64_t uRow ) const noexcept {                         assert( uRow < (m_uRowReservedPackCount * count_pack())); assert( m_puMetaData != nullptr );
   return reinterpret_cast<uint8_t*>( m_puMetaData + (uRow * m_uRowMetaSize) );
}

/** ---------------------------------------------------------------------------
 * @brief Check if cell is null
 * @param uRow row for cell
 * @param uColumn index for cell column
 * @return true if null, false if not null
*/
inline bool table_base::cell_is_null( uint64_t uRow, unsigned uColumn ) const noexcept {           assert( uRow < (m_uRowReservedPackCount * count_pack())); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   uint64_t uNullRow = 0; // flags for null values in row
   const auto* puRow = row_get_null( uRow );
   if( is_null32() ) uNullRow = (uint64_t)*(uint32_t*)puRow;
   else              uNullRow = *(uint64_t*)puRow;

   return (uNullRow & (1ULL << uColumn)) != 0;
}



/** ---------------------------------------------------------------------------
 * @brief Set value in column to null (marks null flag for column)
 * @param uRow row where cell is
 * @param uColumn cell column
*/
inline void table_base::cell_set_null(uint64_t uRow, unsigned uColumn) {                           assert(uRow < (m_uRowReservedPackCount * count_pack())); assert(m_uFlags & (eTableFlagNull32 | eTableFlagNull64));
   auto puRow = row_get_null(uRow);

#ifdef _DEBUG
   uint64_t uNull_d = 0;
   if(is_null32()) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

   if(is_null32()) *(uint32_t*)puRow |= ((uint32_t)1 << uColumn);
   else              *(uint64_t*)puRow |= ((uint64_t)1 << uColumn);

#ifdef _DEBUG
   if(is_null32()) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG
}

/** ---------------------------------------------------------------------------
 * @brief Set value in column to null (marks null flag for column)
 * @param uRow row where cell is
 * @param stringName cell column name
*/
inline void table_base::cell_set_null( uint64_t uRow, std::string_view stringName ) {              assert(uRow < (m_uRowReservedPackCount * count_pack())); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   unsigned uColumnIndex = column_get_index( stringName );
   cell_set_null( uRow, uColumnIndex);
}


inline void table_base::cell_set_not_null(uint64_t uRow, unsigned uColumn) {                       assert(uRow < (m_uRowReservedPackCount * count_pack())); assert(m_uFlags & (eTableFlagNull32 | eTableFlagNull64));
   auto puRow = row_get_null(uRow);

#ifdef _DEBUG
   uint64_t uNull_d = 0;
   if(is_null32()) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

   if(is_null32()) *(uint32_t*)puRow &= ~((uint32_t)1 << uColumn);
   else              *(uint64_t*)puRow &= ~((uint64_t)1 << uColumn);

#ifdef _DEBUG
   if(is_null32()) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

}



/** ===========================================================================
 * \brief simd table, optimized for simd operations using  AoSoA (Array of Structure of Arrays) layout.
 * 
 */
template<std::size_t VALUESIZE = 8, std::size_t PACKCOUNT = 8>
class table : public table_base
{
// ## @API [tag: construct] [description: table construction, lots of constructors to simplify how to create new tables]
public:
   table(): table_base() { common_construct_set_simd(); }
   table(unsigned uRowCount): table_base(uRowCount) { common_construct_set_simd(); }
   table(unsigned uRowCount, unsigned uFlags): table_base(uRowCount, uFlags) { common_construct_set_simd(); }
   table(unsigned uRowCount, unsigned uFlags, unsigned uGrowBy): table_base(uRowCount, uFlags, uGrowBy) { common_construct_set_simd(); }
   table(tag_null): table_base(tag_null{}) { common_construct_set_simd(); }
   table(tag_full_meta): table_base(tag_full_meta{}) { common_construct_set_simd(); }

   void common_construct_set_simd() { m_uValueSize = VALUESIZE; m_uPackCount = PACKCOUNT; }

   //void row_add(uint64_t uCount);

   uint8_t* row_get(uint64_t uRow) const noexcept;


   std::pair<bool, std::string> prepare() { return table_base::prepare(m_uValueSize_s, m_uPackCount_s); }

   static constexpr std::size_t m_uValueSize_s = VALUESIZE;
   static constexpr std::size_t m_uPackCount_s = PACKCOUNT;
};

template<std::size_t VALUESIZE, std::size_t PACKCOUNT>
uint8_t* table<VALUESIZE, PACKCOUNT>::row_get(uint64_t uRow) const noexcept {
   // ## actual row is result of dividing row index by pack count, because each row block contains PACKCOUNT number of rows
   uint64_t uRowPack = uRow / PACKCOUNT;                                                           assert(uRowPack < m_uRowReservedPackCount );
   return m_puData + uRowPack * m_uRowSize;
}

// ---------------------------------------------------------------------------
///  Add rows to table, this will increase the number of rows in table by uCount
/*
template<std::size_t VALUESIZE, std::size_t PACKCOUNT>
void table<VALUESIZE, PACKCOUNT>::row_add(uint64_t uCount) {                                        assert(uCount > 0);
   const uint64_t uRowCountNew = m_uRowCount + uCount;

   // ## Calculate number of row blocks needed for new row count, each block contains STRIDE number of rows
   const uint64_t uRowBlockCountNew = (uRowCountNew + PACKCOUNT - 1) / PACKCOUNT;

   if( uRowBlockCountNew > m_uRowReservedPackCount )
   {
      row_reserve_add( uRowBlockCountNew - m_uRowReservedPackCount );
      m_uRowReservedPackCount = uRowBlockCountNew;
   }

222222222222222222222222222

   m_uRowCount = uRowCountNew;
}
*/


_GD_TABLE_SIMD_END
