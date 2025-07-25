/**
 * @file gd_table.h
 *
 * @brief Common table logic. Include this in all table related functionality.
 *
 *
 *
 *
 *
 */

#pragma once


// ## Default for DEBUG_RELEASE in debug mode is 1 and DEBUG_RELEASE_EXECUTE to execute operations
#if !defined(NODEBUG) && !defined(DEBUG_RELEASE)
#  define DEBUG_RELEASE 1
#  define DEBUG_RELEASE_EXECUTE( expression ) expression
#else
#  ifndef DEBUG_RELEASE_EXECUTE
#     define DEBUG_RELEASE_EXECUTE( expression ) ((void)0)
#  endif
#endif

#if DEBUG_RELEASE > 0
inline constexpr uint8_t uTailetextMarker_d = 0x01;
#endif

#include <cassert>
#include <cmath>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include "gd_types.h"

#if defined( __clang__ )
   #pragma GCC diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wreorder-ctor"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 6387 26495 26812 )
#endif

#if defined(__GNUC__) or defined(__clang__)
#  define GD_TABLE_64BIT __SIZE_WITDH__ == 64
#elif defined(_WIN64)
#  define GD_TABLE_64BIT
#else
   static_assert(false, "unspported implmentation");
#endif



#ifndef _GD_TABLE_BEGIN
#define _GD_TABLE_BEGIN namespace gd { namespace table {
#define _GD_TABLE_END } }
_GD_TABLE_BEGIN
#else
_GD_TABLE_BEGIN
#endif

/// tag dispatcher for speed up unnecessary assignments
struct tag_undefined {};
/// tag dispatcher used to construct object where null values are valid
struct tag_null {};
/// tag dispatcher to mark to use meta information
struct tag_meta {};
/// tag dispatcher to create object with all meta data turned on
struct tag_full_meta {};
/// types are described with names (string)
struct tag_type_name {};
/// types are described with constant (integer value)
struct tag_type_constant {};
/// used in copy operations
using tag_copy = gd::types::tag_copy;
/// for convert methods
using tag_convert = gd::types::tag_convert;
/// for adjusting methods
using tag_adjust = gd::types::tag_adjust;
/// prepare (allocate internal buffers) table to be ready for work
struct tag_prepare {};
/// use name in operation
using tag_name = gd::types::tag_name;
/// use alias in operation
using tag_alias = gd::types::tag_alias;
/// used when wildcard matches are done
struct tag_wildcard {};
/// tag dispatcher used for methods working with values (lenght depends on the context of method)
using tag_value = gd::types::tag_value;
/// tag dispatcher used for methods working with length (lenght depends on the context of method)
struct tag_length {};
/// tag dispatcher for range logic (range is like an area in table with start cell and end cell).
struct tag_range {};
/// tag dispatcher for measurement handling
struct tag_measurement {};


/// tag dispatcher used for methods working with variant object
struct tag_variant {};
/// tag dispatcher used for methods working with variant_view object
struct tag_variant_view {};
/// tag dispatcher used for methods working with arguments object
struct tag_arguments {};

/// tag dispatcher direct access to memory data
using tag_raw = gd::types::tag_raw;
/// tag dispatcher do modify methods to use pointers
struct tag_pointer {};
/// tag dispatcher used for operations related to text
struct tag_text {};

/// ## tag dispatchers for sorting
/// tag dispatcher for selection sort
struct tag_sort_selection {};
/// tag dispatcher for bubble sort
struct tag_sort_bubble {};

/// Operation on specified row
struct tag_row {};
/// Operation on specified column
struct tag_column {};

/// tag dispatcher used when text is sent as argument and should be parsed to extract values.
struct tag_parse {};


struct tag_cell {};
struct tag_columns {};
struct tag_rows {};
struct tag_body {};


/**
 * \brief wrapper for cell objects owned by table
 *
 *
 */
template<typename TABLE> 
struct cell
{
   cell(): m_uRow(0), m_uColumn(0), m_ptable(nullptr) {}
   cell( TABLE* ptable, uint64_t uRow, unsigned uColumn ): m_ptable(ptable), m_uRow(uRow), m_uColumn(uColumn) {}

