/**
 * \file gd_types.h
 * 
 * \brief Core constants for gd type system
 * 
 * gd code is based on a type system where each primitive type has a number. This
 * type system is vital for all gd code and with that type system different logic
 * within gd is able to communicate without knowing about code in different parts.
 * 
 */



#pragma once

#include <cassert>
#include <type_traits>
#include <stdint.h>
#include <string>
#include <string_view>
#include <list>
#include <vector>

#ifndef _GD_TYPES
#define _GD_TYPES
#endif

#ifndef _GD_TYPES_BEGIN
#define _GD_TYPES_BEGIN namespace gd { namespace types {
#define _GD_TYPES_END } }
_GD_TYPES_BEGIN
#else
_GD_TYPES_BEGIN
#endif


#if defined( __clang__ )
   #pragma GCC diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 4267 26495 26812 )
#endif

/*
 * tag dispatchers used in gd code
 * tag dispatchers have zero cost and removes the need to create a lot of different names for methods
 */

// ----------------------------------------------------------------------------
// ## value related tag dispatchers

struct tag_date {};           ///< date is used in some form
struct tag_description {};    ///< description is used in some form
struct tag_id {};             ///< id is used in some form
struct tag_key {};            ///< key is used in some form
struct tag_name {};           ///< name is used in some form
struct tag_table {};          ///< table is used in some form
struct tag_tag {};            ///< tag value
struct tag_value {};          ///< value used in some form

struct tag_main_type {};      /// main type if there are secondary types
struct tag_secondary_type {}; /// secondary type if there are main types

struct tag_method {};         ///< method is used in some form
struct tag_property {};       ///< property is used in some form
struct tag_variable {};       ///< variable is used in some form


// ----------------------------------------------------------------------------
// ## stl tag dispatchers, used to glue with stl

struct tag_list {};           ///< stl std::list object is used in some form
struct tag_map {};            ///< stl std::map object is used in some form
struct tag_pair {};           ///< stl std::pair object is used in some form
struct tag_queue {};          ///< stl std::queue object is used in some form
struct tag_set {};            ///< stl std::set object is used in some form
struct tag_stack {};          ///< stl std::stack object is used in some form
struct tag_string {};         ///< stl std::string object is used in some form
struct tag_tuple {};          ///< stl std::tuple object is used in some form
struct tag_vector {};         ///< stl std::vector object is used in some form

struct tag_recursive {};      ///< stl std::recursive object is used in some form

// ----------------------------------------------------------------------------
// ## format related tag dispatchers
struct tag_ascii {};          ///< data is in ascii format
struct tag_base32 {};         ///< data is in base32 format
struct tag_base64 {};         ///< data is in base64 format
struct tag_csv {};            ///< data is in csv format
struct tag_hex {};            ///< data is in hex format
struct tag_json {};           ///< data is in json format
struct tag_path {};           ///< data is in path format
struct tag_uri {};            ///< data is in uri format (url encoded)
struct tag_url {};            ///< data is in url format
struct tag_utf8 {};           ///< data is in utf8 format
struct tag_xml {};            ///< data is in xml format

// ----------------------------------------------------------------------------
// ## operation related tag dispatchers
struct tag_raw {};            ///< do not perform any smart operations, just raw data


// ----------------------------------------------------------------------------
// ## C++ related tag dispatchers, type of c++ development logic

/// methods used to ask compiler for information
struct tag_ask_compiler {};

/// tag dispatcher used to avoid default implicit construction
struct tag_construct {};

/// methods used for some sort of debug related logic
struct tag_debug {};

/// for internal use only
struct tag_internal {};

/// operations using owner objects, objects that own the data
struct tag_owner {};

/// operations using view objects, objects that do not own the data, just views data stored in other objects
struct tag_view {};

/// argument related operations, could be things like passing arguments or something else
struct tag_argument {};

/// declare pointer to character main type as global 
extern const uint8_t puCharType_g[0x100];

/// declare pointer to character types as global 
extern const uint16_t puCharGroup_g[0x100];

// declare global methods

/// Get the constant number for character type name, names like "space", "digit" etc
uint8_t ctype_g(const std::string_view& stringCType, tag_main_type );
/// Get the constant number for character type name, names like "space", "digit" etc. This can return for more names compared version that return uint8_t
uint16_t ctype_g(const std::string_view& stringCType);
/// detect if text is integer, decimal or text
unsigned detect_ctypegroup_g( const uint8_t* puText, unsigned uLength );
inline unsigned detect_ctypegroup_g( const std::string_view& stringText ) { return detect_ctypegroup_g( (const uint8_t*)stringText.data(), stringText.length() ); }


/*-----------------------------------------*/ /**
   * \brief type numbers for common data types
   *
   * Numbers for common types used i gd code.
   * Using these numbers as a form of meta data in code, it will simply
   * to make code more compatible. Simplify to move data between objects.
   */
enum enumTypeNumber
{
   // ## primitive types
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

   // ## derived types
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
   eTypeNumberNumeric      = 35,
   eTypeNumberDecimal      = 36,
   eTypeNumberUuidString   = 37,

   eTypeNumberMAX          = 37,
};

