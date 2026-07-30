// @FILE [tag: table, simd] [description: Base class for tables optimized for data transfer] [type: class] [name: table_base]

#include "gd_table_simd.h"

#include <variant>

#include "gd_utf8.h"
#include "gd_utf8_2.h"
#include "gd_variant.h"

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>  // _aligned_malloc och _aligned_free
#endif

_GD_TABLE_SIMD_BEGIN

#include <cstdlib>   // För posix_memalign och free
#include <cstdint>   // För uint8_t och uint64_t

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h>  // För _aligned_malloc och _aligned_free
#endif

namespace internal {

/**
 * Allokerar minne med en specifik alignment (justering).
 * Fungerar på Windows, Linux, macOS (Intel & Apple Silicon/ARM).
 */
inline uint8_t* allocate( uint64_t uSize, size_t uAlignment = 64 ) {
#if defined(_MSC_VER) || defined(__MINGW32__)
   return static_cast<uint8_t*>( _aligned_malloc( uSize, uAlignment ) );
#else
   void* pTmp = nullptr;
   if( posix_memalign( &pTmp, uAlignment, uSize ) == 0 ) {
      return static_cast<uint8_t*>( pTmp );
   }
   return nullptr;
#endif
}

/**
 * Frigör minne som allokerats med internal::allocate.
 */
inline void deallocate( uint8_t* pData ) {
   if( pData == nullptr ) return;

#if defined(_MSC_VER) || defined(__MINGW32__)
   _aligned_free( pData );
#else
   free( pData ); // posix_memalign frigörs med vanlig free()
#endif
}

} // namespace internal

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( _MSC_VER )
   #pragma warning( disable: 4996 26812 )
#endif



/// @brief constructor adding columns with type, size and name to table
table_base::table_base(unsigned uFlags, const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorValue) :
   m_uFlags(uFlags), m_uRowSize(0), m_uRowGrowBy(0), m_uRowCount(0), m_uRowReservedPackCount(eSpaceFirstAllocate)
{
   m_pcolumns = new_columns_s();
   for(const auto& it : vectorValue)
   {
      column_add(std::get<0>(it), std::get<1>(it), std::get<2>(it));
   }
}

/** ---------------------------------------------------------------------------
 * @brief construct table from one single variant view value
@code
// create table with one column storing string value, the buffer length is calculated based on string length
gd::table_base::table_base_column_buffer t1( "0123456789", gd::table::tag_prepare{} );
assert( t1.cell_get_variant_view( 0, 0 ).as_string() == "0123456789" );

// create table from one int64_t value
gd::table_base::table_base_column_buffer t2( (int64_t)123456789123456789, gd::table::tag_prepare{} );
assert( t2.cell_get_variant_view( 0, 0 ) == gd::variant_view( ( int64_t )123456789123456789 ) );
@endcode
 * @param variantviewValue variant view value to generate table from
*/
table_base::table_base( gd::variant_view variantviewValue, tag_prepare, unsigned uValueSize, unsigned uPackCount ) :
   m_uFlags(0), m_uRowSize(0), m_uRowGrowBy(0), m_uRowCount(0), m_uRowReservedPackCount( 1 )
{
   m_pcolumns = new_columns_s();
   auto type_ = variantviewValue.type();
   auto size_ = variantviewValue.is_primitive() ? 0 : variantviewValue.length();

   column_add( type_, size_ );

   prepare();
   row_add();

   cell_set( 0, 0, variantviewValue );
}

/** ---------------------------------------------------------------------------
 * @brief Construct table and prepare for adding rows, columns are generated based on type name and column name
 * @param uFlags type of state table works under
 * @param vectorColumn vector with typle that has column type as string and column name
*/
table_base::table_base( const std::vector< std::string_view >& vectorColumn, tag_prepare, unsigned uValueSize, unsigned uPackCount ) :
    m_uFlags( 0 ), m_uRowSize( 0 ), m_uRowGrowBy( 0 ), m_uRowCount( 0 ), m_uRowReservedPackCount( eSpaceFirstAllocate )
{
   m_pcolumns = new_columns_s();

   for( const auto& it : vectorColumn )
   {
      column_add( it, 0 );
   }

   prepare();
}


/** ---------------------------------------------------------------------------
 * @brief Construct table and prepare for adding rows, columns are generated based on type name and column name
 * @param uFlags type of state table works under
 * @param vectorColumn vector with typle that has column type as string and column name
*/
table_base::table_base( unsigned uFlags, const std::vector< std::tuple<std::string_view, std::string_view>>& vectorColumn, tag_prepare, unsigned uValueSize, unsigned uPackCount ) :
    m_uFlags( uFlags ), m_uRowSize( 0 ), m_uRowGrowBy( 0 ), m_uRowCount( 0 ), m_uRowReservedPackCount( eSpaceFirstAllocate )
{
   m_pcolumns = new_columns_s();

   for( const auto& it : vectorColumn )
   {
      column_add( std::get<0>( it ), std::get<1>( it ) );
   }

   prepare();
}

/** ---------------------------------------------------------------------------
 * @brief Construct table and prepare for adding rows, columns are generated based on type name and column name
 * @param uFlags type of state table works under
 * @param vectorColumn vector with typle that has column type as string, value size for types that need it and column name
 * @param uValueSize value size for types that need it
 * @param uPackCount pack count for types that need it
*/
table_base::table_base( unsigned uFlags, const std::vector<std::tuple<std::string_view, unsigned, std::string_view>>& vectorColumn, tag_prepare, unsigned uValueSize, unsigned uPackCount ) :
   m_uFlags( uFlags ), m_uRowSize( 0 ), m_uRowGrowBy( 0 ), m_uRowCount( 0 ), m_uRowReservedPackCount( eSpaceFirstAllocate )
{
   m_pcolumns = new_columns_s();

   for( const auto& it : vectorColumn )
   {
      column_add( std::get<0>( it ), std::get<1>( it ), std::get<2>( it ) );
   }

   prepare();
}


/** ---------------------------------------------------------------------------
 * @brief construct table, prepare buffer and insert values to one single row
 * Construct table if you do not heed details about what happens, maybe just want to pass single or a couple of values in one row
@code
// create table with one column storing string value with max 10 characters and add value
gd::table_base::table_base_column_buffer t1( { { "string", 10, "FName", "0123456789" } }, gd::table::tag_prepare{} );
assert( t1.cell_get_variant_view( 0, "FName" ).as_string() == "0123456789" );  // compare value at R0C0 (C0 = "FName")

// create table with one column storing integer 64 bit value, add some values and compare
gd::table_base::table_base_column_buffer t2( { { "int64", 0, "FInteger", (int64_t)123456789123456789 } }, gd::table::tag_prepare{} );
assert( t2.cell_get_variant_view( 0, "FInteger" ) == gd::variant_view((int64_t)123456789123456789) );
t2.row_add( { {1} }, gd::table::tag_convert{} );
t2.row_add( { {2} }, gd::table::tag_convert{} );
assert( t2.cell_get_variant_view( 2, "FInteger" ) == gd::variant_view((int64_t)2) );
@endcode
 * 
 * @param vectorValue tuple with four values
 * @param vectorValue.[0] type name for column
 * @param vectorValue.[1] buffer size for derived types (primitive types do not need size because table know the size)
 * @param vectorValue.[2] column name
 * @param vectorValue.[3] value inserted to table at first row
 * @param uValueSize value size for types that need it
 * @param uPackCount pack count for types that need it
*/
table_base::table_base( const std::vector<std::tuple<std::string_view, unsigned, std::string_view, gd::variant_view>>& vectorValue, tag_prepare, unsigned uValueSize, unsigned uPackCount ) :
   m_uFlags(0), m_uRowSize(0), m_uRowGrowBy(0), m_uRowCount(0), m_uRowReservedPackCount( 1 )
{
   m_pcolumns = new_columns_s();

   for( const auto& it : vectorValue )
   {
      column_add( std::get<0>( it ), std::get<1>( it ), std::get<2>( it ) );
   }

   prepare();
   row_add();

   for( unsigned u = 0, uMax = (unsigned)vectorValue.size(); u < uMax; u++ )
   {
      cell_set( 0, u, std::get<3>(vectorValue[u] ), tag_convert{});
   }
}

