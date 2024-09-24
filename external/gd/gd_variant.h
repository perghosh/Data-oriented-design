/**
 * \file gd_variant.h
 * 
 * \brief variant stores any primitive value and some common derived value types
 * 
 * Variant is very useful when you need runtime type information for a value.
 * 
 * 
 * 
 */

#pragma once
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <type_traits>

#include "gd_types.h"

#ifndef _GD_BEGIN_VARIANT
#  define GD_BEGIN_VARIANT
#endif

#if defined( __clang__ )
   #pragma GCC diagnostic push
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


#ifndef _GD_BEGIN
namespace gd {
#else
_GD_BEGIN
#endif


namespace variant_type
{
   struct tag_no_allocate {};                                                  // tag dispatcher for overloaded methods avoiding allocation
   struct tag_raw {};                                                          // tag dispatcher setting data externally or without internal logic
   struct tag_utf8 {};                                                         // tag for utf8 formated text
   struct tag_scientific {};                                                   // tag for use when you need exact numbers
   struct tag_explicit {};                                                     // tag used to avoid implicit convertions using variant items

   struct tag_std_variant {};                                                  // operations involving variant from stl

   enum enumGroup
   {
      eGroupNumber        = 0x00000100,
      eGroupInteger       = 0x00000200,
      eGroupDecimal       = 0x00000400,
      eGroupSigned        = 0x00000800,
      eGroupString        = 0x00001000,
      eGroupDate          = 0x00002000,
      eGroupBinary        = 0x00004000,
      eGroupBoolean       = 0x00008000,

      eGroupSize08        = 0x00010000,
      eGroupSize16        = 0x00020000,
      eGroupSize32        = 0x00040000,
      eGroupSize64        = 0x00080000,
      eGroupSize128       = 0x00100000,
      eGroupSize256       = 0x00200000,
      eGroupSize512       = 0x00400000,
   };

   enum enumFlag
   {
      eFlagAllocate     = 0x01000000,                                          // variant object has allocated data on heap
      eFlagLengthPrefix = 0x02000000,                                          // pointer to object is prefixed with length
   };

   /*-----------------------------------------*/ /**
    * \brief variant type based on single number
    *
    *
    */
   enum enumTypeNumber
   {
      eTypeNumberUnknown      = 0,
      eTypeNumberBool         = 1,
      eTypeNumberInt8         = 2,
      eTypeNumberUInt8        = 3,
      eTypeNumberInt16        = 4,
      eTypeNumberUInt16       = 5,
      eTypeNumberInt32        = 6,
      eTypeNumberUInt32       = 7,
      eTypeNumberInt64        = 8,
      eTypeNumberUInt64       = 9,
      eTypeNumberFloat        = 10,
      eTypeNumberDouble       = 11,
      eTypeNumberPointer      = 12,
      eTypeNumberGuid         = 13,
      eTypeNumberString       = 14,
      eTypeNumberUtf8String   = 15,
      eTypeNumberWString      = 16,
      eTypeNumberUtf32String  = 17,
      eTypeNumberBinary       = 18,
      eTypeNumberJson         = 19,
      eTypeNumberXml          = 20,
      eTypeNumberCsv          = 21,
      eTypeNumberVoid         = 22,
      eTypeNumberBit          = 23,
      eTypeNumberInt128       = 24,
      eTypeNumberUInt128      = 25,
      eTypeNumberInt256       = 26,
      eTypeNumberUInt256      = 27,
      eTypeNumberInt512       = 28,
      eTypeNumberUInt512      = 29,
      eTypeNumberHex          = 30,
      eTypeNumberBase32       = 31,
      eTypeNumberDateTime     = 32,
      eTypeNumberDate         = 33,
      eTypeNumberTime         = 34,
   };

   /*-----------------------------------------*/ /**
    * \brief tip constants is for help when to develop logic
    *
    *
    */
   enum enumTypeTip
   {
      // Core values that will fit within a register (there are values above this that also do not need allocation)
      eTypeTipRegister        = eTypeNumberPointer,

   };


   /*-----------------------------------------*/ /**
    * \brief One single type each supported value in variant
    *
    *
    */
   enum enumType
   {
      eTypeUnknown      = eTypeNumberUnknown,
      eTypeBool         = eTypeNumberBool     | eGroupBoolean,
      eTypeInt8         = eTypeNumberInt8     | eGroupInteger  | eGroupSize08  | eGroupSigned,
      eTypeInt16        = eTypeNumberInt16    | eGroupInteger  | eGroupSize16  | eGroupSigned,
      eTypeInt32        = eTypeNumberInt32    | eGroupInteger  | eGroupSize32  | eGroupSigned,
      eTypeInt64        = eTypeNumberInt64    | eGroupInteger  | eGroupSize64  | eGroupSigned,
      eTypeUInt8        = eTypeNumberUInt8    | eGroupInteger  | eGroupSize08,
      eTypeUInt16       = eTypeNumberUInt16   | eGroupInteger  | eGroupSize16,
      eTypeUInt32       = eTypeNumberUInt32   | eGroupInteger  | eGroupSize32,
      eTypeUInt64       = eTypeNumberUInt64   | eGroupInteger  | eGroupSize64,
      eTypeCFloat       = eTypeNumberFloat    | eGroupDecimal  | eGroupSize32,
      eTypeCDouble      = eTypeNumberDouble   | eGroupDecimal  | eGroupSize64,
      eTypePointer      = eTypeNumberPointer,
      eTypeGuid         = eTypeNumberGuid     | eGroupBinary,
      eTypeBinary       = eTypeNumberBinary   | eGroupBinary,
      eTypeString       = eTypeNumberString   | eGroupString,
      eTypeUtf8String   = eTypeNumberUtf8String | eGroupString,
      eTypeWString      = eTypeNumberWString  | eGroupString,
      eTypeUtf32String  = eTypeNumberUtf32String | eGroupString,
      eTypeJson         = eTypeNumberJson     | eGroupString,
      eTypeXml          = eTypeNumberXml      | eGroupString,
      eTypeVoid         = eTypeNumberVoid,
      eTypeBit          = eTypeNumberBit      | eGroupBoolean
   };

