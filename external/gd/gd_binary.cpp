// @FILE [tag: binary] [description: Handle binary data] [type: source] [name: gd_binary.cpp]

#include <cstring>

#include "gd_binary.h"



#if defined(__GNUC__) || defined(__clang__)
// GCC/Clang builtins (work on x86, x86-64, ARM, AArch64, etc.)
#define SWAP16(x) __builtin_bswap16(x)
#define SWAP32(x) __builtin_bswap32(x)
#define SWAP64(x) __builtin_bswap64(x)

#elif defined(_MSC_VER)
// MSVC intrinsics
#include <intrin.h>
#pragma intrinsic(_byteswap_ushort, _byteswap_ulong, _byteswap_uint64)
#define SWAP16(x) _byteswap_ushort(x)
#define SWAP32(x) _byteswap_ulong(x)
#define SWAP64(x) _byteswap_uint64(x)

#else
// Fully portable fallbacks (work everywhere, no undefined behavior)
#define SWAP16(x) ((uint16_t)(( (uint16_t)(x) << 8) | ( (uint16_t)(x) >> 8)))
#define SWAP32(x) ((uint32_t)( \
    (((uint32_t)(x) & 0x000000FFu) << 24) | \
    (((uint32_t)(x) & 0x0000FF00u) << 8)  | \
    (((uint32_t)(x) & 0x00FF0000u) >> 8)  | \
    (((uint32_t)(x) & 0xFF000000u) >> 24) ))
#define SWAP64(x) ((uint64_t)( \
    (((uint64_t)(x) & 0x00000000000000FFull) << 56) | \
    (((uint64_t)(x) & 0x000000000000FF00ull) << 40) | \
    (((uint64_t)(x) & 0x0000000000FF0000ull) << 24) | \
    (((uint64_t)(x) & 0x00000000FF000000ull) << 8)  | \
    (((uint64_t)(x) & 0x000000FF00000000ull) >> 8)  | \
    (((uint64_t)(x) & 0x0000FF0000000000ull) >> 24) | \
    (((uint64_t)(x) & 0x00FF000000000000ull) >> 40) | \
    (((uint64_t)(x) & 0xFF00000000000000ull) >> 56) ))
#endif

// Detect host system endianness at compile time
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
   #define GD_HOST_BIG_ENDIAN 1
#elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   #define GD_HOST_BIG_ENDIAN 0
#elif defined(_WIN32) || defined(__i386__) || defined(__x86_64__) || defined(__arm__) || defined(__aarch64__)
    // Windows, x86, x64, ARM, and AArch64 are little-endian
   #define GD_HOST_BIG_ENDIAN 0
#else
   #error "Cannot determine system endianness"
#endif

_GD_BEGIN

