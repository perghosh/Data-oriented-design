/**
 * \file gd_arguments.h
 * 
 * \brief Pack primitive value and some common derived values in byte buffer. 
 * 
 */



#pragma once
#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_variant.h"
#include "gd_variant_view.h"


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



#ifndef _GD_ARGUMENT_SHARED_BEGIN
#define _GD_ARGUMENT_SHARED_BEGIN namespace gd::argument::shared {
#define _GD_ARGUMENT_SHARED_END }
_GD_ARGUMENT_SHARED_BEGIN
#else
_GD_ARGUMENT_SHARED_BEGIN
#endif

struct tag_list {};                                                            ///< operations that use some sort of container class in stl  
struct tag_memory {};                                                          ///< logic around memory
struct tag_pair {};                                                            ///< tag dispatcher used to select working with pair items instead of vector
struct tag_parse {};                                                           ///< methods that parse
struct tag_parse_type{};                                                       ///< tag to try to parse type of value
struct tag_align{};                                                            ///< align related methods



// ================================================================================================
// ================================================================================= arguments
// ================================================================================================


/**
 * \brief arguments in shared namespace focus on performance and arguments holds a reference counter
 *
 * If you need to store a lot of arguments objects or need store large amount of data then
 * the arguments in shared namespace works better compared to arguments found in argument namespace.
 * 
 * ## memory layout
 * [type and length for name][name in chars][type and length for data]{[length for non primitive types]}[value data]
 * shorter version
 * [uint32][name][uint32]{[uint32]}[data]
 * 
 * Values are store in one single buffer, and each value know its type and the length for the value is also known before value data is found.
 * Because lengths are stored it is fast to move between values in arguments object. Also the data length is
 * stored for the specific type in order to generate proper object value for type.
 * Example: strings need to store the zero ending, but that isn't used to get the
 * string lenght. so data length for "123" is four bytes becuase zero ending is stored.
 * But the value is prefixed with length that matches date and there fore the value 3 is stored in front of "123".
 * 
 * Example: Show memory layout for named char* value
 * - type and length for name                                    : 01 02 03 04
 * - name data                                                   : number of bytes needed to store name
 * - align with 32 bit value
 * - type and length for value                                   : 01 02 03 04
 * - value data                                                  : number of bytes needed to store value
 * - align with 32 bit value
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
public:
   typedef uint8_t            value_type;
   typedef value_type*        pointer;
   typedef const value_type*  const_pointer;
   typedef value_type&        reference;
   typedef const value_type&  const_reference;
   typedef size_t             size_type;
   typedef ptrdiff_t          difference_type;

   typedef uint8_t            param_type;
   typedef uint8_t            argument_type;

   struct view_tag {};                                                         // tag dispatcher used when working with view objects (not owning its data)
   struct tag_view {};                                                         // tag dispatcher used when working with view objects (not owning its data)
   struct tag_no_initializer_list {};                                          // do not select initializer_list versions
   struct tag_name {};                                                         // tag dispatcher for name related operations
   struct tag_description {};                                                  // tag dispatcher where description is usefull


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

      CItem_MAX,

      eValueName = 0b00100000,
      eValueLength = 0b01000000,
      eValueLengthBig = 0b10000000,

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

   /**
    * \brief
    *
    *
    */
   struct buffer
   {
      // ## construction -------------------------------------------------------------

      buffer(): m_uSize( 0 ), m_uBufferSize( 0 ), m_iReferenceCount( 1 ) {}
      buffer( uint64_t uSize, uint64_t uBufferSize ): m_uSize( uSize ), m_uBufferSize( uBufferSize ), m_iReferenceCount( 1 ) {}
      ~buffer() {}

      uint64_t size() const { return m_uSize; }
      void size( uint64_t uSize ) { assert( uSize <= m_uBufferSize ); m_uSize = uSize; }
      uint64_t buffer_size() const { return m_uBufferSize; }
      void buffer_size( uint64_t uBufferSize ) { m_uBufferSize = uBufferSize; }
      uint8_t* data() const { return (uint8_t*)(this) + sizeof(buffer); }

      int get_reference_count() const { return m_iReferenceCount; }
      int add_reference() { m_iReferenceCount++; return m_iReferenceCount; }
      void release() {                                                                             assert( m_iReferenceCount > 0 ); assert( this != &m_buffer_s );
         m_iReferenceCount--;
         if(m_iReferenceCount == 0)
         {
            delete [] (uint8_t*)this;
         }
      }

      // ## attributes
      uint64_t m_uSize;             ///< used size in buffer
      uint64_t m_uBufferSize;       ///< total buffer size
      int      m_iReferenceCount;
   };




   struct argument
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


      const argument& operator>>(int& v) const { v = get_int(); return *this; }
      const argument& operator>>(unsigned int& v) const { v = get_uint(); return *this; }
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

      /// size in bytes for value in arguments
      unsigned int size() const;
      /// native length param
      unsigned int length() const;
      /// get param type
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

      unsigned int as_uint() const { return get_uint(); }
      int          as_int() const { return get_int(); }
      int64_t      as_int64() const { return get_int64(); }
      uint64_t     as_uint64() const { return get_uint64(); }
      std::string  as_string() const { return get_string(); };
      std::string  as_utf8() const { return get_utf8(); };
      gd::variant  as_variant() const { return get_variant(); }
      gd::variant_view as_variant_view() const { return get_variant_view(); }
      std::string_view as_string_view() const { return get_string_view(); }


      int          get_int() const;
      bool         get_bool() const { return get_int(); };
      unsigned int get_uint() const;
      int64_t      get_int64() const;
      uint64_t     get_uint64() const;
      double       get_double() const;
      std::string  get_string() const;
      std::string  get_utf8() const;
      gd::variant  get_variant() const { return arguments::get_variant_s(*this); }
      gd::variant_view  get_variant_view() const { return arguments::get_variant_view_s(*this); }
      gd::variant  get_variant( bool ) const { return arguments::get_variant_s(*this, false); } /// for speed, do not copy data
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
         memcpy(this, &o, sizeof(argument_edit));
      }

      argument_edit& operator=(argument argumentSet) { set(argumentSet); return *this; }


      template<typename ARGUMENT_TYPE>
      argument_edit(arguments* parguments, arguments::const_pointer pPosition, ARGUMENT_TYPE AG): m_pArguments(parguments), m_pPosition(pPosition), argument( AG ) {
         m_pValue = move_to_value_s( (pointer)pPosition );
      }

      template<typename TYPE>
      bool operator==(TYPE v) const { return compare_argument_s(*this, gd::variant_view( v ) ); }

      void set(argument argumentSet);

      arguments* m_pArguments;
      arguments::const_pointer m_pPosition;
      arguments::pointer m_pValue;
   };


   /**
    * @brief iterator for iterating values in params object.
   */
   struct const_iterator
   {
      typedef const_iterator     self;

      const_iterator() : m_parguments(nullptr), m_pPosition(nullptr) {}
      const_iterator(arguments::const_pointer pPosition ): m_pPosition(pPosition) {}
      const_iterator(const arguments* pparams, arguments::const_pointer pPosition) : m_parguments(pparams), m_pPosition(pPosition) {}
      const_iterator(const const_iterator& o) { m_parguments = o.m_parguments; m_pPosition = o.m_pPosition; }
      const_iterator& operator=(const const_iterator& o) { m_parguments = o.m_parguments; m_pPosition = o.m_pPosition; return *this; }

      argument operator*() const {                                                                 assert( m_parguments->verify_d( m_pPosition ));
         return get_argument();
      }
      self& operator++() {                                                                         assert( m_parguments->verify_d( m_pPosition ));
         m_pPosition = m_parguments->next(m_pPosition); return *this;
      }
      self& operator++(int) {                                                                      assert( m_parguments->verify_d( m_pPosition ));
         m_pPosition = m_parguments->next(m_pPosition); return *this;
      }
      bool operator==(const_iterator& o) { return m_pPosition == o.m_pPosition; }
      bool operator!=(const_iterator& o) { return m_pPosition != o.m_pPosition; }

      operator const arguments*() const { return m_parguments; }
      operator arguments::const_pointer() const { return m_pPosition; }

      std::string name() const {                                                                   assert( m_parguments->verify_d( m_pPosition ));
         if( arguments::is_name_s(m_pPosition) == true )
         {
            return std::string(arguments::get_name_s(m_pPosition));
         }
         return std::string();
      }

      std::string_view name(view_tag) const {                                                      assert( m_parguments->verify_d( m_pPosition ));
         if( arguments::is_name_s(m_pPosition) == true )
         {
            return arguments::get_name_s(m_pPosition);
         }
         return std::string_view();
      }

      bool compare_name(std::string_view stringName) const { 
         if( arguments::is_name_s(m_pPosition) == true )
         {
            if( arguments::get_name_s(m_pPosition) == stringName ) return true;
         }
         return false;
      }

      argument get_argument() {
         auto v_ =arguments::get_argument_s(m_pPosition);
         return v_;
      }
      const argument get_argument() const { 
         auto v_ =arguments::get_argument_s(m_pPosition);
         return v_;
      }

      template<std::size_t uIndex>
      auto get() const
      {
         static_assert(uIndex < 2, "Allowed index are 0 and 1, above is not valid");
         if constexpr( uIndex == 0 ) return name();
         if constexpr( uIndex == 1 ) return get_argument();
      }


      // attributes
   public:
      const shared::arguments* m_parguments;
      arguments::const_pointer m_pPosition;
   };


   // ## construction -------------------------------------------------------------
