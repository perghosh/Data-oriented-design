// @FILE [tag: arguments] [description: arguments that packs data without alignment, optimized for size to use less memory and may can store data on stack] [type: header]

/**
 * @file gd_arguments.h
 *
 * @brief Pack primitive and common derived values into a byte buffer for efficient storage and access with focus on memory size. Like key-value pairs in one single buffer.
 *
 *
 | Area                | Methods (Examples)                                                                 | Description                                                                                   |
 |---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | arguments(...), common_construct(...), zero(), buffer_set(...)                     | Constructors for various ways to create, copy, and initialize arguments buffers.              |
 | Appending           | append(...), append_argument(...), append_if(...), append_object(...), operator+=  | Methods for adding values, named arguments, objects, and collections to the buffer.           |
 | Setting             | set(...), set_uuid(...), set_argument(...), operator=                              | Methods for updating or replacing values by name, index, or pointer.                          |
 | Insertion           | insert(...), reserve(...), reserve_no_copy(...)                                    | Methods for inserting values at specific positions and managing buffer capacity.              |
 | Removal             | remove(...), erase(...), shrink_to_fit()                                           | Methods for removing arguments by name, index, or pointer, and optimizing buffer size.        |
 | Retrieval           | get_argument(...), get_argument_all(...), find(...), exists(...), operator[]       | Methods for retrieving values by name, index, or pointer, including type conversion.          |
 | Iteration           | iterator, begin(), end(), next(...), find_all(...), get_argument_section(...)      | Methods for traversing arguments using iterators or pointer-based navigation.                 |
 | Comparison          | compare(...), compare_argument_s(...), compare_group(...), compare_exists(...)     | Methods for comparing argument values, types, and existence between buffers.                  |
 | Printing/Debug      | print(...), print_json(), debug::print(...), print_s(...), print_type_s(...)       | Methods for formatting and printing argument values and buffer contents for debugging.        |
 | Utility/Meta        | clear(), empty(), size(), capacity(), is_owner(), buffer_data(), type_name_s(...)  | Utility methods for clearing, checking, and querying buffer and argument metadata.            |
 *
 * The `arguments` class is designed to manage a collection of named or unnamed values stored in a contiguous byte buffer,
 * optimized for minimal memory usage and fast access. It supports a variety of data types, including integers, floating-point
 * numbers, strings, and binary data, with a flexible memory layout. Each argument is stored as a sequence of bytes
 * following the format: `[uint16 name length][name-data][uint8 type]{[uint32 data length]}[data]`. The class provides methods
 * for appending, retrieving, and manipulating arguments, along with iterators for traversal. It is particularly useful
 * for scenarios requiring compact serialization or key-value pair management.
 *
 * ### Key Features
 * - Compact memory layout with optional names for key-value pairs.
 * - Support for all primitive types (e.g., `int32_t`, `double`) and derived types (e.g., strings, binary blobs).
 * - Efficient append and retrieval operations with minimal overhead.
 * - Iterators for forward traversal of arguments.
 *
 * ### Example Usage
 * Iterate and print values in `arguments`:
 * \code
void print(const gd::argument::arguments& args) {
   for(auto pos = args.next(); pos != nullptr; pos = args.next(pos)) {
      auto name = gd::argument::arguments::get_name_s(pos);
      auto value = gd::argument::arguments::get_argument_s(pos).as_variant_view();
      std::cout << "Name: " << name << ", Value: " << value.as_string() << "\n";
   }
}
 * \endcode
 *
 * Set and get values:
 * \code
gd::argument::arguments args;
args.set("key1", 42);
args.set("key2", "value");
std::cout << "Key1: " << args["key1"].get<int>() << "\n";
std::cout << "Key2: " << gd::argument::get_g<std::string>( args["key2"] ) << "\n";
args.set("key3", 100.01);
std::cout << "Key3: " << args["key3"].as_string() << "\n";
args.append("key4", 200.02);
std::cout << "Key4: " << args["key4"].as_int() << "\n";
args.append( 1000 );
std::cout << "index 4 (zero based): " << args[4].as_double() << "\n";
 * \endcode
 *
 * Named iteration:
 * \code
gd::argument::arguments args8( {{ {"int-number", 100 }, {"float-number", 100.01}, {"string-value", "value"}, {"bool-value", true},
   {"uint64-number", (uint64_t)1000}, {"int64-number", (int64_t)-1000}, {"double-number", 200.02}, {"wstring-value", L"wstring"} } });

for( auto it = args8.named_begin(); it != args8.named_end(); it++ )
{
   std::cout << "Key: " << it.name() << ", Type: " << it.type_number() << ", Value: " << it.as_string() << "\n";
}
 * \endcode
 *
 * ### Memory Management
 * The class can either own its buffer (allocated on the heap) or use an externally provided buffer. If the buffer is owned,
 * it is automatically freed in the destructor.
 *
 */

/**
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0argument` - Represents a single argument in `arguments`.
 * - `0TAG0iterator` - Provides forward traversal of arguments in `arguments`.
 * - `0TAG0construct.arguments` - Constructors and destructors for `arguments`.
 * - `0TAG0operator.arguments` - Overloaded operators for `arguments`.
 * - `0TAG0append.arguments` - Methods for appending values to `arguments`.
 * - `0TAG0set.arguments` - Methods for setting values in the `arguments`.
 * - `0TAG0get.arguments` - Methods for retrieving values from `arguments`.
 * - `0TAG0print.arguments` - Methods for printing values in `arguments`.
 * - `0TAG0free_functions.arguments` - Free functions for working with `arguments`.
 * - `0TAG0buffer.arguments` - Methods for managing the buffer in `arguments`.
 */



#pragma once
#include <cassert>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_variant_arg.h"
#include "gd_arguments_common.h"


/*
| class | Description |
| - | - |
| argument | Manage values in `arguments` |
|   |   |

mdtable

// structured binding
// https://devblogs.microsoft.com/oldnewthing/20201015-00/?p=104369

*/




#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wdeprecated-enum-compare"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 26812 )
#endif

// @AI [tag: gd, arguments] [llm: core]

#ifndef _GD_ARGUMENT_BEGIN
#define _GD_ARGUMENT_BEGIN namespace gd::argument {
#define _GD_ARGUMENT_END }
_GD_ARGUMENT_BEGIN
#else
_GD_ARGUMENT_BEGIN
#endif

// Forward declaration before the arguments class
template<typename ARGUMENTS>
struct iterator_named;


// ================================================================================================
// ====================================================================================== arguments
// ================================================================================================

/**
 * \brief  arguments focus on memory size, to tricks to align and speed up memory access
 *
 * ## memory layout
 * [type and length for name][name in chars][type]{[length for non primitive types]}[value data]
 * shorter version
 * [uint16][name][uint8]{[uint32]}[data]
 *
 \code
 \endcode

*Iterate values in arguments object and print to console*
 \code
void print( const gd::argument::arguments& arguments_ )
{
   for( auto pPosition = arguments_.next(); pPosition != nullptr; pPosition = arguments_.next(pPosition) )
   {
      auto stringName = gd::argument::arguments::get_name_s( pPosition );
      auto value_ = gd::argument::arguments::get_argument_s( pPosition ).as_variant_view();

      std::cout << "Name: " << stringName << ", Value: " << value_.as_string() << "\n";
   }
}
 \endcode

 \code
 \endcode
 */
class arguments
{
// ## typedefs -----------------------------------------------------------------
public:
   struct argument;
   using iterator_category =  std::forward_iterator_tag;
   using value_type =         argument;
   using pointer =            uint8_t*;
   using const_pointer =      const uint8_t*;
   using reference =          value_type&;
   using const_reference =    const value_type&;
   using size_type =          size_t ;
   using difference_type =    std::ptrdiff_t;

   typedef uint8_t            param_type;
   typedef uint8_t            argument_type;

   using tag_pair          = gd::types::tag_pair;                              // tag dispatcher used to select working with pair items
   using tag_view          = gd::types::tag_view;                              // used when working with view objects (not owning its data)
   using tag_string_view   = gd::types::tag_string_view;                       // string view related operations
   using tag_argument      = gd::types::tag_argument;                          // argument related operations
   using tag_name          = gd::types::tag_name;                              // there is some name related logic involved
   using tag_description   = gd::types::tag_description;                       // tag dispatcher where description is useful
   struct tag_no_initializer_list {};                                          // do not select initializer_list versions
   struct tag_internal {};                                                     // tag dispatcher for internal use

   using named_iterator_t = iterator_named<arguments>;
   using const_named_iterator = iterator_named<const arguments>;


public:

   /**
    * \brief group type for values in argument
    *
    */
   enum enumGroup
   {
      eGroupBoolean     = 0x01000000,  // boolean type
      eGroupInteger     = 0x02000000,  // integer types, both unsigned and signed
      eGroupDecimal     = 0x04000000,  // decimal values
      eGroupString      = 0x08000000,  // text values, both ascii, unicode, utf-32
      eGroupBinary      = 0x10000000,  // binary values
   };


   /**
    * \brief type values used for each argument in arguments class
    *
    */
   enum enumCType
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

      eTypeNumberPointer      = 12,  // pointer to memory

      eTypeNumberGuid         = 13, // universal unique identifier

      // ## derived types
      eTypeNumberString       = 14, // ascii string
      eTypeNumberUtf8String   = 15, // utf8 string
      eTypeNumberWString      = 16, // unicode string
      eTypeNumberUtf32String  = 17, // utf32 string

      eTypeNumberBinary       = 18, // binary data

      CType_MAX,


      eType_ParameterName,         // special type for parameter names

      eValueName = 0b00100000,
      eValueLength = 0b01000000,
      eValueArray = 0b10000000,

      eType_MASK = 0b11100000,      // mask for name, length and array markers in byte
      eCType_MASK = 0xffffff00,     // mask to extract byte from full 32 bit number
      eTypeNumber_MASK = (0xffffff00 + eType_MASK), // mask to extract type value
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
      eTypeInt8         = eTypeNumberInt8     | eGroupInteger,
      eTypeInt16        = eTypeNumberInt16    | eGroupInteger,
      eTypeInt32        = eTypeNumberInt32    | eGroupInteger,
      eTypeInt64        = eTypeNumberInt64    | eGroupInteger,
      eTypeUInt8        = eTypeNumberUInt8    | eGroupInteger,
      eTypeUInt16       = eTypeNumberUInt16   | eGroupInteger,
      eTypeUInt32       = eTypeNumberUInt32   | eGroupInteger,
      eTypeUInt64       = eTypeNumberUInt64   | eGroupInteger,
      eTypeFloat        = eTypeNumberFloat    | eGroupDecimal,
      eTypeDouble       = eTypeNumberDouble   | eGroupDecimal,
      eTypePointer      = eTypeNumberPointer,
      eTypeGuid         = eTypeNumberGuid     | eGroupBinary,
      eTypeBinary       = eTypeNumberBinary   | eGroupBinary,
      eTypeString       = eTypeNumberString   | eGroupString,
      eTypeUtf8String   = eTypeNumberUtf8String | eGroupString,
      eTypeWString      = eTypeNumberWString  | eGroupString,
      eTypeUtf32String  = eTypeNumberUtf32String | eGroupString,
   };


                                                                                 static_assert((int)eTypeNumberUInt64 == (int)variant_type::eTypeNumberUInt64); static_assert((int)eTypeNumberDouble == (int)variant_type::eTypeNumberDouble); static_assert((int)eTypeNumberBinary == (int)variant_type::eTypeNumberBinary);
                                                                                 static_assert( (CType_MAX & eType_MASK) == 0 );

   static const unsigned ARGUMENTS_NO_LENGTH = eTypeNumberGuid;

   /**
    * \brief member type for complete value in arguments list
    *
    */
   enum enumPairType
   {
      ePairTypeKey   = (1<<0),        // Key (name for value) in argument list
      ePairTypeValue = (1<<1),        // Value in argument list
      ePairTypeAll   = ePairTypeKey | ePairTypeValue,
   };

   /**
    * Base64 flags used to specify how base64 formated text is formated
    */
   enum enumBase64
   {
      eBase64NoCrLf = 0x01,  ///< don't and carriage return and line feed for each 76 char section
      eBaseNoPad = 0x02,  ///< don't pad with '='
   };