namespace
{
/** -------------------------------------------------------------------------- puHexValue_s
 * @brief Lookup table used to convert hexadecimal value in text to
 * value stored in byte.
 * It takes the ascii code for character and use the ascii number to find
 * what hex value it is in this table.
*/
static const uint8_t puHexValue_s[0x100] =
{
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x20-0x2F */
   0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, /* 0x30-0x3F (0 - 9) */
   0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F (A - F) */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
   0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F (a - f) */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xC0-0xCF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xD0-0xDF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xE0-0xEF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
};

/** -------------------------------------------------------------------------- puHexText_s
 * @brief Lookup table containing hexadecimal string representations of all byte values (0x00-0xFF).
 *        
 * Each byte is represented by exactly two lowercase hex digits. Total size = 512 bytes. 
 */
static const char puHexText_s[512] =
{
    '0','0', '0','1', '0','2', '0','3', '0','4', '0','5', '0','6', '0','7', '0','8', '0','9', '0','a', '0','b', '0','c', '0','d', '0','e', '0','f',
    '1','0', '1','1', '1','2', '1','3', '1','4', '1','5', '1','6', '1','7', '1','8', '1','9', '1','a', '1','b', '1','c', '1','d', '1','e', '1','f',
    '2','0', '2','1', '2','2', '2','3', '2','4', '2','5', '2','6', '2','7', '2','8', '2','9', '2','a', '2','b', '2','c', '2','d', '2','e', '2','f',
    '3','0', '3','1', '3','2', '3','3', '3','4', '3','5', '3','6', '3','7', '3','8', '3','9', '3','a', '3','b', '3','c', '3','d', '3','e', '3','f',
    '4','0', '4','1', '4','2', '4','3', '4','4', '4','5', '4','6', '4','7', '4','8', '4','9', '4','a', '4','b', '4','c', '4','d', '4','e', '4','f',
    '5','0', '5','1', '5','2', '5','3', '5','4', '5','5', '5','6', '5','7', '5','8', '5','9', '5','a', '5','b', '5','c', '5','d', '5','e', '5','f',
    '6','0', '6','1', '6','2', '6','3', '6','4', '6','5', '6','6', '6','7', '6','8', '6','9', '6','a', '6','b', '6','c', '6','d', '6','e', '6','f',
    '7','0', '7','1', '7','2', '7','3', '7','4', '7','5', '7','6', '7','7', '7','8', '7','9', '7','a', '7','b', '7','c', '7','d', '7','e', '7','f',
    '8','0', '8','1', '8','2', '8','3', '8','4', '8','5', '8','6', '8','7', '8','8', '8','9', '8','a', '8','b', '8','c', '8','d', '8','e', '8','f',
    '9','0', '9','1', '9','2', '9','3', '9','4', '9','5', '9','6', '9','7', '9','8', '9','9', '9','a', '9','b', '9','c', '9','d', '9','e', '9','f',
    'a','0', 'a','1', 'a','2', 'a','3', 'a','4', 'a','5', 'a','6', 'a','7', 'a','8', 'a','9', 'a','a', 'a','b', 'a','c', 'a','d', 'a','e', 'a','f',
    'b','0', 'b','1', 'b','2', 'b','3', 'b','4', 'b','5', 'b','6', 'b','7', 'b','8', 'b','9', 'b','a', 'b','b', 'b','c', 'b','d', 'b','e', 'b','f',
    'c','0', 'c','1', 'c','2', 'c','3', 'c','4', 'c','5', 'c','6', 'c','7', 'c','8', 'c','9', 'c','a', 'c','b', 'c','c', 'c','d', 'c','e', 'c','f',
    'd','0', 'd','1', 'd','2', 'd','3', 'd','4', 'd','5', 'd','6', 'd','7', 'd','8', 'd','9', 'd','a', 'd','b', 'd','c', 'd','d', 'd','e', 'd','f',
    'e','0', 'e','1', 'e','2', 'e','3', 'e','4', 'e','5', 'e','6', 'e','7', 'e','8', 'e','9', 'e','a', 'e','b', 'e','c', 'e','d', 'e','e', 'e','f',
    'f','0', 'f','1', 'f','2', 'f','3', 'f','4', 'f','5', 'f','6', 'f','7', 'f','8', 'f','9', 'f','a', 'f','b', 'f','c', 'f','d', 'f','e', 'f','f'
};

} // namespace

/// @API [tag: binary, hex] [description: Hexadecimal related logic, convert from or to hexadecimal, validation etc]

/// @brief Access lookup table used to convert hexadecimal value in text to value stored in byte.
const uint8_t* binary_get_hex_table_g() noexcept
{
    return puHexValue_s;
}

/// @brief Access lookup table containing hexadecimal string representations of all byte values (0x00-0xFF).
const char* binary_get_hex_text_table_g() noexcept
{
   return puHexText_s;
}

/** -------------------------------------------------------------------------- binary_validate_hex_g
 * @brief Validate hex string, returns pair of ( is valid, error message )
 *
 * Checks if the provided string contains valid hexadecimal characters (0-9, a-f, A-F).
 * Empty strings are considered invalid.
 *
 * @param stringHex String view containing the hex string to validate
 * @return std::pair<bool, std::string> Pair containing validation result and error message if invalid
 */