/** ---------------------------------------------------------------------------
 * @brief construct table from another table (creates a copy)
 * @note Do not call this method externally, only for internal use
 * @param o reference to table to construct from
*/
void table_base::common_construct( const table_base& o ) {
   m_uFlags             = o.m_uFlags; 
   m_uValueSize         = o.m_uValueSize;
   m_uPackCount         = o.m_uPackCount;
   m_uRowSize           = o.m_uRowSize;  
   m_uRowMetaSize       = o.m_uRowMetaSize;
   m_uRowCount          = o.m_uRowCount; 
   m_uRowReservedPackCount = o.m_uRowReservedPackCount;

   delete m_puData;

   m_pcolumns = o.m_pcolumns;

   if( o.m_puData != nullptr )
   {
      uint64_t uTotalSize = size_reserved_total();
      m_puData = new uint8_t[uTotalSize];
      memcpy( m_puData, o.m_puData, uTotalSize );

      // ## check if copied table has meta data
      if( o.m_puMetaData != nullptr ) { m_puMetaData = m_puData + (m_uRowReservedPackCount * m_uRowSize); assert( m_uFlags != 0 ); }
      else                            { m_puMetaData = nullptr; }
   }
   else
   {
      m_puData = nullptr;
      m_puMetaData = nullptr;
   }
   m_references = o.m_references;
   m_argumentsProperty = o.m_argumentsProperty;
#ifndef NDEBUG
   //m_uAllocatedBlockSize_d = size_reserved_total();
#endif // NDEBUG

}

/** ---------------------------------------------------------------------------
 * @brief construct table from another table (creates a copy)
 * @note Do not call this method externally, only for internal use
 * @param o reference to table to construct from
*/
void table_base::common_construct( const table_base& o, tag_columns )
{
   m_uFlags             = o.m_uFlags; 
   m_uValueSize         = o.m_uValueSize;
   m_uPackCount         = o.m_uPackCount;
   m_uRowSize           = o.m_uRowSize;
   m_uRowMetaSize       = o.m_uRowMetaSize;
   m_uRowCount          = 0; 
   m_uRowReservedPackCount = 0;

   delete m_puData;
   m_puData = nullptr;
   m_puMetaData = nullptr;

   m_pcolumns = o.m_pcolumns;
   m_pcolumns->add_reference();

   m_argumentsProperty = o.m_argumentsProperty;
}

void table_base::common_construct( const table_base& o, tag_body )
{
   m_uFlags             = o.m_uFlags; 
   m_uValueSize         = o.m_uValueSize;
   m_uPackCount         = o.m_uPackCount;
   m_uRowSize           = o.m_uRowSize;  
   m_uRowMetaSize       = o.m_uRowMetaSize;
   m_uRowCount          = o.m_uRowCount; 
   m_uRowReservedPackCount = o.m_uRowReservedPackCount;

   delete m_puData;

   if( o.m_puData != nullptr )
   {
      uint64_t uTotalSize = size_reserved_total();
      m_puData = new uint8_t[uTotalSize];
      memcpy( m_puData, o.m_puData, uTotalSize );

      // ## check if copied table has meta data
      if( o.m_puMetaData != nullptr ) { m_puMetaData = m_puData + (m_uRowReservedPackCount * m_uRowSize); assert( m_uFlags != 0 ); }
      else                            { m_puMetaData = nullptr; }
   }
   else
   {
      m_puData = nullptr;
      m_puMetaData = nullptr;
   }

   m_references = o.m_references;
   m_argumentsProperty = o.m_argumentsProperty;
}

/** ---------------------------------------------------------------------------
 * @brief construct table from another table (creates a copy)
 * @note Do not call this method externally, only for internal use
 * @param o reference to table to construct from
*/
void table_base::common_construct( detail::columns* pcolumns )
{
   m_uFlags             = 0;
   m_uValueSize         = 0;
   m_uPackCount         = 0;
   m_uRowSize           = 0;  
   m_uRowMetaSize       = 0;
   m_uRowCount          = 0; 
   m_uRowReservedPackCount = 0;

   delete m_puData;
   m_puData = nullptr;
   m_puMetaData = nullptr;

   m_pcolumns = pcolumns;
   m_pcolumns->add_reference();
}


/** ---------------------------------------------------------------------------
 * @brief construct table with columns and prepare for adding rows
 * @param pcolumns pointer to columns object, adds reference to columns
 * @param uRowCount number of rows to reserve space for
 * @param uFlags flags for table state, see eTableFlagNull32, eTableFlagNull64, eTableFlagRowStatus
 * @param uGrowBy how many rows to grow by if table needs more space
*/
table_base::table_base(detail::columns* pcolumns, unsigned uRowCount, unsigned uFlags, unsigned uGrowBy)
{                                                                                                  assert( pcolumns );
   common_construct( pcolumns );

   m_uFlags             = uFlags;
   m_uValueSize         = 0;
   m_uPackCount         = 0;
   m_uRowSize           = 0;
   m_uRowGrowBy         = uGrowBy;
   m_uRowCount          = 0;
   m_uRowReservedPackCount = eSpaceFirstAllocate;

   prepare();

   if( uRowCount > 0 )
   {
      row_reserve_add( uRowCount );
   }
}


