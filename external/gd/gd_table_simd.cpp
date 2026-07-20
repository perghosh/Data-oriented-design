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



std::pair<bool, std::string> table_base::prepare( unsigned uValueSize, unsigned uPackCount )
{                                                                                                  assert(m_vectorColumn.empty() == false && "Table must have at least one column");
                                                                                                   assert(m_puData == nullptr && "Table already prepared");
                                                                                                   assert((uValueSize == 4 || uValueSize == 8) && "Value size must be 4 or 8 bytes");
                                                                                                   assert((uPackCount == 4 || uPackCount == 8 || uPackCount == 16) && "Pack count must be 4, 8, or 16");
   // ## calculate size for each row
   unsigned uRowSize = 0u; // 
   unsigned uColumnCount = (unsigned)m_vectorColumn.size();

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
                                                                                                   assert(uTotalTableSizeCopyTo % 4 == 0 && "Total table size must be multiple of 4");
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
 * @brief Set cell value in table
 * @param uRow row index for cell
 * @param uColumn column index for cell
 * @param uValue value set to cell
 */
void table_base::cell_set_value(uint64_t uRow, unsigned uColumn, uint32_t uValue)
{
                                                                                                   assert(size_value() == 4);
#ifndef NDEBUG
   if(uRow >= m_uRowReservedPackCount || uColumn >= m_vectorColumn.size()) { assert(false); }
#endif // !NDEBUG

                                                                                                   assert(uRow < m_uRowReservedPackCount); assert(uColumn < m_vectorColumn.size());
   auto& columnSet = m_vectorColumn[uColumn];                                                      assert(columnSet.position() < m_uRowSize);
   auto puRow = row_get(uRow);
   auto uRowOffset = offset(uRow, uColumn, tag_column{} );
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
   if( (uRow / m_uPackCount) >= m_uRowReservedPackCount || uColumn >= m_vectorColumn.size()) { assert(false); }
#endif // !NDEBUG
                                                                                                   assert((uRow / m_uPackCount) < m_uRowReservedPackCount); assert(uColumn < m_vectorColumn.size());
   auto& columnSet = m_vectorColumn[uColumn];                                                      assert(columnSet.position() < m_uRowSize);
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
{                                                                                                  assert(uColumn < m_vectorColumn.size());
   auto& columnSet = m_vectorColumn[uColumn];                                                      assert(columnSet.position() < m_uRowSize);
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
{                                                                                                  assert(uColumn < m_vectorColumn.size());
   auto& columnSet = m_vectorColumn[uColumn];                                                      assert(columnSet.position() < m_uRowSize);
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



_GD_TABLE_SIMD_END