std::pair<bool, std::string> binary_validate_hex_g( std::string_view stringHex )
{
   if( stringHex.empty() ) return { false, "Hex string cannot be empty" };

   if( stringHex.length() % 2 != 0 ) return { false, "Hex string must have an even number of characters" };

   // ## Check each character for valid hex values using lookup table
   for( size_t uIndex = 0; uIndex < stringHex.length(); ++uIndex )
   {
      if( puHexValue_s[(uint8_t)stringHex[uIndex]] == 0 && stringHex[uIndex] != '0' ) return {false, std::string("Invalid hex character at position ") + std::to_string(uIndex) + ": '" + stringHex[uIndex] + "'"};
   }

   return { true, "" };
}

/** -------------------------------------------------------------------------- binary_validate_uuid_g
 * @brief Validate uuid string, returns pair of ( is valid, error message )
 *
 * Checks if the provided string is a valid UUID format (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).
 * Uses case-insensitive hexadecimal validation with hyphens in correct positions.
 *
 * @param stringUuid String view containing the UUID string to validate
 * @return std::pair<bool, std::string> Pair containing validation result and error message if invalid
 */
std::pair<bool, std::string> binary_validate_uuid_g( std::string_view stringUuid )
{
   // UUID should be exactly 36 characters (32 hex + 4 hyphens)
   if( stringUuid.length() != 36 ) return { false, "UUID must be exactly 36 characters long (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)" };

   // ## Check for hyphens at correct positions
   if( stringUuid[8] != '-' || stringUuid[13] != '-' || stringUuid[18] != '-' || stringUuid[23] != '-' ) return { false, "UUID must contain hyphens at positions 8, 13, 18, and 23" };

   // ## Check each character for valid hex values or hyphens using lookup table
   for( size_t uIndex = 0; uIndex < stringUuid.length(); ++uIndex )
   {
      if( uIndex == 8 || uIndex == 13 || uIndex == 18 || uIndex == 23 ) continue; // Skip hyphen positions (already validated)

      if( puHexValue_s[(uint8_t)stringUuid[uIndex]] == 0 && stringUuid[uIndex] != '0' ) return {false, std::string("Invalid UUID hex character at position ") + std::to_string(uIndex) + ": '" + stringUuid[uIndex] + "'"};
   }

   return { true, "" };
}

/** -------------------------------------------------------------------------- binary_copy_hex_g (unsafe version)
 * @brief Copy hex string to binary buffer, buffer must be large enough to hold the data
 *
 * Converts a hexadecimal string to its binary representation. This version assumes
 * the output buffer is large enough to hold the data (buffer_size >= hex_string_length/2).
 *
 * @param puBuffer Output buffer to store binary data
 * @param stringHex String view containing the hex string to convert
 */
void binary_copy_hex_g( uint8_t* puBuffer, std::string_view stringHex )
{                                                                                                  assert(puBuffer != nullptr); assert(stringHex.length() % 2 == 0);
   size_t uHexLength = stringHex.length(); // Get the length of the hex string

   // Process hex string two characters at a time using lookup table
   for( size_t uIndex = 0; uIndex < uHexLength; uIndex += 2 )
   {
      puBuffer[uIndex / 2] = ( puHexValue_s[(uint8_t)stringHex[uIndex]] << 4 ) | puHexValue_s[(uint8_t)stringHex[uIndex + 1]];
   }
}

/** -------------------------------------------------------------------------- binary_copy_uuid_g
 * @brief Copy uuid string to binary buffer, buffer must be large enough to hold the data
 *
 * Converts a UUID string to its binary representation. This version assumes
 * the output buffer is large enough to hold the data (16 bytes).
 *
 * @param puBuffer Output buffer to store binary data
 * @param stringUuid String view containing the UUID string to convert
 */