   cell& operator=( const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, m_uColumn, variantviewValue, tag_convert{} ); return *this; }

   operator bool() const      { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator int8_t() const    { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator int16_t() const   { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator int32_t() const   { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator int64_t() const   { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator uint8_t() const   { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator uint16_t() const  { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator uint32_t() const  { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator uint64_t() const  { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator float()  const    { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator double() const    { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator void*() const     { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator const char*() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
#if defined(__cpp_char8_t)
   operator const char8_t*() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
#endif
   operator const wchar_t*() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }
   operator const unsigned char*() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }

   operator std::string() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ).as_string(); }
   operator std::wstring() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ).as_wstring(); }
   operator std::string_view() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ).as_string_view(); }

   gd::variant as_variant( const std::string_view& stringType );
   gd::variant as_variant( unsigned uType );

   
   //operator gd::variant_view() const { return m_ptable->cell_get_variant_view( m_uRow, m_uColumn ); }

   TABLE* m_ptable;		   ///< table pointer rows acts as a container for cell
   uint64_t m_uRow;        ///< index to row for cell
   unsigned m_uColumn;     ///< index to column for cell
};

/// return cell value as variant with specified type
template<typename TABLE> 
gd::variant cell<TABLE>::as_variant( const std::string_view& stringType ) {
   unsigned uType = gd::types::type_g( stringType );
   return as_variant( uType );
}

/// return cell value as variant with specified type
template<typename TABLE> 
gd::variant cell<TABLE>::as_variant( unsigned uType ) {
   gd::variant_view variantviewValue = m_ptable->cell_get_variant_view( m_uRow, m_uColumn );
   gd::variant variantConvertTo;
   variantviewValue.convert_to( uType, variantConvertTo );
   return variantConvertTo;
}



/**
 * \brief wrapper for column objects owned by table
 *
 *
 */
template<typename TABLE> 
struct columns
{
   using iterator = typename TABLE::column_iterator;
   using const_iterator = typename TABLE::column_const_iterator;
   using value_type = typename TABLE::column_value_type;
   using reference = value_type&;
   using const_reference = const value_type&;
   using iterator_category = std::random_access_iterator_tag;

// ## construction -------------------------------------------------------------

   columns(): m_ptable(nullptr) {}
   columns( TABLE* ptable ): m_ptable(ptable) {}
   ~columns() {}

   reference operator[]( std::size_t uIndex ) { return m_ptable->column_get( uIndex ); }
   const_reference& operator[]( std::size_t uIndex ) const { return m_ptable->column_get( uIndex ); }

   iterator        begin() { return m_ptable->column_begin(); }
   iterator        end() { return m_ptable->column_end(); }
   const_iterator  begin() const { return m_ptable->column_begin(); }
   const_iterator  end() const { return m_ptable->column_end(); }
   const_iterator  cbegin() const { return m_ptable->column_begin(); }
   const_iterator  cend() const { return m_ptable->column_end(); }

   std::size_t size() const { return m_ptable->column_size(); }
   bool empty() const { return m_ptable->column_empty(); }

// ## attributes
   TABLE* m_ptable;		   ///< table pointer columns acts as a container for
};


/**
 * \brief wraps a single row in table
 *
 *
 */
template<typename TABLE> 
struct row
{
   // ## construction -------------------------------------------------------------

   row() : m_ptable( nullptr ) {}
   row( TABLE* ptable, uint64_t uRow ): m_uRow(uRow), m_ptable(ptable) {}
   ~row() {}

   cell<TABLE> operator[]( uint32_t uIndex ) { return cell<TABLE>( m_ptable, m_uRow, uIndex ); }
   cell<TABLE> operator[]( const std::string_view& stringName ) const { return cell<TABLE>( m_ptable, m_uRow, m_ptable->column_get_index(stringName) ); }

   uint64_t get_row() const { return m_uRow; }

   gd::variant_view cell_get_variant_view( unsigned uIndex ) const { return m_ptable->cell_get_variant_view( m_uRow, uIndex ); }
   gd::variant_view cell_get_variant_view( const std::string_view& stringName ) const { return m_ptable->cell_get_variant_view( m_uRow, stringName ); }
   std::vector< gd::variant_view > cell_get_variant_view() const { return m_ptable->cell_get_variant_view( m_uRow ); }

   void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, uColumn, variantviewValue ); }
   void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue ) { m_ptable->cell_set( m_uRow, stringName, variantviewValue ); }
   void cell_set( unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert ) { m_ptable->cell_set( m_uRow, uColumn, variantviewValue, tag_convert{} ); }
   void cell_set( const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert ) { m_ptable->cell_set( m_uRow, stringName, variantviewValue, tag_convert{} ); }

   size_t size() const { return m_ptable->get_column_count(); }

   // ## attributes
   uint64_t m_uRow;     ///< active row index 
   TABLE* m_ptable;		///< table pointer rows acts as a container for
};