/*-----------------------------------------*/ /**
 * \brief Flags for common type groups for value types
 * 
 * Group type values to make it easer to build logic around them.
 */
enum enumTypeGroup
{
   eTypeGroupNumber        = 0x0000'0100,                                      // type = number value
   eTypeGroupInteger       = 0x0000'0200,                                      // type = integer value
   eTypeGroupDecimal       = 0x0000'0400,                                      // type = decimal value
   eTypeGroupSigned        = 0x0000'0800,                                      // type = signed number value
   eTypeGroupString        = 0x0000'1000,                                      // type = string type
   eTypeGroupDate          = 0x0000'2000,                                      // type = date type
   eTypeGroupBinary        = 0x0000'4000,                                      // type = binary data
   eTypeGroupBoolean       = 0x0000'8000,                                      // type = boolean

   eTypeGroupSize08        = 0x0001'0000,                                      // size = 8 bit
   eTypeGroupSize16        = 0x0002'0000,                                      // size = 16 bit
   eTypeGroupSize32        = 0x0004'0000,                                      // size = 32 bit
   eTypeGroupSize64        = 0x0008'0000,                                      // size = 64 bit
   eTypeGroupSize128       = 0x0010'0000,                                      // size = 128 bit
   eTypeGroupSize256       = 0x0020'0000,                                      // size = 256 bit
   eTypeGroupSize512       = 0x0040'0000,                                      // size = 512 bit
};

enum enumTypeDetail
{
   eTypeDetailReference    = 0x0100'0000,                                      // reference to value
};

/*-----------------------------------------*/ /**
   * \brief Combined information for type
   *
   *
   */
enum enumType
{
   eTypeUnknown      = eTypeNumberUnknown,
   eTypeBool         = eTypeNumberBool       | eTypeGroupBoolean  | eTypeGroupSize08,
   eTypeInt8         = eTypeNumberInt8       | eTypeGroupInteger  | eTypeGroupSize08   | eTypeGroupSigned,
   eTypeInt16        = eTypeNumberInt16      | eTypeGroupInteger  | eTypeGroupSize16   | eTypeGroupSigned,
   eTypeInt32        = eTypeNumberInt32      | eTypeGroupInteger  | eTypeGroupSize32   | eTypeGroupSigned,
   eTypeInt64        = eTypeNumberInt64      | eTypeGroupInteger  | eTypeGroupSize64   | eTypeGroupSigned,

   eTypeInt128       = eTypeNumberInt128     | eTypeGroupInteger  | eTypeGroupSize128,
   eTypeInt256       = eTypeNumberInt256     | eTypeGroupInteger  | eTypeGroupSize256,
   eTypeInt512       = eTypeNumberInt512     | eTypeGroupInteger  | eTypeGroupSize512,

   eTypeUInt8        = eTypeNumberUInt8      | eTypeGroupInteger  | eTypeGroupSize08,
   eTypeUInt16       = eTypeNumberUInt16     | eTypeGroupInteger  | eTypeGroupSize16,
   eTypeUInt32       = eTypeNumberUInt32     | eTypeGroupInteger  | eTypeGroupSize32,
   eTypeUInt64       = eTypeNumberUInt64     | eTypeGroupInteger  | eTypeGroupSize64,

   eTypeUInt128      = eTypeNumberUInt128    | eTypeGroupInteger  | eTypeGroupSize128,
   eTypeUInt256      = eTypeNumberUInt256    | eTypeGroupInteger  | eTypeGroupSize256,
   eTypeUInt512      = eTypeNumberUInt512    | eTypeGroupInteger  | eTypeGroupSize512,

   eTypeCFloat       = eTypeNumberFloat      | eTypeGroupDecimal  | eTypeGroupSize32,
   eTypeCDouble      = eTypeNumberDouble     | eTypeGroupDecimal  | eTypeGroupSize64,
   eTypePointer      = eTypeNumberPointer,
   eTypeGuid         = eTypeNumberGuid       | eTypeGroupBinary   | eTypeGroupSize128,
   eTypeBinary       = eTypeNumberBinary     | eTypeGroupBinary,
   eTypeString       = eTypeNumberString     | eTypeGroupString,
   eTypeUtf8String   = eTypeNumberUtf8String | eTypeGroupString,
   eTypeWString      = eTypeNumberWString    | eTypeGroupString,
   eTypeUtf32String  = eTypeNumberUtf32String | eTypeGroupString,
   eTypeJson         = eTypeNumberJson       | eTypeGroupString,
   eTypeXml          = eTypeNumberXml        | eTypeGroupString,
   eTypeVoid         = eTypeNumberVoid,
   eTypeBit          = eTypeNumberBit        | eTypeGroupBoolean,

   eTypeRBinary      = eTypeNumberBinary     | eTypeGroupBinary  | eTypeDetailReference,
   eTypeRString      = eTypeNumberString     | eTypeGroupString  | eTypeDetailReference,
   eTypeRUtf8String  = eTypeNumberUtf8String | eTypeGroupString  | eTypeDetailReference,
   eTypeRWString     = eTypeNumberUtf8String | eTypeGroupString  | eTypeDetailReference

};