   /*-----------------------------------------*/ /**
    * \brief wrapper used to handle utf8 text
    */
   struct utf8 
   {
      utf8( const char* pbsz ) : m_pbsz(pbsz), m_uLength( strlen( pbsz ) ) {}
      utf8( const char* pbsz, size_t uLength ) : m_pbsz(pbsz), m_uLength( uLength ) {}
      utf8( const utf8& o ) { m_pbsz = o.m_pbsz; m_uLength = o.m_uLength; }
      // attributes
      public:
      const char* m_pbsz;
      size_t m_uLength;
   };

   enum enumFilter
   {
      eFilterTypeGroup  = 0x0000ff00,     ///< flags used to get group information from type
      eFilterTypeNumber = 0x000000ff,     ///< flags for filter out index for type
      eFilterInternal   = 0xFF000000,     ///< special flags used interally in variant
   };

   /*-----------------------------------------*/ /**
    * \brief wrapper used to set uuid value
    */
   struct uuid 
   { 
      uuid( const uint8_t* v ): m_pbUuid(v) {}
      const uint8_t* m_pbUuid; 
   };

   #pragma pack(push,1)
   /*-----------------------------------------*/ /**
    * \brief guid compatible with GUID in windows
    */
   struct guid 
   {
      unsigned long  Data1;
      unsigned short Data2;
      unsigned short Data3;
      unsigned char  Data4[ 8 ];
   };
   #pragma pack(pop)



}

/// Forward declaration for variant_view class that has the exact same internal
/// data but do now own the data. It is ok to cast from variant to variant_view.
class variant_view;

/**
 * \brief variant holds type and value
 *
 *
 *
 \code
 \endcode
 */
class variant 
{
public:
   /*-----------------------------------------*/ /**
    * \brief wrapper used to handle utf8 text
    */
   struct utf8 
   {
      utf8( const char* pbsz ) : m_pbsz(pbsz), m_uLength( strlen( pbsz ) ) {}
      utf8( const char* pbsz, size_t uLength ) : m_pbsz(pbsz), m_uLength( uLength ) {}
      utf8( const utf8& o ) { m_pbsz = o.m_pbsz; m_uLength = o.m_uLength; }
      // attributes
      public:
      const char* m_pbsz;
      size_t m_uLength;
   };

public:
   enum enumMASK
   {
      eTYPE             = 0x000000ff,  ///< filter type value
      eLENGTH           = 0x02000000,  ///< item is prefixed with length
   };


// construction
public:
   variant()               : m_uType(variant_type::eTypeUnknown)    {}
   variant( bool b )       : m_uType(variant_type::eTypeBool)       { m_V.b = b; }
   variant( int8_t v )     : m_uType(variant_type::eTypeInt8)       { m_V.int8 = v; }
   variant( int16_t v )    : m_uType(variant_type::eTypeInt16)      { m_V.int16 = v; }
   variant( int32_t v )    : m_uType(variant_type::eTypeInt32)      { m_V.int32 = v; }
   variant( int64_t v )    : m_uType(variant_type::eTypeInt64)      { m_V.int64 = v; }
   variant( uint8_t v )    : m_uType(variant_type::eTypeUInt8)      { m_V.uint8 = v; }
   variant( uint16_t v )   : m_uType(variant_type::eTypeUInt16)     { m_V.uint16 = v; }
   variant( uint32_t v )   : m_uType(variant_type::eTypeUInt32)     { m_V.uint32 = v; }
   variant( uint64_t v )   : m_uType(variant_type::eTypeUInt64)     { m_V.uint64 = v; }
   variant( float v )      : m_uType(variant_type::eTypeCFloat)     { m_V.f = v; }
   variant( double v )     : m_uType(variant_type::eTypeCDouble)    { m_V.d = v; }
   variant( void* p )      : m_uType(variant_type::eTypePointer)    { m_V.p = p; }
   variant( const char* v ): m_uType(variant_type::eTypeString|variant_type::eFlagAllocate), m_uSize(strlen(v)) { m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v,  m_uSize + 1u ); }
#if defined(__cpp_char8_t)
   variant( const char8_t* v ): m_uType(variant_type::eTypeUtf8String|variant_type::eFlagAllocate), m_uSize(strlen((const char*)v)) { m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v,  m_uSize + 1u ); }
