

#include <bit>
#include <charconv>

#if defined(__APPLE__) && defined(__GNUG__) && !defined(__clang__)

#  ifdef __AVX2__
#     define GD_X86
#  endif

#elif defined(__clang__)

#  ifdef __AVX2__
#     define GD_X86
#  endif

#else

#  if defined(_MSC_VER)
#     define GD_X86
#  endif

#endif

#ifdef GD_X86

#  include <immintrin.h>
#  include <emmintrin.h>
#  include <smmintrin.h>

#endif


#include "gd_parse.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4267 26495 26812 )
#endif

_GD_PARSE_BEGIN

const uint8_t ASCII_TAB = 9;
const uint8_t ASCII_LINEFEED = 10;
const uint8_t ASCII_CARRAGERETURN = 13;
const uint8_t ASCII_SPACE = 32;


const uint8_t ASCII_TYPE_ALNUM           = 0b0000'0001;                    ///< 1 = alphabet or number
const uint8_t ASCII_TYPE_DIGIT           = 0b0000'0010;                    ///< 2 = digit character (0-9)
const uint8_t ASCII_TYPE_INTEGER         = 0b0000'0100;                    ///< 4 = integer character
const uint8_t ASCII_TYPE_DECIMAL         = 0b0000'1000;                    ///< 8 = decimal character
const uint8_t ASCII_TYPE_SCIENTIFIC      = 0b0001'0000;                    ///< 16 = scientific number character
const uint8_t ASCII_TYPE_HEX             = 0b0010'0000;                    ///< 32 = valid hex character
const uint8_t ASCII_TYPE_SPACE           = 0b0100'0000;                    ///< 64 = space character
const uint8_t ASCII_TYPE_QUOTE           = 0b1000'0000;                    ///< 128 = quote character

/// 256 byte values with bits set to mark different character classes used in parse logic
static const uint8_t pCharacterClass_s[0x100] =
{
 // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
   00,00,00,00,00,00,00,00,00,64,64,00,00,64,00,00, /* 0x00-0x0F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x10-0x1F */
   64,00,128,00,00,00,00,128,00,00,00,02,00,10, 8,00, /* 0x20-0x2F  ,!,",#,$,%,&,',(,),*,+,,,-,.,/*/
   31,31,31,31,31,31,31,31,31,31,00,00,00,00,00,00, /* 0x30-0x3F 0,1,2,3,4,5,6,7,8,9 ... */
   00,01,01,01,01,25,01,01,01,01,01,01,01,01,01,01, /* 0x40-0x4F @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O*/
   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x50-0x5F P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_*/
   00,01,01,01,01,25,01,01,01,01,01,01,01,01,01,01, /* 0x60-0x6F `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o*/
   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x70-0x7F p,q,r,s,t,u,v,W,x,y,z,{,|,},~*/
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x80-0x8F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x90-0x9F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xA0-0xAF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xB0-0xBF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xC0-0xCF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xD0-0xDF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xE0-0xEF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xF0-0xFF */
};

/// 256 byte values with bits set to mark character least type (not the combination)
static const uint8_t pCharacterMinimalClass_s[0x100] =
{
 // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
   00,00,00,00,00,00,00,00,00,64,64,00,00,00,00,00, /* 0x00-0x0F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x10-0x1F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,10, 8,00, /* 0x20-0x2F  ,!,",#,$,%,&,',(,),*,+,,,-,.,/*/
   04,04,04,04,04,04,04,04,04,04,00,00,00,00,00,00, /* 0x30-0x3F 0,1,2,3,4,5,6,7,8,9 ... */
   00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,01, /* 0x40-0x4F @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O*/
   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x50-0x5F P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_*/
   00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,01, /* 0x60-0x6F `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o*/
   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x70-0x7F p,q,r,s,t,u,v,W,x,y,z,{,|,},~*/
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x80-0x8F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x90-0x9F */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xA0-0xAF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xB0-0xBF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xC0-0xCF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xD0-0xDF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xE0-0xEF */
   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xF0-0xFF */
};



const uint8_t JSON_TYPE_KEY            = 0b0000'0001;                    ///< 1 = start of object
const uint8_t JSON_TYPE_STRING         = 0b0000'0010;                    ///< 2 = start of string
const uint8_t JSON_TYPE_ESCAPE         = 0b0000'0100;                    ///< 4 = escaped character in json string
const uint8_t JSON_TYPE_DIVIDE         = 0b0000'1000;                    ///< 8 = character that divide value
const uint8_t JSON_TYPE_VALUE          = 0b0001'0000;                    ///< 16 = marker that value is coming

const char pJson_s[256] =
{        // 0, 1, 2, 3,  4, 5, 6, 7,  8, 9, A, B,  C, D, E, F
   /* 0 */ 00,00,00,00, 00,00,00,00, 00,04,04,00, 00,04,00,00,  /* 0   - 15  ('\t'=9) ('\n'=10) ('\r'=13) */
   /* 1 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 16  - 31  */
   /* 2 */ 00,00,06,00, 00,00,00,00, 00,00,00,00, 01,00,00,00,  /* 32  - 47  ('"'=34) (','=44) */
   /* 3 */ 00,00,00,00, 00,00,00,00, 00,00,16,00, 00,00,00,00,  /* 48  - 63  (':'=58)*/

   /* 4 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 64  - 79  */
   /* 5 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 04,00,00,00,  /* 80  - 95  ('\'=92) */
   /* 6 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 96  - 111 */
   /* 7 */ 00,00,00,00, 00,00,00,00, 00,00,00,01, 00,01,00,00,  /* 112 - 127 ('{'=123) ('}'=125) */

   /* 8 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 128 - 143 */
   /* 9 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 144 - 159 */
   /* A */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 160 - 175 */
   /* B */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 176 - 191 */

   /* C */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 192 - 207 */
   /* D */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 208 - 223 */
   /* E */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 224 - 239 */
   /* F */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00   /* 240 - 255 */
};






void ascii::set_flag( uint8_t uFlag, const uint8_t* puCaracter, unsigned uLength )
{                                                                                                  assert( uLength < 0x100 );
   while( uLength > 0 )
   {
      uLength--;
      m_puBuffer[puCaracter[uLength]] |= uFlag;
   }
}

csv::csv( const std::string_view& stringDelimiter ):
   m_uLineEnd('\n'), m_uQuote('\"') 
{
   if( stringDelimiter.length() > 1 )
   {
      m_uDelimiter = (uint8_t)stringDelimiter[0];
      if( stringDelimiter[1] == '*' ) m_uDelimiter |= eDelimiterMany;
   }
}


const uint8_t* csv::next_value( const uint8_t* puPosition, const uint8_t* pbszEnd ) const
{
   const uint8_t* puNext = puPosition;

   if( is_delimiter_single_char() == true )
   {
      puNext = (const uint8_t*)skip_space_g( (const char*)puNext, (const char*)pbszEnd );
      if( *puNext == get_delimiter() ) 
      {
         puNext++;
         puNext = ( const uint8_t* )skip_space_g( ( const char* )puNext, ( const char* )pbszEnd );
         return puNext;
      }
   }
   else
   {
      if( m_uDelimiter & eDelimiterMany )
      {
         const uint8_t* puStart = puNext;
         uint8_t uDelimiter = get_delimiter();
         while( puNext < pbszEnd && *puNext == uDelimiter ) puNext++;
         return puStart != puNext ? puNext : nullptr;
      }
   }
   return nullptr;
}


/// Move to specified character
/// If not found return pointer is same as input pointer
const char* next_character_g( const char* pbsz, char chFind )
{
   const char* pbszFind = pbsz;   
   while( (*pbszFind != '\0') && !(*pbszFind == chFind) ) pbszFind++;

   if( *pbszFind == '\0' ) return pbsz;
   return pbszFind;
}

/// Move to specified character
/// If not found return pointer is same as input pointer
const char* next_character_g( const char* pbsz, char chFind, tag_reverse )
{
   auto find_ = pbsz;   
   while( !(*find_ == chFind) ) find_--;

   return find_;
}


/// Move to specified character or null terminator
const char* next_character_g( const char* pbsz, char chFind, tag_zero_end )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && !(*find_ == chFind) ) find_++;

   return find_;
}


/// Move to specified character
/// If not found return pointer is same as input pointer
const char* next_character_g( const char* pbsz, const char* pbszEnd, char chFind )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   auto find_ = pbsz;   
   while( (find_ < pbszEnd) && !(*find_ == chFind) ) find_++;

   if( *find_ == chFind ) return find_;
   return pbsz;
}

/// Move to specified character
/// If not found return pointer is same as input pointer
const char* next_character_g( const char* pbsz, const char* pbszEnd, char chFind, tag_avx256 )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
#ifndef GD_X86
   return next_character_g( pbsz, pbszEnd, chFind );
