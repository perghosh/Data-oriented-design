// @FILE [tag: binary] [description: Handle binary data] [type: source] [name: gd_binary.cpp]

#include "gd_binary.h"

_GD_BEGIN

namespace
{
/**
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
} // namespace

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

      if( puHexValue_s[(uint8_t)stringUuid[uIndex]] == 0 ) return { false, std::string( "Invalid UUID hex character at position " ) + std::to_string( uIndex ) + ": '" + stringUuid[uIndex] + "'" };
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
{
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
 * @brief Convert binary data to hexadecimal string
 * 
 * @param puBuffer Input buffer containing binary data
 * @param uBufferSize Size of the input buffer
 * @param bUppercase Use uppercase hex letters (default: false)
 * @return std::string Hexadecimal representation
 */
void binary_to_hex_g( const uint8_t* puBuffer, size_t uBufferSize, std::string& stringHex, bool bUppercase )
{
   static const char* piHexLower = "0123456789abcdef";
   static const char* piHexUpper = "0123456789ABCDEF";
   const char* piHex = bUppercase ? piHexUpper : piHexLower;
   
   for( size_t u = 0; u < uBufferSize; ++u )
   {
      stringHex += piHex[puBuffer[u] >> 4];
      stringHex += piHex[puBuffer[u] & 0x0F];
   }
}

/// @overload binary_to_hex_g
std::string binary_to_hex_g( const uint8_t* puBuffer, size_t uBufferSize, bool bUppercase )
{
   std::string stringHex;
   stringHex.reserve( uBufferSize * 2 );
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
 * @param uStartIndex Starting position for search (default: 0)
 * @return int64_t Position of first occurrence or -1 if not found
 */
int64_t buffer_find_g( const uint8_t* puBuffer, size_t uBufferSize, const uint8_t* puPattern, size_t uPatternSize, size_t uStartIndex )
{
   // Edge cases
   if( uPatternSize == 0 ) return uStartIndex;                               // Empty pattern matches at start index

   if( uPatternSize > uBufferSize || uStartIndex >= uBufferSize ) return -1; // Pattern larger than buffer or start index out of bounds

   if( uStartIndex + uPatternSize > uBufferSize ) return -1;                 // Ensure we don't start beyond the point where pattern could fit

   const uint8_t* puSearchEnd = puBuffer + uBufferSize - uPatternSize; // Last possible position for pattern to fit
   const uint8_t* puCurrent = puBuffer + uStartIndex; // Current position

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

_GD_END
