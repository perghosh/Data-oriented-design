#include "gd_utf8.h"
#include "gd_variant.h"

#include "gd_table_arguments.h"

#if GD_COMPILER_HAS_CPP20_SUPPORT

#if (defined(_M_X64) || (defined(_M_IX86) && defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__x86_64__))

#  include <emmintrin.h>
#  include <smmintrin.h>

#  define GD_X86

#else

#  define GD_APPLE

#endif


#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Winvalid-offsetof"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Winvalid-offsetof"
#elif defined( _MSC_VER )
#endif


_GD_TABLE_ARGUMENTS_BEGIN

constexpr unsigned SPACE_VALUE_SIZE = sizeof( uint32_t );
constexpr unsigned SPACE_ALIGN = sizeof( uint32_t );

// Check memory layout between `table` and `table_column_buffer`
static_assert( offsetof( table, m_argumentsProperty ) == offsetof( gd::table::table_column_buffer, m_argumentsProperty ), "top members in table and table_column_buffer need to match" );


/** ---------------------------------------------------------------------------
 * @brief construct table from one single variant view value
@code
// create table with one column storing string value, the buffer length is calculated based on string length
gd::table::table_column_buffer t1( "0123456789", gd::table::tag_prepare{} );
assert( t1.cell_get_variant_view( 0, 0 ).as_string() == "0123456789" );

// create table from one int64_t value
gd::table::table_column_buffer t2( (int64_t)123456789123456789, gd::table::tag_prepare{} );
assert( t2.cell_get_variant_view( 0, 0 ) == gd::variant_view( ( int64_t )123456789123456789 ) );
@endcode
 * @param variantviewValue variant view value to generate table from
*/
table::table( const gd::variant_view& variantviewValue, tag_prepare ) :
   m_uReservedRowCount( 1 ), m_uFlags(0), m_uRowSize(0), m_uRowCount(0),  m_uRowGrowBy(0)
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
table::table( const std::vector< std::string_view >& vectorColumn, tag_prepare ) :
   m_uReservedRowCount( eSpaceFirstAllocate ), m_uFlags( 0 ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uRowGrowBy( 0 )
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
table::table( unsigned uFlags, const std::vector< std::tuple<std::string_view, std::string_view>>& vectorColumn, tag_prepare ) :
   m_uReservedRowCount( eSpaceFirstAllocate ), m_uFlags( uFlags ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uRowGrowBy( 0 )
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
*/
table::table( unsigned uFlags, const std::vector<std::tuple<std::string_view, unsigned, std::string_view>>& vectorColumn, tag_prepare ) :
   m_uReservedRowCount( eSpaceFirstAllocate ), m_uFlags( uFlags ), m_uRowSize( 0 ), m_uRowCount( 0 ), m_uRowGrowBy( 0 )
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
gd::table::arguments::table t1( { { "string", 10, "FName", "0123456789" } }, gd::table::tag_prepare{} );
assert( t1.cell_get_variant_view( 0, "FName" ).as_string() == "0123456789" );  // compare value at R0C0 (C0 = "FName")

// create table with one column storing integer 64 bit value, add some values and compare
gd::table::arguments::table t2( { { "int64", 0, "FInteger", (int64_t)123456789123456789 } }, gd::table::tag_prepare{} );
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
*/
table::table( const std::vector<std::tuple<std::string_view, unsigned, std::string_view, gd::variant_view>>& vectorValue, tag_prepare ) :
   m_uReservedRowCount( 1 ), m_uFlags(0), m_uRowSize(0), m_uRowCount(0),  m_uRowGrowBy(0)
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
 * @brief construct from table_column_buffer and copy selected row into the constructed table
 * @param pcolumns pointer to columns object, adds reference to columns
 * @param ptable pointer to table rows are copied from
 * @param uFrom first row to copy
 * @param uCount number of rows to copy
*/
table::table( gd::table::detail::columns* pcolumns, const gd::table::dto::table* ptable, uint64_t uFrom, uint64_t uCount )
   : m_puData{}
{
   common_construct( pcolumns );

   m_uFlags             = ptable->m_uFlags; 
   m_uRowSize           = ptable->m_uRowSize;  
   m_uRowMetaSize       = ptable->m_uRowMetaSize;

   if( (uFrom + uCount) >= ptable->get_row_count() )
   {
      uCount = ptable->get_row_count() - uFrom;
   }

   if( uCount > 0 )
   {
      row_reserve_add( uCount );
      append( *ptable, uFrom, uCount );
   }
}

/** ---------------------------------------------------------------------------
 * @brief construct from another table and copy selected row into the constructed table
 * @param pcolumns pointer to columns object, adds reference to columns
 * @param ptable pointer to table rows are copied from
 * @param uFrom first row to copy
 * @param uCount number of rows to copy
*/
table::table( gd::table::detail::columns* pcolumns, const table* ptable, uint64_t uFrom, uint64_t uCount )
   : m_puData{}
{                                                                                                  assert( uFrom < ptable->get_row_count() );
   common_construct( pcolumns );

   m_uFlags             = ptable->m_uFlags; 
   m_uRowSize           = ptable->m_uRowSize;  
   m_uRowMetaSize       = ptable->m_uRowMetaSize;

   if( (uFrom + uCount) >= ptable->get_row_count() )
   {
      uCount = ptable->get_row_count() - uFrom;
   }

   if( uCount > 0 )
   {
      row_reserve_add( uCount );
      append( *ptable, uFrom, uCount );
   }
}

/** ---------------------------------------------------------------------------
* @brief destruct table
* Column information is stored in shared object, reference counter is decreased 
*/
table::~table() 
{
   // ## release arguments if set
   if( empty() == false && is_rowarguments() == true && get_row_count() > 0 )
   {
      auto* puPosition = m_puMetaData; 
      // Calculate offset to arguments object in metadata for each row
      auto* puArguments = row_get_arguments_meta( 0 );
      size_t uOffset = puArguments - puPosition;                                                   assert( uOffset <= (m_uRowMetaSize - sizeof(gd::argument::shared::arguments)) );
      for ( uint64_t uRow = 0; uRow < get_row_count(); uRow++ )
      {
         auto* puRow = row_get_arguments_meta(uRow);
         if ( *(intptr_t*)puRow != 0 )
         {
            auto* pArguments = reinterpret_cast<gd::argument::shared::arguments*>( puRow );
            pArguments->release();
         }
      }
   }

   if( m_pcolumns != nullptr ) m_pcolumns->release();                          // release columns information
   delete[] m_puData;
}


/** ---------------------------------------------------------------------------
 * @brief construct table from another table (creates a copy)
 * @note Do not call this method externally, only for internal use
 * @param o reference to table to construct from
*/
void table::common_construct( const table& o ) {
   m_uFlags             = o.m_uFlags; 
   m_uRowSize           = o.m_uRowSize;  
   m_uRowMetaSize       = o.m_uRowMetaSize;
   m_uRowCount          = o.m_uRowCount; 
   m_uReservedRowCount  = o.m_uReservedRowCount;

   delete m_puData;

   m_pcolumns = o.m_pcolumns;

   if( o.m_puData != nullptr )
   {
      uint64_t uTotalSize = size_reserved_total();
      m_puData = new uint8_t[uTotalSize];
      memcpy( m_puData, o.m_puData, uTotalSize );

      // ## check if copied table has meta data
      if( o.m_puMetaData != nullptr ) { m_puMetaData = m_puData + (m_uReservedRowCount * m_uRowSize); assert( m_uFlags != 0 ); }
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
   m_uAllocatedBlockSize_d = size_reserved_total();
#endif // NDEBUG

}

/** ---------------------------------------------------------------------------
 * @brief construct table from another table (creates a copy)
 * @note Do not call this method externally, only for internal use
 * @param o reference to table to construct from
*/
void table::common_construct( const table& o, tag_columns )
{
   m_uFlags             = o.m_uFlags; 
   m_uRowSize           = o.m_uRowSize;  
   m_uRowMetaSize       = o.m_uRowMetaSize;
   m_uRowCount          = 0; 
   m_uReservedRowCount  = 0;

   delete m_puData;
   m_puData = nullptr;
   m_puMetaData = nullptr;

   m_pcolumns = o.m_pcolumns;
   m_pcolumns->add_reference();

   m_argumentsProperty = o.m_argumentsProperty;
}

void table::common_construct( const table& o, tag_body )
{
   m_uFlags             = o.m_uFlags; 
   m_uRowSize           = o.m_uRowSize;  
   m_uRowMetaSize       = o.m_uRowMetaSize;
   m_uRowCount          = o.m_uRowCount; 
   m_uReservedRowCount  = o.m_uReservedRowCount;

   delete m_puData;

   if( o.m_puData != nullptr )
   {
      uint64_t uTotalSize = size_reserved_total();
      m_puData = new uint8_t[uTotalSize];
      memcpy( m_puData, o.m_puData, uTotalSize );

      // ## check if copied table has meta data
      if( o.m_puMetaData != nullptr ) { m_puMetaData = m_puData + (m_uReservedRowCount * m_uRowSize); assert( m_uFlags != 0 ); }
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
void table::common_construct( detail::columns* pcolumns )
{
   m_uFlags             = 0; 
   m_uRowSize           = 0;  
   m_uRowMetaSize       = 0;
   m_uRowCount          = 0; 
   m_uReservedRowCount  = 0;

   delete m_puData;
   m_puData = nullptr;
   m_puMetaData = nullptr;

   m_pcolumns = pcolumns;
   m_pcolumns->add_reference();
}

/** ---------------------------------------------------------------------------
 * @brief Return number of rows for selected state
 * @param uState state to match if row is counted
 *        0x00 = row is not used, 0x01 = row is in use, 0x02 row is deleted
 * @return Number of rows counted
*/
uint64_t table::get_row_count( uint32_t uState ) const noexcept
{                                                                                                  assert( m_puMetaData != nullptr );
   uint64_t uCount = 0;
   auto uRowMetaSize = size_row_meta();
   auto puPosition = m_puMetaData + uRowMetaSize - eSpaceRowState;
   for( auto itRow = 0u; itRow < m_uReservedRowCount; itRow++ )
   {
      if( *reinterpret_cast<uint32_t*>( puPosition ) == uState ) uCount++;
      puPosition += uRowMetaSize;                                              // move pointer to next row                                                        
   }

   return uCount;
}

/** ---------------------------------------------------------------------------
 * @brief add column to table
 * @param uColumnType column type added
 * @param uSize size for column if (0 if primitive type and size for derived types)
 * @return reference to table
*/
table& table::column_add( unsigned uColumnType, unsigned uSize )
{ 
   if( gd::types::is_primitive_g( uColumnType ) == false ) uSize = gd::types::value_size_g( uColumnType, uSize );
   return column_add( detail::column( uColumnType, uSize ) ); 
}


/** ---------------------------------------------------------------------------
 * @brief Adds column to table
 * @param uColumnType value type for column
 * @param uSize if size isn't a fixed type then this is the max size for value
 * @param stringName column name
 * @param stringAlias column alias
 * @return table_column_buffer& reference to table
*/
table& table::column_add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias )
{                                                                                                  assert( m_pcolumns != nullptr ); assert( gd::types::validate_number_type_g( uColumnType ) ); assert( uSize < 0x1000'0000 );
   detail::column columnAdd;
   unsigned uValueOffset{0};

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
 * @brief Add multiple column from information stored as tuple in vector.
 * @param vectorColumn columns to add
 * @param {unsigned} vectorColumn[][0] column type
 * @param {unsigned} vectorColumn[][1] column max size if needed
 * @param {unsigned} vectorColumn[][2] column name
 * @return table_column_buffer& reference to table
*/
table& table::column_add( const std::vector<std::tuple<unsigned, unsigned, std::string_view>>& vectorColumn )
{                                                                                                  assert( m_puData == nullptr );
   for( const auto& it : vectorColumn )
   {
      column_add( std::get<0>( it ), std::get<1>( it ), std::get<2>( it ) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with pair values
~~~(.cpp)
// create tabl with one row and three columns
gd::table::table_column_buffer tableVariable( 1 );   
tableVariable.column_add( { { "double", 0 }, { "double", 0 }, { "double", 0 }, { "int32", 0 } }, gd::table::tag_type_name{} );
tableVariable.prepare();
~~~
 * @param vectorType vector with pair items "<type_name, size>".
 * @param tag dispatcher to diff from other `column_add` methods.
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const std::vector<std::pair<std::string_view, unsigned>>& vectorType, tag_type_name )
{                                                                                                  assert( m_puData == nullptr ); ; assert( m_pcolumns != nullptr );
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {
      column_add( it->first, it->second );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with tuple values
~~~(.cpp)
// create tabl with one row and three columns
gd::table::table_column_buffer tableVariable( 1 );   
tableVariable.column_add( { { "string", 50, "FName"}, { "string", 50, "FName"}, { "string", 50, "FValue"} }, gd::table::tag_type_name{});
tableVariable.prepare();
~~~
 * @param vectorType vector with tuple items "<type_name, size, column_name>".
 * @param tag_type_name tag dispatcher to diff from other `column_add` methods.
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const std::vector<std::tuple<std::string_view, unsigned, std::string_view>>& vectorType, tag_type_name )
{                                                                                                  assert( m_puData == nullptr ); assert( m_pcolumns != nullptr ); // adding columns need to be done before data block is allocated
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {
      column_add( std::get<0>(*it), std::get<1>(*it), std::get<2>(*it) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with tuple values
~~~(.cpp)
// create table with one row and three columns
gd::table::table_column_buffer tableVariable( 1 );   
tableVariable.column_add( { { "string", 50, "FName", "name"}, { "string", 50, "FLastname", "lastname"}, { "string", 50, "FValue", "value"} }, gd::table::tag_type_name{});
tableVariable.prepare();
~~~
 * @param vectorType vector with tuple items "<type_name, size, column_name>".
 * @param tag_type_name tag dispatcher to diff from other `column_add` methods.
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const std::vector<std::tuple<std::string_view, unsigned, std::string_view, std::string_view>>& vectorType, tag_type_name )
{                                                                                                  assert( m_puData == nullptr ); assert( m_pcolumns != nullptr );
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {
      column_add( std::get<0>(*it), std::get<1>(*it), std::get<2>(*it), std::get<3>(*it) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add columns to table with none derived value types, no need for specify max value length
 * @param vectorType vector with pair items "<type_name, column_name>".
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const std::vector< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name )
{                                                                                                  assert( m_puData == nullptr ); assert( m_pcolumns != nullptr );
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {                                                                                               
#ifndef NDEBUG
      // check type, adding column without size can't be done for derived types
      auto uType_d = gd::types::type_g( std::get<0>(*it) );                                        assert( (gd::types::is_primitive_g( uType_d ) == true) || (uType_d & gd::types::eTypeDetailReference) );
#endif // !NDEBUG
      column_add( std::get<0>(*it), 0, std::get<1>(*it) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add columns to table with none derived value types, no need for specify max value length
@code
// create table with one row and three columns
gd::table::table_column_buffer tableVariable( 10 );   
tableVariable.column_add( { { "int32", "x" }, { "int32", "y" } }, gd::table::tag_type_name{});
tableVariable.prepare();
@endcode
 * @param vectorType vector with pair items "<type_name, column_name>".
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const std::initializer_list< std::pair< std::string_view, std::string_view > >& listType, tag_type_name )
{                                                                                                  assert( m_puData == nullptr ); assert( m_pcolumns != nullptr );
   for( auto it = std::begin( listType ), itEnd = std::end( listType ); it != itEnd; it++ )
   {                                                                                               
#ifndef NDEBUG
      // check type, adding column without size can't be done for derived types
      auto uType_d = gd::types::type_g( std::get<0>(*it) );                                        assert( (gd::types::is_primitive_g( uType_d ) == true) || (uType_d & gd::types::eTypeDetailReference) );
#endif // !NDEBUG
      column_add( std::get<0>(*it), 0, std::get<1>(*it) );
   }

   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief Add column to table
 * Adds columns to value from vector with type constants prepared, no conversion from type name 
 * to type constant is done so this is a bit faster
 * @param vectorType vector with pair constants, first is column type, second is buffer size for derived types
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const std::vector<std::pair<unsigned, unsigned>>& vectorType, tag_type_constant )
{                                                                                                  assert( m_puData == nullptr ); assert( m_pcolumns != nullptr );
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {                                                                                               assert( gd::types::validate_number_type_g( it->first ) == true );
      column_add( it->first, it->second );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add columns and used information from another table
 * @param table_ table that column information is found
 * @return reference to table_column_buffer to nest methods.
*/
table& table::column_add( const table* p_ )
{                                                                                                  assert( p_ != nullptr ); assert( m_pcolumns != nullptr );
   for( auto it = p_->column_begin(), itEnd = p_->column_end(); it != itEnd; it++ )
   {
      detail::column columnAdd( *it ); // copy column
      m_pcolumns->add( std::move( columnAdd ) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief find index to column for column name
 * @param stringName column name column index is returned for
 * @return int index to column for column name if found, -1 if not found
*/
int table::column_find_index( const std::string_view& stringName ) const noexcept
{
   for( auto it = m_pcolumns->begin(), itEnd = m_pcolumns->end(); it != itEnd; it++ )
   {
      auto name_ = it->name();
      if( stringName == name_ ) return (int)std::distance( m_pcolumns->begin(), it );
   }
   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief find index to column for column alias
 * @param stringAlias column alias column index is returned for
 * @return int index to column for column alias if foundm, -1 if not found
*/
int table::column_find_index( const std::string_view& stringAlias, tag_alias ) const noexcept
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
int table::column_find_index( const std::string_view& stringWildcard, tag_wildcard ) const noexcept
{
   for( auto it = m_pcolumns->begin(), itEnd = m_pcolumns->end(); it != itEnd; it++ )
   {
      if( gd::ascii::strcmp( it->name().data(), stringWildcard.data(), gd::utf8::tag_wildcard{}) == 0 )
      {
         return (int)std::distance( m_pcolumns->begin(), it );
      }
   }
   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief get index to column for column name
 * @param stringName column name column index is returned for
 * @return unsigned index to column for column name
*/
unsigned table::column_get_index( const std::string_view& stringName ) const noexcept
{   
   int iIndex = column_find_index( stringName );                                                   assert( iIndex != -1 );
   return (unsigned)iIndex;
}

/** ---------------------------------------------------------------------------
 * @brief get index to column for column alias
 * @param stringAlias column alias column index is returned for
 * @return unsigned index to column for column alias
*/
unsigned table::column_get_index( const std::string_view& stringAlias, tag_alias ) const noexcept
{
   int iIndex = column_find_index( stringAlias, tag_alias{});                                      assert(iIndex != -1);
   return (unsigned)iIndex;
}

/** ---------------------------------------------------------------------------
 * @brief get index to column for column name using wildcard match
 * @param stringWildcard column name column index is returned for
 * @return unsigned index to column for column name
*/
unsigned table::column_get_index( const std::string_view& stringWildcard, tag_wildcard ) const noexcept
{
   int iIndex = column_find_index( stringWildcard, tag_wildcard{});                                assert(iIndex != -1);
   return (unsigned)iIndex;
}


/** ---------------------------------------------------------------------------
 * @brief get indexes for names in list
 * @code
gd::table::dto::table tableDescription( 
   gd::table::dto::table::eTableFlagNull32, 
   {{"rstring", 0, "table_name"}, {"string", 100, "column_name"}, {"uint32", 0, "type"}, {"uint32", 0, "size"} }, 
   gd::table::tag_prepare{}
);
auto vectorTableColumn =  tableTable.column_get_index( { "column_name", "type"});
 * @endcode
 * @param listName list with column names
 * @return std::vector<uint32_t> indexes to found columns
*/
std::vector<uint32_t> table::column_get_index( std::initializer_list<std::string_view> listName ) const noexcept
{
   std::vector<uint32_t> vectorIndex;
   for( auto it = std::begin( listName ), itEnd = std::end( listName ); it != itEnd; it++ )
   {
      int iColumn = column_find_index( *it );
      if( iColumn != -1 ) vectorIndex.push_back( (uint32_t)iColumn );
   }

   return vectorIndex;
}

/** ---------------------------------------------------------------------------
 * @brief get indexes for names in list
 * @param vectorName vector with column names
 * @return std::vector<uint32_t> indexes to found columns
*/
std::vector<uint32_t> table::column_get_index( const std::vector<std::string_view>& vectorName ) const noexcept
{
   std::vector<uint32_t> vectorIndex;
   for( auto it = std::begin( vectorName ), itEnd = std::end( vectorName ); it != itEnd; it++ )
   {
      int iColumn = column_find_index( *it );
      if( iColumn != -1 ) vectorIndex.push_back( (uint32_t)iColumn );
   }

   return vectorIndex;
}

/** ---------------------------------------------------------------------------
* @brief get column type for column indexes in vector
* @param vectorName vector with indexes to columns types are returned for
* @return std::vector<unsigned> type information for columns
*/
std::vector<unsigned> table::column_get_type(const std::vector<unsigned>& vectorIndex) const
{
   std::vector<unsigned> vectorType;
   for(const auto& it : vectorIndex) { vectorType.push_back( column_get_type( it ) ); }

   return vectorType;
}


/** ---------------------------------------------------------------------------
 * @brief check if column name exists in table
 * @param stringName column name existence is checked for
 * @return true if there is a column with given name exists, false otherwise
*/
bool table::column_exists( const std::string_view& stringName ) const noexcept
{
   for( auto it = m_pcolumns->begin(), itEnd = m_pcolumns->end(); it != itEnd; it++ )
   {
      if( stringName == it->name() ) return true;
   }                                                                                                
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief check if column alias exists in table
 * @param stringAlias column alias existence is checked for
 * @return true if there is a column with given alias exists, false otherwise
*/
bool table::column_exists( const std::string_view& stringAlias, tag_alias ) const noexcept
{
   for( auto it = m_pcolumns->begin(), itEnd = m_pcolumns->end(); it != itEnd; it++ )
   {
      if( stringAlias == it->alias() ) return true;
   }
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief rename column in table
 * @note remember that there is a limit on how much data names for column are able to use
 * @code
gd::table::dto::table table( 10 );
std::string stringTable = R"(SELECT name FROM sqlite_master WHERE type='table')";
auto [bOk, stringError] = application::database::EXECUTE_SelectToTable_g( &databaseRead, stringTable, &table); // fills table with data (how that is used isn't shown here)
table.column_rename( 0, "table" ); // rename column
 * @endcode
 * @param uColumn index to column to rename
 * @param stringNewName new name set to column
 * @return std::string_view old name for column
*/
std::string table::column_rename( unsigned uColumn, const std::string_view& stringNewName )
{                                                                                                  assert( uColumn < m_pcolumns->size() );
   detail::column* pcolumn = column_get( uColumn, tag_pointer{} );

   std::string stringOldName( pcolumn->name() );
   pcolumn->name( stringNewName );
   return stringOldName;
}



/**
 * @brief get/set column information iterating all columns in table
 * 
~~~(.cpp)
gd::table::table_column_buffer tableVariable( 100 );   
// .. omitted ..
tableVariable.column_for_each( [puType]( gd::table::table_column_buffer::column& c_, unsigned uIndex ) {
   c_.data( puType[uIndex] );
});

// or

tableVariable.column_for_each( [puType]( auto& c_, auto uIndex ) {
   c_.data( puType[uIndex] );
});
~~~
 * @param callback_ callback method  to process column value
*/
void table::column_for_each( std::function<void( detail::column&, unsigned )> callback_ )
{
   for( unsigned u = 0, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++ )
   {
      callback_( *m_pcolumns->get( u ), u );
   }
}

void table::column_for_each( std::function<void( const detail::column&, unsigned )> callback_ ) const
{
   for( unsigned u = 0, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++ )
   {
      callback_( *m_pcolumns->get( u ), u);
   }
}


/** ---------------------------------------------------------------------------
 * @brief Prepare internal data before use
 * Calculates needed space and allocate memory to store data
 * 
 * @code
gd::table::table_column_buffer tableNumber( 1000 );  // create table and it should store 1000 rows
// add columns to table
tableNumber.column_add("uint32").column_add("uint32").column_add("uint32").column_add("int64").column_add("int16", "c4");
tableNumber.prepare();                               // create internal buffers
tableNumber.row_add( 1000 );                         // add valid rows
tableNumber.column_fill( "c4", ( int16_t )4 );       // fill complete column
tableNumber.column_fill( "c4", ( int16_t )5, 3, 10); // fill row 3 to 10
tableNumber.column_fill( 3, (int64_t)8 );
 * @endcode

 * @return std::pair<bool, std::string> true if ok, false and error information if fail
*/
std::pair<bool, std::string> table::prepare()
{                                                                                                  assert( m_pcolumns->empty() == false ); assert( m_puData == nullptr );
   // ## calculate size for each row
   unsigned uRowSize = 0; // 

   // ### Calculate space needed for each column in table
   for( auto& it : *m_pcolumns )
   {                                                                                               assert( uRowSize % SPACE_ALIGN == 0 );// each value is aligned (4 byte alignment ?) for better performance
      unsigned uState = 0;
      unsigned uSize;                                                          // total buffer size for value (note that buffer may store more than just value)
      unsigned uTypeSize = gd::types::value_size_g( it.ctype() );              // type size if type is primitive or a special fixed type
      if( it.size() > 0 && uTypeSize == 0 )
      {
         uState |= gd::table::arguments::table::eColumnStateLength;
         uSize = gd::types::value_size_g( it.ctype(), it.size() ) + SPACE_VALUE_SIZE;
      }
      else
      {
         if( gd::types::is_reference_g( it.ctype() ) == false )
         {
            uSize = uTypeSize;
         }
         else
         {
            uSize = sizeof( uint64_t );
            uState |= gd::table::arguments::table::table::eColumnStateReference;// for reference values length is store in reference object and index to reference is stored as cell value in table
         }
      }
                                                                                                   assert( uSize > 0 ); assert( uSize < 0x01000000 );

      it.position( uRowSize );                                                 // set offset position for column value, each column knows where value starts in row buffer
      it.state( uState );                                                      // column state (used internally)

      uRowSize += uSize;
      if( uSize % SPACE_ALIGN > 0 ) { uRowSize += (SPACE_ALIGN - (uSize % SPACE_ALIGN)); }// align for better performance
   }

   m_uRowSize = uRowSize;                                                      // final row sizes


   // ## calculate needed meta data size for each row
   unsigned uMetaDataSize = size_row_meta();

   m_uRowMetaSize = uMetaDataSize;

   uint64_t uTotalTableSize = (uRowSize + uMetaDataSize) * m_uReservedRowCount;// calculate size storing table data

   m_puData = new uint8_t[ uTotalTableSize ];
#ifdef _DEBUG
   memset( m_puData, 0, uTotalTableSize );                                     // set data to 0 in debug mode
#endif // _DEBUG

   if( uMetaDataSize > 0 )
   {
      m_puMetaData = m_puData + (m_uReservedRowCount * uRowSize);              // set pointer to meta data section
      memset( m_puMetaData, 0, m_uReservedRowCount * uMetaDataSize );
   }
                                                                                                      
   return { true, "" };
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
void table::row_add( const std::initializer_list<gd::variant_view>& listValue )
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
void table::row_add( const std::initializer_list<gd::variant_view>& vectorValue, tag_convert )
{                                                                                                  assert( vectorValue.size() <= get_column_count() );              
   uint64_t uRow = m_uRowCount;

   row_add();
   
   row_set( uRow, vectorValue, tag_convert{} );
}

void table::row_add( const std::vector<gd::variant_view>& vectorValue )
{                                                                                                  assert( vectorValue.size() <= get_column_count() );              
   uint64_t uRow = m_uRowCount;

   row_add();
   
   row_set( uRow, vectorValue );
}

/** ---------------------------------------------------------------------------
 * @brief Add row to table and insert values from vector to added row
 * @param vectorValue values inserted to added row
 * @param vectorColumn column index where to place values in vector
*/
void table::row_add( const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn )
{                                                                                                  assert( vectorValue.size() == vectorColumn.size() );
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, vectorValue, vectorColumn );
}


/** ---------------------------------------------------------------------------
 * @brief Add row to table and insert values from vector to added row, convert for those types that do not match
 * @param vectorValue values inserted to added row
 * @param vectorColumn column index where to place values in vector
*/
void table::row_add( const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn, tag_convert )
{                                                                                                  assert( vectorValue.size() == vectorColumn.size() );
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, vectorValue, vectorColumn, tag_convert{} );
}

/** ---------------------------------------------------------------------------
 * @brief Add row and set values in row, vector cant be larger than amount of values in row
~~~(.cpp)
gd::table::table_column_buffer table( 100 );
table.column_add( { { "int64", 0, "c1"}, { "int32", 0, "c2"}, { "int16", 0, "c3"} }, gd::table::tag_type_name{} );
table.prepare();

std::vector<gd::variant_view> vectorValue = { {1},{2},{3} }; // three integer values
table.row_add( vectorValue, gd::table::tag_convert{} );      // add values to row, values are converterd to proper type
~~~
 * @param listValue list of values inserted in added row
*/
void table::row_add( const std::vector<gd::variant_view>& vectorValue, tag_convert )
{                                                                                                  assert( vectorValue.size() <= get_column_count() );              
   uint64_t uRow = m_uRowCount;

   row_add();
   
   row_set( uRow, vectorValue, tag_convert{} );
}

/** ---------------------------------------------------------------------------
 * @brief add row and set values
 * @param vectorValue vector with pair values set to row, first is column index and second is value
*/
void table::row_add( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, vectorValue );
}

/** ---------------------------------------------------------------------------
 * @brief add row and set values, convert to right type if value type differ from column
 * @param vectorValue vector with pair values set to row, first is column index and second is value
*/
void table::row_add( const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue, tag_convert )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, vectorValue, tag_convert{} );
}

/** ---------------------------------------------------------------------------
 * @brief add row and set values
 * @code
using namespace gd::table;
table_column_buffer table( 10, tag_null{} );
table.column_add({{"uint32", "MoleculeK"}, {"rstring", "FName"}, {"double", "FX"}, {"double", "FY"},{"double", "FZ"}}, tag_type_name{});
table.prepare();
table.row_add( { { "FName", "H" }, { "FX", 0.0 }, { "FY", 0.0 }, { "FZ", 0.0 } } );
 * @endcode
 * @param vectorValue vector with pair values set to row, first is column name and second is value
*/
void table::row_add( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, vectorValue );
}

/** ---------------------------------------------------------------------------
 * @brief add row and set values, convert to right type if value type differ from column
 * @param vectorValue vector with pair values set to row, first is column name and second is value
*/
void table::row_add( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, vectorValue, tag_convert{} );
}

/** ---------------------------------------------------------------------------
 * @brief Add values from arguments object where names in arguments match column names
 * @param argumentsRow values added to row
*/
void table::row_add( const gd::argument::arguments& argumentsRow, tag_arguments )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, argumentsRow, tag_arguments{} );
}

/** ---------------------------------------------------------------------------
 * @brief Add values from arguments object where names in arguments match column names
 * @param argumentsRow values added to row
*/
void table::row_add( const gd::argument::arguments& argumentsRow, tag_arguments, tag_convert )
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, argumentsRow, tag_arguments{}, tag_convert{} );
}


/** ---------------------------------------------------------------------------
 * @brief Add row and copy data at specified row to the added row
 * @param uRowToCopy index to row where data is copied from
*/
void table::row_add(uint64_t uRowToCopy, tag_copy)
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, uRowToCopy );
}

/** ---------------------------------------------------------------------------
 * @brief add cell values from string where string is divided based on character sent
 * @param uFirst index to first column where first value are inserted
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 */
void table::row_add(unsigned uFirst, const std::string_view& stringRowValue, char chSplit, tag_parse)
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, uFirst, stringRowValue, chSplit, tag_parse{} );
}

/** ---------------------------------------------------------------------------
 * @brief add cell values from string where string is divided based on character sent
 * @param puColumn pointer to column indexes where values are put
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 */
void table::row_add(const unsigned* puColumn, std::string_view& stringRowValue, char chSplit, tag_parse)
{
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   row_set( uRow, puColumn, stringRowValue, chSplit, tag_parse{} );
}

/** ---------------------------------------------------------------------------
 * @brief add cell values from string where string is divided based on character sent
 * @param uFirst index to first column where first value are inserted
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 * @param callback_ callback used to modify parsed values if needed
 * @return bool true if ok, false if error
 */
bool table::row_add(unsigned uFirst, const std::string_view& stringRowValue, char chSplit, std::function< bool( std::vector<std::string>& vectorValue )> callback_, tag_parse)
{                                                                                                  assert( empty( tag_raw{} ) == false);
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   return row_set( uRow, uFirst, stringRowValue, chSplit, callback_, tag_parse{} );
}



/** ---------------------------------------------------------------------------
 * @brief add cell values from string where string is divided based on character sent
 * @param puColumn pointer to column indexes where values are put
 * @param stringRowValue string with values
 * @param chSplit character that separates values
 * @param callback_ callback used to modify parsed values if needed
 * @return bool true if ok, false if error
 */
bool table::row_add(const unsigned* puColumn, std::string_view& stringRowValue, char chSplit, std::function< bool( std::vector<std::string>& vectorValue )> callback_, tag_parse)
{
   uint64_t uRow = m_uRowCount;
   row_add();
   if( is_null() == true ) { row_set_null( uRow ); }
   return row_set( uRow, puColumn, stringRowValue, chSplit, callback_, tag_parse{} );
}

/** ---------------------------------------------------------------------------
 * @brief Add values from arguments object where names in arguments match column names
 * @param argumentsRow values added to row
*/
void table::row_set( uint64_t uRow, const gd::argument::arguments& argumentsRow, tag_arguments )
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
 * @brief Add values from arguments object where names in arguments match column names
 * @param argumentsRow values added to row
*/
void table::row_set( uint64_t uRow, const gd::argument::arguments& argumentsRow, tag_arguments, tag_convert )
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
            cell_set( uRow, iIndex, value_, tag_convert{});
         }
         else
         {
            cell_set_argument( uRow, stringName, value_ );
         }
      }
   }
}


/** ---------------------------------------------------------------------------
 * @brief Set row values
 * @param uRow row where values are set
 * @param listValue list of values inserted to specified row
*/
void table::row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue )
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
void table::row_set( uint64_t uRow, unsigned uFirstColumn, const std::initializer_list<gd::variant_view>& listValue )
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
void table::row_set( uint64_t uRow, const std::initializer_list<gd::variant_view>& listValue, tag_convert )
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
void table::row_set( uint64_t uRow, unsigned uFirstColumn, const std::initializer_list<gd::variant_view>& listValue, tag_convert )
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
void table::row_set( uint64_t uRow, const std::vector<gd::variant_view>& vectorValue )
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
void table::row_set( uint64_t uRow, const std::vector<gd::variant_view>& vectorValue, tag_convert )
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
void table::row_set( uint64_t uRow, const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn )
{                                                                                                  assert( uRow < m_uRowCount ); assert( vectorValue.size() == vectorColumn.size() );
   for( unsigned u = 0, uMax = (unsigned)vectorValue.size(); u < uMax; u++ )
   {
      unsigned uColumn = vectorColumn[u];                                                          assert( uColumn < get_column_count() );
      cell_set( uRow, uColumn, vectorValue[u]);
   }
}


/** ---------------------------------------------------------------------------
 * @brief Set row values and specify column index where value is placed, if value type differ it tries to convert to type in column
 * @param uRow index to row where values are set
 * @param vectorValue vector with values
 * @param vectorColumn index to column where value is placed
*/
void table::row_set( uint64_t uRow, const std::vector<gd::variant_view>& vectorValue, const std::vector<unsigned>& vectorColumn, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount ); assert( vectorValue.size() == vectorColumn.size() );
   for( unsigned u = 0, uMax = (unsigned)vectorValue.size(); u < uMax; u++ )
   {
      unsigned uColumn = vectorColumn[u];                                                          assert( uColumn < get_column_count() );
      cell_set( uRow, uColumn, vectorValue[u], tag_convert{});
   }
}

/** ---------------------------------------------------------------------------
 * @brief set row values
 * @param vectorValue vector with pair values set to row, first is column index and second is value
*/
void table::row_set( uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue )
{                                                                                                  assert( uRow < m_uRowCount );
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {                                                                                               assert( it->first < get_column_count() );
      cell_set( uRow, it->first, it->second );
   }
}

/** ---------------------------------------------------------------------------
 * @brief set row values, convert to right type if value type differ from column
 * @param vectorValue vector with pair values set to row, first is column index and second is value
*/
void table::row_set( uint64_t uRow, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorValue, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount );
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {                                                                                               assert( it->first < get_column_count() );
      cell_set( uRow, it->first, it->second, tag_convert{} );
   }
}

/** ---------------------------------------------------------------------------
 * @brief set row values
 * @param vectorValue vector with pair values set to row, first is column name and second is value
*/
void table::row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue )
{                                                                                                  assert( uRow < m_uRowCount );
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      int iIndex = column_find_index( it->first );
      if( iIndex != -1 ) cell_set( uRow, ( unsigned )iIndex, it->second );
   }
}

/** ---------------------------------------------------------------------------
 * @brief set row values, convert to right type if value type differ from column
 * @param vectorValue vector with pair values set to row, first is column name and second is value
*/
void table::row_set( uint64_t uRow, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorValue, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount );
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it != itEnd; it++ )
   {
      int iIndex = column_find_index( it->first );
      if( iIndex != -1 ) cell_set( uRow, ( unsigned )iIndex, it->second, tag_convert{} );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set data taken from another row in table
 * @param uRow row where data is set to
 * @param uRowToCopy row where data is taken from
*/
void table::row_set(uint64_t uRow, uint64_t uRowToCopy)
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
void table::row_set(uint64_t uRow, unsigned uFirst, const std::string_view& stringRowValue, char chSplit, tag_parse)
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
void table::row_set(uint64_t uRow, const unsigned* puColumn, const std::string_view& stringRowValue, char chSplit, tag_parse)
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
bool table::row_set(uint64_t uRow, unsigned uFirst, const std::string_view& stringRowValue, char chSplit,std::function< bool( std::vector<std::string>& vectorValue )> callback_, tag_parse)
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
bool table::row_set(uint64_t uRow, const unsigned* puColumn, const std::string_view& stringRowValue, char chSplit, std::function< bool( std::vector<std::string>& vectorValue )> callback_, tag_parse)
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
 * @brief Set column sequence in row to value
 * 
 * @code
   using namespace gd::table;

   // create table with columns that has different types
   table_column_buffer table01( table_column_buffer::eTableFlagNull32, { {"int8", "col1"}, {"bool", "col2"}, {"double", "col3"}, {"int16", "col4"}, {"int64", "col5"} }, tag_prepare{});

   table01.row_add();
   table01.row_set_range( table01.get_row_back(), 0, get_column_count(), 0, tag_convert{}); // set all column values to 0 in last row
   table01.row_add();
   // row_set_range has wrappers to avoid setting start column and count for how many columns after that is set
   table01.row_set_range( table01.get_row_back(), 1, tag_convert{}); // set all column values to 1 in last row
   table01.row_add();
   table01.row_set_range( table01.get_row_back(), 2, tag_convert{}); // set all column values to 2 in last row

   std::string stringTable = gd::table::debug::print( table01 );
   // stringTable =
   // 0, 0, 0.000000, 0, 0
   // 1, 0, 1.000000, 1, 1
   // 2, 0, 2.000000, 2, 2

   auto find_ = table01.find( 0, (int8_t)2 ); // find value in column 0 that has the value 2, find_ = 2
   assert( find_ == 2 );

 * @endcode
 * 
 * @param uRow row cell values are set
 * @param uBeginColumn column where to begin to set value
 * @param uCount number of columns to set
 * @param variantviewSet value to set
*/
void table::row_set_range( uint64_t uRow, unsigned uBeginColumn, unsigned uCount, const gd::variant_view variantviewSet, tag_convert )
{                                                                                                  assert( uRow < m_uRowCount ); 
   unsigned uEndColumn = uBeginColumn + uCount;                                                    assert( uEndColumn <= get_column_count() );
   for( unsigned uColumn = uBeginColumn; uColumn < uEndColumn; uColumn++ )
   {
      cell_set( uRow, uColumn, variantviewSet, tag_convert{});
   }
}

void table::reserve(uint64_t uCount)
{
   if( uCount > m_uReservedRowCount )
   {
      row_reserve_add( uCount - m_uReservedRowCount );
   }
   else
   {
      m_uRowCount = uCount;
   }
}



/** ---------------------------------------------------------------------------
 * @brief adds more memory storing row/rows to table
 * @param uCount number of rows to add
*/
void table::row_reserve_add( uint64_t uCount )
{
   uCount += m_uReservedRowCount;

   // ## calculate size needed to store added row count and allocate memory
   uint64_t uTotalTableSize = size_reserved_total();                           // total table memory block size for table
   uint64_t uTotalMetaSize = size_meta_total();                                // meta block size part

   uint64_t uTotalTableSizeCopyTo = size_reserved_total( uCount );             // new block size to store more rows
   uint64_t uTotalMetaSizeCopyTo = size_meta_total( uCount );                  // new meta block size

   uint64_t uCopyRowSize = uTotalTableSize - uTotalMetaSize;

   uint8_t* puDataCopyTo = new uint8_t[ uTotalTableSizeCopyTo ];               // new buffer for table data (both data and meta data)

   if( m_puData != nullptr ) memcpy( puDataCopyTo, m_puData, uCopyRowSize );   // copy row data

   if( m_puMetaData != nullptr )
   {
      // ## copy meta data block to new increased table block
      uint8_t* puMetaData = puDataCopyTo + ( uTotalTableSizeCopyTo - uTotalMetaSizeCopyTo);// position where meta data starts
      memcpy( puMetaData, m_puMetaData, uTotalMetaSize );                      // copy old meta data
      m_puMetaData = puMetaData;                                               // set member meta data pointer to new block
      memset( m_puMetaData + uTotalMetaSize, 0, uTotalMetaSizeCopyTo - uTotalMetaSize );// clear rest of meta data
   }
   else if( uTotalMetaSizeCopyTo > 0 )
   {
      m_puMetaData = puDataCopyTo + ( uTotalTableSizeCopyTo - uTotalMetaSizeCopyTo);// set meta position pointer if meta data is used
   }

   delete [] m_puData;
   m_puData = puDataCopyTo;

   m_uReservedRowCount = uCount;
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block table use to store values
 * @param uRow row where cell value is
 * @param uColumn column for cell
 * @return pointer to memory buffer cell stores its value
*/
uint8_t* table::cell_get( uint64_t uRow, unsigned uColumn ) noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount ); assert( uColumn < m_pcolumns->size() ); assert( m_puData );
   auto& columnSet = *m_pcolumns->get( uColumn );
   auto puRow = row_get( uRow );
   return puRow + columnSet.position();
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block table use to store values
 * @param uRow row where cell value is
 * @param uColumn column for cell
 * @return pointer to memory buffer cell stores its value
*/
const uint8_t* table::cell_get( uint64_t uRow, unsigned uColumn ) const noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount ); assert( uColumn < m_pcolumns->size() ); assert( m_puData );
   auto& columnSet = *m_pcolumns->get( uColumn );
   auto puRow = row_get( uRow );
   return puRow + columnSet.position();
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block table use to store values
 * @param uRow row where cell value is
 * @param stringName name for column cell is found at
 * @return pointer to memory buffer cell stores its value