namespace detail {
   /// get the type number part
   constexpr uint32_t type_number_g( unsigned uType ) { return (uType & 0x000000ff); }
   /// get the type group part
   constexpr uint32_t type_group_g( unsigned uType ) { return (uType & 0x0000ff00); }
   /// get the type size part
   constexpr uint32_t type_size_g( unsigned uType ) { return (uType & 0x00ff0000); }

   /// helper method used to convert one or two character string into 32 bit value
   constexpr uint32_t hash_type16( std::string_view stringType )
   {                                                                                               assert( stringType[0] != 0 );
      uint32_t uHashValue = (uint32_t)stringType[0];
      uHashValue += (uint32_t)stringType[1] << 8;

      return uHashValue;
   }

   /// helper method used to convert first four characters into 32 bit unsigned integer value
   constexpr uint32_t hash_type( std::string_view stringType )
   {
      uint32_t uHashValue = (uint32_t)stringType[0];
      uHashValue += (uint32_t)stringType[1] << 8;
      uHashValue += (uint32_t)stringType[2] << 16;
      uHashValue += (uint32_t)stringType[3] << 24;

      return uHashValue;
   }

   /// helper method used to convert first four characters into 64 bit value
   constexpr uint64_t hash_type64( std::string_view stringType )
   {
      uint64_t uHashValue = (uint32_t)stringType[0];
      uHashValue += (uint64_t)stringType[1] << 8;
      if( stringType[2] != 0 ) { uHashValue += (uint64_t)stringType[2] << 16; }
      else return uHashValue;
      if( stringType[3] != 0 ) { uHashValue += (uint64_t)stringType[3] << 24; }
      else return uHashValue;
      if( stringType[4] != 0 ) { uHashValue += (uint64_t)stringType[4] << 32; }
      else return uHashValue;
      if( stringType[5] != 0 ) { uHashValue += (uint64_t)stringType[5] << 40; }
      else return uHashValue;
      if( stringType[6] != 0 ) { uHashValue += (uint64_t)stringType[6] << 48; }
      else return uHashValue;
      if( stringType[7] != 0 ) { uHashValue += (uint64_t)stringType[7] << 56; }
      else return uHashValue;

      return uHashValue;
   }

   constexpr bool is_boolean( uint32_t uType )   { return (uType & eTypeGroupBoolean) == eTypeGroupBoolean; }
   constexpr bool is_number( uint32_t uType )    { return (uType & eTypeGroupNumber) == eTypeGroupNumber; }
   constexpr bool is_integer( uint32_t uType )   { return (uType & eTypeGroupInteger) == eTypeGroupInteger; }
   constexpr bool is_decimal( uint32_t uType )   { return (uType & eTypeGroupDecimal) == eTypeGroupDecimal; }
   constexpr bool is_date( uint32_t uType )      { return (uType & eTypeGroupDate) == eTypeGroupDate; }
   constexpr bool is_string( uint32_t uType )    { return (uType & eTypeGroupString) == eTypeGroupString; }
   constexpr bool is_binary( uint32_t uType )    { return (uType & eTypeGroupBinary) == eTypeGroupBinary; }

}


/** ---------------------------------------------------------------------------
 * @brief Is type number a c primitive value
 * @param eTypeNumber type to check for primitive
 * @return true if primitive, false if not
*/
constexpr bool is_primitive_g( enumTypeNumber eTypeNumber )
{
   return static_cast<unsigned>( eTypeNumber ) > static_cast<unsigned>( eTypeUnknown ) && static_cast<unsigned>( eTypeNumber ) <= static_cast<unsigned>( eTypeNumberDouble );
}

/// wrapper, cast to `enumTypeNumber`
constexpr bool is_primitive_g( unsigned uTypeNumber ) { return is_primitive_g( static_cast<enumTypeNumber>( uTypeNumber & 0x000000ff ) ); }

/// Check if reference flag is set, what this is used for depends on object but may be that this item is stored as a reference (pointer)
constexpr bool is_reference_g( unsigned uType ) { return uType & eTypeDetailReference; }

/** ---------------------------------------------------------------------------
 * @brief extract group type from complete type
 * @param uType complete type
 * @return unsigned group part from complete value type
*/
constexpr unsigned value_group_type_g( unsigned uType ) { return (uType & 0x0000ff00); }


