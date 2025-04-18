#include "gd_parse_window_line.h"

_GD_PARSE_WINDOW_BEGIN


/** ---------------------------------------------------------------------------
 * \brief Creates and initializes the buffer for the line.
 *
 * This method allocates memory for the main buffer based on the specified size and capacity.
 * If capacity is not specified (equal to 0), it's calculated as 150% of the size.
 * It also sets up the look-ahead buffer to point to the position after the main buffer.
 *
 * \pre m_uSize >= 0x80 (minimum size is 128 bytes)
 * \pre m_uCapacity > m_uSize after initialization
 *
 * \post m_puBuffer points to a newly allocated buffer of size m_uCapacity
 *
 * \code
 * line l(256);
 * l.create(); // Allocates buffer with 256 bytes size and appropriate capacity
 * \endcode
 */
void line::create() 
{                                                                                                  assert( m_uSize >= 0x80 ); // minimum size is 128 bytes
   if( m_uCapacity == 0 ) m_uCapacity = m_uSize + (m_uSize >> 1);              // 50% extra space if not specified
                                                                                                   assert( m_uCapacity > m_uSize ); // capacity must be larger than size
   m_puBuffer = new uint8_t[m_uCapacity]; 
}

/** ---------------------------------------------------------------------------
 * \brief Writes data to the buffer.
 *
 * This method writes data to the buffer at the current last position. If the last position
 * exceeds the buffer size, it rotates the excess data to the beginning of the buffer before
 * writing new data. It will not write more data than the available space in the buffer.
 *
 * \param puData Pointer to the data to write
 * \param uSize Size of the data to write in bytes
 *
 * \return The actual number of bytes written to the buffer
 *
 * \pre uSize != 0 (non-zero size data to write)
 * \post m_uLast <= m_uCapacity (last position within capacity)
 *
 * \code
 * line l(256);
 * l.create();
 * uint8_t data[64] = {...};
 * uint64_t bytesWritten = l.write(data, 64);
 * \endcode
 */
uint64_t line::write(const uint8_t* puData, uint64_t uSize)
{                                                                                                  assert( uSize != 0 );

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

/** ---------------------------------------------------------------------------
 * \brief Rotates the buffer by moving excess data from end to beginning.
 *
 * If the last position (m_uLast) exceeds the buffer size (m_uSize), this method
 * moves the excess data (data between m_uSize and m_uLast) to the beginning of
 * the buffer and updates m_uLast accordingly. If there's no excess data, it
 * simply resets m_uLast to 0.
 *
 * This operation optimizes memory usage by reusing the buffer space and maintaining
 * data continuity when processing large amounts of data.
 *
 * \post If m_uLast > m_uSize, excess data is moved to buffer start and m_uLast is updated
 * \post If m_uLast <= m_uSize, m_uLast is reset to 0
 *
 * \code
 * line l(256);
 * l.create();
 * // After some writes that pushed m_uLast beyond m_uSize...
 * l.rotate(); // Moves excess data to beginning of buffer
 * \endcode
 */
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