*/
uint8_t* table::cell_get( uint64_t uRow, const std::string_view& stringName ) noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringName );
   return cell_get( uRow, uColumnIndex );
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block table use to store values
 * @param uRow row where cell value is
 * @param stringAlias alias for column cell is found at
 * @return pointer to memory buffer cell stores its value
*/
uint8_t* table::cell_get( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringAlias, tag_alias{});
   return cell_get( uRow, uColumnIndex );
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block for cell for name with wildcard matching
 * @param uRow row where cell value is
 * @param stringWildcard match against name for column cell
 * @return pointer to memory buffer cell stores its value
*/
uint8_t* table::cell_get( uint64_t uRow, const std::string_view& stringWildcard, tag_wildcard ) noexcept
{
   unsigned uColumnIndex = column_get_index( stringWildcard, tag_wildcard{});
   return cell_get( uRow, uColumnIndex );
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block table use to store values
 * @param uRow row where cell value is
 * @param stringName name for column cell is found at
 * @return pointer to memory buffer cell stores its value
*/
const uint8_t* table::cell_get( uint64_t uRow, const std::string_view& stringName ) const noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringName );
   return cell_get( uRow, uColumnIndex );
}

/** ---------------------------------------------------------------------------
 * @brief Get position to cell in memory block table use to store values
 * @param uRow row where cell value is
 * @param stringAlias alias for column cell is found at
 * @return pointer to memory buffer cell stores its value
*/
const uint8_t* table::cell_get( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) const noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringAlias, tag_alias{});
   return cell_get( uRow, uColumnIndex );
}


