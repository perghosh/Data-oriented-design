/**
 * \file gd_types.cpp
 * 
 * \brief definitions for gd type system
 * 
 */

#include <chrono>
#include <random>
#include <thread>

#include "gd_types.h"

_GD_TYPES_BEGIN

constexpr uint8_t puCharType_g[0x100] =
{
   //       0, 1, 2, 3,  4, 5, 6, 7,  8, 9, A, B,  C, D, E, F,
   /* 0 */ 00,00,00,00, 00,00,00,00, 01,01,00,00, 00,01,00,00,  /* 0   - 15  */
   /* 1 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 16  - 31  */
   /* 2 */ 01,00,07,00, 00,00,00,07, 00,00,00,00, 00,00,00,00,  /* 32  - 47   ,!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 02,02,02,02, 02,02,02,02, 02,02,00,00, 00,00,00,00,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

   /* 4 */ 00,03,03,03, 03,03,03,03, 03,03,03,03, 03,03,03,03,  /* 64  - 79  */
   /* 5 */ 03,03,03,03, 03,03,03,03, 03,03,03,00, 00,00,00,00,  /* 80  - 95  */
   /* 6 */ 07,03,03,03, 03,03,03,03, 03,03,03,03, 03,03,03,03,  /* 96  - 111 */
   /* 7 */ 03,03,03,03, 03,03,03,03, 03,03,03,00, 00,00,00,00,  /* 112 - 127 */

   /* 8 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 128 - 143 */
   /* 9 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 144 - 159 */
   /* A */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 160 - 175 */
   /* B */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 176 - 191 */

   /* C */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 192 - 207 */
   /* D */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 208 - 223 */
   /* E */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 224 - 239 */
   /* F */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00   /* 240 - 255 */
};




/// 256 word values with bits set to mark different character classes used in parse logic
/// 0x0020 = CHAR_GROUP_DECIMAL | CHAR_GROUP_SCIENTIFIC
/// 0x04E2 = CHAR_GROUP_ALNUM | CHAR_GROUP_SCIENTIFIC | CHAR_GROUP_HEX | CHAR_GROUP_DECIMAL | CHAR_GROUP_DIGIT (number) 
/// 0x0404 = CHAR_GROUP_ALNUM | CHAR_GROUP_ALPHABET
/// 0x0444 = CHAR_GROUP_ALNUM | CHAR_GROUP_HEX | CHAR_GROUP_ALPHABET
/// 0x04E4 = CHAR_GROUP_ALNUM | CHAR_GROUP_SCIENTIFIC | CHAR_GROUP_HEX | CHAR_GROUP_DECIMAL | CHAR_GROUP_ALPHABET
/// 0x0010 = CHAR_GROUP_QUOTE (quote)
constexpr uint16_t puCharGroup_g[0x100] =
{
   //   0,     1,     2,     3,     4,     5,     6,     7,     8,     9,     A,     B,     C,     D,     E,     F
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0000,0x0001,0x0000,0x0000,0x0000, /* 0x00-0x0F */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x10-0x1F */
   0x0001,0x0000,0x0010,0x0000,0x0000,0x0000,0x0000,0x0010,0x0000,0x0001,0x0008,0x0008,0x0000,0x0008,0x00A0,0x1008, /* 0x20-0x2F  ,!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x0000,0x0000,0x0008,0x0008,0x0008,0x0000, /* 0x30-0x3F 0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */
   0x0000,0x0444,0x0444,0x0444,0x0444,0x04E4,0x0444,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404, /* 0x40-0x4F @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O */
   0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0008,0x1000,0x0000,0x0008,0x0008, /* 0x50-0x5F P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_ */
   0x0000,0x0444,0x0444,0x0444,0x0444,0x04E4,0x0444,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404, /* 0x60-0x6F `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o */
   0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x70-0x7F p,q,r,s,t,u,v,w,x,y,z,{,|,},~*/
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x80-0x8F */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x90-0x9F */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xA0-0xAF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xB0-0xBF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xC0-0xCF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xD0-0xDF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xE0-0xEF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xF0-0xFF */
};