#endif
   variant( const wchar_t* v ): m_uType(variant_type::eTypeWString|variant_type::eFlagAllocate), m_uSize(wcslen(v)) { m_V.pwsz = (wchar_t*)allocate(( m_uSize + 1u ) * sizeof(wchar_t)); memcpy( m_V.pwsz, v, ( m_uSize + 1u ) * sizeof(wchar_t) ); }
   variant( const char* v, size_t uLength ): m_uType(variant_type::eTypeString|variant_type::eFlagAllocate), m_uSize(uLength) { m_V.pbsz = (char*)allocate(uLength + 1); memcpy( m_V.pbsz, v, uLength ); m_V.pbsz[uLength] = '\0';  }
   variant( const char* v, size_t uLength, bool ): m_uType(variant_type::eTypeString), m_uSize(uLength) { m_V.pbsz = const_cast<char*>(v); }
   variant( const wchar_t* v, size_t uLength ): m_uType(variant_type::eTypeWString|variant_type::eFlagAllocate), m_uSize(uLength) { m_V.pwsz = (wchar_t*)allocate((uLength + 1) * sizeof(wchar_t)); memcpy( m_V.pwsz, v, (uLength) * sizeof(wchar_t) ); m_V.pwsz[uLength] = L'\0'; }
   variant( const wchar_t* v, size_t uLength, bool ) : m_uType(variant_type::eTypeWString), m_uSize(uLength) { m_V.pwsz = const_cast<wchar_t*>(v); }
   variant( const unsigned char* v, size_t uLength, bool ): m_uType(variant_type::eTypeBinary), m_uSize(uLength) { m_V.pb = const_cast<unsigned char*>(v); }
   variant( const utf8& v ) : m_uType(variant_type::eTypeUtf8String|variant_type::eFlagAllocate), m_uSize(v.m_uLength) { m_V.pbsz = (char*)allocate(  m_uSize + 1u ); memcpy( m_V.pbsz, v.m_pbsz,  m_uSize + 1u ); }
   variant( const utf8& v, unsigned int uType ) : m_uType(uType|variant_type::eFlagAllocate), m_uSize(v.m_uLength) { m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.m_pbsz,  m_uSize + 1u ); }
   variant( const utf8& v, bool ) : m_uType(variant_type::eTypeUtf8String), m_uSize(v.m_uLength) { m_V.pbsz = const_cast<char*>(v.m_pbsz); }
   variant( unsigned int uType, void* v, size_t uLength, size_t uDataLength = 0 ) : m_uType(uType), m_uSize( uLength ) { if( uDataLength == 0 ) uDataLength = uLength; m_V.pb = (unsigned char*)allocate(uDataLength); memcpy( m_V.pb, v, uDataLength );  }
   variant( const uint8_t* v, size_t uLength ): m_uType(variant_type::eTypeBinary|variant_type::eFlagAllocate), m_uSize(uLength) { m_V.pb = (uint8_t*)allocate(uLength); memcpy( m_V.pb, v, uLength ); }
   variant( const variant_type::guid& v ): m_uType(variant_type::eTypeGuid|variant_type::eFlagAllocate), m_uSize(sizeof(variant_type::guid)) { m_V.pb = (uint8_t*)allocate(m_uSize); memcpy( m_V.pb, &v, m_uSize ); }

   variant( const std::string& v ): m_uType(variant_type::eTypeString|variant_type::eFlagAllocate), m_uSize(v.length()) { m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.c_str(), m_uSize + 1u); }
   explicit variant( const std::string_view& v ): m_uType(variant_type::eTypeString|variant_type::eFlagAllocate), m_uSize(v.length()) { m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.data(), m_uSize + 1u); }
   variant( const std::string& v, unsigned int uType ): m_uType(uType|variant_type::eFlagAllocate), m_uSize(v.length()) { m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.c_str(), m_uSize + 1u); }

   variant(const char* v, bool) : m_uType(variant_type::eTypeString), m_uSize(strlen(v)) { m_V.pbsz_const = v; }
   variant(const std::string_view& v, bool) : m_uType(variant_type::eTypeString), m_uSize(v.length()) { m_V.pbsz_const = v.data(); }

   variant( const variant& o ) { common_construct( o ); }                        // copy
   variant( variant&& o ) noexcept { move_construct( o ); }                      // move

   // assign
   variant& operator=( const variant& o ) { clear(); common_construct( o ); return *this; }
   variant& operator=( variant&& o ) noexcept { 
      clear();
      if( this != &o ) { ((uint64_t*)this)[0] = ((uint64_t*)&o)[0]; ((uint64_t*)this)[1] = ((uint64_t*)&o)[1]; o.m_uType = variant_type::eTypeUnknown; }
      return *this; }
   ~variant() { clear(); }

   void operator=( bool b )     { clear(); m_uType = variant_type::eTypeBool; m_V.b = b; }
   void operator=( int8_t v )   { clear(); m_uType = variant_type::eTypeInt8; m_V.int8 = v;  }
   void operator=( int16_t v )  { clear(); m_uType = variant_type::eTypeInt16; m_V.int16 = v; }
   void operator=( int32_t v )  { clear(); m_uType = variant_type::eTypeInt32; m_V.int32 = v; }
   void operator=( int64_t v )  { clear(); m_uType = variant_type::eTypeInt64; m_V.int64 = v; }
   void operator=( uint8_t v )  { clear(); m_uType = variant_type::eTypeUInt8; m_V.uint8 = v;  }
   void operator=( uint16_t v ) { clear(); m_uType = variant_type::eTypeUInt16; m_V.uint16 = v; }
   void operator=( uint32_t v ) { clear(); m_uType = variant_type::eTypeUInt32; m_V.uint32 = v; }
   void operator=( uint64_t v ) { clear(); m_uType = variant_type::eTypeUInt64; m_V.uint64 = v; }
   void operator=( float v )    { clear(); m_uType = variant_type::eTypeCFloat; m_V.f = v;  }
   void operator=( double v )   { clear(); m_uType = variant_type::eTypeCDouble; m_V.d = v;  }
   void operator=( const char* v ) { clear(); m_uType = variant_type::eTypeString|variant_type::eFlagAllocate; m_uSize = (unsigned int)strlen(v); m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v,  m_uSize + 1u ); }
   void operator=( const wchar_t* v ) { clear(); m_uType = variant_type::eTypeWString|variant_type::eFlagAllocate; m_uSize = (unsigned int)wcslen(v); m_V.pwsz = (wchar_t*)allocate(( m_uSize + 1u ) * sizeof(wchar_t)); memcpy( m_V.pwsz, v, ( m_uSize + 1u ) * sizeof(wchar_t) ); }
   void operator=( const utf8& v ) { clear(); m_uType = variant_type::eTypeUtf8String|variant_type::eFlagAllocate; m_uSize = v.m_uLength; m_V.pbsz = (char*)allocate(  m_uSize + 1u ); memcpy( m_V.pbsz, v.m_pbsz,  m_uSize + 1u ); }
