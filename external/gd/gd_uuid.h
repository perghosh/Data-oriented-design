// @FILE [tag: uuid] [description: uuid (Universally unique identifier) value management] [type: header] [name: gd_uuid.h]

#pragma once

#include <cassert>
#include <cstring>
#include <limits>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <string_view>

#include "gd_types.h"
#include "gd_utf8.h"

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




#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd { 
#define _GD_END }
#endif


_GD_BEGIN

struct r64;


/**
 * @brief manage uuid (Universally unique identifier) value 
 * 
~~~{.cpp}
// create new random uuid value
gd::uuid uuidTest1 = gd::uuid::new_uuid_s();
gd::uuid uuidTest2 = gd::uuid::new_uuid_s();
assert( (uuidTest1 == uuidTest2) == false ); // never match
~~~
 */
struct uuid
{
public:
   using tag_null = gd::types::tag_command_nullify; ///< tag used to create null uuid value, it is used to create uuid with no value (all bytes set to 0)
   using tag_random = gd::types::tag_command_random;

public:
   typedef uint8_t           value_type;                                       ///< primitive type
   typedef value_type*       pointer;                                          ///< pointer type to internal buffer of primitive values used to store uuid
   typedef const value_type* const_pointer;                                    ///< const pointer type to internal buffer of primitive values used to store uuid
   typedef uint8_t&          reference;                                        ///< reference to primitive type
   typedef uint8_t const&    const_reference;                                  ///< const reference to primitive type
   typedef uint8_t*          iterator;                                         ///< iterator type, just a uint8_t* pointer
   typedef uint8_t const*    const_iterator;
   typedef std::size_t       size_type;
   typedef std::ptrdiff_t    difference_type;

public:
   uuid();
   uuid( tag_null ) { clear(); }
   uuid( tag_random ) { *this = new_uuid_s(); } ///< create new random uuid value
   uuid( const uuid& o );
   uuid( const uint8_t* puUuid ) { memcpy( m_pbData, puUuid, 16 ); }
   uuid( const char* pbsHexBegin, const char* pbsHexEnd ) { assert( (pbsHexEnd - pbsHexBegin) == 32 || (pbsHexEnd - pbsHexBegin) == 36 ); read( (const value_type*)pbsHexBegin, (const value_type*)pbsHexEnd ); }
   uuid( std::string_view stringUuid ) { assert( stringUuid.length() == 32 || stringUuid.length() == 36 ); read( (const value_type*)stringUuid.data(), (const value_type*)(stringUuid.data() + stringUuid.length()) ); }

#ifdef GD_X86
   uuid(__m128i iUuid) { _mm_store_si128((__m128i*)m_pbData, iUuid);  }
#endif
   uuid( uint64_t u1, uint64_t u2 ) { 
      *(uint64_t*)m_pbData = u1;  
      *(uint64_t*)(m_pbData + sizeof(uint64_t)) = u2;
   }

   /// assign to raw uint8_t*, make sure pointer to buffer it is valid for uuid
   uuid& operator=( const uint8_t* p );
   /// assign to hexadecimal string
   uuid& operator=( const char* pbszHex ) { read( (const value_type*)pbszHex, (const value_type*)(pbszHex + 32) ); return *this; }
   uuid& operator=( const std::string_view& stringUuid ) { read( (const value_type*)stringUuid.data(), (const value_type*)stringUuid.data() + stringUuid.length() ); return *this; }

   /// pointer operator to get buffer pointer
   operator const uint8_t*() const noexcept { return m_pbData; }

public:
   /// compare against another guid, here only pointer to guid is used to compare (for different type of uuid classes)
   bool compare( const_pointer puuid ) const;


    iterator begin() noexcept { return m_pbData; }
    const_iterator begin() const noexcept { return m_pbData; }
    iterator end() noexcept { return m_pbData + size(); }
    const_iterator end() const noexcept { return m_pbData + size(); }

    void read( const value_type* puBegin, const value_type* puEnd );
    void read( const std::string_view& stringUuid ) { read( (const_pointer)stringUuid.data(), (const_pointer)stringUuid.data() + stringUuid.length()  ); }


   /// get pointer to internal uuid buffer
   uint8_t* data() const noexcept { return (uint8_t*)m_pbData; }

   /// object size
   static constexpr size_type static_size() noexcept { return 16; }
   constexpr size_type size() const noexcept { return static_size(); }

   std::string to_string() const;  ///< convert uuid to string in uuid format

   /// clear buffer (set to 00000000-0000-0000-0000-000000000000)
   void clear() noexcept;

   /// create new random uuid
   static uuid new_uuid_s();

