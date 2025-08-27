/**
 * \file gd_table_column-buffer.h
 * 
 * @brief Table used to transfer/move data, Use `gd::table::dto::table` class that is optimized for data transfer.
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
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0construct.table_column_buffer` - construct table_column_buffer
 * - `0TAG0add.table_column_buffer` - add columns to table
 * - `0TAG0find_column.table_column_buffer` - find methods
 * - `0TAG0cell.table_column_buffer` - cell methods like get and set cell value
 * - `0TAG0find.table_column_buffer` - find methods like finding information in table
 * - `0TAG0harvest.table_column_buffer` - harvest logic is to "harvest" information from table
 * - `0TAG0plant.table_column_buffer` - plant logic to insert data into table
 * - `0TAG0attributes.table_column_buffer` - class members in table object
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



#ifndef _GD_TABLE_BEGIN
#  define _GD_TABLE_BEGIN namespace gd { namespace table {
#  define _GD_TABLE_END } }
#endif

_GD_TABLE_BEGIN

class table;
namespace detail { class columns; }

/**
 * \brief  Manage table data store as a big block
 * 
 * 

## Internal data for table

Table stores its data in one single memory block. First part is cell values and
each value has a fixed buffer where data is stored.
int32 is four byte, int64 is 8 byte and string has the max number of bytes specified.
Same logic is used for all primitives and derived types are set with a max buffer size.
Table also support references, they work like pointers so for each column set as 
reference value buffer in table is storing a index to reference and that value can have any size.

Optional is to have meta data for table data. If table store information for null values and/or
valid, deleted or some other row state then a memory block is appended to block that holds table data.
All is stored in one single memory block.

If table need to grow memory block it creates a new block that is larger and data
is copied to that block, old block is deleted.

All table data is stored in one single memory block, this is done to speed up access times and improve cache locality.
Information about each column is stored in `gd::table::dto::table::column` structure.

*Sample data layout*

    ╔═══════╦════════════╦═════════════════════════╦════╗
    ║ int32 ║   int64    ║         string          ║int8║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ║       ║            ║                         ║    ║
    ╠═══════╩════════════╬═════════════════════════╩════╝
    ║ meta data for each ║
    ║ row                ║
    ║                    ║
    ║                    ║
    ║                    ║
    ║                    ║
    ║                    ║
    ╚════════════════════╝


 *
 *
 *
 \code
gd::table::dto::table table( 10 );
// fill table with values (how this is done is not show here)
application::database::EXECUTE_SelectToTable_g( &database, R"(SELECT name FROM sqlite_master WHERE type='table')", &table);
table.column_rename( 0, "table" );
for( auto it = std::begin( tableTable ); it != std::end( tableTable ); it++ )
{
   auto vectorValue = it.get_variant_view( vectorTableColumn );
   std::cout << gd::debug::print( vectorValue ) << "\n";
}

 \endcode

 *Sample on how to create tables*
 \code
 using namespace gd::table::dto;
 // create table that are able to stor null values and holds rowstatus, table still needs columns and after that you need to prepare it
 gd::table::dto::table tableCodeGroup( 10, (table::eTableFlagNull32|table::eTableFlagRowStatus) );

 // creates table ready to be used, this generates three columns
 gd::table::dto::table tableCodeGroup( (table::eTableFlagNull32|table::eTableFlagRowStatus), { { "int64", "GroupK"}, { "rstring", "FName"}, { "rstring", "FDescription"} }, gd::table::tag_prepare{} );
 \endcode

 */
class table_column_buffer
{
public:
   /** 
    * \brief constant numbers used in table or items used in table
    */
   enum 
   { 
      // ## column flags marking column states, how column behaves/works
      eColumnStateLength      = 0x01,                                          ///< column flag marking that value begins with length
      eColumnStateReference   = 0x02,                                          ///< column flag marking that value is stored in reference object
      eColumnStateKey         = 0x04,                                          ///< column acts as key column

      // ## row state flags
      eRowStateUse            = 0x01,                                          ///< row flag marking that row is in use
      eRowStateDeleted        = 0x02,                                          ///< row flag marking that row is deleted

      // ## table flags marking table states, how table behaves
      eTableStateNull32       = 0x0001,                                        ///< reserve 32 bit for each row to mark null for column if no value
      eTableFlagNull32        = 0x0001,                                        ///< reserve 32 bit for each row to mark null for column if no value
      eTableStateNull64       = 0x0002,                                        ///< reserve 64 bit for each row to mark null for column if no value
      eTableFlagNull64        = 0x0002,                                        ///< reserve 64 bit for each row to mark null for column if no value
      eTableStateRowStatus    = 0x0004,                                        ///< enable row status (if row is valid, modified, deleted)
      eTableFlagRowStatus     = 0x0004,                                        ///< enable row status (if row is valid, modified, deleted)
      eTableStateMAX          = 0x0010,                                        ///< max state value

      // ## size information used to calculate space needed by table
      eSpaceNull32Columns     = sizeof( uint32_t ),                            ///< space marking null columns
      eSpaceNull64Columns     = sizeof( uint64_t ),                            ///< space marking null columns
      eSpaceRowState          = sizeof( uint32_t ),                            ///< space where row state data is placed
      eSpaceRowGrowBy         = 10,                                            ///< default number of rows to grow by
      eSpaceFirstAllocate     = 10,                                            ///< number of rows to allocate before any values is added

   };


public:
   /**
    * \brief Information about each column in record
    *
    * `column` has information needed to work with data for each column in record.
    * Each column has a type, size, position (offset in buffer for row)
    * 
    * Each column can have different types, one type and one ctype.
    * The ctype is the C type used to to know how to handle the value in code.
    * The type is the native type used to store the value. Often type and ctype are the same.
    *
    * column also has name and alias, both are stored as offset position in buffer.
    * names m_namesColumn; in table_column_buffer is used to store names and aliases.
    *
    */
   struct column
   {
      column() { memset( this, 0, sizeof(column) ); }
      column( unsigned uCType ) { memset( this, 0, sizeof(column) ); m_uCType = uCType; }
      column( unsigned uCType, unsigned uSize ): m_uCType(uCType), m_uType(uCType), m_uState(0), m_uPosition(0), m_uSize(uSize), m_uPrimitiveSize( gd::types::value_size_g(uCType) ), m_uNameOffset(0), m_uAliasOffset(0) {}
      column( unsigned uCType, unsigned uType, unsigned uSize ): m_uCType(uCType), m_uType(uType), m_uState(0), m_uPosition(0), m_uSize(uSize), m_uPrimitiveSize( gd::types::value_size_g(uCType) ), m_uNameOffset(0), m_uAliasOffset(0) {}
      column( const column* pcolumn ) { memcpy( this, pcolumn, sizeof(column) ); }

      void state( unsigned uState ) { m_uState = uState; }                     ///< Set column state flags
      [[nodiscard]] unsigned state() const noexcept { return m_uState; }       ///< Get column state
      void type( unsigned uType ) { m_uType = uType; }                         ///< Set native type
      [[nodiscard]] unsigned type() const noexcept { return m_uType; }
      void ctype( unsigned uCType ) { m_uCType = uCType; }                     ///< Set C type
      [[nodiscard]] unsigned ctype() const noexcept { return m_uCType; }       ///< Get C type
      /// extract the number type part from ctype
      [[nodiscard]] unsigned ctype_number() const noexcept { return m_uCType & 0x0000'00ff; }
      [[nodiscard]] unsigned ctype_group() const noexcept { return m_uCType & 0x0000'ff00; }
      void position( unsigned uPosition ) { m_uPosition = uPosition; }
      [[nodiscard]] unsigned position() const noexcept { return m_uPosition; }
      void size( unsigned uSize ) { m_uSize = uSize; }
      [[nodiscard]] unsigned size() const noexcept { return m_uSize; }
      void primitive_size( unsigned uSize ) { m_uPrimitiveSize = uSize; }
      [[nodiscard]] unsigned primitive_size() const noexcept { return m_uPrimitiveSize; }
      [[nodiscard]] unsigned name() const { return m_uNameOffset; }
      [[nodiscard]] std::string_view name( const char* pbszBuffer ) const {
         auto p = &pbszBuffer[m_uNameOffset];
         return m_uNameOffset > 0 ? std::string_view(  p, (unsigned)*(uint16_t*)(p - sizeof(uint16_t)) ) : std::string_view(pbszNoName_g); 
      }
      void name( unsigned iOffset ) { m_uNameOffset = iOffset; }
      [[nodiscard]] unsigned alias() const { return m_uAliasOffset; }
      [[nodiscard]] std::string_view alias( const char* pbszBuffer ) const {
         auto p = &pbszBuffer[m_uAliasOffset];
         return m_uAliasOffset > 0 ? std::string_view(  p, (unsigned)*(uint16_t*)(p - sizeof(uint16_t)) ) : std::string_view(pbszNoName_g); 
      }
      void alias( unsigned iOffset ) { m_uAliasOffset = iOffset; }


      void data( uintptr_t uData ) { m_uData = uData; }
      [[nodiscard]] uintptr_t data() const noexcept { return m_uData; }


      // no size or reference value in buffer for value returns true, if size buffer (uint32_t) value is not fixed
      bool is_fixed() const noexcept { return (m_uState & (eColumnStateLength | eColumnStateReference)) == 0; }
      // if value holds value length as prefix in column buffer
      bool is_length() const noexcept { return (m_uState & eColumnStateLength); }
      // if column store value in as reference value
      bool is_reference() const noexcept { return (m_uState & eColumnStateReference); }

      unsigned m_uState;   ///< column state, like length, align
      unsigned m_uType;    ///< native value type
      unsigned m_uCType;   ///< c value type (lower byte has the number for type)
      unsigned m_uPosition;///< position where value starts
      unsigned m_uSize;    ///< max column size (also the internal buffer size), for fixed types this is 0
      unsigned m_uPrimitiveSize;///< size in bytes for each C++ primitive type or some special types like uuid
      unsigned m_uNameOffset;///< offset to location for name in buffer. offset can never be 0 because names always start with name length.
      unsigned m_uAliasOffset;///< offset to location for alias in buffer. offset can never be 0 because names always start with name length.
      uintptr_t m_uData;   ///< custom data, use this to get some specific external logic
   };

   struct column_name : public column
   {
      column_name( const std::string_view& stringName ) : m_stringName( stringName ), column() {}

      const std::string_view& name() const { return m_stringName; }

      std::string_view m_stringName;
   };

   /**
    * @brief iterator to move trough rows in table
    */
   struct iterator_row
   {
      iterator_row(): m_uRow(0), m_ptablecolumnbuffer(nullptr) {}
      iterator_row( uint64_t uRow, table_column_buffer* ptablecolumnbuffer ): m_uRow(uRow), m_ptablecolumnbuffer(ptablecolumnbuffer ) {}

      auto operator*() const { return gd::table::row<table_column_buffer>( m_ptablecolumnbuffer, m_uRow ); }
      operator uint64_t() const noexcept { return m_uRow; }
      uint64_t get_row() const noexcept { return m_uRow; }
      int64_t get_irow() const noexcept { return (int64_t)m_uRow; }

      bool operator==( const iterator_row& o ) const { assert( o.m_ptablecolumnbuffer == m_ptablecolumnbuffer ); return o.m_uRow == m_uRow; }
      bool operator!=( const iterator_row& o ) const { assert( o.m_ptablecolumnbuffer == m_ptablecolumnbuffer ); return o.m_uRow != m_uRow; }

      bool operator<(const iterator_row& o) const noexcept { assert(m_ptablecolumnbuffer == o.m_ptablecolumnbuffer && "Comparing iterators from different tables"); return m_uRow < o.m_uRow; }
      bool operator<=(const iterator_row& o) const noexcept { return !(o < *this); }
      bool operator>(const iterator_row& o) const noexcept { return o < *this; }
      bool operator>=(const iterator_row& o) const noexcept { return !(*this < o); }


      iterator_row& operator++() { m_uRow++; return *this; }
      iterator_row operator++(int) { iterator_row it_ = *this; ++(*this); return it_; }
      iterator_row& operator--() { m_uRow--; return *this; }
      iterator_row operator--(int) { iterator_row it_ = *this; --(*this); return it_; }

      auto operator+( std::ptrdiff_t iDistance ) { return iterator_row( (std::ptrdiff_t)m_uRow + iDistance, m_ptablecolumnbuffer ); }
      auto operator-( std::ptrdiff_t iDistance ) { return iterator_row( (std::ptrdiff_t)m_uRow - iDistance, m_ptablecolumnbuffer ); }

      std::vector< gd::variant_view > get_variant_view() const { return m_ptablecolumnbuffer->row_get_variant_view( m_uRow ); }
      std::vector< gd::variant_view > get_variant_view( const std::vector<unsigned>& vectorColumn ) const { return m_ptablecolumnbuffer->row_get_variant_view( m_uRow, vectorColumn ); }
      std::vector< gd::variant_view > get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const { return m_ptablecolumnbuffer->row_get_variant_view( m_uRow, puIndex, uSize ); }

      gd::variant_view cell_get_variant_view( unsigned uIndex ) const { return m_ptablecolumnbuffer->cell_get_variant_view( m_uRow, uIndex ); }
      gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const { return m_ptablecolumnbuffer->cell_get_variant_view( m_uRow, stringName ); }

      void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue ) { m_ptablecolumnbuffer->cell_set( m_uRow, uColumn, variantviewValue ); }
      void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue ) { m_ptablecolumnbuffer->cell_set( m_uRow, stringName, variantviewValue ); }
      void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert ) { m_ptablecolumnbuffer->cell_set( m_uRow, uColumn, variantviewValue, tag_convert{} ); }
      void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert ) { m_ptablecolumnbuffer->cell_set( m_uRow, stringName, variantviewValue, tag_convert{} ); }

      uint64_t m_uRow;     ///< active row index 
      table_column_buffer* m_ptablecolumnbuffer; ///< pointer to table that owns the iterator
   };

   struct const_iterator_row
   {
      const_iterator_row(): m_uRow(0), m_ptablecolumnbuffer(nullptr) {}
      const_iterator_row( uint64_t uRow, const table_column_buffer* ptablecolumnbuffer ): m_uRow(uRow), m_ptablecolumnbuffer(ptablecolumnbuffer ) {}
      const_iterator_row( int64_t iRow, table_column_buffer* ptablecolumnbuffer ): m_uRow((uint64_t)iRow), m_ptablecolumnbuffer(ptablecolumnbuffer ) {}

      bool operator==( const const_iterator_row& o ) const { assert( o.m_ptablecolumnbuffer == m_ptablecolumnbuffer ); return o.m_uRow == m_uRow; }
      bool operator!=( const const_iterator_row& o ) const { assert( o.m_ptablecolumnbuffer == m_ptablecolumnbuffer ); return o.m_uRow != m_uRow; }

      bool operator<(const const_iterator_row& o) const noexcept { assert(m_ptablecolumnbuffer == o.m_ptablecolumnbuffer && "Comparing iterators from different tables"); return m_uRow < o.m_uRow; }
      bool operator<=(const const_iterator_row& o) const noexcept { return !(o < *this); }
      bool operator>(const const_iterator_row& o) const noexcept { return o < *this; }
      bool operator>=(const const_iterator_row& o) const noexcept { return !(*this < o); }

      operator uint64_t() const  noexcept  { return m_uRow; }
      uint64_t get_row() const noexcept { return m_uRow; }
      int64_t get_irow() const noexcept { return (int64_t)m_uRow; }

      const_iterator_row& operator++() { m_uRow++; return *this; }
      const_iterator_row operator++(int) { const_iterator_row it_ = *this; ++(*this); return it_; }
      const_iterator_row& operator--() { m_uRow--; return *this; }
      const_iterator_row operator--(int) { const_iterator_row it_ = *this; --(*this); return it_; }

      uint64_t m_uRow;
      const table_column_buffer* m_ptablecolumnbuffer;
   };


