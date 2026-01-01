// @FILE [tag: binary] [description: Handle binary data] [type: header] [name: gd_binary.h]

/*
 * GD Binary Library
 *
 * This library provides functionality for handling binary data, including:
 * - Hexadecimal string validation and conversion
 * - UUID string validation and conversion
 * - Binary pattern searching
 * - Endianness-aware reading and writing of binary data
 *
 * The library provides two complementary APIs:
 * 1. Global functions (binary_*_g) for simple operations
 * 2. Template classes (reader<E> and writer<E>) for streaming operations
 *
 * The template classes use the global functions internally to ensure
 * consistent behavior and avoid code duplication.
 */

#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <type_traits>
#include <cstring>

#include "gd_types.h"
#include "gd_debug.h"


#if defined( __clang__ )
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#pragma clang diagnostic ignored "-Wunused-value"
#elif defined( __GNUC__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
#pragma warning(push)
#pragma warning( disable : 4267 26495 26812 )
#endif

 // @AI [tag: gd, variant_view] [llm: core]

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

// @API [tag: binary, hex] [description: Hexadecimal related logic, convert from or to hexadecimal, validation etc]

/// Validate hex string, returns pair of ( is valid, error message )
std::pair<bool, std::string> binary_validate_hex_g( std::string_view stringHex );
/// Validate uuid string, returns pair of ( is valid, error message )
std::pair<bool, std::string> binary_validate_uuid_g( std::string_view stringUuid );

/// Copy hex string to binary buffer, buffer must be large enough to hold the data
void binary_copy_hex_g( uint8_t* puBuffer, std::string_view stringHex );
/// Copy uuid string to binary buffer, buffer must be large enough to hold the data
void binary_copy_uuid_g( uint8_t* puBuffer, std::string_view stringHex );

/// Copy hex string to binary buffer, returns number of bytes copied and only copies up to uBufferSize bytes
size_t binary_copy_hex_g( uint8_t* puBuffer, size_t uBufferSize, std::string_view stringHex );

/// Convert binary data to hexadecimal string
void binary_to_hex_g( const uint8_t* puBuffer, size_t uBufferSize, std::string& stringHex, bool bUppercase = false );
std::string binary_to_hex_g( const uint8_t* puBuffer, size_t uBufferSize, bool bUppercase = false );

// @API [tag: binary, find] [description:  find patterns etc in binary data]

/// Find pattern in buffer, returns index or -1 if not found
int64_t buffer_find_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize, size_t uOffset = 0 );
int64_t buffer_find_last_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize );

// @API [tag: binary, read, write] [description: read and write information from binary data]
// Note: The template classes below (reader and writer) use these global functions internally
// to provide consistent behavior and avoid code duplication.

const uint8_t* binary_read_be_g( const uint8_t* p_, uint16_t& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, uint32_t& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, uint64_t& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, int16_t& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, int32_t& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, int64_t& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, float& v_ );
const uint8_t* binary_read_be_g( const uint8_t* p_, double& v_ );

const uint8_t* binary_read_le_g( const uint8_t* p_, uint16_t& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, uint32_t& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, uint64_t& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, int16_t& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, int32_t& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, int64_t& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, float& v_ );
const uint8_t* binary_read_le_g( const uint8_t* p_, double& v_ );


const uint8_t* binary_read_g( const uint8_t* p_, uint8_t& v_ );
const uint8_t* binary_read_g( const uint8_t* p_, int8_t& v_ );


// Write function declarations
uint8_t* binary_write_be_g( uint8_t* p_, uint16_t v_ );
uint8_t* binary_write_be_g( uint8_t* p_, uint32_t v_ );
uint8_t* binary_write_be_g( uint8_t* p_, uint64_t v_ );
uint8_t* binary_write_be_g( uint8_t* p_, int16_t v_ );
uint8_t* binary_write_be_g( uint8_t* p_, int32_t v_ );
uint8_t* binary_write_be_g( uint8_t* p_, int64_t v_ );
uint8_t* binary_write_be_g( uint8_t* p_, float v_ );
uint8_t* binary_write_be_g( uint8_t* p_, double v_ );

uint8_t* binary_write_le_g( uint8_t* p_, uint16_t v_ );
uint8_t* binary_write_le_g( uint8_t* p_, uint32_t v_ );
uint8_t* binary_write_le_g( uint8_t* p_, uint64_t v_ );
uint8_t* binary_write_le_g( uint8_t* p_, int16_t v_ );
uint8_t* binary_write_le_g( uint8_t* p_, int32_t v_ );
uint8_t* binary_write_le_g( uint8_t* p_, int64_t v_ );
uint8_t* binary_write_le_g( uint8_t* p_, float v_ );
uint8_t* binary_write_le_g( uint8_t* p_, double v_ );

