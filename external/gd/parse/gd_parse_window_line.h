#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_BEGIN
#  define _GD_PARSE_BEGIN namespace gd { namespace parse {
#  define _GD_PARSE_END } }
#  define _GD_PARSE_WINDOW_BEGIN namespace gd { namespace parse { namespace window {
#  define _GD_PARSE_WINDOW_END } } }
#endif

_GD_PARSE_WINDOW_BEGIN

/**
 * \brief A flexible buffer class for managing linear data with look-ahead capabilities.
 *
 * The line class provides a buffer implementation that supports efficient reading,
 * writing, and processing of linear data. It maintains a main buffer of specified size
 * and capacity, along with a look-ahead buffer positioned after the main buffer.
 *
 * Key features include:
 * - Automatic buffer rotation to efficiently handle continuous data streams
 * - Capacity management to prevent buffer overflows
 * - Look-ahead buffer support for parsers or processors that need to peek ahead
 * - STL-compatible iterators for standard algorithm compatibility
 * - Automatic conversion to string_view for string operations
 *
 * This class is designed for scenarios requiring efficient buffer management with
 * potential look-ahead needs, such as parsers, stream processors, or network buffers.
 *
 * \code
 * // Create a line with 1024 bytes size
 * line buffer(1024);
 * buffer.create();
 * 
 * // Write data to the buffer
 * const uint8_t* sourceData = ...;
 * uint64_t bytesWritten = buffer.write(sourceData, dataSize);
 * 
 * // Process data using iterators
 * for(auto it = buffer.begin(); it != buffer.end(); ++it) {
 *     // Process each byte
 * }
 * 
 * // When processing exceeds buffer size, rotate to reuse space
 * buffer.rotate();
 * \endcode
 */
class line
{
public:
// ## types ------------------------------------------------------------------
   using pointer         = uint8_t*;              ///< pointer type
   using const_pointer   = const uint8_t*;        ///< const pointer type
   using reference       = uint8_t&;              ///< reference type
   using const_reference = const uint8_t&;        ///< const reference type
   using size_type       = size_t;                ///< size type
   using iterator        = uint8_t*;               ///< iterator type
   using const_iterator  = const uint8_t*;         ///< const iterator type



// ## construction -------------------------------------------------------------
public:
   line() { memset( this, 0, sizeof( line ) ); }
   line(uint64_t uSize) : m_uSize(uSize), m_uCapacity(uSize + (uSize >> 1)) {}
   line(uint64_t uSize, uint64_t uCapacity) : m_uSize(uSize), m_uCapacity(uCapacity) { assert(m_uCapacity > m_uSize); }
   // copy
   line(const line& o) { common_construct(o); }
   line(line&& o) noexcept { common_construct(std::move(o)); }
   // assign
   line& operator=(const line& o) { common_construct(o); return *this; }
   line& operator=(line&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~line() { delete [] m_puBuffer; }
private:
   // common copy
   void common_construct(const line& o) {}
   void common_construct(line&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   operator std::string_view() const { return std::string_view((const char*)m_puBuffer, size()); } ///< convert to string view


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   /// set last position in buffer, last position is the position where data is written to, marks end of valid information not processed
   void set_last(uint64_t uLast) { assert( uLast <= m_uCapacity ); m_uLast = uLast; }
   void add( uint64_t uAdd ) { m_uLast += uAdd; m_uSizeSummary += uAdd; assert( m_uLast <= m_uCapacity ); }
//@}

/** \name OPERATION
*///@{
   void create();
   uint64_t write(const uint8_t* puData, uint64_t uSize); 
   void rotate();

   /// available free space in buffer, this informs how much data can be written to the buffer
   uint64_t available() const { return m_uCapacity - m_uLast; } 
   /// occupied space in buffer, this informs how much data is in use
   uint64_t occupied() const { return std::min( m_uLast, m_uSize); } 
   /// buffer where to write data
   uint8_t* buffer() { assert(m_uCapacity > m_uSize); assert(m_puBuffer != nullptr); return m_puBuffer + m_uLast; } 
   /// update used size of buffer, update internal attrbutes and clear unused data in buffer if any. Buffer is ready for reading
   void update(uint64_t uSize);

   uint8_t* data() const { assert(m_uCapacity > m_uSize); assert( m_puBuffer != nullptr ); return m_puBuffer; } ///< pointer to buffer
   uint64_t size() const { assert( m_uCapacity > m_uSize ); return m_uSize; } ///< size of buffer
   uint64_t size_summary() const { return m_uSizeSummary; } ///< size of buffer
   uint64_t capacity() const { assert( m_uCapacity > m_uSize ); return m_uCapacity; } ///< capacity of buffer

   bool eof() const { return m_uLast == 0; } ///< end of file, no more data in buffer

   std::pair<const uint8_t*, const uint8_t*> range( gd::types::tag_pair );

   // ## iterator methods

   iterator begin() { return m_puBuffer; }                                    ///< iterator to start of buffer
   iterator end() { return m_puBuffer + std::min( m_uSize, m_uLast); }        ///< iterator to end of buffer
   const_iterator begin() const { return m_puBuffer; }                        ///< const iterator to start of buffer
   const_iterator end() const { return m_puBuffer + std::min( m_uSize, m_uLast); }///< const iterator to end of buffer
   const_iterator cbegin() const { return m_puBuffer; }                       ///< const iterator to start of buffer
   const_iterator cend() const { return m_puBuffer + std::min( m_uSize, m_uLast); }///< const iterator to end of buffer
//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   uint8_t* m_puBuffer = nullptr;              ///< Main buffer data
   uint8_t* m_puLookAheadBuffer = nullptr;     ///< Additional look-ahead buffer
   uint64_t m_uCapacity;                       ///< Buffer capacity
   uint64_t m_uSize;                           ///< Buffer size
   uint64_t m_uFirst = 0;                      ///< First valid character in buffer
   uint64_t m_uLast = 0;                       ///< Last valid character in buffer
   uint64_t m_uSizeSummary = 0;                ///< Size of data that have been processed

// ## free functions ------------------------------------------------------------
public:



};

/** ---------------------------------------------------------------------------
 * @brief Updates the used size of the buffer and clears unused data if necessary.
 *
 * This method adjusts the internal attributes of the buffer by adding the specified size
 * to the current usage. If the last valid position exceeds the buffer's valid data, the
 * remaining unused space in the buffer is cleared (set to zero).
 *
 * @param uAddSize The size to add to the current buffer usage.
 *
 * @pre `m_uCapacity > m_uSize` - The buffer's capacity must be greater than its current size.
 * @pre `m_puBuffer != nullptr` - The buffer pointer must not be null.
 *
 * @post The buffer's `m_uLast` is updated, and unused space is cleared if `m_uLast > m_uCapacity`.
 */
inline void line::update(uint64_t uAddSize) { assert(m_uCapacity > m_uSize); assert(m_puBuffer != nullptr); 
   add(uAddSize);
   if( m_uLast > m_uCapacity ) 
   { 
      uint64_t uAvailable = available();
      std::memset( buffer(), 0, uAvailable );
   }
}


inline std::pair<const uint8_t*, const uint8_t*> line::range(gd::types::tag_pair) { assert( m_puBuffer != nullptr );
   return std::make_pair(m_puBuffer, m_puBuffer + occupied());
}

_GD_PARSE_WINDOW_END