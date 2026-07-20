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
   /** =======================================================================
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
      column() { memset(static_cast<void*>(this), 0, sizeof(column)); }
      column(unsigned uCType) { memset(static_cast<void*>(this), 0, sizeof(column)); m_uCType = uCType; }
      column(unsigned uCType, unsigned uSize) : m_uCType(uCType), m_uType(uCType), m_uState(0), m_uPosition(0), m_uSize(uSize), m_uPrimitiveSize(gd::types::value_size_g(uCType)), m_uNameOffset(0) {}
      column(unsigned uCType, unsigned uType, unsigned uSize) : m_uCType(uCType), m_uType(uType), m_uState(0), m_uPosition(0), m_uSize(uSize), m_uPrimitiveSize(gd::types::value_size_g(uCType)), m_uNameOffset(0) {}
      column(const column* pcolumn) { memcpy(static_cast<void*>(this), pcolumn, sizeof(column)); }

      void state(unsigned uState) { m_uState = uState; }                     ///< Set column state flags
      [[nodiscard]] unsigned state() const noexcept { return m_uState; }       ///< Get column state
      void type(unsigned uType) { m_uType = uType; }                         ///< Set native type
      [[nodiscard]] unsigned type() const noexcept { return m_uType; }
      void ctype(unsigned uCType) { m_uCType = uCType; }                     ///< Set C type
      [[nodiscard]] unsigned ctype() const noexcept { return m_uCType; }       ///< Get C type
      /// extract the number type part from ctype
      [[nodiscard]] unsigned ctype_number() const noexcept { return m_uCType & 0x0000'00ff; }
      [[nodiscard]] unsigned ctype_group() const noexcept { return m_uCType & 0x0000'ff00; }
      void position(unsigned uPosition) { m_uPosition = uPosition; }
      [[nodiscard]] unsigned position() const noexcept { return m_uPosition; }
      void size(unsigned uSize) { m_uSize = uSize; }
      [[nodiscard]] unsigned size() const noexcept { return m_uSize; }
      void primitive_size(unsigned uSize) { m_uPrimitiveSize = uSize; }
      [[nodiscard]] unsigned primitive_size() const noexcept { return m_uPrimitiveSize; }
      [[nodiscard]] unsigned name() const { return m_uNameOffset; }
      [[nodiscard]] std::string_view name(const char* pbszBuffer) const {
         auto p = &pbszBuffer[m_uNameOffset];
         return m_uNameOffset > 0 ? std::string_view(p, (unsigned)*(uint16_t*)(p - sizeof(uint16_t))) : std::string_view(pbszNoName_g);
      }
      void name(unsigned iOffset) { m_uNameOffset = iOffset; }

      void data(uintptr_t uData) { m_uData = uData; }
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
      uintptr_t m_uData;   ///< custom data, use this to get some specific external logic
   };

// ## @API [tag: construct] [description: table construction, lots of constructors to simplify how to create new tables]
public:
   table_base() : m_uFlags(0), m_uRowSize(0), m_uRowCount(0), m_uPackCount(0), m_uRowGrowBy(0) {}
   table_base(unsigned uRowCount) : m_uFlags(0), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(uRowCount), m_uRowGrowBy(0) {}
   table_base(unsigned uRowCount, unsigned uFlags) : m_uFlags(uFlags), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(uRowCount), m_uRowGrowBy(0) { assert(m_uFlags < eTableFlagMAX); }
   table_base(unsigned uRowCount, unsigned uFlags, unsigned uGrowBy) : m_uFlags(uFlags), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(uRowCount), m_uRowGrowBy(uGrowBy) { assert(m_uFlags < eTableFlagMAX); }
   table_base(tag_null) : m_uFlags(eTableFlagNull64), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(0) { assert(m_uFlags < eTableFlagMAX); }
   table_base(tag_full_meta) : m_uFlags(eTableFlagNull64 | eTableFlagRowStatus), m_uRowSize(0), m_uRowCount(0), m_uRowReservedPackCount(0) { assert(m_uFlags < eTableFlagMAX); }

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
   unsigned get_column_count() const noexcept { return (unsigned)m_vectorColumn.size(); }
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
   uint64_t size_reserved_total(uint64_t uRowCount) const noexcept { return (m_uRowSize + size_row_meta()) * uRowCount; }

   bool is_null() const { return m_uFlags & (eTableFlagNull32 | eTableFlagNull64); }
   bool is_null32() const { return m_uFlags & eTableFlagNull32; }
   bool is_null64() const { return m_uFlags & eTableFlagNull64; }
   bool is_rowstatus() const { return m_uFlags & eTableFlagRowStatus; }
   bool is_duplicated_strings() const { return m_uFlags & eTableFlagDuplicateStrings; }
   bool is_rowmeta() const { return m_puMetaData != nullptr; }



   table_base& column_add(const column& columnToAdd) { m_vectorColumn.push_back(columnToAdd); return *this; }
   table_base& column_add(unsigned uColumnType, const std::string_view& stringName) { return column_add(uColumnType, 0, stringName); }
   table_base& column_add(unsigned uColumnType, unsigned uSize);
   table_base& column_add(unsigned uColumnType, unsigned uSize, std::string_view stringName);
   table_base& column_add(std::string_view stringType) { return column_add(column((unsigned)gd::types::type_g(stringType))); }
   table_base& column_add(std::string_view stringType, const std::string_view& stringName) { return column_add((unsigned)gd::types::type_g(stringType), 0, stringName); }
   table_base& column_add(std::string_view stringType, unsigned uSize) { return column_add((unsigned)gd::types::type_g(stringType), uSize); }
   table_base& column_add(std::string_view stringType, unsigned uSize, const std::string_view& stringName) { return column_add((unsigned)gd::types::type_g(stringType), uSize, stringName); }

   table_base& column_add(const std::initializer_list< std::pair< std::string_view, unsigned > >& listType, tag_type_name);
   table_base& column_add(const std::initializer_list< std::tuple< std::string_view, unsigned, std::string_view > >& listType, tag_type_name);
   table_base& column_add(const std::initializer_list< std::pair< std::string_view, std::string_view > >& listType, tag_type_name);

   /// @brief find column index for column name
   int column_find_index(const std::string_view& stringName) const noexcept;
   /// @brief find column index for column name with wildcard, wildcars like ? and * are supported
   int column_find_index(const std::string_view& stringWildcard, tag_wildcard) const noexcept;
   /// @brief get column index for column name, asserts if not found
   unsigned column_get_index(const std::string_view& stringName) const noexcept;
   /// @brief get column index for column name with wildcard, wildcars like ? and * are supported, asserts if not found
   unsigned column_get_index(const std::string_view& stringName, tag_wildcard) const noexcept;


   std::pair<bool, std::string> prepare( unsigned uValueSize, unsigned uStride );

   uint8_t* row_get( uint64_t uRow ) const noexcept {                                              assert(m_uPackCount == 4 || m_uPackCount == 8 && "Pack size must be 4 or 8");
      uint64_t uRowPack = uRow / m_uPackCount;
      return m_puData + uRowPack * m_uRowSize; 
   }

   void row_reserve_add(uint64_t uCount);
   void row_reserve_add() { row_reserve_add(1); }

   void cell_set(uint64_t uRow, unsigned uColumn, uint32_t uValue);
   void cell_set(uint64_t uRow, unsigned uColumn, uint64_t uValue);

protected:
   unsigned count_pack() const noexcept { return m_uPackCount; }
   unsigned size_value() const noexcept { return m_uValueSize; }
   unsigned size_pack() const noexcept { return m_uPackCount * size_value(); }
   unsigned offset(uint64_t uRow, unsigned uColumn, tag_column) const noexcept { return uColumn * size_pack() + (uRow % size_pack()) * size_value(); }


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
   std::vector<column> m_vectorColumn; ///< information about each column in table

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

   void row_add(uint64_t uCount);

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



   m_uRowCount = uRowCountNew;
}


_GD_TABLE_SIMD_END
