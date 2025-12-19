// @FILE [tag: variant, view] [description: General variant item that owns its data, manages all primitive types and some common] [type: header]

/**
 * \file gd_variant.h
 *
 * \brief variant stores any primitive value and some common derived value types
 *
 * Variant is very useful when you need runtime type information for a value.
 * It is similar to gd::variant but it do not own the data, it just holds a view to the data.
 *
 | Area                | Methods (Examples)                                                                                      | Description                                                                                   |
 |---------------------|--------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `variant(...)`, `common_construct(...)`, `move_construct(...)`                                         | Constructors for creating, copying, and moving variant instances with various types.          |
 | Assignment          | `assign(...)`, `operator=`, `_set_value(...)`, `set_void(...)`                                         | Methods for assigning or updating the value and type of a variant.                            |
 | Arithmetic/Operators| `operator+`, `operator-`, `operator+=`, `operator-=`, `operator==`, `operator!=`, `operator<`         | Arithmetic and comparison operators for supported types.                                      |
 | Retrieval           | `get_bool()`, `get_int()`, `get_int32()`, `get_uint()`, `get_int64()`, `get_string()`, `as<TYPE>()`    | Methods for retrieving the value in the desired type, including type-safe conversions.        |
 | Type Queries        | `type()`, `type_number()`, `type_name()`, `get_type_number()`, `get_type_group()`, `is_*()`            | Methods for querying the type, group, and properties of the stored value.                     |
 | Conversion          | `convert(...)`, `cast_as_int16()`, `cast_as_uint16()`, `cast_as_int32()`, `cast_as_uint32()`           | Methods for converting the stored value to another type or casting between compatible types.  |
 | Comparison          | `compare(...)`, `less(...)`                                                                            | Methods for comparing values and types between variants.                                      |
 | Utility/Meta        | `clear()`, `empty()`, `length()`, `get_value_buffer()`, `allocate(...)`, `free_()`                     | Utility methods for memory management, clearing, and querying variant metadata.               |
 | Printing/Debug      | `debug::print(...)`, `debug::print_value(...)`                                                         | Methods for formatting and printing variant values for debugging.                             |
 | Static/Helpers      | `find_s(...)`, `convert_to_s(...)`, `compute_digit_count_s(...)`, `format_s(...)`                      | Static helper methods for searching, conversion, formatting, and metadata computation.        |
 */

#pragma once

#if defined(max)
#  undef max
#endif

#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <type_traits>
#include <limits>

#include "gd_types.h"
#include "gd_variant.h"
#include "gd_debug.h"

#ifndef _GD_BEGIN_VARIANT_VIEW
#  define _GD_BEGIN_VARIANT_VIEW
#endif

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

/**
 * \brief variant_view holds type and value
 *
 *
 *
 \code
 \endcode
 */