/** ---------------------------------------------------------------------------
 * @brief Get pointer to reference for specified cell
 * Get reference object to cell, make sure that cell has reference
 * @param uRow row where cell value is
 * @param uColumn column for cell
 * @return pointer to reference object for cell if not null, if null then nullpointer is returned
*/
const reference* table::cell_get_reference( uint64_t uRow, unsigned uColumn ) const noexcept
{
   if( is_null() == true && cell_is_null( uRow, uColumn ) == true ) return nullptr;
                                                                                                   assert( m_references.size() > 0 );
   const auto& columnGet = *m_pcolumns->get( uColumn );                                            assert( columnGet.is_reference() );
   auto puRow = row_get( uRow ); // buffer to row
   auto puRowValue = puRow + columnGet.position();

   uint64_t uIndex = *(uint64_t*)puRowValue;                                                       assert( uIndex < 0x1000'0000 ); assert( uIndex < m_references.size() ); // realistic value?

   const reference* preference = m_references.at( uIndex );

   return preference;
}


/** ---------------------------------------------------------------------------
 * @brief get cell value as variant_view item
 * @param uRow row index for cell
 * @param uColumn column index to cell
 * @return gd::variant_view cell value
*/
gd::variant_view table::cell_get_variant_view( uint64_t uRow, unsigned uColumn ) const noexcept
{                                                                                                  assert( uRow < get_row_count() ); assert( uRow < m_uReservedRowCount ); assert( m_puData != nullptr );
   if( uColumn >= m_pcolumns->size() )
   {
      uColumn -= (unsigned)m_pcolumns->size();                                // if column index is larger than column count then it is tag_arguments
      return cell_get_variant_view( uRow, uColumn, tag_arguments{});
   }
   
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
 * @brief get cell value as variant_view item
 * 
 * @note This function is used to get cell value from row that has arguments and remember to decrease column index by column count
 * 
 * @param uRow row index for cell
 * @param uColumn column index to cell
 * @param tag_arguments tag to specify that arguments are used
 * @return gd::variant_view cell value
*/
gd::variant_view table::cell_get_variant_view(uint64_t uRow, unsigned uColumn, tag_arguments) const noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount ); assert( is_rowarguments() );
   // get row meta data
   gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow);
   if( *(intptr_t*)pargumentsRow != 0 )
   {
      return (*pargumentsRow)[uColumn].as_variant_view();
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief get cell values within specified column range in row
 * @param uRow index to row where values are read from
 * @param uFromColumn start column where to get values
 * @param uToColumn end column
 * @return std::vector<gd::variant_view> vector of values from
*/
std::vector<gd::variant_view> table::cell_get_variant_view( uint64_t uRow, unsigned uFromColumn, unsigned uToColumn ) const
{                                                                                                  assert( uRow < m_uReservedRowCount ); assert( uFromColumn < get_column_count() ); assert( uToColumn <= get_column_count() );
   std::vector<gd::variant_view> vectorValue; // vector of values taken from row

   for( unsigned u = uFromColumn; u < uToColumn; u++ )
   {
      auto value_ = cell_get_variant_view( uRow, u );
      vectorValue.push_back( value_ );
   }

   return vectorValue;
}

/** ---------------------------------------------------------------------------
 * @brief get cell value
 * @param uRow index to row where cell value is found
 * @param stringName column name for column where cell value is found
 * @return variant_view value is returned in variant view
*/
gd::variant_view table::cell_get_variant_view( uint64_t uRow, const std::string_view& stringName ) const noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount );
   int iColumnIndex = column_find_index( stringName );
   if( iColumnIndex != -1 )
   {
      return cell_get_variant_view( uRow, (unsigned)iColumnIndex );
   }

   gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow);
   if( *(intptr_t*)pargumentsRow != 0 )
   {
      return (*pargumentsRow)[stringName].as_variant_view();
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief get cell value
 * @param uRow index to row where cell value is found
 * @param stringAlias column alias for column where cell value is found
 * @return variant_view value is returned in variant view
*/
gd::variant_view table::cell_get_variant_view( uint64_t uRow, const std::string_view& stringAlias, tag_alias ) const noexcept
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringAlias, tag_alias{});
   return cell_get_variant_view( uRow, uColumnIndex );
}