/**
 * \brief wrapper for rows objects owned by table
 *
@code
gd::table::table_column_buffer read()  
{
   gd::table::table_column_buffer table( 2 );
   table.column_add( { {"int16", 0, "level"},
                       {"uint64", 0, "id"},
                       {"int32", 0, "birth_time_step"},
                       {"int32", 0, "death_time_step"},
                       {"int32", 0, "body_type_id"},
                       {"int32", 0, "graph_id"},
                       {"int32", 0, "trajectory"} }, gd::table::tag_type_name{} );
   table.prepare();

   table.row_add( {0,0,0,0,0,0,0}, gd::table::tag_convert() );
   table.row_add( {1,1,1,1,1,1,1}, gd::table::tag_convert() );

   return table;
}



TEST_CASE( "Test memory", "[table]" ) {
   auto table = read();
   for( auto it : table.rows() )
   {
      it[3] = 5;
      int32_t i = it[3];

      i = it["birth_time_step"];
      i = i + 1;
      it["birth_time_step"] = i;
   }
}

@endcode
 *
 */
template<typename TABLE> 
struct rows
{
   using iterator = typename TABLE::row_iterator;
   using const_iterator = typename TABLE::row_const_iterator;
   using value_type = typename TABLE::row_value_type;
   using iterator_category = std::random_access_iterator_tag;

// ## construction -------------------------------------------------------------

   rows(): m_ptable(nullptr) {}
   rows( TABLE* ptable ): m_ptable(ptable) {}
   ~rows() {}

   value_type operator[]( std::size_t uIndex ) { return m_ptable->row_get( uIndex, tag_cell{} ); }
   const value_type operator[]( std::size_t uIndex ) const { return m_ptable->row_get( uIndex, tag_cell{} ); }

   iterator        begin() { return m_ptable->row_begin(); }
   iterator        end() { return m_ptable->row_end(); }
   const_iterator  begin() const { return m_ptable->row_begin(); }
   const_iterator  end() const { return m_ptable->row_end(); }
   const_iterator  cbegin() const { return m_ptable->row_begin(); }
   const_iterator  cend() const { return m_ptable->row_end(); }

   std::size_t size() const { return m_ptable->row_size(); }
   bool empty() const { return m_ptable->row_empty(); }

// ## attributes
   TABLE* m_ptable;		   ///< table pointer rows acts as a container for
};


/**
 * @brief range object works on a range area in table
 *
 * range holds top left and bottom right position.
 */
struct range
{
// ## construction ------------------------------------------------------------
   range() {}
   range( tag_null ) { clear(); }
   range( uint32_t uColumn, tag_columns ) : m_uColumn1{uColumn}, m_uColumn2{uColumn} {}
   range( uint64_t uRow, tag_rows ) : m_uRow1{uRow}, m_uRow2{uRow} {}
   range( uint64_t uRow, unsigned uColumn ) : m_uRow1{ uRow }, m_uColumn1{ uColumn }, m_uRow2{ (uint64_t)-1 }, m_uColumn2{ (unsigned)-1 } {}
   range( uint64_t uRow1, unsigned uColumn1, uint64_t uRow2, unsigned uColumn2 ) : m_uRow1{ uRow1 }, m_uColumn1{ uColumn1 }, m_uRow2{ uRow2 }, m_uColumn2{ uColumn2 } {}
   // copy
   range( const range& o ) { common_construct( o ); }
   // assign
   range& operator=( const range& o ) { common_construct( o ); return *this; }

