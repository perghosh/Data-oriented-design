// @FILE [tag: table, simd] [description: Base class for tables optimized for data transfer] [type: class] [name: table_base]

#include "gd_table_simd.h"


_GD_TABLE_SIMD_BEGIN




std::pair<bool, std::string> table_base::prepare( unsigned uValueSize, unsigned uStride )
{                                                                                                  assert(m_vectorColumn.empty() == false && "Table must have at least one column");
                                                                                                   assert(m_puData == nullptr && "Table already prepared");
                                                                                                   assert((uValueSize == 4 || uValueSize == 8) && "Value size must be 4 or 8 bytes");
                                                                                                   assert((uStride == 4 || uStride == 8 || uStride == 16) && "Stride must be 4, 8, or 16");
                                                                                                   assert(uRowCount > 0 && "Row count must be greater than 0");
   // ## prepare total size
   uint64_t u

   assert(m_vectorColumn.empty() == false); assert(m_puData == nullptr && "Table have been prepared");
   // ## calculate size for each row
   unsigned uRowSize = 0; // 
   unsigned uColumnCount = (unsigned)m_vectorColumn.size();

   uRowSize = uValueSize * uStride * uCount;                                  // calculate size for each row based on value size and count

   m_uRowSize = uRowSize;                                                     // final row sizes (not that each row containes a stride of columns)


   // ## calculate needed meta data size for each row
   unsigned uMetaDataSize = size_row_meta();

   m_uRowMetaSize = uMetaDataSize;

   uint64_t uTotalTableSize = (uRowSize + uMetaDataSize) * m_uReservedRowCount;// calculate size storing table data

   m_puData = new uint8_t[uTotalTableSize];
#ifdef _DEBUG
   memset(m_puData, 0, uTotalTableSize);                                     // set data to 0 in debug mode
#endif // _DEBUG

   if(uMetaDataSize > 0)
   {
      m_puMetaData = m_puData + (m_uReservedRowCount * uRowSize);              // set pointer to meta data section
      memset(m_puMetaData, 0, m_uReservedRowCount * uMetaDataSize);
   }

   return { true, "" };
}


_GD_TABLE_SIMD_END