/** ---------------------------------------------------------------------------
 * @brief get cell value as variant_view item 
 * This do not check if value is null, it just takes the value from column buffer,
 * Because it does not check for null it is a bit faster compared to default `cell_get_variant_view`
 * @param uRow row index for cell
 * @param uColumn column index to cell
 * @return gd::variant_view cell value
*/
gd::variant_view table::cell_get_variant_view( uint64_t uRow, unsigned uColumn, tag_raw ) const noexcept
{
   const auto& columnGet = *m_pcolumns->get( uColumn );// column information for value
   auto puRow = row_get( uRow ); // buffer to row

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
      {                                                                                            assert( m_references.size() > 0 ); // do we have reference values because they are needed
         uint64_t uIndex = *(uint64_t*)puRowValue;                                                 assert( uIndex < 0x1000'0000 ); // realistic value?
                                                                                                   assert( uIndex < m_references.size() );
         reference* preference = m_references.at( uIndex );
         #if DEBUG_RELEASE > 0
         preference->assert_valid_d();
         #endif
         return gd::variant_view( preference->ctype(), preference->data(), preference->length());
      }
      else { assert(false); }
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief get cell value and if column is named then column gets the index so it is faster in loops next time value is returned
 * @param uRow row index for cell
 * @param pvariantColumn pointer to variant that can have string or column index
 * @return gd::variant_view value in cell
 */
gd::variant_view table::cell_get_variant_view(uint64_t uRow, std::variant<unsigned, std::string_view>* pvariantColumn) const noexcept
{                                                                                                  assert( pvariantColumn != nullptr );
   if( pvariantColumn->index() == 0 )
   {
      return cell_get_variant_view( uRow, std::get<0>( *pvariantColumn ) );
   }
   else
   {
      unsigned uColumn = column_get_index( std::get<1>( *pvariantColumn ) );
      *pvariantColumn = uColumn;
      return cell_get_variant_view( uRow, uColumn );
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief get cell value using column indexes in container, values are placed in argumentsValue with name and value
 * @param uRow row index to get value from
 * @param spanColumn span with column indexes to get value from
 * @param argumentsValue arguments container to place values in
 */
void table::cell_get( uint64_t uRow, std::span<unsigned> spanColumn, gd::argument::arguments& argumentsValue ) const
{                                                                                                  assert( uRow < size() );
   for( unsigned uColumn : spanColumn )
   {
      std::string_view stringColumn = column_get_name( uColumn );
      argumentsValue.push_back( stringColumn, cell_get_variant_view( uRow, uColumn ) );
   }
}

unsigned table::cell_get_length( uint64_t uRow, unsigned uColumnIndex ) const noexcept
{
   unsigned uLength = 0;

   auto v_ = cell_get_variant_view( uRow, uColumnIndex );
   uLength = variant::compute_ascii_size_s( (const gd::variant*)&v_ );

   return uLength;
}


/** ---------------------------------------------------------------------------
 * @brief Set cell value in table
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param variantviewValue value set to cell
*/
void table::cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue )
{
   unsigned uColumnCount = (unsigned)m_pcolumns->size(); // get column count
   if( uColumn < uColumnCount )
   {
                                                                                                      assert( uRow < m_uReservedRowCount );
      auto& columnSet = *m_pcolumns->get( uColumn );                                                  assert( columnSet.position() < m_uRowSize );
      uint8_t* puRow = row_get( uRow );

      if( variantviewValue.is_null() == false )
      {
   #ifndef NDEBUG 
         auto uValueType_d = variantviewValue.type_number();
         auto uColumnType_d = columnSet.ctype_number();
         if( uValueType_d != uColumnType_d ) {                                    // check type, this has to match. You can't set value from type that differ from type in column
            auto stringValueType_d = gd::types::type_name_g( uValueType_d );
            auto stringColumnType_d = gd::types::type_name_g( uColumnType_d );
                                                                                                      assert( uValueType_d == uColumnType_d );
         }
   #endif // !NDEBUG

         auto puBuffer = variantviewValue.get_value_buffer();                     // get pointer to value buffer

         auto puRowValue = puRow + columnSet.position();                          // get position to value in row

         if( columnSet.is_fixed() )
         {                                                                                            assert( columnSet.primitive_size() != 0 );
            memcpy( puRowValue, puBuffer, columnSet.primitive_size() );           // copy value to cell buffer
         }
         else
         {   
            if( columnSet.is_length() == true )
            {                                                                                         assert( variantviewValue.length() <= columnSet.size() );
               unsigned uMaxSize = columnSet.size();
               unsigned uLength = gd::types::value_size_g( variantviewValue.type(), variantviewValue.length() ); // value size in bytes (remember that non fixed value types starts with length information before actual value)
               if( uLength <= uMaxSize )
               {
                  *( uint32_t* )puRowValue = variantviewValue.length();           // set length as first part in buffer
                  puRowValue += sizeof( uint32_t );                               // move to data
                  memcpy( puRowValue, puBuffer, uLength );                        // copy value
               }
            }
            else if( columnSet.is_reference() == true )
            {
               // ## try to find value and store index for found value if it exists, if not add and store new index
               int64_t iIndex = m_references.find( variantviewValue );
               if( iIndex == -1 )
               {
                  iIndex = (int64_t )m_references.add( variantviewValue );
               }

               memcpy( puRowValue, &iIndex, sizeof( intptr_t ) );                 // copy index value into cell
            }
            else { assert(false); }
         }



         if( is_null() == true ) { cell_set_not_null( uRow, uColumn ); }          // set flag that cell has a value if table is using row status meta data
      }
      else
      {
         if( is_null() == true ) { cell_set_null( uRow, uColumn ); }              // cell is null, set null flag
      }
   }
   else
   {                                                                                               assert( is_rowarguments() == true );
      // if column index is larger than column count then it is tag_arguments
      gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow);
      if( *(intptr_t*)pargumentsRow != 0 )
      {
         // if column index is larger than column count then it is tag_arguments
         unsigned uArgumentIndex = uColumn - uColumnCount;                                              
         gd::argument::shared::arguments::pointer pposition_ = pargumentsRow->find( uArgumentIndex );
         if( pposition_ != nullptr )
         {
            pargumentsRow->set( pposition_, variantviewValue );
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
void table::cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue )
{                                                                                                  assert( uRow < m_uReservedRowCount );
   int iColumnIndex = column_find_index( stringName );
   if( iColumnIndex != -1 )
   {
      cell_set(uRow, (unsigned)iColumnIndex, variantviewValue);
   }
   else
   {
      cell_set_argument( uRow, stringName, variantviewValue );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value
 * @param uRow row index for cell
 * @param stringAlias column alias (column has to have a alias)
 * @param variantviewValue value set to cell and cell type need to match
*/
void table::cell_set( uint64_t uRow, const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_alias )
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringAlias, tag_alias{});                            assert(uColumnIndex != ( unsigned )-1);
   cell_set( uRow, uColumnIndex, variantviewValue );
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
void table::cell_set( uint64_t uRow, unsigned uColumn, const gd::variant_view& variantviewValue, tag_convert )
{
   auto& columnSet = *m_pcolumns->get( uColumn );                                                  assert( columnSet.position() < m_uRowSize );
   auto uValueType = variantviewValue.type_number();
   auto uColumnType = columnSet.ctype_number();

   if( uValueType == uColumnType ) 
   {  
      cell_set( uRow, uColumn, variantviewValue ); 
   }
   else
   {
      gd::variant variantConvertTo;
      bool bOk = variantviewValue.convert_to( uColumnType,  variantConvertTo );
      if( bOk == true )
      {
         cell_set( uRow, uColumn, *(gd::variant_view*)&variantConvertTo );     // just cast to variant view, internal data is same just that varaiant view have different logic
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value
 * If value type do not match the type used in column then value is converted to proper type
 * @param uRow row index for cell
 * @param stringName column name (column has to have a name)
 * @param variantviewValue value set to cell and cell type need to match
*/
void table::cell_set( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_convert )
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringName );                                         assert( uColumnIndex != (unsigned)-1 );
   cell_set( uRow, uColumnIndex, variantviewValue, tag_convert{});
}

/** ---------------------------------------------------------------------------
 * @brief Set cell value
 * If value type do not match the type used in column then value is converted to proper type
 * @param uRow row index for cell
 * @param stringAlias column alias (column has to have a name)
 * @param variantviewValue value set to cell and cell type need to match
*/
void table::cell_set( uint64_t uRow, const std::string_view& stringAlias, const gd::variant_view& variantviewValue, tag_convert, tag_alias )
{                                                                                                  assert( uRow < m_uReservedRowCount );
   unsigned uColumnIndex = column_get_index( stringAlias, tag_alias{});                            assert(uColumnIndex != ( unsigned )-1);
   cell_set( uRow, uColumnIndex, variantviewValue, tag_convert{});
}

/** ---------------------------------------------------------------------------
 * @brief Set cell values from values within vector of variant_view values
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param vectorValue list of values set starting from column index
*/
void table::cell_set( uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue )
{                                                                                                  assert( uRow < m_uReservedRowCount ); assert( uColumn < m_pcolumns->size() ); assert( (uColumn + vectorValue.size()) <= m_pcolumns->size() );
   for( auto it : vectorValue ) { cell_set( uRow, uColumn, it ); uColumn++; }
}

/** ---------------------------------------------------------------------------
 * @brief Set cell values from values within vector of variant_view values
 * If value type differ in variant_view object then this tries to convert value to the type for column
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param vectorValue list of values set starting from column index
*/
void table::cell_set( uint64_t uRow, unsigned uColumn, const std::vector<gd::variant_view>& vectorValue, tag_convert )
{                                                                                                  assert( uRow < m_uReservedRowCount ); assert( uColumn < m_pcolumns->size() ); assert( (uColumn + vectorValue.size()) <= m_pcolumns->size() );
   for( auto it : vectorValue ) { cell_set( uRow, uColumn, it, tag_convert{} ); uColumn++; }
}

/** ---------------------------------------------------------------------------
 * @brief Set values in cells for area in range
 * @param rangeSet holds top-left and bottom-right for area where values are set
 * @param variantviewValue values set to cells in range
*/
void table::cell_set( const range& rangeSet, const gd::variant_view& variantviewValue, tag_convert )
{                                                                                                  assert( rangeSet.r1() < get_row_count() ); assert( rangeSet.is_r2() == false || rangeSet.r2() < get_row_count() );
   if( rangeSet.is_r2() == true )
   {
      for( auto uRow = rangeSet.r1(); uRow <= rangeSet.r2(); uRow++ )
      {
         for( auto uColumn = rangeSet.c1(); uColumn <= rangeSet.c2(); uColumn++ )
         {
            cell_set( uRow, (unsigned)uColumn, variantviewValue, tag_convert{});
         }
      }
   }
   else
   {
      cell_set( rangeSet.r1(), (unsigned)rangeSet.c1(), variantviewValue, tag_convert{} );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Set values in cells for area in range
 * @param rangeSet holds top-left and bottom-right for area where values are set
 * @param variantviewValue values set to cells in range
*/
void table::cell_set( const range& rangeSet, const gd::variant_view& variantviewValue )
{                                                                                                  assert( rangeSet.r1() < get_row_count() ); assert( rangeSet.is_r2() == false || rangeSet.r2() < get_row_count() );
   if( rangeSet.is_r2() == true )
   {
      for( auto uRow = rangeSet.r1(); uRow <= rangeSet.r2(); uRow++ )
      {
         for( auto uColumn = rangeSet.c1(); uColumn <= rangeSet.c2(); uColumn++ )
         {
            cell_set( uRow, (unsigned)uColumn, variantviewValue );
         }
      }
   }
   else
   {
      cell_set( rangeSet.r1(), (unsigned)rangeSet.c1(), variantviewValue );
   }
}

/** --------------------------------------------------------------------------
 * @brief set cell value in table
 * 
 * This function is used to set value in row arguments object, if row arguments object is not created then it is created
 * 
 * @param uRow index to row where cell value is set
 * @param stringName column name for cell value
 * @param variantviewValue value set to cell
 */
void table::cell_set_argument( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue ) 
{                                                                                                  assert(uRow < m_uReservedRowCount);
   gd::argument::shared::arguments* parguments = row_get_arguments_pointer(uRow);
   // ## Check if row hold any arguments object, if not then create one
   if( parguments == nullptr )
   {                                                                                               assert( is_rowarguments() == true ); // need flag that row arguments are used
      parguments = row_create_arguments( uRow );
   }

   parguments->set(stringName, variantviewValue);
}

/** --------------------------------------------------------------------------
 * @brief add cell value in table
 * 
 * This function is used to add argument value to row arguments object, if row arguments object is not created then it is created
 * 
 * @param uRow index to row where cell value is set
 * @param stringName column name for cell value
 * @param variantviewValue value set to cell
 */
void table::cell_add_argument( uint64_t uRow, const std::string_view& stringName, const gd::variant_view& variantviewValue ) 
{                                                                                                  assert(uRow < m_uReservedRowCount);
   gd::argument::shared::arguments* parguments = row_get_arguments_pointer(uRow);
   // ## Check if row hold any arguments object, if not then create one
   if( parguments == nullptr )
   {                                                                                               assert( is_rowarguments() == true ); // need flag that row arguments are used
      parguments = row_create_arguments( uRow );
   }

   parguments->append_argument(stringName, variantviewValue);
}




table::row_value_type table::row_get( uint64_t uRow, tag_cell )
{
   std::vector< gd::table::cell< table > > vectorCell;
   for( auto u = 0u, uMax = ( unsigned )m_pcolumns->size(); u < uMax; u++ )
   {
      vectorCell.push_back( gd::table::cell< table >( this, uRow, u ) );
   }
   return vectorCell;
}

/*
table_column_buffer::row_const_value_type table_column_buffer::row_get( uint64_t uRow, tag_cell ) const
{
   std::vector< gd::table::cell< table_column_buffer > > vectorCell;
   for( auto u = 0u, uMax = ( unsigned )m_vectorColumn.size(); u < uMax; u++ )
   {
      vectorCell.push_back( gd::table::cell< table_column_buffer >( this, uRow, u ) );
   }
   return vectorCell;
}
*/


/** ---------------------------------------------------------------------------
 * @brief Return row values in vector as variant view items
 * @param uRow index to row values are returned from
 * @return std::vector<const gd::variant_view> vector holding row values
*/
std::vector<gd::variant_view> table::row_get_variant_view( uint64_t uRow ) const
{                                                                                                  assert( uRow < 0x0100'0000 ); assert( uRow < m_uReservedRowCount );  assert( m_pcolumns != nullptr );
   std::vector<gd::variant_view> vectorValue;

   for( auto u = 0u, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++ )
   {
      vectorValue.push_back( cell_get_variant_view( uRow, u ) );
   }

   if( is_rowarguments() == true )
   {
      // ## if row arguments object for row then get those values

      using namespace gd::argument::shared;
      const gd::argument::shared::arguments* parguments_ = row_get_arguments_pointer(uRow);
      if( parguments_!= nullptr )
      {
         // ## append arguments from row arguments object

         const auto& arguments_ = *parguments_;
         for( auto* pPosition = arguments_.next(); pPosition != nullptr; pPosition = arguments_.next(pPosition) )
         {
            auto v_ = gd::argument::shared::arguments::get_argument_s(pPosition).as_variant_view(); // get argument as variant_view 
            vectorValue.push_back(v_);                                        // add to vector
         }
      }// if( parguments_!= nullptr )
   }// if( is_rowarguments() == true )

   return vectorValue;
}

/** ---------------------------------------------------------------------------
 * @brief Return row values in vector as variant view items
 * 
 * @code
gd::table::arguments::table table_( gd::table::tag_full_meta{} );
table_.column_prepare();
table_.column_add("rstring", 0, "path");
table_.prepare();

auto uRow = table_.row_add_one();
table_.cell_set(uRow, "path", gd::variant_view("C:\\test\\file.txt"));
table_.cell_set(uRow, "path2", gd::variant_view("C:\\test\\file2.txt"));

std::vector<gd::variant_view> vectorGet;
vectorGet = table_.row_get_variant_view( uRow, {0, 1});
std::cout << "\n\nRow Variant View): ";
for (const auto& variant : vectorGet) {
   std::cout << variant.as_string() << " ";
}
std::cout << std::endl;
 * @endcode
 * 
 * @param uRow index to row values are returned from
 * @param puIndex pointer to array with column index values harvested into vector
 * @param uSize number of values to harvest
 * @return std::vector<gd::variant_view> values from row
*/
std::vector<gd::variant_view> table::row_get_variant_view( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const
{                                                                                                  assert( uRow < m_uReservedRowCount );  assert( m_pcolumns != nullptr );
   std::vector<gd::variant_view> vectorValue;
   unsigned uColumnCount = (unsigned)m_pcolumns->size(); // get column count

   for( unsigned u = 0; u < uSize; u++ )
   {
      unsigned uColumn = puIndex[u]; // column index to get value from
      if( uColumn >= uColumnCount )
      {
         uColumn -= uColumnCount;                                             // if column index is larger than column count then it is tag_arguments
         vectorValue.push_back( cell_get_variant_view( uRow, uColumn, tag_arguments{} ) );
      }
      else
      {
         vectorValue.push_back( cell_get_variant_view( uRow, uColumn ) );
      }
   }

   return vectorValue;
}

/** ---------------------------------------------------------------------------
 * @brief Find row with variant view value in column
 * @param uColumn column index to search in
 * @param variantviewFind value to find in column
 * @return int64_t index to row where value is found, -1 if not found
*/
int64_t table::row_get_variant_view( unsigned uColumn, const gd::variant_view& variantviewFind, std::vector<gd::variant_view>& vectorValue ) const
{                                                                                                  assert( m_pcolumns != nullptr );
   int64_t iRow = find_variant_view( uColumn, variantviewFind );
   if( iRow >= 0 )
   {
      row_get_variant_view( (uint64_t)iRow, vectorValue );
   }

   return iRow;
}

/** ---------------------------------------------------------------------------
 * @brief Harvest row values in vector with variant view items
 * @param uRow index to row values are returned from
 * @param vectorValue row values are placed in vector
*/
void table::row_get_variant_view( uint64_t uRow, std::vector<gd::variant_view>& vectorValue ) const
{                                                                                                  assert( uRow < 0x0100'0000 ); assert( uRow < m_uReservedRowCount );
   for( auto u = 0u, uMax = (unsigned)m_pcolumns->size(); u < uMax; u++ )
   {
      vectorValue.push_back( cell_get_variant_view( uRow, u ) );
   }
}

/// add row values for column indexes sent
void table::row_get_variant_view(uint64_t uRow, const unsigned* puIndex, unsigned uSize, std::vector<gd::variant_view>& vectorValue) const
{
   for( unsigned u = 0; u < uSize; u++ )
   {                                                                                               assert( puIndex[u] < get_column_count() );
      vectorValue.push_back( cell_get_variant_view( uRow, puIndex[u] ) );
   }
}


/** ---------------------------------------------------------------------------
 * @brief append values to arguments from a specific row in a table
 * @param uRow index to row in table to add values from
 * @return argumentsValue arguments values are added to
*/
void table::row_get_arguments( uint64_t uRow, gd::argument::arguments& argumentsValue ) const
{                                                                                                  assert( uRow < 0x0100'0000 ); assert( uRow < m_uReservedRowCount );
   for( auto it = column_begin(), itEnd = column_end(); it != itEnd; it++ ) 
   {
      auto stringColumnName = it->name();
      gd::variant_view variantValue = cell_get_variant_view(uRow, stringColumnName);
      // check if the cell value isn't empty, and if not then add to arguments
      if(variantValue.is_null() == false ) { argumentsValue.append_argument(stringColumnName, variantValue);  }
   }

   // ## if row arguments object for row then get those values
   if( is_rowarguments() == true )
   {
      using namespace gd::argument::shared;
      gd::argument::shared::arguments* parguments_ = row_get_arguments_pointer(uRow);
      if( parguments_!= nullptr )
      {
         // ## append arguments from row arguments object
         auto& arguments_ = *parguments_;
         for( auto* pPosition = arguments_.next(); pPosition != nullptr; pPosition = arguments_.next(pPosition) )
         {
            if( gd::argument::shared::arguments::is_name_s( pPosition ) == true )
            {
               auto stringName = gd::argument::shared::arguments::get_name_s(pPosition);           assert( stringName.length() < 256 );
               auto argument_ = gd::argument::shared::arguments::get_argument_s(pPosition);        // convert to internal argument
               auto variantviewValue = gd::argument::shared::arguments::get_variant_view_s(argument_); // get variant_view that is compatible with returned arguments object
               argumentsValue.append_argument(stringName, variantviewValue);
            }
            else
            {
               auto variantValue = gd::argument::shared::arguments::get_variant_view_s(pPosition); 
               argumentsValue.append_argument( variantValue, gd::types::tag_view{});
            }
         }
      }// if( parguments_!= nullptr )
   }// if( is_rowarguments() == true )
}

/** ---------------------------------------------------------------------------
 * @brief return row data in arguments object
 * @param uRow index to row values are returned from
 * @param puIndex pointer to unsigned index array
 * @param uSize number of indexes in array
 * @return gd::argument::arguments arguments object with values from row
*/
gd::argument::arguments table::row_get_arguments( uint64_t uRow, const unsigned* puIndex, unsigned uSize ) const
{
   gd::argument::arguments argumentsValue;
   for( unsigned u = 0; u < uSize; u++ )
   {                                                                                               assert( puIndex[u] < get_column_count() );
      auto stringColumnName = column_get_name( puIndex[u] );
      gd::variant_view variantValue = cell_get_variant_view(uRow, puIndex[u] );
      if(variantValue.is_null() == false ) { argumentsValue.append_argument(stringColumnName, variantValue);  }
   }

   return argumentsValue;
}

/** ---------------------------------------------------------------------------
 * @brief creates arguments object and place this in slot for arguments in meta data for row
 * @param uRow index to row arguments object is created for
 * @return pointer to row metadata created
 */
gd::argument::shared::arguments* table::row_create_arguments(uint64_t uRow)
{                                                                                                  assert(row_is_arguments(uRow) == false);
   // ## get row meta data for arguments slot and make sure this is zeroed out
   gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow); assert( *(intptr_t*)pargumentsRow == 0 );
   // ### create arguments object and place it in row meta data
   new ( pargumentsRow ) gd::argument::shared::arguments();
   return pargumentsRow;
}

/** ---------------------------------------------------------------------------
 * @brief get pointer to row arguments object
 * @param uRow index to row where arguments object is found
 * @return pointer to row arguments object
*/
gd::argument::shared::arguments* table::row_get_arguments_pointer(uint64_t uRow) const noexcept
{                                                                                                  assert(row_is_arguments(uRow) == true);
   gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow);
   if( *(intptr_t*)pargumentsRow != 0 ) return pargumentsRow;
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief get pointer to row arguments object, if not found then create one
 * @param uRow index to row where arguments object is found or created
 * @return pointer to row arguments object
*/
gd::argument::shared::arguments* table::row_get_arguments_pointer(uint64_t uRow)
{
   gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow);
   if( *(intptr_t*)pargumentsRow != 0 ) return pargumentsRow;

   return row_create_arguments( uRow );
}

/** ---------------------------------------------------------------------------
 * @brief delete row arguments object for row
 * @param uRow index to row where arguments object is deleted
*/
void table::row_arguments_delete( uint64_t uRow )
{                                                                                                  assert( row_is_arguments( uRow ) == true );
   gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(uRow);
   if( *(intptr_t*)pargumentsRow != 0 )
   {
      pargumentsRow->~arguments();                                            // call destructor
      *(intptr_t*)pargumentsRow = 0;                                          // clear pointer at position in row meta data
   }
}

/** ---------------------------------------------------------------------------
 * @brief Iterates rows in table, harvest row values and call the callback method with row values
 * @param callback_ callback called for each row in table with cell values in row
 * @return true if rows was processed (depends on callback)
*/
bool table::row_for_each( std::function<bool( std::vector<gd::variant_view>&, uint64_t )> callback_ )
{
   std::vector<gd::variant_view> vectorValue;
   for( uint64_t uRow = 0; uRow < m_uRowCount; uRow++ )
   {
      row_get_variant_view( uRow, vectorValue );
      if( callback_( vectorValue, uRow ) == true )
      {
         vectorValue.clear();
         continue;
      }
      else
      {
         return false;
      }
   }
   return true;
}

bool table::row_for_each( std::function<bool( const std::vector<gd::variant_view>&, uint64_t )> callback_ ) const
{
   std::vector<gd::variant_view> vectorValue;
   for( uint64_t uRow = 0; uRow < m_uRowCount; uRow++ )
   {
      row_get_variant_view( uRow, vectorValue );
      if( callback_( vectorValue, uRow ) == true )
      {
         vectorValue.clear();
         continue;
      }
      else
      {
         return false;
      }
   }
   return true;
}

bool table::row_for_each( uint64_t uFrom, uint64_t uCount, std::function<bool( std::vector<gd::variant_view>&, uint64_t )> callback_ )
{                                                                                                  assert( uFrom < m_uRowCount );
   uint64_t uTo = (uFrom + uCount) < m_uRowCount ? uFrom + uCount : m_uRowCount;
   std::vector<gd::variant_view> vectorValue;
   for( uint64_t uRow = uFrom; uRow < uTo; uRow++ )
   {
      row_get_variant_view( uRow, vectorValue );
      if( callback_( vectorValue, uRow ) == true )
      {
         vectorValue.clear();
         continue;
      }
      else
      {
         return false;
      }
   }
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Loop rows and extract value in specified column that is sent to callback
 * @param uColumn index to column to get value sent to callback
 * @param uFrom start row index
 * @param uCount number of rows
 * @param callback_ callback called for each row
 * @return true if row was processed, false if not all rows was processed
*/
bool table::row_for_each( unsigned uColumn, uint64_t uFrom, uint64_t uCount, std::function<bool( const gd::variant_view&, uint64_t )> callback_ ) const
{
   uint64_t uTo = (uFrom + uCount) < m_uRowCount ? uFrom + uCount : m_uRowCount;
   for( uint64_t uRow = uFrom; uRow < uTo; uRow++ )
   {
      auto variantviewValue = cell_get_variant_view( uRow, uColumn );
      if( callback_( variantviewValue, uRow ) == true )
      {
         continue;
      }
      else
      {
         return false;
      }
   }
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Iterates rows in table, harvest row values and call the callback method with row values
 * 
~~~(.cpp)
   gd::database::sqlite::database databaseWrite;
   // connect to database, omitted...
   gd::database::sqlite::cursor cursorInsert( &databaseWrite );
   // prepare cursor with insert query, omitted ...

   auto tableScatterPlot = GenerateTableData( 1000 ); // code to generate table data
   
   uint64_t uParameterOffset = 0;
   (*(const gd::table::table_column_buffer*)&tableScatterPlot).row_for_each( uSection * uBulkCcount, uBulkCcount, [&cursorInsert, &uParameterOffset]( const auto& vectorValue, auto uIndex ) {
      cursorInsert.bind_parameter( (int)uParameterOffset + 1, vectorValue );// Bind harvested row values to query
      uParameterOffset += vectorValue.size();
      return true;
   } );
   auto [ bOk, stringError ] = cursorInsert.execute();
~~~
 * 
 * @param uFrom First row to process
 * @param uCount Number of rows to process after first
 * @param callback_ callback called from method
 * @return true if all rows was processed, false in cancellation
*/
bool table::row_for_each( uint64_t uFrom, uint64_t uCount, std::function<bool( const std::vector<gd::variant_view>&, uint64_t )> callback_ ) const
{                                                                                                  assert( uFrom < m_uRowCount );
   uint64_t uTo = (uFrom + uCount) < m_uRowCount ? uFrom + uCount : m_uRowCount;
   std::vector<gd::variant_view> vectorValue;
   for( uint64_t uRow = uFrom; uRow < uTo; uRow++ )
   {
      row_get_variant_view( uRow, vectorValue );
      if( callback_( vectorValue, uRow ) == true )
      {
         vectorValue.clear();
         continue;
      }
      else
      {
         return false;
      }
   }
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Returns the absolut position for row in table
 * Sometimes you have some sort of relative position to row in table and need the
 * absolut position based om some sort of table attribute. This method returns the
 * absolut position based on row status.
 * @param uRelativeRow Relative row position
 * @param uStatus status values used to 
 * @return int64_t index to absolut row or -1 if relative row position is not reached
 */
int64_t table::row_get_absolute(uint64_t uRelativeRow, unsigned uStatus) const
{                                                                                                  assert( m_puMetaData != nullptr ); assert( is_rowstatus() == true ); assert( uRelativeRow < get_row_count() );
   auto uRowMetaSize = size_row_meta();
   const auto* puPosition = m_puMetaData;

   uint64_t uMatchRow = 0;    // rows matched against status
   uint64_t uRow = 0;         // absolute row
   for( uint64_t uRowCount = get_row_count(); uMatchRow < uRelativeRow && uRow < uRowCount; uRow++ )
   {
      if( (*reinterpret_cast<const uint32_t*>( puPosition ) & uStatus ) == uStatus ) uMatchRow++;
      puPosition += uRowMetaSize;
   }

   if( uMatchRow == uRelativeRow ) return uRow;                                // if relative row is reached then return absolut row position

   return -1;                                                                  // not found, -1 is returned
}



/** ---------------------------------------------------------------------------
 * @brief find value in column
 * @param uColumn index to column to find value in
 * @param uStartRow Row to start finding for value in
 * @param uCount number of rows to find value
 * @param variantviewFind value to find
 * @return int64_t index for row if found, -1 if not found
*/
int64_t table::find( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept
{                                                                                                  assert( m_puData );
   auto& columnSet = *m_pcolumns->get( uColumn );                                                  assert( variantviewFind.type_number() == columnSet.ctype_number() );
   //const uint8_t* puFindValue = variantviewFind.data();

   if( variantviewFind.is_primitive() )
   {
      uint64_t uEndRow = uStartRow + uCount;                                                       assert( uEndRow <= get_row_count() );

      // ## optimization to find value, this is to direct access value in table data with method `cell_get` that return pointer to value
      if( variantviewFind.is_64() == true )                                    // is value a 64 bit value
      {
         uint64_t uFind = *(uint64_t*)variantviewFind.data();
         for( auto uRow = uStartRow; uRow < uEndRow; uRow++ )
         {
            auto uValue = *(const uint64_t*)cell_get( uRow, uColumn );
            if( uValue == uFind ) return ( int64_t )uRow;
         }
      }
      else
      {                                                                        // 32 bit value (remember that each value is at least 32 bit in table)
         uint32_t uFind = variantviewFind.as_uint();
         for( auto uRow = uStartRow; uRow < uEndRow; uRow++ )
         {
            auto uValue = *(const uint32_t*)cell_get( uRow, uColumn );
            if( uValue == uFind ) return ( int64_t )uRow;
         }
      }
   }
   else
   {
      return find_variant_view( uColumn, uStartRow, uCount, variantviewFind );
   }

   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief find value in table
 * @param uColumn index for column where to find value
 * @param uStartRow row to start search
 * @param uCount number of rows trying to find value in
 * @param variantviewFind value to find
 * @return index to row if value was found, -1 if not found
*/
int64_t table::find_variant_view( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept
{                                                                                                  assert( m_puData );
   auto& columnSet = *m_pcolumns->get( uColumn );                                                  assert( variantviewFind.type_number() == columnSet.ctype_number() );
   uint64_t uEndRow = uStartRow + uCount;                                                          assert( uEndRow <= get_row_count() );
   for( auto uRow = uStartRow; uRow < uEndRow; uRow++ )
   {
      auto variantviewValue = cell_get_variant_view( uRow, uColumn );
      if( variantviewFind == variantviewValue ) return (int64_t)uRow;
   }

   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief find value in table
 * @note checks if row is in used before match of value
 * @param uColumn index for column where to find value
 * @param uStartRow row to start search
 * @param uCount number of rows trying to find value in
 * @param variantviewFind value to find
 * @return index to row if value was found, -1 if not found
*/
int64_t table::find_variant_view( unsigned uColumn, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind, tag_meta ) const noexcept
{                                                                                                  assert( m_puData ); assert( m_puMetaData != nullptr );
   auto& columnSet = *m_pcolumns->get( uColumn );                                                  assert( variantviewFind.type_number() == columnSet.ctype_number() );
   uint64_t uEndRow = uStartRow + uCount;                                                          assert( uEndRow <= get_row_count() );
   for( auto uRow = uStartRow; uRow < uEndRow; uRow++ )
   {
      if( row_is_use( uRow ) == true )                                         // check if row is in use
      {
         auto variantviewValue = cell_get_variant_view( uRow, uColumn );
         if( variantviewFind == variantviewValue ) return (int64_t)uRow;       // found value?
      }
   }

   return -1;
}


/** ---------------------------------------------------------------------------
 * @brief Find row for sorted column
 * Find value in table based on sorted column. Note that to find value using this method you need a 
 * sorted table for the specified column where value exists.
 * @param uColumn index to column value is searched for
 * @param bAscending if column is sorted in ascending or descending order
 * @param uStartRow row to begin search
 * @param uCount number of rows to search in
 * @param variantviewFind value searched for
 * @return index to row if found, -1 if not found
*/
int64_t table::find_variant_view( unsigned uColumn, bool bAscending, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind ) const noexcept
{
   uint64_t uLow = uStartRow;
   uint64_t uHigh = uStartRow + uCount;
   

   if( bAscending == true )
   {
      while( uHigh > uLow )
      {
         uint64_t uMid = (uLow + uHigh) / 2;
         auto value_ = cell_get_variant_view( uMid, uColumn );

         if( value_ == variantviewFind ) return ( int64_t )uMid;
         else if( value_ < variantviewFind )
         {
            uLow = uMid + 1;
         }
         else
         {
            uHigh = uMid - 1;
         }
      }
   }
   else
   {
      while( uHigh > uLow )
      {
         uint64_t uMid = (uLow + uHigh) / 2;
         auto value_ = cell_get_variant_view( uMid, uColumn );

         if( variantviewFind == value_ ) return ( int64_t )uMid;
         else if( variantviewFind < value_ )
         {
            uLow = uMid + 1;
         }
         else
         {
            uHigh = uMid - 1;
         }
      }
   }

   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief find start row for value and number of rows with same value in column
 * @param uColumn index to column where value is searched for
 * @param bAscending sort order for column
 * @param uStartRow first row where search starts
 * @param uCount number of rows to search
 * @param variantviewFind value to find
 * @return range area where value was found
*/
range table::find_variant_view( unsigned uColumn, bool bAscending, uint64_t uStartRow, uint64_t uCount, const gd::variant_view& variantviewFind, tag_range ) const noexcept
{
   range rangeFind( uColumn, tag_columns{} );
   int64_t iIndex = find_variant_view( uColumn, bAscending, uStartRow, uCount, variantviewFind );
   if( iIndex != -1 )
   {
      int64_t iFirstRow = iIndex;
      int64_t iLastRow = iIndex;
      // ## find first and last row with value
      for( int64_t iRow = iIndex - 1; iRow >= ( int64_t )uStartRow; iRow-- )
      {
         auto value_ = cell_get_variant_view( iRow, uColumn );
         if( variantviewFind == value_ ) iFirstRow = iRow;
         else break;
      }

      for( int64_t iRow = iIndex + 1; iRow < ( int64_t )(uStartRow + uCount); iRow++ )
      {
         auto value_ = cell_get_variant_view( iRow, uColumn );
         if( variantviewFind == value_ ) iLastRow = iRow;
         else break;
      }

      rangeFind.rows( iFirstRow, iLastRow );
      
      return rangeFind;
   }

   return range( tag_null{} );
}

size_t table::find_all(uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFilter, std::vector<uint64_t>* pvectorResult) const
{
   size_t uHitCount = 0;
   uint64_t uEndRow = uStartRow + uCount;
   if( uEndRow > get_row_count() ) uEndRow = get_row_count();

   auto itFilterEnd = std::end( vectorFilter );

   for( auto uRow = uStartRow; uRow < uEndRow; uRow++ )
   {
      bool bOk = true;
      for( auto itFilter = std::begin( vectorFilter ); itFilter != itFilterEnd && bOk == true; itFilter++ )
      {
         auto value_ = cell_get_variant_view( uRow, itFilter->first );
         if( value_.compare( itFilter->second ) == true ) continue;
         
         bOk = false;
      }

      if(bOk == true)
      {
         uHitCount++;
         if( pvectorResult != nullptr ) pvectorResult->push_back( uRow );
      }
   }

   return uHitCount;
}

std::vector<uint64_t> table::find_all(uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFilter ) const
{                                                                    
   std::vector<uint64_t> vectorResult;
   find_all( uStartRow, uCount, vectorFilter, &vectorResult );
   return vectorResult;
}


std::vector<uint64_t> table::find_all(const std::vector< std::pair<unsigned, gd::variant_view> >& vectorFilter) const
{
   std::vector<uint64_t> vectorResult;
   find_all( 0, get_row_count(), vectorFilter, &vectorResult );
   return vectorResult;
}

std::vector<uint64_t> table::find_all(const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorFilter) const
{
   std::vector< std::pair<unsigned, gd::variant_view> > vector_;
   for(auto it : vectorFilter)
   {
      unsigned uColumn = column_get_index( it.first );
      vector_.push_back( { uColumn, it.second } );
   }

   std::vector<uint64_t> vectorResult = find_all( vector_ );

   return vectorResult;
}

/** ---------------------------------------------------------------------------
 * @brief find row with matching values in row from vector with column index and value to match
 * @param uStartRow where to start searching for matching row
 * @param uCount number of rows to match
 * @param vectorFind vector with column index and matching value to match against row with same values
 * @return int64_t index to row where values are found or -1 if not found
 */
int64_t table::find(uint64_t uStartRow, uint64_t uCount, const std::vector<std::pair<unsigned, gd::variant_view>>& vectorFind) const noexcept
{                                                                                                  assert( (uStartRow + uCount) <= get_row_count() );
   auto itFindBegin = std::begin( vectorFind );
   auto itFindEnd = std::end( vectorFind );

   for( uint64_t uRow = uStartRow, uEnd = uStartRow + uCount; uRow < uEnd; uRow++ )
   {
      bool bFound = true;
      for( auto it = itFindBegin; it != itFindEnd; it++ )
      {
         auto variantviewValue = cell_get_variant_view( uRow, it->first );
         if( it->second != variantviewValue ) { bFound = false; break; }
      }

      if( bFound == true ) return uRow;
   }

   return -1;
}

/// find row with matching values
int64_t table::find(uint64_t uStartRow, uint64_t uCount, const std::vector<gd::variant_view>& vectorFind) const
{                                                                                                  assert( (uStartRow + uCount) <= get_row_count() );
   std::vector<std::pair<unsigned, gd::variant_view>> vectorFind_;
   for( unsigned uColumn = 0; uColumn < vectorFind.size(); uColumn++ )
   {
      vectorFind_.push_back( { uColumn, vectorFind.at( uColumn ) } );
   }

   int64_t iRow = find( uStartRow, uCount, vectorFind_ );
   return iRow;
}

/// find row with matching values
int64_t table::find(uint64_t uStartRow, uint64_t uCount, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorFind) const
{                                                                                                  assert( (uStartRow + uCount) <= get_row_count() );
   // ## convert to column index
   std::vector<std::pair<unsigned, gd::variant_view>> vectorFind_;
   for( size_t u = 0; u < vectorFind.size(); u++ )
   {
      const auto& pair_ = vectorFind.at( u );
      std::string_view stringColumn = pair_.first;
      auto uColumn = column_get_index( stringColumn );
      vectorFind_.push_back( { uColumn, pair_.second } );
   }

   int64_t iRow = find( uStartRow, uCount, vectorFind_ );
   return iRow;
}


/** ---------------------------------------------------------------------------
 * @brief Finds first row that isn't marked as in use
 * @param uStartRow index where to start search for free row
 * @return 
*/
int64_t table::find_first_free_row( uint64_t uStartRow ) const
{                                                                                                  assert( m_puMetaData != nullptr ); assert( is_rowstatus() == true );
   auto uRowMetaSize = size_row_meta();
   auto puPosition = m_puMetaData + uRowMetaSize - eSpaceRowState;             // set position for first row state value
   puPosition += uStartRow * uRowMetaSize;                                     // move to specified start row
   for( auto itRow = uStartRow; itRow < m_uReservedRowCount; itRow++ )
   {
#ifdef _DEBUG
      auto uState_d = *reinterpret_cast<uint32_t*>( puPosition );
#endif // _DEBUG
      // if the use flag not set then row is free
      if( (*reinterpret_cast<uint32_t*>( puPosition ) & eRowStateUse) == 0 ) return itRow;
      puPosition += uRowMetaSize;                                              // move pointer to next row                                                        
   }

   return -1;
}

/** ---------------------------------------------------------------------------
 * @brief Counts number of rows in use
 * Counts all rows that are marked as used in table
 * @note To run this method you row status has to be valid
 * @return uint64_t number of rows used in table
 */
uint64_t table::count_used_rows() const
{                                                                                                  assert( is_rowstatus() == true );
   uint64_t uRowCount = 0;
   unsigned uRowMetaSize = size_row_meta();
   const uint8_t* puPosition = m_puMetaData + uRowMetaSize - eSpaceRowState;   // set position for first row state value

   for( uint64_t uRow = 0; uRow < m_uReservedRowCount; uRow++ )
   {
#ifdef _DEBUG
      auto uState_d = *reinterpret_cast<const uint32_t*>( puPosition );
#endif // _DEBUG
      // if the use flag not set then row is free
      if( (*reinterpret_cast<const uint32_t*>( puPosition ) & eRowStateUse) != 0 ) { uRowCount++; }
      puPosition += uRowMetaSize;                                              // move pointer to next row                                                        
   }

   return uRowCount;
}

/** ---------------------------------------------------------------------------
 * @brief Counts free rows, rows without the mark for being used
 * @note To run this method you row status has to be valid
 * @return uint64_t number of free rows used in table
 */
uint64_t table::count_free_rows() const
{                                                                                                  assert( is_rowstatus() == true );
   uint64_t uRowCount = 0;
   unsigned uRowMetaSize = size_row_meta();
   const uint8_t* puPosition = m_puMetaData + uRowMetaSize - eSpaceRowState;   // set position for first row state value

   for( uint64_t uRow = 0; uRow < m_uReservedRowCount; uRow++ )
   {
#ifdef _DEBUG
      auto uState_d = *reinterpret_cast<const uint32_t*>( puPosition );
#endif // _DEBUG
      // if the use flag not set then row is free
      if( (*reinterpret_cast<const uint32_t*>( puPosition ) & eRowStateUse) == 0 ) { uRowCount++; }
      puPosition += uRowMetaSize;                                              // move pointer to next row                                                        
   }

   return uRowCount;
}


/** ---------------------------------------------------------------------------
 * @brief Fill column with values
 * @param uColumn index for column that is filled with values
 * @param variantviewValue value to fill cells in column with
 * @param uFromRow from row index value are inserted to
 * @param uToRow to row index value are inserted to
*/
void table::column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uFromRow, uint64_t uToRow )
{                                                                                                  assert( uFromRow >= 0 && uFromRow < m_uRowCount ); assert( uToRow >= 0 && uToRow <= m_uRowCount ); assert( uFromRow <= uToRow );
   for( auto uRow = uFromRow; uRow < uToRow; uRow++ )
   {
      cell_set( uRow, uColumn, variantviewValue );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Fill column with values, if column values do not match type it tries to convert to proper type
 * @param uColumn index for column that is filled with values
 * @param variantviewValue value to fill cells in column with
 * @param uFromRow from row index value are inserted to
 * @param uToRow to row index value are inserted to
*/
void table::column_fill( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uFromRow, uint64_t uToRow, tag_convert )
{                                                                                                  assert( uFromRow >= 0 && uFromRow < m_uRowCount ); assert( uToRow >= 0 && uToRow <= m_uRowCount ); assert( uFromRow <= uToRow );
   const auto& columnSet = *m_pcolumns->get( uColumn );                                            assert( columnSet.position() < m_uRowSize );
   auto uColumnType = columnSet.ctype_number();
   gd::variant variantConvertTo;
   bool bOk = variantviewValue.convert_to( uColumnType,  variantConvertTo );

   for( auto uRow = uFromRow; uRow < uToRow; uRow++ )
   {
      cell_set( uRow, uColumn, *(gd::variant_view*)&variantConvertTo );
   }
}


/** ---------------------------------------------------------------------------
 * @brief Fill column with values
 * @param uColumn index for column that is filled with values
 * @param pvariantviewValue pointer to variant buffer holding values to insert to column
 * @param uCount number of items in variant buffer
 * @param uFromRow from row index value are inserted to
*/
void table::column_fill( unsigned uColumn, const gd::variant_view* pvariantviewValue, size_t uCount, uint64_t uFromRow )
{                                                                                                  assert( uFromRow >= 0 && uFromRow < m_uRowCount ); assert( (uFromRow + uCount) <= m_uRowCount ); 
   for( auto uIndex = 0; uIndex < uCount; uIndex++ )
   {
      cell_set( uFromRow + uIndex, uColumn, pvariantviewValue[uIndex]);
   }
}


/** ---------------------------------------------------------------------------
 * @brief add all rows from table
 * Note that structure from the other table need to be equal, code will assert if not but will not
 * report error in runtime. Developer need to check this in debug
 * @param tableFrom table to add rows from
*/
void table::append( const table& tableFrom )
{                                                                                                  assert( get_column_count() == tableFrom.get_column_count() );   
   row_reserve_add( tableFrom.get_row_count() );                               // pre allocate memory for all rows in table to append

   unsigned uColumnCount = get_column_count();
   for( uint64_t uRowFrom = 0, uMaxRowFrom = tableFrom.get_row_count(); uRowFrom < uMaxRowFrom; uRowFrom++ )
   {
      uint64_t uLastRow = get_row_count();
      row_add();

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         cell_set( uLastRow, uColumn, tableFrom.cell_get_variant_view( uRowFrom, uColumn ) );
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief add all rows from table
 * Note that structure from the other table need to be equal, code will assert if not but will not
 * report error in runtime. Developer need to check this in debug
 * @param tableFrom table to add rows from
*/
void table::append( const table& tableFrom, tag_convert )
{                                                                                                  assert( get_column_count() == tableFrom.get_column_count() );   
   row_reserve_add( tableFrom.get_row_count() );                               // pre allocate memory for all rows in table to append

   unsigned uColumnCount = get_column_count();
   for( uint64_t uRowFrom = 0, uMaxRowFrom = tableFrom.get_row_count(); uRowFrom < uMaxRowFrom; uRowFrom++ )
   {
      uint64_t uLastRow = get_row_count();
      row_add();

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         cell_set( uLastRow, uColumn, tableFrom.cell_get_variant_view( uRowFrom, uColumn ), tag_convert{});
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief append rows from table into current where columns do not match
 * Appends a table data and specify where to pick how data is copied between from table to current
 * @param tableFrom table data is appended from
 * @param puColumnIndexFrom list of column index values are copied from 
 * @param puColumnIndexTo list of column index values are copied to
 * @param uColumnCount number of index values in index lists 
*/
void table::append( const table& tableFrom, const unsigned* puColumnIndexFrom, const unsigned* puColumnIndexTo, unsigned uColumnCount )
{
   row_reserve_add( tableFrom.get_row_count() );                               // pre allocate memory for all rows in table to append

   // ## if table supports null values then set all cells that is added to null before inserting values
   if( is_null() == true )                                                     // is table setup to handle null values ?
   {
      for( uint64_t u = get_row_count(), uMax = u + tableFrom.get_row_count(); u < uMax; u++ )
      {
         row_set_null( u );
      }
   }

   // ## Copy values from `tableFrom` into this table
   for( uint64_t uRowFrom = 0, uMaxRowFrom = tableFrom.get_row_count(); uRowFrom < uMaxRowFrom; uRowFrom++ )
   {
      uint64_t uLastRow = get_row_count();
      row_add();

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         cell_set( uLastRow, puColumnIndexTo[uColumn], tableFrom.cell_get_variant_view(uRowFrom, puColumnIndexFrom[uColumn]));
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief append rows from table into current where columns do not match, if column type differs it converts to the right one
 * Appends a table data and specify where to pick how data is copied between from table to current
 * @param tableFrom table data is appended from
 * @param puColumnIndexFrom list of column index values are copied from 
 * @param puColumnIndexTo list of column index values are copied to
 * @param uColumnCount number of index values in index lists 
*/
void table::append( const table& tableFrom, const unsigned* puColumnIndexFrom, const unsigned* puColumnIndexTo, unsigned uColumnCount, tag_convert )
{
   row_reserve_add( tableFrom.get_row_count() );                               // pre allocate memory for all rows in table to append

   // ## if table supports null values then set all cells that is added to null before inserting values
   if( is_null() == true )                                                     // is table setup to handle null values ?
   {
      for( uint64_t u = get_row_count(), uMax = u + tableFrom.get_row_count(); u < uMax; u++ )
      {
         row_set_null( u );
      }
   }

   // ## Copy values from `tableFrom` into this table
   for( uint64_t uRowFrom = 0, uMaxRowFrom = tableFrom.get_row_count(); uRowFrom < uMaxRowFrom; uRowFrom++ )
   {
      uint64_t uLastRow = get_row_count();
      row_add();

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         cell_set( uLastRow, puColumnIndexTo[uColumn], tableFrom.cell_get_variant_view(uRowFrom, puColumnIndexFrom[uColumn]), tag_convert{} );
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Append rows with values from matching column names
 * @param tableAppend table data is appended from
*/
void table::append( const table& tableAppend, tag_name )
{
   std::vector<unsigned> vectorThis;
   std::vector<unsigned> vectorAppend;
   column_match_s( *this, tableAppend, &vectorThis, &vectorAppend, tag_name{} );

   append( tableAppend, vectorThis.data(), vectorAppend.data(), (unsigned)vectorThis.size() );
}

/** ---------------------------------------------------------------------------
 * @brief Append rows with values from matching column names, if types do not match then try to convert to proper type
 * @param tableAppend table data is appended from
*/
void table::append( const table& tableAppend, tag_name, tag_convert )
{
   std::vector<unsigned> vectorThis;
   std::vector<unsigned> vectorAppend;
   column_match_s( *this, tableAppend, &vectorThis, &vectorAppend, tag_name{} );

   append( tableAppend, vectorThis.data(), vectorAppend.data(), (unsigned)vectorThis.size(), tag_convert{});
}

/** ---------------------------------------------------------------------------
 * @brief add specified rows from table
 * @param tableFrom table to add rows from
 * @param uFrom start row to add
 * @param uCount number of rows to add
*/
void table::append( const table& tableFrom, uint64_t uFrom, uint64_t uCount )
{                                                                                                  assert( (uFrom + uCount) <= tableFrom.get_row_count() ); assert( get_column_count() == tableFrom.get_column_count() );
   if( (get_row_count() + uCount) > m_uReservedRowCount ) row_reserve_add( uCount );

   unsigned uColumnCount = get_column_count();
   for( uint64_t uRowFrom = uFrom, uEnd = uFrom + uCount; uRowFrom < uEnd; uRowFrom++ )
   {
      uint64_t uLastRow = get_row_count();
      row_add();

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         auto v_ = tableFrom.cell_get_variant_view( uRowFrom, uColumn );
         if( v_.is_null() == false ) cell_set( uLastRow, uColumn, v_ );
         else                        cell_set_null( uLastRow, uColumn );
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief add specified rows from table
 * @param tableFrom table to add rows from
 * @param uFrom start row to add
 * @param uCount number of rows to add
*/
void table::append( const gd::table::dto::table& tableFrom, uint64_t uFrom, uint64_t uCount )
{                                                                                                  assert( (uFrom + uCount) <= tableFrom.get_row_count() ); assert( get_column_count() == tableFrom.get_column_count() );
   if( (get_row_count() + uCount) > m_uReservedRowCount ) row_reserve_add( uCount );

   unsigned uColumnCount = get_column_count();
   for( uint64_t uRowFrom = uFrom, uEnd = uFrom + uCount; uRowFrom < uEnd; uRowFrom++ )
   {
      uint64_t uLastRow = get_row_count();
      row_add();

      for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ )
      {
         auto v_ = tableFrom.cell_get_variant_view( uRowFrom, uColumn );
         if( v_.is_null() == false ) cell_set( uLastRow, uColumn, v_ );
         else                        cell_set_null( uLastRow, uColumn );
      }
   }
}




/** ---------------------------------------------------------------------------
 * @brief Clears all internal data and columns. 
 * 
 * When table is cleared, to start working with again you need to add columns and
 * prepare it to add rows again.
*/
void table::clear()
{
   m_uFlags = 0;
   m_uRowSize = 0;
   m_uRowMetaSize = 0;
   m_uRowCount = 0;
   m_uReservedRowCount = 0;

   if( m_puData != nullptr ) delete [] m_puData;
   m_puData = nullptr;
   m_puMetaData = nullptr;

   m_argumentsProperty.clear();
}

/** ---------------------------------------------------------------------------
 * @brief compare rows between this table and sent table
 * @param tableEqualTo table comparing rows from
 * @param uBeginRow index to row comparing from
 * @param uCount number of rows to check
 * @return true if rows are equal, false if not
*/
bool table::equal( const table& tableEqualTo, uint64_t uBeginRow, uint64_t uCount ) const noexcept
{
   uint64_t uEndRow = uBeginRow + uCount;                                      // last row 
   unsigned uColumnCount = get_column_count();                                 // number of columns to check
   if( uColumnCount != tableEqualTo.get_column_count() ) return false;

   for( auto uRow = uBeginRow; uRow < uEndRow; uRow++ )
   {
      for( auto uColumn = 0u; uColumn < uColumnCount; uColumn++ )
      {
         auto VVThis = cell_get_variant_view( uRow, uColumn );
         auto VVOther = tableEqualTo.cell_get_variant_view( uRow, uColumn );
         if( VVThis != VVOther ) return false;
      }
   }

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief harvest row values into vector with arguments
 * @param uBeginRow start row
 * @param uCount number of rows to harvest values from
 * @param vectorArguments vector with arguments values where harvested arguments are inserted to
*/
void table::harvest( uint64_t uBeginRow, uint64_t uCount, std::vector<gd::argument::arguments>& vectorArguments ) const
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
void table::harvest( const std::vector<uint64_t>& vectorRow, std::vector<gd::argument::arguments>& vectorArguments ) const
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
void table::harvest( const std::vector<uint64_t>& vectorRow, std::vector< std::vector<gd::variant_view> >& vectorHarvest ) const
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
void table::harvest(const std::vector<uint64_t>& vectorRow, table& tableHarvest) 
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
 * @brief Harvest data from selected rows in table into table that store harvested data
 * @param vectorRow index for rows where values are harvested from
 * @param tableHarvest table to store harvested data
 */
void table::harvest(const std::vector<uint64_t>& vectorRow, gd::table::dto::table& tableHarvest) const
{
   if( tableHarvest.column_empty() == true )                                   // no columns? the create
   {
      for( auto itColumn = column_cbegin(), itEnd = column_cend(); itColumn != itEnd; itColumn++ )
      {
         argument::column column_;
         itColumn->get( column_ );                                             // get column information
         tableHarvest.column_add( column_ );                                   // add column to table
      }
      tableHarvest.set_reserved_row_count( vectorRow.size() );
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
 * @brief Harvest data from selected columns and selected rows in table into table that store harvested data
@code
gd::table::table tableCommand;
auto pcolumns = gd::table::table::new_columns_s();
tableCommand = pcolumns;
pcolumns->release();

tableCommand.column_add({ { "uuid", 0, "id" }, { "rstring", 0, "section" }, { "rstring", 0, "name" }, { "bool", 0, "required"} }, gd::table::tag_type_name{});

for(auto i = 0; i < 10; i++)
{
   std::string stringRowName = "name_" + std::to_string(i);
   tableCommand.row_add( { {"section", "section_name_required"}, {"required", true}, {"name", stringRowName }}, gd::table::tag_arguments{});
   tableCommand.row_add( { {"section", "section_name"}, {"required", false}, {"name", stringRowName }}, gd::table::tag_arguments{});
}

tableCommand.row_add( { {"section", "section_name"}, {"required", false}, {"name", "test not"}}, gd::table::tag_arguments{});

auto vectorRow = tableCommand.find_all( { {"required", true} } );
gd::table::dto::table table2;
tableCommand.harvest( { { "id" }, { "name" } }, vectorRow, table2 );

std::cout << gd::table::to_string( table2, gd::table::tag_io_cli{});
@codeend
 * @param vectorColumn columns to read data from
 * @param vectorRow rows to read data from
 * @param tableHarvest table where data is placed
 */
void table::harvest(const std::vector< unsigned >& vectorColumn, const std::vector<uint64_t>& vectorRow, gd::table::dto::table& tableHarvest) const
{
   if( tableHarvest.column_empty() == true )                                   // no columns? the create
   {
      for(auto itColumn : vectorColumn)
      {
         argument::column column_;
         column_get( itColumn, column_ );                                      // get column information
         tableHarvest.column_add( column_ );                                   // add column to table
      }
      tableHarvest.set_reserved_row_count( vectorRow.size() );
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
      row_get_variant_view( itRow, vectorColumn, vectorRowData );
      tableHarvest.row_add( vectorRowData );
      vectorRowData.clear();
   }
}


/** ---------------------------------------------------------------------------
 * @brief "plant" values from plant table into current table where there is a match for column names
 * @param table planted values are taken from this table
*/
void table::plant( const table& table, tag_name )
{
   auto uCount = std::min( get_row_count(), table.get_row_count() );
   plant( table, 0, uCount, tag_name{});
}

void table::plant( const table& tablePlant, const std::string_view& stringColumnName )
{
   int iColumnFrom = tablePlant.column_find_index( stringColumnName );                             assert( iColumnFrom != -1 );
   int iColumnTo = column_find_index( stringColumnName );                                          assert( iColumnTo != -1 );

   plant( tablePlant, iColumnFrom, iColumnTo, 0, get_row_count() );
}


/** ---------------------------------------------------------------------------
 * @brief "plant" values from plant table into current table where matching column names exists
 * 
 * @code
TEST_CASE( "plant table data", "[table]" ) {

   // ## table to "plant" values to
   gd::table::table_column_buffer table( 10, gd::table::tag_null{});
   table.column_add( { {"int16", 0, "level"},
                       {"uint64", 0, "id"},
                       {"int32", 0, "body_type_id"},
                       {"int32", 0, "graph_id"},
                       {"int32", 0, "trajectory"} }, gd::table::tag_type_name{} );
   table.prepare();
   table.set_row_count( 10 );                                                  // set number of rows
   table.row_set_null( 0, 10 );                                                // set all values to null


   // ## table to plant values from
   gd::table::table_column_buffer tablePlant( 10, gd::table::tag_null{});
   tablePlant.column_add( { {"int16", 0, "level"},
                            {"uint64", 0, "id"},
                            {"int32", 0, "trajectory"} }, gd::table::tag_type_name{} );
   tablePlant.prepare();

   tablePlant.row_add( {1,2,3}, gd::table::tag_convert() );
   tablePlant.row_add( {1,2,3}, gd::table::tag_convert() );
   tablePlant.row_add( {1,2,3}, gd::table::tag_convert() );
   tablePlant.row_add( {1,2,3}, gd::table::tag_convert() );

   table.plant( tablePlant, gd::table::tag_name{} );                           // plant values

   std::cout << "plant table: \n" << gd::table::debug::print(tablePlant) << "\n";
   std::cout << "result table: \n" << gd::table::debug::print( table ) << "\n";
}
 * @endcode
 * 
 * @param tablePlant table to take values from 
 * @param uFrom row index where values start to paste values in table
 * @param uCount number of rows to paste values to
*/
void table::plant( const table& tablePlant, uint64_t uFrom, uint64_t uCount, tag_name )
{
   // find indexes for names in source
   /*
   std::vector< std::pair< unsigned, unsigned > > vectorMatch;
   for( unsigned u = 0, uMax = tablePlant.get_column_count(); u != uMax; u++ )
   {
      auto name_ = tablePlant.column_get_name( u );
      int iFindColumn = column_find_index( name_ );
      if( iFindColumn != -1 ) { vectorMatch.push_back( { u, (unsigned)iFindColumn } ); }
   }
   */
   auto vectorMatch = column_match_s( *this, tablePlant, tag_name{});

   // ## Loop from "from" row and number of count rows extracting selected values
   //    and set to current table in matching cell.
   for( uint64_t uRow = uFrom, uToRow = uFrom + uCount; uRow < uToRow; uRow++ )
   {
      for( const auto& itIndex : vectorMatch )
      {
         auto value_ = tablePlant.cell_get_variant_view( uRow - uFrom, itIndex.second );
         cell_set( uRow, itIndex.first, value_ );
      }
   }
}

void table::plant( const table& tablePlant, const std::string_view& stringColumnName, uint64_t uFrom, uint64_t uCount )
{
   int iColumnFrom = tablePlant.column_find_index( stringColumnName );                             assert( iColumnFrom != -1 );
   int iColumnTo = column_find_index( stringColumnName );                                          assert( iColumnTo != -1 );

   plant( tablePlant, iColumnFrom, iColumnTo, uFrom, uCount );
}

void table::plant( const table& tablePlant, unsigned uColumnFrom, unsigned uColumnTo, uint64_t uFrom, uint64_t uCount )
{                                                                                                  assert( uColumnFrom < tablePlant.get_column_count() ); assert( uColumnTo < get_column_count() );
   // ## Loop from "from" row and number of count rows extracting selected value
   //    and set to current table in column
   for( uint64_t uRow = uFrom, uToRow = uFrom + uCount; uRow < uToRow; uRow++ )
   {
      auto value_ = tablePlant.cell_get_variant_view( uRow - uFrom, uColumnFrom );
      cell_set( uRow, uColumnTo, value_ );
   }
}

void table::plant( unsigned uColumn, const gd::variant_view& variantviewValue, uint64_t uFrom, uint64_t uCount )
{                                                                                                  assert( uColumn < get_column_count() );
   for( uint64_t uRow = uFrom, uToRow = uFrom + uCount; uRow < uToRow; uRow++ )
   {
      cell_set( uRow, uColumn, variantviewValue );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Swap data in one row into another row and that rows data is inserted in first row
 * @code
   table_column_buffer tableM1( table_column_buffer::eTableFlagNull32, { { "double", 0, "mean"}, { "double", 0, "variance"}, { "double", 0, "error"}, { "uint64", 0, "count"}}, tag_prepare{});

   for( auto i = 0; i < 5; i++ )
   {
      tableM1.row_add( {i,i,i,i}, tag_convert{} );
   }

   std::cout << gd::table::debug::print( tableM1 ) << "\n";
   tableM1.swap( 0, 1 );
   tableM1.swap( 2, 3 );
   std::cout << gd::table::debug::print( tableM1 ) << "\n";
 * @endcode
 * @param uRow1 index to row that get data from the other row
 * @param uRow2 index to other row that get data from first row
*/
void table::swap( uint64_t uRow1, uint64_t uRow2 )
{                                                                                                  assert( uRow1 != uRow2 ); assert( uRow1 < get_row_count() ); assert( uRow2 < get_row_count() );
   const unsigned u128Length = (sizeof(uint64_t) + sizeof(uint64_t));          // 128 bit length in bytes

   unsigned uCount128 = m_uRowSize / (sizeof(uint64_t) + sizeof(uint64_t)); // number of 128 bit sections
   unsigned uTail = m_uRowSize % (sizeof(uint64_t) + sizeof(uint64_t));     // trailing 32 bit values (each row is allways 4 byte aligned)

   auto pRow1 = row_get( uRow1 );                                              // pointer to first row data
   auto pRow2 = row_get( uRow2 );                                              // pointer to second row data

   // ## swap data between rows
   for( unsigned u = 0; u < uCount128; u++ )
   {
#ifdef GD_X86X
      __m128i iSwapData = _mm_loadu_si128( reinterpret_cast<const __m128i* >( pRow1 ) );
      _mm_storeu_si128( reinterpret_cast<__m128i* >(pRow1), _mm_loadu_si128( reinterpret_cast<const __m128i* >( pRow2 ) ) );
      _mm_storeu_si128( reinterpret_cast<__m128i* >(pRow2), iSwapData );
#else
      int64_t iSwapData = *(int64_t*)pRow1;
      *(int64_t*)pRow1 = *(int64_t*)pRow2;
      *(int64_t*)pRow2 = iSwapData;
      int64_t iSwapData2 = *(int64_t*)(pRow1 + sizeof(int64_t));
      *(int64_t*)(pRow1 + sizeof(int64_t)) = *(int64_t*)(pRow2 + sizeof(int64_t));
      *(int64_t*)(pRow2 + sizeof(int64_t)) = iSwapData2;
#endif
      pRow1 += u128Length;
      pRow2 += u128Length;
   }
                                                                                                   assert( uTail < 16 ); assert( (uTail % 4) == 0 );
   while( uTail > 0 )
   {
      uint32_t uSwapData = *(uint32_t*)pRow1;
      *(uint32_t*)pRow1 = *(uint32_t*)pRow2;
      *(uint32_t*)pRow2 = uSwapData;

      pRow1 += sizeof( uint32_t );
      pRow2 += sizeof( uint32_t );
      uTail -= sizeof( uint32_t );
   }

   // ## if metadata for row then switch meta between rows
   if( is_rowmeta() == true )
   {
      pRow1 = row_get_meta( uRow1 );                                           // pointer to first meta data for row
      pRow2 = row_get_meta( uRow2 );                                           // pointer to second meta data for row
      uTail = m_uRowMetaSize;                                                                      assert( uTail < 16 ); assert( (uTail % 4) == 0 );
      while( uTail > 0 )
      {
         uint32_t uSwapData = *(uint32_t*)pRow1;                               // read 4 byte
         *(uint32_t*)pRow1 = *(uint32_t*)pRow2;                                // move 4 byte
         *(uint32_t*)pRow2 = uSwapData;                                        // store 4 bytes read

         pRow1 += sizeof( uint32_t );
         pRow2 += sizeof( uint32_t );
         uTail -= sizeof( uint32_t );
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief sort table rows based on selected column
 * @code
table_column_buffer tableM1( table_column_buffer::eTableFlagNull32, { { "int32", 0, "key"}, { "double", 0, "mean"}, { "double", 0, "variance"}, { "double", 0, "error"}, { "uint64", 0, "count"} }, tag_prepare{});
for( unsigned u = 0; u < 5; u++ ) tableM1.row_add( {u,u,u,u,u}, tag_convert{} );
tableM1.sort( "mean", false, tag_sort_selection{} );
auto stringTable = to_string( tableM1, tag_io_json{});
tableM1.sort( "mean", true, tag_sort_selection{} );
stringTable = to_string( tableM1, tag_io_json{});
 * @endcode
 * @param uColumn index for column to sort on
 * @param bAscending sort order
 * @param uFrom from what row to sort
 * @param uCount number of rows to sort
*/
void table::sort( unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_selection )
{                                                                                                  assert( uColumn < get_column_count() ); assert( uFrom < get_row_count() ); assert( (uFrom + uFrom) <= get_row_count() );
   if( bAscending == true )
   {
      for( uint64_t uRow = uFrom, uRowEnd = uFrom + uCount - 1; uRow <= uRowEnd; uRow++ )
      {
         uint64_t uRowMinimum = uRow;
         auto v1_ = cell_get_variant_view( uRow, uColumn );                                        assert( v1_.is_null() == false );
         for( auto u = uRow + 1; u <= uRowEnd; u++ )
         {
            auto v2_ = cell_get_variant_view( u, uColumn );                                        assert( v2_.is_null() == false );
            if( v2_ < v1_ )                                                    // if value in active row is less then take this to move to value 1
            { 
               v1_ = v2_;
               uRowMinimum = u;  
            }
         }

         if( uRowMinimum != uRow )
         {
            swap( uRowMinimum, uRow );
         }
      }// for( uint64_t uRow = uFrom, uRowEnd = uFrom + uCount - 1; uRow <= uRowEnd; uRow++ )
   }
   else
   {                                                                           // sort in descending order, same as acending just that sorting is done from end to begining
      uint64_t uRow = uFrom + uCount;
      while( uRow != (uFrom + 1) ) 
      {
         uRow--;
         uint64_t uRowMinimum = uRow;
         auto v1_ = cell_get_variant_view( uRow, uColumn );                                        assert( v1_.is_null() == false );
         auto u = uRow;
         while( u > uFrom )
         {
            u--;
            auto v2_ = cell_get_variant_view( u, uColumn );                                        assert( v2_.is_null() == false );
            if( v2_ < v1_ ) 
            { 
               v1_ = v2_;
               uRowMinimum = u;  
            }
         }

         if( uRowMinimum != uRow )
         {
            swap( uRowMinimum, uRow );
         }
      }// while( uRow != (uFrom + 1) ) 
   }
}

/** ---------------------------------------------------------------------------
 * @brief sort table rows based on value in selected column and bubble sort is the sorting method
 * @code
table_column_buffer table( table_column_buffer::eTableFlagNull32, { { "int32", 0, "key"}, { "double", 0, "value"}, tag_prepare{});
for( unsigned u = 0; u < 1000; u++ ) table.row_add( {u,u}, tag_convert{} );
table.sort( "key", false, tag_sort_bubble{} );
auto stringTable = to_string( table, tag_io_json{});
table.sort( 1, true, tag_sort_bubble{} );
stringTable = to_string( table, tag_io_json{});
 * @endcode
 * @param uColumn index for column to sort on
 * @param bAscending sort order, true == ascending, false descending
 * @param uFrom from what row to sort
 * @param uCount number of rows to sort
*/
void table::sort( unsigned uColumn, bool bAscending, uint64_t uFrom, uint64_t uCount, tag_sort_bubble )
{
   bool bSwap;
   if( bAscending == true )
   {
      for( uint64_t uRow = uFrom, uRowEnd = uFrom + uCount; uRow < (uRowEnd - 1); uRow++ )
      {
         bSwap = false;
         for( auto u = uFrom; u < (uRowEnd - uRow - 1); u++ )
         {
            auto v1_ = cell_get_variant_view( u, uColumn );                                        assert( v1_.is_null() == false );
            auto v2_ = cell_get_variant_view( u + 1, uColumn );                                    assert( v2_.is_null() == false );
            if( v2_ < v1_ )
            {
               swap( u + 1, u );
               bSwap = true;
            }
         }

         if( bSwap == false ) return;
      }// for( uint64_t uRow = uFrom, uRowEnd = uFrom + uCount; uRow < (uRowEnd - 1); uRow++ )
   }
   else
   {
      
      //for( uint64_t uRow = uFrom, uRowEnd = uFrom + uCount; uRow < (uRowEnd - 1); uRow++ )
      uint64_t uRowEnd = uFrom + uCount;
      uint64_t uRow = uRowEnd;
      while( uRow > uFrom )
      {
         uRow--;
         bSwap = false;
         //for( auto u = uRow; u > uFrom; u-- )
         auto u = uRowEnd;
         auto uSortStop = uFrom + (uRowEnd - uRow);
         while( u > uSortStop)
         {
            u--;
            auto v1_ = cell_get_variant_view( u, uColumn );                                        assert( v1_.is_null() == false );
            auto v2_ = cell_get_variant_view( u - 1, uColumn );                                    assert( v2_.is_null() == false );
            if( v2_ < v1_ )
            {
               swap( u - 1, u );
               bSwap = true;
            }
         }

         if( bSwap == false ) return;
      }// for( uint64_t uRow = uFrom, uRowEnd = uFrom + uCount; uRow < (uRowEnd - 1); uRow++ )
   }
}

/// split will create a new table with the section range has
table table::split( range rangeSplit ) const
{
   table table( m_pcolumns, this, rangeSplit.r1(), rangeSplit.height() );      // create table with selected rows from range

   return table;
}

/** ---------------------------------------------------------------------------
 * @brief split table into multiple tables where each table gets uRowCount slice of current table
 * @param uRowCount number of rows new tables gets from this table
 * @param vectorSplit vector where new tables are placed
*/
void table::split( uint64_t uRowCount, std::vector<table>& vectorSplit ) const
{
   uint64_t uRow = 0;
   do
   {
      table table( m_pcolumns, this, uRow, uRowCount );                     // create table with selected rows
      vectorSplit.emplace_back( std::move( table ) );

      uRow += uRowCount;
   }
   while( uRow < get_row_count() );
}


/** ---------------------------------------------------------------------------
 * @brief Erase rows from table
 * 
 * *Sample to show how two rows are removed*
 * @verbatim

 OOOOOOOOOOOOOOOOOOOO
 XXXXXXXXXXXXXXXXXXXX < erase data
 XXXXXXXXXXXXXXXXXXXX < erase data
 OOOOOOOOOOOOOOOOOOOO
 OOOOOOOOOOOOOOOOOOOO
 OO
 XX < erase meta
 XX < erase meta
 OO
 OO

 * @endverbatim
 * @param uFrom Start row from where rows are removed
 * @param uCount number of rows that is removed
*/
void table::erase( uint64_t uFrom, uint64_t uCount )
{                                                                                                  assert( (uFrom + uCount) <= get_row_count() ); assert( uFrom < get_row_count() );
   uint64_t uRowCount = get_row_count();           // number of rows in table
   uint64_t uMetaSize = size_row_meta();

   uint64_t uEraseDataSize = uCount * m_uRowSize;  // data size to be erased
   uint64_t uEraseMetaSize = uCount * uMetaSize;   // meta size to be erased

   // ## move meta data if meta is set
   if( m_puMetaData != nullptr )
   {
      // ## erase arguments objects first
      if( is_rowarguments() == true )
      {
         for( uint64_t u = uFrom; u < (uFrom + uCount); u++ )
         {
            gd::argument::shared::arguments* pargumentsRow = (gd::argument::shared::arguments*)row_get_arguments_meta(u);
            if( pargumentsRow != nullptr ) pargumentsRow->release(); // release reference to arguments object
         }
      }

      // ### calculate position after meta block that is erased
      uint8_t* puStartOfMoveBlock = m_puMetaData + ((uFrom + uCount) * uMetaSize);// start of block that will be moved
      uint64_t uMoveSize = (m_puMetaData + (uRowCount * uMetaSize) - puStartOfMoveBlock);

      /// ### calculate to where memmory is moved and move it
      uint8_t* puMoveTo = puStartOfMoveBlock - uEraseMetaSize;
      memcpy( puMoveTo, puStartOfMoveBlock, uMoveSize );
   }

   // ## move
   uint8_t* puStartOfMoveBlock = m_puData + ((uFrom + uCount) * m_uRowSize);// start of block that will be moved
   uint64_t uMoveSize = (m_puData + (uRowCount * m_uRowSize) - puStartOfMoveBlock);

   /// ### calculate to where memmory is moved and move it
   uint8_t* puMoveTo = puStartOfMoveBlock - uEraseDataSize;
   memcpy( puMoveTo, puStartOfMoveBlock, uMoveSize );

   m_uRowCount -= uCount;
}

/** ---------------------------------------------------------------------------
 * Erases multiple rows from the table column buffer by their indices.
 * 
 * This method handles duplicate indices and ensures rows are erased in descending order
 * to prevent index invalidation problems that would occur with ascending order deletion.
 * 
 * @param puRowIndex Pointer to an array of row indices to be erased
 * @param uCount Number of indices in the puRowIndex array
 * @return The actual number of rows removed (may be less than uCount if there are duplicates or out-of-bounds indices)
 */
uint64_t table::erase(const uint64_t* puRowIndex, uint64_t uCount)
{                                                                                                  assert( uCount > 0 ); assert( m_puData );
   std::vector<uint64_t> vectorSorted(puRowIndex, puRowIndex + uCount);

   // ## Remove duplicates
   //    Sort the vector in descending order to ensure that we can safely erase rows without invalidating indices.
   std::sort(vectorSorted.begin(), vectorSorted.end(), [](uint64_t uLeft, uint64_t uRight){ return uLeft > uRight; });

   vectorSorted.erase( std::unique(vectorSorted.begin(), vectorSorted.end()), vectorSorted.end());

   size_t uSize = size();
   size_t uRemoved = 0;

   // ## Process from highest to lowest index (prevents invalidation problems)
   for(const uint64_t uIndex : vectorSorted)
   {
      if(uIndex < uSize)
      {
         erase(uIndex, 1);                                                     // Remove the row
         uRemoved++;
      }
   }
                                                                                                   assert(uRemoved <= uCount);
   return uRemoved;                                                                                
}

/** ---------------------------------------------------------------------------
 * Erases multiple rows from the table column buffer by their indices.
 *
 * Makes sure that the indices are valid and sorted in descending order.
 * 
 * @param puRowIndex Pointer to an array of row indices to be erased
 * @param uCount Number of indices in the puRowIndex array
 */
void table::erase(const uint64_t* puRowIndex, uint64_t uCount, tag_raw )
{                                                                                                  assert( uCount > 0 ); assert( m_puData );
#ifndef NDEBUG
   // ## Check that puRowIndex is not null and uCount is greater than 0
   assert( puRowIndex != nullptr );
   assert(uCount > 0);
   // check that all indices are within bounds and sorted from highest to lowest
   for( uint64_t u = 0; u < uCount; u++ ) assert( puRowIndex[u] < size() ); // check that index is within bounds

   // check that indices are sorted from highest to lowest
   for( uint64_t u = 1; u < uCount; u++ ) assert( puRowIndex[u] <= puRowIndex[u - 1] ); // check that indices are sorted from highest to lowest
#endif

   for( uint64_t u = 0; u < uCount; u++ )
   {
      erase(puRowIndex[u], 1);
   }
}



/** ---------------------------------------------------------------------------
 * @brief Allocate columns object on heap for table
 * @return detail::columns* pointer to allocated  columns object.
*/
detail::columns* table::new_columns_s()
{
   detail::columns* pcolumns = new detail::columns{};
   pcolumns->add_reference();

   return pcolumns;
}

/** ---------------------------------------------------------------------------
 * @brief create new columns object and add columns from selected columns object
 * @param pcolumnsFrom columns object columns are added from
 * @param puColumn pointer to unsigned buffer with column indexes
 * @param uSize number of column indexes in buffer
 * @return pointer to created columns object
 */
detail::columns* table::new_columns_s(const detail::columns* pcolumnsFrom, unsigned* puColumn, unsigned uSize)
{
   detail::columns* pcolumns = new detail::columns{};
   pcolumns->add_reference();

   for(unsigned u = 0; u < uSize; u++)
   {
      unsigned uColumn = puColumn[u];                                                              assert( uColumn < pcolumnsFrom->size() );
      const detail::column* pcolumnAdd = pcolumnsFrom->get( uColumn );
      pcolumns->add( *pcolumnAdd );
   }

   return pcolumns;
}

/** ---------------------------------------------------------------------------
 * @brief Convert names to column indexes in table
 * @param tablecolumnbuffer table to get column indexes from
 * @param vectorName names to match for column indexes are returned for.
 * @return std::vector<int32_t> indexes to columns in table
*/
std::vector<int32_t> table::column_get_index_s( const table& tablecolumnbuffer, const std::vector<std::string>& vectorName )
{
   std::vector<int32_t> vectorColumnIndex;
   for( const auto& it : vectorName )
   {
      vectorColumnIndex.push_back( tablecolumnbuffer.column_get_index( it ) );
   }
   return vectorColumnIndex;
}

/** ---------------------------------------------------------------------------
 * @brief Match columns based on names between two tables, index for matched columns are returned in vector as pair's objects
 * @param t1_ table one to match against table two
 * @param t2_ table two to match against table one
 * @return std::vector< std::pair< unsigned, unsigned > > vector with matched column indexes
*/
std::vector< std::pair< unsigned, unsigned > > table::column_match_s( const table& t1_, const table& t2_, tag_name )
{
   std::vector< std::pair< unsigned, unsigned > > vectorMatch;
   for( unsigned u = 0, uMax = t1_.get_column_count(); u != uMax; u++ )
   {
      auto name_ = t1_.column_get_name( u );
      int iFindColumn = t2_.column_find_index( name_ );
      if( iFindColumn != -1 ) { vectorMatch.push_back( { u, (unsigned)iFindColumn } ); }
   }

   return vectorMatch;
}

/** ---------------------------------------------------------------------------
 * @brief Match columns based on alias between two tables, index for matched columns are returned in vector as pair's objects
 * @param t1_ table one to match against table two
 * @param t2_ table two to match against table one
 * @return std::vector< std::pair< unsigned, unsigned > > vector with matched column indexes
*/
std::vector< std::pair< unsigned, unsigned > > table::column_match_s( const table& t1_, const table& t2_, tag_alias )
{
   std::vector< std::pair< unsigned, unsigned > > vectorMatch;
   for( unsigned u = 0, uMax = t1_.get_column_count(); u != uMax; u++ )
   {
      auto name_ = t1_.column_get_name( u );
      int iFindColumn = t2_.column_find_index( name_, tag_alias{});
      if( iFindColumn != -1 ) { vectorMatch.push_back( { u, (unsigned)iFindColumn } ); }
   }

   return vectorMatch;
}

/** ---------------------------------------------------------------------------
 * @brief Match string values in vectors , index for matched columns are returned in vector as pair's objects
 * @param v1_ vector one to match against vector two
 * @param v2_ vector two to match against vector one
 * @return std::vector< std::pair< unsigned, unsigned > > vector with matched string indexes
*/
std::vector<std::pair<unsigned, unsigned>> table::column_match_s( const std::vector<std::string_view>& v1_, const std::vector<std::string_view>& v2_ )
{
   std::vector< std::pair< unsigned, unsigned > > vectorMatch;
   for( unsigned u1 = 0; u1 != (unsigned)v1_.size(); u1++ )
   {
      auto name_ = v1_[u1]; // name to look for in vector 2
      int iFindIndex = -1; // index in vector two if found
      for( unsigned u2 = 0; iFindIndex == -1 && u2 != ( unsigned )v2_.size(); u2++ )
      {
         if( name_ == v2_[u2] ) iFindIndex = (int)u2;
      }

      if( iFindIndex != -1 ) { vectorMatch.push_back( { u1, (unsigned)iFindIndex } ); }
   }

   return vectorMatch;
}

/** ---------------------------------------------------------------------------
 * @brief Match columns based on names between two tables, index for matched columns are returned in vector as pair's objects
 * @param t1_ table one to match against table two
 * @param t2_ table two to match against table one
 * @param pvector1 gets column index from t1 table that matches column in t2
 * @param pvector2 gets column index from t2 table that matches column in t1
*/
void table::column_match_s( const table& t1_, const table& t2_, std::vector<unsigned>* pvector1, std::vector<unsigned>* pvector2, tag_name )
{
   for( unsigned u = 0, uMax = t1_.get_column_count(); u != uMax; u++ )
   {
      auto name_ = t1_.column_get_name( u );
      int iFindColumn = t2_.column_find_index( name_ );
      if( iFindColumn != -1 ) 
      {
         if( pvector1 != nullptr ) pvector1->push_back( u );
         if( pvector2 != nullptr ) pvector2->push_back( (unsigned)iFindColumn );
      }
   }
}





_GD_TABLE_ARGUMENTS_END

_GD_TABLE_ARGUMENTS_BEGIN

/// print table rows and specifies max number of rows to print
std::string debug::print( const table& table, uint64_t uCount )
{                                                                                                  assert( table.get_column_count() > 0 );
   std::string stringPrint;
   if( uCount > (unsigned)table.get_row_count() ) uCount = (unsigned)table.get_row_count();

   for( unsigned uRow = 0; uRow < uCount; uRow++ )
   {
      auto v_ = table.cell_get_variant_view( uRow, (unsigned)0 );
      if( v_.is_null() == false ) stringPrint += v_.as_string();
      else                        stringPrint += "null";

      for( unsigned uColumn = 1; uColumn < table.get_column_count(); uColumn++ )
      {
         stringPrint += ", ";
         v_ = table.cell_get_variant_view( uRow, uColumn );
         if( v_.is_null() == false ) stringPrint += v_.as_string();
         else                        stringPrint += "null";
      }
      stringPrint += "\n";
   }

   return stringPrint;
}

/// print all rows
std::string debug::print( const table& table ) { return debug::print( table, table.get_row_count() ); }

/// print column information for table
std::string debug::print( const table& table, tag_columns )
{
   std::string stringPrint;
   uint32_t uIndex = 0;
   for( auto it = table.column_begin(), itEnd = table.column_end(); it != itEnd; it++ )
   {
      if( stringPrint.empty() == false ) stringPrint += " ";

      stringPrint += "[";
      stringPrint += "("; stringPrint += std::to_string( uIndex ); stringPrint += ") ";
      stringPrint += table.column_get_name( uIndex );
      stringPrint += ",";
      stringPrint += table.column_get_alias( uIndex );
      stringPrint += ",";
      stringPrint += gd::types::type_name_g( it->ctype() );
      if( it->is_reference() ) { stringPrint += "("; stringPrint += "reference"; stringPrint += ")"; }
      stringPrint += ",";
      stringPrint += std::to_string( it->primitive_size() );
      stringPrint += "]";
      uIndex++;
   }

   return stringPrint;
}

/// print column information in table
std::string debug::print( const table* ptable, tag_columns ) { return print( *ptable, tag_columns{}); }

/// print column information in table
std::string debug::print_column( const table* ptable ) { return print( *ptable, tag_columns{}); }


/// print selected row
std::string debug::print_row( const table& table, uint64_t uRow )
{
   std::string stringPrint;

   if( uRow < table.get_row_count() )
   {
      auto vector_ = table.row_get_variant_view( uRow );
      for( auto it : vector_ )
      {
         if( stringPrint.empty() == false ) stringPrint += ", ";
         if( it.is_null() != true ) stringPrint += it.as_string();
         else                       stringPrint += "null";
      }
      stringPrint += "\n";

   }
   else
   {
      stringPrint += "Max row is:";
      stringPrint += std::to_string( table.get_row_count() );
      stringPrint += "\n";
   }

   return stringPrint;
}


_GD_TABLE_ARGUMENTS_END

#endif // GD_COMPILER_HAS_CPP20_SUPPORT