#if defined(__cpp_char8_t)
   void operator=(const char8_t* v) { clear(); m_uType = variant_type::eTypeUtf8String|variant_type::eFlagAllocate; m_uSize = (unsigned int)strlen((const char*)v); m_V.putf8 = (char8_t*)allocate(  m_uSize + 1u ); memcpy( m_V.putf8, v,  m_uSize + 1u ); }
#endif

   int32_t operator+(int32_t v) { return m_V.int32 + v; }
   uint32_t operator+(uint32_t v) { return m_V.uint32 + v; }
   int64_t operator+(int64_t v) { return m_V.int64 + v; }
   uint64_t operator+(uint64_t v) { return m_V.uint64 + v; }
   int32_t operator-(int32_t v) { return m_V.int32 - v; }
   uint32_t operator-(uint32_t v) { return m_V.uint32 - v; }
   int64_t operator-(int64_t v) { return m_V.int64 - v; }
   uint64_t operator-(uint64_t v) { return m_V.uint64 - v; }

   variant& operator+=(int32_t v) { m_V.int32 += v; return *this; }
   variant& operator+=(uint32_t v) { m_V.uint32 += v; return *this; }
   variant& operator+=(int64_t v) { m_V.int64 += v; return *this; }
   variant& operator+=(uint64_t v) { m_V.uint64 += v; return *this; }
   variant& operator-=(int32_t v) { m_V.int32 -= v; return *this; }
   variant& operator-=(uint32_t v) { m_V.uint32 -= v; return *this; }
   variant& operator-=(int64_t v) { m_V.int64 -= v; return *this; }
   variant& operator-=(uint64_t v) { m_V.uint64 -= v; return *this; }


   void assign( bool v )      { _set_value( v ); }
   void assign( int8_t v )    { _set_value( v ); }
   void assign( int16_t v )   { _set_value( v ); }
   void assign( int32_t v )   { _set_value( v ); }
   void assign( int64_t v )   { _set_value( v ); }
   void assign( uint8_t v )   { _set_value( v ); }
   void assign( uint16_t v )  { _set_value( v ); }
   void assign( uint32_t v )  { _set_value( v ); }
   void assign( uint64_t v )  { _set_value( v ); }
   void assign( float v  )    { _set_value( v ); }
   void assign( double v )    { _set_value( v ); }
   void assign( const char* v ) { _set_value( v ); }
   void assign( const char* v, size_t uLength ) { _set_value( v, uLength ); }
#if defined(__cpp_char8_t)
   void assign( const char8_t* v ) { _set_value( v ); }
   void assign( const char8_t* v, size_t uLength ) { _set_value( v, uLength ); }
#endif
   void assign( const wchar_t* v ) { _set_value( v ); }
   void assign( const unsigned char* v, size_t uLength ) { _set_value( v, uLength ); }
   void assign( const wchar_t* v, unsigned int uLength ) { _set_value( v, uLength ); }
   void assign( const utf8& v, variant_type::tag_utf8 ) { _set_value( v, variant_type::tag_utf8{}); }
   void assign( const utf8& v, unsigned int uType, variant_type::tag_utf8 ) { _set_value( v, uType, variant_type::tag_utf8{} ); }
   void assign( unsigned int uType, uint64_t uValue, variant_type::tag_raw ) { clear(); m_uType = uType; m_uSize = 0; m_V.uint64 = uValue; }
   void assign( const std::string& v ) { _set_value( v ); }
   void assign( const std::wstring& v ) { _set_value( v ); }
   template<typename TYPE>
   void assign( const TYPE* v, unsigned int uType, unsigned int uLength, variant_type::tag_raw ) { clear(); m_uType = uType|variant_type::eFlagAllocate; m_uSize = uLength; m_V.p = (void*)allocate( uLength * sizeof(TYPE) ); memcpy( m_V.p, v, uLength * sizeof(TYPE) ); }
   //void assign_binary( const uint8_t* p, size_t uLength ) { _set_binary_value( p, uLength ); }


   operator bool() const      { assert(type_number() == variant_type::eTypeNumberBool); return m_V.b; }
   operator int8_t() const    { assert(type_number() == variant_type::eTypeNumberInt8); return m_V.int8; }
   operator int16_t() const   { assert(type_number() == variant_type::eTypeNumberInt16); return m_V.int16; }
   operator int32_t() const   { assert(type_number() == variant_type::eTypeNumberInt32); return m_V.int32; }
   operator int64_t() const   { assert(type_number() == variant_type::eTypeNumberInt64); return m_V.int64; }
   operator uint8_t() const   { assert(type_number() == variant_type::eTypeNumberUInt8); return m_V.uint8; }
   operator uint16_t() const  { assert(type_number() == variant_type::eTypeNumberUInt16); return m_V.uint16; }
   operator uint32_t() const  { assert(type_number() == variant_type::eTypeNumberUInt32); return m_V.uint32; }
   operator uint64_t() const  { assert(type_number() == variant_type::eTypeNumberUInt64); return m_V.uint64; }
   operator float()  const    { assert(type_number() == variant_type::eTypeNumberFloat); return m_V.f; }
   operator double() const    { assert(type_number() == variant_type::eTypeNumberDouble); return m_V.d; }
   operator void*() const     { assert(type_number() == variant_type::eTypeNumberPointer); return m_V.p; }
   operator const char*() const { assert(type_number() == variant_type::eTypeNumberString || type_number() == variant_type::eTypeNumberUtf8String || type_number() == variant_type::eTypeNumberJson || type_number() == variant_type::eTypeNumberXml ); return m_V.pbsz; }
