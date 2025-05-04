/**
 * @file gd_parse_window_line.cpp
 */


#include "gd_parse_window_line.h"

_GD_PARSE_WINDOW_BEGIN


/// \brief Copy constructor for line class.
void line::common_construct(const line& o) 
{
   // Copy size and capacity
   m_uSize = o.m_uSize;
   m_uCapacity = o.m_uCapacity;
   m_uFirst = o.m_uFirst;
   m_uLast = o.m_uLast;
   m_uSizeSummary = o.m_uSizeSummary;

   // ## Allocate new memory for the buffer and copy the data
   if(o.m_puBuffer != nullptr) 
   { 
      m_puBuffer = new uint8_t[m_uCapacity];  
      std::memcpy(m_puBuffer, o.m_puBuffer, m_uLast);
   } 
   else 
   {
      m_puBuffer = nullptr;
   }
}

/// \brief Move constructor for line class.
void line::common_construct(line&& o) noexcept 
{
   // Transfer ownership of attributes
   m_uSize = o.m_uSize;
   m_uCapacity = o.m_uCapacity;
   m_uFirst = o.m_uFirst;
   m_uLast = o.m_uLast;
   m_uSizeSummary = o.m_uSizeSummary;

   // Transfer ownership of the buffer
   m_puBuffer = o.m_puBuffer;

   // Leave the source object in a valid but empty state
   o.m_uSize = 0;
   o.m_uCapacity = 0;
   o.m_uFirst = 0;
   o.m_uLast = 0;
   o.m_uSizeSummary = 0;
   o.m_puBuffer = nullptr;
}



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

/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of a sequence of bytes in the buffer.
 *
 * This method searches for the specified sequence of bytes within the buffer,
 * starting from the given offset. If the sequence is found, the index of its
 * first occurrence is returned. If the sequence is not found, -1 is returned.
 *
 * @param puData Pointer to the sequence of bytes to search for.
 * @param uSize The size of the sequence to search for, in bytes.
 * @param uOffset The offset in the buffer to start the search from (default is 0).
 *
 * @return The index of the first occurrence of the sequence in the buffer, or -1 if not found.
 */
int64_t line::find(const uint8_t* puData, uint64_t uSize, uint64_t uOffset) const
{                                                                                                  assert(m_puBuffer != nullptr); assert(m_uLast > 0); assert(m_uLast >= uOffset);
   const uint8_t* puPosition = m_puBuffer + uOffset; // Pointer to start of search
   const uint8_t* puEnd = puPosition + (occupied() - uOffset); // Pointer to end of search

   while( puPosition < puEnd )
   {
      // Check if the current position matches the data to find
      if( std::memcmp(puPosition, puData, uSize) == 0 )
      {
         return puPosition - m_puBuffer;                                       // Return the index of the found data
      }
      puPosition++;                                                            // Move to the next position
   }

   return -1;                                                                  // Data not found
}

/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of a character in the buffer.
 *
 * This method searches for the specified character within the buffer,
 * starting from the given offset. If the character is found, the index of its
 * first occurrence is returned. If the character is not found, -1 is returned.
 *
 * @param iCharacter The character to search for.
 * @param uOffset The offset in the buffer to start the search from (default is 0).
 *
 * @return The index of the first occurrence of the character in the buffer, or -1 if not found.
 */
int64_t line::find(char iCharacter, uint64_t uOffset) const
{                                                                                                  assert(m_puBuffer != nullptr); assert(m_uLast > 0); assert(m_uLast >= uOffset);
   const uint8_t* puPosition = m_puBuffer + uOffset; // Pointer to start of search
   const uint8_t* puEnd = m_puBuffer + occupied(); // Pointer to end of search
   while( puPosition < puEnd )
   {
      // Check if the current position matches the character to find
      if( *puPosition == iCharacter )
      {
         return puPosition - m_puBuffer;                                       // Return the index of the found character
      }
      puPosition++;                                                            // Move to the next position
   }
   return -1;                                                                  // Character not found
}