   /// read uuid value from string, note that string need to be valid for uuid, it doesn't check for internal uuid format errors
   static void read_s( const value_type* puBegin, const value_type* puEnd, value_type* puSet );

public:
    alignas(16) uint8_t m_pbData[16]; ///< internal buffer storing the uuid value 

    static r64 m_r64_s; ///< static member used to generate random uuid values

   /**
    * @brief Lookup table used to convert hexadecimal value in text to value stored in byte.
   */
   inline static const uint8_t m_pHexValue_s[0x70] =
   {
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x20-0x2F */
         0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, /* 0x30-0x3F (0 - 9) */
         0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F (A - F) */
         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
         0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F (a - f) */
   };

};

/** ---------------------------------------------------------------------------
 * @brief Convert uuid to string in uuid format
 * Note that hame mimics the to_string in stl
 * @param uuid uuid value that is converted to string
 * @return string hex string containing uuid vale
*/
inline std::string to_string_g( uuid const& uuid )
{
   const unsigned puDash[] = {3, 5, 7, 9, 0};
   const unsigned* puIndex = &puDash[0];
   std::string stringResult( 36, char() );

   unsigned uIndex = 0;
   auto pbsz = &stringResult[0];
   for( auto it = uuid.m_pbData, itEnd = uuid.m_pbData + 16; it != itEnd; it++, uIndex++ )
   {
      unsigned uByte = ((*it) >> 4) & 0x0F;
      *pbsz++ = uByte <= 9 ? '0' + uByte : 'a' + (uByte - 10);

      uByte = (*it) & 0x0F;
      *pbsz++ = uByte <= 9 ? '0' + uByte : 'a' + (uByte - 10);

      if( uIndex == *puIndex )
      {
         *pbsz++ = '-';
         puIndex++;
      }
   }

   return stringResult;
}

/** ---------------------------------------------------------------------------
 * @brief Convert uuid to string in uuid format
 * @param uuid uuid value that is converted to string
 * @return string hex string containing uuid vale
*/
inline std::string to_hex_g( uuid const& uuid )
{
   std::string stringResult( 32, char() );

   unsigned uIndex = 0;
   auto pbsz = &stringResult[0];
   for( auto it = uuid.m_pbData, itEnd = uuid.m_pbData + 16; it != itEnd; it++, uIndex++ )
   {
      unsigned uByte = ((*it) >> 4) & 0x0F;
      *pbsz++ = uByte <= 9 ? '0' + uByte : 'a' + (uByte - 10);

      uByte = (*it) & 0x0F;
      *pbsz++ = uByte <= 9 ? '0' + uByte : 'a' + (uByte - 10);
   }

   return stringResult;
}



/** ---------------------------------------------------------------------------
 * @brief Default constructor
 * Note that guid value is cleared in debug mode
*/
inline uuid::uuid() {
#ifdef _DEBUG
   clear();
#endif // _DEBUG
}



/** ---------------------------------------------------------------------------
 * @brief load __m128i from memory
 * @param p pointer to uuid data (hopefully) to convert to sse 128 bit value
 * @return __m128i with uuid value
*/
#ifdef GD_X86
inline __m128i load_unaligned_si128( const uint8_t* p ) noexcept
{
   return _mm_loadu_si128( reinterpret_cast< const __m128i* >(p) );
}
#else
inline const uint8_t* load_unaligned_si128(const uint8_t* p) noexcept
{
   return p;
}
#endif


inline uuid::uuid( const uuid& o )
{
#ifdef GD_X86
   __m128i i = load_unaligned_si128( o.m_pbData );
   _mm_storeu_si128( reinterpret_cast<__m128i*>(m_pbData), i );
#else
   memcpy(m_pbData, o, sizeof(m_pbData));
#endif

}


/** ---------------------------------------------------------------------------
 * @brief assign uuid from byte buffer
 * @param p pointer to byte buffer assigned to internal uuid buffer
 * @return reference
*/
inline uuid& uuid::operator=( const uint8_t* p )
{
#ifdef GD_X86
   auto i = load_unaligned_si128( p );
   _mm_storeu_si128( reinterpret_cast< __m128i* >(m_pbData), i );
#else
   memcpy(m_pbData, p, sizeof(m_pbData));
#endif
   return *this;
}

inline bool uuid::compare( const_pointer puuid ) const
{
#ifdef GD_X86
   auto i1 = load_unaligned_si128( m_pbData );
   auto i2 = load_unaligned_si128( puuid );

   auto iCompare = _mm_cmpeq_epi8(i1, i2);
   uint16_t uMask = _mm_movemask_epi8(iCompare);                                                   assert( (uMask == 0xffff) == (memcmp( m_pbData, puuid, sizeof(uuid) ) == 0) );

   return (uMask == 0xffff);

#else
   return memcmp( m_pbData, puuid, sizeof(uuid) ) == 0;
#endif
}