   ~range() {}
   // common copy
   void common_construct( const range& o ) { m_uRow1 = o.m_uRow1; m_uRow2 = o.m_uRow2; m_uColumn1 = o.m_uColumn1; m_uColumn2 = o.m_uColumn2; }

// ## methods -----------------------------------------------------------------
   uint64_t r1() const noexcept { return m_uRow1; }
   uint64_t c1() const noexcept { return m_uColumn1; }
   uint64_t r2() const noexcept { return m_uRow2; }
   uint64_t c2() const noexcept { return m_uColumn2; }
   void r1( uint64_t uRow ) { m_uRow1 = uRow; }
   void r2( uint64_t uRow ) { m_uRow2 = uRow; }
   void r1( int64_t iRow ) { m_uRow1 = (uint64_t)iRow; }
   void r2( int64_t iRow ) { m_uRow2 = (uint64_t)iRow; }
   void c1( uint32_t uColumn ) { m_uColumn1 = uColumn; }
   void c2( uint32_t uColumn ) { m_uColumn1 = uColumn; }


   void top( uint64_t uRow ) { m_uRow1 = uRow; }
   void bottom( uint64_t uRow ) { m_uRow2 = uRow; }
   void left( uint32_t uColumn ) { m_uColumn1 = uColumn; }
   void right( uint32_t uColumn ) { m_uColumn2 = uColumn; }

   uint64_t height() const noexcept { assert( empty() == false ); return (m_uRow2 - m_uRow1) + 1; }
   uint32_t width() const noexcept { assert( empty() == false ); return (m_uColumn2 - m_uColumn1) + 1; }

   void rows( uint64_t uRow1, uint64_t uRow2 ) { m_uRow1 = uRow1; m_uRow2 = uRow2; }
   void columns( uint32_t uColumn1, uint32_t uColumn2 ) { m_uColumn1 = uColumn1; m_uColumn2 = uColumn2; }

   bool is_r2() const { return m_uRow2 != uint64_t(-1); }

   void clear() { m_uRow1 = uint64_t(-1); m_uRow2 = uint64_t(-1); m_uColumn1 = uint32_t(-1); m_uColumn2 = uint32_t(-1); }
   bool empty() const noexcept { return m_uRow1 == uint64_t(-1); }
   uint64_t count() const { return height() * width(); }

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   uint64_t m_uRow1;
   uint32_t m_uColumn1;
   uint64_t m_uRow2;
   uint32_t m_uColumn2;

// ## free functions ----------------------------------------------------------

};


/** ===========================================================================
 * \brief Represents a page of rows in a table, supporting paging operations.
 *
 * The page struct is used to manage a logical "page" of rows within a table, 
 * including information about the page index, page size, header and footer rows, 
 * and the total number of rows. It provides methods to navigate between pages, 
 * retrieve page boundaries, and manage page flags.
 *
 * Flags can be used to mark special page behaviors, such as absolute (header) pages 
 * or pages that include all rows. The struct also provides utility functions to 
 * calculate the first and last row indices for the current page, and to determine 
 * the total number of pages available.
 *
 * Example usage:
 * \code
 * gd::table::page p(0, 100, 2, 2, 1000);
 * uint64_t firstRow = p.first();
 * uint64_t lastRow = p.last();
 * p.next();
 * \endcode
 */
struct page
{
   enum enumFlags
   {
      eFlagNone     = 0x00, ///< no flags
      eFlagAbsolute = 0x01, ///< header page
      eFlagAll      = 0x02, ///< take all rows from page positions
   };
// ## construction ------------------------------------------------------------
   page() {}
   