// static_assert( ctype_g("digit") & CHAR_GROUP_DIGIT, "Wrong type for '1'");
// static_assert( (puCharGroup_g[uint8_t('1')] && ctype_g("digit")) == ctype_g("digit"), "Wrong type for '1'");
// static_assert( ctype_g("alphabet") == CHAR_GROUP_ALPHABET );
// static_assert( is_ctype( 'a', ctype_g("alphabet") ) == true, "Wrong type for 'a'");


/// 256 byte values with bits set to mark different character classes used in parse logic
//static const uint16_t pCharGroup__s[0x100] =
//{
//   // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
//   00,00,00,00,00,00,00,00,00,64,64,00,00,00,00,00, /* 0x00-0x0F */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x10-0x1F */
//   64,00,128,00,00,00,00,128,00,00,00,02,00,10, 8,00, /* 0x20-0x2F  ,!,",#,$,%,&,',(,),*,+,,,-,.,/*/
//   31,31,31,31,31,31,31,31,31,31,00,00,00,00,00,00, /* 0x30-0x3F 0,1,2,3,4,5,6,7,8,9 ... */
//   00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,01, /* 0x40-0x4F @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O*/
//   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x50-0x5F P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_*/
//   00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,01, /* 0x60-0x6F `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o*/
//   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x70-0x7F p,q,r,s,t,u,v,W,x,y,z,{,|,},~*/
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x80-0x8F */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x90-0x9F */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xA0-0xAF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xB0-0xBF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xC0-0xCF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xD0-0xDF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xE0-0xEF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xF0-0xFF */
//};

/// Get constant for specified character type name
uint8_t ctype_g(const std::string_view& stringCType, tag_main_type )
{
   using namespace detail;
   uint32_t uCTypeName = hash_type( stringCType ); 
   switch( uCTypeName )
   {
   case hash_type("space"):            return CHAR_TYPE_SPACE;
   case hash_type("digit"):            return CHAR_TYPE_DIGIT;
   case hash_type("alphabet"):         return CHAR_TYPE_ALPHABET;
   case hash_type("operator"):         return CHAR_TYPE_OPERATOR;
   case hash_type("quote"):            return CHAR_GROUP_QUOTE;
   case hash_type("decimal"):          return CHAR_TYPE_PUNCTUATOR;
   }

   return 0;
}

/// Get constant for specified character type name
uint16_t ctype_g(const std::string_view& stringCType)
{
   using namespace detail;
   uint32_t uCTypeName = hash_type( stringCType ); 
   switch( uCTypeName )
   {
   case hash_type("space"):            return CHAR_GROUP_SPACE;
   case hash_type("digit"):            return CHAR_GROUP_DIGIT;
   case hash_type("alphabet"):         return CHAR_GROUP_ALPHABET;
   case hash_type("operator"):         return CHAR_GROUP_OPERATOR;
   case hash_type("quote"):            return CHAR_GROUP_QUOTE;
   case hash_type("decimal"):          return CHAR_GROUP_DECIMAL;
   case hash_type16("hex"):            return CHAR_GROUP_HEX;
   case hash_type("scientific"):       return CHAR_GROUP_SCIENTIFIC;
   case hash_type("punctuation"):      return CHAR_GROUP_PUNCTUATION;
   case hash_type("bracket"):          return CHAR_GROUP_BRACKET;
   case hash_type("alnum"):            return CHAR_GROUP_ALNUM;
   case hash_type16("xml"):            return CHAR_GROUP_XML;
   case hash_type16("file"):           return CHAR_GROUP_FILE;
   }

   return 0;
}

/** ---------------------------------------------------------------------------
 * @brief Check if string buffer is text, number or decimal
 * This method tries to figure out if value is integer, decimal or text, just those
 * types form text.
 * @param puText start of text
 * @param uLength text length
 * @return constant for the type group, eTypeGroupInteger, eTypeGroupDecimal or eTypeGroupString
 */