public:
   // ## stl container aliases, simplify for templates using table data

   using column_value_type = column;
   using column_const_value_type = const column;
   using column_iterator = std::vector<column>::iterator;
   using column_const_iterator = std::vector<column>::const_iterator;
   typedef std::random_access_iterator_tag iterator_category;

   using row_value_type = std::vector<gd::table::cell<table_column_buffer> >;  ///< used to simplify stl and containers working with table rows
   using row_const_value_type = const std::vector<gd::table::cell<table_column_buffer> >;///< used to simplify stl and containers working with table rows
   using row_iterator = iterator_row;
   using row_const_iterator = const_iterator_row;
   using row_difference_type = std::ptrdiff_t;

   using value_type = row_value_type;
   using const_value_type = row_const_value_type;
   using iterator = iterator_row;
   using const_iterator = const_iterator_row;
   using difference_type = row_difference_type;


// ## construction -------------------------------------------------------------
// 0TAG0construct.table_column_buffer
public:
   /// @name construction
   /// Constructs table_column_buffer objects. 
   /// - `uRowCount` number of rows that are pre allocated when table is prepared
   /// - `uFlags` flags in enum above (eTableFlagNull32 = manage nulls for max 32 columns, eTableFlagNull64 = manage nulls for max 64 columns, eTableFlagRowStatus = reserve space to mark different row states like deleted etc)
   /// - `uGrowBy` how many rows table should grow by if it needs to increase its size.
   /// - `variantviewValue` create table with one value in constructor
   /// - `tag_prepare` prepare complete table in constructor, no need to call any more methods to start using table logic
   ///@{
   table_column_buffer(): m_uFlags(0), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(0), m_uRowGrowBy(0) {}
   table_column_buffer( unsigned uRowCount ): m_uFlags(0), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(uRowCount), m_uRowGrowBy(0) {}
   table_column_buffer( unsigned uRowCount, unsigned uFlags ): m_uFlags(uFlags), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(uRowCount), m_uRowGrowBy(0) { assert( m_uFlags < eTableStateMAX );  }
   table_column_buffer( unsigned uRowCount, unsigned uFlags, unsigned uGrowBy ): m_uFlags(uFlags), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(uRowCount), m_uRowGrowBy(uGrowBy) { assert( m_uFlags < eTableStateMAX );  }
   table_column_buffer( tag_null ): m_uFlags(eTableFlagNull64), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(0) { assert( m_uFlags < eTableStateMAX );  }
   table_column_buffer( tag_full_meta ): m_uFlags(eTableFlagNull64|eTableFlagRowStatus), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(0) { assert( m_uFlags < eTableStateMAX );  }
   table_column_buffer( unsigned uRowCount, tag_null ): m_uFlags(eTableFlagNull64), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(uRowCount) { assert( m_uFlags < eTableStateMAX );  }
   table_column_buffer( unsigned uRowCount, tag_full_meta ): m_uFlags(eTableFlagNull64|eTableFlagRowStatus), m_uRowSize(0), m_uRowCount(0), m_uReservedRowCount(uRowCount) { assert( m_uFlags < eTableStateMAX );  }

   table_column_buffer( unsigned uFlags, const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorValue );

   table_column_buffer( const gd::variant_view& variantviewValue, tag_prepare );
   table_column_buffer( const std::vector< std::string_view >& vectorValue, tag_prepare );
   table_column_buffer( unsigned uFlags, const std::vector< std::tuple< std::string_view, std::string_view > >& vectorValue, tag_prepare );
   table_column_buffer( unsigned uFlags, const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorValue, tag_prepare );
   table_column_buffer( const std::vector< std::tuple< std::string_view, unsigned, std::string_view, gd::variant_view > >& vectorValue, tag_prepare );

   table_column_buffer( const std::string_view& stringColumns, tag_parse, tag_prepare );
   table_column_buffer( unsigned uFlags, const std::string_view& stringColumns, tag_parse, tag_prepare );
   
// copy
   table_column_buffer( const table_column_buffer& o ): m_puData(nullptr) { common_construct( o ); }
   table_column_buffer( const table_column_buffer& o, tag_columns ): m_puData(nullptr) { common_construct( o, tag_columns{}); }
   table_column_buffer( table_column_buffer&& o ) noexcept : m_puData(nullptr) { common_construct( std::move( o ) ); }
   table_column_buffer( const table_column_buffer& o, uint64_t uFrom, uint64_t uCount );
   table_column_buffer( const table_column_buffer& o, const std::vector<uint64_t> vectorRow );
   table_column_buffer( const table_column_buffer& o, const range& rangeCopy );
   table_column_buffer( const table_column_buffer& o, const page& pageCopy );
// assign
   table_column_buffer& operator=( const table_column_buffer& o ) { clear(); common_construct( o ); return *this; }
   table_column_buffer& operator=( table_column_buffer&& o ) noexcept { clear(); common_construct( std::move( o ) ); return *this; }

   ~table_column_buffer() 
   { 
      delete[] m_puData;
   }
   ///@}

private:
// common copy
   void common_construct( const table_column_buffer& o );
   void common_construct( const table_column_buffer& o, tag_columns );
   void common_construct( const table_column_buffer& o, const std::vector<unsigned>& vectorColumn, tag_columns );
   void common_construct( table_column_buffer&& o ) noexcept;