   page( uint64_t uPage, uint64_t uPageSize) : m_uPage{ uPage }, m_uPageSize{ uPageSize }, m_uHeader{ 0 }, m_uFooter{ 0 } {}
   page( uint64_t uPage, uint64_t uPageSize, uint64_t uHeader, uint64_t uFooter) : m_uPage{ uPage }, m_uPageSize{ uPageSize }, m_uHeader{ uHeader }, m_uFooter{ uFooter } {}
   page(uint64_t uPage, uint64_t uPageSize, uint64_t uHeader, uint64_t uFooter, uint64_t uRowCount) : m_uPage{ uPage }, m_uPageSize{ uPageSize }, m_uHeader{ uHeader }, m_uFooter{ uFooter }, m_uRowCount{ uRowCount } {}
   // copy
   page(const page& o) { common_construct(o); }
   // assign
   page& operator=(const page& o) { common_construct(o); return *this; }

   ~page() {}
   // common copy
   void common_construct(const page& o) {
      m_uPage     = o.m_uPage;
      m_uPageSize = o.m_uPageSize;
      m_uHeader   = o.m_uHeader;
      m_uFooter   = o.m_uFooter;
      m_uRowCount = o.m_uRowCount;
   }

// ## operator ----------------------------------------------------------------
   page& operator++() { m_uPage += 1; return *this; }
   page& operator--() { assert( m_uPage > 0 ); m_uPage -= 1; return *this; }

// ## methods -----------------------------------------------------------------
   bool is_all() const noexcept { return ( m_uFlags & eFlagAll ) != 0; }
   bool is_end_of_table() const noexcept { return ( m_uPage >= get_page_count() ); } ///< check if page is end of table
   bool isEOF() const noexcept { return ( m_uPage >= get_page_count() ); } ///< check if page is end of table (EOF)

   uint64_t get_page() const noexcept           { return m_uPage; }
   uint64_t get_page_size() const noexcept      { return m_uPageSize; }
   uint64_t get_header() const noexcept         { return m_uHeader; }
   uint64_t get_footer() const noexcept         { return m_uFooter; }
   uint64_t get_row_count() const noexcept      { return m_uRowCount - (m_uFooter + m_uFooter); }
   void     set_page( uint64_t uPage )          { m_uPage = uPage; }
   void     set_page_size( uint64_t uPageSize ) { m_uPageSize = uPageSize; }
   void     set_header( uint64_t uHeader )      { m_uHeader = uHeader; }
   void     set_footer( uint64_t uFooter )      { m_uFooter = uFooter; }
   void     set_row_count(uint64_t uRowCount)   { m_uRowCount = uRowCount; }

   void     set_flags(uint64_t uSet, uint64_t uClear) { m_uFlags |= uSet; m_uFlags &= ~uClear; }
   void     set_flags(uint64_t uFlags) { m_uFlags = uFlags; }

   void set_page(uint64_t uPage, uint64_t uPageSize, uint64_t uHeader, uint64_t uFooter) { m_uPage = uPage; m_uPageSize = uPageSize; m_uHeader = uHeader; m_uFooter = uFooter; }

   uint64_t first() const noexcept { return m_uPage * m_uPageSize; } ///< first row in page
   uint64_t last() const noexcept { return first() + m_uPageSize - 1; } ///< last row in page

   /// go to next page in table, (sets page to next page in table)
   bool next() { if( m_uPage < get_page_count() ) { m_uPage++; return true; } return false; }
   /// got to last page in table, (sets page to last page in table)
   void goto_last_page() { m_uPage = get_page_count(); }
   /// go to first page in table (sets page to first page in table)
   void goto_first_page() { m_uPage = 0; }

   /// get number of pages available for table
   uint64_t get_page_count() const noexcept;


// ## attributes --------------------------------------------------------------
   uint64_t m_uFlags    = eFlagNone;///< flags used to customize page behaviour
   uint64_t m_uPage     = 0;        ///< Index for page
   uint64_t m_uPageSize = 0;        ///< Number of rows in each page
   uint64_t m_uHeader   = 0;        ///< Number of rows in header
   uint64_t m_uFooter   = 0;        ///< last column in page
   uint64_t m_uRowCount = 0;        ///< Number of rows for table page is working on

};

inline uint64_t page::get_page_count() const noexcept {                                            assert( m_uPageSize != 0 ); 
   if( m_uRowCount < m_uPageSize ) return 1;
   return (uint64_t)ceil( get_row_count() / m_uPageSize ); 
}