public:
   struct argument  //0TAG0argument - gd::argument::arguments::argument
   {
      union value;   // forward declare
      /// default constructor
      argument(): m_eType(arguments::enumType::eTypeUnknown) {}
      //@{
      /// constructors for specified types
      argument(bool b) : m_eType(arguments::eTypeBool) { m_unionValue.b = b; }
      argument(int8_t v) : m_eType(arguments::eTypeInt8) { m_unionValue.v_int8 = v; }
      argument(uint8_t v) : m_eType(arguments::eTypeUInt8) { m_unionValue.v_uint8 = v; }
      argument(int16_t v) : m_eType(arguments::eTypeInt16) { m_unionValue.v_int16 = v; }
      argument(uint16_t v) : m_eType(arguments::eTypeUInt16) { m_unionValue.v_uint16 = v; }
      argument(int32_t v) : m_eType(arguments::eTypeInt32) { m_unionValue.v_int32 = v; }
      argument(uint32_t v) : m_eType(arguments::eTypeUInt32) { m_unionValue.v_uint32 = v; }
      argument(int64_t v) : m_eType(arguments::eTypeInt64) { m_unionValue.v_int64 = v; }
      argument(uint64_t v) : m_eType(arguments::eTypeUInt64) { m_unionValue.v_uint64 = v; }
      argument(float f) : m_eType(arguments::eTypeFloat) { m_unionValue.f = f; }
      argument(double d) : m_eType(arguments::eTypeDouble) { m_unionValue.d = d; }

      //param(binary_support::uuid* p) : m_eCType(params::eCustomTypeUuid) { m_unionValue.p = p; }

      argument(const char* pbsz) : m_eType( enumType(arguments::eTypeString) ) { m_unionValue.pbsz = pbsz; }
#if defined(__cpp_char8_t)
      argument(const char8_t* pbsz) : m_eType(arguments::eTypeUtf8String) { m_unionValue.putf8 = pbsz; }
#endif
      argument(const wchar_t* pwsz) : m_eType( enumType(arguments::eTypeWString) ) { m_unionValue.pwsz = pwsz; }
      argument(arguments::enumCType eType, const char* pbsz) : m_eType(enumType(eType)) { m_unionValue.pbsz = pbsz; }
      argument(arguments::enumCType eType, const wchar_t* pwsz) : m_eType(enumType(eType)) { m_unionValue.pwsz = pwsz; }
      argument(arguments::enumType eType, const char* pbsz) : m_eType(eType) { m_unionValue.pbsz = pbsz; }
      argument(arguments::enumType eType, const wchar_t* pwsz) : m_eType(eType) { m_unionValue.pwsz = pwsz; }
      argument(void* p) : m_eType(arguments::eTypePointer) { m_unionValue.p = p; }
      argument(const uint8_t* p) : m_eType( enumType(arguments::eTypeBinary) ) { m_unionValue.puch = p; }
      argument(arguments::enumType eType,const uint8_t* p) : m_eType(eType) { m_unionValue.puch = p; }
      argument(unsigned uType,const uint8_t* p) : m_eType((enumType)uType) { m_unionValue.puch = p; }
      argument(const uint8_t* p, enumType eType) : m_eType(eType) { m_unionValue.puch = p; }


      //@}

      argument(const argument& o) {
         common_construct(o);
      }
      argument& operator=(const argument& o) {
         common_construct(o);
         return *this;
      }

      void common_construct(const argument& o) {
         m_eType = o.m_eType;
         m_unionValue.d = o.m_unionValue.d;
      }

      bool operator==(const argument& o) const { return compare_argument_s(*this, o); }
      bool operator==(const gd::variant_view& v) const { return compare_argument_s(*this, v); }
      template<typename TYPE>
      bool operator==(TYPE v) const { return compare_argument_s(*this, gd::variant_view( v ) ); }


      bool operator!=(const argument& o) const { return !(*this == o); }
      bool operator!=(const gd::variant_view& v) const { return !(*this == v); }


      bool operator !() { return (type_number() == arguments::eTypeNumberUnknown); }

      operator variant() { return get_variant(); }
      operator variant() const { return get_variant(false); }

      operator variant_view() const { return get_variant_view(); }

      // ## stream operator to set variables of different common types
      const argument& operator>>(bool& v) const { v = get_bool(); return *this; }
      const argument& operator>>(int32_t& v) const { v = get_int(); return *this; }
      const argument& operator>>(int64_t& v) const { v = get_int64(); return *this; }
      const argument& operator>>(uint32_t& v) const { v = get_uint(); return *this; }
      const argument& operator>>(uint64_t& v) const { v = get_uint64(); return *this; }
      const argument& operator>>(std::string& v) const { v = get_string(); return *this; }


      operator bool() const { assert(type_number() == arguments::eTypeNumberBool); return m_unionValue.b; }
      operator int8_t() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt8) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt8) ); return m_unionValue.v_int8; }
      operator uint8_t() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt8) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt8) ); return m_unionValue.v_uint8; }
      operator int16_t() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt16) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt16) ); return m_unionValue.v_int16; }
      operator uint16_t() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt16) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt16) ); return m_unionValue.v_uint16; }
      //operator long() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt32) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt32)); return m_unionValue.v_int32; }
      operator int32_t() const { assert(type_number() >= arguments::eTypeNumberInt32 && type_number() <= arguments::eTypeNumberUInt32); return m_unionValue.v_int32; }
      operator uint32_t() const { assert(type_number() >= arguments::eTypeNumberInt32 && type_number() <= arguments::eTypeNumberUInt32); return m_unionValue.v_uint32; }
      operator int64_t() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt64) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt64)); return m_unionValue.v_int64; }
      operator uint64_t() const { assert(type_number() >= static_cast<decltype(type_number())>(arguments::eTypeNumberInt64) && type_number() <= static_cast<decltype(type_number())>(arguments::eTypeNumberUInt64)); return m_unionValue.v_uint64; }
      operator double() const { assert(type_number() == arguments::eTypeNumberDouble); return m_unionValue.d; }
      operator const char* () const { assert(type_number() == arguments::eTypeNumberUnknown || type_number() == arguments::eTypeNumberString); return (type_number() == arguments::eTypeNumberString) ? m_unionValue.pbsz : ""; }
      operator const uint8_t* () const { assert(type_number() == arguments::eTypeNumberGuid || type_number() == arguments::eTypeNumberBinary); return m_unionValue.puch; }
      operator const wchar_t* () const { assert(type_number() == arguments::eTypeNumberUnknown || type_number() == arguments::eTypeNumberWString); return (type_number() == arguments::eTypeNumberWString) ? m_unionValue.pwsz : L""; }
      operator std::string() const { assert(type_number() == arguments::eTypeNumberUnknown || type_number() == arguments::eTypeNumberString); return (type_number() == arguments::eTypeNumberString) ? m_unionValue.pbsz : ""; }
      operator std::wstring() const { assert(type_number() == arguments::eTypeNumberUnknown || type_number() == arguments::eTypeNumberWString); return (type_number() == arguments::eTypeNumberWString) ? m_unionValue.pwsz : L""; }
      operator void* () const { assert(type_number() == arguments::eTypeNumberUnknown || type_number() == arguments::eTypeNumberPointer); return (type_number() == arguments::eTypeNumberPointer) ? m_unionValue.p : NULL; }

      /// compare two argument values
      bool compare( const argument& o ) const { return compare_argument_s(*this, o); }
      bool compare( const gd::variant_view& o ) const { return compare_s(*this, o); }
      /// compare within group type, if integer all sizes are valid for comparison
      bool compare_group( const argument& o ) const { return compare_argument_group_s(*this, o); }

      /// length in bytes for param
      unsigned int length() const;
      /// get param type
      // @TODO [tag: arguments] [description: this do not return the complete type (with length and array flags) and maybe this is ok but need to be checked, it should return the complete type including flags for size]
      arguments::enumType type() const { return arguments::enumType((unsigned)m_eType & ~eType_MASK); }
      arguments::enumCType type_number() const { return arguments::enumCType((unsigned)m_eType & ~eTypeNumber_MASK); }
      /// return the raw internal type, this has optional flags for type
      unsigned int ctype() const { return (unsigned int)(m_eType & ~eCType_MASK); }
      /// check if param is empty
      bool empty() const { return (m_eType == arguments::eTypeUnknown); }

      void get_binary_as_hex(std::string& s) const;
      unsigned int get_binary_as_hex(char* pbsz, unsigned int uLength) const;

      /// reset param
      void reset(const argument* pParam = nullptr) {
         if( pParam != nullptr ) {
            *this = *pParam;
         }
         else {
            m_eType = arguments::eTypeUnknown;
         }
      }

      bool         as_bool() const { return get_bool(); }
      unsigned int as_uint() const { return get_uint(); }
      int          as_int() const { return get_int(); }
      int64_t      as_int64() const { return get_int64(); }
      uint64_t     as_uint64() const { return get_uint64(); }
      double       as_double() const { return get_double(); }
      std::string  as_string() const { return get_string(); };
      std::string  as_utf8() const { return get_utf8(); };
      gd::variant  as_variant() const { return get_variant(); }
      gd::variant_view as_variant_view() const { return get_variant_view(); }
      std::string_view as_string_view() const { return get_string_view(); }


      bool         get_bool() const;
      int          get_int() const;
      unsigned int get_uint() const;
      int64_t      get_int64() const;
      uint64_t     get_uint64() const;
      double       get_double() const;
      std::string  get_string() const;
      std::string  get_utf8() const;
      gd::variant  get_variant() const { return arguments::get_variant_s(*this); }
      gd::variant_view  get_variant_view() const { return arguments::get_variant_view_s(*this); }
      gd::variant  get_variant( bool ) const { return is_null() == false ? arguments::get_variant_s(*this, false) : variant(); } /// for speed, do not copy data
      value        get_value() { return m_unionValue; }
      const value& get_value() const { return m_unionValue; }
      std::string_view get_string_view() const { return arguments::get_variant_view_s(*this).as_string_view(); }

      std::string  to_string() const { return get_string(); }
      std::string  to_ut8() const { return get_utf8(); }

      bool         is_null() const { return (type_number() == arguments::eTypeNumberUnknown); }
      bool         is_bool() const { return (type_number() == arguments::eTypeNumberBool); }
      bool         is_int32() const { return (type_number() == arguments::eTypeNumberInt32); }
      bool         is_uint32() const { return (type_number() == arguments::eTypeNumberUInt32); }
      bool         is_int64() const { return (type_number() == arguments::eTypeNumberInt64); }
      bool         is_uint64() const { return (type_number() == arguments::eTypeNumberUInt64); }
      bool         is_double() const { return (type_number() == arguments::eTypeNumberDouble); }
      bool         is_uuid() const { return (type_number() == arguments::eTypeNumberGuid); }
      bool         is_string() const { return (type_number() == arguments::eTypeNumberString); }
      bool         is_utf8() const { return (type_number() == arguments::eTypeNumberUtf8String); }
      bool         is_wstring() const { return (type_number() == arguments::eTypeNumberWString); }
      bool         is_true() const;
      bool         is_primitive() const { return (type_number() > arguments::eTypeNumberUnknown && type_number() <= eTypeNumberDouble); } ///< primitive = built in types in C++
      bool         is_text() const { return (m_eType & arguments::eGroupString) != 0;  } ///< text = some sort of string value, ascii, utf8 or unicode
      bool         is_binary() const { return type_number() == arguments::eTypeNumberBinary; } ///< binary = blob data, length is unknown if used in argument (work with this in arguments)
      bool         is_number() const { return (m_eType & (arguments::eGroupInteger|arguments::eGroupDecimal)) != 0; }
      bool         is_decimal() const { return (m_eType & arguments::eGroupDecimal) != 0; }
      bool         is_integer() const { return (m_eType & arguments::eGroupInteger) != 0; }

      void*        get_raw_pointer() const { return m_unionValue.p; }            ///< return raw pointer to value
      void*        get_value_buffer() const { return (void*)&m_unionValue; }     ///< return address pointer to value

      /// get value as specified type, this is used to get value from argument
      template<typename TYPE> TYPE get() const;

      // attributes
   public:
      arguments::enumType m_eType;      // type of value valid for m_unionValue
      union value
      {
         bool b;
         char ch;
         unsigned char uch;
         short s;
         wchar_t wch;
         int8_t  v_int8;
         uint8_t v_uint8;
         int16_t  v_int16;
         uint16_t v_uint16;
         int32_t  v_int32;
         uint32_t v_uint32;
         int64_t v_int64;
         uint64_t v_uint64;
         float f;
         double d;
         const char* pbsz;
#if defined(__cpp_char8_t)
         const char8_t* putf8;
#endif
         const wchar_t* pwsz;
         const uint8_t* puch;
         void* p;

         value(): v_uint64(0) {}
      } m_unionValue;
   };

   struct argument_edit : public argument
   {
      argument_edit() : argument(), m_pArguments(nullptr) {}

      argument_edit(const argument& o) {
         common_construct(o);
      }

      void common_construct(const argument& o) {
         memcpy(static_cast<void*>(this), &o, sizeof(argument_edit));
      }

      argument_edit& operator=(argument argumentSet) { m_pArguments->set(m_pPosition, argumentSet); return *this; }


      template<typename ARGUMENT_TYPE>
      argument_edit(arguments* parguments, arguments::pointer pPosition, ARGUMENT_TYPE AG): m_pArguments(parguments), m_pPosition(pPosition), argument( AG ) {
         m_pValue = move_to_value_s( (pointer)pPosition );
      }

      template<typename TYPE>
      bool operator==(TYPE v) const { return compare_argument_s(*this, gd::variant_view( v ) ); }

      void set(argument argumentSet);

      arguments* m_pArguments;
      arguments::pointer m_pPosition;
      arguments::pointer m_pValue;
   };

   /**
    * @brief iterator_ for iterating values in params object.
    */
   template<typename ARGUMENTS>
   struct iterator_  //0TAG0iterator - iterator used to move forward for values whithin arguments
   {
      using value_type = argument;
      using iterator_category = std::forward_iterator_tag;
      using self = iterator_;
      using difference_type = std::ptrdiff_t;
      using pointer = const argument*;
      using reference = const argument&;


      iterator_() : m_parguments(nullptr), m_uPosition(0) {}
      iterator_(const arguments* parguments) : m_parguments(parguments), m_uPosition(0) {}
      iterator_( const arguments* parguments, size_t uPosition) : m_parguments(parguments), m_uPosition( uPosition ) {}
      iterator_(const iterator_& o) { m_parguments = o.m_parguments; m_uPosition = o.m_uPosition; }
      iterator_& operator=(const iterator_& o) { m_parguments = o.m_parguments; m_uPosition = o.m_uPosition; return *this; }

      bool operator==(const self& o) const { assert( m_parguments == o.m_parguments ); return m_uPosition == o.m_uPosition; }
      bool operator!=(const self& o) const { return !(*this == o); }
      bool operator>(const self& o) const { return m_uPosition > o.m_uPosition; }
      bool operator<(const self& o) const { return m_uPosition < o.m_uPosition; }

      operator const ARGUMENTS*() const { return m_parguments; }
      operator arguments::const_pointer() const { return buffer_offset(); }

      argument operator*() const {                                                                 assert( m_parguments->verify_d( buffer_offset() ));
         return get_argument();
      }
      self& operator++() {                                                                         assert( m_parguments->verify_d( buffer_offset() ));
         m_uPosition = arguments::next_s(m_parguments->buffer_data(), m_uPosition);                assert(m_parguments->verify_d(buffer_offset()));
         return *this;
      }
      self operator++(int) {                                                                       assert( m_parguments->verify_d( buffer_offset() ));
         iterator_ it = *this;
         ++(*this);
         return it;
      }

      /// Compound assignment operator to advance the iterator by a specified number of values.
      self& operator+=(size_t uCount) {
         // Use existing increment operator
         for(size_t i = 0; i < uCount; ++i) { ++(*this); }
         return *this; // Return a reference to this iterator
      }

      /// Advance the iterator by a specified number of string blocks.
      self operator+(size_t uCount) const {
         iterator it = *this;
         for( size_t i = 0; i < uCount; ++i ) ++it;
         return it;
      }

      /// check if name is present for value
      bool is_name() const {                                                                       assert(m_parguments->verify_d(buffer_offset()));
         return ARGUMENTS::is_name_s(buffer_offset());
      }

      /// get name for value
      std::string name() const {                                                                   assert( m_parguments->verify_d( buffer_offset() ));
         if( ARGUMENTS::is_name_s(buffer_offset()) == true ) { return std::string(ARGUMENTS::get_name_s(buffer_offset())); }
         return std::string();
      }

      /// get name as string_view for value
      std::string_view name(tag_view) const {                                                      assert( m_parguments->verify_d( buffer_offset() ));
         if( arguments::is_name_s(buffer_offset()) == true ) { return ARGUMENTS::get_name_s(buffer_offset()); }
         return std::string_view();
      }

      /// compare name with string
      bool compare_name(const std::string_view& stringName) const {
         if( ARGUMENTS::is_name_s(buffer_offset()) == true )
         {
            if( ARGUMENTS::get_name_s(buffer_offset()) == stringName ) return true;
         }
         return false;
      }

      argument get_argument() { return ARGUMENTS::get_argument_s(buffer_offset()); }
      const argument get_argument() const { return ARGUMENTS::get_argument_s(buffer_offset()); }


      template<std::size_t uIndex>
      auto get() const
      {
         static_assert(uIndex < 2, "Allowed index are 0 and 1, above is not valid");
         if constexpr( uIndex == 0 ) return name();
         if constexpr( uIndex == 1 ) return get_argument();
      }

      arguments::const_pointer buffer_offset() const { return m_parguments->buffer_offset(m_uPosition); }


      // attributes
   public:
      const ARGUMENTS* m_parguments; ///< pointer to arguments object
      size_t m_uPosition;            ///< offset position in buffer
   };