#if defined(__cpp_char8_t)
   operator const char8_t*() const { assert(type_number() == variant_type::eTypeNumberUtf8String); return m_V.putf8_const; }
#endif

   operator const wchar_t*() const { assert(type_number() == variant_type::eTypeNumberWString); return m_V.pwsz; }
   operator const unsigned char*() const { assert(type_number()== variant_type::eTypeNumberBinary); return m_V.pb; }

   bool operator==( const variant& o ) const { return compare( o ); }
   bool operator!=( const variant& o ) const { return compare( o ) == false; }

   bool operator<( const variant& o ) const { return less( o ); }


/** \name RAW
*///@{
   void _set_type( uint32_t uType ) { m_uType = uType; }

   void _set_value( bool v )        { clear(); m_uType = variant_type::eTypeBool;     m_V.b = v; }
   void _set_value( int8_t v )      { clear(); m_uType = variant_type::eTypeInt8;     m_V.int8 = v; }
   void _set_value( int16_t v )     { clear(); m_uType = variant_type::eTypeInt16;    m_V.int16 = v; }
   void _set_value( int32_t v )     { clear(); m_uType = variant_type::eTypeInt32;    m_V.int32 = v; }
   void _set_value( int64_t v )     { clear(); m_uType = variant_type::eTypeInt64;    m_V.int64 = v; }
   void _set_value( uint8_t v )     { clear(); m_uType = variant_type::eTypeUInt8;    m_V.uint8 = v; }
   void _set_value( uint16_t v )    { clear(); m_uType = variant_type::eTypeUInt16;   m_V.uint16 = v; }
   void _set_value( uint32_t v )    { clear(); m_uType = variant_type::eTypeUInt32;   m_V.uint32 = v; }
   void _set_value( uint64_t v )    { clear(); m_uType = variant_type::eTypeUInt64;   m_V.uint64 = v; }
   void _set_value( float v )       { clear(); m_uType = variant_type::eTypeCFloat;   m_V.f = v; }
   void _set_value( double v )      { clear(); m_uType = variant_type::eTypeCDouble;  m_V.d = v; }
   void _set_value( const char* v ) { clear(); m_uType = variant_type::eTypeString|variant_type::eFlagAllocate; m_uSize = (unsigned int)strlen(v); m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v, m_uSize); m_V.pbsz[m_uSize] = '\0'; }
   void _set_value( const char* v, unsigned int  uLength ) { clear(); m_uType = variant_type::eTypeString|variant_type::eFlagAllocate; m_uSize = (unsigned int)uLength; m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v, m_uSize); m_V.pbsz[m_uSize] = '\0'; }
#if defined(__cpp_char8_t)
   void _set_value( const char8_t* v ) { clear(); m_uType = variant_type::eTypeUtf8String|variant_type::eFlagAllocate; m_uSize = (unsigned int)strlen((const char*)v); m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v, m_uSize); m_V.pbsz[m_uSize] = '\0'; }
   void _set_value( const char8_t* v, unsigned int  uLength ) { clear(); m_uType = variant_type::eTypeUtf8String|variant_type::eFlagAllocate; m_uSize = (unsigned int)uLength; m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v, m_uSize); m_V.pbsz[m_uSize] = '\0'; }
#endif
   void _set_value( const wchar_t* v ) { clear(); m_uType = variant_type::eTypeWString|variant_type::eFlagAllocate; m_uSize = (unsigned int)wcslen(v); m_V.pwsz = (wchar_t*)allocate(( m_uSize + 1u ) * sizeof(wchar_t)); memcpy( m_V.pwsz, v, ( m_uSize + 1u ) * sizeof(wchar_t) ); }
   void _set_value( const unsigned char* v, unsigned int  uLength ) { clear(); m_uType = variant_type::eTypeBinary|variant_type::eFlagAllocate; m_uSize = (unsigned int)uLength; m_V.pb = (unsigned char*)allocate(uLength); memcpy( m_V.pb, v, uLength ); }
   void _set_value( const wchar_t* v, unsigned int uLength ) { clear(); m_uType = variant_type::eTypeWString|variant_type::eFlagAllocate; m_uSize = (unsigned int)uLength; m_V.pwsz = (wchar_t*)allocate(( m_uSize + 1u ) * sizeof(wchar_t)); memcpy( m_V.pwsz, v, (m_uSize) * sizeof(wchar_t) );  m_V.pwsz[m_uSize] = 0; }
   void _set_value( const utf8& v, variant_type::tag_utf8 ) { clear(); m_uType = variant_type::eTypeUtf8String|variant_type::eFlagAllocate; m_uSize = v.m_uLength; m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.m_pbsz,  m_uSize + 1u ); }
   void _set_value( const utf8& v, unsigned int uType, variant_type::tag_utf8 ) { clear(); m_uType = uType|variant_type::eFlagAllocate; m_uSize = v.m_uLength; m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.m_pbsz, m_uSize + 1u ); }
   void _set_value( const std::string& v ) { clear(); m_uType = variant_type::eTypeString|variant_type::eFlagAllocate; m_uSize = (unsigned int)v.length(); m_V.pbsz = (char*)allocate( m_uSize + 1u ); memcpy( m_V.pbsz, v.c_str(), m_uSize); m_V.pbsz[m_uSize] = '\0'; }
   void _set_value( const std::wstring& v ) { clear(); m_uType = variant_type::eTypeWString|variant_type::eFlagAllocate; m_uSize = (unsigned int)v.length(); m_V.pbsz = (char*)allocate( ( m_uSize + 1u ) * sizeof(wchar_t) ); memcpy( m_V.pbsz, v.c_str(), ( m_uSize + 1u ) * sizeof(wchar_t)); m_V.pwsz[m_uSize] = L'\0'; }
   // void _set_value( _variant v );

   //void _set_binary_value( const uint8_t* v, unsigned int uLength ) { clear(); m_uType = variant_type::eTypeBinary|variant_type::eFlagAllocate; m_uSize = uLength; m_V.pb = (unsigned char*)allocate( m_uSize ); memcpy( m_V.pb, v, m_uSize ); }