constexpr unsigned value_size_g(enumTypeNumber eTypeNumber)
{
   switch( eTypeNumber )
   {
   case eTypeNumberUnknown : return 0;    
   case eTypeNumberBool : return sizeof(uint8_t);
   case eTypeNumberInt8 : return sizeof(int8_t);       
   case eTypeNumberUInt8 : return sizeof(uint8_t);
   case eTypeNumberInt16 : return sizeof(int16_t);
   case eTypeNumberUInt16 : return sizeof(uint16_t);
   case eTypeNumberInt32 : return sizeof(int32_t);
   case eTypeNumberUInt32 : return sizeof(uint32_t);
   case eTypeNumberInt64 : return sizeof(int64_t);
   case eTypeNumberUInt64 : return sizeof(uint64_t);
   case eTypeNumberFloat : return sizeof(float);
   case eTypeNumberDouble : return sizeof(double);
   case eTypeNumberPointer : return sizeof(void*);
   case eTypeNumberGuid : return  16;
   case eTypeNumberString : return 0;     
   case eTypeNumberUtf8String : return 0; 
   case eTypeNumberWString : return 0;    
   case eTypeNumberUtf32String : return 0;
   case eTypeNumberBinary : return 0;     
   case eTypeNumberJson : return 0;       
   case eTypeNumberXml : return 0;        
   case eTypeNumberVoid : return 0;       
   case eTypeNumberBit : return sizeof(uint8_t);        
   case eTypeNumberInt128 : return sizeof(uint64_t) + sizeof(uint64_t);
   case eTypeNumberUInt128 : return sizeof(uint64_t) + sizeof(uint64_t);
   case eTypeNumberHex : return 0;        
   case eTypeNumberBase32 : return 0;     
   case eTypeNumberDateTime : return sizeof(double);
   case eTypeNumberDate : return sizeof(double);
   case eTypeNumberTime : return sizeof(double);
   case eTypeNumberNumeric : return 0;    
   case eTypeNumberDecimal : return 0;    
   case eTypeNumberUuidString : return 0; 
   default: return 0;
   }
}

/** ---------------------------------------------------------------------------
 * @brief get size for type number
 * Size is the needed size in memory to store values of the type
 * @param uTypeNumber type value size is returned for
 * @return unsigned size in bytes for type
*/
constexpr unsigned value_size_g( unsigned uTypeNumber )
{
   return value_size_g( static_cast<enumTypeNumber>(uTypeNumber & 0x00000000ff) );
}

/** ---------------------------------------------------------------------------
 * @brief Return column memory size needed to store data for type number
 * This method returns known size for specific types in bytes. Note that string types adds size for
 * zero termination.
 * @param uTypeNumber type number size is returned for
 * @param uCount number of values of type to calculate size for
 * @return size for type
*/
constexpr unsigned value_size_g(unsigned uTypeNumber, unsigned uCount)
{
   switch( uTypeNumber & 0x00000000ff )
   {
   case eTypeNumberUnknown : return 0;    
   case eTypeNumberBool : return sizeof(uint8_t) * uCount;
   case eTypeNumberInt8 : return sizeof(int8_t) * uCount;       
   case eTypeNumberUInt8 : return sizeof(uint8_t) * uCount;
   case eTypeNumberInt16 : return sizeof(int16_t) * uCount;
   case eTypeNumberUInt16 : return sizeof(uint16_t) * uCount;
   case eTypeNumberInt32 : return sizeof(int32_t) * uCount;
   case eTypeNumberUInt32 : return sizeof(uint32_t) * uCount;
   case eTypeNumberInt64 : return sizeof(int64_t) * uCount;
   case eTypeNumberUInt64 : return sizeof(uint64_t) * uCount;
   case eTypeNumberFloat : return sizeof(float) * uCount;
   case eTypeNumberDouble : return sizeof(double) * uCount;
   case eTypeNumberPointer : return sizeof(void*) * uCount;
   case eTypeNumberGuid : return 16 * uCount;
   case eTypeNumberString : return (sizeof(int8_t) * uCount) + sizeof(int8_t);
   case eTypeNumberUtf8String : return (sizeof(uint8_t) * uCount) + sizeof(int8_t); 
   case eTypeNumberWString : return (sizeof(uint16_t) * uCount) + sizeof(uint16_t);
   case eTypeNumberUtf32String : return (sizeof(uint32_t) * uCount) + sizeof(uint32_t);
   case eTypeNumberBinary : return sizeof(uint8_t) * uCount;     
   case eTypeNumberJson : return sizeof(uint8_t) * uCount;
   case eTypeNumberXml : return sizeof(uint8_t) * uCount;
   case eTypeNumberVoid : return 0;       
   case eTypeNumberBit : return sizeof(uint8_t) * uCount;        
   case eTypeNumberUInt128 : return (sizeof(uint64_t) + sizeof(uint64_t)) * uCount;
   case eTypeNumberDecimal : return 0; 
   default: return 0;
   }
}


/** ---------------------------------------------------------------------------
 * @brief Checks if type number is a valid type
 * The lower byte in  number is used to set number type
 * @param uTypeNumber type to check
 * @return true if type is ok, false if not
*/
constexpr bool validate_number_type_g( unsigned uTypeNumber )
{
   unsigned uCheckType = uTypeNumber & 0x000000ff;
   return uCheckType < eTypeNumberMAX;
}