// ## typedefs -----------------------------------------------------------------
public:
   using iterator =           iterator_<arguments>;
   using const_iterator =     iterator_<const arguments>;


   // ## @API [tag: construct] [description: construction methods for creating arguments instances]
public: //0TAG0construct.arguments
   arguments() { buffer_set(); }

   /** Set buffer and size, use this to avoid heap allocations (if internal data grows over buffer size you will get heap allocation)  */
   arguments(pointer pBuffer, unsigned int uSize) : m_bOwner(false), m_pBuffer(pBuffer), m_uLength(0), m_uBufferLength(uSize) {}
   arguments(const std::string_view& stringName, const gd::variant& variantValue, tag_no_initializer_list);


   arguments(std::pair<std::string_view, gd::variant> pairArgument);
   template <typename... Arguments>
   arguments(std::pair<std::string_view, gd::variant> pairArgument, Arguments... arguments)
   {
      zero();
      auto _argument = get_argument_s(pairArgument.second);
      append_argument(pairArgument.first, _argument);
      append_argument(arguments...);
   }
   arguments( std::initializer_list<std::pair<std::string_view, gd::variant>> listPair); // construct arguments with vector like {{},{}}
   arguments( std::initializer_list<std::pair<std::string_view, gd::variant_view>> listPair, tag_view ); // light weight version to construct arguments with vector like {{},{}}
   arguments( std::vector<std::pair<std::string_view, gd::variant_view>> listPair, tag_view ); // light weight version to construct arguments with vector like {{},{}}
   arguments( const std::initializer_list<std::pair<std::string_view, gd::variant_view>>& listPair, const arguments& arguments_ );
   arguments( const arguments& arguments_, const std::initializer_list<std::pair<std::string_view, gd::variant_view>>& listPair );

   arguments( pointer pBuffer, unsigned int uSize, std::initializer_list<std::pair<std::string_view, gd::variant>> listPair);
   template <size_t SIZE>
   arguments( const std::array<std::byte, SIZE>& buffer, std::initializer_list<std::pair<std::string_view, gd::variant>> listPair ) :
      arguments( (pointer)buffer.data(), (unsigned int)buffer.size(), listPair ) { static_assert(SIZE >= 8, "why less than 8?"); }

   arguments( pointer pBuffer, unsigned int uSize, std::initializer_list<std::pair<std::string_view, gd::variant_view>> listPair, tag_view );
   template <size_t SIZE>
   arguments( const std::array<std::byte, SIZE>& buffer, std::initializer_list<std::pair<std::string_view, gd::variant_view>> listPair, tag_view ) :
      arguments( (pointer)buffer.data(), (unsigned int)buffer.size(), listPair, tag_view{} ) { static_assert(SIZE >= 8, "why less than 8?"); }

   // copy
   arguments(const arguments& o) { buffer_set(); common_construct(o); }
   arguments(arguments&& o) noexcept { common_construct((arguments&&)o); }
   // assign
   arguments& operator=(const arguments& o) { clear(); common_construct(o); return *this; }
   arguments& operator=(arguments&& o) noexcept { clear(); common_construct(o); return *this; }

   arguments& operator=(const std::vector<gd::variant_view>& vectorValue);
   arguments& operator=(const std::initializer_list<std::pair<std::string_view, gd::variant_view>>& listPair);
   arguments& operator=(const std::initializer_list<std::pair<std::string_view, gd::variant>>& listPair);
   arguments& operator=(const std::vector<std::pair<std::string_view, gd::variant_view>>& vectorPair);


   ~arguments() { buffer_delete(); }
protected:
   // common copy
   void common_construct(const arguments& o) {
      if( o.m_uLength )
      {
         reserve_no_copy(o.m_uLength);
         memcpy(m_pBuffer, o.m_pBuffer, o.m_uLength);
      }
      m_bOwner = o.m_bOwner;
      m_uLength = o.m_uLength;
   }

   void common_construct(arguments&& o) noexcept {
      memcpy(static_cast<void*>(this), &o, sizeof(arguments));
      o.m_pBuffer = nullptr;
      o.m_bOwner = false;
      o.m_uLength = 0;
      o.m_uBufferLength = 0;
   }

   void zero() { buffer_set(); };

   // ## @API [tag: operator] [description: overloaded operators]
public: //0TAG0operator.arguments
   argument operator[](unsigned uIndex) { return get_argument(uIndex); }
   argument operator[](std::string_view stringName) { return get_argument(stringName); }
   argument operator[](arguments::const_pointer p) { return get_argument(p); }
   const argument operator[](unsigned uIndex) const { return get_argument(uIndex); }
   const argument operator[](std::string_view stringName) const { return get_argument(stringName); }
   const argument operator[](arguments::const_pointer p) const { return get_argument(p); }
   /// returns first found element of those in list
   const argument operator[](std::initializer_list<std::string_view> list_) const { return get_argument( list_ ); }
   /// index operator edit is needed
   argument_edit operator[](const index_edit& index_);

   argument_edit operator()(unsigned uIndex) {
      pointer pPosition = find(uIndex);
      if( pPosition != nullptr ) { return arguments::get_edit_param_s(this, pPosition); }
      return argument_edit();
   }

   argument_edit operator()(std::string_view stringName) {
      pointer pPosition = find(stringName);
      if( pPosition != nullptr ) { return arguments::get_edit_param_s(this, pPosition); }
      return argument_edit();
   }

   std::pair<std::string_view,gd::variant_view> operator()( unsigned uIndex, tag_pair ) const {
      const_pointer pPosition = find(uIndex);
      if( pPosition != nullptr ) { return { get_name_s(pPosition), get_argument_s(pPosition) }; }
      return { "", gd::variant_view() };
   }

   template<typename VALUE>
   arguments& operator+=( std::pair<const char*, VALUE>&& pair ) { return append_argument(std::string_view(pair.first), gd::variant_view(pair.second)); }
   template<typename VALUE>
   arguments& operator+=( std::pair<std::string_view, VALUE>&& pair ) { return append_argument(pair.first, gd::variant_view(pair.second)); }

   arguments& operator+=( const std::initializer_list< std::pair<std::string_view, gd::variant_view> >& listArgument) { return append_argument(listArgument, tag_view{}); }


   arguments operator<<(const std::pair<std::string_view, gd::variant_view>& pairArgument ) { return append_argument(pairArgument, tag_view{}); }

   /// Append values from another arguments object
   arguments& operator+=( const arguments& arguments_ ) { return append( arguments_ ); }
   arguments& operator+=( const gd::arg_view& arg_ ) { return append_argument(arg_.get_key(), arg_.get_value()); }

   arguments& operator<<( std::pair<const char*, gd::variant_view>&& pair ) { return append_argument(std::string_view(pair.first), gd::variant_view(pair.second)); }
   arguments& operator<<( std::pair<std::string_view, gd::variant_view>&& pair ) { return append_argument(pair.first, gd::variant_view(pair.second)); }

   arguments& operator<<( const gd::arg_view&& arg_ ) { return append_argument(arg_.get_key(), arg_.get_value()); }

   arguments& operator>>( gd::arg_view& arg_ );

   // ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