//@}

private:
   // common copy
   void common_construct( const variant& o ) {
      m_uType = o.m_uType;
      m_uSize = o.m_uSize;
      if( (m_uType & variant_type::eFlagAllocate) == 0 )
      {
         m_V.int64 = o.m_V.int64;
      }
      else if( type_number() == variant_type::eTypeNumberWString )
      {
         m_V.pwsz = (wchar_t*)allocate((m_uSize + 1u) * sizeof(wchar_t)); memcpy( m_V.pwsz, o.m_V.pwsz, (m_uSize + 1u) * sizeof(wchar_t) );
      }
      else if( type_number() == variant_type::eTypeNumberString || type_number() == variant_type::eTypeNumberUtf8String )
      {
         m_V.pbsz = (char*)allocate(size_t(m_uSize + 1)); memcpy( m_V.pbsz, o.m_V.pbsz, m_uSize + 1u );
      }
      else
      {
         m_V.pb = (unsigned char*)allocate(m_uSize); memcpy( m_V.pb, o.m_V.pb, m_uSize );
      }
   }

   void move_construct( variant& o ) {
      ((uint64_t*)this)[0] = ((uint64_t*)&o)[0];
      ((uint64_t*)this)[1] = ((uint64_t*)&o)[1];
      o.m_uType = variant_type::eTypeUnknown;
   }


// operator
public:
   

public:

/** \name GET/SET
*///@{
   bool get_bool() const; 
   int get_int() const;   
   int32_t get_int32() const { if(m_uType == variant_type::eTypeUInt32) return m_V.int32; else return (int32_t)get_int(); }
   unsigned int get_uint() const;   
   int64_t get_int64() const;
   uint64_t get_uint64() const;
   double get_decimal() const;
   std::string get_string() const;
   std::string_view get_string( variant_type::tag_no_allocate ) const;
   std::string_view get_string_view() const { return get_string( variant_type::tag_no_allocate{}); };
   std::wstring get_wstring() const;

   // ## as_* methods, similar to C++ stl to_
   bool as_bool() const { return get_bool(); }
   int as_int() const { return get_int(); }
   unsigned as_uint() const { return get_uint(); }
   int64_t as_int64() const { return get_int64(); }
   uint64_t as_uint64() const { return get_int64(); }
   double as_double() const { return get_decimal(); }
   std::string as_string() const { return get_string(); }
   void as_string( std::string& string_ ) const;
   std::string_view as_string_view() const { return get_string_view(); };
   std::wstring as_wstring() const { return get_wstring(); }
   variant_view as_variant_view() const;

   void set_void( void* p ) { clear(); m_uType = variant_type::eTypeVoid; m_uSize = 0; m_V.p = p; }
   void* get_void() const { return m_V.p; }
   uint64_t get_raw() const { return m_V.uint64; }
//@}

   /// @name cast_as_* convert fast to another type
   /// cast_as_* is used for type casting in a safe way, faster compared to as_* and not as strict is operator casts
   ///@{
   int16_t cast_as_int16() const noexcept;
   uint16_t cast_as_uint16() const noexcept;
   int32_t cast_as_int32() const noexcept;
   uint32_t cast_as_uint32() const noexcept;
   int64_t cast_as_int64() const noexcept;
   uint64_t cast_as_uint64() const noexcept;
   ///@}