// ## operator -----------------------------------------------------------------
public:
   std::vector<gd::variant_view> operator[]( uint64_t uRow ) const { return row_get_variant_view( uRow ); }

   gd::variant_view operator[]( const std::pair<unsigned, unsigned>& pairCell ) const { return cell_get_variant_view( pairCell.first, pairCell.second ); }
   gd::variant_view operator[]( const std::pair<unsigned, const std::string_view>& pairCell ) const { return cell_get_variant_view( pairCell.first, pairCell.second ); }

   table_column_buffer& operator+=( const table_column_buffer& o ) { append( o ); return *this; }

   gd::variant_view operator()( uint64_t uRow, unsigned uColumn ) const { return cell_get_variant_view( uRow, uColumn ); }
   gd::variant_view operator()( uint64_t uRow, const std::string_view& stringName ) const { return cell_get_variant_view( uRow, stringName ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   unsigned get_flags() const noexcept { return m_uFlags; }
   void set_state [[deprecated]] ( uint32_t uFlags ) noexcept { m_uFlags = uFlags; }
   void set_flags( uint32_t uFlags ) noexcept { m_uFlags = uFlags; }
   void set_state [[deprecated]] ( tag_full_meta ) noexcept { m_uFlags = eTableFlagRowStatus|eTableFlagNull64; }
   void set_flags( tag_full_meta ) noexcept { m_uFlags = eTableFlagRowStatus|eTableFlagNull64; }
   void set_state [[deprecated]] ( uint32_t uSet, uint32_t uClear ) noexcept { m_uFlags |= uSet; m_uFlags &= ~uSet; }
   void set_flags( uint32_t uSet, uint32_t uClear ) noexcept { m_uFlags |= uSet; m_uFlags &= ~uSet; }
   unsigned get_column_count() const noexcept { return (unsigned)m_vectorColumn.size(); }
   /// Get number of rows with values
   uint64_t get_row_count() const noexcept { return m_uRowCount; }
   /// Number of rows memory is allocated for
   uint64_t get_reserved_row_count() const noexcept { return m_uReservedRowCount; }
   /// get allocated size in bytes for table
   uint64_t get_reserved_size() const noexcept { return m_uReservedRowCount; }
   uint64_t get_row_count( uint32_t uFlags ) const noexcept;
   /// Last valid row index where to insert cell values
   uint64_t get_row_back() const noexcept { assert( m_puData != nullptr ); return m_uRowCount - 1; }
   void set_row_count( uint64_t uCount ) { assert( uCount <= m_uReservedRowCount ); m_uRowCount = uCount; }
   void set_reserved_row_count( uint64_t uCount ) { assert( uCount >= m_uRowCount ); m_uReservedRowCount = uCount; }

   // ## state methods, check state flags

   bool is_null() const { return m_uFlags & (eTableFlagNull32|eTableFlagNull64); }
   bool is_null32() const { return m_uFlags & eTableFlagNull32; }
   bool is_null64() const { return m_uFlags & eTableFlagNull64; }
   bool is_rowstatus() const { return m_uFlags & eTableFlagRowStatus; }
   bool is_rowmeta() const { return m_puMetaData != nullptr; }

   unsigned size_row() const noexcept { return m_uRowSize; }
   unsigned size_row_meta() const noexcept;
   /// get meta block size
   uint64_t size_meta_total() const noexcept { return size_row_meta() * m_uReservedRowCount; }
   /// get meta block size for rows
   uint64_t size_meta_total( uint64_t uRowCount ) const noexcept { return size_row_meta() * uRowCount; }
   /// calc and return total allocated memory size
   uint64_t size_reserved_total() const noexcept { return (m_uRowSize + size_row_meta()) * m_uReservedRowCount; }
   /// calc and return total allocated memory size for rows
   uint64_t size_reserved_total( uint64_t uRowCount ) const noexcept { return (m_uRowSize + size_row_meta()) * uRowCount; }

   const names& get_names() const noexcept { return m_namesColumn; }

//@}

/** \name OPERATION
*///@{

   // ## column methods

   /// @name column_add
   /// Add columns to table, this is typically done before adding values to table. Remember to call @see prepare before adding data
   /// Parameters:
   /// - `uColumnType` type of column to add @see: gd::types::enumType
   /// - `stringType` type of columns as string name, will be converted to the type number
   /// - `uSize` if type do not have a fixed size then size will have the maximum length for text
   /// - `columnToAdd` has all column properties for column to add
   /// - `stringName` name for column
   /// - `stringAlias` alias name for column (column can have both name and alias)
   /// 0TAG0add.table_column_buffer
   ///@{
   table_column_buffer& column_add( const column& columnToAdd ) { m_vectorColumn.push_back( columnToAdd ); return *this; }
   table_column_buffer& column_add( const column& columnToAdd, const table_column_buffer& tableFrom );
   table_column_buffer& column_add( unsigned uColumnType, const std::string_view& stringName ) { return column_add( uColumnType, 0, stringName ); }
   table_column_buffer& column_add( unsigned uColumnType, unsigned uSize );
   table_column_buffer& column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias );
   table_column_buffer& column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName ) { return column_add( uColumnType, uSize, stringName, std::string_view{} ); }
   table_column_buffer& column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringAlias, tag_alias ) { return column_add( uColumnType, uSize, std::string_view{}, stringAlias ); }
   table_column_buffer& column_add( const std::vector< std::tuple< unsigned, unsigned, std::string_view > >& vectorColumn );
   table_column_buffer& column_add( const std::string_view& stringType ) { return column_add( column( (unsigned)gd::types::type_g( stringType ) ) ); }
   table_column_buffer& column_add( const std::string_view& stringType, const std::string_view& stringName ) { return column_add( (unsigned)gd::types::type_g( stringType ), 0, stringName, std::string_view{}); }
   table_column_buffer& column_add( const std::string_view& stringType, unsigned uSize ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize ); }
   table_column_buffer& column_add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringName ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize, stringName, std::string_view{}); }
   table_column_buffer& column_add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringAlias, tag_alias ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize, std::string_view{}, stringAlias); }
   table_column_buffer& column_add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias ) { return column_add( (unsigned)gd::types::type_g( stringType ), uSize, stringName, stringAlias); }
   table_column_buffer& column_add( const std::string_view& stringName, tag_measurement);

   table_column_buffer& column_add( const std::vector< std::pair< std::string_view, unsigned > >& vectorType, tag_type_name );
   table_column_buffer& column_add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorType, tag_type_name );
   table_column_buffer& column_add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view, std::string_view > >& vectorType, tag_type_name );
   table_column_buffer& column_add( const std::initializer_list< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name );
   table_column_buffer& column_add( const std::vector< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name );
   table_column_buffer& column_add( const std::vector< std::pair< unsigned, unsigned > >& vectorType, tag_type_constant );
   table_column_buffer& column_add( const std::string_view& stringNameStart, const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorType, tag_type_name );
   table_column_buffer& column_add( const table_column_buffer* p_ );
   
   table_column_buffer& column_add( const std::vector< std::tuple< std::string, unsigned, std::string > >& vectorType, tag_type_name );

   std::pair<bool, std::string> column_add( const std::string_view& stringColumns, tag_parse );
   table_column_buffer& column_add( const argument::column& columnAdd ) { return column_add( columnAdd.type(), columnAdd.size(), columnAdd.name(), columnAdd.alias() ); }

   template< typename CLASS >
   table_column_buffer& column_add() { return column_add( CLASS::to_columns(), tag_type_name{} ); }
   template< typename CLASS >
   table_column_buffer& column_add( const std::string_view& stringName ) { return column_add( CLASS::to_columns( stringName ), tag_type_name{} ); }

   // ## Change the table structure after prepare, these methods do a lot of work

   table_column_buffer& column_add( const column& columnToAdd, tag_prepare );
   table_column_buffer& column_add(unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias, tag_prepare);
   table_column_buffer& column_add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorType, tag_type_name, tag_prepare );

   ///@}

   // ### access column or find index for column/columns
   // 0TAG0find_column.table_column_buffer
   /// @brief find column index for column name
   int column_find_index( const std::string_view& stringName ) const noexcept;
   /// @brief find column index for column alias
   int column_find_index( const std::string_view& stringAlias, tag_alias ) const noexcept;
   /// @brief find column index for column name with wildcard, wildcars like ? and * are supported
   int column_find_index( const std::string_view& stringWildcard, tag_wildcard ) const noexcept;
   /// @brief get column index for column name, asserts if not found
   unsigned column_get_index( const std::string_view& stringName ) const noexcept;
   /// @brief get column index for column alias, asserts if not found
   unsigned column_get_index( const std::string_view& stringAlias, tag_alias ) const noexcept;
   /// @brief get column index for column name with wildcard, wildcars like ? and * are supported, asserts if not found
   unsigned column_get_index( const std::string_view& stringName, tag_wildcard ) const noexcept;
   /// @brief get column indexes for column names
   std::vector<uint32_t> column_get_index( std::initializer_list< std::string_view > listName ) const noexcept;
   std::vector<uint32_t> column_get_index( const std::vector< std::string_view >& vectorName ) const noexcept;
   unsigned column_get_index_for_alias( const std::string_view& stringAlias ) const noexcept { return column_get_index( stringAlias, tag_alias{}); }
   unsigned column_get_type( unsigned uIndex ) const { assert( uIndex < get_column_count() ); return m_vectorColumn.at(uIndex).type(); }
   std::vector<unsigned> column_get_type( const std::vector<unsigned>& vectorIndex ) const;
   std::vector<unsigned> column_get_type() const;
   unsigned column_get_ctype( unsigned uIndex ) const { assert( uIndex < get_column_count() ); return m_vectorColumn.at(uIndex).ctype(); }
   std::vector<unsigned> column_get_ctype() const;
   unsigned column_get_ctype_number( unsigned uIndex ) const { assert( uIndex < get_column_count() ); return m_vectorColumn.at(uIndex).ctype_number(); }
   unsigned column_get_size( unsigned uIndex ) const { assert( uIndex < get_column_count() ); return m_vectorColumn.at(uIndex).size(); }
   void column_set_size( unsigned uIndex, unsigned uSize ) { assert( uIndex < get_column_count() ); m_vectorColumn.at(uIndex).size( uSize ); }
   void column_set_size( const std::string_view& stringName, unsigned uSize ) { column_set_size( column_get_index( stringName ), uSize ); }
   unsigned column_get_primitive_size( unsigned uIndex ) const noexcept { assert( uIndex < get_column_count() ); return m_vectorColumn.at(uIndex).primitive_size(); }
   std::string_view column_get_name( unsigned uIndex ) const;
   std::string_view column_get_name( const column& column ) const;
   std::vector<std::string_view> column_get_name() const;
   std::vector<std::string_view> column_get_name( const std::vector<unsigned>& vectorColumn ) const;
   std::string_view column_get_alias( unsigned uIndex ) const;
   std::string_view column_get_alias( const column& column ) const;
   /// Rename column
   std::string_view column_rename( unsigned uColumn, const std::string_view& stringNewName );

   void column_for_each( std::function<void( column&, unsigned )> callback_ );
   void column_for_each( std::function<void( const column&, unsigned )> callback_ ) const;

   // ## fill methods - set 0 or more values in table

   /// @name column_fill
   /// fill specified column with value
   ///@{
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue ) { column_fill(  uColumn, variantviewValue, 0, get_row_count() ); }
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert ) { column_fill(  uColumn, variantviewValue, 0, get_row_count(), tag_convert{}); }
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uBeginRow, uint64_t uEndRow );
   void column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uBeginRow, uint64_t uEndRow, tag_convert );
   void column_fill( unsigned uColumn, const gd::variant_view* pvariantviewValue, size_t uCount, uint64_t uBeginRow );
   void column_fill( unsigned uColumn, const std::vector< gd::variant_view >& vectorValue) { column_fill( uColumn, vectorValue.data(), vectorValue.size(), 0 ); }
   void column_fill( unsigned uColumn, const std::vector< gd::variant_view >& vectorValue, uint64_t uBeginRow ) { column_fill( uColumn, vectorValue.data(), vectorValue.size(), uBeginRow ); }
   ///@}

   template< typename... Arguments >
   void column_fill( const std::string_view& stringName, const gd::variant_view& variantviewValue, Arguments&&... arguments );
   template< typename... Arguments >
   void column_fill( const std::string_view& stringName, const std::vector< gd::variant_view >& vectorValue, Arguments&&... arguments );

   std::vector<column>::iterator column_begin() { return m_vectorColumn.begin(); }
   std::vector<column>::iterator column_end() { return m_vectorColumn.end(); }
   std::vector<column>::const_iterator column_begin() const { return m_vectorColumn.begin(); }
   std::vector<column>::const_iterator column_end() const { return m_vectorColumn.end(); }
   std::vector<column>::const_iterator column_cbegin() const { return m_vectorColumn.cbegin(); }
   std::vector<column>::const_iterator column_cend() const { return m_vectorColumn.cend(); }

   column column_get( std::size_t uIndex ) { return m_vectorColumn[uIndex]; }
   const column&  column_get( std::size_t uIndex ) const { return m_vectorColumn[uIndex]; }
   column* column_get( std::size_t uIndex, tag_pointer ) { return &m_vectorColumn[uIndex]; }
   const column* column_get( std::size_t uIndex, tag_pointer ) const { return &m_vectorColumn[uIndex]; }
   /// read column information for column at index, `argument::column` is used to transfer column data
   void column_get( std::size_t uIndex, argument::column& column_ ) const;

   std::size_t column_size() const { return m_vectorColumn.size(); }
   bool column_empty() const { return m_vectorColumn.empty(); }

   bool column_exists( const std::string_view& stringName ) const noexcept;
   bool column_exists( const std::string_view& stringAlias, tag_alias ) const noexcept;

   /// return collection object wrapping columns
   gd::table::columns<table_column_buffer> columns() { return gd::table::columns<gd::table::table_column_buffer>( this ); }

   /// generate columns information from internal columns in table
   void to_columns( gd::table::detail::columns& columns ) const;
   void to_table( gd::table::table& table ) const;


   /// Prepares table for use, this has to be called before adding values to table
   std::pair<bool, std::string> prepare();


   // ## row methods, row related functionality 

   void row_set_state( uint64_t uRow, unsigned uFlags ) { assert( uRow < m_uReservedRowCount ); *row_get_state( uRow ) = uFlags; }
   void row_set_state( uint64_t uRow, unsigned uSet, unsigned uClear ); 
   uint8_t* row_get( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); return m_puData + uRow * m_uRowSize; }
   uint8_t* row_get_meta( uint64_t uRow ) const noexcept { return row_get_null( uRow ); }
   /// return pointer to section holding null column information
   uint8_t* row_get_null( uint64_t uRow ) const noexcept;
   /// Get pointer to row state part
   uint32_t* row_get_state( uint64_t uRow ) const noexcept;
   /// if row is in used (when state information is used for row)
   bool row_is_use( uint64_t uRow ) const noexcept;
   /// Get pointer to row part used to mark null columns
   uint64_t* row_get_null_columns( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); return reinterpret_cast<uint64_t*>(m_puData + uRow * m_uRowSize); }


   // ### edit rows (add or remove)

   void row_add( uint64_t uCount );
   void row_add() { row_add( 1 ); }
   void row_add( uint64_t uCount, tag_null );
   void row_add(tag_null) { row_add( 1, tag_null{} ); }
   /// Simple add one row to table that is safe (if table have null values these are automatically set to null)
   uint64_t row_add_one();

   /// @name row_add
   /// add row/rows to table and insert values to added row
   ///@{
   void row_add( const std::initializer_list<gd::variant_view>& vectorValue );
   void row_add( const std::initializer_list<gd::variant_view>& vectorValue, tag_convert );
   void row_add( const std::vector<gd::variant_view>& vectorValue );
   void row_add( const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn );
   void row_add( const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn, tag_convert );
   void row_add( const std::vector<gd::variant_view>& vectorValue, tag_convert );
   void row_add( unsigned uFirstColumn, const std::vector<gd::variant_view>& vectorValue, tag_convert );
   void row_add( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue );
   void row_add( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue, tag_convert );
   void row_add( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue );
   void row_add( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert );
   void row_add( const gd::argument::arguments& argumentsRow, tag_arguments );
   void row_add( uint64_t uRowToCopy, tag_copy );
   void row_add( const std::string_view& stringRowValue, char chSplit, tag_parse );

   template< typename OBJECT >
   void row_add( unsigned uFirstColumn, const OBJECT& object_ ) {
      std::vector< gd::variant_view > vectorObject;
      object_.to_values( vectorObject );
      row_add( uFirstColumn, vectorObject, tag_convert{});
   }


   ///@}