class variant_view
{
public:

private:
   enum enumMASK
   {
      eTYPE             = 0x000000ff,  ///< filter type value
      eLENGTH           = 0x00020000,  ///< item is prefixed with length
   };


// ## @API [tag: construct] [description: construction methods for creating variant instances]
public:
   variant_view()               : m_uType(variant_type::eTypeUnknown)    {}
   variant_view( bool b )       : m_uType(variant_type::eTypeBool)       { m_V.b = b; }
   variant_view( int8_t v )     : m_uType(variant_type::eTypeInt8)       { m_V.int8 = v; }
   variant_view( int16_t v )    : m_uType(variant_type::eTypeInt16)      { m_V.int16 = v; }
   variant_view( int32_t v )    : m_uType(variant_type::eTypeInt32)      { m_V.int32 = v; }
   variant_view( int64_t v )    : m_uType(variant_type::eTypeInt64)      { m_V.int64 = v; }
   variant_view( uint8_t v )    : m_uType(variant_type::eTypeUInt8)      { m_V.uint8 = v; }
   variant_view( uint16_t v )   : m_uType(variant_type::eTypeUInt16)     { m_V.uint16 = v; }
   variant_view( uint32_t v )   : m_uType(variant_type::eTypeUInt32)     { m_V.uint32 = v; }
   variant_view( uint64_t v )   : m_uType(variant_type::eTypeUInt64)     { m_V.uint64 = v; }
   variant_view( float v )      : m_uType(variant_type::eTypeCFloat)     { m_V.f = v; }
   variant_view( double v )     : m_uType(variant_type::eTypeCDouble)    { m_V.d = v; }
   variant_view( void* p )      : m_uType(variant_type::eTypePointer)    { m_V.p = p; }
   variant_view( const char* v) : m_uType(variant_type::eTypeString), m_uSize(strlen(v)) { m_V.pbsz_const = v;  }
   variant_view( const char8_t* v) : m_uType(variant_type::eTypeUtf8String), m_uSize(strlen((const char*)v)) { m_V.putf8_const = v;  }
   variant_view( const wchar_t* v) : m_uType(variant_type::eTypeWString), m_uSize(wcslen(v)) { m_V.pwsz_const = v; }
   variant_view( const char* v, size_t uLength) : m_uType(variant_type::eTypeString), m_uSize(uLength) { m_V.pbsz_const = v; }
   variant_view( const std::string& v) : m_uType(variant_type::eTypeString), m_uSize(v.length()) { m_V.pbsz_const = v.c_str(); }
   variant_view( const std::string_view& v) : m_uType(variant_type::eTypeString), m_uSize(v.length()) { m_V.pbsz_const = v.data(); }
   variant_view( const char* v, size_t uLength, bool ): m_uType(variant_type::eTypeString), m_uSize(uLength) { m_V.pbsz = const_cast<char*>(v); }
   variant_view( const wchar_t* v, size_t uLength) : m_uType(variant_type::eTypeWString), m_uSize(uLength) { m_V.pwsz_const = v; }
   variant_view( const unsigned char* v, size_t uLength) : m_uType(variant_type::eTypeBinary), m_uSize(uLength) { m_V.pb_const = v; }
   variant_view( const variant_type::utf8& v) : m_uType(variant_type::eTypeUtf8String), m_uSize(v.m_uLength) { m_V.pbsz_const = v.m_pbsz; }
   variant_view( const variant_type::utf8& v, unsigned int uType) : m_uType(uType), m_uSize(v.m_uLength) { m_V.pbsz_const = v.m_pbsz; }
   variant_view( const variant_type::uuid& v) : m_uType(variant_type::eTypeGuid), m_uSize(16) { m_V.pb_const = v.m_pbUuid; }
   variant_view( const variant_type::guid& v ): m_uType(variant_type::eTypeGuid), m_uSize(sizeof(variant_type::guid)) { m_V.pb_const = (unsigned char*)&v; }
   variant_view( unsigned int uType, void* v, size_t uLength) : m_uType(uType), m_uSize(uLength) { m_V.p = v; }
   variant_view( unsigned int uType, uint64_t v, size_t uLength) : m_uType(uType), m_uSize(uLength) { m_V.uint64 = v; }
   variant_view( const gd::types::binary& v ): m_uType(variant_type::eTypeBinary), m_uSize(v.length()) { m_V.pb_const = v.data(); }

   variant_view( const char* v, bool) : m_uType(variant_type::eTypeString), m_uSize(strlen(v)) { m_V.pbsz_const = v; }
   variant_view( std::string_view v, bool) : m_uType(variant_type::eTypeString), m_uSize(v.length()) { m_V.pbsz_const = v.data(); }

   /// copies variant data into variant_view, this is possible because they are binary compatible, just that variant view do not own the data:
   /// \note flag marking that data is allocated in variant is cleared
   explicit variant_view( const gd::variant& v ) { memcpy(static_cast<void*>(this), &v, sizeof(variant_view) ); m_uType &= ~variant_type::eFlagAllocate; }
   explicit variant_view( const gd::variant* pv ) { memcpy(static_cast<void*>(this), pv, sizeof(variant_view) ); m_uType &= ~variant_type::eFlagAllocate; }