#else
   const char* pbsz256;
   if( (pbszEnd - pbsz) > 32 ) pbsz256 = next_align256_g( pbsz );              // align position for 32 byte blocks if possible to read that length
   else pbsz256 = pbszEnd;                                                     // total size is below 32 bytes

   // ## find characters or end for first part before 32 byte blocks
   for( auto pbszShort = pbsz; pbszShort != pbsz256; pbszShort++ )
   {
      if( *pbszShort == chFind ) return pbszShort;
   }

   const char* pbszPosition = pbsz256;

   auto pbszEnd256 = pbszEnd - 32;
   if( pbszPosition < pbszEnd256 )
   {
      __m256i iCharFind = _mm256_set1_epi8( chFind );                          // value to match find character

      // ## check 32 byte blocks for character
      for( ; pbszPosition < pbszEnd256; pbszPosition += 32 )
      {                                                                                            assert( pbszEnd - pbszPosition >= 32 );
         __m256i i32ByteSection = _mm256_load_si256((__m256i*)pbszPosition );
         __m256i iCompareFind = _mm256_cmpeq_epi8( i32ByteSection, iCharFind );
         int iMaskFind = _mm256_movemask_epi8( iCompareFind ); 
         if( iMaskFind == 0 ) continue;

         uint32_t uPosition = std::countr_zero( (unsigned)iMaskFind );
         return pbszPosition + uPosition;
      }
   }

   for( ; pbszPosition != pbszEnd; pbszPosition++ )
   {
      if( *pbszPosition == chFind ) return pbszPosition;
   }

   return pbsz;                                                                // return original position because character was not found
#endif
}


/// Go to any valid character within ascii buffer
/// If not found return pointer is same as input pointer
const char* next_character_g( const char* pbsz, const ascii& characterFind )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && !(characterFind.find( *find_ ) != true) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Go to any valid character within ascii buffer
/// If not found return pointer is same as input pointer
const char* next_character_g( const char* pbsz, const char* pbszEnd, const ascii& characterFind )
{
   auto find_ = pbsz;   
   while( (find_ < pbszEnd) && !(characterFind.find( *find_ ) != true) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Move to specified character or last position
const char* next_character_or_end_g( const char* pbsz, const char* pbszEnd, char chFind )
{
   while( (pbsz < pbszEnd) && (*pbsz != chFind) ) pbsz++;

   return pbsz;
}

/// Move to specified character or zero ending
const char* next_character_or_end_g( const char* pbsz, char chFind, tag_avx256 )
{
#ifndef GD_X86
   auto p_ = strchr( pbsz, chFind );
   return p_;
#else
   auto pbsz256 = next_align256_g( pbsz );                                     // align position for 32 byte blocks

   // ## find characters or end for first part before 32 byte blocks
   for( auto pbszShort = pbsz; pbszShort != pbsz256; pbszShort++ )
   {
      if( *pbszShort == chFind || *pbszShort == '\0' ) return pbszShort;
   }

   __m256i iCharFind = _mm256_set1_epi8( chFind );                             // value to match find character
   __m256i iCharZero = _mm256_set1_epi8( '\0' );                               // value match zero ending

   const char* pbszPosition = pbsz256;
   bool bFind = true;
   while( bFind == true ) 
   {
      __m256i i32ByteSection = _mm256_load_si256((__m256i*)pbszPosition );
      __m256i iCompareFind = _mm256_cmpeq_epi8( i32ByteSection, iCharFind );
      __m256i iCompareZero = _mm256_cmpeq_epi8( i32ByteSection, iCharZero );

      int iMaskFind = _mm256_movemask_epi8( iCompareFind );                    // match for any character
      int iMaskZero = _mm256_movemask_epi8( iCompareZero );                    // match for zero ending

      if( iMaskFind == 0 && iMaskZero == 0 )
      {
         pbszPosition += 32;
         continue;
      }

      bFind = false;                                                           // break loop (character is found)
      if( iMaskFind != 0 )                                                     // found character ?
      {
         uint32_t uPosition = std::countr_zero( (unsigned)iMaskFind );
         pbszPosition += uPosition;
      }
      else                                                                     // found zero ending
      {                                                                                            assert( iMaskZero == 0 );
         uint32_t uPosition = std::countr_zero( (unsigned)iMaskFind );
         pbszPosition += uPosition;
      }
   }

   return pbszPosition;
#endif
}




/// Move to space character
/// If not found return pointer is same as input pointer
const char* next_space_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && !(pCharacterClass_s[*find_] & ASCII_TYPE_SPACE) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Move to space character
/// If not found return pointer is same as input pointer
const char* next_space_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   auto find_ = pbsz;   
   while( (find_ < pbszEnd) && !(pCharacterClass_s[*find_] & ASCII_TYPE_SPACE) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Move to next non decimal character
/// If not found return pointer is same as input pointer
const char* next_non_decimal_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_DECIMAL) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Move to next non integer character
/// If not found return pointer is same as input pointer
const char* next_non_integer_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_INTEGER) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Move to next non space character
/// If not found return pointer is same as input pointer
const char* next_non_space_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_SPACE) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Move to next non space character
/// If not found return pointer is same as input pointer
const char* next_non_space_or_end_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_SPACE) ) find_++;

   return find_;
}


/// Move to next non space character
/// If not found return pointer is same as input pointer
const char* next_non_space_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   auto find_ = pbsz;   
   while( (find_ < pbszEnd) && (pCharacterClass_s[*find_] & ASCII_TYPE_SPACE) ) find_++;

   if( *find_ == '\0' ) return pbsz;
   return find_;
}

/// Read boolean value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_boolean_g( const char* pbsz, bool& bValue )
{
   auto find_ = skip_alnum_g( pbsz );                                                              assert( (find_ - pbsz) < 6 ); // assert if value do not match with boolean in text ("false")
   if( find_ != pbsz )
   {
      bool bRead = true;
      if( *find_ == '0' || *find_ == 'f' ) bRead = false;

      bValue = bRead;
   }

   return find_;
}

/// Read boolean value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_boolean_g( const char* pbsz, const char* pbszEnd, bool& bValue )
{
   auto find_ = skip_alnum_g( pbsz, pbszEnd );                                                     assert( (find_ - pbsz) < 6 ); // assert if value do not match with boolean in text ("false")
   if( find_ != pbsz )
   {
      bool bRead = true;
      if( *find_ == '0' || *find_ == 'f' ) bRead = false;

      bValue = bRead;
   }

   return find_;
}


/// Read integer value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_int32_g( const char* pbsz, int32_t& iValue )
{
   auto find_ = skip_integer_g( pbsz );                                                            assert( (find_ - pbsz) < 12 ); // assert if number is to big
   if( find_ != pbsz )
   {
      int32_t iRead = 0;
      bool bNegative = false;
      if( *pbsz == '-' ) { bNegative = true; pbsz++; }
      for( ;pbsz != find_; pbsz++ )
      {
         iRead = iRead * 10 + (*pbsz - '0');
      }

      if( bNegative == true ) iRead = -iRead;

      iValue = iRead;
   }

   return find_;
}


/// Read integer value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_int64_g( const char* pbsz, int64_t& iValue )
{
   auto find_ = skip_integer_g( pbsz );                                                            assert( (find_ - pbsz) < 21 ); // assert if number is to big
   if( find_ != pbsz )
   {
      int64_t iRead = 0;
      bool bNegative = false;
      if( *pbsz == '-' ) { bNegative = true; pbsz++; }
      for( ;pbsz != find_; pbsz++ )
      {
         iRead = iRead * 10 + (*pbsz - '0');
      }

      if( bNegative == true ) iRead = -iRead;

      iValue = iRead;
   }

   return find_;
}

/// Read integer value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_int64_g( const char* pbsz, const char* pbszEnd, int64_t& iValue )
{
   auto find_ = skip_integer_g( pbsz, pbszEnd );                                                   assert( (find_ - pbsz) < 21 ); // assert if number is to big
   if( find_ != pbsz )
   {
      int64_t iRead = 0;
      bool bNegative = false;
      if( *pbsz == '-' ) { bNegative = true; pbsz++; }
      for( ;pbsz != find_; pbsz++ )
      {
         iRead = iRead * 10 + (*pbsz - '0');
      }

      if( bNegative == true ) iRead = -iRead;

      iValue = iRead;
   }

   return find_;
}



/// Read double value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_double_g( const char* pbsz, double& dValue )
{
   auto find_ = skip_decimal_g( pbsz );                                                            assert( (find_ - pbsz) < 21 ); // assert if number is to big
   if( find_ != pbsz )
   {
      double dRead = std::strtod( pbsz, nullptr );
      //std::from_chars( pbsz, find_, dRead );  // do not work in clang

      dValue = dRead;
   }

   return find_;
}

/// Read double value from string
/// returns pointer to new position after read, no read and same position is returned
const char* read_double_g( const char* pbsz, const char* pbszEnd, double& dValue )
{
   auto find_ = skip_decimal_g( pbsz, pbszEnd );                                                   assert( (find_ - pbsz) < 21 ); // assert if number is to big
   if( find_ != pbsz )
   {
      double dRead = std::strtod( pbsz, nullptr );
      //std::from_chars( pbsz, find_, dRead ); // do not work in clang

      dValue = dRead;
   }

   return find_;
}

/// Read quoted string and store position and length in string view value
/// returns pointer to new position after read, no read and same position is returned
const char* read_quoted_g( const char* pbsz, const char* pbszEnd, std::string_view& stringValue )
{
   auto find_ = skip_quoted_g( pbsz, pbszEnd );
   if( find_ != pbsz )
   {
      std::size_t uLength = find_ - pbsz;
      uLength -= 2;
      stringValue = std::string_view( pbsz + 1, uLength );
   }

   return find_;
}

