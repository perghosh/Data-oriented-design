// @FILE [tag: binary] [description: Handle binary data] [type: header] [name: gd_binary.h]

#pragma once

#include <cassert>
#include <cstddef>
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

/// Find pattern in buffer, returns index or -1 if not found
int64_t buffer_find_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize, size_t uStartIndex = 0 );
int64_t buffer_find_last_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize );



_GD_END