/**
 * @brief Used for columns without name
*/
constexpr const char* pbszNoName_g = "";


/** ===========================================================================
 * \brief keep constant strings in one single buffer
 * 
 * Useful for names that do not change like column names in tables
 * 
 * With **names** it is possible to store offset position to name. Suitable when you
 * do not want to create simple classes that only has primitive type members.
 * You can then copy/move this memory around without the need to do internal heap
 * allocations.
 * But be aware that to get the actual name you need to combine offset position stored
 * in a member with the **names** buffer.
 * 
 * When owner class is destroyed names object is destroyed and offset positions
 * used for that names object do not work anymore.
 * 
 * @note Note that names is not used to store huge amounts of names or very large
 * texts. Total size for all names stored in names should fit in buffer with max size
 * of 64K.
 * 
 * *buffer format, how name is stored in memory `0` = zero terminator*
 * 'SSname1_value0SSname2_value0SSname3_value0SSname4_value0'
 * - SS = length is stored in two bytes as `unsigned short` 
 * - nameX-value = name text as utf8
 * - 0 = zero terminator
 * 
 */
struct names
{
   names(): m_uSize{0}, m_uMaxSize{0}, m_pbBufferNames{nullptr} {}
   names( const names& o ) noexcept : m_uSize{0}, m_uMaxSize{0}, m_pbBufferNames{nullptr} {
      reserve( m_uMaxSize );
      m_uSize = o.m_uSize;
      memcpy( m_pbBufferNames, o.m_pbBufferNames, m_uSize );
   }
   names( names&& o ) noexcept {
      m_uSize = o.m_uSize;
      m_uMaxSize = o.m_uMaxSize;
      m_pbBufferNames = o.m_pbBufferNames;
      o.m_pbBufferNames = nullptr;
   }
   names& operator=( const names& o ) { 
      delete [] m_pbBufferNames;
      m_pbBufferNames = nullptr;
      m_uMaxSize = 0;
      reserve( o.m_uSize );
      memcpy( m_pbBufferNames, o.m_pbBufferNames, o.m_uSize );
      m_uSize = o.m_uSize;
      return *this; 
   }
   names& operator=( names&& o ) noexcept { 
      m_uSize = o.m_uSize;
      m_uMaxSize = o.m_uMaxSize;
      m_pbBufferNames = o.m_pbBufferNames;
      o.m_pbBufferNames = nullptr;
      return *this; 
   }
   ~names() { delete [] m_pbBufferNames; }

   operator const char* () const { return m_pbBufferNames; }

   /// Add name to names, returns offset position to added name
   uint16_t add( const std::string_view& stringName );
   /// Get name at offset position as std::string_view
   std::string_view get( unsigned uOffset ) const { return get_name_s( m_pbBufferNames, uOffset ); }
   /// Get last used position in internal buffer
   uint16_t last_position() const noexcept { return m_uSize; }
   /// Returns true if no names exists, false if there are names
   bool empty() const noexcept { return m_pbBufferNames == nullptr; }
   /// Reserve memory to store names 
   void reserve( unsigned uSize );
   /// Free allocated memory and reset members
   void clear() {
      delete [] m_pbBufferNames; 
      m_uSize = 0; m_uMaxSize = 0; m_pbBufferNames = nullptr;
   }

   /// total size for all names in buffer, to make it more compatible with stl
   size_t size() const noexcept { assert(m_uSize < 0xf000); return (size_t)m_uSize; }
   /// access internal buffer
   char* data() const noexcept { return m_pbBufferNames; }

   
   uint16_t m_uSize;       ///< Number of bytes used in buffer
   unsigned m_uMaxSize;    ///< total buffer size in bytes
   char* m_pbBufferNames;  ///< buffer where names are stored

   /// How much to grow buffer if more memory is needed
   static const unsigned m_uBufferGrowBy_s = 256;

   /// get name from offset in buffer 
   static std::string_view get_name_s( const char* pbBuffer, unsigned uOffset ) { assert( uOffset < 0x90000 ); // realistic ?
      return std::string_view( pbBuffer + uOffset, *(uint16_t*)(pbBuffer + (uOffset - sizeof(uint16_t))) ); 
   }
};