/** ---------------------------------------------------------------------------
 * @brief Investigate what type text is
 * @param pbsz pointer to first position in text value that is investigated
 * @param uLength length for text to investigate
 * @param puCheckType 
 * @return 
*/
unsigned read_type_g( const char* pbsz, size_t uLength, const unsigned* puCheckType )
{
   using namespace gd::types;

   auto find_group_ = []( const unsigned* puType, unsigned uGroup ) -> unsigned {
      while( *puType )
      {
         if( (value_group_type_g( *puType ) & uGroup) == uGroup ) // check if group is found
         {
            return *puType;
         }

         puType++;
      }

      return 0;
   };

   unsigned uReturnType = 0;
   unsigned uAnyType = 0;
   const unsigned* puType = puCheckType;
   while( *puType )
   {                                                                                               assert( puType - puCheckType < 20 );   // realistic
      if( gd::types::is_primitive_g( *puType ) == false ) { uAnyType = *puType; break; }

      puType++;
   }

   uint8_t uTypeFlags;
   // ## check characters
   if( uLength < 20 )
   {
      uTypeFlags = 0;      
      const char* pbszEnd = pbsz + uLength;
      for( const char* pbszPosition = pbsz; pbszPosition < pbszEnd; pbszPosition++ )
      {
         uint8_t uCharType = pCharacterClass_s[*pbszPosition];
         if( uCharType == 0 ) return uAnyType;

         uTypeFlags |= uCharType; 
      }
   }
   else return uAnyType;


   // ## check if decimal value is found among types to check for
   if( (uAnyType & ASCII_TYPE_DECIMAL) == ASCII_TYPE_DECIMAL )
   {
      uReturnType = find_group_( puCheckType, eTypeGroupDecimal );
   }
   else if( (uTypeFlags & ASCII_TYPE_INTEGER) == ASCII_TYPE_INTEGER )
   {
      uReturnType = find_group_( puCheckType, eTypeGroupInteger );
   }

   return uReturnType != 0 ? uReturnType : uAnyType;
}

/// Skip alphanumeric or digit characters
/// returns pointer to first non alphanumeric or digit character
const char* skip_alnum_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_ALNUM) ) find_++;

   return find_;
}

/// Skip alphanumeric or digit characters
/// returns pointer to first non alphanumeric or digit character
const char* skip_alnum_g( const char* pbsz, const char* pbszEnd )
{
   auto find_ = pbsz;   
   while( (pbsz != pbszEnd) && (pCharacterClass_s[*find_] & ASCII_TYPE_ALNUM) ) find_++;

   return find_;
}

/// Skip decimal characters
/// returns pointer to first non decimal character
const char* skip_decimal_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_DECIMAL) ) find_++;

   return find_;
}

/// Skip decimal characters
/// returns pointer to first non decimal character
const char* skip_decimal_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   while( (pbsz != pbszEnd) && (pCharacterClass_s[*pbsz] & ASCII_TYPE_DECIMAL) ) pbsz++;

   return pbsz;
}

/// Skip integer characters
/// returns pointer to first non integer character
const char* skip_integer_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_INTEGER) ) find_++;

   return find_;
}

/// Skip integer characters
/// returns pointer to first non integer character
const char* skip_integer_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   while( (pbsz != pbszEnd) && (pCharacterClass_s[*pbsz] & ASCII_TYPE_INTEGER) ) pbsz++;

   return pbsz;
}

/// Skip space characters
/// If not found return pointer is same as input pointer
const char* skip_space_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (pCharacterClass_s[*find_] & ASCII_TYPE_SPACE) ) find_++;

   return find_;
}

/// Skip space characters
/// move pointer to next non space character
const char* skip_space_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   while( (pbsz != pbszEnd) && (pCharacterClass_s[*pbsz] & ASCII_TYPE_SPACE) ) pbsz++;

   return pbsz;
}

/// Skip specified character
/// returns pointer to first character that do not match specified
const char* skip_character_g( const char* pbsz, const char* pbszEnd, char chSkip )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( (pbszEnd - pbsz) < 0x10000000 ); // realistic?
   while( (pbsz != pbszEnd) && (*pbsz == chSkip) ) pbsz++;

   return pbsz;
}

/// Skip string value (moves to ending double quote)
const char* skip_string_g( const char* pbsz )
{
   auto find_ = pbsz;   
   while( (*find_ != '\0') && (*find_ != '\"') ) find_++;

   if( *find_ == '\"' ) return find_;

   return pbsz;
}

/// Skip string value (moves to ending double quote)
/// Returns pointer to last double quote character ending string
const char* skip_string_g( const char* pbsz, tag_json )
{
   auto find_ = pbsz;   
   while( *find_ != '\0' )
   {
      if( pJson_s[*find_] == 0 ) { find_++; continue; }

      if( *find_ == '\\' )
      {
         find_++;
         if( *find_ == '\0' ) break;
      }
      else if( (pJson_s[*find_] & JSON_TYPE_STRING) == JSON_TYPE_STRING )
      {
         return find_;
      }

      find_++;
   }

   return pbsz;
}

/// Skip string value (moves to ending double quote)
/// Returns pointer to last double quote character ending string
const char* skip_string_g( const char* pbsz, const char* pbszEnd, tag_json )
{
   auto find_ = pbsz;   
   while( find_ < pbszEnd )
   {
      if( pJson_s[*find_] == 0 ) { find_++; continue; }

      if( (pJson_s[*find_] & JSON_TYPE_ESCAPE) == JSON_TYPE_ESCAPE )
      {
         find_++;
         if( find_ >= pbszEnd ) break;
      }
      else if( (pJson_s[*find_] & JSON_TYPE_STRING) == JSON_TYPE_STRING )
      {
         return find_;
      }

      find_++;
   }

   return pbsz;
}


/// Skips quoted section, first character in string should be the quote that also ends quoted section
/// move pointer past quoted section, double quoted character escapes one quote
const char* skip_quoted_g( const char* pbsz )
{                                                                                                  assert( *pbsz == '\"' || *pbsz == '\'' || *pbsz == '`' );
   char chQuote = *pbsz;
   pbsz++;
   while( *pbsz != '\0' )
   {
      if( *pbsz != chQuote )
      {
         pbsz++;
      }
      else if( *(pbsz + 1) == chQuote )
      {
         pbsz += 2;
      }
      else
      {
         pbsz++;
         break;
      }
   }

   return pbsz;
}


/// Skips quoted section, first character in string should be the quote that also ends quoted section
/// move pointer past quoted section, double quoted character escapes quote character
/// 
/// @code
/// std::string_view stringTest = "123456'7890''123456'7890";
/// auto p_ = next_character_g( stringTest, '\'' );								 REQUIRE( *p_ == '\'' );
/// auto p1_ = skip_quoted_g( p_, stringTest.data() + stringTest.length(), "{*}" ); REQUIRE( *(p1_ - 1) == '\'' );
/// @endcode
const char* skip_quoted_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( *pbsz == '\"' || *pbsz == '\'' || *pbsz == '`' );
   char chQuote = *pbsz;
   pbsz++;
   while( pbsz < pbszEnd )
   {
      if( *pbsz != chQuote )
      {
         pbsz++;
      }
      else if( *(pbsz + 1) == chQuote )
      {
         pbsz += 2;
      }
      else
      {
         pbsz++;                                                               // move to character after quote
         break;                                                                // end while loop, matching quote is found
      }
   }

   return pbsz;
}


/// Skips quoted section, first character in string should be the quote that also ends quoted section
/// move pointer past quoted section, if quote is escaped with "\"" character it will be skipped
const char* skip_escaped_g( const char* pbsz )
{                                                                                                  assert( *pbsz == '\"' || *pbsz == '\'' || *pbsz == '`' );
   char chQuote = *pbsz;
   pbsz++;
   while( *pbsz != '\0' )
   {
      if( *pbsz != chQuote )
      {
         pbsz++;
      }
      else if( *(pbsz - 1) == '\\' )
      {
         pbsz++;
      }
      else
      {
         pbsz++;
         break;
      }
   }

   return pbsz;
}

/// Skips quoted section, first character in string should be the quote that also ends quoted section
/// move pointer past quoted section, if quote is escaped with "\"" character it will be skipped
const char* skip_escaped_g( const char* pbsz, const char* pbszEnd )
{                                                                                                  assert( *pbsz == '\"' || *pbsz == '\'' || *pbsz == '`' );
   char chQuote = *pbsz;
   pbsz++;
   while( pbsz < pbszEnd )
   {
      if( *pbsz != chQuote )
      {
         pbsz++;
      }
      else if( *(pbsz - 1) == '\\' )
      {
         pbsz++;
      }
      else
      {
         pbsz++;                                                               // move to character after quote
         break;                                                                // end while loop, matching quote is found
      }
   }

   return pbsz;
}