/// return start position to buffer where values are stored
   pointer get_buffer_start() { return m_pBuffer; }
   const_pointer get_buffer_start() const { return m_pBuffer; }
   /// return last position for buffer where values are stored
   pointer get_buffer_end() { return m_pBuffer + m_uLength; }
   const_pointer get_buffer_end() const { return m_pBuffer + m_uLength; }
//@}

/** \name OPERATION
*///@{

   // return if object owns memory, if it does it should be deleted when arguments goes out of scope
   bool is_owner() const noexcept { return m_bOwner; }

   // ## @API [tag: append] [description: append data to arguments]
   //    note: remember that each value has its type and type in stream is just
   //    one byte. That means that the amount of information about the type is
   //    limited. This is the reason why each type only has it's type number.

   arguments& append(std::nullptr_t) { return append(eTypeNumberUnknown, nullptr, 0); }
   arguments& append(int8_t v) { return append(eTypeNumberInt8, (const_pointer)&v, sizeof(int8_t)); }
   arguments& append(uint8_t v) { return append(eTypeNumberUInt8, (const_pointer)&v, sizeof(uint8_t)); }
   arguments& append(int16_t v) { return append(eTypeNumberInt16, (const_pointer)&v, sizeof(int16_t)); }
   arguments& append(uint16_t v) { return append(eTypeNumberUInt16, (const_pointer)&v, sizeof(uint16_t)); }
   arguments& append(int32_t v) { return append(eTypeNumberInt32, (const_pointer)&v, sizeof(int32_t)); }
   arguments& append(uint32_t v) { return append(eTypeNumberUInt32, (const_pointer)&v, sizeof(uint32_t)); }
   arguments& append(int64_t v) { return append(eTypeNumberInt64, (const_pointer)&v, sizeof(int64_t)); }
   arguments& append(uint64_t v) { return append(eTypeNumberUInt64, (const_pointer)&v, sizeof(uint64_t)); }
   arguments& append(const std::string_view& v) { return append((eTypeNumberString | eValueLength), (const_pointer)v.data(), (unsigned int)v.length() + 1); }
   arguments& append(const std::wstring_view& v) { return append((eTypeNumberWString | eValueLength), (const_pointer)v.data(), ((unsigned int)v.length() + 1) * sizeof(wchar_t)); }
#if defined(__cpp_char8_t)
   arguments& append(const char8_t* v) { return append((eTypeNumberUtf8String | eValueLength), (const_pointer)v, (unsigned int)strlen( (const char*)v ) + 1); }
   arguments& append(const char8_t* v, unsigned uLength) { return append((eTypeNumberUtf8String | eValueLength), (const_pointer)v, uLength + 1); }
#endif
   template<typename VALUE, typename... NEXT>
   void append_many(VALUE value_, NEXT... next_) {
      append( value_ );
      if constexpr (sizeof...(next_) > 0) { append_many( next_... ); }
   }
   arguments& append( const argument& argumentValue, tag_argument );
   arguments& append( const gd::variant_view& variantValue, tag_view );


   arguments& append(param_type uType, const_pointer pBuffer, unsigned int uLength);

   arguments& append(const std::string_view& stringName, std::nullptr_t) { return append(stringName, eTypeNumberUnknown, nullptr, 0); }
   arguments& append(const std::string_view& stringName, bool v) { return append(stringName, eTypeNumberBool, (const_pointer)&v, sizeof(bool)); }
   arguments& append(const std::string_view& stringName, int8_t v) { return append(stringName, eTypeNumberInt8, (const_pointer)&v, sizeof(int8_t)); }
   arguments& append(const std::string_view& stringName, uint8_t v) { return append(stringName, eTypeNumberUInt8, (const_pointer)&v, sizeof(uint8_t)); }
   arguments& append(const std::string_view& stringName, int16_t v) { return append(stringName, eTypeNumberInt16, (const_pointer)&v, sizeof(int16_t)); }
   arguments& append(const std::string_view& stringName, uint16_t v) { return append(stringName, eTypeNumberUInt16, (const_pointer)&v, sizeof(uint16_t)); }
   arguments& append(const std::string_view& stringName, int32_t v) { return append(stringName, eTypeNumberInt32, (const_pointer)&v, sizeof(int32_t)); }
   arguments& append(const std::string_view& stringName, uint32_t v) { return append(stringName, eTypeNumberUInt32, (const_pointer)&v, sizeof(uint32_t)); }
   arguments& append(const std::string_view& stringName, int64_t v) { return append(stringName, eTypeNumberInt64, (const_pointer)&v, sizeof(int64_t)); }
   arguments& append(const std::string_view& stringName, uint64_t v) { return append(stringName, eTypeNumberUInt64, (const_pointer)&v, sizeof(uint64_t)); }
   arguments& append(const std::string_view& stringName, float v) { return append(stringName, eTypeNumberFloat, (const_pointer)&v, sizeof(float)); }
   arguments& append(const std::string_view& stringName, double v) { return append(stringName, eTypeNumberDouble, (const_pointer)&v, sizeof(double)); }
   arguments& append(const std::string_view& stringName, const char* v) { return append(stringName, (eTypeNumberString | eValueLength), (const_pointer)v, (unsigned int)strlen(v) + 1); }
   arguments& append(const std::string_view& stringName, const std::string_view& v) { return append(stringName, (eTypeNumberString | eValueLength), (const_pointer)v.data(), (unsigned int)v.length() + 1); }
   arguments& append(const std::string_view& stringName, std::wstring_view v) { return append(stringName, (eTypeNumberWString | eValueLength), (const_pointer)v.data(), ((unsigned int)v.length() + 1) * sizeof(wchar_t)); }
#if defined(__cpp_char8_t)
   arguments& append(const std::string_view& stringName, const char8_t* v) { return append( stringName, (eTypeNumberUtf8String | eValueLength), (const_pointer)v, (unsigned int)strlen( (const char*)v ) + 1); }
   arguments& append(const std::string_view& stringName, const char8_t* v, unsigned uLength) { return append( stringName, (eTypeNumberUtf8String | eValueLength), (const_pointer)v, uLength + 1); }