   variant_view( const variant_view& o ) { common_construct( o ); }            // copy
   variant_view( variant_view&& o ) noexcept { common_construct( std::move( o ) ); }// move

// ## @API [tag: operator] [description: operators like =, +=, -=, +, -]

   variant_view& operator=( const variant_view& o ) {
      common_construct( o );
      return *this;
   }
   variant_view& operator=( variant_view&& o ) noexcept {
      if( this != &o ) { ((uint64_t*)this)[0] = ((uint64_t*)&o)[0]; ((uint64_t*)this)[1] = ((uint64_t*)&o)[1]; o.m_uType = variant_type::eTypeUnknown; }
      return *this; }
   ~variant_view() {}

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
   void operator=(const char* v) { clear(); m_uType = variant_type::eTypeString; m_uSize = (unsigned int)strlen(v); m_V.pbsz_const = v; }
   void operator=(const std::string& v) { clear(); m_uType = variant_type::eTypeString; m_uSize = (unsigned int)v.length(); m_V.pbsz_const = v.c_str(); }
#if defined(__cpp_char8_t)
   void operator=(const char8_t* v) { clear(); m_uType = variant_type::eTypeUtf8String; m_uSize = (unsigned int)strlen((const char*)v); m_V.putf8_const = v; }
#endif
   void operator=(const wchar_t* v) { clear(); m_uType = variant_type::eTypeWString; m_uSize = (unsigned int)wcslen(v); m_V.pwsz_const = v; }
   void operator=(const variant_type::utf8& v) { clear(); m_uType = variant_type::eTypeUtf8String; m_uSize = v.m_uLength; m_V.pbsz_const = v.m_pbsz; }
   void operator=(const variant_type::uuid& v) { clear(); m_uType = variant_type::eTypeGuid; m_uSize = 16; m_V.pb_const = v.m_pbUuid; }

   int32_t operator+(int32_t v) { return m_V.int32 + v; }
   uint32_t operator+(uint32_t v) { return m_V.uint32 + v; }
   int64_t operator+(int64_t v) { return m_V.int64 + v; }
   uint64_t operator+(uint64_t v) { return m_V.uint64 + v; }
   int32_t operator-(int32_t v) { return m_V.int32 - v; }
   uint32_t operator-(uint32_t v) { return m_V.uint32 - v; }
   int64_t operator-(int64_t v) { return m_V.int64 - v; }
   uint64_t operator-(uint64_t v) { return m_V.uint64 - v; }

   variant_view& operator+=(int32_t v) { m_V.int32 += v; return *this; }
   variant_view& operator+=(uint32_t v) { m_V.uint32 += v; return *this; }
   variant_view& operator+=(int64_t v) { m_V.int64 += v; return *this; }
   variant_view& operator+=(uint64_t v) { m_V.uint64 += v; return *this; }
   variant_view& operator-=(int32_t v) { m_V.int32 -= v; return *this; }
   variant_view& operator-=(uint32_t v) { m_V.uint32 -= v; return *this; }
   variant_view& operator-=(int64_t v) { m_V.int64 -= v; return *this; }
   variant_view& operator-=(uint64_t v) { m_V.uint64 -= v; return *this; }

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
   operator const unsigned char*() const { assert(type_number() == variant_type::eTypeNumberGuid || type_number() == variant_type::eTypeNumberBinary); return m_V.pb; }

   operator std::string_view() const { assert(type_number() == variant_type::eTypeNumberString || type_number() == variant_type::eTypeNumberUtf8String || type_number() == variant_type::eTypeNumberJson || type_number() == variant_type::eTypeNumberXml ); return std::string_view( m_V.pbsz, m_uSize ); }