/** ---------------------------------------------------------------------------
 * @brief Convert type name from string to constant type value
 @code
enumType eType = type_g("int32");   assert( eType == eTypeInt32 );
eType = type_g("int8");             assert( eType == eTypeInt8 );
 @endcode
 * @param stringType type sent as string
 * @return {enumType} type constant
*/
constexpr enumType type_g( const std::string_view& stringType )
{                                                                                                  assert( stringType.length() >= 3 );
   using namespace detail;

   enumType eType = eTypeUnknown;

   uint32_t uTypeName = hash_type( stringType ); 

   switch( uTypeName )
   {
   case hash_type("unkn"): eType = eTypeUnknown;  break;                       // unknown (0)
   case hash_type("null"): eType = eTypeUnknown;  break;                       // null, like unknown (0)

   case hash_type("bina"): eType = eTypeBinary;  break;                        // binary
   case hash_type("bool"): eType = eTypeBool;  break;                          // bool
   case hash_type("doub"): eType = eTypeCDouble;  break;                       // double
   case hash_type("floa"): eType = eTypeCFloat;  break;                        // float

   case hash_type("i128"): eType = eTypeInt128;  break;                        // int128
   case hash_type("i256"): eType = eTypeInt256;  break;                        // int254
   case hash_type("i512"): eType = eTypeInt512;  break;                        // int512

   case hash_type("int8"): eType = eTypeInt8;  break;                          // int8
   case hash_type("int1"): eType = eTypeInt16;  break;                         // int16
   case hash_type("int3"): eType = eTypeInt32;  break;                         // int32
   case hash_type("int6"): eType = eTypeInt64;  break;                         // int64
   case hash_type("poin"): eType = eTypePointer;  break;                       // pointer

   case hash_type("rbin"): eType = eTypeRBinary;  break;                       // rbinary (binary reference)
   case hash_type("rstr"): eType = eTypeRString;  break;                       // rstring (string reference)
   case hash_type("rutf"): eType = eTypeRUtf8String;  break;                   // rutf8 (utf8 reference)

   case hash_type("stri"): eType = eTypeString;  break;

   case hash_type("u128"): eType = eTypeUInt128;  break;                       // uint128
   case hash_type("u256"): eType = eTypeUInt256;  break;                       // uint256
   case hash_type("u512"): eType = eTypeUInt512;  break;                       // uint512

   case hash_type("uint"):                                                     // uint8, uint16, uint32, uint64
      {
         if( stringType[4] == '8' ) eType = eTypeUInt8;
         else if( stringType[4] == '1' ) eType = eTypeUInt16;
         else if( stringType[4] == '3' ) eType = eTypeUInt32;
         else if( stringType[4] == '6' ) eType = eTypeUInt64;
         else { static_assert("invalid type name"); assert( false ); }
      }
      break;
   case hash_type("uuid"): eType = eTypeGuid;  break;                          // uuid
   case hash_type("utf8"): eType = eTypeUtf8String;  break;                    // utf8
   case hash_type("wstr"): eType = eTypeWString;  break;                       // wstring
   case hash_type("utf3"): eType = eTypeUtf32String;  break;                   // uft32
   default: assert(false);
   }

   return eType;
}

template<typename TYPE>
constexpr enumType type_g( TYPE, tag_ask_compiler )
{
   if      constexpr( std::is_same<bool, TYPE>::value ) return eTypeBool;
   else if constexpr( std::is_same<int8_t, TYPE>::value ) return eTypeInt8;
   else if constexpr( std::is_same<uint8_t, TYPE>::value ) return eTypeUInt8;
   else if constexpr( std::is_same<int16_t, TYPE>::value ) return eTypeInt16;
   else if constexpr( std::is_same<uint16_t, TYPE>::value ) return eTypeUInt16;
   else if constexpr( std::is_same<int32_t, TYPE>::value ) return eTypeInt32;
   else if constexpr( std::is_same<uint32_t, TYPE>::value ) return eTypeUInt32;
   else if constexpr( std::is_same<int64_t, TYPE>::value ) return eTypeInt64;
   else if constexpr( std::is_same<uint64_t, TYPE>::value ) return eTypeUInt64;
   else if constexpr( std::is_same<float, TYPE>::value ) return eTypeCFloat;
   else if constexpr( std::is_same<double, TYPE>::value ) return eTypeCDouble;
   else if constexpr( std::is_same<void*, TYPE>::value ) return eTypePointer;
   else if constexpr( std::is_same<char*, TYPE>::value ) return eTypeString;
   else if constexpr( std::is_same<std::string_view, TYPE>::value ) return eTypeString;
   else if constexpr( std::is_same<std::string, TYPE>::value ) return eTypeString;
   else if constexpr( std::is_same<std::wstring_view, TYPE>::value ) return eTypeWString;
   else if constexpr( std::is_same<std::wstring, TYPE>::value ) return eTypeWString;
   //else static_assert( false, "Invalid type");
}

template<typename TYPE>
constexpr enumType type_g( tag_ask_compiler )
{
   if      constexpr( std::is_same<bool, TYPE>::value ) return eTypeBool;
   else if constexpr( std::is_same<int8_t, TYPE>::value ) return eTypeInt8;
   else if constexpr( std::is_same<uint8_t, TYPE>::value ) return eTypeUInt8;
   else if constexpr( std::is_same<int16_t, TYPE>::value ) return eTypeInt16;
   else if constexpr( std::is_same<uint16_t, TYPE>::value ) return eTypeUInt16;
   else if constexpr( std::is_same<int32_t, TYPE>::value ) return eTypeInt32;
   else if constexpr( std::is_same<uint32_t, TYPE>::value ) return eTypeUInt32;
   else if constexpr( std::is_same<int64_t, TYPE>::value ) return eTypeInt64;
   else if constexpr( std::is_same<uint64_t, TYPE>::value ) return eTypeUInt64;
   else if constexpr( std::is_same<float, TYPE>::value ) return eTypeCFloat;
   else if constexpr( std::is_same<double, TYPE>::value ) return eTypeCDouble;
   else if constexpr( std::is_same<void*, TYPE>::value ) return eTypePointer;
   else if constexpr( std::is_same<char*, TYPE>::value ) return eTypeString;
   else if constexpr( std::is_same<std::string_view, TYPE>::value ) return eTypeString;
   else if constexpr( std::is_same<std::string, TYPE>::value ) return eTypeString;
   else if constexpr( std::is_same<std::wstring_view, TYPE>::value ) return eTypeWString;
   else if constexpr( std::is_same<std::wstring, TYPE>::value ) return eTypeWString;
   //else static_assert( false, "Invalid type");
}