void binary_copy_uuid_g( uint8_t* puBuffer, std::string_view stringUuid )
{
   size_t uUuidLength = stringUuid.length();
   size_t uBufferIndex = 0;

   // Process UUID string using lookup table
   for( size_t uIndex = 0; uIndex < uUuidLength; )
   {

      if( stringUuid[uIndex] == '-' ) { uIndex++; continue; }                 // Skip hyphens

      // Process two hex characters using lookup table
      puBuffer[uBufferIndex++] = ( puHexValue_s[(uint8_t)stringUuid[uIndex]] << 4 ) | puHexValue_s[(uint8_t)stringUuid[uIndex + 1]];
      uIndex += 2;
   }
}

/** -------------------------------------------------------------------------- binary_copy_hex_g (safe version)
 * @brief Copy hex string to binary buffer, returns number of bytes copied and only copies up to uBufferSize bytes
 *
 * Converts a hexadecimal string to its binary representation. This version is safe
 * and will only copy up to uBufferSize bytes. Returns the actual number of bytes copied.
 *
 * @param puBuffer Output buffer to store binary data
 * @param uBufferSize Size of the output buffer in bytes
 * @param stringHex String view containing the hex string to convert
 * @return size_t Number of bytes actually copied to the buffer
 */
size_t binary_copy_hex_g( uint8_t* puBuffer, size_t uBufferSize, std::string_view stringHex )
{
   size_t uHexLength = stringHex.length(); // Get the length of the hex string
   size_t uMaxBytes = ( uHexLength / 2 ); // Maximum bytes that can be represented by the hex string
   size_t uBytesToCopy = ( uBufferSize < uMaxBytes ) ? uBufferSize : uMaxBytes; // Determine how many bytes to copy

   // ## Process hex string two characters at a time using lookup table
   for( size_t uIndex = 0; uIndex < uBytesToCopy * 2; uIndex += 2 )
   {
      puBuffer[uIndex / 2] = ( puHexValue_s[(uint8_t)stringHex[uIndex]] << 4 ) | puHexValue_s[(uint8_t)stringHex[uIndex + 1]];
   }

   return uBytesToCopy;
}

/** -------------------------------------------------------------------------- binary_to_hex_g
 * @brief Convert binary data to hexadecimal string and append it to `stringHex`
 * 
 * The existing content of `stringHex` is preserved. The hexadecimal representation 
 * of the input buffer is appended to the string.
 *
 * @param puBuffer    Input buffer containing binary data
 * @param uBufferSize Size of the input buffer
 * @param stringHex   Output string (existing content is kept, hex is appended)
 * @param bUppercase  Use uppercase hex letters (A-F) if true
 */
void binary_to_hex_g( const uint8_t* puBuffer, size_t uBufferSize, std::string& stringHex, bool bUppercase )
{
   const size_t uOldSize = stringHex.size();
   const size_t uOutSize = uOldSize + uBufferSize * 2;
   
   stringHex.resize( uOutSize );                                              // append space + exact final size and set ending null terminator (if needed)

   char* piHex = stringHex.data() + uOldSize;                                 // start writing at the end
   if( bUppercase == false )
   {
      // ## lowercase using small 16-byte table + arithmetic (faster & better for compiler)
      static const char* piHexLower = "0123456789abcdef";

      for( size_t u = 0; u < uBufferSize; ++u )
      {
         const uint8_t uByte = puBuffer[u];
         piHex[0] = piHexLower[uByte >> 4];
         piHex[1] = piHexLower[uByte & 0x0F];
         piHex += 2;
      }
   }
   else
   {
      // ## Uppercase path using small 16-byte table + arithmetic (faster & better for compiler)
      static const char* piHexUpper = "0123456789ABCDEF";

      for( size_t u = 0; u < uBufferSize; ++u )
      {
         const uint8_t uByte = puBuffer[u];
         piHex[0] = piHexUpper[uByte >> 4];
         piHex[1] = piHexUpper[uByte & 0x0F];
         piHex += 2;
      }
   }
}


/// @overload binary_to_hex_g
std::string binary_to_hex_g( const uint8_t* puBuffer, size_t uBufferSize, bool bUppercase )
{
   std::string stringHex;
   binary_to_hex_g( puBuffer, uBufferSize, stringHex, bUppercase );
   return stringHex;
}