#endif
   arguments& append(const std::string_view& stringName, param_type uType, const_pointer pBuffer, unsigned int uLength) { return append(stringName.data(), (uint32_t)stringName.length(), uType, pBuffer, uLength); }
   arguments& append(const char* pbszName, uint32_t uNameLength, param_type uType, const_pointer pBuffer, unsigned int uLength);
   template<typename POINTER>
   arguments& append(const char* pbszName, uint32_t uNameLength, param_type uType, POINTER pBuffer, unsigned int uLength) {
      static_assert(std::is_pointer<POINTER>::value);
      return append(pbszName, uNameLength, uType, (const_pointer)pBuffer, uLength);
   }
   arguments& append( const arguments& arguments_ );
   arguments& append( const std::vector<gd::variant_view>& vectorValue );
   arguments& append( const std::vector<std::pair<std::string_view,std::string_view>>& vectorStringValue );
   arguments& append( const std::vector<std::pair<std::string,std::string>>& vectorStringValue );
   arguments& append( const std::vector<std::pair<std::string,gd::variant>>& vectorStringVariant );
   arguments& append( const std::vector<std::pair<std::string_view, gd::variant_view>>& vectorStringVariantView );
   arguments& append( const std::vector<std::pair<std::string_view,std::string_view>>& vectorStringValue, tag_parse_type );
   arguments& append( const std::vector<std::pair<std::string,std::string>>& vectorStringValue, tag_parse_type );
   std::pair<bool, std::string>  append( const std::string_view& stringValue, tag_parse );

   arguments& append(const std::string_view& stringName, const std::vector<argument>& vectorArgument);
   arguments& append(const std::string_view& stringName, const std::vector<gd::variant_view>& vectorArgument);

   template<typename ARGUMENTS>
   arguments& append( const ARGUMENTS& arguments_, const std::initializer_list<std::string_view>& list_ );

   template<typename VALUE>
   arguments& append_pair( const std::pair<std::string_view,VALUE>& pair_ ) {
      return append_argument( pair_.first, pair_.second );
   }

   /// Append named `argument`
   arguments& append_argument(const std::string_view& stringName, argument argumentValue);

   arguments& append_argument(const variant& variantValue);
   arguments& append_argument(const variant_view& variantviewValue, tag_view);

   arguments& append_argument(std::string_view stringName, const gd::variant& variantValue) {
      auto argumentValue = get_argument_s(variantValue);
      const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
      unsigned uType = argumentValue.type_number();
      if( uType > ARGUMENTS_NO_LENGTH ) { uType |= eValueLength; }
      return append(stringName, uType, pData, argumentValue.length());
   }

   arguments& append_argument(const std::string_view& stringName, const gd::variant_view& variantValue);
   arguments& append_argument(const std::string_view& stringName, const gd::variant_view& variantValue, tag_view) { return append_argument( stringName, variantValue ); }

   arguments& append_argument(const std::pair<std::string_view, gd::variant>& pairArgument) {
      return append_argument(pairArgument.first, pairArgument.second);
   }
   arguments& append_argument(const std::pair<std::string_view, gd::variant_view>& pairArgument, tag_view) {
      return append_argument(pairArgument.first, pairArgument.second);
   }
   arguments& append_argument(const std::string_view stringName, const std::string_view& stringValue, tag_parse_type );
   arguments& append_argument(const std::string_view& stringName, const std::string& stringValue);

   arguments& append_argument( const std::initializer_list< std::pair<std::string_view, gd::variant_view> >& vectorArgument, tag_view );
   arguments& append_argument( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorArgument, tag_view );


   arguments& append_binary( const uint8_t* puData, unsigned int uLength ) { return append(eTypeNumberBinary, puData, uLength); }
   arguments& append_binary(std::string_view stringName, const uint8_t* puData, unsigned int uLength) { return append(stringName, (eTypeNumberBinary | eValueLength), puData, uLength); }
   arguments& append_uuid( const uint8_t* puData ) { return append(eTypeNumberGuid, puData, 16); }
   arguments& append_uuid(std::string_view stringName, const uint8_t* puData ) { return append(stringName, eTypeNumberGuid, puData, 16); }

   template<typename VALUE>
   arguments& append_if(const std::string_view& stringName, VALUE value );

   template<typename OBJECT>
   arguments& append_object(const std::string_view& stringName, const OBJECT object );
   template<typename OBJECT>
   arguments& append_object( const OBJECT object ) { return append_object( std::string_view(), object ); }

   arguments& push_back( const variant_view& variantviewValue ) { return append_argument(variantviewValue, tag_view{}); }
   arguments& push_back( std::string_view stringName, const variant_view& variantviewValue ) { return append_argument( stringName, variantviewValue, tag_view{}); }
   arguments& push_back( const std::pair<std::string_view, gd::variant_view>& pairArgument ) { return append_argument(pairArgument, tag_view{}); }

   // ## @API [tag: set] [description: set methods, if value exists it is overwritten, otherwise it is appended]
   //    Set values for selected position in buffer, it could be for a name, index or pointer
   //    If position is not found, new value is appended to buffer

   arguments& set(const std::string_view& stringName, std::nullptr_t) { return set(stringName, eTypeNumberBool, nullptr, 0); }
   arguments& set(const std::string_view& stringName, bool v) { return set(stringName, eTypeNumberBool, (const_pointer)&v, sizeof(bool)); }
   arguments& set(const std::string_view& stringName, int8_t v) { return set(stringName, eTypeNumberInt8, (const_pointer)&v, sizeof(int8_t)); }
   arguments& set(const std::string_view& stringName, uint8_t v) { return set(stringName, eTypeNumberUInt8, (const_pointer)&v, sizeof(uint8_t)); }
   arguments& set(const std::string_view& stringName, int16_t v) { return set(stringName, eTypeNumberInt16, (const_pointer)&v, sizeof(int16_t)); }
   arguments& set(const std::string_view& stringName, uint16_t v) { return set(stringName, eTypeNumberUInt16, (const_pointer)&v, sizeof(uint16_t)); }
   arguments& set(const std::string_view& stringName, int32_t v) { return set(stringName, eTypeNumberInt32, (const_pointer)&v, sizeof(int32_t)); }
   arguments& set(const std::string_view& stringName, uint32_t v) { return set(stringName, eTypeNumberUInt32, (const_pointer)&v, sizeof(uint32_t)); }
   arguments& set(const std::string_view& stringName, int64_t v) { return set(stringName, eTypeNumberInt64, (const_pointer)&v, sizeof(int64_t)); }
   arguments& set(const std::string_view& stringName, uint64_t v) { return set(stringName, eTypeNumberUInt64, (const_pointer)&v, sizeof(uint64_t)); }

   arguments& set(const std::string_view& stringName, float v) { return set(stringName, eTypeNumberFloat, (const_pointer)&v, sizeof(float)); }
   arguments& set(const std::string_view& stringName, double v) { return set(stringName, eTypeNumberDouble, (const_pointer)&v, sizeof(double)); }

   arguments& set(const std::string_view& stringName, const char* v) { return set(stringName, std::string_view(v) ); }
   arguments& set_uuid(const std::string_view& stringName, const uint8_t* puData) { return set(stringName, eTypeNumberGuid, (const_pointer)puData, 16); }


   arguments& set(std::string_view stringName, std::string_view v, tag_string_view) { return set(stringName, (eTypeNumberString | eValueLength), (const_pointer)v.data(), (unsigned int)v.length() + 1); }

   arguments& set(pointer pPosition, const gd::variant_view& variantValue) { return set(pPosition, variantValue, nullptr); }
   arguments& set(pointer pPosition, const gd::variant_view& variantValue, pointer* ppPosition);
   arguments& set(std::string_view stringName, const gd::variant_view& variantValue);

   arguments& set(const arguments& arguments_);

   arguments& set(std::string_view stringName, param_type uType, const_pointer pBuffer, unsigned int uLength) { return set(stringName.data(), (uint32_t)stringName.length(), uType, pBuffer, uLength); }
   arguments& set(const char* pbszName, uint32_t uNameLength, param_type uType, const_pointer pBuffer, unsigned int uLength);
   arguments& set(pointer pPosition, param_type uType, const_pointer pBuffer, unsigned int uLength) { return set( pPosition, uType, pBuffer, uLength, nullptr ); }
   arguments& set(pointer pPosition, param_type uType, const_pointer pBuffer, unsigned int uLength, pointer* ppPosition);

   void set( pointer pposition, const argument& argumentSet, tag_argument );
   arguments& set( pointer pposition, const argument& argumentSet ) { set( pposition, argumentSet, tag_argument{}); return *this; }

   // ## @API [tag: insert] [description: inserts a new argument at the specified position]

   pointer insert( size_t uIndex, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_view );
   pointer insert( pointer pPosition, const gd::variant_view& variantviewValue, tag_view );
   pointer insert( pointer pPosition, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_view );

   // ## @API [tag: merge] [description: merges values from another arguments object]

   /// merge values from another arguments object, onlye named values are merged
   arguments& merge(const arguments& arguments_);

   // ## @API [tag: iterator] [description: provides iterators for traversing arguments]

   // ### value based iterators, faster but less support for stl iterator logic

   iterator begin() { return iterator( this ); }
   iterator end() { return iterator( this, m_uLength ); }
   const_iterator begin() const { return const_iterator( this ); }
   const_iterator end() const { return const_iterator( this, m_uLength ); }
   const_iterator cbegin() const { return const_iterator( this ); }
   const_iterator cend() const { return const_iterator( this, m_uLength ); }

   // ### named based iterators, slower but with better support for stl iterator logic

   named_iterator_t named_begin();
   named_iterator_t named_end();
   const_named_iterator named_begin() const;
   const_named_iterator named_end() const;
   const_named_iterator named_cbegin() const;
   const_named_iterator named_cend() const;

   // ## @API [tag: count] [description: count related methods, things that information about number of items in arguments, e.g. size, empty, capacity]

   [[nodiscard]] unsigned int capacity() const { assert(m_pBuffer != nullptr); return m_uBufferLength; }

   bool empty() const noexcept { return m_uLength == 0; }
   size_t size( tag_memory ) const noexcept { return m_uLength; }
   unsigned int count(std::string_view stringName) const;
   /// number of arguments found in arguments object
   [[nodiscard]] size_t size() const;

   // ## @API [tag: find] [description: find methods to find values within arguments objects]

   [[nodiscard]] pointer find(unsigned int uIndex);
   [[nodiscard]] const_pointer find(unsigned int uIndex) const;
   [[nodiscard]] pointer find(const std::string_view& stringName);
   [[nodiscard]] const_pointer find(const std::string_view& stringName) const;
   [[nodiscard]] const_pointer find(std::string_view stringName, const_pointer pPosition) const;
   [[nodiscard]] const_pointer find(const std::pair<std::string_view, gd::variant_view>& pairMatch) const;
   [[nodiscard]] const_pointer find( const std::string_view& stringName, unsigned uIndex ) const;

   [[nodiscard]] std::pair<argument,argument> find_pair(const std::string_view& stringName) const;

   std::vector<const_pointer> find_all(std::string_view stringName) const;
   /// find param value for name
   [[nodiscard]] argument find_argument(std::string_view stringName) const {
      const_pointer pPosition = find(stringName);
      if( pPosition ) return get_argument_s(pPosition);
      return argument();
   }
   [[nodiscard]] argument find_argument(std::string_view stringName, unsigned uIndex) const {
      const_pointer pPosition = find(stringName, uIndex);
      if( pPosition ) return get_argument_s(pPosition);
      return argument();
   }

   /// return argument object that can be used to edit value if found
   [[nodiscard]] argument_edit find_edit_argument(std::string_view stringName);
   /// find param value for name and start from position to search
   [[nodiscard]] argument find_argument(const std::string_view& stringName, const_pointer pPosition) const;

   [[nodiscard]] bool exists( const std::string_view& stringName ) const;
   [[nodiscard]] bool exists( const std::initializer_list<std::string_view>& listName ) const;
   [[nodiscard]] bool exists_any( const std::initializer_list<std::string_view>& listName ) const;
   [[nodiscard]] std::pair<bool, std::string> exists( const std::initializer_list<std::string_view>& listName, tag_name ) const { return exists_s( *this, listName, tag_name{}); }
   [[nodiscard]] std::pair<bool, std::string> exists( const std::initializer_list<std::pair<std::string_view, std::string_view>>& listName, tag_description ) const { return exists_s( *this, listName, tag_description{}); }
   [[nodiscard]] std::pair<bool, std::string> exists_any_of( const std::initializer_list<std::string_view>& listName, tag_name ) const { return exists_any_of_s( *this, listName, tag_name{}); }

   // ## @API [tag: compare] [description: compare methods, checks if values in arguments are equal]

   [[nodiscard]] bool compare(const std::pair<std::string_view, gd::variant_view>& pairMatch) const { return find(pairMatch) != nullptr; }
   [[nodiscard]] bool compare(const std::string_view& stringName, const arguments& argumentsCompareTo) const;
   [[nodiscard]] bool compare_exists(const arguments& argumentsExists) const { return compare_exists_s( *this, argumentsExists ); }

   // ## @API [tag: walk, iterator] [description: walk between items in arguments, moves pointer to next value, can't go back]

   [[nodiscard]] pointer next() { return m_uLength > 0 ? m_pBuffer : nullptr; }
   [[nodiscard]] const_pointer next() const { return m_uLength > 0 ? m_pBuffer : nullptr; }
   [[nodiscard]] pointer next(pointer pPosition) const {                                           assert( verify_d(pPosition) );
      auto p = next_s(pPosition);
      return p < get_buffer_end() ? p : nullptr;
   }
   [[nodiscard]] const_pointer next(const_pointer pPosition) const {                               assert( verify_d(pPosition) );
      auto p = next_s(pPosition);
      return p < get_buffer_end() ? p : nullptr;
   }

   // ## @API [tag: misc] [description: miscellaneous methods for arguments management]

   /// cleans upp interal data and set it as empty
   void clear();

   /// Return raw data buffer
   [[nodiscard]] void* data() { return m_pBuffer; }

   // ## @API [tag: get] [description: get methods to retrieve values from arguments objects, note that arguments store `argument` values, argument is a type of variant]

   [[nodiscard]] argument get_argument() const { return get_argument_s(m_pBuffer); }
   [[nodiscard]] argument get_argument(const_pointer pPosition) const {                assert( verify_d(pPosition) );
      return get_argument_s(pPosition);
   }

   [[nodiscard]] argument get_argument(unsigned int uIndex) const;
   [[nodiscard]] argument get_argument(std::string_view stringName) const { return find_argument(stringName); }
   [[nodiscard]] argument get_argument(std::string_view stringName, size_t uIndex ) const { return find_argument(stringName, (unsigned)uIndex ); }
   [[nodiscard]] argument get_argument( std::tuple<std::string_view, size_t> index_ ) const { return find_argument( std::get<0>(index_), (unsigned)std::get<1>(index_) ); }
   [[nodiscard]] argument get_argument(std::string_view stringName, unsigned uSecondIndex, tag_section ) const;
   template<class DEFAULT>
   [[nodiscard]] DEFAULT get_argument(const std::string_view& stringName, DEFAULT defaultValue) const {
      argument  v = find_argument(stringName);
      if( v.is_null() ) return defaultValue;
      return static_cast<DEFAULT>(v);
   }
   template<class DEFAULT>
   [[nodiscard]] DEFAULT get_argument(const std::string_view& stringName, unsigned uIndex, DEFAULT defaultValue) const {
      argument  v = find(stringName, uIndex);
      if( v.is_null() ) return defaultValue;
      return static_cast<DEFAULT>(v);
   }
   [[nodiscard]] std::string get_argument(const std::string_view& stringName, const std::string& stringDefault) const {
      argument  v = find_argument(stringName);
      if( v.is_null() ) return stringDefault;
      return v.get_string();
   }

   [[nodiscard]] argument get_argument( const std::initializer_list<std::string_view>& listName ) const;
   [[nodiscard]] std::vector<argument> get_argument( const std::vector<const_pointer>& vectorPosition ) const;

   [[nodiscard]] std::optional<argument> get_argument(size_t uIndex, gd::types::tag_optional) const;
   [[nodiscard]] std::optional<argument> get_argument(const std::string_view& stringName, gd::types::tag_optional) const;


   /**
    * Try to get value for param name, if not found then insert `vInsert` into params
    * object and return param for that inserted value.
    * Make sure that vInsert is compatible with values that can be stored as param
    */
   template<typename INSERT_VALUE>
   [[nodiscard]] argument get_argument(std::string_view stringName, const INSERT_VALUE& vInsert) {
      auto paramV = find_argument(stringName);
      if( paramV.empty() == true ) {
         auto uOffset = get_buffer_end() - get_buffer_start();
         append(stringName, vInsert);
         return arguments::get_argument_s(get_buffer_start() + uOffset);
      }
      return paramV;
   }

   /// return all values for name, name for value do not need to be unique. if name is not found empty vector is returned
   [[nodiscard]] std::vector<argument> get_argument_all(std::string_view stringName) const { return get_argument_all_s(get_buffer_start(), get_buffer_end(), stringName); }
   [[nodiscard]] std::vector<gd::variant_view> get_argument_all(std::string_view stringName, tag_view) const { return get_argument_all_s(get_buffer_start(), get_buffer_end(), stringName, tag_view{} ); }
   [[nodiscard]] std::vector<gd::variant_view> get_argument_section(std::string_view stringName, tag_view) const { return get_argument_section_s(get_buffer_start(), get_buffer_end(), stringName, tag_view{} ); }

   /// return all values for name or names as vector of type TYPE
   template<typename TYPE>
   [[nodiscard]] std::vector<TYPE> get_all(const std::string_view& stringName) const {
      std::vector<TYPE> vectorValue;
      auto vector_ = get_argument_all(stringName);
      for( const auto& argument : vector_ ) { vectorValue.push_back(argument.get<TYPE>()); }
      return vectorValue;
   }


   std::vector<argument> get_argument( std::vector< std::string_view > vectorName ) const;

   /// return first value for name
   gd::variant_view get_variant_view( const std::string_view& stringName ) const { return get_argument( stringName ).get_variant_view(); }
   [[nodiscard]] std::pair< std::string_view, gd::variant_view > get_variant_view(unsigned int uIndex, tag_pair ) const;

   /// return first value for index as optional
   std::optional<gd::variant_view> get_variant_view(size_t uIndex, gd::types::tag_optional) const;
   /// return first value for name as optional
   std::optional<gd::variant_view> get_variant_view( const std::string_view& stringName, gd::types::tag_optional ) const;
                                                                                                   // @CODE [tag: object, glue] [description: Methods to get and set objects into arguments object, add the `get_object` method as free function]
   template<typename OBJECT>
   void get_object( const std::string_view& stringPrefixFind, OBJECT& object_ );
   template<typename OBJECT>
   void get_object( OBJECT& object_ ) { get_object( std::string_view(), object_ ); }
   template<typename OBJECT>
   OBJECT get_object();
   template<typename OBJECT>
   OBJECT get_object( const std::string_view& stringPrefixFind );


   // ## @API [tag: print] [description: Methods used to format argument values, values are mostly printed into std::string object]

   std::string print() const;
   std::string print( const_iterator itBegin ) const { return print(itBegin, cend(), ", "); };
   std::string print( const_iterator itBegin, const_iterator itEnd ) const { return print(itBegin, itEnd, ", "); };
   std::string print( const_iterator itBegin, const_iterator itEnd, std::string_view stringSplit) const;
   std::string print( const std::string_view& stringSplit, gd::types::tag_key ) const;
   std::string print( gd::types::tag_key ) const { return print( std::string_view(", "), gd::types::tag_key{}); }
   std::string print( const std::string_view& stringSplit, gd::types::tag_value ) const;
   std::string print( gd::types::tag_value ) const { return print( std::string_view(", "), gd::types::tag_value{}); }
   std::string print_json() const;

   std::string print(std::string_view stringFormat) const;