   bool operator==( bool v_ ) const { return compare( variant_view( v_ ) ); }
   bool operator==( const variant_view& o ) const { return compare( o ); }
   bool operator!=( const variant_view& o ) const { return compare( o ) == false; }

   bool operator<( const variant_view& o ) const { return less( o ); }

// ## @API [tag: assign] [description: assign methods, setting value of variant]

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
   //void assign( const char* v, uint32_t uLength ) { _set_value( v, uLength ); }
   void assign( const char* v, uint64_t uLength ) { assert( uLength < std::numeric_limits<uint32_t>::max() ); _set_value( v, (uint32_t)uLength ); }
   void assign( const std::string_view& v ) { _set_value( v.data(), v.length() ); }
#if defined(__cpp_char8_t)
   void assign( const char8_t* v ) { _set_value( v ); }
#endif
   void assign( const wchar_t* v ) { _set_value( v ); }
   void assign( const unsigned char* v, size_t uLength ) { _set_value( v, uLength ); }
   void assign( const wchar_t* v, unsigned int uLength ) { _set_value( v, uLength ); }
   void assign( const variant_type::utf8& v ) { _set_value( v ); }
   void assign( const variant_type::utf8& v, unsigned int uType ) { _set_value( v, uType ); }
   void assign(const variant_type::uuid& v) { _set_value(v); }

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
   void _set_value( const char* v ) { clear(); m_uType = variant_type::eTypeString; m_uSize = (unsigned int)strlen(v); m_V.pbsz_const = v; }
   void _set_value( const char* v, unsigned int  uLength ) { clear(); m_uType = variant_type::eTypeString; m_uSize = (unsigned int)uLength; m_V.pbsz_const = v; }
#if defined(__cpp_char8_t)
   void _set_value( const char8_t* v ) { clear(); m_uType = variant_type::eTypeUtf8String; m_uSize = (unsigned int)strlen((const char*)v); m_V.putf8_const = v; }
   void _set_value( const char8_t* v, unsigned int  uLength ) { clear(); m_uType = variant_type::eTypeUtf8String; m_uSize = (unsigned int)uLength; m_V.putf8_const = v; }
#endif
   void _set_value( const wchar_t* v ) { clear(); m_uType = variant_type::eTypeWString; m_uSize = (unsigned int)wcslen(v); m_V.pwsz_const = v; }
   void _set_value( const unsigned char* v, unsigned int  uLength ) { clear(); m_uType = variant_type::eTypeBinary; m_uSize = (unsigned int)uLength; m_V.pb_const = v; }
   void _set_value( const wchar_t* v, unsigned int uLength ) { clear(); m_uType = variant_type::eTypeWString; m_uSize = (unsigned int)uLength; m_V.pwsz_const = v; }
   void _set_value( const variant_type::utf8& v ) { clear(); m_uType = variant_type::eTypeUtf8String; m_uSize = v.m_uLength; m_V.pbsz_const = v.m_pbsz; }
   void _set_value(const variant_type::utf8& v, unsigned int uType) { clear(); m_uType = uType; m_uSize = v.m_uLength; m_V.pbsz_const = v.m_pbsz; }
   void _set_value(const variant_type::uuid& v) { clear(); m_uType = variant_type::eTypeGuid; m_uSize = 16; m_V.pb_const = v.m_pbUuid; }

   // void _set_value( _variant v );

   //void _set_binary_value( const uint8_t* v, unsigned int uLength ) { clear(); m_uType = variant_type::eTypeBinary; m_uSize = uLength; m_V.pb = (unsigned char*)allocate( m_uSize ); memcpy( m_V.pb, v, m_uSize ); }

//@}

private:
   // common copy
   void common_construct( const variant_view& o ) {
      ((uint64_t*)this)[0] = ((uint64_t*)&o)[0];
      ((uint64_t*)this)[1] = ((uint64_t*)&o)[1];
   }

