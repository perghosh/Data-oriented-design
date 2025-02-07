#include "gd_strings.h"

_GD_BEGIN

/// align to 4 byte boundary
inline uint32_t align32_g( uint32_t uLength ) 
{
   if( uLength % 4 != 0 ) uLength = (uLength + 3) & ~3;
   return uLength;
}

/// align to 4 byte boundary
inline uint64_t align32_g( uint64_t uLength ) 
{
   if( uLength % 4 != 0 ) uLength = (uLength + 3) & ~3;
   return uLength;
}

/// align to 64 byte cache line
inline uint64_t align_cache_g( uint64_t uLength ) 
{
   if( uLength % 64 != 0 ) uLength = (uLength + 63) & ~63;
   return uLength;
}


/** ---------------------------------------------------------------------------
 * @brief get std::string_view from buffer at offset position
 * @param uOffset offset position in buffer
 * @return std::string_view for string at offset position 
 */
std::string_view strings32::at(uint64_t uOffset) const
{                                                                                                  assert( uOffset < m_uSize );
   const uint8_t* puPosition = get_position(uOffset); 
   return to_string_view_s(m_puBuffer, uOffset);
}


/** ---------------------------------------------------------------------------
 * @brief Appends a new string to the buffer.
 *
 * This method appends a new string to the internal buffer by writing a 32-bit length header
 * followed by the string data. The total block size (header + data + padding) is aligned to a 4-byte boundary.
 * It reserves additional space if necessary and, in debug builds, clears any extra padding bytes.
 *
 * @param stringAppend A std::string_view representing the string to be appended.
 */