/**
 * \brief reference store blob data, binary or string
 *
 * Note that reference and references work together and they use a trick to minimize
 * memory allocations. `references` stores a list of reference object and data in same
 * allocated memory block. The first part in memory block is data used by reference object
 * and after comes the data.
 * memory block - [reference... data...]
 */
struct reference
{
   // ## construction -------------------------------------------------------------

   reference(): m_iReferenceCount(-1), m_uType(0), m_uSize(0), m_uLength(0), m_uCapacity(0) {}
   reference( unsigned uType, unsigned uSize ): m_iReferenceCount(1), m_uType(uType), m_uLength(uSize), m_uSize(uSize), m_uCapacity(uSize) {}
   reference( unsigned uType, unsigned uLength, unsigned uSize ): m_iReferenceCount(1), m_uType(uType), m_uLength(uLength), m_uSize(uSize), m_uCapacity(uSize) { assert(uLength <= uSize); }

   ~reference() {}

   /// return references (users) of data
   unsigned ctype() const noexcept { return m_uType; }
   unsigned length() const noexcept { return m_uLength; }
   unsigned size() const noexcept { return m_uSize; }
   unsigned capacity() const noexcept { return m_uCapacity; }

   int reference_count() const noexcept { return m_iReferenceCount; }
   void add_reference() { m_iReferenceCount++; }
   void release() { m_iReferenceCount--; }

   uint8_t* data() const { return (uint8_t*)this + sizeof(reference); }
   uint8_t* data_end() const { return (uint8_t*)this + sizeof(reference) + length(); }
   uint8_t* data_end( unsigned uOffset ) const { return (uint8_t*)this + sizeof(reference) + length() + uOffset; }

   void set_size( unsigned uSize ) { assert( uSize <= m_uCapacity ); m_uSize = uSize; }


   // ## attributes
   int m_iReferenceCount;  ///< reference counter
   unsigned m_uType;		   ///< Type of value that is store, @see:
   unsigned m_uLength;     ///< Work length for value, for example this will show string length (differs from needed allocation size)
   unsigned m_uSize;       ///< item size 
   unsigned m_uCapacity;   ///< max buffer size (capacity to store memory for this reference)


#if DEBUG_RELEASE > 0
   /// check internal state, throws if there are some error
   void assert_valid_d() const;
   std::string dump_d() const;
   /// clone working buffer to clone buffer used to check for unwanted writes
   void clone_d();
   uint8_t* data_clone_d() const { return (uint8_t*)m_puClone_d + sizeof(reference); }
   unsigned m_uAllocated_d;///< size allocated in debug
   uint8_t* m_puClone_d = nullptr; ///< copy of working buffer to check for overwrites

   bool compare_d() const { return ( memcmp( m_puClone_d, this, m_uAllocated_d ) == 0 ); }
   void delete_d() { assert( m_puClone_d != nullptr ); 
      delete [] m_puClone_d; m_puClone_d = nullptr; }
#endif // DEBUG_RELEASE


   // static void delete_reference_s( void* p ) { delete [] (uint8_t*)p; }
};




/**
 * \brief container for reference items storing blob data
 *
 * Blob items managed by references should not be deleted until where the data is
 * used is deleted. It is not coded to be able to remove and insert values into
 * references object.
 * 
 * @code
// add three values into and find one of those
gd::table::references referencesTest;
referencesTest.add( "123" );
referencesTest.add( "456" );
referencesTest.add( "789" );
auto iIndex = referencesTest.find( "456" );
 * @endcode
 * 
 * @code
// table using references internally to store blob information,
// here you do not need to set max column size
gd::table::table_column_buffer tableTest( 100 );                               // pre allocate 100 rows
tableTest.column_add( { { "rstring", 0}, { "rstring", 0}, { "rstring", 0} }, gd::table::tag_type_name{} ); // add columns
tableTest.prepare();                                                           // prepare table

tableTest.row_add();                                                           // add row to table
tableTest.cell_set( 0, 0, "123" );
tableTest.cell_set( 0, 1, "456" );
tableTest.cell_set( 0, 2, "789" );
tableTest.row_add();                                                           // and another row
tableTest.cell_set( 1, 0, "123" );
tableTest.cell_set( 1, 1, "456" );
tableTest.cell_set( 1, 2, "789" );

// add get value from row 1 and cell 0
{
   auto v_ = tableTest.cell_get_variant_view( 1, 0 );
   auto s_ = v_.as_string();
}

// copy table add get value from row 1 and cell 0
{
   gd::table::table_column_buffer tableTest2( tableTest );
   auto v_ = tableTest2.cell_get_variant_view( 1, 0 );
   auto s_ = v_.as_string();
}
 * @endcode
 */