   void common_construct( variant_view&& o ) {
      ((uint64_t*)this)[0] = ((uint64_t*)&o)[0];
      ((uint64_t*)this)[1] = ((uint64_t*)&o)[1];
      o.m_uType = variant_type::eTypeUnknown;
   }


// operator
public:


public:

// ## @API [tag: get/set] [description: getter and setter methods]

/** \name GET/SET
*///@{
   bool get_bool() const;
   int get_int() const;
   //int32_t get_int32() const { if(m_uType == variant_type::eTypeUInt32) return m_V.int32; else return (int32_t)get_int(); }
   unsigned int get_uint() const;
   int64_t get_int64() const;
   uint64_t get_uint64() const;
   double get_decimal() const;
   double get_double() const { return get_decimal(); };
   std::string get_string() const;
   std::string get_string( gd::variant_type::tag_scientific ) const;
   std::string_view get_string_view() const;
   std::wstring get_wstring() const;

   // ## as_* methods, similar to C++ stl to_
   bool as_bool() const { return get_bool(); }
   int as_int() const { return get_int(); }
   unsigned as_uint() const { return get_uint(); }
   int64_t as_int64() const { return get_int64(); }
   uint64_t as_uint64() const { return get_uint64(); }
   double as_double() const { return get_decimal(); }
   std::string as_string() const { return get_string(); }
   std::string as_string( gd::variant_type::tag_scientific ) const { return get_string( gd::variant_type::tag_scientific{}); }
   std::wstring as_wstring() const { return get_wstring(); }
   std::string_view as_string_view() const { return get_string_view(); }
   gd::variant as_variant() const;
   void* as_void() const { return get_void(); }
   /// get value as template type
   template <typename TYPE> TYPE as() const;

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

   // ## buffer methods, use this if you need speed, no heap allocations

   char* get_string( char* pbszBuffer ) const;

   void set_void( void* p ) { clear(); m_uType = variant_type::eTypeVoid; m_uSize = 0; m_V.p = p; }
   void* get_void() const { return m_V.p; }

   const uint8_t* get_value_buffer() const noexcept;

   /// get raw union value, this doesn't mean that variant has a uint64_t value
   uint64_t get_raw() const { return m_V.uint64; }
//@}

/** \name OPERATION
*///@{
   /// Get type variant holds, last two bytes holds static type information
   uint32_t type() const noexcept { return m_uType; }
   uint32_t type_number() const noexcept { return (m_uType & eTYPE); }
   std::string_view type_name() { return variant::get_type_name_s(m_uType); }
   constexpr std::string_view type_name() const { return variant::get_type_name_s(m_uType); }
   variant_type::enumTypeNumber get_type_number() const noexcept { return variant_type::enumTypeNumber(m_uType & variant_type::enumFilter::eFilterTypeNumber); }
   variant_type::enumGroup get_type_group() const noexcept { return variant_type::enumGroup(m_uType & variant_type::enumFilter::eFilterTypeGroup); }

   /// Set type, make sure you know why
   void set_type( uint32_t uType ) { m_uType = uType; }

// ## @API [tag: is] [description: is methods used to ask variant_view for its type and value]

   bool is_null() const { return (m_uType == variant_type::eTypeUnknown); }
   bool is_bool() const { return (m_uType & variant_type::eGroupBoolean ? true : false); }
   bool is_int() const { return (m_uType & variant_type::eGroupInteger ? true : false); }
   bool is_integer() const { return (m_uType & variant_type::eGroupInteger ? true : false); }
   bool is_decimal() const { return (m_uType & variant_type::eGroupDecimal ? true : false); }
   bool is_number() const { return m_uType & (variant_type::eGroupInteger | variant_type::eGroupDecimal) ? true : false; }
   bool is_string() const { return (m_uType & variant_type::eGroupString ? true : false); }
   bool is_binary() const { return (m_uType & variant_type::eGroupBinary ? true : false); }

