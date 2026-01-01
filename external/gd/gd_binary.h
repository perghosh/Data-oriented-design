// @FILE [tag: binary] [description: Handle binary data] [type: header] [name: gd_binary.h]

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

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

const uint8_t* binary_read_be_g(const uint8_t* p_, uint16_t& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, uint32_t& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, uint64_t& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, int16_t& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, int32_t& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, int64_t& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, float& v_);
const uint8_t* binary_read_be_g(const uint8_t* p_, double& v_);

const uint8_t* binary_read_le_g(const uint8_t* p_, uint16_t& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, uint32_t& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, uint64_t& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, int16_t& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, int32_t& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, int64_t& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, float& v_);
const uint8_t* binary_read_le_g(const uint8_t* p_, double& v_);


const uint8_t* binary_read_g(const uint8_t* p_, uint8_t& v_);
const uint8_t* binary_read_g(const uint8_t* p_, int8_t& v_);


// Write function declarations
uint8_t* binary_write_be_g(uint8_t* p_, uint16_t v_);
uint8_t* binary_write_be_g(uint8_t* p_, uint32_t v_);
uint8_t* binary_write_be_g(uint8_t* p_, uint64_t v_);
uint8_t* binary_write_be_g(uint8_t* p_, int16_t v_);
uint8_t* binary_write_be_g(uint8_t* p_, int32_t v_);
uint8_t* binary_write_be_g(uint8_t* p_, int64_t v_);
uint8_t* binary_write_be_g(uint8_t* p_, float v_);
uint8_t* binary_write_be_g(uint8_t* p_, double v_);

uint8_t* binary_write_le_g(uint8_t* p_, uint16_t v_);
uint8_t* binary_write_le_g(uint8_t* p_, uint32_t v_);
uint8_t* binary_write_le_g(uint8_t* p_, uint64_t v_);
uint8_t* binary_write_le_g(uint8_t* p_, int16_t v_);
uint8_t* binary_write_le_g(uint8_t* p_, int32_t v_);
uint8_t* binary_write_le_g(uint8_t* p_, int64_t v_);
uint8_t* binary_write_le_g(uint8_t* p_, float v_);
uint8_t* binary_write_le_g(uint8_t* p_, double v_);

uint8_t* binary_write_g(uint8_t* p_, uint8_t v_);
uint8_t* binary_write_g(uint8_t* p_, int8_t v_);


namespace binary {
   /**
    * @brief Read big-endian byte order items from buffer.
    */
   struct read_be {
      const uint8_t* m_puPosition;
      const uint8_t* m_puBegin;
      const uint8_t* m_puEnd;
      
      read_be(const uint8_t* puBegin, const uint8_t* puEnd) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puEnd) {}
      
      read_be(const uint8_t* puBegin, size_t uSize) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puBegin + uSize) {}
   };
   
   /**
    * @brief Read little-endian byte order items from buffer.
    */
   struct read_le {
      const uint8_t* m_puPosition;
      const uint8_t* m_puBegin;
      const uint8_t* m_puEnd;
      
      read_le(const uint8_t* puBegin, const uint8_t* puEnd) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puEnd) {}
      
      read_le(const uint8_t* puBegin, size_t uSize) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puBegin + uSize) {}
   };
   
   /**
    * @brief Write big-endian byte order items to buffer.
    */
   struct write_be {
      uint8_t* m_puPosition;
      uint8_t* m_puBegin;
      uint8_t* m_puEnd;
      
      write_be(uint8_t* puBegin, uint8_t* puEnd) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puEnd) {}
      
      write_be(uint8_t* puBegin, size_t uSize) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puBegin + uSize) {}
   };
   
   /**
    * @brief Write little-endian byte order items to buffer.
    */
   struct write_le {
      uint8_t* m_puPosition;
      uint8_t* m_puBegin;
      uint8_t* m_puEnd;
      
      write_le(uint8_t* puBegin, uint8_t* puEnd) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puEnd) {}
      
      write_le(uint8_t* puBegin, size_t uSize) 
         : m_puPosition(puBegin), m_puBegin(puBegin), m_puEnd(puBegin + uSize) {}
   };
   
   // Stream operators for read_be
   inline read_be& operator>>(read_be& stream, uint16_t& v_) {
      stream.m_puPosition = binary_read_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, uint32_t& v_) {
      stream.m_puPosition = binary_read_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, uint64_t& v_) {
      stream.m_puPosition = binary_read_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, int16_t& v_) {
      stream.m_puPosition = binary_read_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, int32_t& v_) {
      stream.m_puPosition = binary_read_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, int64_t& v_) {
      stream.m_puPosition = binary_read_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, uint8_t& v_) {
      stream.m_puPosition = binary_read_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_be& operator>>(read_be& stream, int8_t& v_) {
      stream.m_puPosition = binary_read_g(stream.m_puPosition, v_);
      return stream;
   }
   
   // Stream operators for read_le
   inline read_le& operator>>(read_le& stream, uint16_t& v_) {
      stream.m_puPosition = binary_read_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, uint32_t& v_) {
      stream.m_puPosition = binary_read_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, uint64_t& v_) {
      stream.m_puPosition = binary_read_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, int16_t& v_) {
      stream.m_puPosition = binary_read_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, int32_t& v_) {
      stream.m_puPosition = binary_read_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, int64_t& v_) {
      stream.m_puPosition = binary_read_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, uint8_t& v_) {
      stream.m_puPosition = binary_read_g(stream.m_puPosition, v_);
      return stream;
   }
   inline read_le& operator>>(read_le& stream, int8_t& v_) {
      stream.m_puPosition = binary_read_g(stream.m_puPosition, v_);
      return stream;
   }
   
   // Stream operators for write_be
   inline write_be& operator<<(write_be& stream, uint16_t v_) {
      stream.m_puPosition = binary_write_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, uint32_t v_) {
      stream.m_puPosition = binary_write_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, uint64_t v_) {
      stream.m_puPosition = binary_write_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, int16_t v_) {
      stream.m_puPosition = binary_write_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, int32_t v_) {
      stream.m_puPosition = binary_write_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, int64_t v_) {
      stream.m_puPosition = binary_write_be_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, uint8_t v_) {
      stream.m_puPosition = binary_write_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_be& operator<<(write_be& stream, int8_t v_) {
      stream.m_puPosition = binary_write_g(stream.m_puPosition, v_);
      return stream;
   }
   
   // Stream operators for write_le
   inline write_le& operator<<(write_le& stream, uint16_t v_) {
      stream.m_puPosition = binary_write_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, uint32_t v_) {
      stream.m_puPosition = binary_write_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, uint64_t v_) {
      stream.m_puPosition = binary_write_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, int16_t v_) {
      stream.m_puPosition = binary_write_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, int32_t v_) {
      stream.m_puPosition = binary_write_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, int64_t v_) {
      stream.m_puPosition = binary_write_le_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, uint8_t v_) {
      stream.m_puPosition = binary_write_g(stream.m_puPosition, v_);
      return stream;
   }
   inline write_le& operator<<(write_le& stream, int8_t v_) {
      stream.m_puPosition = binary_write_g(stream.m_puPosition, v_);
      return stream;
   }
} // namespace binary



_GD_END