uint8_t* binary_write_g( uint8_t* p_, uint8_t v_ );
uint8_t* binary_write_g( uint8_t* p_, int8_t v_ );


namespace binary {

/// Endian enumeration for specifying byte order
enum class enumEndian { eEndianBig, eEndianLittle, eEndianNative };

/// Template class for reading binary data with specified endianness
/// This class uses the global binary_read_* functions internally to ensure
/// consistent behavior and avoid code duplication.
template <enumEndian E>
struct reader {
   /// Create a reader with begin and end pointers
   reader( const uint8_t* puBegin, const uint8_t* puEnd ): m_puPosition( puBegin ), m_puBegin( puBegin ), m_puEnd( puEnd ) {}

   /// Create a reader with begin pointer and size
   reader( const uint8_t* puBegin, size_t uSize ): m_puPosition( puBegin ), m_puBegin( puBegin ), m_puEnd( puBegin + uSize ) {}

   /// Generic constructor for contiguous containers (std::array, std::vector, std::string)
   template <typename CONTAINER>
   reader( CONTAINER& container_ ): reader( reinterpret_cast<uint8_t*>(container_.data()), container_.size() * sizeof(typename CONTAINER::value_type) ) {
      // Ensure we aren't trying to write to complex types (like std::string inside a vector)
      // We only want raw data buffers.
      static_assert( std::is_trivially_copyable<typename CONTAINER::value_type>::value, "Container value_type must be trivially copyable (POD)" );
   }

   /// Check if at or past end of buffer
   bool eof() const { return m_puPosition >= m_puEnd; }

   /// Check if previous operation had an error (overflow)
   bool error() const { return m_puPosition > m_puEnd; }

   /// Read a value and return it
   template <typename T>
   T read() {
      T value;
      *this >> value;
      return value;
   }

   /// Get current position from beginning
   size_t position() const { return m_puPosition - m_puBegin; }

   // Returns a marker that can be used to restore position
   size_t mark() const { return m_puPosition - m_puBegin; }

   /// Reset position to previously obtained mark
   void reset( size_t uMark ) { if( uMark <= size() ) { m_puPosition = m_puBegin + uMark; } }

   /// Get remaining bytes
   size_t remaining() const { return m_puEnd - m_puPosition; }

   /// Get total size
   size_t size() const { return m_puEnd - m_puBegin; }

   /// Seek to specific position
   void seek( size_t uPosition ) {
      if( uPosition <= size() ) { m_puPosition = m_puBegin + uPosition; }
   }

   /// Skip ahead by specified number of bytes
   void skip( size_t uCountbytes ) {
      if( uCountbytes <= remaining() ) { m_puPosition += uCountbytes; }
      else { m_puPosition = m_puEnd; }                                   // Skip to end if would overflow
   }

   /// Peek at next byte without advancing position
   uint8_t peek() const {
      return m_puPosition < m_puEnd ? *m_puPosition : 0;
   }

   /// Read raw bytes to buffer (not that this has not runtime checks)
   void read_bytes( uint8_t* puData, size_t uSize ) {                         assert( m_puPosition + uSize <= m_puEnd );
         std::memcpy( puData, m_puPosition, uSize );
         m_puPosition += uSize;
   }
   void read_bytes( void* pData, size_t uSize ) { read_bytes( static_cast<uint8_t*>( pData ), uSize ); }


   const uint8_t* m_puPosition;  ///< Current position in buffer
   const uint8_t* m_puBegin;     ///< Start of buffer
   const uint8_t* m_puEnd;       ///< End of buffer
};

/// Template class for writing binary data with specified endianness
/// This class uses the global binary_write_* functions internally to ensure
/// consistent behavior and avoid code duplication.
template <enumEndian E>
struct writer {
   /// Create a writer with begin and end pointers
   writer( uint8_t* puBegin, uint8_t* puEnd ): m_puPosition( puBegin ), m_puBegin( puBegin ), m_puEnd( puEnd ) {}

   /// Create a writer with begin pointer and size
   writer( uint8_t* puBegin, size_t uSize ): m_puPosition( puBegin ), m_puBegin( puBegin ), m_puEnd( puBegin + uSize ) {}
   writer( void* puBegin, size_t uSize ): m_puPosition( static_cast<uint8_t*>(puBegin) ), m_puBegin( static_cast<uint8_t*>(puBegin) ), m_puEnd( puBegin + uSize ) {}

   /// Generic constructor for contiguous containers (std::array, std::vector, std::string)
   template <typename CONTAINER>
   writer( CONTAINER& container_ ): writer( reinterpret_cast<uint8_t*>(container_.data()), container_.size() * sizeof(typename CONTAINER::value_type) ) {
      // Ensure we aren't trying to write to complex types (like std::string inside a vector)
      // We only want raw data buffers.
      static_assert( std::is_trivially_copyable<typename CONTAINER::value_type>::value, "Container value_type must be trivially copyable (POD)" );
   }