inline void uuid::read( const value_type* puBegin, const value_type* puEnd )
{
   read_s( puBegin, puEnd, m_pbData );
}

/** ---------------------------------------------------------------------------
 * @brief clear internal buffer storing uuid value
*/
inline void uuid::clear() noexcept
{
   *(uint64_t*)m_pbData = 0ULL;  
   *(uint64_t*)(m_pbData + sizeof(uint64_t)) = 0ULL;
}

/** ---------------------------------------------------------------------------
 * @brief equal operator for uuid objects
 * @param uuid1 first uuid to be compared
 * @param uuid2 second uuid to be compared
 * @return true if equal, false if not
*/
inline bool operator==(const uuid& uuid1, const uuid& uuid2) noexcept
{
#ifdef GD_X86
   auto i1 = load_unaligned_si128( uuid1 );
   auto i2 = load_unaligned_si128( uuid2 );

   auto iCompare = _mm_cmpeq_epi8(i1, i2);
   uint16_t uMask = _mm_movemask_epi8(iCompare);

   return (uMask == 0xffff);

   /* sse4.1
   __m128i i = _mm_xor_si128(i1, i2);                                          // clear all bits using xor
   return _mm_test_all_zeros(i, i) != 0;                                       // compare if value bytes are zero
   */
#else
   return memcmp(uuid1.m_pbData, uuid2.m_pbData, sizeof(uuid1.m_pbData)) == 0;
#endif
}

/** ---------------------------------------------------------------------------
 * @brief not equal operator ofr uuid objects
 * @param uuid1 first uuid to be compared
 * @param uuid2 second uuid to be compared
 * @return true if not equal
*/
inline bool operator!=( const uuid& uuid1, const uuid& uuid2 ) noexcept
{
   return !( uuid1 == uuid2 );
}


/** ---------------------------------------------------------------------------
 * @brief compare two uuid values
 * Code similar to the boost implementation
 * @param uuid1 uuidLeft left uuid value to be compared
 * @param uuidRight right value to be compared
 * @return boolean true if left is smaller than right, false if not
*/
inline bool operator<(const uuid& uuidLeft, const uuid& uuidRight) noexcept
{
#ifdef GD_X86
    __m128i iLeft = load_unaligned_si128(uuidLeft);
    __m128i iRight = load_unaligned_si128(uuidRight);

    // documentation from boost implementation

    // To emulate lexicographical_compare behavior we have to perform two comparisons - the forward and reverse one.
    // Then we know which bytes are equivalent and which ones are different, and for those different the comparison results
    // will be opposite. Then we'll be able to find the first differing comparison result (for both forward and reverse ways),
    // and depending on which way it is for, this will be the result of the operation. There are a few notes to consider:
    //
    // 1. Due to little endian byte order the first bytes go into the lower part of the xmm registers,
    //    so the comparison results in the least significant bits will actually be the most signigicant for the final operation result.
    //    This means we have to determine which of the comparison results have the least significant bit on, and this is achieved with
    //    the "(x - 1) ^ x" trick.
    // 2. Because there is only signed comparison in SSE/AVX, we have to invert byte comparison results whenever signs of the corresponding
    //    bytes are different. I.e. in signed comparison it's -1 < 1, but in unsigned it is the opposite (255 > 1). To do that we XOR left and right,
    //    making the most significant bit of each byte 1 if the signs are different, and later apply this mask with another XOR to the comparison results.
    // 3. pcmpgtw compares for "greater" relation, so we swap the arguments to get what we need.

    const __m128i iSignsMask = _mm_xor_si128(iLeft, iRight);

    __m128i iCompare = _mm_cmpgt_epi8(iRight, iLeft);
    __m128i iCompareRight = _mm_cmpgt_epi8(iLeft, iRight);

    iCompare = _mm_xor_si128(iSignsMask, iCompare);
    iCompareRight = _mm_xor_si128(iSignsMask, iCompareRight);

    uint32_t uToCompareLeft = static_cast< uint32_t >(_mm_movemask_epi8(iCompare)); 
    uint32_t rToCompareRight = static_cast< uint32_t >(_mm_movemask_epi8(iCompareRight));

    uToCompareLeft = (uToCompareLeft - 1u) ^ uToCompareLeft;
    rToCompareRight = (rToCompareRight - 1u) ^ rToCompareRight;

    return uToCompareLeft < rToCompareRight;
#else
    return memcmp( uuidLeft, uuidRight, sizeof(uuidLeft.m_pbData) ) < 0;
#endif
}