/// @name row_set
/// set values in row
///@{
   void row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue );
   void row_set( uint64_t uRow, unsigned uSart, const std::initializer_list<gd::variant_view>& listValue );
   void row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, unsigned uSart, const std::initializer_list<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue );
   void row_set( uint64_t uRow, unsigned uOffset, const std::vector<gd::variant_view>& listValue);
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, unsigned uSart, const std::vector<gd::variant_view>& listValue, tag_convert );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue, const std::vector<unsigned>& vectorColumn );
   void row_set( uint64_t uRow, const std::vector<gd::variant_view>& listValue, const std::vector<unsigned>& vectorColumn, tag_convert );
   void row_set( uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue );
   void row_set( uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue, tag_convert );
   void row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue );
   void row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert );
   void row_set( uint64_t uRow, const gd::argument::arguments& argumentsRow, tag_arguments );
   void row_set( uint64_t uRow, uint64_t uRowToCopy );
   void row_set( uint64_t uRow, const std::string_view& stringRowValue, char chSplit, tag_parse );
   void row_set_null( uint64_t uRow );
   void row_set_null( uint64_t uFrom, uint64_t uCount );
   void row_set_range( uint64_t uRow, const gd::variant_view variantviewSet, tag_convert ) { row_set_range( uRow, 0, get_column_count(), variantviewSet, tag_convert{}); }
   void row_set_range( uint64_t uRow, unsigned uStartColumn, unsigned uCount, const gd::variant_view variantviewSet, tag_convert );

   // ### support for external objects
   template< typename OBJECT >
   void row_set( uint64_t uRow, unsigned uFirstColumn, const OBJECT& object_ );
   template< typename OBJECT >
   void row_set( uint64_t uRow, const std::string_view& stringFind, const OBJECT& object_ );