public:
   arguments() {}

   /** Set buffer and size, use this to avoid heap allocations (if internal data grows over buffer size you will get heap allocation)  */
   arguments(const std::string_view& stringName, const gd::variant& variantValue, tag_no_initializer_list );


   arguments(std::pair<std::string_view, gd::variant> pairArgument);
   template <typename... Arguments>
   arguments(std::pair<std::string_view, gd::variant> pairArgument, Arguments... arguments)
   {
      zero();
      auto _argument = get_argument_s(pairArgument.second);
      append_argument(pairArgument.first, _argument);
      append_argument(arguments...);
   }
   arguments(std::initializer_list<std::pair<std::string_view, gd::variant>> listPair); // construct arguments with vector like {{},{}}
   arguments(std::initializer_list<std::pair<std::string_view, gd::variant_view>> listPair, tag_view ); // light weight version to construct arguments with vector like {{},{}}
   arguments(std::vector<std::pair<std::string_view, gd::variant_view>> listPair, tag_view ); // light weight version to construct arguments with vector like {{},{}}   

   // copy
   arguments(const arguments& o) { common_construct(o); }
   arguments(arguments&& o) noexcept { common_construct((arguments&&)o); }
   // assign
   arguments& operator=(const arguments& o) { common_construct(o); return *this; }
   arguments& operator=(arguments&& o) noexcept { common_construct(o); return *this; }

   arguments& operator=(std::initializer_list<std::pair<std::string_view, gd::variant>> listPair);

   ~arguments() { 
      buffer_delete();
      //if( m_bOwner ) delete[] m_pBuffer;
   }