unsigned detect_ctypegroup_g( const uint8_t* puText, unsigned uLength )
{
   unsigned uType = 0;
   unsigned uCharType;
   const uint8_t* puPosition = puText;
   const uint8_t* puEnd = puText + uLength;

   if( uLength == 0 ) return 0;

   // ## special case, check for starting - character
   uCharType = get_ctype_g( *puPosition );
   if( *puPosition == '-' )
   {
      uType = CHAR_GROUP_DIGIT;
      puPosition++;
   }
   else if( *puPosition == '.' )
   {
      uType = CHAR_GROUP_DECIMAL;
      puPosition++;
   }
   else if( uCharType & CHAR_GROUP_DIGIT )
   {
      uType = CHAR_GROUP_DIGIT;
      puPosition++;
   }
   else { return eTypeGroupString; }

   // ## Check rest to find out if it is a digit, decimal or string
   //    If we have found a digit and then finds a decimal character type is
   //    modified to decimal. 
   //    But if type is decimal and it finds another decimal type is string
   const uint16_t uFlagToCheck = (CHAR_GROUP_DIGIT | CHAR_GROUP_DECIMAL);
   while( puPosition != puEnd )
   {
      auto uCharType = get_ctype_g( *puPosition );
      uCharType &= uFlagToCheck; 
      if( uCharType != 0 )
      {
         // Test if only decimal, if no digit and only decimal we have to check for proper decimal value
         if( uCharType == CHAR_GROUP_DECIMAL )
         {
            if( uType == CHAR_GROUP_DIGIT ) uType = CHAR_GROUP_DECIMAL;
            else 
            {
               uType = CHAR_GROUP_ALPHABET;                                    // set to text because we have more than one decimal value
               break;
            }
         }
      }
      else                                                                     // not a number or decimal char, then it is probably text
      {
         uType = CHAR_GROUP_ALPHABET;
         break;
      }

      puPosition++;
   }

   // ## return type group
   if( uType == CHAR_GROUP_DIGIT ) return eTypeGroupInteger;
   else if( uType == CHAR_GROUP_DECIMAL ) return eTypeGroupDecimal;

   return eTypeGroupString;
}

/// Generate a random UUID v4
/// Uses thread_local mt19937_64: seeded once per thread, generates 128 bits in 2 calls instead of 16.
uuid uuid_generate_g()
{
   // One engine per thread — no locks, no contention
   thread_local std::mt19937_64 rng = []()
   {
      // Seed with multiple entropy sources to avoid same-sequence threads
      std::mt19937_64 mt_;
      std::seed_seq seedseq_
      {
          static_cast<uint64_t>( std::chrono::high_resolution_clock::now().time_since_epoch().count() ),
          static_cast<uint64_t>( std::hash<std::thread::id>{}( std::this_thread::get_id() ) ),
          static_cast<uint64_t>( reinterpret_cast<uintptr_t>( &mt_ ) )  // stack address for extra variance
      };
      mt_.seed( seedseq_ );
      return mt_;
   }( );

   uint64_t uHi = rng();
   uint64_t uLo = rng();

   // Stamp RFC 4122 v4 (random) version and variant bits
   uHi = ( uHi & 0xFFFFFFFFFFFF0FFFULL ) | 0x0000000000004000ULL;  // version = 4
   uLo = ( uLo & 0x3FFFFFFFFFFFFFFFULL ) | 0x8000000000000000ULL;  // variant = 0b10

   uuid uuid_;
   memcpy( uuid_.m_puData, &uHi, 8 );
   memcpy( uuid_.m_puData + 8, &uLo, 8 );
   return uuid_;
}

/** ---------------------------------------------------------------------------
 * @brief Hex nibble decode table — maps ASCII byte to its 4-bit value.
 * Indices below 0x30 ('0') are unreachable for valid hex input; entries for
 * non-hex characters are 0 (asserts guard misuse in debug builds).
 * Covers 0x30–0x66 ('0'–'f'), 104 bytes — fits in one or two cache lines.
 */
static constexpr uint8_t puHexNibble_s[0x67] =
{
   //  0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x00-0x0F */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x10-0x1F */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x2F */
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x3F  '0'-'9' */
   0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40-0x4F  'A'-'F' */
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x5F */
   0x00, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,                                                        /* 0x60-0x66  'a'-'f' */
};