#ifndef NDEBUG
   bool verify_d(const_pointer pPosition) const;
#endif

   // ## @API [tag: buffer, remmove] [description: Internal buffer related and methods to remove values]

   /// erase argument value at iterator
   iterator erase(iterator itPosition) { remove(static_cast<const_pointer>( itPosition )); return itPosition < end() ? itPosition : end(); }
   /// erase argument value at iterator
   const_iterator erase(const_iterator itPosition) { remove(static_cast<const_pointer>( itPosition )); return itPosition < cend() ? itPosition : cend(); }
   /// make sure internal buffer can hold specified number of bytes, buffer data is copied if increased
   bool reserve(unsigned int uCount);
   //bool reserve(uint64_t uCount) { assert( uCount < 0x1'0000'0000 ); return reserve(static_cast<unsigned int>(uCount)); }
   /// Remove param starting at position, remember that if you are string positions in buffer they are invalidated with this method
   void remove( const std::string_view& stringName );
   /// Remove param at index
   void remove( size_t uIndex );
   void remove(const_pointer pPosition);
   void remove(const_iterator it) { remove( static_cast<const_pointer>(it) ); }
   void remove_all( const std::string_view& stringName );

   /// make sure internal buffer can hold specified number of bytes, no copying just reserving data
   pointer reserve_no_copy(unsigned int uCount) {
      if( is_owner() == false || m_pBuffer == nullptr || uCount > m_uBufferLength ) { return _reserve_no_copy(uCount + (uCount >> 1)); }
      return m_pBuffer;
   }
   pointer _reserve_no_copy(unsigned int uCount);

   /// Resize one argument within arguments object, do not use this if you do not know how arguments work!!
   int resize(pointer pPosition, int iOffset, int iNewOffset);
   /// remove unused memory
   void shrink_to_fit();

   static bool is_name_s(const_pointer pPosition) {
      assert(*pPosition != 0);
      return *pPosition == arguments::eType_ParameterName;
   }
   std::string_view get_name(const_pointer pPosition) { return get_name_s( pPosition ); }

   // ## @API [tag: internal, free-functions] [description: Internal free functions for argument manipulation, if used outside make sure you know the internals]

   /// ## Move logic
   static pointer move_to_value_s(pointer pPosition);
   static const_pointer move_to_value_s(const_pointer pPosition);

   static pointer move_to_value_data_s(pointer pPosition);
   static const_pointer move_to_value_data_s(const_pointer pPosition);


   /// ## Compare logic
   /// Compare argument name if argument has name
   static bool compare_name_s(const_pointer pPosition, std::string_view stringName);
   /// compare arguments
   static bool compare_argument_s(const argument& argument1, const argument& argument2);
   static bool compare_argument_s(const argument& a, const gd::variant_view& v);
   static bool compare_argument_group_s(const argument& argument1, const argument& argument2);
   static bool compare_argument_group_s(const argument& a1, const gd::variant_view& v2);

   static bool compare_exists_s(const arguments& argumentsSource, const arguments& argumentsFind);
   /// compare if argument type is fixed size, this is useful when setting values in arguments object
   static constexpr bool is_type_fixed_size_s(unsigned uType) { return (uType & ~eTypeNumber_MASK) <= eTypeNumberGuid; }

   /// compare type for type
   static inline bool compare_type_s(const argument& v1, const argument& v2) { return v1.type_number() == v2.type_number(); }
   static inline bool compare_type_s(const argument& v1, unsigned int uType ) { return v1.type_number() == (uType & ~eTypeNumber_MASK); }

   static bool compare_s(const argument& v1, const gd::variant_view v2);

   /// ## name and type methods
   static std::string_view get_name_s(const_pointer pPosition) {                                   assert(arguments::is_name_s(pPosition));
      pPosition++;                                                               // move past type (now on name length)
      return std::string_view(reinterpret_cast<const char*>(pPosition + 1), *pPosition);// generate string_view for name
   }

   /// ## argument methods
   /// return param the position points to
   static argument get_argument_s(const_pointer pPosition);
   /// return editable param based on position
   static argument_edit get_edit_param_s(arguments* parguments, pointer pPosition);
   /// count internal param length in bytes
   static unsigned int get_total_param_length_s(const_pointer pPosition);
   static unsigned int get_total_param_length_s(std::string_view stringName, const argument argumentValue);
   static std::vector<argument> get_argument_all_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName);
   static std::vector<gd::variant_view> get_argument_all_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName, tag_view);
   static std::vector<std::pair<std::string_view, gd::variant_view>> get_argument_all_s(const_pointer pBegin, const_pointer pEnd,  tag_view, tag_pair);
   static std::vector<gd::variant_view> get_argument_section_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName, tag_view);

   /// ## move methods
   /// move pointer to next value in buffer
   static pointer next_s(pointer pPosition);
   static const_pointer next_s(const_pointer pPosition);
   static const_pointer next_s(const_pointer pPosition, unsigned uSecondIndex, const_pointer pEnd );
   static pointer next_s(pointer pPosition, unsigned uSecondIndex, const_pointer pEnd );
   static size_t next_s(const_pointer pbuffer, size_t uOffset);

   /// ## Calculate size in bytes needed for argument values stored in arguments object
   static unsigned int sizeof_s(const argument& argumentValue);
   static unsigned int sizeof_s(const gd::variant_view& VV_, tag_view);
   static unsigned int sizeof_s( const std::string_view& stringName, const gd::variant_view& VV_, tag_view );
   static unsigned int sizeof_s(uint32_t uNameLength, param_type uType, unsigned int uLength);
   static inline unsigned int sizeof_name_s(uint32_t uNameLength) { return uNameLength + 2; }
   static inline unsigned int sizeof_name_s(const_pointer pPosition) {
      if( *pPosition == eType_ParameterName ) return 2 + pPosition[1];           // name marker = 1 byte, name length = 1 byte and the total name length
      return 0;
   }

   static constexpr inline unsigned int sizeof_value_prefix(param_type uType) { return uType & eValueLength ? sizeof(uint32_t) + 1 : 1; }

   static inline unsigned int length_name_s(const_pointer pPosition) {
      if( *pPosition == eType_ParameterName ) return  pPosition[1];              // the total name length found in byte with name length
      return 0;
   }

   /// ## Append arguments

   /// append pair to arguments
   static void append_argument_s(arguments& arguments, const std::pair<std::string_view, gd::variant>& pairArgument) {
      arguments.append_argument(pairArgument);
   }

   /// append multiple pairs
   template<typename First, typename... Argument>
   static void append_argument_s(arguments& rArguments, const First& pairArgument, Argument... pairNext) {
      rArguments.append_argument(pairArgument);
      append_argument_s(rArguments, pairNext...);
   }

   /// append pairs using initializer list
   static void append_argument_s( arguments& rArguments, std::initializer_list<std::pair<std::string_view, gd::variant>> listPair ) {
      for( auto it : listPair ) rArguments.append_argument(it);
   }

   /// ## Create arguments object
   ///
   /// Create arguments object from pair
   static arguments create_s(const std::pair<std::string_view, gd::variant>& pairArgument) {
      arguments A;
      append_argument_s(A, pairArgument);
      return A;
   }

   /// Create arguments object from pair list
   static arguments create_s(std::initializer_list<std::pair<std::string_view, gd::variant>> listPair) {
      arguments A;
      for( auto it : listPair ) A.append_argument(it);
      return A;
   }

   /// Create arguments object from arguments
   static arguments create_s(const std::string_view& stringName, const gd::variant& variantValue, tag_no_initializer_list) {
      arguments A(stringName, variantValue, tag_no_initializer_list{});
      return A;
   }


   /// ## dump information about values
   static inline std::string print_s(const_pointer pPosition) { return print_s(pPosition, ePairTypeAll); }
   static std::string print_s(const_pointer pPosition, uint32_t uPairType );
   static void print_name_s(const_pointer pPosition, std::string& stringPrint);
   static void print_type_s(const_pointer pPosition, std::string& stringPrint);
   static void print_value_s(const_pointer pPosition, std::string& stringPrint);

   /// ## find out type for value
   static unsigned int type_s(const_pointer pPosition) noexcept;
   constexpr static unsigned int type_s(unsigned int uType) { return uType & ~eType_MASK; } // only type (no size)
   constexpr static unsigned int ctype_s(unsigned int uType) { return uType & ~eCType_MASK; } // last byte (type and size)
   constexpr static unsigned int type_number_s(unsigned int uType) { return uType & ~eTypeNumber_MASK; }
   constexpr static std::string_view type_name_s(uint32_t uType);


   /// count zero terminator length in bytes if type is some sort of string
   constexpr static unsigned int get_string_zero_terminate_length_s(unsigned int uType) {
      uType = uType & ~eTypeNumber_MASK;
      switch( uType ) {
         case eTypeNumberString:
         case eTypeNumberUtf8String:
            return 1;
         case eTypeNumberWString:
            return 2;
         case eTypeNumberUtf32String:
            return 4;
      }
      return 0;
   }

   /// ## `variant` methods
   /// get argument value as variant
   static gd::variant get_variant_s(const argument& argumentValue);
   static gd::variant get_variant_s(const argument& argumentValue, bool);
   static std::vector<gd::variant> get_variant_s( const std::vector<argument>& vectorValue );

   static gd::variant_view get_variant_view_s(const argument& argumentValue);
   static std::vector<gd::variant_view> get_variant_view_s(const std::vector<argument>& vectorValue);

   static argument get_argument_s(const gd::variant& variantValue);
   static argument get_argument_s(const gd::variant_view& variantValue);

   // ## validate methods

   /// Check that specified values exist in arguments object
   static std::pair<bool, std::string> exists_s( const arguments& argumentsValidate, const std::initializer_list<std::string_view>& listName, tag_name );
   /// Check that specified values exist in arguments object and return error text that you can specify
   static std::pair<bool, std::string> exists_s( const arguments& argumentsValidate, const std::initializer_list<std::pair<std::string_view, std::string_view>>& listValue, tag_description );
   /// Check if any of the name values are found in arguments
   static std::pair<bool, std::string> exists_any_of_s( const arguments& argumentsValidate, const std::initializer_list<std::string_view>& listName, tag_name );

   /// copy name into buffer `pCopyTo` points to
   static unsigned memcpy_s( pointer pCopyTo, const char* pbszName, unsigned uLength );
   /// copy value into buffer `pCopyTo` points to
   static uint64_t memcpy_s( pointer pCopyTo, argument_type uType, const_pointer pBuffer, unsigned int uLength );
   //@}