/** --------------------------------------------------------------------------- 
 * Searches for the first occurrence of any character marked as present in the
 * provided span, starting from the specified offset.
 *
 * @param span256_ A 256-element span where non-zero values mark characters to find.
 *                 Each index represents an ASCII/byte value, and non-zero values
 *                 at that index indicate the character should be matched.
 * @param uOffset Starting position in the buffer to begin the search.
 * @return The index position of the first matching character, or -1 if not found.
 *
 * This method uses a lookup table approach where span256_ acts as a presence map
 * for characters to be found. For each byte in the buffer, its value is used as
 * an index into span256_ - if the value at that index is non-zero, the character
 * is considered a match.
 */
int64_t line::find(const std::span<const uint8_t>& span256_, uint64_t uOffset) const
{                                                                                                  assert(m_puBuffer != nullptr); assert( span256_.size() == 256 ); assert(m_uLast > 0); assert(m_uLast >= uOffset); // Ensure span size is 256
   const uint8_t* puPosition = m_puBuffer + uOffset; // Pointer to start of search
   const uint8_t* puEnd = puPosition + (occupied() - uOffset); // Pointer to end of search
                                                                                                   assert( puPosition <= puEnd );
   while( puPosition < puEnd )
   {
      // Check if the current position is any of the characters in the span
      if( span256_[*puPosition] != 0 )
      {
         return puPosition - m_puBuffer;                                       // Return the index of the found character
      }
      puPosition++;                                                            // Move to the next position
   }
   return -1;                                                                  // Character not found
}


/** ---------------------------------------------------------------------------
 * @brief Counts the occurrences of a specific byte sequence in the buffer.
 *
 * This method counts how many times a specified sequence of bytes appears in
 * the buffer, starting from a given offset. It uses the find method to locate
 * each occurrence and increments the count accordingly.
 *
 * @param puData Pointer to the byte sequence to count.
 * @param uSize Size of the byte sequence to count, in bytes.
 * @param uOffset The offset in the buffer to start counting from (default is 0).
 *
 * @return The number of occurrences of the specified byte sequence in the buffer.
 */
uint64_t line::count(const uint8_t* puData, uint64_t uSize, uint64_t uOffset) const
{                                                                                                  assert(puData != nullptr); assert(uSize > 0); assert(uOffset < m_uLast);assert( uSize < size_margin());
   uint64_t uCount = 0;       // Counter for occurrences
   int64_t iPosition = (int64_t)uOffset;

   // ## Start counting and use the find method to locate occurrences of the data sequence
   while( (iPosition = find(puData, uSize, iPosition) ) != -1 )
   {
      uCount++;                                                               // Increment the count for each occurrence
      iPosition += uSize;                                                     // Move the search position forward to avoid overlapping matches
   }

   return uCount;
}

/** ---------------------------------------------------------------------------
 * @brief Counts the occurrences of a specific character in the buffer.
 *
 * This method counts how many times a specified character appears in the
 * buffer, starting from a given offset. It uses the find method to locate
 * each occurrence and increments the count accordingly.
 *
 * @param iCharacter The character to count.
 * @param uOffset The offset in the buffer to start counting from (default is 0).
 *
 * @return The number of occurrences of the specified character in the buffer.
 */
uint64_t line::count(char iCharacter, uint64_t uOffset) const
{                                                                                                  assert(m_puBuffer != nullptr); assert(uOffset < m_uLast);
   uint64_t uCount = 0;           // Counter for occurrences
   int64_t iPosition = (int64_t)uOffset;

   // ## Use the find method to locate occurrences of the character
   while( (iPosition = find(iCharacter, iPosition) ) != -1 )
   {
      uCount++;                  // Increment the count for each occurrence
      iPosition++;               // Move to the next position to avoid counting the same character
   }

   return uCount;
}


_GD_PARSE_WINDOW_END