/**
 * \brief Used to help write compiler code where type gets a name
 *
 * With this class the string literal `_ctype` works and cname can be used to
 * pase a type name in other classes
 */
struct cname
{
// ## construction ------------------------------------------------------------
   cname() {}
   cname( const std::string_view& stringCName, tag_construct ): m_stringCName(stringCName) {}
// copy
   cname(const cname& o) { common_construct(o); }
// assign
   cname& operator=(const cname& o) { common_construct(o); return *this; }

// common copy
   void common_construct(const cname& o) { m_stringCName = o.m_stringCName; }

// ## methods -----------------------------------------------------------------

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   std::string_view m_stringCName;

};

inline cname operator ""_ctype(const char* pbszCType, size_t uSize) {
   return cname( std::string_view{ pbszCType, uSize }, tag_construct{});
}

// ## type characters

constexpr uint8_t CHAR_TYPE_SPACE           = 01;
constexpr uint8_t CHAR_TYPE_DIGIT           = 02;
constexpr uint8_t CHAR_TYPE_ALPHABET        = 03;
constexpr uint8_t CHAR_TYPE_OPERATOR        = 04;
constexpr uint8_t CHAR_TYPE_BRACKET         = 05;
constexpr uint8_t CHAR_TYPE_PUNCTUATOR      = 06;
constexpr uint8_t CHAR_TYPE_QUOTE           = 07;

/// Return type bit for named type
constexpr uint8_t cchartype_g(const std::string_view& stringCType, tag_main_type)
{
   if( stringCType == "space" )              return CHAR_TYPE_SPACE;
   else if( stringCType == "digit" )         return CHAR_TYPE_DIGIT;
   else if( stringCType == "alphabet" )      return CHAR_TYPE_ALPHABET;
   else if( stringCType == "alpha" )         return CHAR_TYPE_ALPHABET;
   else if( stringCType == "bracket" )       return CHAR_TYPE_BRACKET;
   else if( stringCType == "operator" )      return CHAR_TYPE_OPERATOR;
   else if( stringCType == "punctuator" )    return CHAR_TYPE_PUNCTUATOR;
   else if( stringCType == "quote" )         return CHAR_TYPE_QUOTE;

   return 0;
}



// ## group characters

constexpr uint16_t CHAR_GROUP_SPACE          = 0b0000'0000'0000'0001;          /// 0x0001    1     Space characters
constexpr uint16_t CHAR_GROUP_DIGIT          = 0b0000'0000'0000'0010;          /// 0x0002    2     digits (only numbers)
constexpr uint16_t CHAR_GROUP_ALPHABET       = 0b0000'0000'0000'0100;          /// 0x0004    4     Alphabet characters A-Z and a-z
constexpr uint16_t CHAR_GROUP_OPERATOR       = 0b0000'0000'0000'1000;          /// 0x0008    8     Operators like + - * /
constexpr uint16_t CHAR_GROUP_QUOTE          = 0b0000'0000'0001'0000;          /// 0x0010    16    Quote characters are ' " `
constexpr uint16_t CHAR_GROUP_DECIMAL        = 0b0000'0000'0010'0000;          /// 0x0020    32    Decimal character are .
constexpr uint16_t CHAR_GROUP_HEX            = 0b0000'0000'0100'0000;          /// 0x0040    64    Hexadecimal characters like 0-9, a-f and A-F
constexpr uint16_t CHAR_GROUP_SCIENTIFIC     = 0b0000'0000'1000'0000;          /// 0x0080    128   Scientific characters are things . E for exponent and space that is alowed
constexpr uint16_t CHAR_GROUP_PUNCTUATION    = 0b0000'0001'0000'0000;          /// 0x0100    256   Characters like . ? : , ; !
constexpr uint16_t CHAR_GROUP_BRACKET        = 0b0000'0001'0000'0000;          /// 0x0200    512   Brackets are ( ) [ ] { }
constexpr uint16_t CHAR_GROUP_ALNUM          = 0b0000'0010'0000'0000;          /// 0x0400    1024  Digit, alphabet charatcter or underscore _
constexpr uint16_t CHAR_GROUP_XML            = 0b0000'0100'0000'0000;          /// 0x0800    2048  XML specific charcters are < > & ! ?
constexpr uint16_t CHAR_GROUP_FILE           = 0b0000'1000'0000'0000;          /// 0x1000    4096  Characters specific to file names