inline std::string uuid::to_string() const { return to_string_g(*this); }



// ----------------------------------------------------------------------------
// ------------------------------------------------------------------------ r64
// ----------------------------------------------------------------------------


/** ---------------------------------------------------------------------------
 * \brief r64 is only used for uuid to produce random uuid values.
 * Used as a singleton in uuu
 */
struct r64
{
// ## construction ------------------------------------------------------------
   r64() : m_UID( 0, (uint64_t)-1 ) {}

   uuid new_uuid() {
#ifdef GD_X86
      const __m128i and_mask = _mm_set_epi64x(0xFFFFFFFFFFFFFF3Full, 0xFF0FFFFFFFFFFFFFull);
      const __m128i or_mask =  _mm_set_epi64x(0x0000000000000080ull, 0x0040000000000000ull);
      __m128i iNumber = _mm_set_epi64x(m_UID(m_mt19937_s), m_UID(m_mt19937_s));
      __m128i iUuid = _mm_or_si128(_mm_and_si128(iNumber, and_mask), or_mask);
      return  uuid( iUuid );
#else
      return  uuid( m_UID(m_mt19937_s), m_UID(m_mt19937_s) );
#endif
   }

// ## attributes
   std::uniform_int_distribution<uint64_t> m_UID;

   static std::random_device m_random_device_s; ///< seed for random generater
   static std::mt19937_64 m_mt19937_s;          ///< random generate (see: https://en.cppreference.com/w/cpp/numeric/random/mersenne_twister_engine) 
};

/// construct the static member `m_r64_s` in uuid used to create random uuid values
inline r64 uuid::m_r64_s;

/** ---------------------------------------------------------------------------
 * @brief create new random uuid value
 * Note that this method uses a singleton version of r64 (random 64) to create random uuid value
 * @return uuid with a random value
*/
inline uuid uuid::new_uuid_s()
{
   return uuid::m_r64_s.new_uuid();
}


/** ---------------------------------------------------------------------------
 * @brief Read uuid from uuid formated string
 * Supported formats are `00000000000000000000000000000000`, `00000000-0000-0000-0000-000000000000` 
 * or `{00000000-0000-0000-0000-000000000000}`.
 * @param puBegin start of uuid formated string value
 * @param puEnd end of uuid formated string value
 * @param puSet uuid buffer for converted hex string 
*/
inline void uuid::read_s( const value_type* puBegin, const value_type* puEnd, value_type* puSet )
{
   auto uLength = puEnd - puBegin;                                                                 assert( uLength == 32 || uLength == 36 || uLength == 38 );
   if( uLength == 32 )
   {
      for( ; puBegin != puEnd; puBegin += 2 )
      {                                                                                            assert( *puBegin < sizeof(m_pHexValue_s) ); assert( *(puBegin + 1) < sizeof(m_pHexValue_s) );
         *puSet = m_pHexValue_s[*puBegin] << 4;                                                    assert( m_pHexValue_s[*puBegin] != 0 || *puBegin == '0' );
         *puSet += m_pHexValue_s[*(puBegin + 1)];                                                  assert( m_pHexValue_s[*(puBegin + 1)] != 0 || *(puBegin + 1) == '0' );
         puSet++;
      }
   }
   else
   {
      if( uLength == 38 )
      {
         puBegin++;
         puEnd--;
      }

      while( puBegin != puEnd )
      {
         if( *puBegin != '-' )
         {                                                                                         assert( *puBegin < sizeof( m_pHexValue_s ) ); assert( *(puBegin + 1) < sizeof( m_pHexValue_s ) );
            *puSet = m_pHexValue_s[*puBegin] << 4;                                                 assert( m_pHexValue_s[*puBegin] != 0 || *puBegin == '0' );
            *puSet += m_pHexValue_s[*(puBegin + 1)];                                               assert( m_pHexValue_s[*(puBegin + 1)] != 0 || *(puBegin + 1) == '0' );
            puSet++;
            puBegin += 2;
         }
         else 
         {                                                                                         assert( (puEnd - puBegin) == 28 || (puEnd - puBegin) == 23 || (puEnd - puBegin) == 18 || (puEnd - puBegin) == 13 );
            puBegin += 1; 
         }
      }
   }
}

inline std::random_device r64::m_random_device_s;
inline std::mt19937_64 r64::m_mt19937_s( r64::m_random_device_s() );



_GD_END
