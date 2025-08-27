#include "gd_translate.h"


_GD_TRANSLATE_BEGIN

/** ---------------------------------------------------------------------------
 * @brief Check if pointer is aligned
 * @tparam TYPE primitive type to get align size
 * @param p_ pointer to check if address is aligned
 * @return boolean true if aligned with type or false if not
*/
template<typename TYPE>
inline bool is_aligned( const void* p_) noexcept 
{
   return !(reinterpret_cast<std::uintptr_t>(p_) % alignof(TYPE));
}

/*
template<typename TYPE>
inline const void* find_aligned( const void* p_) noexcept 
{
   auto uMove = reinterpret_cast<std::uintptr_t>(p_) % alignof(TYPE);
   return (void*)((uintptr_t)p_ + (alignof(TYPE) - uMove));
}
*/



/**
 * @brief Lookup table for Base64 character values.
 *
 * The piBase64Value_s array maps each possible byte value (0-255) to its corresponding
 * Base64 digit value (0-63), or -1 if the byte is not a valid Base64 character.
 * This table is used for fast decoding of Base64-encoded data.
 *
 * Base64 is a binary-to-text encoding scheme that represents binary data in an ASCII string format
 * by translating it into a radix-64 representation. It uses 64 different ASCII characters (A-Z, a-z, 0-9, +, /)
 * to encode arbitrary binary data, making it suitable for transmission over text-based protocols.
 * Each group of 3 bytes is encoded as 4 Base64 characters, and padding '=' is used when the input
 * is not a multiple of 3 bytes.
 *
 * Example usage:
 *   - Encoding: Converts binary data to a Base64 string.
 *   - Decoding: Converts a Base64 string back to binary data.
 */
static int8_t constexpr piBase64Value_s[] = {
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x00-0x0F
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x10-0x1F
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, // 0x20-0x2F ( +, / )
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, // 0x30-0x3F (0 - 9)
   -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 0x40-0x4F (? A - O) 65 - 79
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, // 0x50-0x5F
   -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 0x60-0x6F (? a - o)
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 0x70-0x7F
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x80-0x8F
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x90-0x9F
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0xA0-0xAF
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0xB0-0xBF
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0xC0-0xCF
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0xD0-0xDF
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0xE0-0xEF
   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 0xF0-0xFF
};

/// List of all base64 digits in right order based on their value
static const char pchBase64Digit_s[64] = 
{
   'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
   'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
   'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
   'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
   'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
   '8', '9', '+', '/' 
};


// 0	A        26	a       52 0       62 +
// 1	B        27	b       53 1       63 /
// 2	C        28	c       54 2
// 3	D        29	d       55 3
// 4	E        30	e       56 4
// 5	F        31	f       57 5
// 6	G        32	g       58 6
// 7	H        33	h       59 7
// 8	I        34	i       60 8
// 9	J        35	j       61 9
// 10	K        36	k
// 11	L        37	l
// 12	M        38	m
// 13	N        39	n
// 14	O        40	o
// 15	P        41	p
// 16	Q        42	q
// 17	R        43	r
// 18	S        44	s
// 19	T        45	t
// 20	U        46	u
// 21	V        47	v
// 22	W        48	w
// 23	X        49	x
// 24	Y        50	y
// 25	Z        51	z




static uint8_t constexpr uInvalidBit_s = 0b1000'0000;

bool base64_validate_g( const uint8_t* puStream, size_t uLength, uint8_t const** ppuPosition )
{
   const uint8_t* puEnd = puStream + uLength;

   std::size_t uAlign = alignof( uint64_t );

   /*
   if( is_aligned<uint64_t>( puStream ) == false || uLength < (alignof( uint64_t ) * 3)  )
   {
      while(  )
   }
   */

   while( puStream < puEnd )
   {
      if( piBase64Value_s[*puStream] & uInvalidBit_s )
      {
         // ## Check if this is the last padding part using = characters
         while( puStream < puEnd && *puStream == '=' ) puStream++;
         if( puStream < puEnd )
         {
            if( ppuPosition != nullptr ) *ppuPosition = puStream;
            return false;
         }
         return true;
      }
      
      puStream++;
   }

   return true;
}

//    size_t in_len = data.size();            
//    size_t out_len = 4 * ((in_len + 2) / 3);