/** -------------------------------------------------------------------------- buffer_find_g
 * @brief Find pattern in buffer
 *
 * Searches for the first occurrence of a pattern within a buffer starting from a specified index.
 * Uses an efficient algorithm for pattern matching.
 *
 * @param pbBuffer Buffer to search in
 * @param uBufferSize Size of the buffer
 * @param puPattern Pattern to search for
 * @param uPatternSize Size of the pattern
 * @param uOffset Starting position for search (default: 0)
 * @return int64_t Position of first occurrence or -1 if not found
 */
int64_t buffer_find_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize, size_t uOffset )
{
   // Edge cases
   if( uPatternSize == 0 ) return uOffset;                                    // Empty pattern matches at start index

   if( uPatternSize > uBufferSize || uOffset >= uBufferSize ) return -1;      // Pattern larger than buffer or start index out of bounds

   if( uOffset + uPatternSize > uBufferSize ) return -1;                      // Ensure we don't start beyond the point where pattern could fit

   const uint8_t* puSearchEnd = puBuffer + uBufferSize - uPatternSize; // Last possible position for pattern to fit
   const uint8_t* puCurrent = puBuffer + uOffset; // Current position

   // ## Use Boyer-Moore-like approach for efficiency
   while( puCurrent <= puSearchEnd )
   {
      // Check for pattern match
      if( memcmp( puCurrent, puPattern, uPatternSize ) == 0 ) return static_cast<int64_t>( puCurrent - puBuffer );

      puCurrent++;                                                           // Move to next position
   }

   return -1;  // Pattern not found
}


/** -------------------------------------------------------------------------- buffer_find_last_g
 * @brief Find last occurrence of pattern in buffer
 * 
 * @param puBuffer Buffer to search in
 * @param uBufferSize Size of the buffer
 * @param puPattern Pattern to search for
 * @param uPatternSize Size of the pattern
 * @return int64_t Position of last occurrence or -1 if not found
 */
int64_t buffer_find_last_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize )
{
   if( uPatternSize == 0 || uPatternSize > uBufferSize ) return -1;
   
   // Search backwards
   for( int64_t i = uBufferSize - uPatternSize; i >= 0; --i )
   {
      if( memcmp( puBuffer + i, puPattern, uPatternSize ) == 0 )
         return i;
   }
   
   return -1;
}

// Your SWAP16, SWAP32, SWAP64 macros/intrinsics here
// (as previously defined with GCC/Clang/MSVC support)

// ## Big-endian global readers

/// @brief Read big-endian 16 values from binary data ------------------------
const uint8_t* binary_read_be_g(const uint8_t* p_, uint16_t& v_)
{
    std::memcpy(&v_, p_, sizeof(v_));
#if !GD_HOST_BIG_ENDIAN
    v_ = SWAP16(v_);
#endif
    return p_ + sizeof(v_);
}

/// @brief Read big-endian 32 values from binary data ------------------------
const uint8_t* binary_read_be_g(const uint8_t* p_, uint32_t& v_)
{
    std::memcpy(&v_, p_, sizeof(v_));
#if !GD_HOST_BIG_ENDIAN
    v_ = SWAP32(v_);
#endif
    return p_ + sizeof(v_);
}

/// @brief Read big-endian 64 values from binary data ------------------------
const uint8_t* binary_read_be_g(const uint8_t* p_, uint64_t& v_)
{
    std::memcpy(&v_, p_, sizeof(v_));
#if !GD_HOST_BIG_ENDIAN
    v_ = SWAP64(v_);
#endif
    return p_ + sizeof(v_);
}

const uint8_t* binary_read_be_g(const uint8_t* p_, int16_t& v_)
{
    uint16_t uValue;
    p_ = binary_read_be_g(p_, uValue);
    v_ = static_cast<int16_t>(uValue);
    return p_;
}