// ##
public:
   // ## buffer methods, used to access buffer data 0TAG0buffer.arguments

   void buffer_set() { memset(static_cast<void*>(this), 0, sizeof( arguments ) ); }
   void buffer_set( pointer p_ ) { m_pBuffer = p_; }
   void buffer_delete() { if( is_owner() ) delete [] m_pBuffer; }
   pointer buffer_data() { return m_pBuffer; }
   pointer buffer_data_end() { return m_pBuffer + m_uLength; }
   const_pointer buffer_data() const { return m_pBuffer; }
   const_pointer buffer_data_end() const { return m_pBuffer + m_uLength; }
   unsigned int buffer_size() const { return m_uLength; }
   void buffer_set_size(unsigned uSize) { m_uLength = uSize; }
   void buffer_set_size(uint64_t uSize) { m_uLength = (unsigned)uSize; }
   unsigned int buffer_buffer_size() const { return m_uBufferLength; }
   size_t buffer_offset( const_pointer pPosition ) const { assert( verify_d( pPosition ) ); return (pPosition - m_pBuffer); }
   const_pointer buffer_offset( size_t uPosition ) const { assert( uPosition <= (size_t)m_uLength ); return (m_pBuffer + uPosition); }



// ## attributes ----------------------------------------------------------------
public:
   bool           m_bOwner;      ///< if buffer is owned (delete in destructor)
   pointer        m_pBuffer;     ///< pointer to byte array
   unsigned int   m_uLength;     ///< length in use
   unsigned int   m_uBufferLength;///< length for byte array

   static size_type npos;

};

/// Return value as specified template type
/// Sample usage: `arguments a_( { {"int-number", 100 } } ); a_[0u].get<int>();`
template<typename TYPE>
TYPE arguments::argument::get() const {
   static_assert(std::is_arithmetic_v<TYPE> ||
      std::is_same_v<TYPE, std::string> ||
      std::is_same_v<TYPE, std::string_view> ||
      std::is_same_v<TYPE, std::wstring> ||
      std::is_same_v<TYPE, const char*> ||
      std::is_same_v<TYPE, const wchar_t*>,
      "TYPE must be a primitive type or supported string type");

   if constexpr (std::is_same_v<TYPE, bool>) {
      return get_bool();
   }
   else if constexpr (std::is_same_v<TYPE, int8_t>) {
      return static_cast<int8_t>(*this);
   }
   else if constexpr (std::is_same_v<TYPE, uint8_t>) {
      return static_cast<uint8_t>(*this);
   }
   else if constexpr (std::is_same_v<TYPE, int16_t>) {
      return static_cast<int16_t>(*this);
   }
   else if constexpr (std::is_same_v<TYPE, uint16_t>) {
      return static_cast<uint16_t>(*this);
   }
   else if constexpr (std::is_same_v<TYPE, int32_t> || std::is_same_v<TYPE, int>) {
      return get_int();
   }
   else if constexpr (std::is_same_v<TYPE, uint32_t> || std::is_same_v<TYPE, unsigned int>) {
      return get_uint();
   }
   else if constexpr (std::is_same_v<TYPE, int64_t>) {
      return get_int64();
   }
   else if constexpr (std::is_same_v<TYPE, uint64_t>) {
      return get_uint64();
   }
   else if constexpr (std::is_same_v<TYPE, float>) {
      return static_cast<float>(get_double());
   }
   else if constexpr (std::is_same_v<TYPE, double>) {
      return get_double();
   }
   else if constexpr (std::is_same_v<TYPE, std::string>) {
      return get_string();
   }
   else if constexpr (std::is_same_v<TYPE, std::string_view>) {
      return get_string_view();
   }
   else if constexpr (std::is_same_v<TYPE, std::wstring>) {
      return static_cast<std::wstring>(*this);
   }
   else if constexpr (std::is_same_v<TYPE, const char*>) {
      return static_cast<const char*>(*this);
   }
   else if constexpr (std::is_same_v<TYPE, const wchar_t*>) {
      return static_cast<const wchar_t*>(*this);
   }
}


/// assign arguments with vector containing gd::variant_view values
inline arguments& arguments::operator=( const std::vector<gd::variant_view>& vectorValue)
{
   clear();
   return append(vectorValue);
}
/// assign arguments with vector containing pair of string and gd::variant values
inline arguments& arguments::operator=(const std::initializer_list<std::pair<std::string_view, gd::variant>>& listPair)
{
   clear();
   for( const auto& it : listPair ) append_argument(it);
   return *this;
}

/// assign arguments with vector containing pair of string and gd::variant values
inline arguments& arguments::operator=(const std::initializer_list<std::pair<std::string_view, gd::variant_view>>& listPair)
{
   clear();
   for( const auto& it : listPair ) append_argument(it, tag_view{});
   return *this;
}

/// assign arguments with vector containing pair of string and gd::variant_view values
inline arguments& arguments::operator=( const std::vector<std::pair<std::string_view, gd::variant_view>>& vectorPair)
{
   clear();
   return append(vectorPair);
}

/// extract argument value into arg_view object
inline arguments& arguments::operator>>( gd::arg_view& arg_ ) 
{ 
   if( exists(arg_.get_key()) == true ) { 
      arg_.set_value( this->get_argument(arg_.get_key()).as_variant_view() ); 
   }
   return *this; 
}


/// append values from vector with variant_view items
inline arguments& arguments::append( const std::vector<gd::variant_view>& vectorValue ) {
   for( const auto& it : vectorValue  ) append( it, tag_view{} );
   return *this;
}

/// append values from vector with pairs of string_view items
inline arguments& arguments::append( const std::vector<std::pair<std::string_view, std::string_view>>& vectorStringValue ) {
   for( auto it : vectorStringValue ) append( it.first, it.second );
   return *this;
}

/// append values from vector with pairs of string items
inline arguments& arguments::append( const std::vector<std::pair<std::string, std::string>>& vectorStringValue ) {
   for( auto it : vectorStringValue ) append( it.first, it.second );
   return *this;
}

/// append values from vector with variant items
inline arguments& arguments::append( const std::vector<std::pair<std::string,gd::variant>>& vectorStringVariant ) {
   for( auto it : vectorStringVariant ) append_argument( it.first, it.second );
   return *this;
}

/// append values from vector with pair of string_view and variant_view items
inline arguments& arguments::append(const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorStringVariantView ) {
   for( auto it : vectorStringVariantView ) append_argument(it.first, it.second);
   return *this;
}


/// append values from vector with variant items
inline arguments& arguments::append( const std::vector<std::pair<std::string_view,std::string_view>>& vectorStringValue, tag_parse_type ) {
   for( auto it : vectorStringValue ) append_argument( it.first, it.second, tag_parse_type{} );
   return *this;
}

inline arguments& arguments::append( const std::vector<std::pair<std::string,std::string>>& vectorStringValue, tag_parse_type ) {
   for( auto it : vectorStringValue ) append_argument( it.first, it.second, tag_parse_type{} );
   return *this;
}

inline arguments& arguments::append(const std::string_view& stringName, const std::vector<argument>& vectorArgument) {
   for( const auto& it : vectorArgument ) append_argument(stringName, it);
   return *this;
}

inline arguments& arguments::append(const std::string_view& stringName, const std::vector<gd::variant_view>& vectorArgument) {
   for( const auto& it : vectorArgument ) append_argument(stringName, it);
   return *this;
}

/**
 * @brief Appends arguments from another arguments object whose names are present in the specified list.
 * @tparam ARGUMENTS The type representing the arguments container.
 * @param arguments_ The arguments object to extract and append arguments from.
 * @param list_ A list of argument names to filter which arguments to append.
 * @return A reference to the modified arguments object (*this) after appending the selected arguments.
 *
 * @code
// Example adding argument values from gd::argument::arguments to gd::argument::shared::arguments
gd::argument::arguments arguments_;
arguments_.append("arg1", 42);
arguments_.append("arg2", "Hello");
gd::argument::shared::arguments argumentsNew;
argumentsNew.append(arguments_, { "arg1", "arg2" });
 * @endcode
 */
template<typename ARGUMENTS>
inline arguments& arguments::append(const ARGUMENTS& arguments_, const std::initializer_list<std::string_view>& list_) {
   for( auto pPosition = arguments_.next(); pPosition != nullptr; pPosition = arguments_.next(pPosition) ) {
      if( ARGUMENTS::is_name_s(pPosition) == true ) {                          // check if position is name
         auto stringName = ARGUMENTS::get_name_s(pPosition);                   // get name from position
         if( std::find(list_.begin(), list_.end(), stringName) != list_.end() ) {
            auto value_ = ARGUMENTS::get_argument_s(pPosition);                // get argument value
            append_argument(stringName, value_.as_variant_view());             // append argument value
         }
      }
   }
   return *this;
}

/// specialization for vector of pairs with string_view and variant_view
template<>
inline arguments& arguments::append<std::vector<std::pair<std::string_view, gd::variant_view>>>( const std::vector<std::pair<std::string_view, gd::variant_view>>& arguments_, const std::initializer_list<std::string_view>& list_) {
   for(const auto& [name, value] : arguments_) {
      if(std::find(list_.begin(), list_.end(), name) != list_.end()) {
         append_argument(name, value);
      }
   }
   return *this;
}



/// appends value if it is true (true, valid pointer, non 0 value for numbers, non empty strings)
template<typename VALUE>
inline arguments& arguments::append_if(const std::string_view& stringName, VALUE value ) {
   if constexpr( std::is_pointer_v<VALUE> ) {
      if( value == nullptr ) return *this;
   }
   argument argumentValue( value );
   if( argumentValue.is_true() )
   {
      return append_argument(stringName, argumentValue);
   }
   return *this;
}

/// append object, object need to implement `to_values` and static member called `to_member_name`
template<typename OBJECT>
inline arguments& arguments::append_object(const std::string_view& stringPrefixFind, const OBJECT object_) {
   std::vector< gd::variant_view > vectorObject;
   object_.to_values( vectorObject );
   for(size_t uIndex = 0, uMax = vectorObject.size(); uIndex < uMax; uIndex++) {
      std::string stringName = OBJECT::to_member_name( uIndex, stringPrefixFind );
      append_argument( stringName, vectorObject.at( uIndex ) );
   }
   return *this;
}

template<typename OBJECT>
inline void arguments::get_object(const std::string_view& stringPrefixFind, OBJECT& object_) {
   std::vector< gd::variant_view > vector_;
   for(unsigned uIndex = 0, uMax = OBJECT::to_member_count(); uIndex < uMax; uIndex++) {
      std::string stringName = OBJECT::to_member_name( uIndex, stringPrefixFind );
      vector_.push_back( get_argument( stringName ).as_variant_view() );
   }
   object_ = vector_;
}

template<typename OBJECT>
inline OBJECT arguments::get_object() {
   OBJECT object_;
   get_object( object_ );
   return object_;
}

template<typename OBJECT>
inline OBJECT arguments::get_object( const std::string_view& stringPrefixFind ) {
   OBJECT object_;
   get_object( stringPrefixFind, object_ );
   return object_;
}

#ifndef NDEBUG
inline bool arguments::verify_d(const_pointer pPosition) const {
   bool bOk = ( pPosition >= buffer_data() );
   if( bOk == true ){
      bOk = ( pPosition <= buffer_data_end() );
   }
   return bOk;
}
#endif

// ================================================================================================
// ================================================================================= arguments_return
// ================================================================================================

/**
 * \brief simplifies using type deduction to return value as a pair
 *
 * arguments_return is just to simplify how to write code returning values.
 * constructing `arguments` needs two "{{ }}" and �arguments_return� only needs one like {}
 *
 \code
 // sample on how to return
 return { "return", true }
 \endcode
 */
class arguments_return : public arguments
{
public:
   arguments_return( std::pair<std::string_view, gd::variant> pairArgument ) {          // construct with pair (useful returning arguments from function)
      zero();
      append_argument(pairArgument);
   }
};

constexpr uint8_t ctype_size[arguments::CType_MAX] = {
   0,       // eTypeNumberUnknown = 0,
   1,       // eTypeNumberBool = 1,
   1,       // eCTypeNumberInt8 = 2,
   1,       // eCTypeNumberUInt8,
   2,       // eCTypeNumberInt16,
   2,       // eCTypeNumberUInt16,
   4,       // eCTypeNumberInt32,
   4,       // eCTypeNumberUInt32,
   8,       // eCTypeNumberInt64,
   8,       // eCTypeNumberUInt64,

   sizeof(float), // eTypeNumberFloat,
   sizeof(double),// eTypeNumberDouble,

   sizeof(void*), //eTypeNumberPointer,
   16,      // eTypeNumberGuid
};