/** ---------------------------------------------------------------------------
 * @brief convert chars to base64 format
 * The base64 format are able to store 3 bytes with 4 "digits". Compare this with 
 * decimal where one byte might need 3 digits.
 * check here to test encoding to base64: https://codebeautify.org/ascii-to-base64-converter
 * @param puStream byte stream that is converted to base64
 * @param uLength length for byte stream to convert
 * @param puBuffer base64 encoded characters
 * @return number of base64 digits generated
*/
size_t base64_encode_g( const uint8_t* puStream, size_t uLength, uint8_t* puBuffer )
{
   uint8_t uValue;                  // current base64 digit value 
   const uint8_t* puRead = puStream;// pointer to read stream for current read position
   uint8_t* puWrite = puBuffer;     // write position to buffer that gets base64 digits
   unsigned uBase64SectionCount = ((unsigned)uLength / 3); // number of complete base64 sections

   for( auto u = 0u; u < uBase64SectionCount; u++ )
   {
      uint32_t uBase64Value = 0;

      // ## convert to base64 value
      uBase64Value |= (*puRead++ << 0x10);
      uBase64Value |= (*puRead++ << 0x08);
      uBase64Value |= *puRead++;


      // ## Write value to buffer with the 64 base (64 different "digits")
      uValue = (uBase64Value >> 18) & 0x3f;
      *puWrite++ = pchBase64Digit_s[ uValue ];
      uValue = (uBase64Value >> 12) & 0x3f;
      *puWrite++ = pchBase64Digit_s[ uValue ];
      uValue = (uBase64Value >> 6) & 0x3f;
      *puWrite++ = pchBase64Digit_s[ uValue ];
      uValue = uBase64Value  & 0x3f;
      *puWrite++ = pchBase64Digit_s[ uValue ];
   }

   // ## Convert the last trailing characters if any
   switch( uLength % 3 )
   {
   case 2:                                                                     // two extra characters that need to be converted
      uValue = (*puRead & 0xfc) >> 2;                                          // first 6 bits from byte
      *puWrite++ = pchBase64Digit_s[ uValue ];
      
      uValue = ((*puRead & 0x03) << 4) + ((puRead[1] & 0xf0) >> 4);            // last two bits from first byte and first 4 bits from second byte
      *puWrite++ = pchBase64Digit_s[ uValue ];

      uValue = (puRead[1] & 0x0f) << 2;                                        // last four bits from second byte
      *puWrite++ = pchBase64Digit_s[ uValue ];

      *puWrite++ = '=';                                                        // pad with =
      break;

   case 1:                                                                     // one extra character that need to be converted
      uValue = (*puRead & 0xfc) >> 2;                                          // first 6 bits from byte
      *puWrite++ = pchBase64Digit_s[ uValue ];

      uValue = ((*puRead & 0x03) << 4);                                        // last two bits from byte
      *puWrite++ = pchBase64Digit_s[ uValue ];

      *puWrite++ = '=';                                                        // pad with =
      *puWrite++ = '=';                                                        // pad with =
      break;

   case 0:
      break;
   }

   *puWrite = '\0';

   return puWrite - puBuffer;                                                  // return number of base64 digits (and padding)
}

/** ---------------------------------------------------------------------------
 * @brief encode chars to base64 format and place it in vector
 * @param puStream byte stream that is converted to base64
 * @param uLength length for byte stream to convert
 * @param vectorBase64 reference to vector where base64 digits are placed
 * @return number of base64 digits generated
*/
size_t base64_encode_g( const char* puStream, size_t uLength, std::vector<char>& vectorBase64 )
{
   size_t uBufferSize = base64_size_g( uLength ) + 1; // Needed size to store bytes as base64 and zero terminator
   
   auto uVectorSize = vectorBase64.size();
   uBufferSize += uVectorSize;                                                 // add vector size if any, this might be used if data exists in vector

   vectorBase64.resize( uBufferSize );                                         // preallocate buffer
   auto uSize = base64_encode_g( (const uint8_t*)puStream, uLength, (uint8_t*)vectorBase64.data() + uVectorSize );

   return uSize;
}

/// Converts byte stream to vector and return it
std::vector<char> base64_encode_g( const char* puStream, size_t uLength, tag_vector )
{
   std::vector<char> vectorBase64;

   base64_encode_g( puStream, uLength, vectorBase64 );

   return vectorBase64;
}

/// Encode byte stream to base64 as string and return it
std::string base64_encode_g( const char* puStream, size_t uLength, tag_string )
{
   std::vector<char> vectorBase64;

   base64_encode_g( puStream, uLength, vectorBase64 );

   return std::string( vectorBase64.data(), vectorBase64.size() - 1 );
}