protected:
   // common copy
   void common_construct(const arguments& o) {
      if( is_null() == false) m_pbuffer->release();
      if( o.is_null() == false )
      {
         m_pbuffer = o.m_pbuffer;
         m_pbuffer->add_reference();
      }
      else
      {
         m_pbuffer = &m_buffer_s;
      }
   }

   void common_construct(arguments&& o) noexcept {
      if( is_null() == false) m_pbuffer->release();
      m_pbuffer->release();
      m_pbuffer = o.m_pbuffer;
      o.m_pbuffer = &m_buffer_s;
   }

   void zero() { release(); };
   void release() { if( is_null() == false ) { m_pbuffer->release(); m_pbuffer = &m_buffer_s; } }
   bool is_null() const { return m_pbuffer == &m_buffer_s; }

   // ## operator -----------------------------------------------------------------
public:
   argument operator[](unsigned uIndex) { return get_argument(uIndex); }
   argument operator[](std::string_view stringName) { return get_argument(stringName); }
   argument operator[](arguments::const_pointer p) { return get_argument(p); }
   const argument operator[](unsigned uIndex) const { return get_argument(uIndex); }
   const argument operator[](std::string_view stringName) const { return get_argument(stringName); }
   const argument operator[](arguments::const_pointer p) const { return get_argument(p); }

   argument_edit operator()(unsigned uIndex) {
      const_pointer pPosition = find(uIndex);
      if( pPosition != nullptr ) { return arguments::get_edit_param_s(this, pPosition); }
      return argument_edit();
   }

   argument_edit operator()(std::string_view stringName) { 
      const_pointer pPosition = find(stringName);
      if( pPosition != nullptr ) { return arguments::get_edit_param_s(this, pPosition); }
      return argument_edit();
   }

   std::pair<std::string_view,gd::variant_view> operator()( unsigned uIndex, tag_pair ) const {
      const_pointer pPosition = find(uIndex);
      if( pPosition != nullptr ) { return { get_name_s(pPosition), get_argument_s(pPosition) }; }
      return { "", gd::variant_view() };
   }

   arguments& operator+=( const std::pair<std::string_view, gd::variant_view>& pairArgument ) { return append_argument( pairArgument, tag_view{} ); }

   arguments operator<<(const std::pair<std::string_view, gd::variant_view>& pairArgument ) { return append_argument(pairArgument, tag_view{}); }

   /// Append values from another arguments object
   arguments& operator+=( const arguments& arguments_ ) { return append( arguments_ ); }


   // ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
