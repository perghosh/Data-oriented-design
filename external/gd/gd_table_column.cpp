#include "gd_table_column.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( _MSC_VER )
#endif

_GD_TABLE_DETAIL_BEGIN


void column::get(argument::column& column_) const
{
   column_.type( type() );
   column_.size( size() );
   column_.name( name() );
   column_.alias( alias() );
}




/** ---------------------------------------------------------------------------
 * @brief add column to table
 * @param uColumnType column type added
 * @param uSize size for column if (0 if primitive type and size for derived types)
 * @return reference to table
*/
columns& columns::add( unsigned uColumnType, unsigned uSize )
{ 
   if( gd::types::is_primitive_g( uColumnType ) == false ) uSize = gd::types::value_size_g( uColumnType, uSize );
   return add( column( uColumnType, uSize ) ); 
}


/** ---------------------------------------------------------------------------
 * @brief Adds column to table
 * @param uColumnType value type for column
 * @param uSize if size isn't a fixed type then this is the max size for value
 * @param stringName column name
 * @param stringAlias column alias
 * @return columns& reference to table
*/
columns& columns::add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias )
{                                                                                                  assert( gd::types::validate_number_type_g( uColumnType ) ); assert( uSize < 0x1000'0000 );
   column columnAdd;

   columnAdd.type( uColumnType );
   columnAdd.ctype( uColumnType );
   columnAdd.primitive_size( gd::types::value_size_g( uColumnType ) );

   if( gd::types::is_primitive_g( uColumnType ) == false && gd::types::is_reference_g( uColumnType ) == false )
   {
      uSize = gd::types::value_size_g( uColumnType, uSize );
   }

   columnAdd.size( uSize );
   columnAdd.name( stringName );
   columnAdd.alias( stringAlias );

   m_vectorColumn.push_back( std::move( columnAdd ) );
   
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add multiple column from information stored as tuple in vector.
 * @param vectorColumn columns to add
 * @param {unsigned} vectorColumn[][0] column type
 * @param {unsigned} vectorColumn[][1] column max size if needed
 * @param {unsigned} vectorColumn[][2] column name
 * @return columns& reference to table
*/
columns& columns::add( const std::vector<std::tuple<unsigned, unsigned, std::string_view>>& vectorColumn )
{
   for( const auto& it : vectorColumn )
   {
      add( std::get<0>( it ), std::get<1>( it ), std::get<2>( it ) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with pair values
~~~(.cpp)
// create tabl with one row and three columns
gd::table::dto tableVariable( 1 );   
tableVariable.add( { { "double", 0 }, { "double", 0 }, { "double", 0 }, { "int32", 0 } }, gd::table::tag_type_name{} );
tableVariable.prepare();
~~~
 * @param vectorType vector with pair items "<type_name, size>".
 * @param tag dispatcher to diff from other `add` methods.
 * @return reference to columns to nest methods.
*/
columns& columns::add( const std::vector<std::pair<std::string_view, unsigned>>& vectorType, tag_type_name )
{
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {
      add( it->first, it->second );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with tuple values
~~~(.cpp)
// create tabl with one row and three columns
gd::table::dto tableVariable( 1 );   
tableVariable.add( { { "string", 50, "FName"}, { "string", 50, "FName"}, { "string", 50, "FValue"} }, gd::table::tag_type_name{});
tableVariable.prepare();
~~~
 * @param vectorType vector with tuple items "<type_name, size, column_name>".
 * @param tag_type_name tag dispatcher to diff from other `add` methods.
 * @return reference to columns to nest methods.
*/
columns& columns::add( const std::vector<std::tuple<std::string_view, unsigned, std::string_view>>& vectorType, tag_type_name )
{
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {
      add( std::get<0>(*it), std::get<1>(*it), std::get<2>(*it) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add zero or more columns to table
 * Add column based om information found in vector with tuple values
~~~(.cpp)
// create table with one row and three columns
gd::table::dto tableVariable( 1 );   
tableVariable.add( { { "string", 50, "FName", "name"}, { "string", 50, "FLastname", "lastname"}, { "string", 50, "FValue", "value"} }, gd::table::tag_type_name{});
tableVariable.prepare();
~~~
 * @param vectorType vector with tuple items "<type_name, size, column_name>".
 * @param tag_type_name tag dispatcher to diff from other `add` methods.
 * @return reference to columns to nest methods.
*/
columns& columns::add( const std::vector<std::tuple<std::string_view, unsigned, std::string_view, std::string_view>>& vectorType, tag_type_name )
{
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {
      add( std::get<0>(*it), std::get<1>(*it), std::get<2>(*it), std::get<3>(*it) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add columns to table with none derived value types, no need for specify max value length
 * @param vectorType vector with pair items "<type_name, column_name>".
 * @return reference to columns to nest methods.
*/
columns& columns::add( const std::vector< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name )
{
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {                                                                                               
#ifndef NDEBUG
      // check type, adding column without size can't be done for derived types
      auto uType_d = gd::types::type_g( std::get<0>(*it) );                                        assert( (gd::types::is_primitive_g( uType_d ) == true) || (uType_d & gd::types::eTypeDetailReference) );
#endif // !NDEBUG
      add( std::get<0>(*it), 0, std::get<1>(*it) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief add columns to table with none derived value types, no need for specify max value length
@code
// create table with one row and three columns
gd::table::columns tableVariable( 10 );   
tableVariable.add( { { "int32", "x" }, { "int32", "y" } }, gd::table::tag_type_name{});
tableVariable.prepare();
@endcode
 * @param vectorType vector with pair items "<type_name, column_name>".
 * @return reference to columns to nest methods.
*/
columns& columns::add( const std::initializer_list< std::pair< std::string_view, std::string_view > >& listType, tag_type_name )
{
   for( auto it = std::begin( listType ), itEnd = std::end( listType ); it != itEnd; it++ )
   {                                                                                               
#ifndef NDEBUG
      // check type, adding column without size can't be done for derived types
      auto uType_d = gd::types::type_g( std::get<0>(*it) );                                        assert( (gd::types::is_primitive_g( uType_d ) == true) || (uType_d & gd::types::eTypeDetailReference) );
#endif // !NDEBUG
      add( std::get<0>(*it), 0, std::get<1>(*it) );
   }

   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief Add column to table
 * Adds columns to value from vector with type constants prepared, no conversion from type name 
 * to type constant is done so this is a bit faster
 * @param vectorType vector with pair constants, first is column type, second is buffer size for derived types
 * @return reference to columns to nest methods.
*/
columns& columns::add( const std::vector<std::pair<unsigned, unsigned>>& vectorType, tag_type_constant )
{
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd; it++ )
   {                                                                                               assert( gd::types::validate_number_type_g( it->first ) == true );
      add( it->first, it->second );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add columns and used information from another table
 * @param table_ table that column information is found
 * @return reference to columns to nest methods.
*/
columns& columns::add( const columns* p_ )
{                                                                                                  assert( p_ != nullptr );
   for( auto it = p_->begin(), itEnd = p_->end(); it != itEnd; it++ )
   {
      column columnAdd( *it ); // copies column memory but we need to fix offset positions for name and alias if they are set

      m_vectorColumn.push_back( std::move( columnAdd ) );
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief find index to column for column name
 * @param stringName column name column index is returned for
 * @return int index to column for column name if found, -1 if not found
*/
int columns::find_index( const std::string_view& stringName ) const noexcept
{
   for( auto it = std::begin( m_vectorColumn ), itEnd = std::end( m_vectorColumn ); it != itEnd; it++ )
   {
      if( stringName == it->name() ) return (int)std::distance( std::begin( m_vectorColumn ), it );
   }
   return -1;
}



_GD_TABLE_DETAIL_END