   /// Check if at or past end of buffer
   bool eof() const { return m_puPosition >= m_puEnd; }

   /// Check if previous operation had an error (overflow)
   bool error() const { return m_puPosition > m_puEnd; }

   /// Get current position from beginning
   size_t position() const { return m_puPosition - m_puBegin; }

   /// Get remaining bytes
   size_t remaining() const { return m_puEnd - m_puPosition; }

   /// Get total size
   size_t size() const { return m_puEnd - m_puBegin; }

   /// Seek to specific position
   void seek( size_t uPosition ) {
      if( uPosition <= size() ) { m_puPosition = m_puBegin + uPosition; }
   }

   /// Skip ahead by specified number of bytes
   void skip( size_t uCountbytes ) {
      if( uCountbytes <= remaining() ) { m_puPosition += uCountbytes; }
      else { m_puPosition = m_puEnd; }                                   // Skip to end if would overflow
   }

   /// Write raw bytes to buffer (not that this has not runtime checks)
   void write_bytes( const uint8_t* puData, size_t uSize ) {                  assert( m_puPosition + uSize <= m_puEnd );
         std::memcpy( m_puPosition, puData, uSize );
         m_puPosition += uSize;
   }
   void write_bytes( const void* pData, size_t uSize ) { write_bytes( static_cast<const uint8_t*>( pData ), uSize ); }

   uint8_t* m_puPosition;  ///< Current position in buffer
   uint8_t* m_puBegin;     ///< Start of buffer
   uint8_t* m_puEnd;       ///< End of buffer
};

// Helper to get unsigned type for size
template <typename T>
using make_uint_t = typename std::make_unsigned<T>::type;

// Generic Read Implementation
// Helper function to call appropriate global read function based on endianness
template <enumEndian E, typename T>
const uint8_t* call_global_read( const uint8_t* p_, T& v_ ) {
   if constexpr( E == enumEndian::eEndianBig ) {
      return binary_read_be_g( p_, v_ );
   }
   else if constexpr( E == enumEndian::eEndianLittle ) {
      return binary_read_le_g( p_, v_ );
   }
   else { // eEndianNative
      return binary_read_g( p_, v_ );
   }
}

// Helper function to call appropriate global write function based on endianness
template <enumEndian E, typename T>
uint8_t* call_global_write( uint8_t* p_, T v_ ) {
   if constexpr( E == enumEndian::eEndianBig ) {
      return binary_write_be_g( p_, v_ );
   }
   else if constexpr( E == enumEndian::eEndianLittle ) {
      return binary_write_le_g( p_, v_ );
   }
   else { // eEndianNative
      return binary_write_g( p_, v_ );
   }
}

/// Generic Read Operator using global functions
/// This operator allows reading arithmetic types from a binary stream
/// with proper bounds checking and endianness handling.
template <enumEndian E, typename T>
binary::reader<E>& operator>>( binary::reader<E>& reader_, T& v_ ) {
   // Only allow arithmetic types (int, float, etc) to prevent buffer overflows on structs
   static_assert( std::is_arithmetic<T>::value, "Type must be arithmetic" );

   // Bounds check with proper error handling
   if( reader_.m_puPosition + sizeof( T ) > reader_.m_puEnd ) {
      // Set stream position to end to indicate error
      reader_.m_puPosition = reader_.m_puEnd;
      v_ = T{}; // Initialize with default value
      return reader_;
   }

   reader_.m_puPosition = call_global_read<E>( reader_.m_puPosition, v_ );
   return reader_;
}

/// Generic Write Operator using global functions
/// This operator allows writing arithmetic types to a binary stream
/// with proper bounds checking and endianness handling.
template <enumEndian E, typename TYPE>
binary::writer<E>& operator<<( binary::writer<E>& writer_, TYPE v_ ) {
   static_assert( std::is_arithmetic<TYPE>::value, "Type must be arithmetic" );

   // Bounds check with proper error handling
   if( writer_.m_puPosition + sizeof( TYPE ) > writer_.m_puEnd ) {
      // Set stream position to end to indicate error
      writer_.m_puPosition = writer_.m_puEnd;
      return writer_;
   }

   writer_.m_puPosition = call_global_write<E>( writer_.m_puPosition, v_ );
   return writer_;
}

// Type aliases for your convenience
using read_be = reader<enumEndian::eEndianBig>;
using read_le = reader<enumEndian::eEndianLittle>;
using write_be = writer<enumEndian::eEndianBig>;
using write_le = writer<enumEndian::eEndianLittle>;


} // namespace binary



_GD_END