/** \name OPERATION
*///@{
   /// Get type variant holds, last two bytes holds static type information
   uint32_t type() const noexcept { return m_uType; }
   void type( uint32_t uType ) { m_uType = uType; }
   uint32_t type_number() const noexcept { return (m_uType & eTYPE); }
   void type_number( uint32_t uType ) { m_uType &= 0xffff'ff00; m_uType |= (uType & eTYPE); }
   static uint32_t type_number_s( unsigned uType ) { return uType & variant_type::enumFilter::eFilterTypeNumber; }
   std::string_view type_name() noexcept { return variant::get_type_name_s(m_uType); }
   constexpr std::string_view type_name() const noexcept { return variant::get_type_name_s(m_uType); }
   variant_type::enumTypeNumber get_type_number() const noexcept { return variant_type::enumTypeNumber(m_uType & variant_type::enumFilter::eFilterTypeNumber); }
   variant_type::enumGroup get_type_group() const noexcept { return variant_type::enumGroup(m_uType & variant_type::enumFilter::eFilterTypeGroup); }
   static variant_type::enumGroup get_type_group_s( unsigned uType ) noexcept { return variant_type::enumGroup(uType & variant_type::enumFilter::eFilterTypeGroup); }




   // ## Group type comparison, check if value is within specified group type

   bool is_null() const          { return (m_uType == variant_type::eTypeUnknown); }
   bool is_bool() const          { return (m_uType & variant_type::eGroupBoolean ? true : false); }
   bool is_int() const           { return (m_uType & variant_type::eGroupInteger ? true : false); }
   bool is_integer() const       { return (m_uType & variant_type::eGroupInteger ? true : false); }
   bool is_signed() const        { return (m_uType & variant_type::eGroupSigned ? true : false); }
   bool is_decimal() const       { return (m_uType & variant_type::eGroupDecimal ? true : false); }
   bool is_string() const        { return (m_uType & variant_type::eGroupString ? true : false); }
   static bool is_string_s( unsigned uType ) { return (uType & variant_type::eGroupString ? true : false); }
   bool is_binary() const        { return (m_uType & variant_type::eGroupBinary ? true : false); }

   bool is_08() const noexcept   { return m_uType & variant_type::eGroupSize08; }
   bool is_16() const noexcept   { return m_uType & variant_type::eGroupSize16; }
   bool is_32() const noexcept   { return m_uType & variant_type::eGroupSize32; }
   bool is_64() const noexcept   { return m_uType & variant_type::eGroupSize64; }
   bool is_32or64() const noexcept   { return m_uType & (variant_type::eGroupSize32|variant_type::eGroupSize64); }


   bool is_json() const { return (m_uType & variant_type::enumFilter::eFilterTypeGroup) == variant_type::eTypeNumberJson ? true : false; }
   bool is_xml() const { return (m_uType & variant_type::enumFilter::eFilterTypeGroup) == variant_type::eTypeNumberXml ? true : false; }

   bool is_primitive() const { return (type_number() > variant_type::eTypeNumberUnknown && type_number() <= variant_type::eTypeNumberDouble); } ///< primitive = built in types in C++

   bool is_true() const;

   bool is_char_string() const { return (type_number() == variant_type::eTypeNumberString || type_number() == variant_type::eTypeNumberUtf8String ? true : false); }

   bool is_void() const { return (m_uType & variant_type::enumFilter::eFilterTypeGroup) == variant_type::eTypeNumberVoid ? true : false; }

   void convert( variant_type::enumType eType );
   void convert( unsigned uType ) { convert( (variant_type::enumType)uType ); }
   void convert( const std::string_view& stringType );

   bool compare( const variant& v ) const;
   bool less( const variant& v ) const;
   /// return pointer to char buffer
   const char* c_str() const {                                                 assert( is_string() );
      return m_V.pbsz; 
   }
   /// return pointer to wide char buffer
   const wchar_t* c_wstr() const {                                             assert( is_string() );
      return m_V.pwsz; 
   }
   /// return pointer to char buffer
   const uint8_t* c_str_uft8() const {                                         assert( is_string() );
      return m_V.pb_const; 
   }

   
   uint32_t length() const { return m_uSize; }

//@}


/** \name LOGICAL
*///@{
   //bool is_true() const throw();
   void clear() { 
      if( (m_uType & variant_type::eFlagAllocate) == variant_type::eFlagAllocate ) { free_(); } 
      m_uType = variant_type::eTypeUnknown; 
   }
   bool empty() const { return m_uType == variant_type::eTypeUnknown; }
   /// return pointer to internal data regardless if it is a primitive type or extended type
   const uint8_t* get_value_buffer() const noexcept;
//@}

protected:
/** \name INTERNAL
*///@{
   void* allocate( size_t uSize ) { return  ::malloc( uSize ); }
   void free_() { 
      if( !(m_uType & variant_type::eFlagLengthPrefix) ) { ::free( m_V.p ); }
      else                                               { ::free( ((unsigned char*)m_V.p - sizeof(uint32_t)) ); } 
   }
   //@}

// attributes
public:
   uint32_t m_uType;
   uint32_t m_uSize;    ///< Holds size for data that varies in size, size should be compatible with type that variant is holding. 
                        ///< If string it could be 0 and they you need to calculate size
   union value
   {
      bool        b;
      int8_t      int8;
      int16_t     int16;
      int32_t     int32;
      int64_t     int64;
      uint8_t     uint8;
      uint16_t    uint16;
      uint32_t    uint32;
      uint64_t    uint64;
      char*       pbsz;
      const char* pbsz_const;
#if defined(__cpp_char8_t)
      char8_t*    putf8;
      const char8_t* putf8_const;
#endif
      char32_t    putf32;
      wchar_t*    pwsz;
      wchar_t*    pwsz_const;
      unsigned char* pb;
      const unsigned char* pb_const;
      float       f;
      double      d;
      void*       p;   
   } m_V;

/** \name INTERNAL FREE FUNCTIONS
*///@{
   static constexpr std::string_view get_type_name_s(uint32_t uType)
   {
      switch( uType & variant::enumMASK::eTYPE )
      {
      case variant_type::enumTypeNumber::eTypeNumberUnknown: return "unknown";
      case variant_type::enumTypeNumber::eTypeNumberBool: return "boolean";
      case variant_type::enumTypeNumber::eTypeNumberInt8: return "int8";
      case variant_type::enumTypeNumber::eTypeNumberInt16: return "int16";
      case variant_type::enumTypeNumber::eTypeNumberInt32: return "int32";
      case variant_type::enumTypeNumber::eTypeNumberInt64: return "int64";
      case variant_type::enumTypeNumber::eTypeNumberUInt8: return "uint8";
      case variant_type::enumTypeNumber::eTypeNumberUInt16: return "uint16";
      case variant_type::enumTypeNumber::eTypeNumberUInt32: return "uint32";
      case variant_type::enumTypeNumber::eTypeNumberUInt64: return "uint64";
      case variant_type::enumTypeNumber::eTypeNumberFloat: return "float";
      case variant_type::enumTypeNumber::eTypeNumberDouble: return "double";
      case variant_type::enumTypeNumber::eTypeNumberGuid:  return "guid";
      case variant_type::enumTypeNumber::eTypeNumberUtf8String: return "utf8";
      case variant_type::enumTypeNumber::eTypeNumberUtf32String: return "utf32";
      case variant_type::enumTypeNumber::eTypeNumberString: return "string";
      case variant_type::enumTypeNumber::eTypeNumberWString: return "wstring";
      case variant_type::enumTypeNumber::eTypeNumberBinary:  return "binary";
      case variant_type::enumTypeNumber::eTypeNumberJson: return "json";
      case variant_type::enumTypeNumber::eTypeNumberXml: return "xml";
      case variant_type::enumTypeNumber::eTypeNumberVoid: return "void";
      case variant_type::enumTypeNumber::eTypeNumberBit: return "bit";
         break;
      }

      return std::string_view();
   }
//@}

// ## free functions ------------------------------------------------------------
public:
   static std::vector<variant>::const_iterator find_s( const std::vector<variant>& vectorFindIn, const variant& variantFind  );
   static std::vector<std::pair<variant, variant>>::const_iterator find_s( const std::vector< std::pair<variant, variant> >& vectorFindIn, std::pair<const variant&, const variant&> pairFind );
   /// Convert variant from one type to another
   static bool convert_to_s( const gd::variant* pvariantFrom, gd::variant* pvariantTo, unsigned uType );

   // ## Methods used to compute meta data about value in variant 

   static unsigned compute_digit_count_s( uint32_t uNumber );
   static unsigned compute_digit_count_s( int32_t iNumber );
   static unsigned compute_digit_count_s( uint64_t uNumber );
   static unsigned compute_digit_count_s( int64_t iNumber );
   static unsigned compute_digit_count_s( double dNumber, variant_type::tag_scientific );
   static unsigned compute_digit_count_s( const variant* pvariantValue );

   static unsigned compute_fraction_count_s( double dNumber, unsigned uMax );

   /// check if double value should be printed to string with exponent marker
   static bool is_exponent_s( double dNumber, uint64_t uMax );

   /// compute
   static unsigned compute_ascii_size_s( const variant* pvariantValue );

   /// check for string compatible with char buffer, usefull when working with ascii and utf8
   static bool is_char_string_s( unsigned uType ) { return (type_number_s( uType ) == variant_type::eTypeNumberString || type_number_s( uType ) == variant_type::eTypeNumberUtf8String ? true : false); }

   /// convert and concatenate variant values to string and return the concatenated full string
   static std::string format_s( const std::initializer_list<variant>& listValue );
   static void format_s( const std::initializer_list<variant>& listValue, std::string& stringFormat );
   template<typename... VARIANTS>
   static std::string format_s(variant first_, VARIANTS&&... rest_) {
      std::initializer_list<variant> list_{ first_, rest_... };
      return format_s( list_ );
   }
};

