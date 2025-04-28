/*
* @brief translate from different text formats
* 
*/


#pragma once
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>
#include <cstring>


#ifndef _GD_TRANSLATE_BEGIN
#define _GD_TRANSLATE_BEGIN namespace gd { namespace translate {
#define _GD_TRANSLATE_END } }
#endif


_GD_TRANSLATE_BEGIN

/// tag dispatcher used for string operations
struct tag_string {};

/// tag dispatcher used for vector operations
struct tag_vector {};


/// Calculate needed size to store bytes as base64
/// This calculates length and needed padding
/// Each 3 byte section is converted to 4 base64 digits, it also adds ending with 3 and zero out last two bits 
/// Sample for one character: 4 * 1 / 3 + 3 = 4, next step will zero out the last two bits (already 0 here) 
/// Sample for two characters: 4 * 2 / 3 + 3 = 5, next step will zero out the last two bits and final result is 4
/// Sample for three characters: 4 * 3 / 3 + 3 = 7, next step will zero out the last two bits and final result is 4
/// Sample for four characters: 4 * 4 / 3 + 3 = 8, next step will zero out the last two bits (already 0 here), final result is 8
inline size_t base64_size_g( uint64_t uByteLength ) noexcept { return ((4 * uByteLength / 3) + 3) & ~3; }

/// Validate base64 stream
bool base64_validate_g( const uint8_t* puStream, size_t uLength, uint8_t const** ppuPosition );
inline bool base64_validate_g( const uint8_t* puStream, size_t uLength ) { return base64_validate_g( puStream, uLength, nullptr ); }
inline bool base64_validate_g( const char* puStream ) { return base64_validate_g( (const uint8_t*)puStream, strlen( puStream ), nullptr); }

/// Convert bytes to base64
size_t base64_encode_g( const uint8_t* puStream, size_t uLength, uint8_t* puBuffer );
inline size_t base64_encode_g( const char* puStream, uint8_t* puBuffer ) { return base64_encode_g( (const uint8_t*)puStream, strlen( puStream ), puBuffer); }
size_t base64_encode_g( const char* puStream, size_t uLength, std::vector<char>& vectorBase64 );
/// Wrapper method to return base64 digits in vector
std::vector<char> base64_encode_g( const char* puStream, size_t uLength, tag_vector );
inline std::vector<char> base64_encode_g( const char* puStream, tag_vector ) { return base64_encode_g( puStream, strlen(puStream), tag_vector{} ); }
/// Wrapper method to return base64 digits in string
std::string base64_encode_g( const char* puStream, size_t uLength, tag_string );
inline std::string base64_encode_g( const char* puStream, tag_string ) { return base64_encode_g( puStream, strlen(puStream), tag_string{} ); }
inline std::string base64_encode_g( const std::string& stringStream, tag_string ) { return base64_encode_g( stringStream.c_str(), stringStream.length(), tag_string{} ); }


size_t base64_decode_g( const uint8_t* puStream, size_t uLength, uint8_t* puBuffer );
inline size_t base64_decode_g( const uint8_t* puStream, uint8_t* puBuffer ) { return base64_decode_g( puStream, strlen( (const char*)puStream ), puBuffer ); }
size_t base64_decode_g( const char* puStream, size_t uLength, std::vector<uint8_t>& vectorBase64 );
std::vector<uint8_t> base64_decode_g( const char* puStream, size_t uLength, tag_vector );
std::string base64_decode_g( const char* puStream, size_t uLength, tag_string );
inline std::string base64_decode_g( const std::string& stringStream, tag_string ) { return base64_decode_g( stringStream.c_str(), stringStream.length(), tag_string{} ); }






_GD_TRANSLATE_END