/** ---------------------------------------------------------------------------
 * @brief Convert hex string_view to binary span (pre-allocated, exact fit assumed).
 * Input must be a plain hex string with no separators — caller is responsible
 * for ensuring `spanBinary.size() == stringHex.length() / 2`.
 * Each pair of hex characters is decoded into one output byte using a branchless
 * nibble table; no format overhead, no allocations, no bounds checks at runtime.
 *
 * @param stringHex  hex-encoded source, e.g. "deadbeef..."
 * @param spanBinary pre-allocated destination; size must be `stringHex.length() / 2`
 */
void from_string_g( const std::string_view& stringHex, std::span<uint8_t> spanBinary, tag_hex )
{                                                                                                    assert( (stringHex.length() & 1) == 0 );
                                                                                                     assert( spanBinary.size() >= stringHex.length() / 2 );
   const uint8_t* puBegin = reinterpret_cast<const uint8_t*>( stringHex.data() );
   const uint8_t* puEnd   = puBegin + stringHex.length();
   uint8_t*       puSet   = spanBinary.data();

   while( puBegin != puEnd )
   {                                                                                                 assert( *puBegin < sizeof(puHexNibble_s) ); assert( *(puBegin + 1) < sizeof(puHexNibble_s) );
      *puSet++ = static_cast<uint8_t>( puHexNibble_s[*puBegin] << 4 ) | puHexNibble_s[*(puBegin + 1)];
      puBegin += 2;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Convert UUID-formatted string_view to a `gd::types::uuid` value.
 * Accepts the three canonical RFC 4122 formats:
 *   - 32-char compact:   `00000000000000000000000000000000`
 *   - 36-char dashed:    `00000000-0000-0000-0000-000000000000`
 *   - 38-char braced:    `{00000000-0000-0000-0000-000000000000}`
 * Dashes and braces are skipped; every other character is a hex nibble.
 * Two nibbles are merged into one output byte via `puHexNibble_s`.
 * No heap use, no branching inside the decode loop.
 *
 * @param stringUuid  UUID string in one of the three formats above
 * @return uuid       decoded 16-byte UUID value
 */
uuid from_string_g( std::string_view stringUuid, tag_uuid )
{                                                                                                    assert( stringUuid.length() == 32 || stringUuid.length() == 36 || stringUuid.length() == 38 );
   uuid uuid_;

   const uint8_t* puBegin = reinterpret_cast<const uint8_t*>( stringUuid.data() );
   const uint8_t* puEnd   = puBegin + stringUuid.length();
   uint8_t*       puSet   = uuid_.m_puData;

   // ## skip surrounding braces for the 38-char `{...}` format
   if( stringUuid.length() == 38 ) { puBegin++; puEnd--; }

   if( stringUuid.length() == 32 )
   {
      // ## compact path — no dashes, unroll the 16 iterations in pairs of two bytes
      //    to give the compiler better auto-vectorisation hints
      for( ; puBegin != puEnd; puBegin += 2 )
      {                                                                                              assert( *puBegin < sizeof(puHexNibble_s) ); assert( *(puBegin + 1) < sizeof(puHexNibble_s) );
         *puSet++ = static_cast<uint8_t>( puHexNibble_s[*puBegin] << 4 ) | puHexNibble_s[*(puBegin + 1)];
      }
   }
   else
   {
      // ## dashed path — skip '-' separators at positions 8, 13, 18, 23 of the 36-char form
      while( puBegin != puEnd )
      {
         if( *puBegin == '-' ) { puBegin++; continue; }
                                                                                                     assert( *puBegin < sizeof(puHexNibble_s) ); assert( *(puBegin + 1) < sizeof(puHexNibble_s) );
         *puSet++ = static_cast<uint8_t>( puHexNibble_s[*puBegin] << 4 ) | puHexNibble_s[*(puBegin + 1)];
         puBegin += 2;
      }
   }

   return uuid_;
}



_GD_TYPES_END