   bool is_08() const noexcept   { return m_uType & variant_type::eGroupSize08; }
   bool is_16() const noexcept   { return m_uType & variant_type::eGroupSize16; }
   bool is_32() const noexcept   { return m_uType & variant_type::eGroupSize32; }
   bool is_64() const noexcept   { return m_uType & variant_type::eGroupSize64; }
   bool is_32or64() const noexcept   { return m_uType & (variant_type::eGroupSize32|variant_type::eGroupSize64); }

   bool is_json() const { return (m_uType & variant_type::enumFilter::eFilterTypeNumber) == variant_type::eTypeNumberJson ? true : false; }
   bool is_xml() const { return (m_uType & variant_type::enumFilter::eFilterTypeNumber) == variant_type::eTypeNumberXml ? true : false; }
   bool is_void() const { return (((m_uType & variant_type::enumFilter::eFilterTypeNumber) == variant_type::eTypeNumberVoid) || ((m_uType & variant_type::enumFilter::eFilterTypeNumber) == variant_type::eTypeNumberPointer)) ? true : false; }

   bool is_primitive() const { return (type_number() > variant_type::eTypeNumberUnknown && type_number() <= variant_type::eTypeNumberDouble); } ///< primitive = built in types in C++

   bool is_true() const;

   bool is_char_string() const { return (type_number() == variant_type::eTypeNumberUtf8String || type_number() == variant_type::eTypeNumberString ? true : false); }

   /// Adjust internal size to match the size spefied (this needs internal knowledge of the memory used)
   void adjust( unsigned uMemorySize );
   /// change internal value, remember to not change to types that need to allocate data
   void convert( variant_type::enumType eType );
   /// convert internal value to new variant value with specified type
   bool convert_to( unsigned uType, variant& variantTo ) const;
   gd::variant convert_to( unsigned uType ) const;
   gd::variant convert_to( const std::string_view& stringType ) const;

   /// return pointer to char buffer
   const char* c_str() const {                                                                     assert( is_string() );
      return m_V.pbsz;
   }
   /// return pointer to wide char buffer
   const wchar_t* c_wstr() const {                                                                 assert( is_string() );
      return m_V.pwsz;
   }

   const uint8_t* data() const noexcept;

// ## @API [tag: compare] [description: methods to compare value in variant_view]

   bool compare( const variant_view& v ) const;
   bool compare( const std::string_view& string_, variant_type::tag_explicit ) const noexcept { assert(is_char_string()); return (string_.length() == length() && memcmp( m_V.p, string_.data(), length() ) == 0); }

   bool less( const variant_view& v ) const;
   uint32_t length() const { return m_uSize; }
   uint32_t length_in_bytes() const;

   void to( std::string& stringOut );

//@}

/** \name LOGICAL
*///@{
   //bool is_true() const throw();
   void clear() {
      //if( (m_uType & variant_type::eFlagAllocate) == variant_type::eFlagAllocate ) { free_(); }
      m_uType = variant_type::eTypeUnknown;
   }
   bool empty() const { return m_uType == variant_type::eTypeUnknown; }
//@}

protected:
/** \name INTERNAL
*///@{
   //void* allocate( size_t uSize ) { return  ::malloc( uSize ); }
   void free_() {
      //if( !(m_uType & variant_type::eFlagLengthPrefix) ) { ::free( m_V.p ); }
      //else                                               { ::free( ((unsigned char*)m_V.p - sizeof(uint32_t)) ); }
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
      const wchar_t* pwsz_const;
      unsigned char* pb;
      const unsigned char* pb_const;
      float       f;
      double      d;
      void*       p;
   } m_V;

// ## free functions ------------------------------------------------------------
public:

   static void copy_s( const variant_view& variantviewFrom,  variant& variantTo );
   static void copy_s( const variant& variantFrom, variant_view& variantviewTo );