/** ---------------------------------------------------------------------------
 * @brief decode base64 to chars and place it in buffer, make sure buffer is large enough
 * @param puStream base64 stream that is decoded into chars
 * @param uLength length of base64 stream
 * @param puBuffer pointer to buffer where converted characters are stored
 * @return number of converted chars
*/
size_t base64_decode_g( const uint8_t* puStream, size_t uLength, uint8_t* puBuffer )
{
   uint8_t uValue;
   const uint8_t* puRead = puStream;// pointer to read stream for current read position
   const uint8_t* puEnd = puStream + uLength;// end of stream
   uint8_t* puWrite = puBuffer;     // write position to buffer
   unsigned uTrailCount = 0;
   if( puRead[uLength - 1] == '=' ) { uLength--; uTrailCount = 2; }
   if( puRead[uLength - 1] == '=' ) { uLength--; uTrailCount = 1; }
   unsigned uBase64SectionCount = uLength >= 4 ? ((unsigned)uLength / 4) : 0; // number of complete base64 sections

   for( auto u = 0u; u < uBase64SectionCount; u++ )
   {
      uint32_t uBase64Value = 0;
      uValue = piBase64Value_s[*puRead++];                                                         assert( uValue <= 63 );
      uBase64Value = (uValue << 18);
      uValue = piBase64Value_s[*puRead++];                                                         assert( uValue <= 63 );
      uBase64Value |= (uValue << 12);
      uValue = piBase64Value_s[*puRead++];                                                         assert( uValue <= 63 );
      uBase64Value |= (uValue << 6);
      uValue = piBase64Value_s[*puRead++];                                                         assert( uValue <= 63 );
      uBase64Value |= uValue;

      *puWrite++ = (uint8_t)((uBase64Value >> 16) & 0xff);
      *puWrite++ = (uint8_t)((uBase64Value >> 8) & 0xff);
      *puWrite++ = (uint8_t)(uBase64Value & 0xff);
   }

   // get last padding part
   if( puRead < puEnd )
   {
      uint32_t uBase64Value = 0;

      uValue = piBase64Value_s[*puRead++];                                                         assert( uValue <= 63 );
      uBase64Value = (uValue << 18);
      uValue = piBase64Value_s[*puRead++];                                                         assert( uValue <= 63 );
      uBase64Value |= (uValue << 12);
      uValue = piBase64Value_s[*puRead++];                                                         
      if( uValue <= 63 ) uBase64Value |= (uValue << 6);
      uValue = piBase64Value_s[*puRead++];                                                         
      if( uValue <= 63 ) uBase64Value |= uValue;

      *puWrite++ = (uint8_t)((uBase64Value >> 16) & 0xff);
      if( uTrailCount > 1 ) *puWrite++ = (uint8_t)((uBase64Value >> 8) & 0xff);
   }

   *puWrite = '\0';

   return puWrite - puStream - 1;
}


/** ---------------------------------------------------------------------------
 * @brief convert base64 to byte format and place it in vector
 * @param puStream base64 stream that is converted to bytes
 * @param uLength length for base64 stream to convert
 * @param vectorBase64 reference to vector where byte digits are placed
 * @return number of base64 digits generated
*/
size_t base64_decode_g( const char* puStream, size_t uLength, std::vector<uint8_t>& vectorBase64 )
{
   size_t uBufferSize = base64_size_g( uLength ) + 1; // Needed size to store bytes as base64 and zero terminator
   
   auto uVectorSize = vectorBase64.size();
   uBufferSize += uVectorSize;                                                 // add vector size if any, this might be used if data exists in vector

   vectorBase64.resize( uBufferSize );                                         // preallocate buffer
   auto uSize = base64_decode_g( (const uint8_t*)puStream, uLength, (uint8_t*)vectorBase64.data() + uVectorSize );

   return uSize;
}

/// Converts base54 stream to bytes in vector and return it
std::vector<uint8_t> base64_decode_g( const char* puStream, size_t uLength, tag_vector )
{
   std::vector<uint8_t> vectorBase64;

   base64_decode_g( puStream, uLength, vectorBase64 );

   return vectorBase64;
}


/// Decodes base64 stream to string and return it
std::string base64_decode_g( const char* puStream, size_t uLength, tag_string )
{
   std::vector<uint8_t> vectorBase64;

   base64_decode_g( puStream, uLength, vectorBase64 );

   return std::string( (const char*)vectorBase64.data(), vectorBase64.size() - 1 );
}




_GD_TRANSLATE_END