/// return start position to buffer where values are stored
   pointer get_buffer_start() { return m_pbuffer->data(); }
   const_pointer get_buffer_start() const { return m_pbuffer->data(); }
   /// return last position for buffer where values are stored
   pointer get_buffer_end() { return m_pbuffer->data() + m_pbuffer->size(); }
   const_pointer get_buffer_end() const { return m_pbuffer->data() + m_pbuffer->size(); }
//@}

/** \name OPERATION
*///@{

   // return if object owns memory, if it does it should be deleted when arguments goes out of scope
   bool is_owner() const noexcept { return m_bOwner; }

   // ## append adds values to stream
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
   arguments& append( const std::vector<std::pair<std::string_view,std::string_view>>& vectorStringValue );
   arguments& append( const std::vector<std::pair<std::string,std::string>>& vectorStringValue );
   arguments& append( const std::vector<std::pair<std::string,gd::variant>>& vectorStringVariant );
   arguments& append( const std::vector<std::pair<std::string_view,std::string_view>>& vectorStringValue, tag_parse_type );
   arguments& append( const std::vector<std::pair<std::string,std::string>>& vectorStringValue, tag_parse_type );
   std::pair<bool, std::string>  append( const std::string_view& stringValue, tag_parse );

   /// Append named `argument`
   arguments& append_argument(const std::string_view& stringName, argument argumentValue);

   arguments& append_argument(const variant& variantValue);

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

   arguments& append_argument(const std::pair<std::string_view, gd::variant_view>& pairArgument) {
      return append_argument(pairArgument.first, pairArgument.second );
   }
   arguments& append_argument(const std::pair<std::string_view, gd::variant_view>& pairArgument, tag_view) {
      return append_argument(pairArgument.first, pairArgument.second );
   }
   arguments& append_argument(const std::string_view stringName, const std::string_view& stringValue, tag_parse_type );

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
   arguments& set(const std::string_view& stringName, const char* v) { return set(stringName, std::string_view(v) ); }
   arguments& set_uuid(const std::string_view& stringName, const uint8_t* puData) { return set(stringName, eTypeNumberGuid, (const_pointer)puData, 16); }


   // uuid
   arguments& set(std::string_view stringName, std::string_view v) { return set(stringName, (eTypeNumberString | eValueLength), (const_pointer)v.data(), (unsigned int)v.length() + 1); }

   arguments& set(std::string_view stringName, const gd::variant_view& variantValue);

   arguments& set(std::string_view stringName, param_type uType, const_pointer pBuffer, unsigned int uLength) { return set(stringName.data(), (uint32_t)stringName.length(), uType, pBuffer, uLength); }
   arguments& set(const char* pbszName, uint32_t uNameLength, param_type uType, const_pointer pBuffer, unsigned int uLength);
   arguments& set(pointer pPosition, param_type uType, const_pointer pBuffer, unsigned int uLength);

   // TODO: Implement set methods

   const_iterator begin() const { return const_iterator( this, buffer_data() ); }
   const_iterator end() const { return const_iterator( nullptr ); }

   [[nodiscard]] uint64_t capacity() const { return buffer_buffer_size(); }