   static constexpr std::string_view get_type_name_s(uint32_t uType)
   {
      switch( uType & variant::enumMASK::eTYPE )
      {
      case variant_type::enumTypeNumber::eTypeNumberUnknown: return "unknown";
      case variant_type::enumTypeNumber::eTypeNumberBit: return "bit";
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
      case variant_type::enumTypeNumber::eTypeNumberPointer: return "pointer";
      case variant_type::enumTypeNumber::eTypeNumberGuid:  return "guid";
      case variant_type::enumTypeNumber::eTypeNumberBinary:  return "binary";
      case variant_type::enumTypeNumber::eTypeNumberUtf8String: return "utf8";
      case variant_type::enumTypeNumber::eTypeNumberUtf32String: return "utf32";
      case variant_type::enumTypeNumber::eTypeNumberString: "string";
      case variant_type::enumTypeNumber::eTypeNumberWString: "wstring";
      case variant_type::enumTypeNumber::eTypeNumberJson: return "json";
      case variant_type::enumTypeNumber::eTypeNumberXml: return "xml";
      case variant_type::enumTypeNumber::eTypeNumberVoid: return "void";
         break;
      }

      return std::string_view();
   }

   /// convert and concatenate variant values to string and return the concatenated full string
   static std::string format_s( const std::initializer_list<variant_view>& listValue );
   static void format_s( const std::initializer_list<variant_view>& listValue, std::string& stringFormat );
   template<typename... VARIANTS>
   static std::string format_s(variant_view first_, VARIANTS&&... rest_) {
      std::initializer_list<variant_view> list_{ first_, rest_... };
      return format_s( list_ );
   }

   static variant_view parse_to_primitive_s( const std::string_view& stringValue );
};

/**
 * @brief method to simplify code writing, using similar style as `std::get`
 * *sample code*
 * @code
gd::variant_view variantview_( "Hello World!" );
std::string stringText = variantview_.as<std::string>();
std::cout << stringText << std::endl;
auto stringAlsoText = variantview_.as<decltype(stringText)>();
std::cout << stringText << std::endl;
assert( stringText == stringAlsoText );
 * @endcode
 * @tparam TYPE value type to convert/return
 * @return return the value as specified type
 */
template<typename TYPE>
inline TYPE variant_view::as() const {
   if constexpr ( std::is_same_v<TYPE, std::string> ) {
      return as_string();
   }
   else if constexpr ( std::is_same_v<TYPE, std::string_view> ) {
      return as_string_view();
   }
   else if constexpr ( std::is_same_v<TYPE, std::wstring> ) {
      return as_wstring();
   }
   else if constexpr ( std::is_same_v<TYPE, bool> ) {
      return as_bool();
   }
   else if constexpr ( std::is_same_v<TYPE, int> ) {
      return as_int();
   }
   else if constexpr ( std::is_same_v<TYPE, unsigned> ) {
      return as_uint();
   }
   else if constexpr ( std::is_same_v<TYPE, int64_t> ) {
      return as_int64();
   }
   else if constexpr ( std::is_same_v<TYPE, uint64_t> ) {
      return as_uint64();
   }
   else if constexpr ( std::is_same_v<TYPE, double> ) {
      return as_double();
   }
   else if constexpr ( std::is_same_v<TYPE, void*> ) {
      return as_void();
   }
   else {
      static_assert(gdd::always_false<TYPE>::value, "unsupported type");
   }
}


/// Return variant_view as variant object
/// Converts variant_view object to variant
inline gd::variant variant_view::as_variant() const {
   gd::variant variantResult;
   copy_s( *this, variantResult );
   return variantResult;
}

/// Return pointer to internal data regardless if it is a primitive type or extended type
inline const uint8_t* variant_view::get_value_buffer() const noexcept {
   if( is_primitive() ) return (uint8_t*)this + offsetof(variant_view, m_V);
   return (const uint8_t*)m_V.pb_const;
}


/// cast internal value to int16, make sure that you know that value is within the bounds for int16
inline int16_t variant_view::cast_as_int16() const  noexcept {                                     assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int16_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.int16;
}