/// Return type bit for named type
/// Use the string literal `_ctype` to check character group
constexpr uint16_t ctype_g(const std::string_view& stringCType , tag_ask_compiler)
{
        if( stringCType == "space" )         return CHAR_GROUP_SPACE;
   else if( stringCType == "digit" )         return CHAR_GROUP_DIGIT;
   else if( stringCType == "alphabet" )      return CHAR_GROUP_ALPHABET;
   else if( stringCType == "alpha" )         return CHAR_GROUP_ALPHABET;
   else if( stringCType == "operator" )      return CHAR_GROUP_OPERATOR;
   else if( stringCType == "quote" )         return CHAR_GROUP_QUOTE;
   else if( stringCType == "decimal" )       return CHAR_GROUP_DECIMAL;
   else if( stringCType == "hex" )           return CHAR_GROUP_HEX;
   else if( stringCType == "scientific" )    return CHAR_GROUP_SCIENTIFIC;
   else if( stringCType == "punctuation" )   return CHAR_GROUP_PUNCTUATION;
   else if( stringCType == "bracket" )       return CHAR_GROUP_BRACKET;
   else if( stringCType == "alnum" )         return CHAR_GROUP_ALNUM;
   else if( stringCType == "xml" )           return CHAR_GROUP_XML;
   else if( stringCType == "file" )          return CHAR_GROUP_FILE;

   return 0;
}

inline uint8_t ctype_g(uint8_t uCharacter, tag_main_type ) { return puCharType_g[uCharacter]; }

constexpr uint16_t ctype_g(const cname& cname) { return ctype_g( cname.m_stringCName, tag_ask_compiler{}); }

/// check if character matches group number 
inline bool is_ctype_g(char ch, uint16_t uCType) {
   return (puCharGroup_g[uint8_t(ch)] & uCType) == uCType;
}


/// @brief check if character belong to a specific character group
/// Very fast to check what type of character it is
/// Sample: `if(gd::types::is_ctype_g('1', "digit"_ctype) == true)`
inline bool is_ctype_g(char ch, const cname& cname_) {
   auto uCType = ctype_g( cname_ );
   return (puCharGroup_g[uint8_t(ch)] & uCType ) == uCType;
}

/// Get ctype flags for character code
inline uint16_t get_ctype_g( uint8_t u ) { return puCharGroup_g[u]; }


/** ---------------------------------------------------------------------------
 * @brief return name for type
 * Type is found in first byte where the gd type number logic is used
 * 
 * @code
// Sample to show how to get type name from table called table_column_buffer
gd::table::table_column_buffer tableIterate( 100 );
tableIterate.column_add( { { "int64", 0, "c1"}, { "int32", 0, "c2"}, { "int16", 0, "c3"} }, gd::table::tag_type_name{} );
tableIterate.prepare();

auto columns_ = tableIterate.columns();
for( auto& it : columns_ ) { std::cout << gd::types::type_name_g( it.ctype() ); }
 * @endcode
 * 
 * @param uType type value type name is returned for
 * @return std::string_view name for type
*/
constexpr std::string_view type_name_g(uint32_t uType)
{
   switch( uType & 0x000000ff )
   {
   case eTypeNumberUnknown: return "unknown";
   case eTypeNumberBool: return "boolean";
   case eTypeNumberInt8: return "int8";
   case eTypeNumberInt16: return "int16";
   case eTypeNumberInt32: return "int32";
   case eTypeNumberInt64: return "int64";
   case eTypeNumberUInt8: return "uint8";
   case eTypeNumberUInt16: return "uint16";
   case eTypeNumberUInt32: return "uint32";
   case eTypeNumberUInt64: return "uint64";
   case eTypeNumberFloat: return "float";
   case eTypeNumberDouble: return "double";
   case eTypeNumberGuid:  return "guid";
   case eTypeNumberUtf8String: return "utf8";
   case eTypeNumberUtf32String: return "utf32";
   case eTypeNumberString: return "string";
   case eTypeNumberWString: return "wstring";
   case eTypeNumberBinary:  return "binary";
   case eTypeNumberJson: return "json";
   case eTypeNumberXml: return "xml";
   case eTypeNumberVoid: return "void";
   case eTypeNumberBit: return "bit";
   case eTypeNumberInt128: return "i128";
   case eTypeNumberUInt128: return "u128";
   case eTypeNumberInt256: return "i256";
   case eTypeNumberUInt256: return "u256";
   case eTypeNumberInt512: return "i512";
   case eTypeNumberUInt512: return "u512";
   case eTypeNumberHex: return "hex";
   case eTypeNumberBase32: return "base32";
   case eTypeNumberDateTime: return "datetime";
   case eTypeNumberDate: return "date";
   case eTypeNumberTime: return "time";
   case eTypeNumberNumeric: return "numeric";
   case eTypeNumberDecimal: return "decimal";
   case eTypeNumberUuidString: return "uuid";
      break;
   }

   return std::string_view();
}

