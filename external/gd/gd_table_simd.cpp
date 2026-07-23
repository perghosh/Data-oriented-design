// @FILE [tag: table, simd] [description: Base class for tables optimized for data transfer] [type: class] [name: table_base]

#include "gd_table_simd.h"

#include <variant>

#include "gd_utf8.h"
#include "gd_utf8_2.h"
#include "gd_variant.h"



_GD_TABLE_SIMD_BEGIN

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

   m_puData = new uint8_t[uTotalTableSize];
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
   uint8_t* puDataCopyTo = new uint8_t[uTotalTableSizeCopyTo];                // new buffer for table data (both data and meta data)

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

   delete[] m_puData;
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