/** \name COUNT
*///@{
   bool empty() const noexcept { return m_pbuffer->size() == 0; }
   size_t size( tag_memory ) const noexcept { return buffer_size(); }
   unsigned int count(std::string_view stringName) const;
//@}

/** \name FIND
* Find methods, finds position or param value for name
*///@{
   [[nodiscard]] pointer find(unsigned int uIndex);
   [[nodiscard]] const_pointer find(unsigned int uIndex) const;
   [[nodiscard]] pointer find(const std::string_view& stringName);
   [[nodiscard]] const_pointer find(const std::string_view& stringName) const;
   [[nodiscard]] const_pointer find(std::string_view stringName, const_pointer pPosition) const;
   [[nodiscard]] const_pointer find(const std::pair<std::string_view, gd::variant_view>& pairMatch) const;

   [[nodiscard]] std::pair<argument,argument> find_pair(const std::string_view& stringName) const;

   std::vector<const_pointer> find_all(std::string_view stringName) const;
   /// find param value for name
   [[nodiscard]] argument find_argument(std::string_view stringName) const {
      const_pointer pPosition = find(stringName);
      if( pPosition ) return get_argument_s(pPosition);
      return argument();
   }

   /// return argument object that can be used to edit value if found
   [[nodiscard]] argument_edit find_edit_argument(std::string_view stringName);
   /// find param value for name and start from position to search
   [[nodiscard]] argument find_argument(const std::string_view& stringName, const_pointer pPosition) const;

   [[nodiscard]] bool exists( const std::string_view& stringName ) const;
   [[nodiscard]] std::pair<bool, std::string> exists( const std::initializer_list<std::string_view>& listName, tag_name ) const { return exists_s( *this, listName, tag_name{}); }
   [[nodiscard]] std::pair<bool, std::string> exists( const std::initializer_list<std::pair<std::string_view, std::string_view>>& listName, tag_description ) const { return exists_s( *this, listName, tag_description{}); }
   [[nodiscard]] std::pair<bool, std::string> exists_any_of( const std::initializer_list<std::string_view>& listName, tag_name ) const { return exists_any_of_s( *this, listName, tag_name{}); }


//@}

/** \name COMPARE
* compare functionality, checks if values in arguments are equal
*///@{
   [[nodiscard]] bool compare(const std::pair<std::string_view, gd::variant_view>& pairMatch) const { return find(pairMatch) != nullptr; }
   [[nodiscard]] bool compare(const std::string_view& stringName, const arguments& argumentsCompareTo) const;
   [[nodiscard]] bool compare_exists(const arguments& argumentsExists) const { return compare_exists_s( *this, argumentsExists ); }