/// skips wild card search pattern if pattern is matched, no match will return original position in search text passed to method
/// 
/// @code
/// std::string_view stringTest = "12345{=id}12345";
/// auto p_ = next_character_g( stringTest, '{' );									 REQUIRE( *p_ == '{' );
/// auto p1_ = skip_wildcard_g( p_, stringTest._Unchecked_end(), "{*}" );	 REQUIRE( *(p1_ - 1) == '}' );
/// @endcode
/// 
/// @code
/// std::string_view stringTest = "12345{{=id}}12345";
/// auto p_ = next_character_g( stringTest, '{' );									 REQUIRE( *p_ == '{' );
/// auto p1_ = skip_wildcard_g( p_, stringTest.data() + stringTest.length(), "{{*}}"); REQUIRE(*(p1_ - 1) == '}' ); REQUIRE( *(p1_ - 2) == '}' );
/// @endcode
const char* skip_wildcard_g( const char* pbsz, const char* pbszEnd, const char* pbszWildcard, unsigned uLength )
{                                                                                                  assert( pbsz <= pbszEnd ); assert( uLength < 1000 ); // realistic values
   const char* pbszPosition = pbsz;
   const char* pbszWildcardEnd = pbszWildcard + uLength;
   while( pbszWildcard < pbszWildcardEnd )                                     // more wild-card pattern ?
   {
      if( *pbszWildcard == '*' )                                               // * = any character 1 or more times until next non wild card character
      {
         pbszWildcard++;                                                                           assert( *pbszWildcard != '*' && *pbszWildcard != '?' ); // shouldn't place two wild cards together with *
         char chFind = *pbszWildcard;
         while( pbszPosition < pbszEnd && *pbszPosition != chFind ) pbszPosition++; // find next non wild card character or end of search text
      }
      else if( *pbszWildcard == '?' )
      {
         pbszWildcard++;                                                                           assert( *pbszWildcard != '*' ); // don't place two wild cards together with *
         if( pbszPosition < pbszEnd ) pbszPosition++;
      }
      else
      {                                                                                            assert( *pbszWildcard != '*' && *pbszWildcard != '?' ); // no wild card here
         if( *pbszWildcard == *pbszPosition )                                  // match character, if same move to next
         {
            pbszWildcard++;
            pbszPosition++;
         }
         else
         {
            return pbsz;                                                       // no wild card pattern match, return start position in search string   
         }
      }
   }

   return pbszPosition;
}

/** ---------------------------------------------------------------------------
 * @brief read words from line, each name is separated with space
 * @code
 * std::vector<std::string_view> vectorWord;
 * std::string stringLine = "one two three four five";
 * gd::parse::read_line_g( stringLine.c_str(), stringLine.c_str() + stringLine.length(), vectorWord );
 * assert( vectorWord.size() == 5 );
 * @endcode
 * @param pbsz start of line
 * @param pbszEnd end of line
 * @param vectorWord vector where read word positions are stored
 * @return true if ok, false and error information if error
*/
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::string_view>& vectorWord )
{
   const char* pbszPosition = pbsz; // position in line
   while( pbszPosition < pbszEnd )
   {
      if( pCharacterClass_s[(uint8_t)*pbszPosition] & ASCII_TYPE_SPACE )
      {
         while( pbszPosition < pbszEnd && (pCharacterClass_s[(uint8_t)*pbszPosition] & ASCII_TYPE_SPACE) != 0 ) pbszPosition++;
      }
      else
      {  // ## read and store word in vector
         const char* pbszBegin = pbszPosition;
         while( pbszPosition < pbszEnd && (pCharacterClass_s[(uint8_t)*pbszPosition] & ASCII_TYPE_SPACE) == 0 ) pbszPosition++;

         size_t uLength = pbszPosition - pbszBegin;
         vectorWord.push_back( { pbszBegin, uLength } );
      }
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief read words from line, each name is separated with space
 * @param pbsz start of line
 * @param pbszEnd end of line
 * @param vectorWord vector where read words are stored
 * @return true if ok, false and error information if error
*/
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::string>& vectorWord )
{
   const char* pbszPosition = pbsz; // position in line
   while( pbszPosition < pbszEnd )
   {
      if( pCharacterClass_s[(uint8_t)*pbszPosition] & ASCII_TYPE_SPACE )
      {
         while( pbszPosition < pbszEnd && (pCharacterClass_s[(uint8_t)*pbszPosition] & ASCII_TYPE_SPACE) != 0 ) pbszPosition++;
      }
      else
      {  // ## read and store word in vector
         const char* pbszBegin = pbszPosition;
         while( pbszPosition < pbszEnd && (pCharacterClass_s[(uint8_t)*pbszPosition] & ASCII_TYPE_SPACE) == 0 ) pbszPosition++;

         size_t uLength = pbszPosition - pbszBegin;
         vectorWord.push_back( { pbszBegin, uLength } );
      }
   }

   return { true, pbszPosition };
}

/** ---------------------------------------------------------------------------
 * @brief read words enclosed in characters marked as start and end for word
 * @param pbsz start of line
 * @param pbszEnd line end
 * @param chBeginMarker character that marks start for word
 * @param chEndMarker character that marks end for word
 * @param vectorWord list of read words
 * @return true if ok, false and position in line if error
 */
std::pair<bool, const char*> read_line_g(const char* pbsz, const char* pbszEnd, char chBeginMarker, char chEndMarker, std::vector<std::string_view>& vectorWord)
{
   const char* pbszPosition = pbsz; // position in line
   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chBeginMarker ) { pbszPosition++; }
      else
      {
         pbszPosition++;
         const char* pbszBegin = pbszPosition;
         while(pbszPosition < pbszEnd && *pbszPosition != chEndMarker)
         {
            pbszPosition++;
         }

         if( *pbszPosition == chEndMarker )
         { 
            size_t uLength = pbszPosition - pbszBegin;
            vectorWord.push_back( { pbszBegin, uLength } );
            pbszPosition++;
         }
         else { return { false, pbszBegin }; }
      }
   }

   return { true, pbszPosition };
}


/** ---------------------------------------------------------------------------
 * @brief write templated buffer text to string
 * @param pbsz start of templated buffer
 * @param pbszEnd end of buffer
 * @param chBeginMarker character that marks start of template argument
 * @param chEndMarker end of template argument
 * @param argumentsValue 
 * @param stringTo 
 * @return 
 */
std::pair<bool, const char*> write_line_g(const char* pbsz, const char* pbszEnd, char chBeginMarker, char chEndMarker, const argument::arguments& argumentsValue, std::string& stringTo)
{
   unsigned uCount = 0;
   const char* pbszPosition = pbsz; // position in line
   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chBeginMarker ) 
      { 
         stringTo += *pbszPosition;
         pbszPosition++; 
      }
      else
      {
         pbszPosition++;
         const char* pbszBegin = pbszPosition;
         while(pbszPosition < pbszEnd && *pbszPosition != chEndMarker)
         {
            pbszPosition++;
         }

         if( *pbszPosition == chEndMarker )
         { 
            size_t uLength = pbszPosition - pbszBegin;
            std::string_view stringName( pbszBegin, uLength );
            if( stringName.empty() == false )
            {
               stringTo += argumentsValue[stringName].as_string();
            }
            else
            {
               stringTo += argumentsValue[uCount].as_string();
            }
            pbszPosition++;
            uCount++;
         }
         else { return { false, pbszBegin }; }
      }
   }

   return { true, pbszPosition };
}

/** ---------------------------------------------------------------------------
 * @brief read values in line into vector
 * 
 * @code
    using namespace gd::table::dto;
std::string stringCsvCodeGroup = R"(
1,Group1,Group1 description
2,Group2,Group2 description
3,Group3,Group3 description
)";

gd::table::dto::table tableCodeGroup( (table::eTableFlagNull32|table::eTableFlagRowStatus), 
                                      { { "int64", "GroupK"}, 
                                        { "rstring", "FName"}, 
                                        { "rstring", "FDescription"} }, gd::table::tag_prepare{} );

auto vectorType = tableCodeGroup.column_get_type();
gd::table::read_g( tableCodeGroup, stringCsvCodeGroup, ',', '\n', gd::table::tag_io_csv{} );

stringCsvCodeGroup = R"(
4, "G" ,"no description"
10, "group 10" ,no quote
20000, "end group","unknown"
)";

gd::table::read_g( tableCodeGroup, stringCsvCodeGroup, ',', '\n', gd::table::tag_io_csv{} );

std::cout << gd::table::debug::print( tableCodeGroup );
 * 
 * @endcode
 * 
 * @param pbsz start of line to read
 * @param pbszTextEnd end of text
 * @param vectorValue values are stored in this vector
 * @param vectorType expected types to find in line that is read
 * @param csv rules for csv file
 * @return true if line value is read, false and position if error
*/
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszTextEnd, std::vector<gd::variant_view>& vectorValue, std::vector<unsigned> vectorType, const csv& csv, std::function<bool( gd::variant_view&, unsigned )> callback_ )
{
   // bool bLineEndFound = false;      // marks if we have passed lineend character
   const auto* pbszPosition = pbsz; // current possition

   pbszPosition = skip_space_g( pbszPosition, pbszTextEnd );                       // move to first non space character

   // ## Find end of line
   const char* pbszEnd = strchr( pbszPosition, pbszTextEnd, csv.get_lineend(), csv );              assert( pbszEnd == nullptr || *pbszEnd == csv.get_lineend() );
   if( pbszEnd == nullptr ) { pbszEnd = pbszTextEnd; }

   unsigned uIndex = 0;
   for( auto it = std::begin( vectorType ), itEnd = std::end( vectorType ); it != itEnd && pbszPosition < pbszEnd; it++ )
   {
      auto pbszValueStart = pbszPosition;                                      // start position for value
      auto uType = *it;
      gd::variant_view v_;
      if( uType & gd::types::eTypeGroupDecimal )
      {
         double dValue = 0;
         pbszPosition = read_double_g( pbszPosition, pbszEnd, dValue );
         if( pbszValueStart != pbszPosition ) v_ = dValue;
      }
      else if( uType & gd::types::eTypeGroupInteger )
      {
         int64_t iValue = 0;
         pbszPosition = read_int64_g( pbszPosition, pbszEnd, iValue );
         if( pbszValueStart != pbszPosition ) v_ = iValue;
      }
      else if( uType & gd::types::eTypeGroupString )
      {
         if( csv.is_quote( *pbszPosition ) == true )
         {
            std::string_view stringValue;
            pbszPosition = read_quoted_g( pbszPosition, pbszEnd, stringValue );
            if( pbszValueStart != pbszPosition ) v_ = stringValue;
         }
         else
         {
            const char* pbszDelimiter = strchr( pbszPosition, pbszEnd, csv.get_delimiter() );
            if(pbszDelimiter == nullptr) pbszDelimiter = pbszEnd;

            v_ = std::string_view( pbszPosition, pbszDelimiter - pbszPosition ); // set string to variant view value
            pbszPosition = pbszDelimiter;                                   // set position to end of value
         }
      }
      else if( uType & gd::types::eTypeGroupBoolean )
      {
         bool bValue = false;
         pbszPosition = read_boolean_g( pbszPosition, pbszEnd, bValue );
         if( pbszValueStart != pbszPosition ) v_ = bValue;
      }

      if( callback_ )                                                          // if callback the preprocess value before adding
      {
         if( callback_( v_, uIndex ) == false ) return { false, pbszValueStart };
      }

      vectorValue.push_back( v_ );
      uIndex++;

      // move past delimiter if it is on position
      if( *pbszPosition == csv.get_delimiter() && pbszPosition < pbszEnd ) pbszPosition++;

      /*
      auto pbszNext = csv.next_value( pbszPosition, pbszEnd );
      if( pbszNext == nullptr )
      {
         // ## compare if line end is found
         pbszPosition = skip_character_g( pbszPosition, pbszEnd, ' ');
         if( *pbszPosition == csv.get_lineend() )                              // found end of line? <-- important !!
         {
            pbszPosition++;
            bLineEndFound = true;                                              // mark that we have found lineend character
            break;                                                             // breaks out of loop - found newline
         }
         else
         {
            pbszPosition = skip_space_g( pbszPosition, pbszEnd );
         }
      }
      else
      {
         pbszPosition = pbszNext;
      }
      */
   }


   // ## check if we have passed end of line
   //    We are only reading the amount of values sent to method, because of this
   //    we might be "ready" reading values before lineend is found. Therefore we
   //    need to move past lineend if lineend isn't passed.
   if( pbszPosition != nullptr && pbszPosition < pbszTextEnd )
   {
      pbszPosition = pbszEnd;
      if( pbszPosition != nullptr && pbszPosition < pbszTextEnd && *pbszPosition == csv.get_lineend() ) pbszPosition++;
   }

   return { true, pbszPosition };
}