/// Append named `argument`
inline arguments& arguments::append_argument(const std::string_view& stringName, argument argumentValue) {
   auto _l = argumentValue.length();
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   return append(stringName, argumentValue.ctype(), pData, argumentValue.length());
}

/// set value from variant_view at position
inline arguments& arguments::set(pointer pPosition, const gd::variant_view& variantValue, pointer* ppPosition) {
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();
   unsigned uLength;
   if( uType > ARGUMENTS_NO_LENGTH )
   {
      uLength = variantValue.length() + get_string_zero_terminate_length_s( uType );
      uType |= eValueLength;
   }
   else
   {
      uLength = ctype_size[uType];
   }
   return set(pPosition, uType, pData, uLength, ppPosition);
}

/// set value from variant_view for named argument
inline arguments& arguments::set(std::string_view stringName, const gd::variant_view& variantValue) {
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();                               // get full type, including length
   unsigned uLength;
   if( uType > ARGUMENTS_NO_LENGTH )
   {
      uType |= eValueLength;
      uLength = variantValue.length() + get_string_zero_terminate_length_s( uType );
   }
   else
   {
      uLength = ctype_size[uType];
   }
   return set(stringName, uType, pData, uLength);
}


/// return argument object that can be used to edit value
inline arguments::argument_edit arguments::find_edit_argument(std::string_view stringName) {
   pointer pPosition = find(stringName);
   if( pPosition ) return get_edit_param_s(this, pPosition);
   return argument_edit();
}

/// Get argument value and start from offset position in buffer, use this if there are multiple values with same name
inline arguments::argument arguments::find_argument(const std::string_view& stringName, const_pointer pPosition) const {
   pPosition = find(stringName, pPosition);
   if( pPosition ) return get_argument_s(pPosition);
   return argument();
}

/// check if argument exists among values in arguments object
inline bool arguments::exists( const std::string_view& stringName ) const {
   const_pointer pPosition = find(stringName);
   return pPosition != nullptr;
}

/// check if all with name in list exists among values in arguments object, if not found return false
inline bool arguments::exists( const std::initializer_list<std::string_view>& listName ) const {
   for( const auto& stringName : listName ) {
      if( exists(stringName) == false ) return false; // if one is not found return false
   }
   return true; // all names found
}

/// check if any of the names in list exists among values in arguments object, if one is found return true
inline bool arguments::exists_any( const std::initializer_list<std::string_view>& listName ) const {
   for( const auto& stringName : listName ) {
      if( exists(stringName) == true ) return true; // if one is found return true
   }
   return false; // no names found
}


/// compare if value is equal for specified name
/// comparing text is faster with this internal compare method because no object is created on heap holding argument value
inline bool arguments::compare(const std::string_view& stringName, const arguments& argumentsCompareTo) const {
   const argument argumentCompare = get_argument(stringName);
   if( argumentCompare.is_null() ) return false;
   const argument argumentCompareTo = argumentsCompareTo.get_argument(stringName);
   return compare_argument_s(argumentCompare, argumentCompareTo);
}

// ================================================================================================
// =============================================================================== GLOBAL FUNCTIONS
// ================================================================================================

template< typename TYPE>
TYPE get_g( const arguments::argument& argument_ )
{
   return argument_.template get<TYPE>();
}


// ================================================================================================
// ======================================================================================= ITERATOR
// ================================================================================================


/**
 * @brief Enhanced iterator for arguments class that provides easy access to both names and values
 *
 * This iterator extends the functionality of the existing iterator_ class to provide
 * better access to argument names and supports structured bindings for convenient
 * iteration over name-value pairs.
 */
template<typename ARGUMENTS>
struct iterator_named
{
   using value_type = std::pair<std::string_view, arguments::argument>;
   using iterator_category = std::forward_iterator_tag;
   using self = iterator_named;
   using difference_type = std::ptrdiff_t;
   using pointer = const value_type*;
   using reference = const value_type&;

   // Constructors
   iterator_named() : m_parguments(nullptr), m_uPosition(0) {}
   iterator_named(const ARGUMENTS* parguments) : m_parguments(parguments), m_uPosition(0) {}
   iterator_named(const ARGUMENTS* parguments, size_t uPosition) : m_parguments(parguments), m_uPosition(uPosition) {}
   iterator_named(const iterator_named& o) : m_parguments(o.m_parguments), m_uPosition(o.m_uPosition) {}

   // Assignment
   iterator_named& operator=(const iterator_named& o) {
      m_parguments = o.m_parguments;
      m_uPosition = o.m_uPosition;
      return *this;
   }

   // Comparison operators
   bool operator==(const self& o) const {                                                          assert(m_parguments == o.m_parguments);
      return m_uPosition == o.m_uPosition;
   }
   bool operator!=(const self& o) const { return !( *this == o ); }
   bool operator>(const self& o) const { return m_uPosition > o.m_uPosition; }
   bool operator<(const self& o) const { return m_uPosition < o.m_uPosition; }

   // Conversion operators
   operator const ARGUMENTS* () const { return m_parguments; }
   operator arguments::const_pointer() const { return buffer_offset(); }

   // Dereference operators - returns name-value pair
   value_type operator*() const {                                                                  assert(m_parguments->verify_d(buffer_offset()));
      auto position_ = buffer_offset();
      auto name = ARGUMENTS::is_name_s(position_) ? ARGUMENTS::get_name_s(position_) : std::string_view{};
      auto arg = ARGUMENTS::get_argument_s(position_);
      return std::make_pair(name, arg);
   }

   // For pointer-like access
   struct proxy {
      value_type pair;
      proxy(value_type p) : pair(std::move(p)) {}
      const value_type* operator->() const { return &pair; }
   };

   proxy operator->() const { return proxy(operator*()); }

   // Increment operators
   self& operator++() {
      assert(m_parguments->verify_d(buffer_offset()));
      m_uPosition = arguments::next_s(m_parguments->buffer_data(), m_uPosition);                   assert(m_parguments->verify_d(buffer_offset()));
      return *this;
   }

   self operator++(int) {                                                                          assert(m_parguments->verify_d(buffer_offset()));
      iterator_named it = *this;
      ++( *this );
      return it;
   }

   // Compound assignment operator
   self& operator+=(size_t uCount) {
      for( size_t i = 0; i < uCount; ++i ) { ++( *this ); }
      return *this;
   }

   // Addition operator
   self operator+(size_t uCount) const {
      iterator_named it = *this;
      for( size_t i = 0; i < uCount; ++i ) ++it;
      return it;
   }

   /// Type number for current argument
   unsigned type_number() const { return ARGUMENTS::type_s(buffer_offset()); }

   // Is current value named
   bool is_name() const {                                                                          assert(m_parguments->verify_d(buffer_offset()));
      return ARGUMENTS::is_name_s(buffer_offset());
   }

   /// Get name as std::string_view
   std::string_view name() const {                                                                 assert(m_parguments->verify_d(buffer_offset()));
      if( ARGUMENTS::is_name_s(buffer_offset()) ) { return ARGUMENTS::get_name_s(buffer_offset()); }
      return std::string_view{};
   }

   /// Get name as std::string
   std::string name_string() const { auto name_ = name(); return std::string(name_); }

   bool compare_name(const std::string_view& stringName) const {
      if( ARGUMENTS::is_name_s(buffer_offset()) ) {
         return ARGUMENTS::get_name_s(buffer_offset()) == stringName;
      }
      return false;
   }

   // Value access methods
   arguments::argument value() const {
      assert(m_parguments->verify_d(buffer_offset()));
      return ARGUMENTS::get_argument_s(buffer_offset());
   }

   arguments::argument get_argument() const {
      return value();
   }

   // Convenience methods for common value types
   template<typename TYPE>
   TYPE get() const {
      return value().template get<TYPE>();
   }

   bool as_bool() const { return value().as_bool(); }
   int as_int() const { return value().as_int(); }
   uint32_t as_uint() const { return value().as_uint(); }
   int64_t as_int64() const { return value().as_int64(); }
   uint64_t as_uint64() const { return value().as_uint64(); }
   double as_double() const { return value().get_double(); }
   std::string as_string() const { return value().as_string(); }
   gd::variant_view as_variant_view() const { return value().as_variant_view(); }

   // Structured binding support (for C++17 and later)
   template<std::size_t uIndex>
   auto get() const {
      static_assert( uIndex < 2, "Index must be 0 (name) or 1 (value)" );
      if constexpr( uIndex == 0 ) return name();
      if constexpr( uIndex == 1 ) return value();
   }

   /// Internal buffer access
   arguments::const_pointer buffer_offset() const { arguments::const_pointer pposition_ = m_parguments->buffer_offset(m_uPosition); return pposition_; }
   /// Get current position in buffer
   size_t position() const { return m_uPosition; }
   /// Get pointer to arguments container
   const ARGUMENTS* container() const { return m_parguments; }

   // Validation
   bool is_valid() const {
      return m_parguments != nullptr &&
         m_uPosition <= m_parguments->buffer_size() &&
         ( m_uPosition == m_parguments->buffer_size() ||
            m_parguments->verify_d(buffer_offset()) );
   }

public:
   const ARGUMENTS* m_parguments; ///< Pointer to the arguments container
   size_t m_uPosition; ///< Current position in the buffer
};

// Type aliases for convenience
using named_iterator_t = iterator_named<arguments>;
using const_named_iterator = iterator_named<const arguments>;

// Helper functions to create named iterators
template<typename ARGUMENTS>
iterator_named<ARGUMENTS> make_iterator_named(const ARGUMENTS* arguments_) { return iterator_named<ARGUMENTS>(arguments_); }

template<typename ARGUMENTS>
iterator_named<ARGUMENTS> make_iterator_named(const ARGUMENTS* args, size_t position) { return iterator_named<ARGUMENTS>(args, position); }

// ## named iterator methods

inline arguments::named_iterator_t arguments::named_begin() {
   return named_iterator_t(this);
}

inline arguments::named_iterator_t arguments::named_end() {
   return named_iterator_t(this, m_uLength);
}

inline arguments::const_named_iterator arguments::named_begin() const {
   return const_named_iterator(this);
}

inline arguments::const_named_iterator arguments::named_end() const {
   return const_named_iterator(this, m_uLength);
}

inline arguments::const_named_iterator arguments::named_cbegin() const {
   return const_named_iterator(this);
}

inline arguments::const_named_iterator arguments::named_cend() const {
   return const_named_iterator(this, m_uLength);
}


// ================================================================================================
// ================================================================================= FREE FUNCTIONS
// ================================================================================================


constexpr std::string_view arguments::type_name_s(uint32_t uType)
{
   const auto uNumberType = uType &  ~arguments::eTypeNumber_MASK;
   switch( uNumberType )
   {
   case arguments::eTypeNumberUnknown     : return "unknown";
   case arguments::eTypeNumberBool        : return "bool";
   case arguments::eTypeNumberInt8        : return "int8";
   case arguments::eTypeNumberUInt8       : return "uint8";
   case arguments::eTypeNumberInt16       : return "int16";
   case arguments::eTypeNumberUInt16      : return "uint16";
   case arguments::eTypeNumberInt32       : return "int32";
   case arguments::eTypeNumberUInt32      : return "uint32";
   case arguments::eTypeNumberInt64       : return "int64";
   case arguments::eTypeNumberUInt64      : return "uint64";
   case arguments::eTypeNumberFloat       : return "float";
   case arguments::eTypeNumberDouble      : return "double";
   case arguments::eTypeNumberPointer     : return "pointer";
   case arguments::eTypeNumberGuid        : return "guid";
   case arguments::eTypeNumberString      : return "ascii";
   case arguments::eTypeNumberUtf8String  : return "utf8";
   case arguments::eTypeNumberWString     : return "unicode";
   case arguments::eTypeNumberUtf32String : return "utf32";
   case arguments::eTypeNumberBinary      : return "binary";
   default:                                                                      assert(false);
      return "ERROR";
   }
}

_GD_ARGUMENT_END

_GD_ARGUMENT_BEGIN

// @DEBUG @API [tag: arguments, debug] [summary: Print arguments structure and data to string for debug purposes]

namespace debug {
   std::string print( const arguments::argument& argumentToPrint );
   std::string print( const arguments& argumentsToPrint );
   std::string print( const arguments& argumentsToPrint, const std::string_view& stringSeparator );
   std::string print( const std::vector<arguments>& vectorToPrint );
   //std::string print( const std::vector<arguments>* vectorToPrint );
}
_GD_ARGUMENT_END


#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