///@}

   /// @name row_clear
   /// clears all rows in table
   ///@{
   /// Clears all rows in table (just set the row count to 0)
   void row_clear() { m_uRowCount = 0; }
   ///@}

    /// @name row_delete
   /// deletes last row in table
   ///@{
   /// Deletes last row in table (by decreasing the row count)
   void row_delete() { if(m_uRowCount > 0) m_uRowCount--; }
   ///@}

   /// @name row_reserve_add
   /// reserve memory to store more rows in table
   ///@{
   void row_reserve_add( uint64_t uCount );
   void row_reserve_add() { row_reserve_add( 1 ); }
   ///@}

   row_value_type row_get( uint64_t uRow, tag_cell );
   // row_const_value_type row_get( uint64_t uRow, tag_cell ) const;

   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow ) const;
   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow, unsigned uFirstColumn, unsigned uCount ) const;
   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow, unsigned uFirstColumn ) const { return row_get_variant_view( uRow, uFirstColumn, get_column_count() - uFirstColumn ); }
   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const;
   std::vector<gd::variant_view> row_get_variant_view( uint64_t uRow, const std::vector<unsigned>& vectorIndex ) const { return row_get_variant_view( uRow, vectorIndex.data(), (unsigned)vectorIndex.size() ); }
   void row_get_variant_view( uint64_t uRow, std::vector<gd::variant_view>& vectorValue ) const;
   void row_get_variant_view( uint64_t uRow, unsigned uOffset, std::vector<gd::variant_view>& vectorValue ) const;
   void row_get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize, std::vector<gd::variant_view>& vectorValue ) const;
   void row_get_variant_view( uint64_t uRow, const std::vector<unsigned>& vectorIndex, std::vector<gd::variant_view>& vectorValue ) const { row_get_variant_view( uRow, vectorIndex.data(), (unsigned)vectorIndex.size(), vectorValue ); }

   int64_t row_get_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind, std::vector<gd::variant_view>& vectorValue ) const;

   /// @name get values in row packed in arguments object
   /// reserve memory to store more rows in table
   ///@{
   void row_get_arguments( uint64_t uRow, gd::argument::arguments& argumentsValue ) const;
   gd::argument::arguments row_get_arguments( uint64_t uRow ) const { gd::argument::arguments a_; row_get_arguments( uRow, a_ ); return a_; }
   gd::argument::arguments row_get_arguments( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const;
   gd::argument::arguments row_get_arguments( uint64_t uRow, const std::vector<unsigned>& vectorIndex ) const { return row_get_arguments( uRow, vectorIndex.data(), (unsigned)vectorIndex.size() ); }   
   ///@}

   bool row_for_each( std::function<bool( std::vector<gd::variant_view>&, uint64_t )> callback_ );
   bool row_for_each( std::function<bool( const std::vector<gd::variant_view>&, uint64_t )> callback_ ) const;
   bool row_for_each( uint64_t uFrom, uint64_t uCount, std::function<bool( std::vector<gd::variant_view>&, uint64_t )> callback_ );
   bool row_for_each( uint64_t uFrom, uint64_t uCount, std::function<bool( const std::vector<gd::variant_view>&, uint64_t )> callback_ ) const;
   bool row_for_each( unsigned uColumn, std::function<bool( const gd::variant_view&, uint64_t )> callback_ ) const { return row_for_each( uColumn, 0, get_row_count(), callback_ ); }
   bool row_for_each( unsigned uColumn, uint64_t uFrom, uint64_t uCount, std::function<bool( const gd::variant_view&, uint64_t )> callback_ ) const;

   iterator_row row_begin() { return iterator_row( (uint64_t)0, this ); }
   iterator_row row_end() { return iterator_row( get_row_count(), this); }
   const_iterator_row row_begin() const { return const_iterator_row( (uint64_t)00, this); }
   const_iterator_row row_end() const { return const_iterator_row( get_row_count(), this); }
   const_iterator_row row_cbegin() const { return const_iterator_row( (uint64_t)00, this); }
   const_iterator_row row_cend() const { return const_iterator_row( get_row_count(), this); }

   /// get row index based on row status 
   int64_t row_get_absolute( uint64_t uRelativeRow, unsigned uStatus ) const;

   iterator begin() { return row_begin(); }
   iterator end() { return row_end(); }
   const_iterator begin() const { return row_begin(); }
   const_iterator end() const { return row_end(); }
   const_iterator cbegin() const { return row_cbegin(); }
   const_iterator cend() const { return row_cend(); }

   /// return collection object wrapping rows
   gd::table::rows<table_column_buffer> rows() { return gd::table::rows<gd::table::table_column_buffer>( this ); }

   // ## cell methods, cell related functionality  0TAG0cell.table_column_buffer

   uint64_t cell_offset( uint64_t uRow, unsigned uColumn ) noexcept;
   uint64_t cell_offset(uint64_t uRow, const std::string_view& stringName) noexcept;

   uint8_t* cell_get( uint64_t uRow, unsigned uColumn ) noexcept;
   const uint8_t* cell_get( uint64_t uRow, unsigned uColumn ) const noexcept;
   uint8_t* cell_get( uint64_t uRow, const std::string_view& stringName ) noexcept;
   uint8_t* cell_get( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) noexcept;
   uint8_t* cell_get( uint64_t uRow, const std::string_view& stringWildcard, tag_wildcard ) noexcept;
   const uint8_t* cell_get( uint64_t uRow, const std::string_view& stringName ) const noexcept;
   const uint8_t* cell_get( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) const noexcept;

   template <typename TYPE>
   TYPE cell_get( uint64_t uRow, unsigned uColumn ) const noexcept;

   bool cell_is_null( uint64_t uRow, unsigned uColumn ) const noexcept;
   const reference* cell_get_reference( uint64_t uRow, unsigned uColumn ) const noexcept;

   gd::variant_view cell_get_variant_view( uint64_t uRow, unsigned uColumn ) const noexcept;
   std::vector< gd::variant_view > cell_get_variant_view( uint64_t uRow, unsigned uFromColumn, unsigned uToColumn ) const;
   std::vector< gd::variant_view > cell_get_variant_view( uint64_t uRow ) const { return cell_get_variant_view( uRow, 0, get_column_count() ); }
   gd::variant_view cell_get_variant_view( uint64_t uRow, const std::string_view& stringName ) const noexcept;
   gd::variant_view cell_get_variant_view( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) const noexcept;
   gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const noexcept { assert(m_uRowCount != 0); return cell_get_variant_view( m_uRowCount -1, stringName ); }
   gd::variant_view cell_get_variant_view( const std::string_view& stringAlias, tag_alias ) const noexcept { assert(m_uRowCount != 0); return cell_get_variant_view( m_uRowCount -1, stringAlias, tag_alias{}); }
   /// return value without any checks, use this if you know about the internals and just need the value
   gd::variant_view cell_get_variant_view( uint64_t uRow, unsigned uColumn, tag_raw ) const noexcept;
   /// get cell value using name or column index, if name then column gets index to speed up the process next time value is returned
   gd::variant_view cell_get_variant_view( uint64_t uRow, std::variant< unsigned, std::string_view >* pvariantColumn ) const noexcept;



   unsigned cell_get_length( uint64_t uRow, unsigned uColumn ) const noexcept;

   void cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue );
   void cell_set( uint64_t uRow, const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_alias );
   void cell_set_null( uint64_t uRow, unsigned uColumn );
   void cell_set_null( uint64_t uRow, const std::string_view& stringName );
   void cell_set_not_null( uint64_t uRow, unsigned uColumn );
   void cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert );
   void cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue, tag_adjust );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_adjust );
   void cell_set( uint64_t uRow, const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_convert, tag_alias );
   void cell_set( uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue );
   void cell_set( uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue, tag_convert );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const std::vector<gd::variant_view>& vectorValue );
   void cell_set( uint64_t uRow, const std::string_view& stringName, const std::vector<gd::variant_view>& vectorValue, tag_convert );

   void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue ) { assert(m_uRowCount != 0); cell_set( m_uRowCount - 1, uColumn, variantviewValue ); }
   void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue ) { assert(m_uRowCount != 0); cell_set( m_uRowCount - 1, stringName, variantviewValue ); }
   void cell_set( const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_alias ) { assert(m_uRowCount != 0); cell_set( m_uRowCount - 1, stringAlias, variantviewValue, tag_alias{}); }

   void cell_set( const range& rangeSet, const gd::variant_view& variantviewValue );
   void cell_set( const range& rangeSet, const gd::variant_view& variantviewValue, tag_convert );

   // ## find methods 0TAG0find.table_column_buffer

   int64_t find( unsigned uColumn, const gd::variant_view& variantviewFind ) const noexcept { return find( uColumn, 0, get_row_count(), variantviewFind ); }
   int64_t find( const std::string_view& stringName, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( stringName, 0, get_row_count(), variantviewFind ); }
   int64_t find( unsigned uColumn, bool bAscending, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( uColumn, bAscending, 0, get_row_count(), variantviewFind ); }
   int64_t find( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;

   int64_t find_variant_view( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   int64_t find_variant_view( const std::string_view& stringName, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind, tag_meta ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, bool bAscending, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept;
   range find_variant_view( unsigned uColumn, bool bAscending, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind, tag_range ) const noexcept;
   int64_t find_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( uColumn, 0, get_row_count(), variantviewFind); }
   int64_t find_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind, tag_meta ) const noexcept { return find_variant_view( uColumn, 0, get_row_count(), variantviewFind, tag_meta{}); }
   int64_t find_variant_view( const std::string_view& stringName, const gd::variant_view& variantviewFind ) const noexcept { return find_variant_view( column_get_index( stringName ), 0, get_row_count(), variantviewFind); }
   range find_variant_view( unsigned uColumn, bool bAscending, const gd::variant_view& variantviewFind, tag_range ) const noexcept { return find_variant_view( uColumn, bAscending, 0, get_row_count(), variantviewFind, tag_range{}); }

   
   int64_t find( uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFind ) const noexcept;
   int64_t find( uint64_t uStartRow, uint64_t uCount, const std::vector<gd::variant_view>& vectorFind ) const;
   int64_t find( const std::vector<gd::variant_view>& vectorFind ) const { return find( 0, get_row_count(), vectorFind ); }
   int64_t find( uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorFind ) const;
   int64_t find( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorFind ) const { return find( 0, get_row_count(), vectorFind ); }

   /// Find first row marked as free (flag `eRowStateUse` is not used)
   int64_t find_first_free_row( uint64_t uStartRow ) const;
   int64_t find_first_free_row() const { return find_first_free_row( 0 ); }

   /// counts rows in use
   uint64_t count_used_rows() const;
   /// count number of free rows
   uint64_t count_free_rows() const;


   // ## property methods for table - set or get property values for table and other property methods

   template<typename TYPE>
   void property_set( const std::string_view& stringName, TYPE value_ ) { m_argumentsProperty.set( stringName, value_ ); }
   void property_set( const std::pair< std::string, gd::variant_view>& pair_ ) { m_argumentsProperty.set( pair_.first, pair_.second ); }
   gd::argument::arguments::argument property_get( const std::string_view& stringName ) const { return m_argumentsProperty.get_argument( stringName ); }
   bool property_exists( const std::string_view& stringName ) const { return (m_argumentsProperty.find( stringName ) != nullptr); }

   /// @name append
   /// append row data from another table into this table
   ///@{
   void append( const table_column_buffer& tableFrom );
   void append( const table_column_buffer& tableFrom, tag_convert );
   void append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom );
   void append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, tag_convert );
   void append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo );
   void append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo, tag_convert );
   void append( const table_column_buffer& tableFrom, const unsigned* puColumnIndexFrom, const unsigned* puColumnIndexTo, unsigned uColumnCount );
   void append( const table_column_buffer& tableFrom, const unsigned* puColumnIndexFrom, const unsigned* puColumnIndexTo, unsigned uColumnCount, tag_convert );
   void append( const table_column_buffer& tableFrom, tag_name );
   void append( const table_column_buffer& tableFrom, tag_name, tag_convert );
   void append( const table_column_buffer& tableFrom, uint64_t uFrom, uint64_t uCount );
   void append( const table_column_buffer& tableFrom, uint64_t uFrom, uint64_t uCount, std::vector< unsigned > vectorColumn );

   void append( const table* ptable_ );
   void append( const table* ptable_, uint64_t uFrom, uint64_t uCount );
   ///@}

   /// @brief size is same as `get_row_count and returns number of rows
   size_t size() const { return (size_t)get_row_count(); }
   /// clears all internal data in table, like a reset (columns are also deleted) 
   void clear();
   /// check if table is empty, don't have and data in table rows
   bool empty() const noexcept { return (m_puData == nullptr || m_uRowSize == 0); }
   /// checks if table isn't even initialized (not able to store data)
   bool empty( tag_raw ) const noexcept { return (m_uRowSize == 0); }

   /// @name equal operations, compare table or parts of table with the `equal` (= operator)
   /// append row data from one table into this table
   ///@{
   bool equal( const table_column_buffer& tableEqualTo, uint64_t uBeginRow, uint64_t uCount ) const noexcept;
   bool equal( const table_column_buffer& tableEqualTo ) const noexcept { return equal( tableEqualTo, 0, get_row_count() ); }
   ///@}

   /// @name harvest values from table into other type of container objects
   /// 0TAG0harvest.table_column_buffer
   ///@{

   template <typename TYPE>
   std::vector<TYPE> harvest( uint64_t uRow, unsigned uColumn, unsigned uCount, tag_row ) const noexcept;
   template <typename TYPE>
   std::vector<TYPE> harvest( uint64_t uRow, const std::string_view& stringName, unsigned uCount, tag_row ) const noexcept { return harvest<TYPE>( uRow, column_get_index( stringName ), uCount, tag_row{} ); }
   template<typename TYPE>
   std::vector< TYPE > harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount ) const;
   template<typename TYPE>
   std::vector< TYPE > harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount, tag_null ) const;
   template<typename TYPE>
   std::vector< TYPE > harvest( const std::string_view& stringColumnName, uint64_t uFrom, uint64_t uCount ) const { return harvest<TYPE>( column_get_index(stringColumnName), uFrom, uCount); }
   template<typename TYPE>
   std::vector< TYPE > harvest( unsigned uColumn ) const { return harvest<TYPE>( uColumn, (uint64_t)0, get_row_count() ); }
   template<typename TYPE>
   std::vector< TYPE > harvest( const std::string_view& stringColumnName ) const { return harvest<TYPE>( column_get_index(stringColumnName), ( uint64_t )0, get_row_count()); }
   template<typename TYPE>
   std::vector< TYPE > harvest( const std::string_view& stringColumnName, tag_null ) const { return harvest<TYPE>( column_get_index(stringColumnName), ( uint64_t )0, get_row_count(), tag_null{}); }

   /// harvest row values into vector with arguments
   void harvest( uint64_t uBeginRow, uint64_t uCount, std::vector<gd::argument::arguments>& vectorArguments ) const;
   void harvest( std::vector<gd::argument::arguments>& vectorArguments ) const { harvest( 0, get_row_count(), vectorArguments ); }
   std::vector<gd::argument::arguments> harvest( tag_arguments ) const { std::vector<gd::argument::arguments> v_; harvest( 0, get_row_count(), v_ ); return v_; }
   void harvest( const std::vector<uint64_t>& vectorRow, std::vector<gd::argument::arguments>& vectorArguments ) const;
   void harvest( const std::vector<uint64_t>& vectorRow, std::vector< std::vector<gd::variant_view> >& vectorRowValue ) const;

   void harvest( const std::vector< unsigned >& vectorColumn, const std::vector<uint64_t>& vectorRow, table_column_buffer& tableHarvest ) const;
   void harvest( const std::vector< std::string_view >& vectorColumnName, const std::vector<uint64_t>& vectorRow, table_column_buffer& tableHarvest ) const;
   ///@}
   
   // ## plant methods, insert values into table
   //    0TAG0plant.table_column_buffer

   void plant( const table_column_buffer& table, tag_name );
   void plant( const table_column_buffer& table, tag_name, tag_convert );
   void plant( const table_column_buffer& table, const std::string_view& stringColumnName );
   void plant( const table_column_buffer& table, uint64_t uFromRow, uint64_t uToRow, tag_name );
   void plant( const table_column_buffer& tablePlant, uint64_t uFrom, uint64_t uCount, tag_name, tag_convert );
   void plant( const table_column_buffer& table, const std::string_view& stringColumnName, uint64_t uFromRow, uint64_t uCount );
   void plant( const table_column_buffer& tablePlant, unsigned uColumnFrom, unsigned uColumnTo, uint64_t uFrom, uint64_t uCount );
   template< typename TYPE >
   void plant( unsigned uColumn, const std::vector< TYPE >& vectorValue, uint64_t uFrom, uint64_t uCount );
   template< typename TYPE >
   void plant( unsigned uColumn, const std::vector< TYPE >& vectorValue );

   template< typename TYPE, typename... Arguments >
   void plant( const std::string_view& stringName, const std::vector< TYPE >& vectorValue, Arguments&&... arguments );

   void plant( unsigned uColumn, const gd::variant_view& variantviewValue );
   void plant( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uFrom, uint64_t uCount );

   void swap( uint64_t uRow1, uint64_t uRow2 );

   // https://github.com/kevinhermawan/sortire

   void sort( unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_selection );
   void sort( unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_bubble );

   void sort( unsigned uColumn, bool bAscending ) { sort( uColumn, bAscending, 0, get_row_count(), tag_sort_selection{} ); }

   void sort_null(unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_bubble);
   void sort_null( unsigned uColumn, bool bAscending ) { sort_null( uColumn, bAscending, 0, get_row_count(), tag_sort_bubble{} ); }

   template<typename TAG_ALGORITHM>
   void sort( unsigned uColumn, bool bAscending, TAG_ALGORITHM tag_ ) { sort( uColumn, bAscending, 0, get_row_count(), tag_ ); }
   template<typename TAG_ALGORITHM>
   void sort( unsigned uColumn, TAG_ALGORITHM tag_ ) { sort( uColumn, true, 0, get_row_count(), tag_); }
   template<typename TAG_ALGORITHM>
   void sort( const std::string_view& stringColumnName, TAG_ALGORITHM tag_ ) { sort( column_get_index(stringColumnName), true, 0, get_row_count(), tag_); }
   template<typename TAG_ALGORITHM>
   void sort( const std::string_view& stringColumnName, bool bAscending, TAG_ALGORITHM tag_ ) { sort( column_get_index(stringColumnName), bAscending, 0, get_row_count(), tag_); }
   template<typename TAG_ALGORITHM>
   void sort( const std::string_view& stringColumnName, bool bAscending, uint64_t uFrom, uint64_t uCount, TAG_ALGORITHM tag_ ) { sort( column_get_index(stringColumnName), bAscending, uFrom, uCount, tag_); }