const uint8_t* binary_read_be_g(const uint8_t* p_, int32_t& v_)
{
    uint32_t uValue;
    p_ = binary_read_be_g(p_, uValue);
    v_ = static_cast<int32_t>(uValue);
    return p_;
}

const uint8_t* binary_read_be_g(const uint8_t* p_, int64_t& v_)
{
    uint64_t uValue;
    p_ = binary_read_be_g(p_, uValue);
    v_ = static_cast<int64_t>(uValue);
    return p_;
}

const uint8_t* binary_read_be_g(const uint8_t* p_, float& v_)
{
    uint32_t uValue;
    p_ = binary_read_be_g(p_, uValue);
    v_ = static_cast<float>(uValue);
    return p_;
}

const uint8_t* binary_read_be_g(const uint8_t* p_, double& v_)
{
    uint64_t uValue;
    p_ = binary_read_be_g(p_, uValue);
    v_ = static_cast<double>(uValue);
    return p_;
}

// ## Little-endian global readers

/// @brief Read little-endian 16 values from binary data ---------------------
const uint8_t* binary_read_le_g(const uint8_t* p_, uint16_t& v_)
{
    std::memcpy(&v_, p_, sizeof(v_));
#if GD_HOST_BIG_ENDIAN
    v_ = SWAP16(v_);
#endif
    return p_ + sizeof(v_);
}

/// @brief Read little-endian 32 values from binary data
const uint8_t* binary_read_le_g(const uint8_t* p_, uint32_t& v_)
{
    std::memcpy(&v_, p_, sizeof(v_));
#if GD_HOST_BIG_ENDIAN
    v_ = SWAP32(v_);
#endif
    return p_ + sizeof(v_);
}

/// @brief Read little-endian 64 values from binary data
const uint8_t* binary_read_le_g(const uint8_t* p_, uint64_t& v_)
{
    std::memcpy(&v_, p_, sizeof(v_));
#if GD_HOST_BIG_ENDIAN
    v_ = SWAP64(v_);
#endif
    return p_ + sizeof(v_);
}

const uint8_t* binary_read_le_g(const uint8_t* p_, int16_t& v_)
{
    uint16_t uValue;
    p_ = binary_read_le_g(p_, uValue);
    v_ = static_cast<int16_t>(uValue);
    return p_;
}

const uint8_t* binary_read_le_g(const uint8_t* p_, int32_t& v_)
{
    uint32_t uValue;
    p_ = binary_read_le_g(p_, uValue);
    v_ = static_cast<int32_t>(uValue);
    return p_;
}

const uint8_t* binary_read_le_g(const uint8_t* p_, int64_t& v_)
{
    uint64_t uValue;
    p_ = binary_read_le_g(p_, uValue);
    v_ = static_cast<int64_t>(uValue);
    return p_;
}

const uint8_t* binary_read_le_g(const uint8_t* p_, float& v_)
{
    uint32_t uValue;
    p_ = binary_read_le_g(p_, uValue);
    v_ = static_cast<float>(uValue);
    return p_;
}

const uint8_t* binary_read_le_g(const uint8_t* p_, double& v_)
{
    uint64_t uValue;
    p_ = binary_read_le_g(p_, uValue);
    v_ = static_cast<double>(uValue);
    return p_;
}

const uint8_t* binary_read_g(const uint8_t* p_, uint8_t& v_)
{
    v_ = static_cast<uint8_t>(*p_);
    return p_ + 1;
}


const uint8_t* binary_read_g(const uint8_t* p_, int8_t& v_)
{
    v_ = static_cast<int8_t>(*p_);
    return p_ + 1;
}


// ## Big-endian global writers

/// @brief Write big-endian 16-bit value to binary data
uint8_t* binary_write_be_g(uint8_t* p_, uint16_t v_)
{
#if !GD_HOST_BIG_ENDIAN
    v_ = SWAP16(v_);
#endif
    std::memcpy(p_, &v_, sizeof(v_));
    return p_ + sizeof(v_);
}