constexpr uint32_t typenumber_to_type_g(uint32_t uType)
{
   switch( uType & 0x000000ff )
   {
   case eTypeNumberUnknown: return eTypeUnknown;
   case eTypeNumberBool: return eTypeBool;
   case eTypeNumberInt8: return eTypeInt8;
   case eTypeNumberInt16: return eTypeInt16;
   case eTypeNumberInt32: return eTypeInt32;
   case eTypeNumberInt64: return eTypeInt64;
   case eTypeNumberUInt8: return eTypeUInt8;
   case eTypeNumberUInt16: return eTypeUInt16;
   case eTypeNumberUInt32: return eTypeUInt32;
   case eTypeNumberUInt64: return eTypeUInt64;
   case eTypeNumberFloat: return eTypeCFloat;
   case eTypeNumberDouble: return eTypeCDouble;
   case eTypeNumberGuid:  return eTypeGuid;
   case eTypeNumberUtf8String: return eTypeUtf8String;
   case eTypeNumberUtf32String: return eTypeUtf32String;
   case eTypeNumberString: return eTypeString;
   case eTypeNumberWString: return eTypeWString;
   case eTypeNumberBinary:  return eTypeBinary;
      break;
   default: assert(false);
   }

   return eTypeUnknown;
}


/** ---------------------------------------------------------------------------
 * @brief Convert type names to vector with type numbers
 * @param listName list with names that is converted to type numbers
 * @return std::vector<unsigned> vector with type numbers for type names
*/
inline std::vector<unsigned> types_g( std::initializer_list< const char*> listName )
{
   std::vector<unsigned> vectorType;
   for( auto it : listName )
   {
      vectorType.push_back( type_g( it ) );
   }

   return vectorType;
}

/** ---------------------------------------------------------------------------
 * @brief Convert type names to vector with type numbers
 * @param listName list with names that is converted to type numbers
 * @return std::vector<unsigned> vector with type numbers for type names
*/
inline std::vector<unsigned> types_g( std::vector<std::string_view> vectorName )
{
   std::vector<unsigned> vectorType;
   for( auto& it : vectorName )
   {
      vectorType.push_back( type_g( it ) );
   }

   return vectorType;
}


enum enumAlign
{
   enumAlignLeft    = 0b0000'0000,                                             // left align (default)
   enumAlignRight   = 0b0000'0001,                                             // right align
   enumAlignCenter  = 0b0000'0010,                                             // center align
   enumAlignTop     = 0b0000'0000,                                             // top align (default)
   enumAlignBottom  = 0b0000'0100,                                             // bottom align
   enumAlignMiddle  = 0b0000'1000,                                             // middle align
};

/** ---------------------------------------------------------------------------
 * @brief return alignment for type
 * @param eTypeNumber type alignment is returned for
 * @return alignment flags for type @see: enumAlign
*/
constexpr unsigned align_g(enumTypeNumber eTypeNumber)
{
   switch( eTypeNumber )
   {
   case eTypeNumberUnknown : return 0;    

   case eTypeNumberBool : 
   case eTypeNumberInt8 : 
   case eTypeNumberUInt8 :
   case eTypeNumberInt16 : 
   case eTypeNumberUInt16 :
   case eTypeNumberInt32 : 
   case eTypeNumberUInt32 : 
   case eTypeNumberInt64 : 
   case eTypeNumberUInt64 : 
   case eTypeNumberFloat : 
   case eTypeNumberDouble : 
   case eTypeNumberPointer : 
   case eTypeNumberGuid : return (unsigned)enumAlignRight;

   case eTypeNumberString : 
   case eTypeNumberUtf8String : 
   case eTypeNumberWString : 
   case eTypeNumberUtf32String : 
   case eTypeNumberBinary : 
   case eTypeNumberJson : 
   case eTypeNumberXml : 
   case eTypeNumberVoid : return 0;       

   case eTypeNumberBit : 
   case eTypeNumberInt128 : 
   case eTypeNumberUInt128 : return (unsigned)enumAlignRight;
   case eTypeNumberHex : return 0;        
   case eTypeNumberBase32 : return 0;     
   case eTypeNumberDateTime : return (unsigned)enumAlignRight;
   case eTypeNumberDate : return (unsigned)enumAlignRight;
   case eTypeNumberTime : return (unsigned)enumAlignRight;
   case eTypeNumberNumeric : return (unsigned)enumAlignRight;
   case eTypeNumberDecimal : return (unsigned)enumAlignRight;
   case eTypeNumberUuidString : return (unsigned)enumAlignRight;
   default: return 0;
   }
}

/// How to align type when printed as text
inline unsigned align_g( unsigned uType ) { return align_g( enumTypeNumber(uType & 0xFF) ); }


// ## helpers for templates

// Default case: any type T is not considered a list, hence false
template <typename T>
struct is_list : std::false_type {};

// Specialization for std::list: this type is indeed a list, so true
template <typename T, typename Alloc>
struct is_list<std::list<T, Alloc>> : std::true_type {};

// Default case: any type T is not considered a vector, hence false
template <typename T>
struct is_vector : std::false_type {};

// Specialization for std::vector: this type is indeed a vector, so true
template <typename T, typename Alloc>
struct is_vector<std::vector<T, Alloc>> : std::true_type {};






#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif


_GD_TYPES_END