void strings32::append(const std::string_view& stringAppend)  
{                                                                                                  assert( stringAppend.length() < 0xffff'ffff );
   auto uSize = stringAppend.size();                                          // uSize: Length of the appended string                                            
   decltype( uSize ) uBlockSize = align32_g(uSize + sizeof(uint32_t));        // uBlockSize: Total block size (length size header + string data + padding)
   reserve_add(uBlockSize);                                                   // Ensure there is enough capacity in the buffer for the new block

   // Position the insertion pointer at the end of the buffer
   uint8_t* puInsertionPosition = m_puBuffer + m_uSize;                                            assert( m_uSize % sizeof(uint32_t) == 0 );
   uint32_t uLength = (uint32_t)uSize;                                        // uLength: Length of the string block, compiler will optimize this
   *reinterpret_cast<uint32_t*>(puInsertionPosition) = uLength;               // Write the string length at the beginning of the string block  
   memcpy(puInsertionPosition + sizeof(uint32_t), stringAppend.data(), uSize);// Copy the string data into the block after the length header  

#ifndef NDEBUG // ubd debug, clear padding bytes (never read those bytes so it is not necessary to clear them in release)
   if(uBlockSize > sizeof(uint32_t) + uSize)                                  // If padding is needed to reach a 4-byte alignment  
   {  
      memset(puInsertionPosition + 4 + uSize, 0, uBlockSize - 4 - uSize);     // padd bytes with zero  
   }  
#endif

   m_uSize += uBlockSize;                                                     // Update the used buffer size to include the new block  
}  


/** ---------------------------------------------------------------------------
 * @brief Counts the total number of stored strings.
 *
 * This method iterates through the internal buffer, which contains multiple string blocks.
 * Each block starts with a 32-bit length header and is aligned to a 4-byte boundary.
 * The method increments a counter for every block encountered until the end of the buffer is reached.
 *
 * @return The total number of strings stored in the buffer.
 */
size_t strings32::count() const  
{  
   size_t uCount = 0;                                                         // uCount: Counter for the number of strings  
   uint8_t* puPosition = m_puBuffer;                                          // puCurrent: Pointer used to iterate through the buffer  
   uint8_t* puPositionEnd = m_puBuffer + m_uSize;                             // puPositionEnd: Pointer to the end of the buffer
   while( puPosition < puPositionEnd)                                         // Loop through each string block  
   {  
      uint32_t uLength = *reinterpret_cast<uint32_t*>(puPosition);            // uLength: Length of the current string  
      uLength = align32_g(uLength + (uint32_t)sizeof(uint32_t));              // Align the length to a 4-byte boundary
      puPosition += uLength;                                                  // Advance to the next string block  
      uCount++;                                                               // Increment the count of strings  
   }  

   return uCount;                                                             // Return the total count of stored strings  
}  

/** ---------------------------------------------------------------------------
 * Removes a string from the buffer at the given iterator position.
 * 
 * @param {iterator} it - The iterator pointing to the position where the string should be removed.
 * 
 * @description This method:
 * - Retrieves the position of the string from the iterator.
 * - Calculates the length of the string including its header and any padding.
 * - Shifts all subsequent data in the buffer left to fill the gap left by the removed string.
 * - Reduces the total buffer size accordingly.
 * 
 * @note 
 * - The string length is stored as a `uint32_t` at the beginning of the string's block.
 * - `align32_g` is assumed to be a function for 32-bit alignment.
 * - `memmove` is used for safe memory movement due to potential overlapping regions.
 */
void strings32::erase( iterator it )   
{  
   uint8_t* puPosition = it.get_position();                                                        assert( puPosition < buffer_end() ); assert( puPosition >= buffer() );
   uint32_t uLength = *reinterpret_cast<uint32_t*>(puPosition);               // uLength: Length of the string to remove  
   uint32_t uBlockSize = align32_g(uLength + (uint32_t)sizeof(uint32_t));     // uBlockSize: Total block size (header + data + padding) of the string to remove
   uint8_t* puNext = puPosition + uBlockSize;                                 // puNext: Pointer to the next block after the removed string
   uint64_t uMoveSize = m_uSize - (puNext - m_puBuffer);                      // uMoveSize: Number of bytes to shift left to fill the gap  
   memmove(puPosition, puNext, uMoveSize);                                    // Shift the subsequent blocks left to overwrite the removed block  
   m_uSize -= uBlockSize;                                                     // Decrease the used buffer size by the size of the removed block  
}  


/** ---------------------------------------------------------------------------
 * @brief Replaces the string at the given iterator position with a new string.
 *
 * This method manages the replacement of a string within a custom string buffer
 * where each string is prefixed with its length in 32-bit unsigned integer format,
 * and the data is stored in 32-byte aligned blocks.
 *
 * @param itPosition An iterator pointing to the position where the string should be replaced.
 * @param stringReplace The new string to replace the existing one.
 *
 * @details
 * - **Buffer Management**: 
 *   - Checks if the new string requires more space than the existing one; if so, 
 *     it ensures the buffer has enough capacity by potentially increasing its size.
 *   - If the new string's block size differs from the old one, it shifts the data 
 *     following the replacement point to maintain buffer integrity.
 *
 * - **Assertions**: 
 *   - Ensures the position for replacement is within the buffer's bounds.
 *
 * @pre 
 * - `itPosition` must be a valid iterator pointing to a position within the string buffer.
 * - `stringReplace` should not exceed the maximum size that can be represented by `uint32_t`.
 *
 * @post 
 * - The content at `itPosition` is replaced with `stringReplace`.
 * - If necessary, the subsequent data in the buffer is shifted, and `m_uSize` is updated accordingly.
 */
void strings32::replace( uint8_t* puPosition, const std::string_view& stringReplace)  
{                                                                                                  assert( puPosition >= buffer() ); assert( puPosition < buffer_end() ); assert( (puPosition - buffer()) % 4 == 0 );
#ifndef NDEBUG
   std::string_view string_begin_d = to_string_view_s( buffer(), puPosition - buffer() );
   auto uLength_d = string_begin_d.size();
#endif

   // ## Get the position of the string to replace and calculate the block sizes
   uint64_t uOffset = puPosition - m_puBuffer;                                 // Offset of the string to replace
   uint8_t* puReplacePosition = puPosition;                                                        assert( puReplacePosition < buffer_end() ); assert( puReplacePosition >= buffer() );
   uint32_t uOldLength = *reinterpret_cast<uint32_t*>(puReplacePosition);      // uOldLength: Length of the existing string  
   uint32_t uOldBlockSize = align32_g(uOldLength + (uint32_t)sizeof(uint32_t));// uOldBlockSize: Total block size of the existing string  

   // ## Calculate the new block size and check if the buffer can accommodate the new string
   uint32_t uNewLength = static_cast<uint32_t>(stringReplace.size());           // uNewLength: Length of the new string  
   uint32_t uNewBlockSize = align32_g(uNewLength + (uint32_t)sizeof(uint32_t));         // uNewBlockSize: Total block size required for the new string  
   int64_t iSizeDifference = static_cast<int64_t>(uNewBlockSize) - static_cast<int64_t>(uOldBlockSize);  // iSizeDifference: Difference between new and old block sizes  

   // ## Check if the new block is larger than the old one, then make sure the buffer can accommodate the extra bytes and update position
   if( iSizeDifference > 0 ) 
   { 
      reserve_add(static_cast<uint64_t>(iSizeDifference));  
      puReplacePosition = buffer() + uOffset;                                                      assert( puReplacePosition < buffer_end() ); assert( puReplacePosition >= buffer() );
   }  

   // ## Check if there is any size difference, then shift the subsequent data to accommodate the new block size
   if( iSizeDifference != 0 )                                                 // If there is any size difference  
   {  
      uint8_t* puSource = puReplacePosition + uOldBlockSize;                  // puSource: Pointer to the data after the old block  
      uint64_t uMoveSize = m_uSize - (puSource - m_puBuffer);                 // uMoveSize: Number of bytes to move to adjust for the size change, it's the size from start of value to end of buffer that is used  
      memmove(puSource + iSizeDifference, puSource, uMoveSize);               // Shift the subsequent data to accommodate the new block size  
      m_uSize += iSizeDifference;                                             // Update the used buffer size after the replacement  
   }  

   *reinterpret_cast<uint32_t*>(puReplacePosition) = uNewLength;              // Write the new string length into the block header  
   puReplacePosition += sizeof(uint32_t);                                     // Move the pointer to the start of the string data
   memcpy(puReplacePosition, stringReplace.data(), uNewLength);               // Copy the new string data into the block  
#ifndef NDEBUG
   std::string_view string_d( (const char*)puReplacePosition, uNewLength );
   uLength_d = string_d.size();
#endif
}  



/** ---------------------------------------------------------------------------
 * Reserves space for at least `uSize` bytes in the buffer, resizing if necessary.
 * This method ensures that the buffer has enough space to hold the specified number of bytes without reallocation during future insertions.
 *
 * @param {uint64_t} uSize - The minimum number of bytes to reserve. 
 *
 * @description 
 * - If `uSize` is greater than the current buffer size, this method:
 *   - Calculates a new buffer size, starting at least from 64 bytes if the current buffer is empty, or doubles the current size until it meets or exceeds `uSize`.
 *   - Allocates a new buffer of this calculated size.
 *   - If an existing buffer is present, it copies the current content to the new buffer before deallocating the old one.
 *   - Updates internal pointers and size variables to reflect the new buffer state.
 *
 * @note 
 * - Memory allocation for the new buffer is done with `new uint8_t[]`, which could throw if memory allocation fails.
 * - The method uses exponential growth strategy for buffer resizing to amortize the cost of reallocations.
 *
 * @example
 * auto pstrings32 = new strings32();
 * pstrings32->reserve(1024);  // Ensures at least 1024 bytes are available
 */
void strings32::reserve( uint64_t uSize )  
{                                                                                                  assert(uSize < 0xffff'ffff'ffff);  // realistic ??
   if( uSize > m_uBufferSize )                                                // If the current capacity is insufficient  
   {  
      uint64_t uNewBufferSize = align_cache_g(uSize + (uSize / 2));           // Align the requested size to cache line size
      uint8_t* puNewBuffer = new uint8_t[uNewBufferSize];                     // puNewBuffer: Allocate a new buffer with the increased capacity  
      if (m_puBuffer != nullptr)                                              // If an old buffer exists  
      {  
         memcpy(puNewBuffer, m_puBuffer, m_uSize);                            // Copy existing data to the new buffer  
         delete[] m_puBuffer;                                                 // Free the old buffer memory  
      }  
      m_puBuffer = puNewBuffer;                                               // Update the buffer pointer to point to the new buffer  
      m_uBufferSize = uNewBufferSize;                                         // Update the total capacity of the buffer  
   }  
}  

/** ---------------------------------------------------------------------------
 * Advances the buffer offset by moving past a string block.
 * 
 * @param uOffset The current offset in the buffer from which to advance.
 * @return uint64_t The offset to next string
 * 
 * @note 
 * - The method assumes `uOffset` is aligned to a 32-bit boundary and within the buffer size.
 * - `align32_g` is used to calculate the size of the string block, including any necessary padding to align to 32 bits.
 * - The string length is expected at the beginning of each block.
 * 
 * @pre 
 * - `uOffset < m_uSize` : Ensures the offset is within the buffer.
 * - `(uOffset % sizeof(uint32_t)) == 0` : Ensures the offset is 32-bit aligned.
 */
uint64_t strings32::advance(uint64_t uOffset) const 
{                                                                                                  assert(uOffset < m_uSize); assert(( uOffset % sizeof(uint32_t) ) == 0);
   uint32_t uLength = *reinterpret_cast<uint32_t*>( m_puBuffer + uOffset );   // uLength: Length of the current string  
   uint32_t uBlockSize = align32_g(uLength + (uint32_t)sizeof(uint32_t));     // uBlockSize: Total block size of the current string  
   return uOffset + uBlockSize;
}


_GD_END