/// Return pointer to internal data regardless if it is a primitive type or extended type
inline const uint8_t* variant::get_value_buffer() const noexcept { 
   if( is_primitive() ) return (uint8_t*)this + offsetof(variant, m_V); 
   return (const uint8_t*)m_V.pb_const;
}

/// cast internal value to int16, make sure that you know that value is within the bounds for int16
inline int16_t variant::cast_as_int16() const  noexcept {                                          assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int16_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.int16;
}

/// cast internal value to uint16, make sure that you know that value is within the bounds for uint16
inline uint16_t variant::cast_as_uint16() const noexcept {                                         assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int16_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.uint16;
}

/// cast internal value to int32, make sure that you know that value is within the bounds for int32
inline int32_t variant::cast_as_int32() const noexcept {                                           assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.int32;
}

/// cast internal value to uint32, make sure that you know that value is within the bounds for uint32
inline uint32_t variant::cast_as_uint32() const noexcept {                                         assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.uint32;
}

/// cast internal value to int64, make sure that you know that value is within the bounds for int64
inline int64_t variant::cast_as_int64() const noexcept {                                           assert( gd::types::value_size_g( m_uType ) == sizeof(int64_t)); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.int64;
}

/// cast internal value to uint64, make sure that you know that value is within the bounds for uint64
inline uint64_t variant::cast_as_uint64() const noexcept {                                         assert( gd::types::value_size_g( m_uType ) == sizeof(int64_t) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.uint64;
}


/** ---------------------------------------------------------------------------
 * @brief convert c++ stl variant to variant
 * @param v_ c++ stl variant holding value that is converted to variant
 * @return variant value in variant object
*/
template< typename VARIANT >
variant to_variant_g( const VARIANT& v_, variant_type::tag_std_variant ) {

   struct convert
   {
      void operator()(bool v_) { m_variant = v_; }
      void operator()(int8_t v_) { m_variant = v_; }
      void operator()(uint8_t v_) { m_variant = v_; }
      void operator()(int16_t v_) { m_variant = v_; }
      void operator()(uint16_t v_) { m_variant = v_; }
      void operator()(int32_t v_) { m_variant = v_; }
      void operator()(uint32_t v_) { m_variant = v_; }
      void operator()(int64_t v_) { m_variant = v_; }
      void operator()(uint64_t v_) { m_variant = v_; }
      void operator()(double v_) { m_variant = v_; }
      void operator()(void* v_) { m_variant = v_; }
      void operator()(const std::string& v_) { m_variant = v_; }
      void operator()(const std::string_view& v_) { m_variant.assign( (const char*)v_.data(), (size_t)v_.length() ); }

      operator variant() { return m_variant; }

      variant m_variant;
   } convert_;

   std::visit(convert_, v_);
   return convert_;
}




// ä'static_assert( sizeof(_variant) == 16, "_variant size isn't 16 bytes" );
static_assert( sizeof(variant) == 16, "variant size isn't 16 bytes" );

namespace debug {
   std::string print( const variant& v );
   std::string print_value( const variant& v );
   std::string print( const std::vector<variant>& v_ );
}

} // namespace gd


#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif

