#include "gd_window_line.h"

_GD_PARSE_WINDOW_BEGIN


void line::create() 
{                                                                                                  assert( m_uSize >= 0x80 ); // minimum size is 128 bytes
   if( m_uCapacity == 0 ) m_uCapacity = m_uSize + (m_uSize >> 1);              // 50% extra space if not specified
                                                                                                   assert( m_uCapacity > m_uSize ); // capacity must be larger than size
   m_puBuffer = new uint8_t[m_uCapacity]; 
   m_puLookAheadBuffer = m_puBuffer + m_uSize;                                 // set look ahead buffer to end of line
}

uint64_t line::write(const uint8_t* puData, uint64_t uSize)
{                                                                                                 assert( uSize != 0 );

   uint8_t* puBuffer = m_puBuffer;

   // ## swap ending into start of buffer if needed
   if( m_uLast > m_uSize )
   {
      uint64_t uSwapSize = m_uLast - m_uSize;                                  // Calculate swap size
      std::memmove(puBuffer, puBuffer + m_uSize, uSwapSize);
      puBuffer += uSwapSize;                                                   // Move buffer pointer to start empty space
      m_uLast = uSwapSize;
   }
   else { puBuffer += m_uLast; }                                               // Move buffer pointer to start empty space

   uint64_t uAvailable = available();                                          // Calculate available space
   uint64_t uToWrite = std::min(uSize, uAvailable);                            // Don't write more than available

   if(uToWrite == 0) return 0;

   std::memcpy( puBuffer, puData, uToWrite );
   m_uLast += uToWrite;                                                                            assert(m_uLast <= m_uCapacity); // last position must be smaller than capacity

   return uToWrite;
}

void line::rotate()
{
   if( m_uLast > m_uSize )
   {
      uint64_t uSwapSize = m_uLast - m_uSize;                                  // Calculate swap size
      std::memmove(m_puBuffer, m_puBuffer + m_uSize, uSwapSize);
      m_uLast = uSwapSize;
   }
   else
   {
      m_uLast = 0;                                                             // nothing to rotate
   }
}


_GD_PARSE_WINDOW_END