/** ---------------------------------------------------------------------------
 * @brief 
 * @param pbsz 
 * @param pbszEnd 
 * @param vectorValue 
 * @param csv 
 * @return 
 */
std::pair<bool, const char*> read_line_g(const char* pbsz, const char* pbszEnd, std::vector<std::string>& vectorValue, const csv& csv)
{
   const auto* pbszPosition = pbsz;

   if(pbszEnd == nullptr)
   {
      pbszEnd = strchr( pbsz, csv.get_lineend(), csv );
      if( pbszEnd == nullptr ) pbszEnd = pbsz + strlen( pbsz );
   }

   pbszPosition = skip_space_g( pbszPosition, pbszEnd );                       // move to first non space character
   while(pbszPosition < pbszEnd)
   {
      if( *pbszPosition != csv.get_delimiter() )
      {
         std::string stringText;
         const char* pbszDelimiter;
         pbszPosition = skip_space_g( pbszPosition, pbszEnd );                 // move to first non space character
         if( *pbszPosition != csv.get_quote() )                                // not a quote character
         {
            const char* pbszTo = strchr( pbszPosition, pbszEnd, csv.get_delimiter() );// move to last character
            if( pbszTo == nullptr ) pbszTo = pbszEnd;
            
            pbszDelimiter = pbszTo;

            // ## go back to first visible character
            pbszDelimiter--;
            while( pbszDelimiter > pbszPosition && *pbszDelimiter == ' ' ) pbszDelimiter--;

            while(pbszPosition <= pbszDelimiter)
            {
               stringText += *pbszPosition;
               pbszPosition++;
            }

            pbszPosition = pbszTo;
            if( *pbszPosition == csv.get_delimiter() ) pbszPosition++;
            vectorValue.push_back( std::move( stringText ) );
         }
      }
      else
      {
         std::string stringText;
         std::string_view stringRead;
         pbszPosition = read_quoted_g( pbszPosition, pbszEnd, stringRead );
         for( auto it = std::begin( stringRead ), itEnd = std::end( stringRead ); it != itEnd; it++ )
         {
            stringText += *it;
            if( *it == csv.get_quote() ) it++;
         }

         vectorValue.push_back( std::move( stringText ) );
      }
   }

   return { true, pbszPosition };
}



/** ---------------------------------------------------------------------------
 * @brief read line from test pointers are positioned at
 * @param pbsz pointer to start of text line is read
 * @param pbszEnd pointer to end of text
 * @param stringLine reference to string_view that gets line
 * @param pbszNewLine what marks a new line
 * @return const char* pointer to end of line if line was found
*/
const char* get_line_g( const char* pbsz, const char* pbszEnd, std::string_view& stringLine, const char* pbszNewLine )
{
   unsigned uLength = (unsigned)strlen( pbszNewLine );
   const char* pbszPosition = pbsz;

   const char* pbszFind = strstr( pbszPosition, pbszEnd, pbszNewLine, uLength, nullptr );

   if( pbszFind != nullptr )
   {
      stringLine = std::string_view( pbsz, pbszFind - pbsz );
   }

   return pbszFind;
}

/** ---------------------------------------------------------------------------
 * @brief read line from test pointers are positioned at
 * @param pbsz pointer to start of text line is read
 * @param pbszEnd pointer to end of text
 * @param stringLine reference to string_view that gets line
 * @param pbszNewLine what marks a new line
 * @param csv rules for reading line, this has information to skip parts where end may exist but is passed
 * @return const char* pointer to end of line if line was found
*/
const char* get_line_g( const char* pbsz, const char* pbszEnd, std::string_view& stringLine, const char* pbszNewLine, const csv& csv )
{
   unsigned uLength = (unsigned)strlen( pbszNewLine );
   const char* pbszPosition = pbsz;

   const char* pbszFind = strstr( pbszPosition, pbszEnd, pbszNewLine, uLength, csv, nullptr );

   if( pbszFind != nullptr )
   {
      stringLine = std::string_view( pbsz, pbszFind - pbsz );
   }

   return pbszFind;
}

/// find line in string
const char* get_line_g( const char* pbsz, const char* pbszEnd, std::string_view& stringLine, const csv& csv )
{
   const char* pbszPosition = pbsz;

   const char* pbszFind = std::strchr( pbszPosition, csv.get_lineend() );

   if( pbszFind != nullptr )
   {
      stringLine = std::string_view( pbsz, pbszFind - pbsz );
   }

   return pbszFind;
}


/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` but do not search to more than pbszEnd
 * @param pbszText text to search for character in
 * @param chFind character to find
 * @return pointer to position where character was found, null if not found
*/
const char* strchr( const char* pbszText, char chFind )
{
   const char* pbszPosition = pbszText;

   while( *pbszPosition != '\0' && *pbszPosition != chFind ) pbszPosition++;

   if( *pbszPosition == chFind ) return pbszPosition;

   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` but do not search to more than pbszEnd
 * @param pbszText text to search for character in
 * @param pbszEnd end position to stop search
 * @param chFind character to find
 * @return pointer to position where character was found, null if not found
*/
const char* strchr( const char* pbszText, const char* pbszEnd, char chFind )
{
   const char* pbszPosition = pbszText;

   while( pbszPosition < pbszEnd && *pbszPosition != chFind ) pbszPosition++;

   if( *pbszPosition == chFind ) return pbszPosition;

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` except here we are using line parsing rules used for csv files
 * @param pbszText text to search within
 * @param chFind character to find
 * @param csv csv object with rules on how to move in text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to character classes to know how to interpret characters 
*/
const char* strchr( const char* pbszText, char chFind, const csv& csv, const uint8_t* puCharacterClass )
{
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszText;

   while( *pbszPosition != '\0' && *pbszPosition != chFind )
   {
      if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( csv.is_quote( *pbszPosition ) == true )
         {
            // ## found quote, text within quote is skipped
            pbszPosition = skip_quoted_g( pbszPosition );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if( *pbszPosition == chFind ) return pbszPosition;

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` except here we are using line parsing rules used for csv files
 * @param pbszBegin start position for text to find character within
 * @param pbszEnd last position in text
 * @param chFind character to find
 * @param csv csv object with rules on how to move in text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to character classes to know how to interpret characters 
*/
const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const csv& csv, const uint8_t* puCharacterClass )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;

   while( pbszPosition < pbszEnd && *pbszPosition != chFind )
   {
      if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( csv.is_quote( *pbszPosition ) == true )
         {
            // ## found quote, text within quote is skipped
            pbszPosition = skip_quoted_g( pbszPosition );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if( *pbszPosition == chFind ) return pbszPosition;

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief find character that is used to scope text, that menas that there is start and end character to contain other text
 * @param pbszBegin where to start searching
 * @param pbszEnd where to end searching
 * @param chFind character to find
 * @param chScopeFind matching character that starts a new scope
 * @return pointer to found character or nullptr if not found
 */
const char* strchr(const char* pbszBegin, const char* pbszEnd, char chFind, char chScopeFind, tag_scope)
{
   int iLevel = 0;
   const char* pbszPosition = pbszBegin;
   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         if(*pbszPosition != chScopeFind)
         {
            pbszPosition++;
            continue;
         }

         iLevel++;
         pbszPosition++;
         continue;
      }
      else
      {
         if(iLevel == 0)
         {
            return pbszPosition;
         }

         iLevel--;
         pbszPosition++;
      }
   }
   return nullptr;
}



