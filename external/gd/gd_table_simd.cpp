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
{
   assert(gd::types::validate_number_type_g(uColumnType)); assert(uSize < 0x1000'0000);
   if(gd::types::is_primitive_g(uColumnType) == false) uSize = gd::types::value_size_g(uColumnType, uSize);
   return column_add({ uColumnType, uSize });
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
table_base& table_base::column_add(unsigned uColumnType, unsigned uSize, std::string_view stringName)
{                                                                                                  assert(uColumnType != 0); assert(gd::types::validate_number_type_g(uColumnType)); assert(uSize < 0x1000'0000);
   column columnAdd;

   columnAdd.type(uColumnType);
   columnAdd.ctype(uColumnType);
   columnAdd.primitive_size(gd::types::value_size_g(uColumnType));

   if(gd::types::is_primitive_g(uColumnType) == false && gd::types::is_reference_g(uColumnType) == false)
   {
      uSize = gd::types::value_size_g(uColumnType, uSize);
   }

   columnAdd.size(uSize);

   if(stringName.empty() == false)
   {                                                                                               assert(m_namesColumn.empty() == true || column_find_index(stringName) == -1); // check if field name exists
      // ## adds name to internal buffer and returns index in that buffer that is set 
      auto uNameIndex = m_namesColumn.add(stringName);
      columnAdd.name(uNameIndex);
   }

   m_vectorColumn.push_back(columnAdd);

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
{
   assert(m_puData == nullptr);
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
{
   assert(m_puData == nullptr);
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
 * @brief find index to column for column name
 * @param stringName column name column index is returned for
 * @return int index to column for column name if found, -1 if not found
*/
int table_base::column_find_index(const std::string_view& stringName) const noexcept
{                                                                                                  assert(m_namesColumn.empty() == false);
   for(auto it = std::begin(m_vectorColumn), itEnd = std::end(m_vectorColumn); it != itEnd; it++)
   {
      if(stringName == it->name(m_namesColumn)) return (int)std::distance(std::begin(m_vectorColumn), it);
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
   for(auto it = std::begin(m_vectorColumn), itEnd = std::end(m_vectorColumn); it != itEnd; it++)
   {
      if(gd::ascii::strcmp(it->name(m_namesColumn).data(), stringWildcard.data(), gd::utf8::tag_wildcard{}) == 0)
      {
         return (int)std::distance(std::begin(m_vectorColumn), it);
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



std::pair<bool, std::string> table_base::prepare( unsigned uValueSize, unsigned uStride )
{                                                                                                  assert(m_vectorColumn.empty() == false && "Table must have at least one column");
                                                                                                   assert(m_puData == nullptr && "Table already prepared");
                                                                                                   assert((uValueSize == 4 || uValueSize == 8) && "Value size must be 4 or 8 bytes");
                                                                                                   assert((uStride == 4 || uStride == 8 || uStride == 16) && "Stride must be 4, 8, or 16");
   // ## calculate size for each row
   unsigned uRowSize = 0u; // 
   unsigned uColumnCount = (unsigned)m_vectorColumn.size();

   uRowSize = uValueSize * uStride * uColumnCount;                            // calculate size for each row based on value size and count

   m_uRowSize = uRowSize;                                                     // final row sizes (not that each row containes a stride of columns)


   // ## calculate needed meta data size for each row
   unsigned uMetaDataSize = size_row_meta();

   m_uRowMetaSize = uMetaDataSize;

   uint64_t uTotalTableSize = (uRowSize + uMetaDataSize) * m_uReservedBlockCount;// calculate size storing table data

   m_puData = new uint8_t[uTotalTableSize];
#ifdef _DEBUG
   memset(m_puData, 0, uTotalTableSize);                                     // set data to 0 in debug mode
#endif // _DEBUG

   if(uMetaDataSize > 0)
   {
      m_puMetaData = m_puData + (m_uReservedBlockCount * uRowSize);              // set pointer to meta data section
      memset(m_puMetaData, 0, m_uReservedBlockCount * uMetaDataSize);
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief adds more memory storing row/rows to table
 * @param uCount number of rows to add
*/
void table_base::row_reserve_add(uint64_t uCount)
{
   uCount += m_uReservedBlockCount;

   // ## calculate size needed to store added row count and allocate memory
   uint64_t uTotalTableSize = size_reserved_total();                           // total table memory block size for table
   uint64_t uTotalMetaSize = size_meta_total();                                // meta block size part

   uint64_t uTotalTableSizeCopyTo = size_reserved_total(uCount);               // new block size to store more rows
   uint64_t uTotalMetaSizeCopyTo = size_meta_total(uCount);                    // new meta block size

   uint64_t uCopyRowSize = uTotalTableSize - uTotalMetaSize;

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

   m_uReservedBlockCount = uCount;
}



_GD_TABLE_SIMD_END