//@}

/** \name MOVE move pointer between values in arguments
* move operations used to move between values, can't go back. only forward
*///@{
   [[nodiscard]] pointer next() { return m_pbuffer->size() > 0 ? m_pbuffer->data() : nullptr; }
   [[nodiscard]] const_pointer next() const { return m_pbuffer->size() > 0 ? m_pbuffer->data() : nullptr; }
   [[nodiscard]] pointer next(pointer pPosition) {                             assert( verify_d(pPosition) );
      auto p = next_s(pPosition);
      return p < buffer_data_end() ? p : nullptr;
   }
   [[nodiscard]] const_pointer next(const_pointer pPosition) const {           assert( verify_d(pPosition) );
      auto p = next_s(pPosition);
      return p < buffer_data_end() ? p : nullptr;
   }
//@}

/** \name VALIDATE validation operations for argumetns
* 
*///@{
//@}


   /// number of arguments found in arguments object
   [[nodiscard]] size_t size() const;

   /// cleans upp interal data and set it as empty
   void clear();

/** \name ARGUMENT
* get argument value from arguments
*///@{
   [[nodiscard]] argument get_argument() const { if( buffer_size() ) return get_argument_s(buffer_data()); else return argument();  }
   [[nodiscard]] argument get_argument(const_pointer pPosition) const {                assert( verify_d(pPosition) );
      return get_argument_s(pPosition); 
   }

   [[nodiscard]] argument get_argument(unsigned int uIndex) const;
   [[nodiscard]] argument get_argument(std::string_view stringName) const { return find_argument(stringName); }
   template<class DEFAULT>
   [[nodiscard]] DEFAULT get_argument(const std::string_view& stringName, DEFAULT defaultValue) const {
      argument  v = find_argument(stringName);
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
      return argument();
   }

   /// return all values for name
   [[nodiscard]] std::vector<argument> get_argument_all(std::string_view stringName) const { return get_argument_all_s(get_buffer_start(), get_buffer_end(), stringName); }
   [[nodiscard]] std::vector<gd::variant_view> get_argument_all(std::string_view stringName, tag_view) const { return get_argument_all_s(get_buffer_start(), get_buffer_end(), stringName, tag_view{} ); }
   [[nodiscard]] std::vector<gd::variant_view> get_argument_section(std::string_view stringName, tag_view) const { return get_argument_section_s(get_buffer_start(), get_buffer_end(), stringName, tag_view{} ); }

   std::vector<argument> get_argument( std::vector< std::string_view > vectorName ) const;

   /// return first value for name 
   gd::variant_view get_variant_view( const std::string_view& stringName ) const { return get_argument( stringName ).get_variant_view(); }
   [[nodiscard]] std::pair< std::string_view, gd::variant_view > get_variant_view(unsigned int uIndex, tag_pair ) const;
//@}

   template<typename OBJECT>
   void get_object( const std::string_view& stringPrefixFind, OBJECT& object_ );
   template<typename OBJECT>
   void get_object( OBJECT& object_ ) { get_object( std::string_view(), object_ ); }
   template<typename OBJECT>
   OBJECT get_object();
   template<typename OBJECT>
   OBJECT get_object( const std::string_view& stringPrefixFind );


/** \name PRINT
* Methods used to format argument values into text
*///@{
   std::string print() const;
   std::string print(const_iterator itBegin) const { return print(itBegin, end(), ", "); };
   std::string print( const_iterator itBegin, const_iterator itEnd ) const { return print(itBegin, itEnd, ", "); };
   std::string print(const_iterator itBegin, const_iterator itEnd, std::string_view stringSplit) const;
   std::string print_json() const;

   std::string print(std::string_view stringFormat) const;
//@}

   


#if defined(_DEBUG) || defined(DEBUG) || !defined(NODEBUG)
   bool verify_d(const_pointer pPosition) const { return pPosition >= m_pbuffer->data() && pPosition < ( m_pbuffer->data() + m_pbuffer->size() ) ? true : false; }