/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` except here we are using line parsing rules used for sql strings
 * @param pbszText text to search within
 * @param chFind character to find
 * @param sql sql object with rules on how to move in text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to character classes to know how to interpret characters 
*/
const char* strchr( const char* pbszText, char chFind, const sql& sql, const uint8_t* puCharacterClass )
{
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszText;

   while( *pbszPosition != '\0' && *pbszPosition != chFind )
   {
      if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( sql.is_quote( *pbszPosition ) == true )
         {
            // ## found quote, text within quote is skipped
            pbszPosition = skip_quoted_g( pbszPosition );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if( *pbszPosition == chFind ) return pbszPosition;

   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` except here we are using line parsing rules used for sql files
 * @param pbszBegin start position for text to find character within
 * @param pbszEnd last position in text
 * @param chFind character to find
 * @param sql sql object with rules on how to move in text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to character in text if found, nullptr if not found
*/
const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const sql& sql, const uint8_t* puCharacterClass )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;  // position in text

   while( pbszPosition < pbszEnd && *pbszPosition != chFind )
   {
      if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( sql.is_quote( *pbszPosition ) == true )
         {
            // ## found quote, text within quote is skipped
            pbszPosition = skip_quoted_g( pbszPosition );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if( *pbszPosition == chFind ) return pbszPosition;                          // found character? return pointer

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` except here we are using line parsing rules used for sql strings
 * @param pbszText text to search within
 * @param chFind character to find
 * @param json_rule json_rule object with rules on how to move in text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to character classes to know how to interpret characters 
*/
const char* strchr( const char* pbszText, char chFind, const json_rule& json_rule, const uint8_t* puCharacterClass )
{
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszText;

   while( *pbszPosition != '\0' && *pbszPosition != chFind )
   {
      if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( json_rule.is_quote( *pbszPosition ) == true )
         {
            // ## found quote, text within quote is skipped
            pbszPosition = skip_escaped_g( pbszPosition );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if( *pbszPosition == chFind ) return pbszPosition;

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief find character similar to c-method `strchr` except here we are using line parsing rules used for sql files
 * @param pbszBegin start position for text to find character within
 * @param pbszEnd last position in text
 * @param chFind character to find
 * @param json_rule json_rule object with rules on how to move in text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to character in text if found, nullptr if not found
*/
const char* strchr( const char* pbszBegin, const char* pbszEnd, char chFind, const json_rule& json_rule, const uint8_t* puCharacterClass )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;  // position in text

   while( pbszPosition < pbszEnd && *pbszPosition != chFind )
   {
      if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( json_rule.is_quote( *pbszPosition ) == true )
         {
            // ## found quote, text within quote is skipped
            pbszPosition = skip_escaped_g( pbszPosition );
         }
         else
         {
            pbszPosition++;
            continue;
         }
      }
   }

   if( *pbszPosition == chFind ) return pbszPosition;                          // found character? return pointer

   return nullptr;
}



/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of the byte string needle in the byte string pointed to by haystack.
 * @param pbszBegin start of text (haystack) to find string (needle) in 
 * @param pbszEnd end of text
 * @param pbszFind pointer to string that is searched for within text
 * @param uLength length of string
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to string in text if found, nullptr if not found
*/
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const uint8_t* puCharacterClass )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find
   
   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         pbszPosition++;
         continue;
      }
      else
      {
         if( uLength == 1 || memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 ) return pbszPosition; // found text? return pointer to text

         pbszPosition++;
      }
   }

   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of the byte string needle in the byte string pointed to by haystack.
 * @param pbszBegin start of text (haystack) to find string (needle) in 
 * @param pbszEnd end of text
 * @param pbszFind pointer to string that is searched for within text
 * @param uLength length of string
 * @param csv parse rules, adapted to sql query format
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to string in text if found, nullptr if not found
*/
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const csv& csv, const uint8_t* puCharacterClass )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find
   
   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
         {
            pbszPosition++;
            continue;
         }
         else
         {
            if( csv.is_quote( *pbszPosition ) == true )
            {
               // ## found quote, text within quote is skipped
               pbszPosition = skip_quoted_g( pbszPosition );
            }
            else
            {
               pbszPosition++;
               continue;
            }
         }
      }
      else
      {
         if( uLength == 1 || memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 ) return pbszPosition; // found text? return pointer to text

         pbszPosition++;
      }
   }

   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of the byte string needle in the byte string pointed to by haystack.
 * @param pbszBegin start of text (haystack) to find string (needle) in 
 * @param pbszEnd end of text
 * @param pbszFind pointer to string that is searched for within text
 * @param uLength length of string
 * @param sql parse rules, adapted to sql query format
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to string in text if found, nullptr if not found
*/
const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const sql& sql, const uint8_t* puCharacterClass )
{                                                                                                  assert( pbszBegin <= pbszEnd );
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find
   
   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
         {
            pbszPosition++;
            continue;
         }
         else
         {
            if( sql.is_quote( *pbszPosition ) == true )
            {
               // ## found quote, text within quote is skipped
               pbszPosition = skip_quoted_g( pbszPosition );
            }
            else
            {
               pbszPosition++;
               continue;
            }
         }
      }
      else
      {
         if( memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 ) return pbszPosition; // found text? return pointer to text

         pbszPosition++;
      }
   }

   return nullptr;
}

const char* strstr( const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const sql& sql, const uint8_t* puCharacterClass, tag_wildcard )
{
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find
   
   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
         {
            pbszPosition++;
            continue;
         }
         else
         {
            if( sql.is_quote( *pbszPosition ) == true )
            {
               // ## found quote, text within quote is skipped
               pbszPosition = skip_quoted_g( pbszPosition );
            }
            else
            {
               pbszPosition++;
               continue;
            }
         }
      }
      else
      {
         if( memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 ) return pbszPosition; // found text? return pointer to text

         pbszPosition++;
      }
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Find string in json formated text (strings are skipped and comments is not supported)
 * @param pbszBegin start of json text
 * @param pbszEnd end of json text
 * @param pbszFind start of string to find
 * @param uLength length of string to find
 * @param json_rule json logic to know how to find in json text
 * @param puCharacterClass ascii text block (256 bytes) with character classes or null to use default
 * @return pointer to name if found, nullptr if not found
 */
const char* strstr(const char* pbszBegin, const char* pbszEnd, const char* pbszFind, unsigned uLength, const json_rule& json_rule, const uint8_t* puCharacterClass)
{
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszPosition = pbszBegin;   // position in text
   char chFind = *pbszFind;                // first character in text to find

   pbszFind++;    // No need to compare first character
   uLength--;     // decrease length that is used when we compare rest after first character has been found

   while( pbszPosition < pbszEnd )
   {
      if( *pbszPosition != chFind )
      {
         if( !(puCharacterClass[*pbszPosition] & ASCII_TYPE_QUOTE) )
         {
            pbszPosition++;
            continue;
         }
         else
         {
            if( json_rule.is_quote( *pbszPosition ) == true )
            {
               // ## found quote, text within quote is skipped
               pbszPosition = skip_escaped_g( pbszPosition );
            }
            else
            {
               pbszPosition++;
               continue;
            }
         }
      }
      else
      {
         if( memcmp( pbszPosition + 1, pbszFind, uLength ) == 0 ) return pbszPosition; // found text? return pointer to text

         pbszPosition++;
      }
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Copy char buffer into string, stops copying when buffer ends or stop character is found
 * @param pbszText text to copy from
 * @param stringCopy string to copy to
 * @param chCopyTo stop copy
 * @return position to text position where copying ended
 */
const char* strcpy( const char* pbszText, std::string& stringCopy, char chCopyTo ) 
{
   const char* pbszPosition = pbszText;
   while( *pbszPosition != '\0' && *pbszPosition != chCopyTo )
   {
      stringCopy += *pbszPosition;
      pbszPosition++;
   }
    
   return pbszPosition;
}

/** ---------------------------------------------------------------------------
 * @brief Copy char buffer into string, stops copying when buffer ends or stop character is found
 * @param pbszText text to copy from
 * @param stringCopy string to copy to
 * @param vectorCopyTo vector with characters to check for if stop copying
 * @return position to text position where copying ended
 */
const char* strcpy( const char* pbszText, std::string& stringCopy, const std::vector<char>& vectorCopyTo )
{
   bool bCopy = true;
   const char* pbszPosition = pbszText;
   while( bCopy == true && *pbszPosition != '\0' )
   {
      for( auto it : vectorCopyTo )
      {
         if( *pbszPosition == it ) { bCopy = false; }
      }

      if( bCopy == true ) { stringCopy += *pbszPosition; }

      pbszPosition++;
   }
    
   return pbszPosition;
}


void strchr_for_each( const std::string_view& stringSql, char chFind, const sql& sql, const uint8_t* puCharacterClass, std::function<const char*( const std::string_view&, int )> callback_ )
{                                                                                                  
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszStart = stringSql.data();
   const char* pbszEnd = stringSql.data() + stringSql.length();
   int iIndex = 0;
   const char* pbszFrom = pbszStart;
   while( const char* pbszFind = gd::parse::strchr( pbszFrom, pbszEnd, chFind, sql, puCharacterClass ) )
   {
      auto pbszNnext = callback_( std::string_view( pbszFrom, pbszFind - pbszFrom ), iIndex );
      iIndex++;
      if( pbszNnext == nullptr ) pbszFrom = pbszFind + 1;                      // move over found character as default if no new position is returned
      else pbszFrom = pbszNnext;                                                                   assert( pbszFrom <= pbszEnd );// next position to search for
   }

   if( pbszFrom < pbszEnd )
   {
      callback_( std::string_view( pbszFrom, pbszEnd - pbszFrom ), -1 );
   }
}


void strstr_for_each( const std::string_view& stringSql, const char* pbszFind, unsigned uLength, const sql& sql, const uint8_t* puCharacterClass, std::function<void( const std::string_view&, int )> callback_ )
{
   if( puCharacterClass == nullptr ) puCharacterClass = pCharacterClass_s;

   const char* pbszStart = stringSql.data();
   const char* pbszEnd = stringSql.data() + stringSql.length();
   int iIndex = 0;
   const char* pbszFrom = pbszStart;
   while( const char* pbszPosition = gd::parse::strstr( pbszFrom, pbszEnd, pbszFind, uLength, sql, puCharacterClass ) )
   {
      callback_( std::string_view( pbszFrom, pbszPosition - pbszFrom ), iIndex );
      iIndex++;
      pbszFrom = pbszPosition + uLength;
   }

   if( pbszFrom < pbszEnd )
   {
      callback_( std::string_view( pbszFrom, pbszEnd - pbszFrom ), -1 );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Read key value pairs in query string
 * Query string format is like this `"key1=value1&key2=value2"`.
 * @param pbsz start position for string holding key value pairs
 * @param pbszEnd last position for string holding key value pairs
 * @param vectorValue vectur values are added to
 * @param querystring query string object with rules for how to extract key value pairs
 * @return true if ok, false and buffer position for error if failed
*/
std::pair<bool, const char*> read_line_g( const char* pbsz, const char* pbszEnd, std::vector<std::pair<std::string_view, std::string_view>>& vectorValue, const querystring& querystring )
{
   std::string_view stringKey;
   std::string_view stringValue;
   enum { key, value } eValueType = key;

   auto pbszPosition = pbsz;
   while( pbszPosition < pbszEnd )
   {
      char chToken = *pbszPosition; 
      if( querystring.is_name( chToken ) == true )
      {
         eValueType = key;
         pbszPosition++;
      }
      else if( querystring.is_value( chToken ) == true )
      {                                                                                            assert( eValueType == key );
         eValueType = value;
         pbszPosition++;
      }
      else
      {
         auto pbszFirst = pbszPosition;
         auto stop_ = eValueType == key ? querystring.get_value() : querystring.get_name();
         pbszPosition = next_character_or_end_g( pbszPosition, pbszEnd, stop_ );

         if( eValueType == key ) stringKey = std::string_view{ pbszFirst, size_t(pbszPosition - pbszFirst) };
         else if( eValueType == value )
         {
            if( stringKey.empty() == false )
            {
               stringValue = std::string_view{ pbszFirst, size_t( pbszPosition - pbszFirst ) };
               vectorValue.push_back( { stringKey, stringValue } );
               stringKey = std::string_view{};
            }
            else
            {
               return { false, pbszPosition };
            }
         }
      }
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Read key value pairs in query string
 * @code
std::string stringUrl = "h ttp://localhost:8080/script?name=system%2Fdatabase-list-users&session=00000000000000000000000000000000";
auto position_ = stringUrl.find( '?' );
if( position_ != std::string::npos ) stringUrl = stringUrl.substr( position_ + 1 );

std::vector<std::pair<std::string,std::string>> vectorQueryString;
gd::parse::read_line_g( stringUrl, vectorQueryString, gd::parse::querystring(), []( const std::string_view& stringKey, const std::string_view& stringValue ) {
	std::string stringFormatedValue;
	gd::utf8::uri::convert_uri_to_uf8( stringValue, stringFormatedValue );// convert from uri to utf8, values are uri formated
	return std::pair{ std::string( stringKey ), stringFormatedValue };
});
 * @endcode
 * @param pbsz start position for string holding key value pairs
 * @param pbszEnd last position for string holding key value pairs
 * @param vectorValue vectur values are added to
 * @param querystring query string object with rules for how to extract key value pairs
 * @param format_ callback for custom formating
 * @return true if ok, false and buffer position for error if failed
*/
std::pair<bool, const char*> read_line_g(
   const char* pbsz,
   const char* pbszEnd,
   std::vector<std::pair<std::string, std::string>>& vectorValue,
   const querystring& querystring,
   std::function< std::pair< std::string, std::string >( std::string_view, std::string_view )> format_ )
{
   std::string_view stringKey;
   std::string_view stringValue;
   enum { key, value } eValueType = key;

   auto pbszPosition = pbsz;
   while( pbszPosition < pbszEnd )
   {
      char chToken = *pbszPosition;
      if( querystring.is_name( chToken ) == true )
      {
         eValueType = key;
         pbszPosition++;
      }
      else if( querystring.is_value( chToken ) == true )
      {
         assert( eValueType == key );
         eValueType = value;
         pbszPosition++;
      }
      else
      {
         auto pbszFirst = pbszPosition;
         auto stop_ = eValueType == key ? querystring.get_value() : querystring.get_name();
         pbszPosition = next_character_or_end_g( pbszPosition, pbszEnd, stop_ );

         if( eValueType == key ) stringKey = std::string_view{ pbszFirst, size_t( pbszPosition - pbszFirst ) };
         else if( eValueType == value )
         {
            if( stringKey.empty() == false )
            {
               stringValue = std::string_view{ pbszFirst, size_t( pbszPosition - pbszFirst ) };

               if( format_ )
               {
                  auto pairKeyValue = format_( stringKey, stringValue );
                  vectorValue.push_back( std::move( pairKeyValue ) );
               }
               else
               {
                  vectorValue.push_back( { std::string( stringKey ), std::string( stringValue ) } );
               }

               stringKey = std::string_view{};
            }
            else
            {
               return { false, pbszPosition };
            }
         }
      }
   }

   return { true, "" };
}

// ## split methods

/** ---------------------------------------------------------------------------
 * @brief 
 * @param stringText 
 * @param stringSplit 
 * @param vectorPart 
 * @param csv 
 */
void split_g(const std::string_view &stringText,
             const std::string_view &stringSplit,
             std::vector<std::string> &vectorPart, const csv &csv) {
    assert(stringSplit.length() > 0);
    std::string stringPart;              // Store string parts added to vector
    auto uLength = stringSplit.length(); // Split string lenght
    const uint8_t *pubszSplitWith = reinterpret_cast<const uint8_t *>(
        stringSplit.data()); // help compiler to optimize ?

    const uint8_t *pubszPosition =
        reinterpret_cast<const uint8_t *>(stringText.data()); // start of text
    const uint8_t *pubszTextEnd = reinterpret_cast<const uint8_t *>(
        stringText.data() + stringText.length()); // end of text
    while (pubszPosition != pubszTextEnd) {
        assert(pubszPosition < pubszTextEnd);
        assert(*pubszPosition != 0);
        // No split character?
        if (*pubszPosition != *pubszSplitWith) {
            if (csv.is_quote(*pubszPosition) == true) // check for csv quote
            { // csv quote is found, read complete quoted value
                std::string_view stringValue;
                pubszPosition = (const uint8_t *)read_quoted_g(
                    (char *)pubszPosition, (char *)pubszTextEnd, stringValue);
                if (pubszPosition != pubszTextEnd) {
                    pubszPosition++; // move past separator (otherwise it is
                                     // some sort of format error)
                    vectorPart.emplace_back(
                        stringValue); // append value to vector
                }
            } else {
                const char *pbegin_ = (const char *)pubszPosition;
                while (pubszPosition != pubszTextEnd &&
                       *pubszPosition != *pubszSplitWith)
                    pubszPosition++;

                stringPart.append(
                    pbegin_,
                    pubszPosition -
                        decltype(pubszPosition)(
                            pbegin_)); // add value to string part that is added
                                       // when splitter is found
            }
        }
        // Compare if split text sequence is found
        else if (*pubszPosition == *pubszSplitWith) {
            bool bFoundSplitter = true;
            // if only one character is used to split than skip all these
            // characters if there are more than one
            if (uLength == 1) {
                while (pubszPosition != pubszTextEnd &&
                       *pubszPosition == *pubszSplitWith)
                    pubszPosition++;
            } else if (memcmp((void *)pubszPosition, (void *)pubszSplitWith,
                              uLength) == 0) {
                pubszPosition += uLength;
            } else {
                bFoundSplitter = false;
                stringPart += *pubszPosition;
            }

            if (bFoundSplitter == true) // if splitter is found then add string
                                        // part to vector and clear it
            {
                vectorPart.emplace_back(stringPart);
                stringPart.clear();
            }
        }
    }

    if (stringPart.empty() == false) {
        vectorPart.emplace_back(stringPart);
    } // add last part if any
}

/** ---------------------------------------------------------------------------
 * @brief Escapes csv text characters that have special meaning within csv formated text
 * Sample: The string `123"456"789` will be escaped to `123""456""789`
 * @param stringText string to escape
 */
void escape_g( std::string& stringText, tag_csv )
{
   std::string stringNew;
   stringNew.reserve( stringText.length() );
   for( auto it : stringText )
   {
      if( it > '"' ) stringNew += it;
      else
      {
         if( it == '"' ) stringNew += std::string_view{ "\"\"" };
         else if( it == '\n' ) stringNew += std::string_view{ "\\\n" };
         else stringNew += it;
      }
   }

   stringText = std::move( stringNew );
}

/** ---------------------------------------------------------------------------
 * @brief Escapes sql text characters that have special meaning within sql formated text
 * In sql string containing ' characters, they are duplicated so instead of one ' you get two like this ''
 * Sample: The string `123'456'789` will be escaped to `123''456''789`
 * @param stringText string to escape
 */
void escape_g( std::string& stringText, tag_sql )
{
   std::string stringNew;
   stringNew.reserve( stringText.length() );
   for( auto it : stringText )
   {
      if( it > '\'' ) stringNew += it;
      else
      {
         if( it == '\'' ) stringNew += std::string_view{ "\'\'" };
         else stringNew += it;
      }
   }

   stringText = std::move( stringNew );
}

_GD_PARSE_END

_GD_PARSE_JSON_BEGIN

const char* next_key_g( const char* pbszJson, const char* pbszName, unsigned uNameLength, int* piLevel )
{
   enum { unknown, object, key, value } eState = key;

   int iLevel = 0;

   const uint8_t* puzNext = ( const uint8_t* )pbszJson;

   while( *puzNext != '\0' )
   {
#ifndef NODEBUG
      char chCheck_d = *puzNext;
#endif
      if( pJson_s[*puzNext] == 0 ) { puzNext++; continue; }

      if( (pJson_s[*puzNext] & JSON_TYPE_STRING) == JSON_TYPE_STRING )
      {
         puzNext++;                                                            // move to name (move past '"' character)
         if( eState == key )
         {
            // compare key name
            if( memcmp( ( void* )pbszName, puzNext, uNameLength ) == 0 )
            {
               if( piLevel != nullptr ) *piLevel = iLevel;
               return ( const char* )puzNext;
            }
            puzNext = skip_string_g( puzNext );                                // move past key name
         }
         else
         {
            const uint8_t* p_ = ( const uint8_t* )skip_string_g( ( const char* )puzNext, gd::parse::tag_json{} );
            if( p_ > puzNext || pJson_s[*p_] == JSON_TYPE_STRING )
            {
               puzNext = p_ + 1;
               eState = key;
            }
         }
      }
      else if( (pJson_s[*puzNext] & JSON_TYPE_VALUE ) == JSON_TYPE_VALUE )
      {
         eState = value;
      }
      else if( (pJson_s[*puzNext] & JSON_TYPE_KEY ) == JSON_TYPE_KEY )
      {
         if( *puzNext == ',' ) { eState = key; }
         else if( *puzNext == '{' ) { eState = key; iLevel++; }
         else                  { eState = value; iLevel--; }
      }

      puzNext++;
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Find key within json object scope "{}"
 * @param pbszJson pointer to json text that should point into scope where key 
 * @param pbszName key name
 * @param uNameLength key name length
 * @return pointer to key in same scope if found, null if not found
*/
const char* next_key_g( const char* pbszJson, const char* pbszName, unsigned uNameLength, tag_scope )
{
   enum { unknown, object, key, value } eState = key;

   int iLevel = 0;

   const uint8_t* puzNext = ( const uint8_t* )pbszJson;

   while( *puzNext != '\0' )
   {
#ifndef NDEBUG
      char chCheck_d = *puzNext;
#endif
      if( pJson_s[*puzNext] == 0 ) { puzNext++; continue; }

      if( (pJson_s[*puzNext] & JSON_TYPE_STRING) == JSON_TYPE_STRING )
      {
         puzNext++;                                                            // move to name (move past '"' character)
         if( eState == key )
         {
            // compare key name if level is 0 (only compare keys in scope)
            if( iLevel == 0 && memcmp( ( void* )pbszName, puzNext, uNameLength ) == 0 )
            {
               return ( const char* )puzNext;
            }
            puzNext = skip_string_g( puzNext );                                // move past key name
         }
         else
         {
            const uint8_t* p_ = ( const uint8_t* )skip_string_g( ( const char* )puzNext, gd::parse::tag_json{} );
            if( p_ > puzNext || pJson_s[*p_] == JSON_TYPE_STRING )
            {
               puzNext = p_ + 1;
               eState = key;
            }
         }
      }
      else if( (pJson_s[*puzNext] & JSON_TYPE_VALUE ) == JSON_TYPE_VALUE )
      {
         eState = value;
      }
      else if( (pJson_s[*puzNext] & JSON_TYPE_KEY ) == JSON_TYPE_KEY )
      {
         if( *puzNext == ',' ) { eState = key; }
         else if( *puzNext == '{' ) { eState = key; iLevel++; }
         else                  
         {
            eState = value; 
            iLevel--; 
            if( iLevel < 0 ) return nullptr;                                   // skip seaarch if out of scope
         }
      }

      puzNext++;
   }

   return nullptr;
}


/// Move from key to value that belongs to key
/// 
const char* find_key_value_g( const char* pbszKey )
{                                                                                                  assert( *pbszKey == '\"' || *(pbszKey - 1) == '\"' );
   const uint8_t* puzNext = ( const uint8_t* )pbszKey;
   if( *puzNext == '\"' ) puzNext++;
   puzNext = skip_string_g( puzNext );                                                             assert( *puzNext == '\"' );// pass key name
   puzNext++;
   puzNext = strchr( puzNext, ':' );
   if( puzNext != nullptr )
   {
      puzNext++;
      return skip_space_g( (const char*)puzNext );
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief read json value to variant_view object
 * No error checking is done, the pointer `pbszJson` has to be placed at start of 
 * json value.
 * @param pbszJson pointer that start at json value
 * @param variantviewValue reference to `variant_view` value is placed in
*/
void read_value( const char* pbszJson, gd::variant_view& variantviewValue )
{                                                                                                  assert( *pbszJson != '\0' );
   const uint8_t* puNext = ( const uint8_t* )pbszJson;
   if( *puNext == '\"' )                                                       // check if string value
   {
      puNext++;
      const uint8_t* puBegin = puNext;
      while( *puNext != '\0' && *puNext != '\"' )
      {
         if( *puNext != '\\' ) { puNext++; continue; }                         // no escape character, go to next
         else
         {                                                                     // escape character, disable next value
            puNext++;
            if( *puNext != '\0' ) puNext++;
         }
      }

      variantviewValue.assign( (const char*)puBegin, puNext - puBegin );
   }
   else
   {                                                                                               assert( pCharacterClass_s[*puNext] & ASCII_TYPE_DECIMAL );
      bool bDecimal = false;
      const uint8_t* puBegin = puNext;
      uint8_t uType = pCharacterClass_s[*puNext];
      while( uType & ASCII_TYPE_DECIMAL )
      {
         if( *puNext == '.' ) bDecimal = true;
         puNext++;
         uType = pCharacterClass_s[*puNext];
      }

      if( bDecimal == false )
      {
         int64_t iValue = std::strtoll( (const char*)puBegin, nullptr, 10 );
         variantviewValue.assign( iValue );
      }
      else
      {
         double dValue = std::strtod( (const char*)puBegin, nullptr );
         variantviewValue.assign( dValue );
      }
   }
}

const char* read_key_value_g( const char* pbszKey, gd::variant_view& variantviewValue )
{
   const char* pbszValue = find_key_value_g( pbszKey );
   if( pbszValue != nullptr )
   {
      read_value( pbszValue, variantviewValue );
   }
   return pbszValue;
}

/** ---------------------------------------------------------------------------
 * @brief Read key value for key that pointer in text points at
 * @param pbszKey pointer to key in json string where the value is read from
 * @return value as variant_view for key at position in json text
*/
gd::variant_view read_key_value_g( const char* pbszKey )
{
   gd::variant_view variantviewValue;
   read_key_value_g( pbszKey, variantviewValue );
   return variantviewValue;
}

/** -------------------------------------------------------------------
   * @brief Split string into multiple string_view and store them in vector, string is split where char is found
   * @param stringText text to split into multiple parts (remember to not destroy string as long as work with string_view items are used)
   * @param chSplitWith character used to mark where to split text
   * @param vectorPart vector where strings are stored
*/
void split_g( const std::string_view& stringText, char chSplitWith, std::vector<std::string_view>& vectorPart, const csv& csv )
{
   std::string stringPart;                // Store string parts added to vector

   const char* pbszPartStart = stringText.data();
   const char* pbszPartEnd = nullptr;
   for( const char* pbszPosition = stringText.data(), *pbszEnd = stringText.data() + stringText.length(); pbszPosition != pbszEnd; pbszPosition++ )
   {
      if( *pbszPosition == chSplitWith )
      {
         if( pbszPartEnd == nullptr ) pbszPartEnd = pbszPosition;
         vectorPart.emplace_back( std::string_view( pbszPartStart, pbszPartEnd - pbszPartStart ) );
         pbszPartStart = pbszPosition + 1;
         pbszPartEnd = nullptr;
      }
      else if( csv.is_quote( *pbszPosition ) == true )
      {
         pbszPartStart = pbszPosition + 1;
         // ## found quote, text within quote is skipped
         pbszPosition = skip_quoted_g( pbszPosition );
         pbszPartEnd = pbszPosition - 1;
      }
      else
      {
         pbszPosition++;
         continue;
      }
   }

   // if part contains text or last position in string is same as split character, then add one more to vector
   if( pbszPartStart <= &stringText.back() || stringText.back() == chSplitWith ) vectorPart.emplace_back( std::string_view( pbszPartStart, (stringText.data() + stringText.length()) - pbszPartStart));
}


_GD_PARSE_JSON_END