struct references
{
   // ## construction -------------------------------------------------------------

   references() {}
   references( const references& o );
   references( references&& o ) noexcept {
      m_vectorReference = std::move( m_vectorReference );
   }
   references& operator=( const references& o );
   references& operator=( references&& o ) noexcept {
      m_vectorReference = std::move( o.m_vectorReference );
      return *this;
   }
   ~references() {
#if DEBUG_RELEASE > 0
      for( auto& it : m_vectorReference ) { ((reference*)it.get())->delete_d(); }
#endif
   }

   /// adds value to references internal list of values
   uint64_t add( const gd::variant_view& v_ );
   /// set blob value to value with specified index
   void set( uint64_t uIndex, const uint8_t* puData, unsigned uSize );

   /// Return pointer to reference item in internal list
   reference* at( std::size_t uIndex ) const noexcept { return (reference*)m_vectorReference[uIndex].get(); }
   /// Find index for value if it exist in internal list
   int64_t find( const gd::variant_view& variantviewFindValue ) const noexcept;
   /// Add to reference counter for specific value at index
   void add_reference( std::size_t uIndex ) { assert( uIndex < m_vectorReference.size() ); at(uIndex)->add_reference(); }
   /// Get number of reference items in internal list
   std::size_t size() const noexcept { return m_vectorReference.size(); }
   /// Returns whether the references is empty (i.e. no references added).
   bool empty() const noexcept { return m_vectorReference.empty(); }

   /// Allocate memory for reference object and return pointer to reference item
   reference* allocate( const reference& r_ );
   reference* allocate( const uint8_t* puData ) { return allocate( *(reference*)puData ); }


   // ## attributes
   //std::vector<std::unique_ptr< reference, decltype(&reference::delete_reference_s) > > m_vectorReference;
   std::vector< std::unique_ptr<uint8_t[]> > m_vectorReference;

   static void copy_data_s( reference* preference, const uint8_t* puData, unsigned uSize );
};

// ## helper object used to pass table information as arguments
namespace argument
{

   /**
    * \brief column is a data transfer object that can be used to create or work on table columns
    *
    * `column` hold information
    */
   struct column
   {
   // ## construction ------------------------------------------------------------
      column() {}
      // copy
      column(const column& o) { common_construct(o); }
      // assign
      column& operator=(const column& o) { common_construct(o); return *this; }

      ~column() {}
      // common copy
      void common_construct(const column& o) {}

   // ## methods -----------------------------------------------------------------
      unsigned type() const { return m_uType; }
      void type( unsigned uType ) { m_uType = uType; }
      unsigned size() const { return m_uSize; }
      void size( unsigned uSize ) { m_uSize = uSize; }
      std::string_view name() const { return m_stringName; }
      void name( const std::string_view& stringName ) { m_stringName = stringName; }
      std::string_view alias() const { return m_stringAlias; }
      void alias( const std::string_view& stringAlias ) { m_stringAlias = stringAlias; }

      void clear() noexcept { m_uSize = 0; m_stringName = std::string_view{}; m_stringAlias = std::string_view{}; }


   // ## attributes --------------------------------------------------------------
      unsigned m_uType = 0;
      unsigned m_uSize = 0;
      std::string_view m_stringName;
      std::string_view m_stringAlias;
   };

}

/// read argument column from vector with strings
std::pair<bool, std::string> assign_to_column_g( argument::column& c_, const std::vector<std::string_view>& vectorColumnData );







_GD_TABLE_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