#endif
//@}

/** \name BUFFER
*///@{
   /// make sure internal buffer can hold specified number of bytes, buffer data is copied if increased
   bool reserve(uint64_t uCount);
   /// Remove param starting at position, remember that if you are string positions in buffer they are invalidated with this method
   void remove( const std::string_view& stringName );
   void remove(const_pointer pPosition);
   void remove(const_iterator it) { remove(it); }
   /// make sure internal buffer can hold specified number of bytes, no copying just reserving data

   /// Resize one argument within arguments object, do not use this if you do not know how arguments work!!
   int resize(pointer pPosition, int iOffset, int iNewOffset);
   /// remove unused memory
   void shrink_to_fit();
//@}

   static bool is_name_s(const_pointer pPosition) {                                                assert(*pPosition != 0);
      uint32_t uType = *(uint32_t*)pPosition;
      uType = uType >> 24;
      return uType == arguments::eType_ParameterName;
   }
   std::string_view get_name(const_pointer pPosition) { return get_name_s( pPosition ); }

/** \name INTERNAL FREE FUNCTIONS
*///@{
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
      const char* pbszName = (const char*)pPosition + sizeof( uint32_t );
      uint32_t uLength = *(uint32_t*)pPosition & 0x00FFFFFF;
      return std::string_view(pbszName, uLength);// generate string_view for name
   }

   /// ## argument methods
   /// return param the position points to
   static argument get_argument_s(const_pointer pPosition);
   /// return editable param based on position
   static argument_edit get_edit_param_s(arguments* parguments, const_pointer pPosition);
   /// count internal param length in bytes
   static uint64_t get_total_param_length_s(const_pointer pPosition) noexcept;
   static unsigned int get_total_param_length_s(std::string_view stringName, const argument argumentValue);
   static std::vector<argument> get_argument_all_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName);
   static std::vector<gd::variant_view> get_argument_all_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName, tag_view);
   static std::vector<gd::variant_view> get_argument_section_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName, tag_view);

   /// ## move methods
   /// move pointer to next value in buffer
   static pointer next_s(pointer pPosition);
   static const_pointer next_s(const_pointer pPosition);

   /// ## Calculate size in bytes needed for argument values stored in arguments object
   static unsigned int sizeof_s(const argument& argumentValue);
   static unsigned int sizeof_s(uint32_t uNameLength, param_type uType, unsigned int uLength);
   static inline unsigned int sizeof_name_s(uint32_t uNameLength) noexcept { return uNameLength + sizeof(uint32_t); }
   static inline unsigned int sizeof_name_s(uint32_t uNameLength, tag_align) noexcept;
   static inline unsigned int sizeof_name_s(const_pointer pPosition) noexcept; 
   static inline unsigned int sizeof_name_s(const_pointer pPosition, tag_align) noexcept; 

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
   constexpr static unsigned int type_s(const_pointer pPosition) noexcept;
   static unsigned int type_s(const_pointer pPosition, uint32_t* puSize ) noexcept;
   constexpr static unsigned int type_s(unsigned int uType) noexcept { return uType & ~eType_MASK; } // only type (no size)
   constexpr static unsigned int ctype_s(unsigned int uType) noexcept { return uType & ~eCType_MASK; } // last byte (type and size)
   constexpr static unsigned int type_number_s(unsigned int uType) noexcept { return uType & ~eTypeNumber_MASK; }
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

   //static arguments read_json_s(const argument& argumentValue);
//@}