/** \name RANGE
* Range operations for selected parts in table
*///@{
   /// return range for complete table
   range get_range() const { return range( 0, 0, get_row_count() - 1, get_column_count() ); }
   /// get range object for all cells in column
   range range_column( unsigned uColumn ) { return range( 0, uColumn, get_row_count() - 1, uColumn ); }
   range range_column( const std::string_view& stringColumnName  ) { return range_column( column_get_index(stringColumnName) ); }
   /// get range object for all cells in row
   range range_row( uint64_t uRow ) { return range( uRow, 0, uRow, get_column_count() - 1 ); }
//@}

   void split( uint64_t uRowCount, std::vector<table_column_buffer>& vectorSplit );
   std::vector<table_column_buffer> split( uint64_t uRowCount );
   void split( uint64_t uRowCount, std::vector<table>& vectorSplit );

/** \name ERASE
* Erase parts in table
*///@{
   /// Erase from row index and number of rows from that row
   void erase( uint64_t uFrom, uint64_t uCount );
   /// Erase selected row
   void erase( uint64_t uRow ) { erase( uRow, 1 ); }
   /// Erase selected rows
   uint64_t erase(const uint64_t* puRowIndex, uint64_t uCount);
   /// Erase selected rows, rows should be sorted in descending order
   void erase(const uint64_t* puRowIndex, uint64_t uCount, tag_raw );
   /// Erase selected rows
   uint64_t erase(const std::vector<uint64_t>& vectorRowIndex) { return erase(vectorRowIndex.data(), (uint64_t)vectorRowIndex.size()); }
   /// Erase selected rows, rows should be sorted in descending order
   void erase(const std::vector<uint64_t>& vectorRowIndex, tag_raw) { erase(vectorRowIndex.data(), (uint64_t)vectorRowIndex.size(), tag_raw{}); }
//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
// 0TAG0attributes.table_column_buffer
public:
   uint8_t* m_puData = nullptr;        ///< data to hold values in table
   uint8_t* m_puMetaData = nullptr;    ///< data block with row meta information
   unsigned m_uFlags;                  ///< state information for table
   unsigned m_uRowSize;                ///< row size in bytes
   unsigned m_uRowMetaSize;            ///< meta data size in bytes for each row
   unsigned m_uRowGrowBy = eSpaceRowGrowBy;///< if table needs more space, this holds number of rows to grow by
   uint64_t m_uRowCount;               ///< row count (row count * row size = total amount of bytes allocated)
   uint64_t m_uReservedRowCount;       ///< reserved row count, max number of rows that can be placed in allocated memory
   gd::argument::arguments m_argumentsProperty; ///< table properties
   references m_references;            ///< Stores blob data
   names m_namesColumn;                ///< names for columns in table. this works like a data store for const text values used to store column names and aliases
   std::vector<column> m_vectorColumn; ///< information about each column in table

#ifndef NDEBUG
   uint64_t m_uAllocatedBlockSize_d = 0;
#endif // NDEBUG



// ## free functions ------------------------------------------------------------
public:
   /// return state value for row, moves in row buffer to location where state is stored and return state value
   //static uint32_t get_row_state_s( const uint8_t* puRow ) { return *(uint32_t*)(puRow + eSpaceNullColumns); }

   // ## modify null flag for row
   inline static void cell_set_null_s( uint8_t* puRow, unsigned uIndex ) { assert( uIndex < 64 ); *(uint64_t*)puRow |= (1ULL << uIndex); }
   inline static void cell_set_not_null_s( uint8_t* puRow, unsigned uIndex ) { assert( uIndex < 64 ); *(uint64_t*)puRow &= ~(1ULL << uIndex); }
   inline static bool cell_is_null_s( uint8_t* puRow, unsigned uIndex ) { assert( uIndex < 64 ); return (*(uint64_t*)puRow & (1ULL << uIndex)) != 0; }

   // ## convert names to column index in table
   std::vector<int32_t> column_get_index_s( const table_column_buffer& tablecolumnbuffer, const std::vector<std::string>& vectorName );

   // ## match columns between tables

   /// return column indexes for tables that match based on name
   static std::vector< std::pair< unsigned, unsigned > > column_match_s( const table_column_buffer& t1_, const table_column_buffer& t2_, tag_name );
   /// return column indexes for tables that match based on alias
   static std::vector< std::pair< unsigned, unsigned > > column_match_s( const table_column_buffer& t1_, const table_column_buffer& t2_, tag_alias );
   /// return match between two vectors containing string_view objects, this for more general use where table need to match for external objects
   static std::vector< std::pair< unsigned, unsigned > > column_match_s( const std::vector<std::string_view>& v1_, const std::vector<std::string_view>& v2_ );
   /// fill vectors with indexes to columns that have same names
   static void column_match_s( const table_column_buffer& t1_, const table_column_buffer& t2_, std::vector<unsigned>* pvector1, std::vector<unsigned>* pvector2, tag_name );

   static void join_s( const table_column_buffer* pT1_, unsigned uColumn1, const table_column_buffer* pT2_, unsigned uColumn2, std::vector< std::pair<uint64_t, uint64_t> >& vectorMatch );

};

inline void table_column_buffer::common_construct( table_column_buffer&& o ) noexcept {            assert( m_puData == nullptr );
   m_uFlags          = o.m_uFlags; 
   m_uRowSize        = o.m_uRowSize;  
   m_uRowMetaSize    = o.m_uRowMetaSize;
   m_uRowCount       = o.m_uRowCount; 
   m_uReservedRowCount = o.m_uReservedRowCount;
   m_puData          = o.m_puData; o.m_puData = nullptr;
   m_puMetaData      = o.m_puMetaData; o.m_puMetaData = nullptr;
   m_vectorColumn    = std::move( o.m_vectorColumn );
   m_namesColumn     = std::move( o.m_namesColumn );
   m_references      = std::move( o.m_references );
   m_argumentsProperty = std::move( o.m_argumentsProperty );
#ifndef NDEBUG
   m_uAllocatedBlockSize_d = o.m_uAllocatedBlockSize_d;
#endif // NDEBUG

}

/** ---------------------------------------------------------------------------
 * @brief return column type for all columns in table
 * @return std::vector<unsigned> vector with column type for all columns
*/
inline std::vector<unsigned> table_column_buffer::column_get_type() const {
   std::vector<unsigned> vectorType;
   for(const auto it : m_vectorColumn) { vectorType.push_back( it.type() ); }
   return vectorType;
}

/** ---------------------------------------------------------------------------
* @brief return column type for all columns in table
* @return std::vector<unsigned> vector with column type for all columns
*/
inline std::vector<unsigned> table_column_buffer::column_get_ctype() const {
   std::vector<unsigned> vectorCType;
   for(const auto it : m_vectorColumn) { vectorCType.push_back( it.ctype() ); }
   return vectorCType;
}


/** ---------------------------------------------------------------------------
 * @brief Get number of bytes used to store meta information for each row
 * Meta information is a small memory part in meta part memory that stores
 * information about each row if table is created to store meta information.
 * Here you can set information like if the row is in use, deleted and what
 * columns contains NULL values.
 * @return unsigned bytes needed to store meta information for row
*/
inline unsigned table_column_buffer::size_row_meta() const noexcept {
   unsigned uMetaDataSize = 0;
   if( is_null32() == true )      uMetaDataSize += eSpaceNull32Columns;
   else if( is_null64() == true ) uMetaDataSize += eSpaceNull64Columns;

   if( is_rowstatus() == true )   uMetaDataSize += eSpaceRowState;

   return uMetaDataSize;
}

/** ---------------------------------------------------------------------------  
 * @brief Add row to table (note that table has "taken" rows and reserved or allocated rows)  
 *  
 * Add row/rows to table, if number of total rows need larger memory block table will grow  
 * with "grow by" member or if "grow by" member is 0 it will grow by adding 50% to  
 * total amount of rows.  
 *  
 * @note If the table is generated to store null values, this method does not set cells in the new row to null.  
 *  
 * @param uCount number of rows to add  
 */
inline void table_column_buffer::row_add( uint64_t uCount ) { 
   m_uRowCount += uCount; 
   if( m_uRowCount > m_uReservedRowCount ) {
      uint64_t uAddRowCount = m_uRowCount - m_uReservedRowCount;               // number of rows to grow
      if( m_uRowGrowBy == 0 ) { uAddRowCount += m_uRowCount / 2; }             // add 50% extra rows
      else                    { uAddRowCount += m_uRowGrowBy; }                // add with grow by
      row_reserve_add( uAddRowCount );                                         // increase memory block
   }
}

/** ---------------------------------------------------------------------------
 * @brief Adds rows to the table and sets their values to null if the table supports null values.
 * 
 * This method increases the row count of the table by the specified number of rows. If the table
 * is configured to support null values, the newly added rows will have all their columns set to null.
 * If the table's current row count exceeds the reserved row count, the table's memory block will grow
 * to accommodate the additional rows.
 * 
 * @param uCount The number of rows to add to the table.
 * @param tag_null A tag indicating that the added rows should have their values set to null.
 * 
 * @note This method asserts that the table supports null values.
 */
inline void table_column_buffer::row_add( uint64_t uCount, tag_null ) {                            assert( is_null() == true );
    auto uBegin = m_uRowCount;
    row_add( uCount );
    row_set_null(uBegin, m_uRowCount - uBegin);                               // set all new rows to null
}

/** ---------------------------------------------------------------------------
 * @brief Adds a single row to the table.
 * 
 * This method is a simplified version of adding rows to the table, specifically designed for the common operation of adding one row at a time.
 * It increases the row count by one and ensures that the table has enough memory allocated to accommodate the new row.
 * If the table supports null values, the newly added row will have all its columns set to null.
 * 
 * @return uint64_t The index of the newly added row.
 */
inline uint64_t table_column_buffer::row_add_one() {
   row_add(1);
   if( is_null() == true ) { row_set_null(m_uRowCount - 1); }                 // set all new rows to null
   return m_uRowCount - 1;
}


/** ---------------------------------------------------------------------------
 * @brief get column name for column index
 * @param uIndex index to column name is returned for if column has name
 * @return std::string_view with column name, empty string if column do not have a name 
*/
inline std::string_view table_column_buffer::column_get_name( unsigned uIndex ) const {            assert( uIndex < m_vectorColumn.size() );
   const auto& column = m_vectorColumn[uIndex];
   unsigned uOffset = column.name();
   if( uOffset != 0 ) { return m_namesColumn.get( uOffset ); }
   return std::string_view();
}