/** ---------------------------------------------------------------------------
 * @brief add column to table
 * @param uColumnType column type added. types are defined in gd::types and samples are
 *                    eTypeUInt32, eTypeInt64, eTypeDouble, eTypeString. Primitive types are supported
 *                    and some common extended types.
 * @param uSize size for column if(0 if primitive type and size for derived types, primitive types know the size)
 * @return reference to table
*/
table_base& table_base::column_add(unsigned uColumnType, unsigned uSize)
{                                                                                                  assert(gd::types::validate_number_type_g(uColumnType)); assert(uSize < 0x1000'0000);
   if(gd::types::is_primitive_g(uColumnType) == false) uSize = gd::types::value_size_g(uColumnType, uSize);
   return column_add(detail::column(uColumnType, uSize));
}


/** ---------------------------------------------------------------------------
 * @brief Adds column to table
 * Values are checked in debug mode and not in runtime
 * if column types are generated in runtime remember to check for validity outside method.
 * @param uColumnType value type for column
 * @param uSize if size isn't a fixed type then this is the max size for value
 * @param stringName column name
 * @param stringAlias column alias
 * @return table_column_buffer& reference to table
*/
table_base& table_base::column_add(unsigned uColumnType, unsigned uSize, std::string_view stringName, std::string_view stringAlias)
{                                                                                                  assert( gd::types::validate_number_type_g( uColumnType ) ); assert( uSize < 0x1000'0000 );
#ifndef NDEBUG
// ## if size is 4 then check for not setting larger types ....................
   if(size_value() == 4)
   {
      if(gd::types::is_primitive_g(uColumnType) == true)
      {
         assert(gd::types::detail::is_size64_g(uColumnType) == false);         // primitive types have a fixed size, no need to specify size for primitive types
      }
   }
#endif // NDEBUG
   detail::column columnAdd;

   columnAdd.type( uColumnType );
   columnAdd.ctype( uColumnType );
   columnAdd.primitive_size( gd::types::value_size_g( uColumnType ) );
   columnAdd.name( stringName );
   columnAdd.alias( stringAlias );

   if( gd::types::is_primitive_g( uColumnType ) == false && gd::types::is_reference_g( uColumnType ) == false )
   {
      uSize = gd::types::value_size_g( uColumnType, uSize );
   }

   columnAdd.size( uSize );

   m_pcolumns->add( columnAdd );
   
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with pair values
~~~(.cpp)
// create tabl with one row and three columns
gd::table::simd::table<> tableVariable( 1 );
tableVariable.column_add( { { "double", 0 }, { "double", 0 }, { "double", 0 }, { "int32", 0 } }, gd::table::tag_type_name{} );
tableVariable.prepare();
~~~
 * @param vectorType vector with pair items "<type_name, size>".
 * @param tag dispatcher to diff from other `column_add` methods.
 * @return reference to table_column_buffer to nest methods.
*/
table_base& table_base::column_add(const std::initializer_list<std::pair<std::string_view, unsigned>>& listType, tag_type_name)
{                                                                                                  assert(m_puData == nullptr);
   for(auto it = std::begin(listType), itEnd = std::end(listType); it != itEnd; it++)
   {
      column_add(it->first, it->second);
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with tuple values
~~~(.cpp)
// create tabl with one row and three columns
gd::table::simd::table<> tableVariable( 1 );
tableVariable.column_add( { { "string", 50, "FName"}, { "string", 50, "FAlias"}, { "string", 50, "FValue"} }, gd::table::tag_type_name{});
tableVariable.prepare();
~~~
 * @param vectorType vector with tuple items "<type_name, size, column_name>".
 * @param tag_type_name tag dispatcher to diff from other `column_add` methods.
 * @return reference to table_column_buffer to nest methods.
*/
table_base& table_base::column_add(const std::initializer_list<std::tuple<std::string_view, unsigned, std::string_view>>& listType, tag_type_name)
{                                                                                                  assert(m_puData == nullptr);
   for(auto it = std::begin(listType), itEnd = std::end(listType); it != itEnd; it++)
   {
      column_add(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it));
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add columns to table with none derived value types, no need for specify max value length
@code
// create table with one row and three columns
gd::table::simd::table<> tableVariable( 10 );
tableVariable.column_add( { { "int32", "x" }, { "int32", "y" } }, gd::table::tag_type_name{});
tableVariable.prepare();
@endcode
 * @param vectorType vector with pair items "<type_name, column_name>".
 * @return reference to table_column_buffer to nest methods.
*/
table_base& table_base::column_add(const std::initializer_list< std::pair< std::string_view, std::string_view > >& listType, tag_type_name)
{
   assert(m_puData == nullptr);
   for(auto it = std::begin(listType), itEnd = std::end(listType); it != itEnd; it++)
   {
#ifndef NDEBUG
      // check type, adding column without size can't be done for derived types
      auto uType_d = gd::types::type_g(std::get<0>(*it));                                        assert((gd::types::is_primitive_g(uType_d) == true) || (uType_d & gd::types::eTypeDetailReference));
#endif // !NDEBUG
      column_add(std::get<0>(*it), 0, std::get<1>(*it));
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add columns and used information from another table
 * @param table_ table that column information is found
 * @return reference to table_column_buffer to nest methods.
*/
table_base& table_base::column_add(const table_base* p_)
{                                                                                                  assert(p_ != nullptr); assert(m_pcolumns != nullptr);
   for(auto it = p_->column_begin(), itEnd = p_->column_end(); it != itEnd; it++)
   {
      detail::column columnAdd(*it); // copy column
      m_pcolumns->add(std::move(columnAdd));
   }

   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief find index to column for column name
 * @param stringName column name column index is returned for
 * @return int index to column for column name if found, -1 if not found
*/
int table_base::column_find_index(std::string_view stringName) const noexcept
{                                                                                                  assert(m_namesColumn.empty() == false);
   for(auto it = column_begin(), itEnd = column_end(); it != itEnd; it++)
   {
      if(stringName == it->name())  return (int)std::distance(column_begin(), it);
   }
   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief find index to column for column alias
 * @param stringAlias column alias column index is returned for
 * @return int index to column for column alias if foundm, -1 if not found
*/
int table_base::column_find_index( std::string_view stringAlias, tag_alias ) const noexcept
{
   for( auto it = m_pcolumns->begin(), itEnd = m_pcolumns->end(); it != itEnd; it++ )
   {
      if( stringAlias == it->alias() ) return (int)std::distance( m_pcolumns->begin(), it );
   }
   return -1;
}


/** ---------------------------------------------------------------------------
 * @brief find index to column for column name using wildcard match
 * @param stringWildcard wildcard name column index is returned for
 * @return int index to column for column name if found, -1 if not found
*/
int table_base::column_find_index(const std::string_view& stringWildcard, tag_wildcard) const noexcept
{                                                                                                  assert(m_namesColumn.empty() == false);
   for(auto it = column_begin(), itEnd = column_end(); it != itEnd; it++)
   {
      if(gd::ascii::strcmp(it->name(), stringWildcard.data(), gd::utf8::tag_wildcard{}) == 0)
      {
         return (int)std::distance(column_begin(), it);
      }
   }
   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief get index to column for column name
 * @param stringName column name column index is returned for
 * @return unsigned index to column for column name
*/
unsigned table_base::column_get_index(const std::string_view& stringName) const noexcept
{
   int iIndex = column_find_index(stringName);                                                     assert(iIndex != -1);
   return (unsigned)iIndex;
}

/** ---------------------------------------------------------------------------
 * @brief get index to column for column alias
 * @param stringAlias column alias column index is returned for
 * @return unsigned index to column for column alias
*/
unsigned table_base::column_get_index(const std::string_view& stringAlias, tag_alias) const noexcept
{
   int iIndex = column_find_index(stringAlias, tag_alias{});                                       assert(iIndex != -1);
   return (unsigned)iIndex;
}


/** ---------------------------------------------------------------------------
 * @brief get index to column for column name using wildcard match
 * @param stringWildcard column name column index is returned for
 * @return unsigned index to column for column name
*/
unsigned table_base::column_get_index(const std::string_view& stringWildcard, tag_wildcard) const noexcept
{
   assert(m_namesColumn.empty() == false);
   int iIndex = column_find_index(stringWildcard, tag_wildcard{});                                 assert(iIndex != -1);
   return (unsigned)iIndex;
}



std::pair<bool, std::string> table_base::prepare( unsigned uValueSize, unsigned uPackCount )
{                                                                                                  assert( m_pcolumns->empty() == false && "Table must have at least one column" ); assert( m_puData == nullptr && "Table already prepared" );
                                                                                                   assert(m_puData == nullptr && "Table already prepared");
                                                                                                   assert((uValueSize == 4 || uValueSize == 8) && "Value size must be 4 or 8 bytes");
                                                                                                   assert((uPackCount == 4 || uPackCount == 8 || uPackCount == 16) && "Pack count must be 4, 8, or 16");
   // ## calculate size for each row
   unsigned uRowSize = 0u; // 
   unsigned uColumnCount = (unsigned)m_pcolumns->size();

   uRowSize = uValueSize * uPackCount * uColumnCount;                            // calculate size for each row based on value size and count

   m_uRowSize = uRowSize;                                                     // final row sizes (not that each row containes a stride of columns)


   // ## calculate needed meta data size for each row
   unsigned uMetaDataSize = size_row_meta();

   m_uRowMetaSize = uMetaDataSize;

   uint64_t uTotalTableSize = (uRowSize + uMetaDataSize) * m_uRowReservedPackCount;// calculate size storing table data

   //m_puData = new (std::align_val_t(64)) uint8_t[uTotalTableSize];
   m_puData = internal::allocate( uTotalTableSize );
#ifdef _DEBUG
   memset(m_puData, 0, uTotalTableSize);                                     // set data to 0 in debug mode
#endif // _DEBUG

   if(uMetaDataSize > 0)
   {
      m_puMetaData = m_puData + (m_uRowReservedPackCount * uRowSize);              // set pointer to meta data section
      memset(m_puMetaData, 0, m_uRowReservedPackCount * uMetaDataSize);
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief append values to arguments from a specific row in a table
 * @param uRow index to row in table to add values from
 * @return argumentsValue arguments values are added to
*/
void table_base::row_get_arguments( uint64_t uRow, gd::argument::arguments& argumentsValue ) const
{                                                                                                  assert( uRow < 0x0100'0000 ); assert( uRow < count_reserved_row());
   for( auto it = column_begin(), itEnd = column_end(); it != itEnd; it++ ) 
   {
      auto stringColumnName = it->name();
      gd::variant_view variantValue = cell_get_variant_view(uRow, stringColumnName);
      // check if the cell value isn't empty, and if not then add to arguments
      if(variantValue.is_null() == false ) { argumentsValue.append_argument(stringColumnName, variantValue);  }
   }
}


/** ---------------------------------------------------------------------------
 * @brief Return row values in vector as variant view items
 * @param uRow index to row values are returned from
 * @return std::vector<const gd::variant_view> vector holding row values
*/
std::vector<gd::variant_view> table_base::row_get_variant_view( uint64_t uRow ) const
{                                                                                                  assert( uRow < count_reserved_row());
   std::vector<gd::variant_view> vectorValue;

   for( auto u = 0u, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++ )
   {
      vectorValue.push_back( cell_get_variant_view( uRow, u ) );
   }

   return vectorValue;
}

/** ---------------------------------------------------------------------------
 * @brief Return row values in vector as variant view items
 * @param uRow index to row values are returned from
 * @param puIndex pointer to array with column index values harvested into vector
 * @param uSize number of values to harvest
 * @return std::vector<gd::variant_view> values from row
*/
std::vector<gd::variant_view> table_base::row_get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const
{
   std::vector<gd::variant_view> vectorValue;
   for( unsigned u = 0; u < uSize; u++ )
   {                                                                                               assert( puIndex[u] < get_column_count() );
      vectorValue.push_back( cell_get_variant_view( uRow, puIndex[u] ) );
   }

   return vectorValue;
}

/** ---------------------------------------------------------------------------
 * @brief Harvest row values in vector with variant view items
 * @param uRow index to row values are returned from
 * @param vectorValue row values are placed in vector
*/
void table_base::row_get_variant_view(uint64_t uRow, std::vector<gd::variant_view>& vectorValue) const
{                                                                                                  assert(uRow < 0x0100'0000); assert(uRow < count_reserved_row());
   for(auto u = 0u, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++)
   {
      vectorValue.push_back(cell_get_variant_view(uRow, u));
   }
}

/** ---------------------------------------------------------------------------
 * @brief Harvest row values in vector with variant view items
 * @param uRow index to row values are returned from
 * @param uOffset start column to read values from
 * @param vectorValue row values are placed in vector
 */
void table_base::row_get_variant_view(uint64_t uRow, unsigned uOffset, std::vector<gd::variant_view>& vectorValue) const
{                                                                                                  assert(uRow < 0x0100'0000); assert(uRow < count_reserved_row());
   for(auto u = uOffset, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++)
   {
      vectorValue.push_back(cell_get_variant_view(uRow, u));
   }
}


/// add row values for column indexes sent
void table_base::row_get_variant_view(uint64_t uRow, const unsigned* puIndex, unsigned uSize, std::vector<gd::variant_view>& vectorValue) const
{
   for(unsigned u = 0; u < uSize; u++)
   {                                                                                               assert(puIndex[u] < get_column_count());
      vectorValue.push_back(cell_get_variant_view(uRow, puIndex[u]));
   }
}




void table_base::row_add(uint64_t uCount)
{                                                                                                  assert( uCount > 0);
   const uint64_t uRowCountNew = m_uRowCount + uCount;

   // ## Calculate number of row blocks needed for new row count, each block contains STRIDE number of rows
   const uint64_t uRowBlockCountNew = (uRowCountNew + count_pack() - 1) / count_pack();

   if(uRowBlockCountNew > m_uRowReservedPackCount)
   {
      row_reserve_add(uRowBlockCountNew - m_uRowReservedPackCount);
      m_uRowReservedPackCount = uRowBlockCountNew;
   }

   m_uRowCount = uRowCountNew;
}

/** ---------------------------------------------------------------------------
 * @brief Add row and set values in row, list cant be larger than amount of values in row
~~~(.cpp)
gd::table::table_column_buffer t( 1, 0, 1 ); // one row is allocated, not using row state, grow table with one reserved row if needed
t.column_add( { { "int32", 0, "number1"}, { "int32", 0, "number2"}, { "int32", 0, "number3"} }, gd::table::tag_type_name{} );
t.prepare();
t.row_add( { 0,0,0 } );
t.row_add( { 1,1,1 } );
t.row_add( { 2,2,2 } );
~~~
 * @param listValue list of values inserted in added row
*/
void table_base::row_add( const std::initializer_list<gd::variant_view>& listValue )
{                                                                                                  assert( listValue.size() <= get_column_count() );              
   uint64_t uRow = m_uRowCount;

   row_add();
   
   row_set( uRow, listValue );
}

/** ---------------------------------------------------------------------------
 * @brief Add row and set values in row, vector cant be larger than amount of values in row
~~~(.cpp)
~~~
 * @param listValue list of values inserted in added row
*/
void table_base::row_add( const std::initializer_list<gd::variant_view>& vectorValue, tag_convert )
{                                                                                                  assert( vectorValue.size() <= get_column_count() );              
   uint64_t uRow = m_uRowCount;

   row_add();
   
   row_set( uRow, vectorValue, tag_convert{} );
}

void table_base::row_add( const std::vector<gd::variant_view>& vectorValue )
{                                                                                                  assert( vectorValue.size() <= get_column_count() );              
   uint64_t uRow = m_uRowCount;

   row_add();
   
   row_set( uRow, vectorValue );
}

/** ---------------------------------------------------------------------------
 * @brief Set row values
 * @param uRow row where values are set
 * @param listValue list of values inserted to specified row
*/
void table_base::row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue )
{                                                                                                  assert( uRow < m_uRowCount ); assert( listValue.size() <= get_column_count() );   
   unsigned uIndex = 0;
   for( auto it = std::begin( listValue ), itEnd = std::end( listValue ); it != itEnd; ++it )
   {
      cell_set( uRow, uIndex, *it );
      uIndex++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set row values
 * @param uRow row where values are set
 * @param uFirstColumn row where values are set
 * @param listValue list of values inserted to specified row
*/
void table_base::row_set( uint64_t uRow, unsigned uFirstColumn, const std::initializer_list<gd::variant_view>& listValue )
{                                                                                                  assert( uRow < m_uRowCount ); assert( (listValue.size() + uFirstColumn) <= get_column_count() );   
   unsigned uIndex = uFirstColumn;
   if( is_null() == true ) row_set_null( uRow );
   for( auto it = std::begin( listValue ), itEnd = std::end( listValue ); it != itEnd; ++it )
   {
      cell_set( uRow, uIndex, *it );
      uIndex++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set row values and if column values do not match type it tries to convert to proper type
 * @param uRow row where values are set
 * @param listValue list of values inserted to specified row
*/
void table_base::row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount ); assert( listValue.size() <= get_column_count() );   
   unsigned uIndex = 0;
   if( is_null() == true ) row_set_null( uRow );
   for( auto it = std::begin( listValue ), itEnd = std::end( listValue ); it != itEnd; ++it )
   {
      cell_set( uRow, uIndex, *it, tag_convert{} );
      uIndex++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set row values and if column values do not match type it tries to convert to proper type
 * @param uRow row where values are set
 * @param uFirstColumn row where values are set
 * @param listValue list of values inserted to specified row
*/
void table_base::row_set( uint64_t uRow, unsigned uFirstColumn, const std::initializer_list<gd::variant_view>& listValue, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount ); assert( (listValue.size() + uFirstColumn) <= get_column_count() );   
   unsigned uIndex = uFirstColumn;
   for( auto it = std::begin( listValue ), itEnd = std::end( listValue ); it != itEnd; ++it )
   {
      cell_set( uRow, uIndex, *it, tag_convert{});
      uIndex++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set row values
 * @param uRow row where values are set
 * @param vectorValue vector of values inserted to specified row
*/
void table_base::row_set( uint64_t uRow, const std::vector<gd::variant_view>& vectorValue )
{                                                                                                  assert( uRow < m_uRowCount ); assert( vectorValue.size() <= get_column_count() );   
   unsigned uIndex = 0;
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; ++it )
   {
      cell_set( uRow, uIndex, *it );
      uIndex++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set row values, if value type differ it tries to convert to type in column
 * @param uRow row where values are set
 * @param vectorValue vector of values inserted to specified row
*/
void table_base::row_set( uint64_t uRow, const std::vector<gd::variant_view>& vectorValue, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount ); assert( vectorValue.size() <= get_column_count() );   
   unsigned uIndex = 0;
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; ++it )
   {
      cell_set( uRow, uIndex, *it, tag_convert{});
      uIndex++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set row values and specify column index where value is placed
 * @param uRow index to row where values are set
 * @param vectorValue vector with values
 * @param vectorColumn index to column where value is placed
*/
void table_base::row_set(uint64_t uRow, const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn)
{                                                                                                  assert(uRow < m_uRowCount); assert(vectorValue.size() == vectorColumn.size());
   for(unsigned u = 0, uMax = (unsigned)vectorValue.size(); u < uMax; u++)
   {
      unsigned uColumn = vectorColumn[u];                                                          assert(uColumn < get_column_count());
      cell_set(uRow, uColumn, vectorValue[u]);
   }
}


/** ---------------------------------------------------------------------------
 * @brief Set row values and specify column index where value is placed, if value type differ it tries to convert to type in column
 * @param uRow index to row where values are set
 * @param vectorValue vector with values
 * @param vectorColumn index to column where value is placed
*/
void table_base::row_set(uint64_t uRow, const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn, tag_convert)
{                                                                                                  assert(uRow < m_uRowCount); assert(vectorValue.size() == vectorColumn.size());
   for(unsigned u = 0, uMax = (unsigned)vectorValue.size(); u < uMax; u++)
   {
      unsigned uColumn = vectorColumn[u];                                                          assert(uColumn < get_column_count());
      cell_set(uRow, uColumn, vectorValue[u], tag_convert{});
   }
}

/** ---------------------------------------------------------------------------
 * @brief set row values
 * @param vectorValue vector with pair values set to row, first is column index and second is value
*/
void table_base::row_set(uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue)
{                                                                                                  assert(uRow < m_uRowCount);
   for(auto it = std::begin(vectorValue), itEnd = std::end(vectorValue); it != itEnd; it++)
   {
      assert(it->first < get_column_count());
      cell_set(uRow, it->first, it->second);
   }
}

/** ---------------------------------------------------------------------------
 * @brief set row values, convert to right type if value type differ from column
 * @param vectorValue vector with pair values set to row, first is column name and second is value
 */
void table_base::row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount );
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      int iIndex = column_find_index( it->first );
      if( iIndex != -1 ) cell_set( uRow, ( unsigned )iIndex, it->second, tag_convert{} );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Add values from arguments object where names in arguments match column names
 * @param argumentsRow values added to row
*/
void table_base::row_set( uint64_t uRow, const gd::argument::arguments& argumentsRow, tag_arguments )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   for( auto pPosition = argumentsRow.next(); pPosition != nullptr; pPosition = argumentsRow.next(pPosition) )
   {
      if( gd::argument::arguments::is_name_s(pPosition) == true )
      {
         auto stringName = gd::argument::arguments::get_name_s( pPosition );
         auto value_ = gd::argument::arguments::get_argument_s( pPosition ).as_variant_view();

         int iIndex = column_find_index( stringName );
         if( iIndex != -1 )
         {
            cell_set( uRow, iIndex, value_ );
         }
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set data taken from another row in table
 * @param uRow row where data is set to
 * @param uRowToCopy row where data is taken from
*/
void table_base::row_set(uint64_t uRow, uint64_t uRowToCopy)
{                                                                                                  assert( uRow < m_uRowCount ); assert( uRowToCopy <= get_column_count() );   
   // ## Copy row data
   const uint8_t* puRowToCopy = row_get( uRowToCopy );
   uint8_t* puRow = row_get( uRow );
   memcpy( puRow, puRowToCopy, m_uRowSize );                                   // copy row data

   // ## Copy row meta datadata
   puRowToCopy = row_get_meta( uRowToCopy );
   puRow = row_get_meta( uRow );
   memcpy( puRow, puRowToCopy, m_uRowMetaSize );                               // copy meta data about row
}

/** ---------------------------------------------------------------------------
 * @brief set cell values from string where string is divided based on character sent
 * @param uRow Row number where cell values are set
 * @param uFirst column where to start to insert values
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 */
void table_base::row_set(uint64_t uRow, unsigned uFirst, const std::string_view& stringRowValue, char chSplit, tag_parse)
{
   std::vector<std::size_t> vectorOffset;       // positions for each column in string
   std::vector<std::string_view> vectorValue;   // cell values

   gd::utf8::offset( stringRowValue, chSplit, vectorOffset );                  // offset column positions, they are divided with 'chSplit'
   if( stringRowValue.back() != chSplit ) vectorOffset.push_back(stringRowValue.length());// add last position to add last section without code after loop

   gd::utf8::split( stringRowValue, vectorOffset, vectorValue );

   unsigned uCoulmnCount = (unsigned)vectorValue.size() + uFirst;
   if( uCoulmnCount > get_column_count() ) { uCoulmnCount = get_column_count(); } // not more than number of columns in table?
   for( unsigned uColumn = uFirst, uIndex = 0; uColumn < uCoulmnCount; uColumn++, uIndex++ )
   {
      const auto stringValue = vectorValue.at( uIndex );
      if( stringValue.empty() == false )
      {
         cell_set( uRow, uColumn, stringValue, tag_convert{} );
      }
      else
      {
         // if null values in table then set to null, otherwise skip it
         if( is_null() == true ) { cell_set_null( uRow, uColumn ); }
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief set cell values from string where string is divided based on character sent
 * @param uRow Row number where cell values are set
 * @param puColumn pointer to column indexes where to place column values
 *        @note make shoure that there are enough values for values in string, this is not checked        
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 */
void table_base::row_set(uint64_t uRow, const unsigned* puColumn, const std::string_view& stringRowValue, char chSplit, tag_parse)
{
   std::vector<std::size_t> vectorOffset;       // positions for each column in string
   std::vector<std::string_view> vectorValue;   // cell values

   gd::utf8::offset( stringRowValue, chSplit, vectorOffset );                  // offset column positions, they are divided with 'chSplit'
   if( stringRowValue.back() != chSplit ) vectorOffset.push_back(stringRowValue.length());// add last position to add last section without code after loop

   gd::utf8::split( stringRowValue, vectorOffset, vectorValue );

   unsigned uCount = (unsigned)vectorValue.size();
   for( unsigned uIndex = 0; uIndex < uCount; uIndex++ )
   {                                                                                               assert( puColumn[uIndex] < get_column_count() );
      const auto stringValue = vectorValue.at( uIndex );
      if( stringValue.empty() == false )
      {
         cell_set( uRow, puColumn[uIndex], stringValue, tag_convert{});
      }
      else
      {
         // if null values in table then set to null, otherwise skip it
         if( is_null() == true ) { cell_set_null( uRow, puColumn[uIndex] ); }
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief set cell values from string where string is divided based on character sent
 * @param uRow Row number where cell values are set
 * @param uFirst column where to start to insert values
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 */
bool table_base::row_set(uint64_t uRow, unsigned uFirst, const std::string_view& stringRowValue, char chSplit,std::function< bool( std::vector<std::string>& vectorValue )> callback_, tag_parse)
{
   std::vector<std::size_t> vectorOffset;       // positions for each column in string
   std::vector<std::string> vectorValue;        // cell values

   gd::utf8::offset( stringRowValue, chSplit, vectorOffset );                  // offset column positions, they are divided with 'chSplit'
   if( stringRowValue.back() != chSplit ) vectorOffset.push_back(stringRowValue.length());// add last position to add last section without code after loop

   gd::utf8::split( stringRowValue, vectorOffset, vectorValue );

   if( callback_( vectorValue ) == true )
   {
      unsigned uCoulmnCount = (unsigned)vectorValue.size() + uFirst;
      if( uCoulmnCount > get_column_count() ) { uCoulmnCount = get_column_count(); } // not more than number of columns in table?
      for( unsigned uColumn = uFirst, uIndex = 0; uColumn < uCoulmnCount; uColumn++, uIndex++ )
      {
         const auto& stringValue = vectorValue.at( uIndex );
         if( stringValue.empty() == false )
         {
            cell_set( uRow, uColumn, stringValue, tag_convert{} );
         }
         else
         {
            // if null values in table then set to null, otherwise skip it
            if( is_null() == true ) { cell_set_null( uRow, uColumn ); }
         }
      }

      return true;
   }

   return false;
}


/** ---------------------------------------------------------------------------
 * @brief set cell values from string where string is divided based on character sent
 * @param uRow Row number where cell values are set
 * @param puColumn pointer to column indexes where to place column values
 *        @note make shoure that there are enough values for values in string, this is not checked        
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 */
bool table_base::row_set(uint64_t uRow, const unsigned* puColumn, const std::string_view& stringRowValue, char chSplit, std::function< bool( std::vector<std::string>& vectorValue )> callback_, tag_parse)
{
   std::vector<std::size_t> vectorOffset;       // positions for each column in string
   std::vector<std::string> vectorValue;        // cell values

   gd::utf8::offset( stringRowValue, chSplit, vectorOffset );                  // offset column positions, they are divided with 'chSplit'
   if( stringRowValue.back() != chSplit ) vectorOffset.push_back(stringRowValue.length());// add last position to add last section without code after loop

   gd::utf8::split( stringRowValue, vectorOffset, vectorValue );

   if( callback_( vectorValue ) == true )
   {
      unsigned uCount = (unsigned)vectorValue.size();
      for( unsigned uIndex = 0; uIndex < uCount; uIndex++ )
      {                                                                                            assert( puColumn[uIndex] < get_column_count() );
         const auto stringValue = vectorValue.at( uIndex );
         if( stringValue.empty() == false )
         {
            cell_set( uRow, puColumn[uIndex], stringValue, tag_convert{});
         }
         else
         {
            // if null values in table then set to null, otherwise skip it
            if( is_null() == true ) { cell_set_null( uRow, puColumn[uIndex] ); }
         }
      }
      return true;
   }

   return false;
}


/** ---------------------------------------------------------------------------
 * @brief Set raw data to row, data must be of correct size
 * @param uRow row where data is set
 * @param praw_ pointer to raw data
 */
void table_base::row_set( uint64_t uRow, const void* praw_, tag_raw )
{                                                                                                  assert( uRow < m_uRowCount ); assert( praw_ != nullptr );
   uint8_t* puRow = row_get( uRow );
   memcpy( puRow, praw_, m_uRowSize );
}



/** ---------------------------------------------------------------------------
 * @brief adds more memory storing row/rows to table
 * @param uCount number of rows to add
*/
void table_base::row_reserve_add(uint64_t uCount)
{
   uCount += m_uRowReservedPackCount;

   // ## calculate size needed to store added row count and allocate memory
   uint64_t uTotalTableSize = size_reserved_total();                           // total table memory block size for table
   uint64_t uTotalMetaSize = size_meta_total();                                // meta block size part

   uint64_t uTotalTableSizeCopyTo = size_reserved_total(uCount);               // new block size to store more rows
   uint64_t uTotalMetaSizeCopyTo = size_meta_total(uCount);                    // new meta block size

   uint64_t uCopyRowSize = uTotalTableSize - uTotalMetaSize;
                                                                                                   assert(((uTotalTableSizeCopyTo - uTotalMetaSizeCopyTo) % 4 == 0) && "Total table size must be multiple of 4");
   uint8_t* puDataCopyTo = internal::allocate( uTotalTableSizeCopyTo ); // new buffer for table data (both data and meta data)

   if(m_puData != nullptr) memcpy(puDataCopyTo, m_puData, uCopyRowSize);      // copy row data

   if(m_puMetaData != nullptr)
   {
      // ## copy meta data block to new increased table block
      uint8_t* puMetaData = puDataCopyTo + (uTotalTableSizeCopyTo - uTotalMetaSizeCopyTo);// position where meta data starts
      memcpy(puMetaData, m_puMetaData, uTotalMetaSize);                      // copy old meta data
      m_puMetaData = puMetaData;                                               // set member meta data pointer to new block
      memset(m_puMetaData + uTotalMetaSize, 0, uTotalMetaSizeCopyTo - uTotalMetaSize);// clear rest of meta data
   }
   else if(uTotalMetaSizeCopyTo > 0)
   {
      m_puMetaData = puDataCopyTo + (uTotalTableSizeCopyTo - uTotalMetaSizeCopyTo);// set meta position pointer if meta data is used
   }

   internal::deallocate( m_puData );
   m_puData = puDataCopyTo;

   m_uRowReservedPackCount = uCount;
}

/** ---------------------------------------------------------------------------
 * @brief Get pointer to reference for specified cell
 * Get reference object to cell, make sure that cell has reference
 * @param uRow row where cell value is
 * @param uColumn column for cell
 * @return pointer to reference object for cell if not null, if null then nullpointer is returned
*/
const reference* table_base::cell_get_reference( uint64_t uRow, unsigned uColumn ) const noexcept
{
   if( is_null() == true && cell_is_null( uRow, uColumn ) == true ) return nullptr;
                                                                                                   assert( m_references.size() > 0 );
   int64_t iIndex = -1;
   if(size_value() == 8 )
   {
      auto uIndex = cell_get_value64(uRow, uColumn);                                               assert(uIndex < 0x1000'0000); assert(uIndex < m_references.size()); // realistic value?
      iIndex = (int64_t)uIndex;
   }
   else
   {
      auto uIndex = cell_get_value32(uRow, uColumn);                                               assert(uIndex < 0x1000'0000); assert(uIndex < m_references.size()); // realistic value?
      iIndex = (int64_t)uIndex;
   }

   if(iIndex < 0) return nullptr;
   const reference* preference = m_references.at( iIndex );

   return preference;
}

/// Get cell value as 32 bit unsigned integer (no checks, raw value)
uint32_t table_base::cell_get_value32(uint64_t uRow, unsigned uColumn) const noexcept
{
   auto puRow = row_get( uRow ); // buffer to row
   auto uRowOffset = offset(uRow, uColumn, tag_column{});
   auto puRowValue = puRow + uRowOffset;

   uint32_t uValue = gd::types::cast_g<uint32_t>(puRowValue);
   return uValue;
}

/// Get cell value as 64 bit unsigned integer (no checks, raw value)
uint64_t table_base::cell_get_value64(uint64_t uRow, unsigned uColumn) const noexcept
{
   auto puRow = row_get( uRow ); // buffer to row
   auto uRowOffset = offset(uRow, uColumn, tag_column{});
   auto puRowValue = puRow + uRowOffset;

   uint64_t uValue = gd::types::cast_g<uint64_t>(puRowValue);
   return uValue;
}

/** ---------------------------------------------------------------------------
 * @brief get cell value as variant_view item
 * @param uRow row index for cell
 * @param uColumn column index to cell
 * @return gd::variant_view cell value
*/
gd::variant_view table_base::cell_get_variant_view( uint64_t uRow, unsigned uColumn ) const noexcept
{                                                                                                  assert( uRow < get_row_count() ); assert( uRow < (m_uRowReservedPackCount * count_pack())); assert( m_puData != nullptr );
   const auto& columnGet = *m_pcolumns->get( uColumn );// column information for value
   auto puRow = row_get( uRow ); // buffer to row

   if( is_null() == false || cell_is_null( uRow, uColumn ) == false )
   {
      auto puRowValue = puRow + columnGet.position();

      if( columnGet.is_fixed() == true )                                       // primitive type
      {
         unsigned uSize = gd::types::value_size_g( static_cast< gd::types::enumTypeNumber >(columnGet.ctype_number()) );
         if( uSize > sizeof( uint64_t ) )
         {
            return gd::variant_view( columnGet.ctype(), puRowValue, uSize );
         }
         else
         {
            uint64_t uValue = 0;
            if( uSize == sizeof( uint64_t ) ) uValue = *( uint64_t* )puRowValue;
            else                              uValue = *( uint32_t* )puRowValue;

            return gd::variant_view( columnGet.ctype(), uValue, 0 );
         }
      }
      else
      {
         if( columnGet.is_length() == true )
         {
            uint32_t uLength = *( uint32_t* )puRowValue;
            puRowValue += sizeof( uint32_t );
            return gd::variant_view( columnGet.ctype(), puRowValue, uLength );
         }
         else if( columnGet.is_reference() == true )
         {                                                                                         assert( m_references.size() > 0 ); // do we have reference values because they are needed
            // get index number to string in among reference values
            uint64_t uIndex = *(uint64_t*)puRowValue;                                              assert( uIndex < 0x1000'0000 ); // realistic value?
                                                                                                   assert( uIndex < m_references.size() );
            reference* preference = m_references.at( uIndex );
            #if DEBUG_RELEASE > 0
            preference->assert_valid_d();
            #endif
            return gd::variant_view( preference->ctype(), preference->data(), preference->length());
         }
         else { assert(false); }
      }
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief get cell value
 * @param uRow index to row where cell value is found
 * @param stringName column name for column where cell value is found
 * @return variant_view value is returned in variant view
*/
gd::variant_view table_base::cell_get_variant_view(uint64_t uRow, const std::string_view& stringName) const noexcept
{                                                                                                  assert(uRow < count_reserved_row());
   unsigned uColumnIndex = column_get_index(stringName);
   return cell_get_variant_view(uRow, uColumnIndex);
}

/** ---------------------------------------------------------------------------
 * @brief get cell values within specified column range in row
 * @param uRow index to row where values are read from
 * @param uFromColumn start column where to get values
 * @param uToColumn end column
 * @return std::vector<gd::variant_view> vector of values from
*/
std::vector<gd::variant_view> table_base::cell_get_variant_view( uint64_t uRow, unsigned uFromColumn, unsigned uToColumn ) const
{                                                                                                  assert(uRow < count_reserved_row()); assert( uFromColumn < get_column_count() ); assert( uToColumn <= get_column_count() );
   std::vector<gd::variant_view> vectorValue; // vector of values taken from row

   for( unsigned u = uFromColumn; u < uToColumn; u++ )
   {
      auto value_ = cell_get_variant_view( uRow, u );
      vectorValue.push_back( value_ );
   }

   return vectorValue;
}



/** ---------------------------------------------------------------------------
 * @brief Set cell value in table
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param uValue value set to cell
 */
void table_base::cell_set_value(uint64_t uRow, unsigned uColumn, uint32_t uValue)
{                                                                                                  assert(size_value() == 4);
                                                                                                   assert(uRow < (m_uRowReservedPackCount * count_pack())); assert(uColumn < m_pcolumns->size());
   auto puRow = row_get(uRow);
   auto uRowOffset = offset(uRow, uColumn, tag_column{});                                          assert(uRowOffset <= (m_uRowSize + size_value()));
   auto puRowValue = puRow + uRowOffset;

   memcpy(puRowValue, &uValue, sizeof(uint32_t));
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value in table
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param uValue value set to cell
 */
void table_base::cell_set_value(uint64_t uRow, unsigned uColumn, uint64_t uValue)
{                                                                                                  assert(size_value() == 8);
#ifndef NDEBUG
   if( (uRow / m_uPackCount) >= m_uRowReservedPackCount || uColumn >= m_pcolumns->size()) { assert(false); }
#endif // !NDEBUG
                                                                                                   assert(uRow < (m_uRowReservedPackCount * count_pack())); assert(uColumn < m_pcolumns->size());
   auto puRow = row_get(uRow);
   auto uRowOffset = offset(uRow, uColumn, tag_column{});
   auto puRowValue = puRow + uRowOffset;

   memcpy(puRowValue, &uValue, sizeof(uint64_t));
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value in table
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param variantviewValue value set to cell
*/
void table_base::cell_set(uint64_t uRow, unsigned uColumn, gd::variant_view variantviewValue)
{                                                                                                  assert(uColumn < m_pcolumns->size());
   auto& columnSet = *m_pcolumns->get(uColumn);                                                    assert(columnSet.position() < m_uRowSize);
   auto puRow = row_get(uRow);

   if(variantviewValue.is_null() == false)
   {
#ifndef NDEBUG 
      auto uValueType_d = variantviewValue.type_number();
      auto uColumnType_d = columnSet.ctype_number();
      if(uValueType_d != uColumnType_d) {                                    // check type, this has to match. You can't set value from type that differ from type in column
         [[maybe_unused]] auto stringValueType_d = gd::types::type_name_g(uValueType_d);
         [[maybe_unused]] auto stringColumnType_d = gd::types::type_name_g(uColumnType_d);
         assert(uValueType_d == uColumnType_d || (variantviewValue.is_char_string() && variant::is_char_string_s(uColumnType_d) == true));
      }
#endif // !NDEBUG

      auto puBuffer = variantviewValue.get_value_buffer();                     // get pointer to value buffer

      auto puRowValue = puRow + columnSet.position();                          // get position to value in row

      if(columnSet.is_fixed())
      {
         if(size_value() == 8) { cell_set_value(uRow, uColumn, variantviewValue.cast_as_uint64()); } 
         else if(size_value() == 4) { cell_set_value(uRow, uColumn, variantviewValue.cast_as_uint32()); } 
      }
      else
      {
         if(columnSet.is_reference() == true)
         {
            // ## reference type            
            int64_t iIndex;

            if(is_duplicated_strings() == false)
            {
               // ### try to find value and store index for found value if it exists, if not add and store new index
               iIndex = m_references.find(variantviewValue);
               if(iIndex == -1)
               {
                  iIndex = (int64_t)m_references.add(variantviewValue);
               }
            }
            else
            {
               iIndex = (int64_t)m_references.add(variantviewValue);           // skip to find existing value, just add new value
            }

            if(size_value() == 8) { cell_set_value(uRow, uColumn, static_cast<uint64_t>(iIndex)); }
            else if(size_value() == 4) { cell_set_value(uRow, uColumn, static_cast<uint32_t>(iIndex)); }
         }
         else { assert(false); }
      }



      if(is_null() == true) { cell_set_not_null(uRow, uColumn); }          // set flag that cell has a value if table is using row status meta data
   }
   else
   {
      if(is_null() == true) { cell_set_null(uRow, uColumn); }              // cell is null, set null flag
   }
}


/** ---------------------------------------------------------------------------
 * @brief Set cell value in table, convert to proper value type used in column if value type do not match
~~~(.cpp)
using namespace gd::table;
table_column_buffer table( 100 );
table.column_add( { { "utf8", 50, "c1"}, { "string", 50, "c2"}, { "int32", 0, "c3"} }, gd::table::tag_type_name{} );
table.prepare();
table.row_add();
table.cell_set( 0, 0, 10, tag_convert{} );
table.cell_set( 0, 1, 20.5, tag_convert{} );
table.cell_set( 0, 2, "20.5", tag_convert{} );
~~~
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param variantviewValue value set to cell
 * @param tag dispatch
*/
void table_base::cell_set(uint64_t uRow, unsigned uColumn, gd::variant_view variantviewValue, tag_convert)
{                                                                                                  assert(uColumn < m_pcolumns->size()); assert(uRow < count_reserved_row());
   auto& columnSet = *m_pcolumns->get(uColumn);                                                    assert(columnSet.position() < m_uRowSize);
   auto uValueType = variantviewValue.type_number();
   auto uColumnType = columnSet.ctype_number();

   if(uValueType == uColumnType)
   {
      cell_set(uRow, uColumn, variantviewValue);
   }
   else
   {
      gd::variant variantConvertTo;
      bool bOk = variantviewValue.convert_to(uColumnType, variantConvertTo);
      if(bOk == true)
      {
         cell_set(uRow, uColumn, *(gd::variant_view*)&variantConvertTo);     // just cast to variant view, internal data is same just that varaiant view have different logic
      }
      else
      {
         if(variantviewValue.is_null() == true && is_null() == true)
         {
            cell_set_null(uRow, uColumn);
         }
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value
 * @param uRow row index for cell
 * @param stringName column name (column has to have a name)
 * @param variantviewValue value set to cell and cell type need to match
*/
void table_base::cell_set( uint64_t uRow, const std::string_view& stringName, gd::variant_view variantviewValue )
{                                                                                                  assert(uRow < count_reserved_row());
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   cell_set( uRow, uColumnIndex, variantviewValue );
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value
 * @param uRow row index for cell
 * @param stringAlias column alias (column has to have a alias)
 * @param variantviewValue value set to cell and cell type need to match
*/
void table_base::cell_set( uint64_t uRow, const std::string_view& stringAlias, gd::variant_view variantviewValue, tag_alias )
{                                                                                                  assert(uRow < count_reserved_row());
   unsigned uColumnIndex = column_get_index( stringAlias, tag_alias{});                            assert(uColumnIndex != ( unsigned )-1);
   cell_set( uRow, uColumnIndex, variantviewValue );
}

/** --------------------------------------------------------------------------- offset_find_column
 * @brief Find column index for offset in table data
 * @param uOffset byte offset in table data
 * @param tag_size8 tag indicating 8-bit size
 * @return column index or -1 if not found
 */
int table_base::offset_find_column(uint64_t uOffset, gd::types::tag_size8) const noexcept
{                                                                                                  assert(m_uRowSize > 0 && size_pack() > 0);
   const uint64_t uTotalRowDataSize = m_uRowCount * m_uRowSize;
   if(uOffset >= uTotalRowDataSize) return -1;

   const uint64_t uOffsetInRowPack = uOffset % m_uRowSize;
   const uint64_t uColumn = uOffsetInRowPack / size_pack();                                        assert(uColumn < get_column_count());

   return static_cast<int>(uColumn);
}

/** --------------------------------------------------------------------------- offset_find_row
 * @brief Find row index for offset in table data
 * @param uOffset byte offset in table data
 * @param tag_size8 tag indicating 8-bit size
 * @return row index or -1 if not found
 */
int64_t table_base::offset_find_row(uint64_t uOffset, gd::types::tag_size8) const noexcept
{                                                                                                  assert(m_uRowSize > 0 && size_pack() > 0);
   const uint64_t uTotalRowDataSize = m_uRowCount * m_uRowSize;                                             
   if(uOffset >= uTotalRowDataSize) return -1;
   
   const uint64_t uRow = uOffset / m_uRowSize;                                                     assert(uRow < get_row_count());
   const uint64_t uOffsetInRowPack = uOffset % m_uRowSize;
   const uint64_t uColumn = uOffsetInRowPack / size_pack();                                        assert(uColumn < get_column_count());
   const uint64_t uRowIndex = uRow * get_column_count() + uColumn;                                 assert(uRowIndex < (m_uRowCount * get_column_count()));

   return static_cast<int64_t>(uRowIndex);
}

/** ---------------------------------------------------------------------------
 * @brief harvest row values into vector with arguments
 * @param uBeginRow start row
 * @param uCount number of rows to harvest values from
 * @param vectorArguments vector with arguments values where harvested arguments are inserted to
*/
void table_base::harvest( uint64_t uBeginRow, uint64_t uCount, std::vector<gd::argument::arguments>& vectorArguments ) const
{
   if( uBeginRow < get_row_count() )
   {
      uint64_t uEndRow = uBeginRow + uCount;                                   // last row 
      if( uEndRow > get_row_count() ) uEndRow = get_row_count();               // is end row within table bounds

      // ## loop rows and harvest values in rows
      for( auto uRow = uBeginRow; uRow < uEndRow; uRow++ )
      {
         gd::argument::arguments arguments;
         row_get_arguments( uRow, arguments );
         vectorArguments.push_back( std::move( arguments ) );
      }
   }
}


/** ---------------------------------------------------------------------------
* @brief harvest row values into vector with arguments
* @param vectorRow rows values are harvested from
* @param vectorArguments vector with arguments values where harvested arguments are inserted to
*/
void table_base::harvest( const std::vector<uint64_t>& vectorRow, std::vector<gd::argument::arguments>& vectorArguments ) const
{
   for( auto itRow : vectorRow )
   {                                                                                               assert( itRow < get_row_count() );
      gd::argument::arguments arguments;
      row_get_arguments( itRow, arguments );
      vectorArguments.push_back( std::move( arguments ) );
   }
}

/** ---------------------------------------------------------------------------
* @brief harvest row values into vector that stores row vectors collecting values
* @param vectorRow rows values are harvested from
* @param vectorHarvest vector with vector that collects values from selected rows
*/
void table_base::harvest( const std::vector<uint64_t>& vectorRow, std::vector< std::vector<gd::variant_view> >& vectorHarvest ) const
{
   for( auto itRow : vectorRow )
   {                                                                                               assert( itRow < get_row_count() );
      std::vector< gd::variant_view > vectorValue;
      vectorValue.reserve( get_column_count() );
      row_get_variant_view( itRow, vectorValue );
      vectorHarvest.emplace_back( std::move( vectorValue ) );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Harvest selected rows from table into table that store harvested data
 * @param vectorRow index for rows where values are harvested from
 * @param tableHarvest table to store harvested data
 */
void table_base::harvest(const std::vector<uint64_t>& vectorRow, table_base& tableHarvest)
{
   if( tableHarvest.get_columns() == nullptr )                                 // do we have columns?
   {
      tableHarvest.set_reserved_row_count( vectorRow.size() );
      tableHarvest.set_columns( get_columns() );
      tableHarvest.prepare();
   }
   else
   {
      tableHarvest.row_reserve_add( vectorRow.size() );
   }

   // ## Loop selected rows and copy row data to harvest table
   std::vector<gd::variant_view> vectorRowData;
   for(auto itRow : vectorRow)
   {                                                                                               assert( itRow < get_row_count() );
      row_get_variant_view( itRow, vectorRowData );
      tableHarvest.row_add( vectorRowData );
      vectorRowData.clear();
   }
}



/** ---------------------------------------------------------------------------
 * @brief Clears all internal data and columns. 
 * 
 * When table is cleared, to start working with again you need to add columns and
 * prepare it to add rows again.
*/
void table_base::clear()
{
   m_uFlags = 0;
   m_uRowSize = 0;
   m_uRowMetaSize = 0;
   m_uRowCount = 0;
   m_uRowReservedPackCount = 0;

   if( m_puData != nullptr ) internal::deallocate( m_puData );
   m_puData = nullptr;
   m_puMetaData = nullptr;

   //m_argumentsProperty.clear();
}


/** ---------------------------------------------------------------------------
 * @brief Allocate columns object on heap for table
 * @return detail::columns* pointer to allocated  columns object.
 */
detail::columns* table_base::new_columns_s()
{
   detail::columns* pcolumns = new detail::columns{};
   pcolumns->add_reference();

   return pcolumns;
}




_GD_TABLE_SIMD_END