/// cast internal value to uint16, make sure that you know that value is within the bounds for uint16
inline uint16_t variant_view::cast_as_uint16() const noexcept {                                    assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int16_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.uint16;
}

/// cast internal value to int32, make sure that you know that value is within the bounds for int32
inline int32_t variant_view::cast_as_int32() const noexcept {                                      assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.int32;
}

/// cast internal value to uint32, make sure that you know that value is within the bounds for uint32
inline uint32_t variant_view::cast_as_uint32() const noexcept {                                    assert( (gd::types::value_size_g( m_uType ) == sizeof(int64_t)) || (gd::types::value_size_g( m_uType ) == sizeof( int32_t)) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.uint32;
}

/// cast internal value to int64, make sure that you know that value is within the bounds for int64
inline int64_t variant_view::cast_as_int64() const noexcept {                                      assert( gd::types::value_size_g( m_uType ) == sizeof(int64_t)); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.int64;
}

/// cast internal value to uint64, make sure that you know that value is within the bounds for uint64
inline uint64_t variant_view::cast_as_uint64() const noexcept {                                    assert( gd::types::value_size_g( m_uType ) == sizeof(int64_t) ); assert( m_uType & gd::types::eTypeGroupInteger );
   return m_V.uint64;
}

/// convert value to variant with specified type
inline gd::variant variant_view::convert_to( unsigned uType ) const {
   gd::variant variantTo;
   convert_to( uType, variantTo );
   return variantTo;
}

/// convert value to variant with specified type
inline gd::variant variant_view::convert_to( const std::string_view& stringType ) const
{
   auto eType = gd::types::type_g( stringType );                                                   assert( eType != gd::types::eTypeUnknown );
   return convert_to( (unsigned)eType );
}

/** ---------------------------------------------------------------------------
 * @brief convert c++ stl variant to variant view
 * @param v_ c++ stl variant holding value that is converted to variant_view
 * @return variant value in variant_view object
*/
template< typename VARIANT >
variant_view to_variant_view_g( const VARIANT& v_, variant_type::tag_std_variant ) {

   struct convert
   {
      void operator()(bool v_) { m_variant_view = v_; }
      void operator()(int8_t v_) { m_variant_view = v_; }
      void operator()(uint8_t v_) { m_variant_view = v_; }
      void operator()(int16_t v_) { m_variant_view = v_; }
      void operator()(uint16_t v_) { m_variant_view = v_; }
      void operator()(int32_t v_) { m_variant_view = v_; }
      void operator()(uint32_t v_) { m_variant_view = v_; }
      void operator()(int64_t v_) { m_variant_view = v_; }
      void operator()(uint64_t v_) { m_variant_view = v_; }
      void operator()(double v_) { m_variant_view = v_; }
      void operator()(void* v_) { m_variant_view = v_; }
      void operator()(const std::string& v_) { m_variant_view = v_; }
      void operator()(const std::string_view& v_) { m_variant_view.assign( (const char*)v_.data(), (size_t)v_.length() ); }

      operator variant_view() { return m_variant_view; }

      variant_view m_variant_view;
   } convert_;

   std::visit(convert_, v_);
   return convert_;
}


// �'static_assert( sizeof(_variant) == 16, "_variant size isn't 16 bytes" );
static_assert( sizeof(variant_view) == 16, "variant size isn't 16 bytes" );
static_assert( sizeof( gd::variant ) == sizeof( gd::variant_view ), "variant and variant_view have different sizes!!!" );

// @DEBUG @API [tag: variant, debug] [summary: Variant debug methods, prints information about variants]

namespace debug {
   std::string print( const variant_view& v );
   std::string print_value( const variant_view& v );
   std::string print( const std::vector<variant_view>& v_ );
   std::string print( const std::vector<variant_view>& v_, std::function< std::string( const variant_view& ) > callback_ );
}

_GD_END

#if defined(__clang__)
   #pragma GCC diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