// ## 
public:
   void buffer_delete() { if( m_pbuffer != &m_buffer_s ) { m_pbuffer->release(); m_pbuffer = &m_buffer_s; }  }
   pointer buffer_data() { return m_pbuffer->data(); }
   const_pointer buffer_data() const { return m_pbuffer->data(); }
   const_pointer buffer_data_end() const { return m_pbuffer->data() + m_pbuffer->size(); }
   uint64_t buffer_size() const { return m_pbuffer->size(); }
   uint64_t buffer_buffer_size() const { return m_pbuffer->buffer_size(); }
   void buffer_set_size( uint64_t uSize ) { m_pbuffer->size( uSize ); }
   void buffer_release() { 
      if( is_null() == false ) 
      {
         m_pbuffer->release();
         m_pbuffer = &m_buffer_s;
      }
   }
   unsigned buffer_reference_count() const { return m_pbuffer->m_iReferenceCount; }



// ## attributes ----------------------------------------------------------------
public:
   bool           m_bOwner;      ///< if buffer is owned (delete in destructor)
   pointer        m_pBuffer;     ///< pointer to byte array
   //unsigned int   m_uLength;     ///< length in use
   //unsigned int   m_uBufferLength;///< length for byte array

   buffer* m_pbuffer = &m_buffer_s;

   inline static buffer m_buffer_s{ 0ull, 0ull };

   static size_type npos;

};

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

/// append values from vector with variant items
inline arguments& arguments::append( const std::vector<std::pair<std::string_view,std::string_view>>& vectorStringValue, tag_parse_type ) {
   for( auto it : vectorStringValue ) append_argument( it.first, it.second, tag_parse_type{} );
   return *this;
}

inline arguments& arguments::append( const std::vector<std::pair<std::string,std::string>>& vectorStringValue, tag_parse_type ) {
   for( auto it : vectorStringValue ) append_argument( it.first, it.second, tag_parse_type{} );
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


inline arguments& arguments::set(std::string_view stringName, const gd::variant_view& variantValue) {
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();
   unsigned uLength;
   if( uType > ARGUMENTS_NO_LENGTH ) 
   { 
      unsigned uZeroEnd = 0;
      if( uType == eTypeNumberWString )
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
   const_pointer pPosition = find(stringName);
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

/// compare if value is equal for specified name
/// comparing text is faster with this internal compare method because no object is created on heap holding argument value
inline bool arguments::compare(const std::string_view& stringName, const arguments& argumentsCompareTo) const {
   const argument argumentCompare = get_argument(stringName);
   if( argumentCompare.is_null() ) return false;
   const argument argumentCompareTo = argumentsCompareTo.get_argument(stringName);
   return compare_argument_s(argumentCompare, argumentCompareTo);
}



// ================================================================================================
// ================================================================================= FREE FUNCTIONS
// ================================================================================================

/// get type number from position, make sure position points to type
constexpr unsigned int arguments::type_s(const_pointer pPosition) noexcept {
   uint32_t u_ = *(uint32_t*)pPosition;
   uint32_t uType = (u_ >> 24) & ~eType_MASK;                                  // get value type
                                                                                                   assert( uType < CItem_MAX );
   return uType;
}

/// return type and value size if pointer to unsigned is sent
inline unsigned int arguments::type_s(const_pointer pPosition, uint32_t* puSize ) noexcept {
   uint32_t u_ = *(uint32_t*)pPosition;
   uint32_t uType = (u_ >> 24) & ~eType_MASK;                                  // get value type
                                                                                                   assert( uType < CItem_MAX );
   if( puSize != nullptr ) *puSize = u_ & 0xFFFFFF;
   return uType;
}

/// return type as text name
inline constexpr std::string_view arguments::type_name_s(uint32_t uType)
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


_GD_ARGUMENT_SHARED_END

_GD_ARGUMENT_SHARED_BEGIN
namespace debug {
   std::string print( const arguments::argument& argumentToPrint );
   std::string print( const arguments& argumentsToPrint );
   std::string print( const arguments& argumentsToPrint, const std::string_view& stringSeparator );
   std::string print( const std::vector<arguments>& vectorToPrint );
   //std::string print( const std::vector<arguments>* vectorToPrint );
}
_GD_ARGUMENT_SHARED_END


#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