/** ---------------------------------------------------------------------------
 * @brief get column name for column object
 * @param column column object name is returned for
 * @return std::string_view with column name, empty string if column do not have a name 
*/
inline std::string_view table_column_buffer::column_get_name( const column& column ) const {
   unsigned uOffset = column.name();
   if( uOffset != 0 ) { return m_namesColumn.get( uOffset ); }
   return std::string_view();
}

/** ---------------------------------------------------------------------------
 * @brief Returns names for columns that has name in vector, empty string if no name 
 * @return std::vector<std::string_view> vector with column names
*/
inline std::vector<std::string_view> table_column_buffer::column_get_name() const
{
   std::vector<std::string_view> vectorName;
   for( unsigned uColumn = 0; uColumn < get_column_count(); uColumn++ ) {
      vectorName.push_back( column_get_name( uColumn ) );
   }
   return vectorName;
}

/** ---------------------------------------------------------------------------
 * @brief Returns names for columns that has name in vector, empty string if no name 
 * @param vectorColumn column indexes names are returned for
 * @return std::vector<std::string_view> vector with names
*/
inline std::vector<std::string_view> table_column_buffer::column_get_name(const std::vector<unsigned>& vectorColumn) const {
   std::vector<std::string_view> vectorName;
   for(auto uColumn : vectorColumn) {                                                              assert( uColumn < get_column_count() );
      vectorName.push_back( column_get_name( uColumn ) );
   }
   return vectorName;
}

/** ---------------------------------------------------------------------------
 * @brief get column alias for column index
 * @param uIndex index to column alias is returned for if column has name
 * @return std::string_view with column name, empty string if column do not have alias
*/
inline std::string_view table_column_buffer::column_get_alias( unsigned uIndex ) const {           assert( uIndex < m_vectorColumn.size() );
   const auto& column = m_vectorColumn[uIndex];
   unsigned uOffset = column.alias();
   if( uOffset != 0 ) { return m_namesColumn.get( uOffset ); }
   return std::string_view();
}

/** ---------------------------------------------------------------------------
 * @brief get column alias for column object
 * @param column column object alias is returned for
 * @return std::string_view with column alias, empty string if column do not have a alias 
*/
inline std::string_view table_column_buffer::column_get_alias( const column& column ) const {
   unsigned uOffset = column.alias();
   if( uOffset != 0 ) { return m_namesColumn.get( uOffset ); }
   return std::string_view();
}

/** ---------------------------------------------------------------------------
 * @brief Fill column with value
 * @param stringName column name to fill with value
 * @param variantviewValue value to fill
 * @param arguments extra arguments to set to and from rows
*/
template< typename... Arguments >
void table_column_buffer::column_fill( const std::string_view& stringName, const gd::variant_view& variantviewValue, Arguments&&... arguments ) {
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   column_fill( uColumnIndex, variantviewValue, std::forward< Arguments >(arguments)... );         // select proper method to fill column with value
}

/** ---------------------------------------------------------------------------
 * @brief Fill column with value
 * @param stringName column name to fill with value
 * @param vectorValue values to add to column
 * @param arguments extra arguments to set to and from rows
*/
template< typename... Arguments >
void table_column_buffer::column_fill( const std::string_view& stringName, const std::vector< gd::variant_view >& vectorValue, Arguments&&... arguments ) {
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   column_fill( uColumnIndex, vectorValue, std::forward< Arguments >(arguments)... );              // select proper method to fill column with values from vector
}


/** ---------------------------------------------------------------------------
 * @brief Return pointer to row null value section (flags in metadata marking cell null values)
 * This is the first part of meta data for each row, if table is created to store null values for each column
 * @param uRow index for row null value is returned for
 * @return uint8_t* pointer to row null value section
*/
inline uint8_t* table_column_buffer::row_get_null( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( m_puMetaData != nullptr ); 
   return reinterpret_cast<uint8_t*>( m_puMetaData + (uRow * m_uRowMetaSize) );
}

/** ---------------------------------------------------------------------------
 * @brief get position in buffer to row state information for row at index
 * @param uRow index to row where state is located
 * @return uint32_t* pointer to position in internal buffer for row state
*/
inline uint32_t* table_column_buffer::row_get_state( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( is_rowstatus() == true ); 
   // calculate number of bytes used to store flags for culumns marked as null (cant be over sizeof(uint32_t) * 2 or 8 bytes)
   // note that state cant be set to both 32 and 64 columns
   // calculate size for null values to know offset for state value
   unsigned uNullSize = (m_uFlags & (eTableFlagNull32|eTableFlagNull64));                          assert(( m_uFlags & ( eTableFlagNull32 | eTableFlagNull64 ) ) != 3); // cant be both 32 and 64
   uNullSize = uNullSize * sizeof(uint32_t);                                                       assert( uNullSize <= (sizeof(uint32_t) * 2) );
   return reinterpret_cast<uint32_t*>( m_puMetaData + (uRow * m_uRowMetaSize) + uNullSize ); // return pointer to state value
}

/** ---------------------------------------------------------------------------
 * @brief set and clear row state flags
 * @param uRow index for row state is modified for
 * @param uSet flags set to row
 * @param uClear flags cleared
*/
inline void table_column_buffer::row_set_state( uint64_t uRow, unsigned uSet, unsigned uClear ) { assert( uRow < m_uReservedRowCount ); 
   uint32_t* puFlags = row_get_state( uRow );
   *puFlags |= uSet;
   *puFlags &= ~uClear;
}

/** ---------------------------------------------------------------------------
 * @brief check if row is in use
 * @param uRow index to row where state is located that is checked for use
 * @return bool true if row is used, false if not
*/
inline bool table_column_buffer::row_is_use( uint64_t uRow ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( is_rowstatus() == true ); 
   // calculate number of bytes used to store flags for culumns marked as null (cant be over sizeof(uint32_t) * 2 or 8 bytes)
   // note that state cant be set to both 32 and 64 columns
   unsigned uNullSize = (m_uFlags & (eTableFlagNull32|eTableFlagNull64)) * sizeof(uint32_t);     assert( uNullSize <= (sizeof(uint32_t) * 2) );
   return (*reinterpret_cast<uint32_t*>( m_puMetaData + (uRow * m_uRowMetaSize) + uNullSize ) & (uint32_t)eRowStateUse) == (uint32_t)eRowStateUse; // return if row is used
}


/** ---------------------------------------------------------------------------
 * @brief set all columns to null in row
 * @param uRow index to row where values are set to null
*/
inline void table_column_buffer::row_set_null( uint64_t uRow ) { assert( uRow < m_uReservedRowCount ); assert( is_null() == true );
   auto puRow = row_get_null( uRow );

   if( is_null32() ) *(uint32_t*)puRow =((uint32_t)-1);
   else              *(uint64_t*)puRow =((uint64_t)-1);
}

/** ---------------------------------------------------------------------------
 * @brief Set all values in row to null
 * @param uFrom start row to set all values to null
 * @param uCount number of sequential rows to set to null
*/
inline void table_column_buffer::row_set_null( uint64_t uFrom, uint64_t uCount ) { assert( (uFrom + uCount) <= get_row_count() );
   for( auto u = uFrom, uMax = (uFrom + uCount); u < uMax; u++ ) row_set_null( u );
}

/** ---------------------------------------------------------------------------
 * @brief set row values from external c++ object
 * @param uRow index for row where values are placed
 * @param uFirstColumn first column where first value in external object is placed, rest is placed in following columns
 * @param object_ object with values that are placed on row
 */
template< typename OBJECT >
void table_column_buffer::row_set( uint64_t uRow, unsigned uFirstColumn, const OBJECT& object_ ) {
   std::vector< gd::variant_view > vectorObject;
   object_.to_values( vectorObject );
   row_set( uRow, uFirstColumn, vectorObject, tag_convert{});
}


/** ---------------------------------------------------------------------------
 * @brief set row values from external c++ object
 * For this to work the external c++ object needs to implement a member method
 * named to `to_values` and a static member method called `to_member_name`
 * @param uRow index for row where values are placed
 * @param stringPrefixFind if named column where to start to set values from object is prefixed.
          prefix names enables the posibility to store multiple objects with same
          object type on same row.
 * @param object_ object with values that are placed on row
 */
template< typename OBJECT >
void table_column_buffer::row_set( uint64_t uRow, const std::string_view& stringPrefixFind, const OBJECT& object_ ) {
   std::vector< gd::variant_view > vectorObject;
   object_.to_values( vectorObject );
   std::string stringName = OBJECT::to_member_name( 0, stringPrefixFind );
   unsigned uFirstColumn = column_get_index( stringName ); 
   row_set( uRow, uFirstColumn, vectorObject, tag_convert{});
}


/** ---------------------------------------------------------------------------
 * @brief If you know the type value in column and it is not null then this is very fast to return exact value
 * @param uRow index for row where cell values is found
 * @param uColumn index to column in row value is returned
 * @return TYPE value returned that will have the specified type
*/
template <typename TYPE>
TYPE table_column_buffer::cell_get( uint64_t uRow, unsigned uColumn ) const noexcept {
   return *(TYPE*)cell_get( uRow, uColumn );
}

/** ---------------------------------------------------------------------------
 * @brief Check if cell is null
 * @param uRow row for cell
 * @param uColumn index for cell column
 * @return true if null, false if not null
*/
inline bool table_column_buffer::cell_is_null( uint64_t uRow, unsigned uColumn ) const noexcept { assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   uint64_t uNullRow = 0;
   auto puRow = row_get_null( uRow );
   if( is_null32() ) uNullRow = (uint64_t)*(uint32_t*)puRow;
   else              uNullRow = *(uint64_t*)puRow;
   
   return (uNullRow & (1ULL << uColumn)) != 0;
}

/** ---------------------------------------------------------------------------
 * @brief Set value in column to null (marks null flag for column)
 * @param uRow row where cell is
 * @param uColumn cell column
*/
inline void table_column_buffer::cell_set_null( uint64_t uRow, unsigned uColumn ) { assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   auto puRow = row_get_null( uRow );

#ifdef _DEBUG
   uint64_t uNull_d = 0;
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

   if( is_null32() ) *(uint32_t*)puRow |= ((uint32_t)1 << uColumn);
   else              *(uint64_t*)puRow |= ((uint64_t)1 << uColumn);

#ifdef _DEBUG
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

}


/** ---------------------------------------------------------------------------
 * @brief Set value in column to null (marks null flag for column) 
 * @param uRow row where cell is
 * @param stringName cell column name
*/
inline void table_column_buffer::cell_set_null( uint64_t uRow, const std::string_view& stringName ) { assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   unsigned uColumnIndex = column_get_index( stringName ); 
   cell_set_null( uRow , uColumnIndex);
}


inline void table_column_buffer::cell_set_not_null( uint64_t uRow, unsigned uColumn ) { 
                                                                                                   assert( uRow < m_uReservedRowCount ); assert( m_uFlags & (eTableFlagNull32|eTableFlagNull64) );
   auto puRow = row_get_null( uRow );

#ifdef _DEBUG
   uint64_t uNull_d = 0;
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

   if( is_null32() ) *(uint32_t*)puRow &= ~((uint32_t)1 << uColumn);
   else              *(uint64_t*)puRow &= ~((uint64_t)1 << uColumn);

#ifdef _DEBUG
   if( is_null32() ) uNull_d = *(uint32_t*)puRow;
   else              uNull_d = *(uint64_t*)puRow;
#endif // _DEBUG

}

