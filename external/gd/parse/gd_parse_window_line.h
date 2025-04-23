/**
 * @file gd_parse_window_line.h
 */


#pragma once

#include <cassert>
#include <cstring>
#include <span>
#include <string>
#include <string_view>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_BEGIN
#  define _GD_PARSE_BEGIN namespace gd { namespace parse {
#  define _GD_PARSE_END } }
#  define _GD_PARSE_WINDOW_BEGIN namespace gd { namespace parse { namespace window {
#  define _GD_PARSE_WINDOW_END } } }
#endif

_GD_PARSE_WINDOW_BEGIN

/**
 * \class line
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
 * Example usage:
 * @code
 *   // Create a buffer with an initial size of 1024 bytes
 *   line buffer(1024, gd::types::tag_create{});
 *   
 *   // Write data to the buffer
 *   const char* data = "Hello, world!";
 *   buffer.write((const uint8_t*)data, strlen(data));
 *   
 *   // Process the buffer
 *   int64_t pos = buffer.find(',');
 *   if(pos != -1) {
 *     // Found a comma at position 'pos'
 *   }
 *   
 *   // Convert to string_view for text operations
 *   std::string_view sv = buffer;
 * @endcode
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
   /// construct without need to call create method, called in constructor
   line(uint64_t uSize, gd::types::tag_create) : m_uSize(uSize), m_uCapacity(uSize + (uSize >> 1)) { create(); }
   line(uint64_t uSize, uint64_t uCapacity) : m_uSize(uSize), m_uCapacity(uCapacity) { assert(m_uCapacity > m_uSize); }
   /// construct without need to call create method, called in constructor
   line(uint64_t uSize, uint64_t uCapacity, gd::types::tag_create) : m_uSize(uSize), m_uCapacity(uCapacity) { assert(m_uCapacity > m_uSize); create(); }
   // copy
   line(const line& o) { common_construct(o); }
   line(line&& o) noexcept { common_construct(std::move(o)); }
   // assign
   line& operator=(const line& o) { common_construct(o); return *this; }
   line& operator=(line&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~line() { delete [] m_puBuffer; }
private:
   // common copy
   void common_construct(const line& o);
   void common_construct(line&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:
   uint8_t& operator[](uint64_t uIndex) { assert(m_uCapacity > m_uSize); assert(m_puBuffer != nullptr); return m_puBuffer[uIndex]; } ///< access buffer data
   operator std::string_view() const { return std::string_view((const char*)m_puBuffer, size()); } ///< convert to string view


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   /// set last position in buffer, last position is the position where data is written to, marks end of valid information not processed
   void set_last(uint64_t uLast) { assert( uLast <= m_uCapacity ); m_uLast = uLast; }
   /// add to last position in buffer, this will add to the last position and update the size of the buffer
   /// When writing data to the buffer, call this to update internal data size or call `update` method
   void add( uint64_t uAdd ) { m_uLast += uAdd; m_uSizeSummary += uAdd; assert( m_uLast <= m_uCapacity ); }
//@}

/** \name OPERATION
*///@{
/// create buffer, this will allocate memory for the buffer and set the size of the buffer
   void create();
   /// write data to buffer, this will write data to the end of the buffer and return the number of bytes written
   uint64_t write(const uint8_t* puData, uint64_t uSize); 
   /// rotate buffer, move data from end of buffer to start of buffer
   void rotate();
   /// close buffer, free memory and reset attributes
   void close() { delete[] m_puBuffer; m_puBuffer = nullptr; m_uLast = 0; m_uSizeSummary = 0; } ///< close buffer, free memory and reset attributes

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
   uint64_t size_margin() const { return m_uCapacity - m_uSize; }              ///< size of the extra space after buffer used to process data

   bool eof() const { return m_uLast == 0; } ///< end of file, no more data in buffer
   bool empty() const { return m_uLast == 0; } ///< buffer is empty

   /// get start and end of occupied data in buffer
   std::pair<const uint8_t*, const uint8_t*> range( gd::types::tag_pair );

   // ## find methods

   int64_t find(const uint8_t* puData, uint64_t uSize, uint64_t uOffset = 0) const; ///< find data in buffer
   int64_t find(const std::string_view& stringData, uint64_t uOffset = 0) const { return find((const uint8_t*)stringData.data(), stringData.length(), uOffset); } ///< find data in buffer
   int64_t find(char iCharacter, uint64_t uOffset = 0) const;
   int64_t find(const std::span<const uint8_t>& span256_, uint64_t uOffset = 0) const;

   // ## count methods

   uint64_t count(const uint8_t* puData, uint64_t uSize, uint64_t uOffset = 0) const; ///< count data in buffer
   uint64_t count(const std::string_view& stringData, uint64_t uOffset = 0) const { return count((const uint8_t*)stringData.data(), stringData.length(), uOffset); } ///< count data in buffer
   uint64_t count(char iCharacter, uint64_t uOffset = 0) const;

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

/// return start and end of occupied data in buffer
inline std::pair<const uint8_t*, const uint8_t*> line::range(gd::types::tag_pair) { assert( m_puBuffer != nullptr );
   return std::make_pair(m_puBuffer, m_puBuffer + occupied());
}

/*
 * \class line
 * 
 * Advanced usage:
 * \code
   // Define the file to read
   std::string stringFile = R"(D:\temp\sqlite3.c)";
   std::ifstream file_(stringFile, std::ios::binary);

   // Characters to count
   std::array<uint8_t, 256> arrayToCount = { 0 };
   arrayToCount['a'] = 1; arrayToCount['b'] = 2; arrayToCount['c'] = 3; arrayToCount['d'] = 4;
   
   unsigned puCount[4] = {0}; // Initialize counts for each character

   // Create and initialize the line buffer
   gd::parse::window::line windowLine_(256, gd::types::tag_create{});

   // Read the file into the buffer
   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);  
   auto uSize = file_.gcount();
   windowLine_.update(uSize);

   std::span<const uint8_t> span256_(arrayToCount.data(), 256);

   // Process the file
   while(windowLine_.eof() == false)
   {
      uint64_t uOffset = 0;
      int64_t iFind = windowLine_.find( { arrayToCount.data(), 256 }, uOffset );
      for( ; iFind != -1; iFind = windowLine_.find( { arrayToCount.data(), 256 }, uOffset ) )
      {
         // Increment the count for the found character
         char found_ = windowLine_[iFind];
         unsigned uCharacter = arrayToCount[found_];
         puCount[(uCharacter-1)]++;
         iFind++;
         uOffset = iFind;
      }

      // Rotate the buffer and read more data
      windowLine_.rotate();
      file_.read((char*)windowLine_.buffer(), windowLine_.available());
      uSize = file_.gcount();
      windowLine_.update(uSize);
   }

   // Output the counts for each character
   for(size_t u = 0; u < 4; ++u)
   {
      std::cout << "Character '" << char('a' + u) << "' count: " << puCount[u] << "\n";
   }

   file_.close();
 * 
 * \endcode
 * 
 */


_GD_PARSE_WINDOW_END