/// @brief Write big-endian 32-bit value to binary data
uint8_t* binary_write_be_g(uint8_t* p_, uint32_t v_)
{
#if !GD_HOST_BIG_ENDIAN
    v_ = SWAP32(v_);
#endif
    std::memcpy(p_, &v_, sizeof(v_));
    return p_ + sizeof(v_);
}

/// @brief Write big-endian 64-bit value to binary data
uint8_t* binary_write_be_g(uint8_t* p_, uint64_t v_)
{
#if !GD_HOST_BIG_ENDIAN
    v_ = SWAP64(v_);
#endif
    std::memcpy(p_, &v_, sizeof(v_));
    return p_ + sizeof(v_);
}

uint8_t* binary_write_be_g(uint8_t* p_, int16_t v_)
{
    return binary_write_be_g(p_, static_cast<uint16_t>(v_));
}

uint8_t* binary_write_be_g(uint8_t* p_, int32_t v_)
{
    return binary_write_be_g(p_, static_cast<uint32_t>(v_));
}

uint8_t* binary_write_be_g(uint8_t* p_, int64_t v_)
{
    return binary_write_be_g(p_, static_cast<uint64_t>(v_));
}

uint8_t* binary_write_be_g(uint8_t* p_, float v_)
{
    uint32_t uValue;
    std::memcpy(&uValue, &v_, sizeof(v_));
    return binary_write_be_g(p_, uValue);
}

uint8_t* binary_write_be_g(uint8_t* p_, double v_)
{
    uint64_t uValue;
    std::memcpy(&uValue, &v_, sizeof(v_));
    return binary_write_be_g(p_, uValue);
}

// ## Little-endian global writers

/// @brief Write little-endian 16-bit value to binary data
uint8_t* binary_write_le_g(uint8_t* p_, uint16_t v_)
{
#if GD_HOST_BIG_ENDIAN
    v_ = SWAP16(v_);
#endif
    std::memcpy(p_, &v_, sizeof(v_));
    return p_ + sizeof(v_);
}

/// @brief Write little-endian 32-bit value to binary data
uint8_t* binary_write_le_g(uint8_t* p_, uint32_t v_)
{
#if GD_HOST_BIG_ENDIAN
    v_ = SWAP32(v_);
#endif
    std::memcpy(p_, &v_, sizeof(v_));
    return p_ + sizeof(v_);
}

/// @brief Write little-endian 64-bit value to binary data
uint8_t* binary_write_le_g(uint8_t* p_, uint64_t v_)
{
#if GD_HOST_BIG_ENDIAN
    v_ = SWAP64(v_);
#endif
    std::memcpy(p_, &v_, sizeof(v_));
    return p_ + sizeof(v_);
}

uint8_t* binary_write_le_g(uint8_t* p_, int16_t v_)
{
    return binary_write_le_g(p_, static_cast<uint16_t>(v_));
}

uint8_t* binary_write_le_g(uint8_t* p_, int32_t v_)
{
    return binary_write_le_g(p_, static_cast<uint32_t>(v_));
}

uint8_t* binary_write_le_g(uint8_t* p_, int64_t v_)
{
    return binary_write_le_g(p_, static_cast<uint64_t>(v_));
}

uint8_t* binary_write_le_g(uint8_t* p_, float v_)
{
    uint32_t uValue;
    std::memcpy(&uValue, &v_, sizeof(v_));
    return binary_write_le_g(p_, uValue);
}

uint8_t* binary_write_le_g(uint8_t* p_, double v_)
{
    uint64_t uValue;
    std::memcpy(&uValue, &v_, sizeof(v_));
    return binary_write_le_g(p_, uValue);
}

// ## 8-bit global writers (endianness irrelevant)

uint8_t* binary_write_g(uint8_t* p_, uint8_t v_)
{
    *p_ = v_;
    return p_ + 1;
}

uint8_t* binary_write_g(uint8_t* p_, int8_t v_)
{
    *p_ = static_cast<uint8_t>(v_);
    return p_ + 1;
}


_GD_END