/** ---------------------------------------------------------------------------
 * @brief Append to table from another table and select from that table what columns that are added
 * @param tableFrom table data is appended from
 * @param vectorColumnIndex list of column index values used to append from
*/
inline void table_column_buffer::append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom ) { assert( vectorColumnIndexFrom.size() <= get_column_count() );
   std::vector<unsigned> vectorColumnIndexTo;
   for( decltype( vectorColumnIndexFrom.size() ) u = 0, uMax = vectorColumnIndexFrom.size(); u < uMax; u++ ) { vectorColumnIndexTo.push_back( (unsigned)u ); }
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), (unsigned)vectorColumnIndexFrom.size() );
}

/** ---------------------------------------------------------------------------
 * @brief Append to table from another table and select from that table what columns that are added
 * @param tableFrom table data is appended from
 * @param vectorColumnIndex list of column index values used to append from
*/
inline void table_column_buffer::append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, tag_convert ) { assert( vectorColumnIndexFrom.size() <= get_column_count() );
   std::vector<unsigned> vectorColumnIndexTo;
   for( decltype( vectorColumnIndexFrom.size() ) u = 0, uMax = vectorColumnIndexFrom.size(); u < uMax; u++ ) { vectorColumnIndexTo.push_back( (unsigned)u ); }
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), (unsigned)vectorColumnIndexFrom.size(), tag_convert{});
}


/** ---------------------------------------------------------------------------
 * @brief Append selected data from "from" table into selected columns in source (this) table
 * @param tableFrom table data is appended from
 * @param vectorColumnIndexFrom index to columns cell values are copied from
 * @param vectorColumnIndexTo index to columns cell values are copied to
*/
inline void table_column_buffer::append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo ) {
   unsigned uColumnCount = (unsigned)std::min<unsigned>( (unsigned)vectorColumnIndexFrom.size(), (unsigned)vectorColumnIndexTo.size() );
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), uColumnCount );
}

/** ---------------------------------------------------------------------------
 * @brief Append selected data from "from" table into selected columns in source (this) table
 * @param tableFrom table data is appended from
 * @param vectorColumnIndexFrom index to columns cell values are copied from
 * @param vectorColumnIndexTo index to columns cell values are copied to
*/
inline void table_column_buffer::append( const table_column_buffer& tableFrom, const std::vector<unsigned>& vectorColumnIndexFrom, const std::vector<unsigned>& vectorColumnIndexTo, tag_convert ) {
   unsigned uColumnCount = (unsigned)std::min<unsigned>( (unsigned)vectorColumnIndexFrom.size(), (unsigned)vectorColumnIndexTo.size() );
   append( tableFrom, vectorColumnIndexFrom.data(), vectorColumnIndexTo.data(), uColumnCount, tag_convert{});
}

/** ---------------------------------------------------------------------------
 * @brief find value in column
 * @param stringName name for column where value are searched
 * @param uStartRow start row
 * @param uCount number of rows to search in
 * @param variantviewFind value to search for
 * @return index to row if value was found, -1 if not found
 */
inline int64_t table_column_buffer::find_variant_view(const std::string_view& stringName, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind) const noexcept {
   unsigned uColumn = column_get_index(stringName);
   return find_variant_view( uColumn, uStartRow, uCount, variantviewFind );
}

/** ---------------------------------------------------------------------------
 * @brief harvest selected columns from selected rows into table
 * @param vectorColumnName columns to harvest values from
 * @param vectorRow selected rows to read values from
 * @param tableHarvest table to the put read values from selected rows into
 */
inline void table_column_buffer::harvest(const std::vector< std::string_view >& vectorColumnName, const std::vector<uint64_t>& vectorRow, table_column_buffer& tableHarvest) const {
   std::vector< unsigned > vectorColumn = column_get_index( vectorColumnName );
   harvest( vectorColumn, vectorRow, tableHarvest );
}

/** ---------------------------------------------------------------------------
 * @brief return vector with values in row. cast to specified type
 * @param uRow row where cells exists
 * @param uColumn first column
 * @param uCount number of columns from first placing values in vector
 * @return vector with cell values from specified row
*/
template <typename TYPE>
std::vector<TYPE> table_column_buffer::harvest( uint64_t uRow, unsigned uColumn, unsigned uCount, tag_row ) const noexcept { assert( (uColumn + uCount) <= get_column_count() ); assert( column_get_primitive_size( uColumn ) == sizeof(TYPE) );
   std::vector<TYPE> vectorType; // vector that gets values in row
   vectorType.reserve( uCount );
   const TYPE* p_ = (const TYPE*)cell_get( uRow, uColumn );                    // first cell position in row
   for( auto u = 0; u < uCount; u++ ) { 
      vectorType.emplace_back( p_[u] ); 
   }
   return vectorType;
}


/** ---------------------------------------------------------------------------
 * @brief get vector with values based starting from selected row and count of values
 * @param uColumn column index values are taken from
 * @param uFrom start row where harvesting starts
 * @param uCount number of values (rows) to harvest
 * @return vector std::vector< TYPE > vector with harvested values
*/
template<typename TYPE>
inline std::vector< TYPE > table_column_buffer::harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount ) const {
   std::vector< TYPE > vector_;
   vector_.reserve( uCount );
   auto uEndRow = uFrom + uCount; 
   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = column_get_ctype( uColumn );
   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) )                    // check if return type is same as calculated type, then no conversion is needed (faster)
   {
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         TYPE v_ = (TYPE)cell_get_variant_view( uRow, uColumn );
         vector_.push_back( v_ );
      }
   }
   else
   {                                                                           // return type do not match column type, we need to convert value to requested type
      gd::variant variantConverted;
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         auto VVValue = cell_get_variant_view( uRow, uColumn );
         VVValue.convert_to( eType, variantConverted );
         TYPE v_ = (TYPE)variantConverted;
         vector_.push_back( v_ );
      }
   }

   return vector_;
}

/** ---------------------------------------------------------------------------
 * @brief get vector with values based starting from selected row and count of values
 * Method checks for null values, if null values are found they are skipped
 * @param uColumn column index values are taken from
 * @param uFrom start row where harvesting starts
 * @param uCount number of values (rows) to harvest
 * @return vector std::vector< TYPE > vector with harvested values
*/
template<>
inline std::vector< std::string > table_column_buffer::harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount ) const {
   std::vector< std::string > vector_;
   vector_.reserve( uCount );
   uint64_t uEndRow = uFrom + uCount;
   for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
   {
      std::string v_ = cell_get_variant_view( uRow, uColumn ).as_string();
      vector_.push_back( v_ );
   }

   return vector_;
}

template<typename TYPE>
inline std::vector< TYPE > table_column_buffer::harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount, tag_null ) const {
   std::vector< TYPE > vector_;
   vector_.reserve( uCount );
   auto uEndRow = uFrom + uCount; 
   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = column_get_ctype( uColumn );
   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) )                    // check if return type is same as calculated type, then no conversion is needed (faster)
   {
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         if(cell_is_null(uRow, uColumn)) continue;
         TYPE v_ = (TYPE)cell_get_variant_view( uRow, uColumn );
         vector_.push_back( v_ );
      }
   }
   else
   {                                                                           // return type do not match column type, we need to convert value to requested type
      gd::variant variantConverted;
      for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
      {
         if(cell_is_null(uRow, uColumn)) continue;
         auto VVValue = cell_get_variant_view( uRow, uColumn );
         VVValue.convert_to( eType, variantConverted );
         TYPE v_ = (TYPE)variantConverted;
         vector_.push_back( v_ );
      }
   }

   return vector_;
}

template<>
inline std::vector< std::string > table_column_buffer::harvest( unsigned uColumn, uint64_t uFrom, uint64_t uCount, tag_null ) const {
   std::vector< std::string > vector_;
   vector_.reserve( uCount );
   uint64_t uEndRow = uFrom + uCount;
   for( auto uRow = uFrom; uRow < uEndRow; uRow++ )
   {
      if(cell_is_null(uRow, uColumn)) continue;
      std::string v_ = cell_get_variant_view( uRow, uColumn ).as_string();
      vector_.push_back( v_ );
   }

   return vector_;
}

template<typename TYPE>
void table_column_buffer::plant( unsigned uColumn, const std::vector< TYPE >& vectorValue, uint64_t uFrom, uint64_t uCount ) { assert( uColumn < get_column_count() ); 
   auto eType = gd::types::type_g<TYPE>( gd::types::tag_ask_compiler{});
   auto uColumnType = column_get_ctype( uColumn );
   uint64_t uEndRow = uFrom + uCount;
   if( uCount > vectorValue.size() ) uCount = vectorValue.size();
   if( (( unsigned )eType & 0xff) == (uColumnType & 0xff) )                    // check if return type is same as calculated type, then no conversion is needed (faster)
   {
      for( uint64_t uIndex = 0; uIndex < uCount; uIndex++ )
      {
         cell_set( uIndex + uFrom, uColumn, vectorValue[uIndex] );
      }
   }
   else
   {                                                                           // return type do not match column type, we need to convert value to requested type
      gd::variant variantConverted;
      for( uint64_t uIndex = 0; uIndex < uCount; uIndex++ )
      {
         cell_set( uIndex + uFrom, uColumn, vectorValue[uIndex], tag_convert{} );
      }
   }
}

template<typename TYPE>
void table_column_buffer::plant( unsigned uColumn, const std::vector< TYPE >& vectorValue ) {
   plant( uColumn, vectorValue, 0, get_row_count() );
}

template< typename TYPE, typename... Arguments >
void table_column_buffer::plant( const std::string_view& stringName, const std::vector< TYPE >& vectorValue, Arguments&&... arguments ) {
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   plant( uColumnIndex, vectorValue, std::forward< Arguments >(arguments)... ); // call matching plant method using index for column
}

inline void table_column_buffer::plant( unsigned uColumn, const gd::variant_view& variantviewValue ) {
   plant( uColumn, variantviewValue, 0, get_row_count() );
}


namespace serialize {
   template<typename ARCHIVE>
   void save( ARCHIVE& archive, table_column_buffer& t_, const unsigned uVersion )
   {
      const auto& names = t_.get_names();

      auto uNamesSize = names.size();
      archive << uNamesSize;
      archive.save_binary( names.data(), uNamesSize );

      uint32_t uColumnCount = t_.get_column_count();
      archive << uColumnCount;

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         const auto& column = t_.column_get( uColumn );
         archive.save_binary( &column, sizeof( column ) );
      }
   }
}


/** @TAG ##gd #table #debug #print
 * 
 * @code
// Sample on how to collect column structure and print it to strings
void RunExpression_g( const std::string_view& stringExpression, 
                     const gd::argument::shared::arguments& argumentsCode, 
                     const gd::table::dto::table* ptableLineList, 
                     gd::table::dto::table* ptableSnippet)
{
   #ifndef NDEBUG
   std::string stringTableList_d = gd::table::debug::print(ptableLineList, gd::table::tag_columns{});
   std::string stringTableSnippet_d = gd::table::debug::print(ptableSnippet, gd::table::tag_columns{});
   #endif

   // ... rest of the code
 * @endcode
 * 
 */

namespace debug {
   std::string print( const table_column_buffer& table, uint64_t uCount );
   std::string print( const table_column_buffer& table );
   std::string print( const table_column_buffer& table, tag_columns );
   std::string print( const table_column_buffer* ptable, tag_columns );
   std::string print_column( const table_column_buffer* ptable );
   std::string print_row( const table_column_buffer& table, uint64_t uRow );
}

_GD_TABLE_END

_GD_TABLE_BEGIN
namespace dto {
   using table = table_column_buffer;
}
_GD_TABLE_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif

