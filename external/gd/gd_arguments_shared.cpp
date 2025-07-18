/**
 * \file gd_arguments_shared.cpp
 * 
 * \brief Pack primitive and common derived values into a byte buffer for efficient storage and access.
 *
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0construct.arguments` - Insert operations
 * - `0TAG0append.arguments` - Append operations
 * - `0TAG0set.arguments` - Set operations
 * - `0TAG0insert.arguments` - Insert operations
 * - `0TAG0merge.arguments` - Merge operations
 * - `0TAG0find.arguments` - Find operations
 * - `0TAG0print.arguments` - Find operations
 */


#include <iterator>
#include <cwchar>

#include "gd_utf8.h"  

#include "gd_arguments_shared.h"  

#if GD_COMPILER_HAS_CPP20_SUPPORT

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wswitch"
   #pragma clang diagnostic ignored "-Wformat"
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
   #pragma GCC diagnostic ignored "-Wswitch"
   #pragma GCC diagnostic ignored "-Wformat"
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996 6054 6387 26812 33010 ) // disable warnings for buffer that might not be zero terminated and 
#endif



_GD_ARGUMENT_SHARED_BEGIN

inline uint32_t align32_g( uint32_t uLength ) 
{
   if( uLength % 4 != 0 ) uLength = (uLength + 3) & ~3;
   return uLength;
}

inline uint64_t align32_g( uint64_t uLength ) 
{
   if( uLength % 4 != 0 ) uLength = (uLength + 3) & ~3;
   return uLength;
}


/*----------------------------------------------------------------------------- move_to_value */ /**
 * Moves pointer to value part in argument
 * If argument has a name and position pointer is at the name, then use this method
 * to move position to value part.
 * \note first byte for name part is number identifying parameter name, next byte is name lenght
 * \param pPosition position that is moved if it is on name
 * \return arguments::const_pointer returns pointer to value
 */
arguments::pointer arguments::move_to_value_s(pointer pPosition)
{                                                                                                  assert( pPosition != nullptr );
   uint32_t u_ = *(uint32_t*)pPosition;
   enumCType eType = (enumCType)(u_ >> 24);                                    // get section type
   if( eType == eType_ParameterName )
   {
      uint32_t uLength = u_ & 0x00FFFFFF;
      uLength = align32_g( uLength );

      pPosition += sizeof( uint32_t ) + uLength;
   }
#ifndef NDEBUG
   u_ = *(uint32_t*)pPosition;
   eType = (enumCType)(u_ >> 24);
   auto typename_d = gd::types::type_name_g( eType & ~eTypeNumber_MASK );
#endif
   return pPosition;
}

/// move past name to value data (dont use this if pointer is not on name for value)
arguments::pointer arguments::move_to_value_data_s(pointer pPosition)
{                                                                                                  assert( pPosition != nullptr );
   uint32_t u_ = *(uint32_t*)pPosition;
   enumCType eType = (enumCType)(u_ >> 24);                                                        assert( ((unsigned)eType & ~eTypeNumber_MASK) == eType_ParameterName );
   pPosition += sizeof(uint32_t);

   return pPosition;
}

/// move past name to value data (dont use this if pointer is not on name for value)
arguments::const_pointer arguments::move_to_value_data_s(const_pointer pPosition)
{                                                                                                  assert( pPosition != nullptr );
   uint32_t u_ = *(uint32_t*)pPosition;
   enumCType eType = (enumCType)(u_ >> 24);                                                        assert( ((unsigned)eType & ~eTypeNumber_MASK) == eType_ParameterName );
   pPosition += sizeof(uint32_t);

   return pPosition;
}


/*----------------------------------------------------------------------------- move_to_value */ /**
 * Moves pointer to value part in argument if it is placed on name for value
 * If argument has a name and position pointer is at the name, then use this method
 * to move position to value part.
 * \note first byte for name part is number identifying parameter name, next byte is name length
 * \param pPosition position that is moved if it is on name
 * \return arguments::const_pointer returns pointer to value
 */
arguments::const_pointer arguments::move_to_value_s(const_pointer pPosition)
{
   uint32_t u_ = *(uint32_t*)pPosition;
   enumCType eType = (enumCType)(u_ >> 24);                                    // get section type
   if( eType == eType_ParameterName )
   {
      uint32_t uLength = u_ & 0x00FFFFFF;
      uLength = align32_g( uLength );

      pPosition += sizeof( uint32_t ) + uLength;
   }
#ifndef NDEBUG
   u_ = *(uint32_t*)pPosition;
   eType = (enumCType)(u_ >> 24);
   auto typename_d = gd::types::type_name_g( eType & ~eTypeNumber_MASK );
#endif
   return pPosition;
}


/*----------------------------------------------------------------------------- compare_name_s */ /**
 * compare name at argument position in arguments buffer
 * \param pPosition position in arguments buffer
 * \param stringName name to compare with
 * \return bool true if name matches, false if not
 */
bool arguments::compare_name_s(const_pointer pPosition, std::string_view stringName)
{
   if( is_name_s(pPosition) && get_name_s(pPosition) == stringName ) return true;
   return false;
}

const char _binary_pszHEX[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };


template <typename CHAR>
inline void _binary_to_hex(CHAR* pTPosition, const uint8_t* pBytes, unsigned uLength)
{
   *pTPosition = (CHAR)0;                                                        // add char 0 ending to string
   const uint8_t* pBytesStop = pBytes - uLength;

   // optimize, align for 4 byte sections
   for( unsigned int u = uLength % 4; u > 0; u-- )
   {
      pBytes--;
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[*pBytes & 0x0F];
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[(*pBytes & 0xF0) >> 4];

   }

   while( pBytes != pBytesStop )
   {
      pBytes--;
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[*pBytes & 0x0F];
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[(*pBytes & 0xF0) >> 4];

      pBytes--;
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[*pBytes & 0x0F];
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[(*pBytes & 0xF0) >> 4];

      pBytes--;
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[*pBytes & 0x0F];
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[(*pBytes & 0xF0) >> 4];

      pBytes--;
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[*pBytes & 0x0F];
      pTPosition--;
      *pTPosition = (CHAR)_binary_pszHEX[(*pBytes & 0xF0) >> 4];
   }
}

/*----------------------------------------------------------------------------- length */ /**
 * return size buffer for argument in bytes, this do not include the space needed for type and size
 * \return unsigned int number of bytes param occupies 
 */
unsigned int arguments::argument::size() const
{
   auto eTypeNumber = type_number();
   if( eTypeNumber < arguments::eTypeNumberString ) { return ctype_size[(unsigned)eTypeNumber]; }
   else
   {
      if( ctype() & eValueLength )
      {
         // ## value in bytes are stored two unsigned values before data
         auto uSize = *(uint32_t*)(m_unionValue.puch - sizeof(uint32_t) * 2);                      
         uSize &= 0x00FFFFFF;                                                                      assert(uSize < 0x00A00000); // realistic// clear type
         uSize -= sizeof(uint32_t);                                            // remove size for type and size (the prefixed size value includs it)
         return uSize;
      }
   }
                                                                                                   assert( false );
   return 0;
}

/*----------------------------------------------------------------------------- length */ /**
 * return length for argument in bytes
 \note if argument is holding a string, then ending zero isn't counted
 * \return unsigned int number of bytes param occupies 
 */
unsigned int arguments::argument::length() const
{
   auto eTypeNumber = type_number();
   if( eTypeNumber < arguments::eTypeNumberString ) { return ctype_size[(unsigned)eTypeNumber]; }
   else
   {
      if( ctype() & eValueLength )
      {
         // ## native value size is stored in front of value
         auto uSize = *(uint32_t*)(m_unionValue.puch - sizeof(uint32_t));                          assert(uSize < 0x00A00000); // realistic
         return uSize;
      }

      return (unsigned int)strlen(m_unionValue.pbsz);                          // don't add zero terminator, shared::arguments do not store the extra 0 ending for characters
   }
                                                                                                   assert( false );
   return 0;
}

unsigned int arguments::argument::get_binary_as_hex(char* pbsz, unsigned int uLength) const
{                                                                                assert(uLength > 0);
   // each byte needs two characters
   uLength--;                                               // zero character position
   uLength >>= 1;                                           // number of bytes that string can hold

   auto uParamLength = length();
   if( uLength > uParamLength ) { uLength = uParamLength; }

   char* pbszPosition = &pbsz[uLength << 1];
   const uint8_t* pBytes = m_unionValue.puch + uLength;
   _binary_to_hex( pbszPosition, pBytes, uLength);

   return uLength;
}

#pragma warning( disable : 6255 )

/*----------------------------------------------------------------------------- get_binary_as_hex */ /**
 * Get binary value as hexadecimal formated string
 * \param stringHex
 * \return void
 */
void arguments::argument::get_binary_as_hex(std::string& stringHex) const
{
   char* pbsz;
   unsigned int uLength = length() * 2;

   if( uLength < 0x1000 )
   {
      pbsz = (char*)alloca((size_t)uLength + 1);
   }
   else
   {
      pbsz = new char[(size_t)uLength + 1];
   }

   get_binary_as_hex(pbsz, uLength + 1);
   stringHex = std::string_view( pbsz, uLength );

   if( uLength >= 0x1000 ) delete[] pbsz;
}

/** ----------------------------------------------------------------------------- get_bool */ /**
 * @brief Tries to convert value to bool and returns that bool value.
 *        For numbers, 0 is false and any other value is true.
 *        For strings, empty is false, non-empty is true.
 *        For floating point, 0.0 is false, otherwise true.
 *        For bool, returns the stored value.
 *        For unknown types, returns false.
 * @return bool converted value
 */
bool arguments::argument::get_bool() const
{
   switch( type_number_s(m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      return false;
   case arguments::eTypeNumberBool:
      return m_unionValue.b;
   case arguments::eTypeNumberInt8:
      return m_unionValue.v_int8 != 0;
   case arguments::eTypeNumberUInt8:
      return m_unionValue.v_uint8 != 0;
   case arguments::eTypeNumberInt16:
      return m_unionValue.v_int16 != 0;
   case arguments::eTypeNumberUInt16:
      return m_unionValue.v_uint16 != 0;
   case arguments::eTypeNumberInt32:
      return m_unionValue.v_int32 != 0;
   case arguments::eTypeNumberUInt32:
      return m_unionValue.v_uint32 != 0;
   case arguments::eTypeNumberInt64:
      return m_unionValue.v_int64 != 0;
   case arguments::eTypeNumberUInt64:
      return m_unionValue.v_uint64 != 0;
   case arguments::eTypeNumberFloat:
      return m_unionValue.f != 0.0f;
   case arguments::eTypeNumberDouble:
      return m_unionValue.d != 0.0;
   case arguments::eTypeNumberString:
   case arguments::eTypeNumberUtf8String:
      return ( m_unionValue.pbsz != nullptr && *m_unionValue.pbsz != '\0' );
   case arguments::eTypeNumberWString:
      return ( m_unionValue.pwsz != nullptr && *m_unionValue.pwsz != L'\0' );
   default:
      assert(false);
      return false;
   }
}

/*----------------------------------------------------------------------------- get_int */ /**
 * Tries to convert value to int and returns that int value, if it failles to convert to int then 0 is returned
 * \return int converted value
 */
int arguments::argument::get_int() const
{
   int iValue = 0;

   switch( type_number_s( m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      iValue = 0;
      break;
   case arguments::eTypeNumberBool :
      iValue = ( m_unionValue.b == true ? 1 : 0 );
      break;
   case arguments::eTypeNumberInt8:
      iValue = (int)m_unionValue.v_int8;
      break;
   case arguments::eTypeNumberUInt8:
      iValue = (int)m_unionValue.v_uint8;
      break;
   case arguments::eTypeNumberInt16 :
      iValue = (int)m_unionValue.v_int16;
      break;
   case arguments::eTypeNumberUInt16:
      iValue = (int)m_unionValue.v_uint16;
      break;
   case arguments::eTypeNumberInt32 :
      iValue = (int)m_unionValue.v_int32;
      break;
   case arguments::eTypeNumberUInt32 :
      iValue = (int)m_unionValue.v_uint32;
      break;
   case arguments::eTypeNumberInt64 :
      iValue = (int)m_unionValue.v_int64;
      break;
   case arguments::eTypeNumberUInt64 :
      iValue = (int)m_unionValue.v_uint64;
      break;
   case arguments::eTypeNumberFloat :
      iValue = (int)m_unionValue.f;
      break;
   case arguments::eTypeNumberDouble :
      iValue = (int)m_unionValue.d;
      break;
   case arguments::eTypeNumberString :
   case arguments::eTypeNumberUtf8String :
      iValue = atoi( m_unionValue.pbsz );
      break;
   case arguments::eTypeNumberWString :
#if defined(_MSC_VER)
      iValue = _wtoi( m_unionValue.pwsz );
#else      
      iValue = wcstol( m_unionValue.pwsz, 0, 10 );
#endif      
      break;
   default:
                                                                                                   assert( false );
   }

   return iValue;
}

/*----------------------------------------------------------------------------- get_uint */ /**
 * Try to get param value as unsigned int
 * \return unsigned int converted value
 */
unsigned int arguments::argument::get_uint() const
{
   unsigned int uValue = 0;

   switch( type_number_s( m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      uValue = 0;
      break;
   case arguments::eTypeNumberBool:
      uValue = (m_unionValue.b == true ? 1 : 0);
      break;
   case arguments::eTypeNumberInt8:
      uValue = (unsigned int)m_unionValue.v_int8;
      break;
   case arguments::eTypeNumberUInt8:
      uValue = (unsigned int)m_unionValue.v_uint8;
      break;
   case arguments::eTypeNumberInt16:
      uValue = (unsigned int)m_unionValue.v_int16;
      break;
   case arguments::eTypeNumberUInt16:
      uValue = (unsigned int)m_unionValue.v_uint16;
      break;
   case arguments::eTypeNumberInt32:
      uValue = (unsigned int)m_unionValue.v_int32;
      break;
   case arguments::eTypeNumberUInt32:
      uValue = (unsigned int)m_unionValue.v_uint32;
      break;
   case arguments::eTypeNumberInt64:
      uValue = (unsigned int)m_unionValue.v_int64;
      break;
   case arguments::eTypeNumberUInt64:
      uValue = (unsigned int)m_unionValue.v_uint64;
      break;
   case arguments::eTypeNumberFloat:
      uValue = (unsigned int)m_unionValue.f;
      break;
   case arguments::eTypeNumberDouble:
      uValue = (unsigned int)m_unionValue.d;
      break;
   case arguments::eTypeNumberString:
   case arguments::eTypeNumberUtf8String:
      uValue = strtoul(m_unionValue.pbsz, nullptr, 10);
      break;
   case arguments::eTypeNumberWString:
      uValue = wcstoul(m_unionValue.pwsz, nullptr, 10);
      break;
   default:
      assert(false);
   }


   return uValue;
}

/*----------------------------------------------------------------------------- get_int */ /**
 * Tries to convert value to int and returns that int value, if it fails to convert to int then 0 is returned
 * \return int64_t converted value
 */
int64_t arguments::argument::get_int64() const
{
   int64_t iValue = 0;

   switch( type_number_s( m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      iValue = 0;
      break;
   case arguments::eTypeNumberBool :
      iValue = ( m_unionValue.b == true ? 1 : 0 );
      break;
   case arguments::eTypeNumberInt8:
      iValue = (int64_t)m_unionValue.v_int8;
      break;
   case arguments::eTypeNumberUInt8:
      iValue = (int64_t)m_unionValue.v_uint8;
      break;
   case arguments::eTypeNumberInt16 :
      iValue = (int64_t)m_unionValue.v_int16;
      break;
   case arguments::eTypeNumberUInt16:
      iValue = (int64_t)m_unionValue.v_uint16;
      break;
   case arguments::eTypeNumberInt32 :
      iValue = (int64_t)m_unionValue.v_int32;
      break;
   case arguments::eTypeNumberUInt32 :
      iValue = (int64_t)m_unionValue.v_uint32;
      break;
   case arguments::eTypeNumberInt64 :
      iValue = (int64_t)m_unionValue.v_int64;
      break;
   case arguments::eTypeNumberUInt64 :
      iValue = (int64_t)m_unionValue.v_uint64;
      break;
   case arguments::eTypeNumberFloat :
      iValue = (int64_t)m_unionValue.f;
      break;
   case arguments::eTypeNumberDouble :
      iValue = (int64_t)m_unionValue.d;
      break;
   case arguments::eTypeNumberString :
   case arguments::eTypeNumberUtf8String :
      iValue = atoll( m_unionValue.pbsz );
      break;
   case arguments::eTypeNumberWString :
      iValue = std::wcstoll( m_unionValue.pwsz, 0, 10 );
      break;
   default:
                                                                                 assert( false );
   }

   return iValue;
}

/*----------------------------------------------------------------------------- get_int */ /**
 * Tries to convert value to int and returns that int value, if it fails to convert to int then 0 is returned
 * \return uint64_t converted value
 */
uint64_t arguments::argument::get_uint64() const
{
   uint64_t uValue = 0;

   switch( type_number_s( m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      uValue = 0;
      break;
   case arguments::eTypeNumberBool :
      uValue = ( m_unionValue.b == true ? 1 : 0 );
      break;
   case arguments::eTypeNumberInt8:
      uValue = (uint64_t)m_unionValue.v_int8;
      break;
   case arguments::eTypeNumberUInt8:
      uValue = (uint64_t)m_unionValue.v_uint8;
      break;
   case arguments::eTypeNumberInt16 :
      uValue = (uint64_t)m_unionValue.v_int16;
      break;
   case arguments::eTypeNumberUInt16:
      uValue = (uint64_t)m_unionValue.v_uint16;
      break;
   case arguments::eTypeNumberInt32 :
      uValue = (uint64_t)m_unionValue.v_int32;
      break;
   case arguments::eTypeNumberUInt32 :
      uValue = (uint64_t)m_unionValue.v_uint32;
      break;
   case arguments::eTypeNumberInt64 :
      uValue = (uint64_t)m_unionValue.v_int64;
      break;
   case arguments::eTypeNumberUInt64 :
      uValue = (uint64_t)m_unionValue.v_uint64;
      break;
   case arguments::eTypeNumberFloat :
      uValue = (uint64_t)m_unionValue.f;
      break;
   case arguments::eTypeNumberDouble :
      uValue = (uint64_t)m_unionValue.d;
      break;
   case arguments::eTypeNumberString :
   case arguments::eTypeNumberUtf8String :
      uValue = std::stoull( m_unionValue.pbsz );
      break;
   case arguments::eTypeNumberWString :
      uValue = std::wcstoull( m_unionValue.pwsz, nullptr, 10 );
      break;
   default:
                                                                                 assert( false );
   }

   return uValue;
}





double arguments::argument::get_double() const
{
   double dValue = 0;

   switch( type_number_s(m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      dValue = 0;
      break;
   case arguments::eTypeNumberBool:
      dValue = (m_unionValue.b == true ? 1 : 0);
      break;
   case arguments::eTypeNumberInt8:
      dValue = (double)m_unionValue.v_int8;
      break;
   case arguments::eTypeNumberUInt8:
      dValue = (double)m_unionValue.v_uint8;
      break;
   case arguments::eTypeNumberInt16:
      dValue = (double)m_unionValue.v_int16;
      break;
   case arguments::eTypeNumberUInt16:
      dValue = (double)m_unionValue.v_uint16;
      break;
   case arguments::eTypeNumberInt32:
      dValue = (double)m_unionValue.v_int32;
      break;
   case arguments::eTypeNumberUInt32:
      dValue = (double)m_unionValue.v_uint32;
      break;
   case arguments::eTypeNumberInt64:
      dValue = (double)m_unionValue.v_int64;
      break;
   case arguments::eTypeNumberUInt64:
      dValue = (double)m_unionValue.v_uint64;
      break;
   case arguments::eTypeNumberFloat:
      dValue = (double)m_unionValue.f;
      break;
   case arguments::eTypeNumberDouble:
      dValue = m_unionValue.d;
      break;
   case arguments::eTypeNumberString:
   case arguments::eTypeNumberUtf8String:
      dValue = std::strtod(m_unionValue.pbsz, nullptr);
      break;
   case arguments::eTypeNumberWString:
      dValue = wcstod(m_unionValue.pwsz, nullptr);
      break;
   default:
      assert(false);
   }


   return dValue;
}

/*----------------------------------------------------------------------------- get_string */ /**
 * Return value as string object
 * \return std::string string object with value for param
 */
std::string arguments::argument::get_string() const
{
   std::string s;
   if( ctype_s( m_eType ) == (arguments::eTypeNumberString | eValueLength) || ctype_s( m_eType ) == (arguments::eTypeNumberUtf8String | eValueLength) )
   {
      s = std::string_view(m_unionValue.pbsz, length() );                     // try for string before converting other possible values (remember to not include last zero ending as text)
   }
   else
   {
      char pbsz[40];
#ifdef _DEBUG
      memset(pbsz, 'x', 40);
      pbsz[39] = L'\0';
#else
      pbsz[0] = L'\0';
#endif
      enumCType eCType = static_cast<enumCType>( type_number_s(m_eType) );
      switch( eCType )
      {
      case arguments::eTypeNumberUnknown:
         pbsz[0] = '\0';
         break;
      case arguments::eTypeNumberBool:
         pbsz[0] = '0';
         pbsz[1] = '\0';
         if( m_unionValue.b == true ) pbsz[0] = '1';
         break;
      case arguments::eTypeNumberInt8:
         gd::utf8::itoa( (int)m_unionValue.v_int8, pbsz );
         break;
      case arguments::eTypeNumberUInt8:
         gd::utf8::utoa( (unsigned int)m_unionValue.v_uint8, pbsz );
         break;
      case arguments::eTypeNumberInt16:
         gd::utf8::itoa( (int)m_unionValue.v_int16, pbsz );
         break;
      case arguments::eTypeNumberUInt16:
         gd::utf8::utoa( (unsigned int)m_unionValue.v_uint16, pbsz );
         break;
      case arguments::eTypeNumberInt32:
         gd::utf8::itoa( m_unionValue.v_int32, pbsz );
         break;
      case arguments::eTypeNumberUInt32:
         gd::utf8::utoa( m_unionValue.v_uint32, pbsz );
         break;
      case arguments::eTypeNumberInt64:
         gd::utf8::itoa( m_unionValue.v_int64, pbsz );
         break;
      case arguments::eTypeNumberUInt64:
         gd::utf8::utoa( m_unionValue.v_uint64, pbsz );
         break;
      case arguments::eTypeNumberFloat:
         sprintf(pbsz, "%g", (double)m_unionValue.f);
         break;
      case arguments::eTypeNumberDouble:
         sprintf(pbsz, "%g", m_unionValue.d);
         break;
      case arguments::eTypeNumberPointer:
         // When storing pointer and get_string is called, it thinks that the pointer
         // is to char string. If you haven't set this to char pointer and runs get_string
         // there will be an error.
                                                                                 assert(strlen(m_unionValue.pbsz) < 0x000F0000); // realistic
         s = m_unionValue.pbsz;
         break;
      case arguments::eTypeNumberString:
      case arguments::eTypeNumberUtf8String:
      {
         s = std::string_view(m_unionValue.pbsz);
      }
      break;
      case arguments::eTypeNumberWString:
      {
         gd::utf8::convert_utf16_to_uft8( reinterpret_cast<const uint16_t*>(m_unionValue.pwsz), s);
      }
      break;
      case arguments::eTypeNumberBinary:
      {
         get_binary_as_hex(s);
         return std::move(s);
      }
      case arguments::eTypeNumberGuid:
      {
         constexpr unsigned int uUuidSize = 16;
         //pbsz[uUuidSize] = '\0';
         _binary_to_hex(pbsz + uUuidSize * 2, (uint8_t*)m_unionValue.p + uUuidSize, uUuidSize);
      }
      break;
      default:
         assert(false);
      }

      if( s.empty() == true ) s = pbsz;
   }

   return std::move(s);
}

/** ---------------------------------------------------------------------------
 * Return value as string object
 * \return std::string string object with value for param
 */
std::string arguments::argument::get_utf8() const
{
   std::string s;
   if( ctype_s( m_eType ) == (arguments::eTypeNumberString | eValueLength) || ctype_s( m_eType ) == (arguments::eTypeNumberUtf8String | eValueLength) )
   {
      s = std::string_view(m_unionValue.pbsz, length() - 1); // try for string before converting other possible values (remember to not include last zero ending as text)
   }
   else
   {
      char pbsz[40];
#ifdef _DEBUG
      memset(pbsz, 'x', 40);
      pbsz[39] = L'\0';
#else
      pbsz[0] = L'\0';
#endif
      enumCType eCType = static_cast<enumCType>( type_number_s(m_eType) );
      switch( eCType )
      {
      case arguments::eTypeNumberBool:
         pbsz[0] = '0';
         pbsz[1] = '\0';
         if( m_unionValue.b == true ) pbsz[0] = '1';
         break;
      case arguments::eTypeNumberInt8:
         gd::utf8::itoa( (int)m_unionValue.v_int8, pbsz );
         break;
      case arguments::eTypeNumberUInt8:
         gd::utf8::utoa( (unsigned int)m_unionValue.v_uint8, pbsz );
         break;
      case arguments::eTypeNumberInt16:
         gd::utf8::itoa( (int)m_unionValue.v_int16, pbsz );
         break;
      case arguments::eTypeNumberUInt16:
         gd::utf8::utoa( (unsigned int)m_unionValue.v_uint16, pbsz );
         break;
      case arguments::eTypeNumberInt32:
         gd::utf8::itoa( m_unionValue.v_int32, pbsz );
         break;
      case arguments::eTypeNumberUInt32:
         gd::utf8::utoa( m_unionValue.v_uint32, pbsz );
         break;
      case arguments::eTypeNumberInt64:
         gd::utf8::itoa( m_unionValue.v_int64, pbsz );
         break;
      case arguments::eTypeNumberUInt64:
         gd::utf8::utoa( m_unionValue.v_uint64, pbsz );
         break;
      case arguments::eTypeNumberFloat:
         sprintf(pbsz, "%g", (double)m_unionValue.f);
         break;
      case arguments::eTypeNumberDouble:
         sprintf(pbsz, "%g", m_unionValue.d);
         break;
      case arguments::eTypeNumberPointer:
         // When storing pointer and get_string is called, it thinks that the pointer
         // is to char string. If you haven't set this to char pointer and runs get_string
         // there will be an error.
                                                                                 assert(strlen(m_unionValue.pbsz) < 0x000F0000); // realistic
         s = m_unionValue.pbsz;
         break;
      case arguments::eTypeNumberString:
      case arguments::eTypeNumberUtf8String:
      {
         s = std::string_view(m_unionValue.pbsz);
      }
      break;
      case arguments::eTypeNumberWString:
      {
         gd::utf8::convert_utf16_to_uft8( reinterpret_cast<const uint16_t*>(m_unionValue.pwsz), s);
      }
      break;
      case arguments::eTypeNumberBinary:
      {
         get_binary_as_hex(s);
         return std::move(s);
      }
      case arguments::eTypeNumberGuid:
      {
         constexpr unsigned int uUuidSize = 16;
         //pbsz[uUuidSize] = '\0';
         _binary_to_hex(pbsz + uUuidSize * 2, (uint8_t*)m_unionValue.p + uUuidSize, uUuidSize);
      }
      break;
      default:
         assert(false);
      }

      if( s.empty() == true ) s = pbsz;
   }

   return std::move(s);
}


/*----------------------------------------------------------------------------- is_true */ /**
 * Checks to see if argument is a `true` value. When argument is a number then 0 = false, 
 * everything else ia true. Same with decimal values. For strings empty strings are false, 
 * and strings with text are true.
 * \return bool
 */
bool arguments::argument::is_true() const
{
   bool bTrue = false;

   switch( type_number_s(m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      bTrue = false;
      break;
   case arguments::eTypeNumberBool:
      bTrue = m_unionValue.b;
      break;
   case arguments::eTypeNumberInt8:
      bTrue = (m_unionValue.v_int8 != 0 ? true : false);
      break;
   case arguments::eTypeNumberUInt8:
      bTrue = (m_unionValue.v_uint8 != 0 ? true : false);
      break;
   case arguments::eTypeNumberInt16:
      bTrue = (m_unionValue.v_int16 != 0 ? true : false);
      break;
   case arguments::eTypeNumberUInt16:
      bTrue = (m_unionValue.v_uint16 != 0 ? true : false);
      break;
   case arguments::eTypeNumberInt32:
      bTrue = (m_unionValue.v_int32 != 0 ? true : false);
      break;
   case arguments::eTypeNumberUInt32:
      bTrue = (m_unionValue.v_uint32 != 0 ? true : false);
      break;
   case arguments::eTypeNumberInt64:
      bTrue = (m_unionValue.v_int64 != 0 ? true : false);
      break;
   case arguments::eTypeNumberUInt64:
      bTrue = (m_unionValue.v_uint64 != 0 ? true : false);
      break;
   case arguments::eTypeNumberFloat:
      bTrue = (m_unionValue.f != 0.0 ? true : false);
      break;
   case arguments::eTypeNumberDouble:
      bTrue = (m_unionValue.d != 0.0 ? true : false);
      break;
   case arguments::eTypeNumberString:
   case arguments::eTypeNumberUtf8String:
      bTrue = (m_unionValue.pbsz != nullptr && *m_unionValue.pbsz != '\0' ? true : false);
      break;
   case arguments::eTypeNumberWString:
      bTrue = (m_unionValue.pwsz != nullptr && *m_unionValue.pbsz != L'\0' ? true : false);
      break;
   default:
      assert(false);                                                             // Why are you here ????
   }

   return bTrue;
}

void arguments::argument_edit::set(const argument& argumentSet)
{
   
   m_pArguments->set(m_pValue, argumentSet.type(), (const_pointer)argumentSet.get_value_buffer(), argumentSet.length());
   /*
   auto eType = argumentSet.type_number();
   if( is_type_fixed_size_s(eType) == true && eType == type_number() )
   {
      auto pValueData = m_pValue + 1;                                            // move past type to data
      unsigned uSize = ctype_size[eType];
      memcpy(pValueData, argumentSet.get_value_buffer(), uSize);
   }
   else
   {
      m_pArguments->set((pointer)m_pPosition, argumentSet.type(), (const_pointer)argumentSet.get_value_buffer(), argumentSet.length());
   }

   */
}

/** ---------------------------------------------------------------------------
 * @brief Set value at position in arguments buffer
 * @param pposition pointer to valid position within arguments buffer 
 * @param argumentSet value to set
 */
void arguments::set(pointer pposition, const argument& argumentSet, tag_argument)
{                                                                                                  assert( pposition >= get_buffer_start() ); assert( pposition < get_buffer_end() );
   pointer ppositionValue = move_to_value_s( pposition );
   set(pposition, argumentSet.type(), (const_pointer)argumentSet.get_value_buffer(), argumentSet.length());
}

// 0TAG0construct.arguments

/*----------------------------------------------------------------------------- arguments */ /**
 * construct arguments with pair object having name and value
 * *sample*
~~~{.cpp}
   arguments test({ "test", 1010101 });
   auto result = test.get_argument("test");
   if( result == arguments::argument( 1010101 ) )
   {
      std::cout << "equal !!" << std::endl;
   }
~~~ 
*/
arguments::arguments(std::pair<std::string_view, gd::variant> pairArgument)
{
   zero();
   auto _argument = get_argument_s( pairArgument.second );
   append_argument(pairArgument.first, _argument);
}


/** ---------------------------------------------------------------------------
 * @brief Constructs an arguments object from an initializer list of string-variant pairs.
 * @param listPair An initializer list of pairs containing string views and gd::variant values.
 * Initializes the object by appending each pair from the list.
 */
arguments::arguments(std::initializer_list<std::pair<std::string_view, gd::variant>> listPair)
{
   zero();
   for( auto it : listPair ) append_argument(it);
}

/** ---------------------------------------------------------------------------
 * @brief Constructs an arguments object from an initializer list of string-variant_view pairs with a tag_view.
 * @param listPair An initializer list of pairs containing string views and gd::variant_view values.
 * @param tag_view A tag indicating the use of variant_view (distinguishes constructor overload).
 * Initializes the object by appending each pair from the list using the tag_view overload.
 */
arguments::arguments( std::initializer_list<std::pair<std::string_view, gd::variant_view>> listPair, tag_view )
{
   zero();
   for( auto it : listPair ) append_argument( it, tag_view{} );
}

/** ---------------------------------------------------------------------------
 * @brief Constructs an arguments object from a vector of string-variant_view pairs with a tag_view.
 * @param listPair A vector of pairs containing string views and gd::variant_view values.
 * @param tag_view A tag indicating the use of variant_view (distinguishes constructor overload).
 * Initializes the object by appending each pair from the vector using the tag_view overload.
 */
arguments::arguments( std::vector<std::pair<std::string_view, gd::variant_view>> vectorPair, tag_view )
{
   zero();
   append(vectorPair);
}

/** ---------------------------------------------------------------------------
* @brief Constructs an arguments object from an initializer list and another arguments object.
*
* appending key-value pairs from the provided initializer list, and appending all elements from another arguments object.
* 
* ```{.cpp}
void print( const gd::argument::shared::arguments arguments_ )
{
   std::cout << arguments_.print() << std::endl;
}

TEST_CASE( "add from arguments", "[arguments]" ) {
gd::argument::shared::arguments arguments_( { {"1", 1}, {"1", 1}, {"1", 1} } );

   // how to do a one liner
   print({ { {"2", 2}, { "3", 3 } }, arguments_});
   print({ arguments_, { {"1", 1}, {"1", 1} } });
}
* ```
*
* @param listPair An initializer list of pairs containing string views as keys and 
*                 gd::variant_view as values to be added to the arguments object.
* @param arguments_ A const reference to another arguments object whose contents 
*                   will be appended to this object.
*/
arguments::arguments(const std::initializer_list<std::pair<std::string_view, gd::variant_view>>& listPair, const arguments& arguments_ )
{
   zero();
   for( auto it : listPair ) append_argument(it, tag_view{});
   append(arguments_);
}

arguments::arguments(const arguments& arguments_ , const std::initializer_list<std::pair<std::string_view, gd::variant_view>>& listPair )
{
   zero();
   append(arguments_);
   for( auto it : listPair ) append_argument(it, tag_view{});
}

/** ---------------------------------------------------------------------------
 * @brief Constructs an arguments object from a single string-variant pair with a tag to bypass initializer list.
 * @param stringName The string view representing the argument name.
 * @param variantValue The gd::variant value associated with the argument name.
 * @param tag_no_initializer_list A tag to explicitly indicate this constructor does not use an initializer list.
 * Initializes the object by appending the single name-value pair.
 */
arguments::arguments(const std::string_view& stringName, const gd::variant& variantValue, arguments::tag_no_initializer_list)
{
   zero();
   append_argument(stringName, variantValue);
}

/// assign arguments with vector containing gd::variant_view values
arguments& arguments::operator=( const std::vector<gd::variant_view>& vectorValue)
{
   clear();
   return append(vectorValue);
}
/// assign arguments with vector containing pair of string and gd::variant values
arguments& arguments::operator=(const std::initializer_list<std::pair<std::string_view, gd::variant>>& listPair)
{
   clear();
   for( auto it : listPair ) append_argument(it);
   return *this;
}
/// assign arguments with vector containing pair of string and gd::variant_view values
arguments& arguments::operator=( const std::vector<std::pair<std::string_view, gd::variant_view>>& vectorPair)
{
   clear();
   return append(vectorPair);
}




/** ---------------------------------------------------------------------------
 * @brief index operator where editable argument is returned.
 * @code
TEST_CASE( "[gd] arguments using index", "[gd]" ) {
   gd::argument::arguments arguments_;
   arguments_.append("1", 1);
   arguments_.append("2", "2");
   arguments_.append("3", 3);
   arguments_.append("4", 4);
   arguments_.append("5", 5);
   arguments_.append_many( 100, 200, 300, 400, 500 );

   using namespace gd::argument;
   std::string_view stringName01 = "1";
   gd::argument::index index( stringName01 );
   auto edit_ = arguments_[index];
   auto edit1_ = arguments_["1"_index];
   assert( (int)edit_ == (int)edit1_ );
   arguments_[index] = 100;
   int iNumber1 = arguments_["1"];
   iNumber1 *= 2;
   arguments_[index] = iNumber1;
   int iNumber7a = arguments_[7];
   int iNumber7b = arguments_[7_index];
   assert( iNumber7a == iNumber7b );
}
 * @endcode
 * @param index_edit_ `index_edit` object used to get part within arguments
 * @return argument_edit for index value or empty argument_edit object if not found
 */
arguments::argument_edit arguments::operator[](const index_edit& index_edit_) 
{
   pointer pPosition = nullptr;

   if( index_edit_.is_string() == true )
   {
      pPosition = find( index_edit_.get_string() );
      if( index_edit_.is_second_index() == true )
      {
         pPosition = next_s( pPosition, index_edit_.get_second_index(), get_buffer_end());
      }
   }
   else if( index_edit_.is_index() == true )
   {
      pPosition = find( (unsigned)index_edit_.get_index() );
   }

   if( pPosition != nullptr )
   {
      return arguments::get_edit_param_s(this, pPosition);
   }

   return argument_edit();
}

// 0TAG0append.arguments

/// append argument value to arguments
/// tagged with tag_arguments to avoid conflicting with other append methods
arguments& arguments::append(const argument& argumentValue, tag_argument)
{
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();
   if( uType > ARGUMENTS_NO_LENGTH ) { uType |= eValueLength; }

   return append( uType, pData, argumentValue.length() );
}

/// append variant_view value to arguments
/// tagged with tag_view to avoid conflicting with other append methods
arguments& arguments::append(const gd::variant_view& variantValue, tag_view)
{
   auto argumentValue = get_argument_s(variantValue);
   return append( argumentValue, tag_argument{});
}


/*-----------------------------------------------------------------------------
 * @brief add arguments from another arguments object
 * @param argumentsFrom arguments object to add values from
 * @return arguments& reference to arguments object
 */
arguments& arguments::append(const arguments& argumentsFrom)
{
   for( auto pPosition = argumentsFrom.next(); pPosition != nullptr; pPosition = argumentsFrom.next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
         auto name_ = arguments::get_name_s(pPosition);
         auto argument_ = arguments::get_argument_s( pPosition );
         append_argument( name_, argument_ );
      }
   }

   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief append values to arguments from semicolon separated string
 * This method adds values from string, values are converted to proper type but
 * you need to make sure that the format is ok.
 * format: `name,type,value;name,type,value;name,type,value`
 * example: `one,int32,1000;two,double,.234;city,string,London`
 * @code
gd::argument::arguments argumentsTest;
argumentsTest.append( "one,int32,1000;two,double,.234;city,string,London", gd::argument::tag_parse{} );
std::string stringData = gd::argument::debug::print( argumentsTest );
std::cout << stringData;
 * @endcode
 * @param stringValue string with values to add
 * @return reference to arguments
 */
std::pair<bool, std::string> arguments::append(const std::string_view& stringValue, tag_parse)
{
   std::vector<std::size_t> vectorOffset;       // positions for each value in string
   std::vector<std::size_t> vectorValue;        // positions for value part in string
   std::vector<std::string_view> vectorValueData;// value part information

   gd::utf8::offset( stringValue, ';', vectorOffset );                         // offset value positions, they are divided with ';', all ';' are marked (distance from start of string)
   if( stringValue.back() != ';' ) vectorOffset.push_back(stringValue.length());// add last position to add last section without code after loop

   std::size_t uFrom = 0;  // start offset for column properties walking columns in string
   std::size_t uTo;        // end offset for column walking columns in string
   for( auto itField : vectorOffset )
   {
      uTo = itField;                                                           // end position for column

      // Check length, if negative or zero the format is invalid
      if( (uTo - uFrom) <= 0 ) return { false, std::string( stringValue ) };

      std::string_view stringArgument( stringValue.data() + uFrom, uTo - uFrom );// select column properties in string with column information
      gd::utf8::offset( stringArgument, ',', vectorValue );                    // set offset positions where string is splitted
      gd::utf8::split( stringArgument, vectorValue, vectorValueData );         // split column properties in to vector of strings

      std::string_view stringName = vectorValueData.at( 0 );
      std::string_view stringType = vectorValueData.at( 1 );
      gd::variant_view value_ = vectorValueData.at( 2 );
      unsigned uType = gd::types::type_g( stringType );
      if(uType != 0)
      {
         if( gd::types::detail::type_group_g( uType ) == gd::types::eTypeGroupString )
         {
            if(uType == gd::types::eTypeUtf8String || uType == gd::types::eTypeString)
            {
               value_.set_type( uType );
               append_argument( stringName, value_ );
            }
         }
         else
         {
            gd::variant variantValue = value_.convert_to( uType );
            append_argument( stringName, variantValue );
         }
      }

      uFrom = uTo + 1;                                                         // move to next value

      // ## clear to prepare for next value
      vectorValue.clear();
      vectorValueData.clear();
   }


   return { true, "" };
}

arguments::pointer arguments::insert(pointer pPosition, argument_type uType, const_pointer pBuffer, unsigned int uLength)
{
   uint64_t uReserveLength = buffer_size();                                    // current buffer size
   uReserveLength += uLength + sizeof(uint32_t) * 2;                           // value length (and prefix value length for strings)
   uint64_t uOffset = pPosition - buffer_data();

   reserve( uReserveLength );

   auto pdata_ = buffer_data() + uOffset;

   return nullptr;
}


/*-----------------------------------------------------------------------------
 * Add typed argument to binary stream of bytes
 * \param uType type of parameter
 * \param pBuffer pointer to binary data
 * \param uLength number of bytes added to stream
 * \return arguments& reference to arguments object
 */
arguments& arguments::append( argument_type uType, const_pointer pBuffer, unsigned int uLength)
{ 
   uint64_t uReserveLength = buffer_size();                                    // current buffer size
   uReserveLength += uLength + sizeof(uint32_t) * 2;                           // value length (and prefix value length for strings)
   uReserveLength += sizeof( uint16_t ) + sizeof(uint32_t);                    // value type and value length if needed
   uReserveLength = (uReserveLength + 3) & ~3;                                 // align

   reserve( uReserveLength );

   auto uPosition = buffer_size();                                                                 assert( uPosition % 4 == 0 ); // get active position for next value in buffer
#ifndef NDEBUG
   auto uBegin_d = uPosition;                                                  // used to calculate the total size for value stored in arguments object
#endif // _DEBUG
   auto pdata_ = buffer_data();

   pdata_ += uPosition;
   auto uCopySize = memcpy_s( pdata_, uType, pBuffer, uLength );

   uPosition += uCopySize;
   buffer_set_size( uPosition );                                                                   assert(buffer_size() < buffer_buffer_size());

#ifndef NDEBUG
   auto uValueSize_d = uPosition - uBegin_d;
#endif // _DEBUG
   return *this;
}


/*----------------------------------------------------------------------------- append */ /**
 * Add typed value to arguments
 * @note This methods does the "meat" adding named values to arguments, it is advanced".
 *       To understand this method you need to know the internal memory structure of `arguments`
 *       and know how the CPU works, it is optimized. Each value is typed and holds its size
 *       to make it faster to move from one value to next.
 * 
 * Name for value is in ascii format with beginning marker that for name name, then length for name
 * and the name. After name type of value is added and the value size needed to store value data in memory.
 * Then the value or if value needs length the length is added.
 * Lastly data for value is added to memory.
 * *In memory it looks like this:*
 * [name type][name length][name text][value type]{native value length}[value data]
 * \param pbszName name for value
 * \param uNameLength character count for name
 * \param uType type of value (a byte value)
 * \param pBuffer data, pointer to bytes holding data
 * \param uLength length for data
 * \return arguments& argument object for nested calls
 */
arguments& arguments::append(const char* pbszName, uint32_t uNameLength, argument_type uType, const_pointer pBuffer, unsigned int uLength)
{                                                                                                  assert(m_pbuffer->get_reference_count() <= 1); assert( uNameLength < 0x1000 ); // no change if two or more holds value and realistic name lenghts
#ifndef NDEBUG
   enumCType eType_d = (enumCType)(uType & ~eTypeNumber_MASK);                 // get absolute variable type
   auto typename_d = gd::types::type_name_g( eType_d );                        // readable name for type
#endif // _DEBUG

   // ## calculate needed size to make sure internal buffer is large enough
   uint64_t uReserveLength = buffer_size();                                    // current buffer size
   /// ### [name type and nanme lenghth][name value]{value type and value buffer length]{native value length}[value data] = total byte count needed to store value
   uReserveLength += uNameLength + sizeof( uint32_t );                         // name if any and length value for name
   uReserveLength += uLength + sizeof(uint32_t);                               // value length (and prefix value length for strings)
   uReserveLength += sizeof(uint32_t) * 3;                                     // padding space to enable space for 32 bit alignment (name and value is aligned) and four bytes extra
   uReserveLength = align32_g( uReserveLength );                               // align
      
   // reserve data for name, length for name, name type, data type and data value
   // uReserveLength should be enough for -> [name type][name length][name text][value type]{native value length}[value data]
   reserve( uReserveLength );

   auto uPosition = buffer_size();                                                                 assert( uPosition % 4 == 0 );
#ifndef NDEBUG
   auto uBegin_d = uPosition;                                                  // keep position to check total length for value in debug
#endif // _DEBUG
   auto pdata_ = buffer_data();

   uint32_t uTypeAndSize = (eType_ParameterName << 24) | uNameLength;
   *(uint32_t*)(pdata_ + uPosition) = uTypeAndSize;                            // set name type
   uPosition += sizeof( uint32_t );                                            // move past key type and length (stored in 32 bit value)
   memcpy(&pdata_[uPosition], pbszName, uNameLength);                          // copy name
   uNameLength = align32_g( uNameLength );                                     // align if needed
   uPosition += uNameLength;                                                   // add length for name
                                                                                                   assert( uPosition % 4 == 0 );
   if( (uType & eValueLength) == 0 )                                           // if type doesn't have specified length flag then just copy data into buffer
   {
      uint32_t uValueLength = uLength;
      uLength = align32_g( uLength );                                          // align to 32 bit boundary
      uTypeAndSize = (uType << 24) | uLength;
      *(uint32_t*)(pdata_ + uPosition) = uTypeAndSize;                         // set type and size
      uPosition += sizeof( uint32_t );

      memcpy(&pdata_[uPosition], pBuffer, uValueLength);                       // copy value data
      uPosition += uLength;                                                    // add aligned length
      buffer_set_size( uPosition );                                                                assert(buffer_size() < buffer_buffer_size());
   }
   else
   {
      uint32_t uValueLength = uLength;                                         // value length in bytes (storage needed to hold data)
      uLength += sizeof( uint32_t );                                           // add value length to total value size
      uLength = align32_g( uLength );                                          // align to 32 bit boundary
      uTypeAndSize = (uType << 24) | uLength;                                  // set value type and length in 32 bit value
      *(uint32_t*)(pdata_ + uPosition) = uTypeAndSize;                         // set type and size
      uPosition += sizeof( uint32_t );                                         // move past type and size

      uint32_t uCompleteType = gd::types::typenumber_to_type_g( uType & ~eType_MASK );// get the full type information from gd types to investigate the object length

      // ## fix size to the actual length for value, this is to improve the speed
      //    generating value objects from data
      if(uCompleteType & gd::types::eTypeGroupString)
      {
         if(( uType & eTypeNumber_MASK ) == eTypeNumberWString)
         {                                                                                         assert( (uValueLength % 2) == 0 );// make sure unicode is correct
            uValueLength = uValueLength >> 1;                                  // unicode string, length is cut in half
         }
      }

      *(uint32_t*)(pdata_ + uPosition) = uValueLength;
      memcpy(&pdata_[uPosition + sizeof( uint32_t )], pBuffer, uLength);       // copy data
      uPosition += uLength;                                                    // move past data for value (length and data)
      buffer_set_size( uPosition );                                                                assert(buffer_size() < buffer_buffer_size());
   }

#ifndef NDEBUG
   auto uValueSize_d = uPosition - uBegin_d;                                   // total size needed for value in `arguments` object
#endif // _DEBUG
   return *this;
}

/*----------------------------------------------------------------------------- append_argument */ /**
 * Add argument from variant, this value isn't named
 * \param variantValue argument value added
 * \return arguments::arguments& reference to this if nested operations is wanted
 */
arguments& arguments::append_argument(const variant& variantValue)
{
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();
   if( uType > ARGUMENTS_NO_LENGTH ) { uType |= eValueLength; }

   return append(uType, pData, argumentValue.length());
}

/** --------------------------------------------------------------------------
 * Appends an argument with an optional name and value to the arguments collection.
 * @description Adds a named or unnamed argument to the collection based on the provided name and value.
 *              Handles different value types, adjusting type flags and lengths as needed (e.g., for strings or binary data).
 * @param {const std::string_view&} stringName - The name of the argument (optional; empty if unnamed).
 * @param {const gd::variant_view&} variantValue - The value of the argument, stored as a variant view.
 * @returns {arguments&} A reference to the current arguments object for chaining.
 */
arguments& arguments::append_argument(const std::string_view& stringName, const gd::variant_view& variantValue)
{
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();                               // get type for value
   unsigned uLength;                                                           // length for value
   if( stringName.empty() == false )                                           // if name is given then add name to value
   {
      if( uType > ARGUMENTS_NO_LENGTH )
      {
         unsigned uZeroEnd = 0;
         if( uType >= eTypeNumberString && uType <= eTypeNumberBinary ) { uType |= eValueLength; }

         uLength = variantValue.length();

         return append(stringName, uType, pData, uLength);
      }
      else
      {
         return append(stringName, uType, pData, argumentValue.length());
      }
   }

   // ## no name, just add value

   if( uType > ARGUMENTS_NO_LENGTH )
   {
      unsigned uZeroEnd = 0;
      if( uType >= eTypeNumberString && uType <= eTypeNumberBinary ) { uType |= eValueLength; }

      uLength = variantValue.length();

      return append(uType, pData, uLength);
   }

   return append(uType, pData, argumentValue.length());
}

/** --------------------------------------------------------------------------
 * @brief Appends a named argument with a string value to the arguments collection, converting it to an appropriate type.
 *
 * This method takes a name and a string value, detects the value’s type (e.g., integer, floating-point, or string), 
 * and appends it to the collection as a variant. It attempts to convert the string to an integer or double if applicable, 
 * falling back to the original string value otherwise.
 *
 * @param stringName The name of the argument (can be empty for unnamed arguments).
 * @param stringValue The string value to append, which will be type-detected and converted.
 * @param tag_parse_type Tag for overload resolution, typically an empty struct.
 * @return arguments& A reference to the current arguments object for chaining.
 * @note The method uses type detection to determine if the string represents an integer or floating-point value.
 */
arguments& arguments::append_argument(const std::string_view& stringName, const std::string_view& stringValue, tag_parse_type )
{
   gd::variant_view v_ = stringValue;
   unsigned uTypeGroup = gd::types::detect_ctypegroup_g( stringValue );
   if( uTypeGroup & gd::types::eTypeGroupInteger )
   {
      auto to_ = v_.convert_to( gd::types::type_g( "int64" ) );
      append_argument( stringName, to_ );
   }
   else if( uTypeGroup & gd::types::eTypeGroupInteger )
   {
      auto to_ = v_.convert_to( gd::types::type_g( "double" ) );
      append_argument( stringName, to_ );
   }
   else { append_argument( stringName, v_ ); }

   return *this;
}

/// @brief Appends a named argument with a string value to the arguments collection, converting it to an appropriate type.
arguments& arguments::append_argument( const std::initializer_list< std::pair<std::string_view, gd::variant_view> >& vectorArgument, tag_view )
{
   for( const auto& it : vectorArgument )
   {
      append_argument( it, tag_view{});
   }

   return *this;
}

/// @brief Appends a named argument with a string value to the arguments collection, converting it to an appropriate type.
arguments& arguments::append_argument( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorArgument, tag_view )
{
   for( const auto& it : vectorArgument )
   {
      append_argument( it, tag_view{});
   }

   return *this;
}

// 0TAG0set.arguments

/** ---------------------------------------------------------------------------
 * @brief Sets or updates a named value in the arguments buffer.
 * 
 * This method searches for a value with the specified name in the arguments buffer. 
 * If the value exists, it updates the value in place if the type and size match. 
 * Otherwise, it replaces the existing value, resizing the buffer if necessary. 
 * If the value does not exist, it appends the new value to the buffer.
 * 
 * @param pbszName Pointer to the name of the value to set.
 * @param uNameLength Length of the name in bytes.
 * @param uType Type of the value to set.
 * @param pBuffer Pointer to the data buffer containing the value.
 * @param uLength Length of the value in bytes.
 * @return arguments& Reference to the current arguments object for chaining.
 * 
 * @note If the value type is fixed-size and matches the existing type, the value is updated in place.
 *       Otherwise, the buffer is resized to accommodate the new value.
 * 
 * @warning Ensure that the provided pointers and lengths are valid and within bounds.
 */
arguments& arguments::set(const char* pbszName, uint32_t uNameLength, param_type uType, const_pointer pBuffer, unsigned int uLength)
{
   pointer pPosition = find(std::string_view(pbszName, uNameLength));
   if( pPosition == nullptr )
   {  // value was not found, just add it
      return append( pbszName, uNameLength, uType, pBuffer, uLength );
   }

   // ## Found value - replace
   // get current argument
   argument argumentOld = arguments::get_argument_s(pPosition);

   // check if type is equal and fixed length, then just copy new value over the old one.
   if( arguments::compare_type_s(argumentOld, uType) == true && (uType & (eValueLength|eValueLengthBig)) == 0 )
   {
      pPosition = move_to_value_s( pPosition );
      pPosition += sizeof( uint32_t );                                         assert(pPosition < get_buffer_end());
      memcpy(pPosition, pBuffer, uLength);
      return *this;
   }
   else
   {
      auto uOldSize = arguments::sizeof_name_s( uNameLength );
      uOldSize = align32_g( uOldSize );
      uOldSize += arguments::sizeof_s( argumentOld );
      uOldSize = align32_g( uOldSize );

      auto uNewSize = arguments::sizeof_s( uNameLength, uType, uLength );      // calculate total size for new value

      if( uOldSize != uNewSize ) 
      { 
         if( uNewSize > uOldSize ) 
         { 
            size_t uOffset = pPosition - buffer_data();                        // offset value from start of buffer, needed to set new position if new block is allocated
            
            // reserv memory if needed and if memory is reserved, reset position
            if( reserve( buffer_size() + uNewSize ) == true ) 
            { 
               pPosition = buffer_data() + uOffset;                            // reset position
            }
         }

         resize(pPosition, uOldSize, uNewSize); 
      }

      pPosition = move_to_value_s( pPosition );                                                    assert( ((intptr_t)pPosition % 4) == 0 );

      if( (uType & eValueLength) == 0 )                                        // if type doesn't have specified length flag then just copy data into buffer
      {
         uint32_t uValueLength = uLength;                                      // hold value length
         uLength = align32_g( uLength );                                       // align to 32 bit boundary
         uint32_t uTypeAndSize = (uType << 24) | uLength;
         *(uint32_t*)(pPosition) = uTypeAndSize;                               // set type and size
         pPosition += sizeof( uint32_t );

         memcpy(pPosition, pBuffer, uValueLength);
      }
      else
      {
         unsigned uTotalLength = uLength;
         uint32_t uValueLength = uLength;                                      // value length in bytes (storage needed to hold data)
         uTotalLength += sizeof( uint32_t );                                   // add value length to total value size
         uTotalLength = align32_g( uTotalLength );                             // align to 32 bit boundary
         uint32_t uTypeAndSize = (uType << 24) | uTotalLength;                 // set value type and length in 32 bit value
         *(uint32_t*)(pPosition) = uTypeAndSize;                               // set type and size
         pPosition += sizeof( uint32_t );                                      // move past type and size

         uint32_t uCompleteType = gd::types::typenumber_to_type_g( uType & ~eType_MASK );// get the full type information from gd types to investigate the object length

         // ## fix size to the actual length for value, this is to improve the speed
         //    generating value objects from data
         if(uCompleteType & gd::types::eTypeGroupString)
         {
            if(( uType & eTypeNumber_MASK ) == eTypeNumberWString)
            {                                                                                      assert( (uValueLength % 2) == 0 );// make sure unicode is correct
               uValueLength = uValueLength >> 1;                               // unicode string, length is cut in half
            }
         }
         *(uint32_t*)(pPosition) = uValueLength;
         pPosition += sizeof( uint32_t );

         memcpy(pPosition, pBuffer, uLength);                                  // copy data
      }
   }

   return *this;
}

arguments& arguments::set(pointer pPosition , param_type uType, const_pointer pBuffer, unsigned int uLength, pointer* ppPosition)
{
   // get current argument
   argument argumentOld = arguments::get_argument_s(pPosition);

   // check if type is equal and fixed length, then just copy new value over the old one.
   if( arguments::compare_type_s(argumentOld, uType) == true && (uType & (eValueLength|eValueLengthBig)) == 0 )
   {
      pPosition = move_to_value_s( pPosition );
      pPosition += sizeof( uint32_t );                                         assert(pPosition < get_buffer_end());
      memcpy(pPosition, pBuffer, uLength);
      return *this;
   }
   else
   {
      unsigned uOldSize = 0;
      unsigned uNewSize = 0;
      if( is_name_s(pPosition) == true ) 
      {
         uOldSize = arguments::sizeof_name_s( pPosition );
         uOldSize = align32_g( uOldSize );
         uNewSize = uOldSize;
      }
      
      uOldSize += arguments::sizeof_s( argumentOld );
      uOldSize = align32_g( uOldSize );

      uNewSize += arguments::sizeof_s( 0, uType, uLength );                    // calculate total size for new value

      if( uOldSize != uNewSize ) 
      { 
         if( uNewSize > uOldSize ) 
         { 
            size_t uOffset = pPosition - buffer_data();                        // offset value from start of buffer, needed to set new position if new block is allocated
            
            // reserv memory if needed and if memory is reserved, reset position
            if( reserve( buffer_size() + uNewSize ) == true ) 
            { 
               pPosition = buffer_data() + uOffset;                            // reset position
            }
         }

         resize(pPosition, uOldSize, uNewSize); 
      }

      pPosition = move_to_value_s( pPosition );                                                    assert( ((intptr_t)pPosition % 4) == 0 );

      if( (uType & eValueLength) == 0 )                                        // if type doesn't have specified length flag then just copy data into buffer
      {
         uint32_t uValueLength = uLength;                                      // hold value length
         uLength = align32_g( uLength );                                       // align to 32 bit boundary
         uint32_t uTypeAndSize = (uType << 24) | uLength;
         *(uint32_t*)(pPosition) = uTypeAndSize;                               // set type and size
         pPosition += sizeof( uint32_t );

         memcpy(pPosition, pBuffer, uValueLength);
      }
      else
      {
         unsigned uTotalLength = uLength;
         uint32_t uValueLength = uLength;                                      // value length in bytes (storage needed to hold data)
         uTotalLength += sizeof( uint32_t );                                   // add value length to total value size
         uTotalLength = align32_g( uTotalLength );                             // align to 32 bit boundary
         uint32_t uTypeAndSize = (uType << 24) | uTotalLength;                 // set value type and length in 32 bit value
         *(uint32_t*)(pPosition) = uTypeAndSize;                               // set type and size
         pPosition += sizeof( uint32_t );                                      // move past type and size

         uint32_t uCompleteType = gd::types::typenumber_to_type_g( uType & ~eType_MASK );// get the full type information from gd types to investigate the object length

         // ## fix size to the actual length for value, this is to improve the speed
         //    generating value objects from data
         if(uCompleteType & gd::types::eTypeGroupString)
         {
            if(( uType & eTypeNumber_MASK ) == eTypeNumberWString)
            {                                                                                      assert( (uValueLength % 2) == 0 );// make sure unicode is correct
               uValueLength = uValueLength >> 1;                               // unicode string, length is cut in half
            }
         }
         *(uint32_t*)(pPosition) = uValueLength;
         pPosition += sizeof( uint32_t );

         memcpy(pPosition, pBuffer, uLength);                                  // copy data
      }
   }

   if( ppPosition != nullptr ) *ppPosition = pPosition;

   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief set value at position that pointer is at, make sure that pPosition is on a valid position
 * @param pPosition pointer to position where value is set
 * @param uType type of value, its a byte value (uint8_t)
 * @param pBuffer pointer to buffer with data that is copied into arguments buffer
 * @param uLength size in bytes for data to copy
 * @return position where the value was copied to
 */
arguments::pointer arguments::set(pointer pPosition, param_type uType, const_pointer pBuffer, unsigned int uLength, tag_internal )
{                                                                                                  assert( pPosition >= buffer_data() ); assert( pPosition < buffer_data_end() );
   argument argumentOld = arguments::get_argument_s(pPosition);                // get current argument
   auto pPositionValue = move_to_value_s(pPosition);
   if( arguments::compare_type_s(argumentOld, uType) == true && (uType & (eValueLength | eValueLengthBig)) == 0 ) // same value with fixed size?
   {
      pPositionValue += sizeof( uint32_t );                                    // move to buffer where value is stored
      memcpy(pPositionValue, pBuffer, uLength);
   }
   else
   {                                                                                               assert( (pPositionValue - pPosition) < 255 ); // realistic
      auto uOldSize = get_total_param_length_s(pPosition);
      unsigned uNameLength = arguments::sizeof_name_s( pPosition );
      auto uNewSize = uNameLength + uLength + sizeof_value_prefix( uType );
      if( uOldSize != uNewSize ) 
      { 
         if( uOldSize < uNewSize ) reserve( uNewSize );                        // increase buffer if needed

         resize(pPosition, uOldSize, uNewSize); 
      }

      pPosition += uNameLength;
      *pPosition = uType;
      pPosition++;

      if( (uType & eValueLength) == 0 )                                        // if type doesn't have specified length flag then just copy data into buffer
      {
         memcpy(pPosition, pBuffer, uLength);
      }
      else
      {
         *(uint32_t*)pPosition = (uint32_t)uLength;                            // set length for data
         pPosition += sizeof(uint32_t);
         memcpy(pPosition, pBuffer, uLength);                                  // copy data
      }
   }

   return pPosition;
}

// 0TAG0insert.arguments

/** ---------------------------------------------------------------------------
 * @brief Insert named value at position in arguments buffer
 * @code
 * gd::argument::shared::arguments arguments_;
 * arguments_.append_many( 100, 200, 300, 400, 500 );
 * // insert named value at position 2, value name is "value-name" and value is 250
 * arguments_.insert( 2, "value-name", 250, gd::argument::shared::arguments::tag_view{});
 * std::cout << arguments_.print() << "\n";
 * @endcode
 * @param uIndex index where value is inserted
 * @param stringName name for value that is inserted
 * @param variantviewValue value to be inserted
 * @return position where the original value was
 */
arguments::pointer arguments::insert(size_t uIndex, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_view )
{
   pointer pposition = find( (unsigned)uIndex );
   if(pposition != nullptr)
   {
      pposition = insert( pposition, stringName, variantviewValue, tag_view{} );
   }
   else
   {
      append_argument( stringName, variantviewValue, tag_view{} );
      pposition = get_buffer_end();
   }

   return pposition;
}

/** ---------------------------------------------------------------------------
 * @brief insert variant value at position
 * @param pPosition position where value is inserted, values after are moved one step to end
 * @param variantviewValue value to insert
 * @return pointer new position where the value that `pPosition` pointed to
 */
arguments::pointer arguments::insert(pointer pPosition, const gd::variant_view& variantviewValue, tag_view)
{                                                                                                  assert( pPosition >= buffer_data() ); assert( pPosition <= buffer_data_end() );
#ifndef NDEBUG
   // auto string_d = debug::print( *this );
#endif // !NDEBUG

   uint64_t uOffset = pPosition - buffer_data();                                                   assert( uOffset < buffer_size() );
   // ## calculate expand size   
   unsigned uSizeInsert = sizeof_s( variantviewValue, tag_view{}) ;
   uSizeInsert = align32_g( uSizeInsert );

   reserve( buffer_size() + uSizeInsert );                                     // increase size if needed

   pPosition = buffer_data() + uOffset;                                        // reset pointer to buffer where to insert value

   uint64_t uMoveSize = buffer_size() - uOffset;                               // calculate memory size to move
   memmove( pPosition + uSizeInsert, pPosition, uMoveSize );                   // move memory

   auto argumentValue = get_argument_s(variantviewValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();

   uint64_t uByteCount = memcpy_s( pPosition, uType, pData, uSizeInsert - sizeof( uint32_t ) ); // copy data, decrease size with size needed to describe
   pPosition += uByteCount;                                                                        assert( uSizeInsert == uByteCount );
   buffer_set_size( buffer_size() + uByteCount );                              // increase used buffer size

#ifndef NDEBUG
   // string_d = debug::print( *this );
#endif // !NDEBUG

   return pPosition;
}

/** ---------------------------------------------------------------------------
 * @brief insert value at position, position need to point to start of value
 * @code
 gd::argument::shared::arguments arguments_;
 arguments_.append_many( 100, 200, 300, 400, 500 );
 arguments_.insert( 2, "test", 250, gd::argument::shared::arguments::tag_view{});
 std::cout << arguments_.print() << "\n";
 * @emdcode
 * @param pPosition position in internal buffer holding arguments value
 * @param stringName name for value to insert
 * @param variantviewValue value to insert
 * @return pointer to same value that position was at from start
 */
arguments::pointer arguments::insert(pointer pPosition, const std::string_view& stringName, const gd::variant_view& variantviewValue, tag_view)
{                                                                                                  assert( pPosition >= buffer_data() ); assert( pPosition <= buffer_data_end() );
#ifndef NDEBUG
   auto string_d = debug::print( *this );
#endif // !NDEBUG

   // calculate offset from start of buffer whre value is to be inserted
   uint64_t uOffset = pPosition - buffer_data();                                                   assert( uOffset < buffer_size() );
   // ## calculate expand size   
   unsigned uSizeInsert = sizeof_s( stringName, variantviewValue, tag_view{}) ;
   uSizeInsert = align32_g( uSizeInsert );

   reserve( buffer_size() + uSizeInsert );                                     // increase size if needed

   pPosition = buffer_data() + uOffset;                                        // reset pointer to buffer where to insert value

   // ## move memory to make space for new value
   uint64_t uMoveSize = buffer_size() - uOffset;                               // calculate memory size to move
   memmove( pPosition + uSizeInsert, pPosition, uMoveSize );                   // move memory

   // ## insert name for value
   uint64_t uByteCount = memcpy_s( pPosition, stringName.data(), (unsigned)stringName.length() );
   pPosition += uByteCount;

   // ## get pointer to value in argument
   auto argumentValue = get_argument_s(variantviewValue);                      // convert to argument
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();

   // ## copy value
   uSizeInsert = sizeof_s( variantviewValue, tag_view{} ) ;
   uByteCount += memcpy_s( pPosition, uType, pData, uSizeInsert - sizeof( uint32_t ) ); // copy data, decrease size with size needed to describe
   pPosition += uByteCount;                                                                        
   buffer_set_size( buffer_size() + uByteCount );                              // increase used buffer size

#ifndef NDEBUG
   string_d = debug::print( *this );
#endif // !NDEBUG

   return pPosition;
}

// 0TAG0merge.arguments

/** ---------------------------------------------------------------------------
 * @brief Merge two arguments objects
 @code
 void print( const gd::argument::arguments arguments_ )
{
   std::cout << arguments_.print() << std::endl;
}

TEST_CASE( "merge", "[arguments]" ) {
   gd::argument::arguments arguments_( { {"A", "A"}, {"B", "B"}, {"C", "C"} } );
   gd::argument::arguments argumentsMerge( { {"B", "B"}, {"C", "C"}, {"D", "D"}, {"E", "E"} } );
   arguments_.merge(argumentsMerge);
   print(arguments_);  // prints "A": A, "B": B, "C": C, "D": D, "E": E
}
@endcode
 * @param argumentsFrom arguments object to merge with
 * @return arguments& reference to this
 */ 
arguments& arguments::merge(const arguments& argumentsFrom)
{
   for( auto it = std::begin(argumentsFrom); it != std::end(argumentsFrom); it++ )
   {
      if( it.is_name() == true )
      {
         if( find(it.name()) == nullptr )
         {
            append_argument(it.name(), it.get_argument());
         }
      }
   }
   return *this;
}


/*----------------------------------------------------------------------------- count */ /**
 * Count param values for name
 * \param stringName name that is counted
 * \return unsigned int number of param values found for name
 */
unsigned int arguments::count(std::string_view stringName) const
{
   unsigned int uCount = 0;
   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
         if( arguments::get_name_s(pPosition) == stringName ) uCount++;
      }
   }

   return uCount;
}

// 0TAG0find.arguments

/*----------------------------------------------------------------------------- find */ /**
 * Get position to parameter for index
 * \param uIndex index to value within arguments object
 * \return gd::argument::arguments::pointer position to name if found, otherwise null
 */
arguments::pointer arguments::find( unsigned int uIndex )
{
   pointer pPosition = buffer_data();
   while( uIndex > 0 && (pPosition = next(pPosition)) != nullptr )
   {
      uIndex--;
   }

   return pPosition;
}


/*----------------------------------------------------------------------------- find */ /**
 * Get position to parameter for index
 * with the name the first is taken
 * \param uIndex index to value within arguments object
 * \return gd::argument::arguments::const_pointer position to name if found, otherwise null
 */
arguments::const_pointer arguments::find( unsigned int uIndex ) const
{
   const_pointer pPosition = buffer_data();
   while( uIndex > 0 && (pPosition = next(pPosition)) != nullptr )
   {
      uIndex--;
   }

   return pPosition;
}


/*----------------------------------------------------------------------------- find */ /**
 * Try to find pointer in arguments that match name, if there are more than one parameter
 * with the name the first is taken
 * \param stringName
 * \return gd::argument::arguments::const_pointer position to name if found, otherwise null
 */
arguments::pointer arguments::find(const std::string_view& stringName)
{
   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
#ifdef _DEBUG
         auto name_d = arguments::get_name(pPosition);
#endif // _DEBUG

         if( arguments::get_name(pPosition) == stringName ) return pPosition;
      }
   }

   return nullptr;
}


/*----------------------------------------------------------------------------- find */ /**
 * Try to find parameter in params that match name, if there are more than one parameter
 * with the name the first is taken
 * \param stringName
 * \return gd::argument::arguments::const_pointer position to name if found, otherwise null
 */
arguments::const_pointer arguments::find(const std::string_view& stringName) const
{
   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
         auto key_ = arguments::get_name_s(pPosition);
         if( key_ == stringName ) return pPosition;
      }
   }

   return nullptr;
}


/*----------------------------------------------------------------------------- find */ /**
 * Try to find parameter in params that match name, if there are more than one parameter
 * with the name the first is taken
 * \param stringName name to find
 * \param pOffsetPosition position where to start from, like offset position
 * \return gd::argument::arguments::const_pointer position to name if found, otherwise null
 */
arguments::const_pointer arguments::find(std::string_view stringName, const_pointer pOffsetPosition) const
{                                                                                                  assert( pOffsetPosition >= get_buffer_start() ); assert( pOffsetPosition < get_buffer_end() );
   for( ; pOffsetPosition != nullptr; pOffsetPosition = next(pOffsetPosition) )
   {
      if( arguments::is_name_s(pOffsetPosition) == true )
      {
         auto key_ = arguments::get_name_s(pOffsetPosition);
         if( key_ == stringName ) return pOffsetPosition;
      }
   }

   return nullptr;
}

arguments::const_pointer arguments::find(const std::pair<std::string_view, gd::variant_view>& pairMatch) const
{
   const_pointer pPosition = find(pairMatch.first);
   if( pPosition != nullptr )
   {
      argument argumentFind = get_argument_s(pPosition);
      if( argumentFind.compare( pairMatch.second ) == true ) return pPosition;
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief finds value within section and section is a name and the trailing values without name
 * @param pairMatch name for section and value that is searched for
 * @return const_pointer pointer to value if value is found or nullptr if not found
 */
arguments::const_pointer arguments::find(const std::pair<std::string_view, gd::variant_view>& pairMatch, tag_section ) const
{
   const_pointer pPosition = find(pairMatch.first);
   if( pPosition != nullptr )
   {
      argument argumentFind = get_argument_s(pPosition);
      if( argumentFind.compare( pairMatch.second ) == true ) return pPosition;

      /// try to find value in among values that follow without name, stop searching when new name is found
      while(( pPosition = next(pPosition) ) != nullptr)
      {
         if(arguments::is_name_s(pPosition) == false)
         {
            argumentFind = get_argument_s(pPosition);
            if( argumentFind.compare( pairMatch.second ) == true ) return pPosition;
         }
         else { break; }
      }
   }

   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief Tries to find two values with same name and return those two in pair object
 * This is more av of conveniance method to find two related values (same name) and put them in a pair object
 * @param stringName name that two values are searced for
 * @return std::pair<arguments::argument, arguments::argument> pair object with values found for name
 */
std::pair<arguments::argument, arguments::argument> arguments::find_pair(const std::string_view& stringName) const
{
   unsigned uCount = 0;
   std::pair<arguments::argument, arguments::argument> pair_;

   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
         if(arguments::get_name_s(pPosition) == stringName)
         {
            if( uCount == 0 ) { pair_.first = arguments::get_argument_s( pPosition ); uCount++; }
            else
            {
               pair_.second = arguments::get_argument_s( pPosition );
               break;
            }
         }
      }
   }

   return pair_;
}

/*----------------------------------------------------------------------------- find_all */ /**
 * Find parameters for name, place them in vector and return
 * If there are multiple values with same name and you want to get all values for name
 * then this can deliver positions for those values and remove the need to search for each
 * value.
 * \param stringName name for parameter
 * \return std::vector<gd::argument::arguments::const_pointer> vector where parameters are placed
 */
std::vector<arguments::const_pointer> arguments::find_all(std::string_view stringName) const
{
   std::vector<arguments::const_pointer> vectorP;
   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
         if( arguments::get_name_s(pPosition) == stringName ) vectorP.push_back( pPosition );
      }
   }

   return vectorP;
}

/**----------------------------------------------------------------------------
 * @brief Find named arguments from vector with names
 * @param vectorName list of names values are returned for
 * @return std::vector<arguments::argument> vector with values
*/
std::vector<arguments::argument> arguments::get_argument( std::vector<std::string_view> vectorName ) const
{
   std::vector<arguments::argument> vectorValue;
   for( const auto& itName : vectorName )
   {
      const_pointer pPosition = find( itName );
      if( pPosition != nullptr ) { vectorValue.push_back( get_argument_s( pPosition ) ); }
   }
   return vectorValue;
}

arguments::pointer arguments::set( pointer pPosition, const gd::variant_view& variantValue, tag_view)
{
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();
   unsigned uLength;
   if( uType > ARGUMENTS_NO_LENGTH ) 
   { 
      uType |= eValueLength;
      uLength = variantValue.length();
   }
   else
   {
      uLength = ctype_size[uType];
   }

   pPosition = set(pPosition, uType, pData, uLength, tag_internal{} );
   return pPosition;
}

void arguments::set_argument_section(const std::string_view& stringName, const std::vector<gd::variant_view>& vectorValue)
{
#ifndef NDEBUG
   auto string_d = gd::argument::shared::debug::print( *this );
#endif // !NDEBUG

   pointer pPosition = find( stringName );
   pointer pEnd = get_buffer_end();

   if(pPosition != nullptr)
   {
      auto it = vectorValue.begin();
      pPosition = set( pPosition, *it, tag_view{} );
      it++;
      for( ; it != vectorValue.end(); it++ ) 
      {
         pointer pNext = next_s( pPosition );
         if(pNext < pEnd)
         {
            if(is_name_s(pNext) == false)
            {
               pPosition = set( pNext, *it, tag_view{} );
            }
            else
            {
#ifndef NDEBUG
               string_d = gd::argument::shared::debug::print( *this );
#endif // !NDEBUG
               // TODO: Insert value before
               pPosition = insert( pNext, *it, tag_view{} );
               pEnd = get_buffer_end();
            }
         }
         else
         {  // ## if here we have moved to end and need to add value
            append( *it, tag_view{} );
            pEnd = get_buffer_end();
         }
      }
   }
}

// 0TAG0print.arguments

/*----------------------------------------------------------------------------- print */ /**
 * Print all values into string
 * *sample*
~~~{.cpp}
   gd::argument::arguments argumentsValue;

   argumentsValue.append("AAA", int32_t(1111));
   argumentsValue.append("BBB", int32_t(2222));

   auto stringAll = argumentsValue.print();
~~~
 * \return std::string all values as text
 */
std::string arguments::print() const
{
   std::string stringPrint;

   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( stringPrint.empty() == false ) stringPrint += std::string_view(", ");

      stringPrint += arguments::print_s(pPosition);
   }

   return stringPrint;
}

/** ---------------------------------------------------------------------------
 * @brief Formats and prints arguments based on a specified format string.
 * 
 * This method processes a format string containing placeholders enclosed in `{}`. 
 * Each placeholder is replaced with the string representation of the corresponding 
 * argument value from the `arguments` object. Text outside of placeholders is copied 
 * directly to the output string.
 * 
 * @param stringFormat The format string containing placeholders (e.g., "Hello, {name}!").
 * @return std::string A formatted string with placeholders replaced by argument values.
 * 
 * @note Placeholders must match the names of arguments stored in the `arguments` object.
 *       If a placeholder does not match any argument, it will be left empty in the output.
 * 
 * @code
gd::argument::arguments args;
args.append("name", "World");
std::string result = args.print("Hello, {name}!");
// result: "Hello, World!"
 * @endcode
 * 
 * @code
std::string stringFolder = "test-folder";
gd::argument::arguments arguments_;
arguments_.append("test-folder", stringFolder);
arguments_.append("database-file", "test.db");
std::cout << arguments_.print( "Folder where database are places is {} and database file is: {}\n" );
 * @endcode
 * 
 * @warning Ensure that the format string is well-formed, with properly matched `{}`.
 */
std::string arguments::print(std::string_view stringFormat) const
{
   unsigned uArgumentCount = 0; // number of arguments found
   std::string stringPrint;
   const char* piPosition = stringFormat.data();
   const char* piEnd = piPosition + stringFormat.length();

   while(piPosition < piEnd)
   {
      const char* piTo = piPosition;
      // Find next '{'
      while(piTo < piEnd && *piTo != '{') { piTo++; }

      // Copy characters up to pTo
      stringPrint.append(piPosition, piTo - piPosition);

      if( piTo < piEnd && *piTo == '{' )
      {
         piTo++; // Skip '{'
         const char* pbszPosition = piTo;
         const char* pbszBegin = piTo;

         // Copy characters until '}' or end
         while( piTo < piEnd && *piTo != '}' )
         {
            pbszPosition++;
            piTo++;
         }

#ifndef NDEBUG
         std::string string_d = std::string(pbszBegin, pbszPosition - pbszBegin);
#endif // NDEBUG

         if( pbszPosition - pbszBegin > 0 ) stringPrint += get_argument(std::string_view(pbszBegin, pbszPosition - pbszBegin)).get_string();
         else
         {
            stringPrint += get_argument(uArgumentCount).get_string();
         }
         uArgumentCount++;

         if(piTo < piEnd && *piTo == '}')
         {
            piTo++; // Skip '}'
         }
      }

      piPosition = piTo;
   }

   return stringPrint;
}

/*----------------------------------------------------------------------------- print_json */ /**
 * print in a json format
 * \return std::string
 */
std::string arguments::print_json() const
{
   std::string stringPrint;

   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( stringPrint.empty() == false ) stringPrint += std::string_view(", ");

      argument argumentValue = get_argument_s(pPosition);

      if( argumentValue.is_text() )
      {
         stringPrint += "\"";
         stringPrint += arguments::print_s(pPosition);
         stringPrint += "\"";
      }
      else
      {
         stringPrint += arguments::print_s(pPosition);
      }
   }

   return stringPrint;
}


/*----------------------------------------------------------------------------- print */ /**
 * Print selected values into string
 * *sample*
~~~{.cpp}
   gd::argument::arguments argumentsValue;

   argumentsValue.append("AAA", int32_t(1111));
   argumentsValue.append("BBB", int32_t(2222));
   argumentsValue.append("CCC", int32_t(3333));
   argumentsValue.append("DDD", int32_t(4444));

   auto stringAll = argumentsValue.print( std::begin( argumentsValue ) );
   auto it =  std::begin( argumentsValue );
   it++;
   it++;
   auto stringThirdAndFourth = argumentsValue.print( it );  // out: "CCC": 3333, "DDD": 4444
~~~
 * \param itBegin start printing values
 * \param itEnd quit printing values at this position
 * \param stringSplit text to split values for better format
 * \return std::string selected values as text
 */
std::string arguments::print( const_iterator itBegin, const_iterator itEnd, std::string_view stringSplit ) const
{
   std::string stringPrint;

   for( ; itBegin != itEnd; itBegin++ )
   {
      if( stringPrint.empty() == false ) stringPrint += stringSplit;

      stringPrint += arguments::print_s(itBegin);
   }

   return stringPrint;
}

/** --------------------------------------------------------------------------- print */ /**
 * @brief Prints the names of all arguments, separated by the specified string.
 *
 * This method iterates through all arguments and appends their names to a string,
 * separated by the provided delimiter. Only the names of the arguments are included
 * in the output string.
 *
 * @param stringSplit The string used to separate the names of the arguments.
 * @param tag_key A tag to specify that only the keys (names) of the arguments should be printed.
 * @return A string containing the names of all arguments, separated by the specified string.
 */
std::string arguments::print( const std::string_view& stringSplit, gd::types::tag_key) const
{
   std::string stringPrint;

   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( stringPrint.empty() == false ) stringPrint += stringSplit;

      if( is_name_s(pPosition) == true )
      {
         print_name_s(pPosition, stringPrint);
      }
   }

   return stringPrint;
}

/** --------------------------------------------------------------------------- print */ /**
 * @brief Prints the values of all arguments, separated by the specified string.
 *
 * This method iterates through all arguments and appends their values to a string,
 * separated by the provided delimiter. Only the values of the arguments are included
 * in the output string.
 *
 * @param stringSplit The string used to separate the values of the arguments.
 * @param tag_value A tag to specify that only the values of the arguments should be printed.
 * @return A string containing the values of all arguments, separated by the specified string.
 */
std::string arguments::print( const std::string_view& stringSplit, gd::types::tag_value) const
{
   std::string stringPrint;

   for( auto pPosition = next(); pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( stringPrint.empty() == false ) stringPrint += stringSplit;

      if( is_name_s(pPosition) == true )
      {
         print_name_s(pPosition, stringPrint);
      }
   }

   return stringPrint;
}




/*----------------------------------------------------------------------------- reserve */ /**
 * Make sure internal buffer can hold number of bytes requested
 * arguments object has data and member in one single memory block. First in buffer
 * is members and then data.
 \note if buffer is increased it is increased with 1.5 * count
 \note if buffer is realocated to increase buffer size and there is multiple owners, 
       these other owners will not be updated.
       You need to make sure that you have the only owner of the buffer before calling this function but it does allow it.
 * \param uCount number of bytes to reserve
 * \return bool true if new buffer is allocated, false if not
 */
bool arguments::reserve(uint64_t uCount)
{                                                                                                  assert( (uCount % 4) == 0 );
#ifndef NDEBUG
   auto uTotalBufferSize_d = buffer_buffer_size();                                                 
   assert( uTotalBufferSize_d == 0 || (uTotalBufferSize_d + sizeof( buffer )) % 64 == 0);
#endif

   if( uCount > buffer_buffer_size() )
   {
      uint64_t uBufferSize = (sizeof( buffer ) + uCount + ( uCount >> 1 ) + 63) & ~63;             // calculate new size and align to 64 byte
      uint8_t* pdata_ = new uint8_t[uBufferSize];

      // ## copy old buffer to new buffer
      uint64_t uOldSize = sizeof( buffer );
      uOldSize += buffer_size();
      memcpy(pdata_, m_pbuffer, uOldSize);

      buffer_release();                                                        // release old buffer
      m_pbuffer = (buffer*)pdata_;

      uBufferSize -= sizeof( buffer );                                         // decrease actual buffer data size with buffer, first in buffer is tha class buffer
      m_pbuffer->buffer_size( uBufferSize );

      return true;
   }

   return false;
}

void arguments::remove(const std::string_view& stringName)
{
   auto pposition = find( stringName );
   if(pposition != nullptr)
   {
      remove( pposition );
   }
}


/*----------------------------------------------------------------------------- remove */ /**
 * Remove param from params by removing section in internal buffer holding param values
 * \param pPosition position where to remove
 */
void arguments::remove(const_pointer pPosition)
{                                                                                assert( verify_d(pPosition) );
   auto uSize = arguments::get_total_param_length_s( pPosition );

   // removes section by moving memory
   // [..........xxxxxxxxxx..........]
   // [....................] ( removed "xxxxxxxxxx" )
   memmove( (void*)pPosition, (void*)(pPosition + uSize), get_buffer_end() - (pPosition + uSize) ); // move memory
   auto uSetSize = buffer_size();                                                                  assert( uSetSize >= uSize );
   uSetSize -= uSize;                                                                              assert( uSetSize % 4 == 0 );
   buffer_set_size( uSetSize );

   //m_uLength -= uSize;                                                           assert((int)m_uLength >= 0);
}


// TODO: Implement resize in order to be able to modify a value in arguments object
/*----------------------------------------------------------------------------- resize */ /**
 * Resize buffer, this is used to make space for new value at specific position (replace value)
 * \param pPosition
 * \param iOffset
 * \param iNewOffset
 * \return int
 */
int64_t arguments::resize(pointer pPosition, int64_t iOffset, int64_t iNewOffset)
{
   int64_t iSizeChange = iNewOffset - iOffset;

   if( iSizeChange != 0 )                                                         // need more  space for value compared to current value
   {
      // ## shrink section by moving memory
      // [..........xxxxxxxxxX..........] X = pPosition + iOffset
      // [..........xxxxxxxX..........] ( removed "xx" ) X = pPosition + iNewOffset
      // get_buffer_end() - (pPosition + iOffset) = number of bytes to be moved
      // ## expand section by moving memory
      // [..........xxxxxxxxxX..........] X = pPosition + iOffset
      // [..........ooooxxxxxxxxxX..........] ( added "oooo" ) X = pPosition + iNewOffset
      // get_buffer_end() - (pPosition + iOffset) = number of bytes to be moved
      void* pdestination = pPosition + iNewOffset;
      void* psource = pPosition + iOffset;
      size_t uCount = get_buffer_end() - (pPosition + iOffset);
      if( uCount != 0 ) { memmove( pdestination, psource, uCount ); } // move memory

      auto uPosition = buffer_size();
      uPosition += iSizeChange;
      buffer_set_size( uPosition );
   }

   return iSizeChange;
}

/** ---------------------------------------------------------------------------
 * @brief Remove unused memory from arguments object
 * 
 * *Sample to show how values are set to arguments and memory is "shrinked".*
 * @code
auto set_ = [](const char* pbszName, char chSet, arguments& a_) -> std::string {
   std::string stringSet;
   for(int i = 0; i < 100; i++)
   {
      stringSet += chSet;
      a_.set( pbszName, stringSet.c_str() );
   }
   return stringSet;
};

arguments a_;
const char* pbszName = "AA";

auto s_ = set_( pbszName, *pbszName, a_ );
s_ = s_.substr( 0, 10 );
a_.set( pbszName, s_.c_str() );
a_.shrink_to_fit();

pbszName = "BB";
s_ = set_( pbszName, *pbszName, a_ );
s_ = s_.substr( 0, 10 );
a_.set( pbszName, s_.c_str() );
a_.shrink_to_fit();

pbszName = "CC";
s_ = set_( pbszName, *pbszName, a_ );
s_ = s_.substr( 0, 10 );
a_.set( pbszName, s_.c_str() );
a_.shrink_to_fit();

s_ = gd::argument::debug::print( a_ );
std::cout << "arguments : " << s_ << "\n";

a_.remove( "BB" );

s_ = gd::argument::debug::print( a_ );
std::cout << "arguments : " << s_ << "\n";

 * @endcode
*/
void arguments::shrink_to_fit()
{                                                                                                  assert( buffer_reference_count() == 1 );
   if( capacity() > size( tag_memory{} ) )
   {
      auto uSize = buffer_size();                                              // get current sizes
      uSize += sizeof( buffer );                                               // add size for buffer object because all is in one memory block
      uint8_t* pdata_ = new unsigned char[uSize];
      memcpy(pdata_, m_pbuffer, uSize);                                        // copy data
      delete [] m_pbuffer;
      m_pbuffer = (buffer*)pdata_;
      m_pbuffer->buffer_size( uSize );                                         // set max buffer size
   }
}

/*----------------------------------------------------------------------------- size */ /**
 * Count number of param values in params object and return count
 * \return size_t number of values in params object
 */
size_t arguments::size() const
{
   if( empty() == true ) return 0;

   size_t uCount = 1;
   const_pointer pPosition = buffer_data();
   while( (pPosition = next(pPosition)) != nullptr )
   {
      uCount++;
   }
   return uCount;
}

/** ---------------------------------------------------------------------------
 * @brief Free allocated memory if any and set to empty
*/
void arguments::clear()
{
   if( m_pbuffer != &m_buffer_s )
   {
      m_pbuffer->release();
      m_pbuffer = &m_buffer_s;
   }
}


   
/*----------------------------------------------------------------------------- get_param */ /**
 * Get param at specified position
 * \param uIndex position in buffer where param is found
 * \return params::param param value returned
 */
arguments::argument arguments::get_argument(unsigned int uIndex) const
{
   if( empty() == false )
   {
      const_pointer pPosition = buffer_data();
      while( uIndex > 0 && (pPosition = next( pPosition )) != nullptr )
      {
         uIndex--;
      }

      if( pPosition != nullptr ) return arguments::get_argument_s( pPosition );
   }
   return argument();
}

/** ---------------------------------------------------------------------------
 * @brief return value within the named value section.
 * If `arguments` stores value without name/key then these "belongs" or can act
 * as they belongs to the starting name value. And this make arguments a bit more
 * flexible when to store lists of values.
 * @code
 * @endcode
 * @param stringName section name, value with name where to start finding value
 * @param uSecondIndex index for non named values after first value with matched name
 * @return argument with value or argument with null
 */
arguments::argument arguments::get_argument(std::string_view stringName, unsigned uSecondIndex, tag_section ) const
{
   const auto* pPosition = find(stringName);
   if( pPosition != nullptr )
   {
      if( uSecondIndex == 0 ) return get_argument_s(pPosition);

      pPosition = next_s( pPosition, uSecondIndex, get_buffer_end() );
      if( pPosition != nullptr ) return get_argument_s(pPosition);
   }

   return argument();
}

/** ---------------------------------------------------------------------------
 * @brief return first value found from list of names
 * @code
gd::argument::arguments argumentsUser;
// omitted
auto user_ = argumentsUser.get_argument({ "alias", "name", "city" });
 * @endcode
 * @param listName list of names to check if argument exists for
 * @return argument with value
*/
arguments::argument arguments::get_argument( const std::initializer_list<std::string_view>& listName ) const
{
   for( auto it = std::begin( listName ), itEnd = std::end( listName ); it != itEnd; it++ )
   {
      argument v_ = find_argument( *it );
      if( v_.is_null() == false ) return v_;
   }
   return argument();
}

/** ---------------------------------------------------------------------------
 * @brief return arguments in vector for each position
 * Note: very important that positions for values are valid positions in arguments object, otherwise it will crash
 * @param vectorPosition positions for each value
 * @return std::vector<arguments::argument> vector holding values for each position
 */
std::vector<arguments::argument> arguments::get_argument(const std::vector<const_pointer>& vectorPosition) const
{
   std::vector<arguments::argument> vector_;
   for(auto it : vectorPosition)
   {
      auto argument_ = get_argument( it );
      vector_.push_back( argument_ );
   }

   return vector_;
}

/** ---------------------------------------------------------------------------
 * @brief Get param at specified position
 * \param uIndex position in buffer where param is found
 * @return std::pair<std::string_view, gd::variant_view> pair value with argument name and argument value as variant_view
*/
std::pair<std::string_view, gd::variant_view> arguments::get_variant_view( unsigned int uIndex, tag_pair ) const
{
   if( empty() == false )
   {
      const_pointer pPosition = buffer_data();
      while( uIndex > 0 && (pPosition = next( pPosition )) != nullptr )
      {
         uIndex--;
      }

      if( pPosition != nullptr )
      {
         std::string_view s_;
         if( is_name_s(pPosition) == true ) s_ = get_name_s( pPosition );
         return { s_, get_argument_s( pPosition ).get_variant_view() };
      }
   }

   return std::pair<std::string_view, gd::variant_view>();
}

/*----------------------------------------------------------------------------- compare_argument_s */ /**
 * compare to argument values if equal
 * \param v1 first argument that is compared
 * \param argument2 second argument to compare
 * \return bool true if equal, false if not
 */
bool arguments::compare_argument_s(const argument& argument1, const argument& argument2)
{
   auto eType = argument1.type_number();
   if( eType != argument2.type_number() ) return false;
   switch( eType )
   {
   case arguments::eTypeNumberUnknown: return true;
   case arguments::eTypeNumberBool: return argument1.m_unionValue.b == argument2.m_unionValue.b;
   case arguments::eTypeNumberInt8: return argument1.m_unionValue.v_int8 == argument2.m_unionValue.v_int8;
   case arguments::eTypeNumberUInt8: return argument1.m_unionValue.v_uint8 == argument2.m_unionValue.v_uint8;
   case arguments::eTypeNumberInt16: return argument1.m_unionValue.v_int16 == argument2.m_unionValue.v_int16;
   case arguments::eTypeNumberUInt16: return argument1.m_unionValue.v_uint16 == argument2.m_unionValue.v_uint16;
   case arguments::eTypeNumberInt32: return argument1.m_unionValue.v_int32 == argument2.m_unionValue.v_int32;
   case arguments::eTypeNumberUInt32: return argument1.m_unionValue.v_uint32 == argument2.m_unionValue.v_uint32;
   case arguments::eTypeNumberInt64: return argument1.m_unionValue.v_int64 == argument2.m_unionValue.v_int64;
   case arguments::eTypeNumberUInt64: return argument1.m_unionValue.v_uint64 == argument2.m_unionValue.v_uint64;
   case arguments::eTypeNumberPointer: return argument1.m_unionValue.v_uint64 == argument2.m_unionValue.v_uint64;
      return argument1.m_unionValue.p == argument2.m_unionValue.p;
      break;
   case arguments::eTypeNumberGuid: 
      return memcmp(argument1.m_unionValue.p, argument2.m_unionValue.p, 16) == 0;
      break;
   case arguments::eTypeNumberFloat: return argument1.m_unionValue.f == argument2.m_unionValue.f;
   case arguments::eTypeNumberDouble: return argument1.m_unionValue.d == argument2.m_unionValue.d;
   case arguments::eTypeNumberString: 
   case arguments::eTypeNumberUtf8String:
      return strcmp(argument1.m_unionValue.pbsz, argument2.m_unionValue.pbsz) == 0;
      break;
   case arguments::eTypeNumberWString:
      return wcscmp(argument1.m_unionValue.pwsz, argument2.m_unionValue.pwsz) == 0;
      break;
   }

   return false;
}

/*----------------------------------------------------------------------------- compare_argument_s */ /**
 * Compare argument with variant_view.
 * \param a argument value compared to variant_view
 * \param v variant_view value compared to argument
 * \return bool true if equal, false if not equal
 */
bool arguments::compare_argument_s(const argument& a, const gd::variant_view& v)
{
   auto eType = a.type_number();
   if( eType == v.type_number() )
   {
      switch( eType )
      {
      case arguments::eTypeNumberUnknown: return true;
      case arguments::eTypeNumberBool: return a.m_unionValue.b == v.m_V.b;
      case arguments::eTypeNumberInt8: return a.m_unionValue.v_int8 == v.m_V.int8;
      case arguments::eTypeNumberUInt8: return a.m_unionValue.v_uint8 == v.m_V.uint8;
      case arguments::eTypeNumberInt16: return a.m_unionValue.v_int16 == v.m_V.int16;
      case arguments::eTypeNumberUInt16: return a.m_unionValue.v_uint16 == v.m_V.uint16;
      case arguments::eTypeNumberInt32: return a.m_unionValue.v_int32 == v.m_V.int32;
      case arguments::eTypeNumberUInt32: return a.m_unionValue.v_uint32 == v.m_V.uint32;
      case arguments::eTypeNumberInt64: return a.m_unionValue.v_int64 == v.m_V.int64;
      case arguments::eTypeNumberUInt64: return a.m_unionValue.v_uint64 == v.m_V.uint64;
      case arguments::eTypeNumberPointer: return a.m_unionValue.v_uint64 == v.m_V.uint64;
         return a.m_unionValue.p == v.m_V.p;
         break;
      case arguments::eTypeNumberGuid: 
         return memcmp(a.m_unionValue.p, v.m_V.p, 16) == 0;
         break;
      case arguments::eTypeNumberFloat: return a.m_unionValue.f == v.m_V.f;
      case arguments::eTypeNumberDouble: return a.m_unionValue.d == v.m_V.d;
      case arguments::eTypeNumberString:
      case arguments::eTypeNumberUtf8String:
         return strcmp(a.m_unionValue.pbsz, v.m_V.pbsz) == 0;
         break;
      case arguments::eTypeNumberWString:
         return wcscmp(a.m_unionValue.pwsz, v.m_V.pwsz) == 0;
         break;
      }
   }

   return false;
}

/*----------------------------------------------------------------------------- compare_argument_group_s */ /**
 * compare arguments based on group, it is a broader comparison that tries to match value
 * even if it isn't same type
 * \param argument1 first argument that is compared
 * \param argument2 second argument to compare
 * \return bool true if equal, false if not
 */
bool arguments::compare_argument_group_s(const argument& argument1, const argument& argument2)
{
   if( argument1.is_number() == true )
   {
      if( argument1.is_decimal() ) return argument1.get_double() == argument2.get_double();
      else return argument1.get_int64() == argument2.get_int64();
   }

   return compare_argument_s( argument1, argument2 );
}



/** ---------------------------------------------------------------------------
 * @brief compare argument group type with variant_view group type
 * @param argument1 argument object that is compared
 * @param VV2 variant view object to compare to
 * @return true if same group type, false if not
*/
bool arguments::compare_argument_group_s(const argument& argument1, const gd::variant_view& VV2)
{
   if( argument1.is_number() == VV2.is_number() )
   {
      if( argument1.is_decimal() ) return argument1.get_double() == VV2.get_double();
      else return argument1.get_int64() == VV2.get_int64();
   }

   return compare_s( argument1, VV2 );
}

/** ---------------------------------------------------------------------------
 * @brief Compare if source arguments values contains values from exists arguments
 * @param argumentsSource Source arguments to look for arguments to match against exists arguments
 * @param argumentsExists arguments to find in source arguments
 * @return true if all exists arguments is found in source
*/
bool arguments::compare_exists_s( const arguments& argumentsSource, const arguments& argumentsExists )
{
   for( auto it = argumentsExists.begin(), itEnd = argumentsExists.end(); it != itEnd; it++ )
   {
      auto stringExistsName = it.name( tag_view{} );
      if( stringExistsName.empty() == false )
      {
         auto pposition = argumentsSource.find( stringExistsName );
         if( pposition == nullptr ) return false;                              // not found ? then agruments do not exist in source

         auto argument_ = get_argument_s( pposition );
         if( argument_.compare( it.get_argument() ) == true ) continue;

         return false;
      }
   }

   return true;
}

bool arguments::compare_s(const argument& v1, const gd::variant_view v2)
{
   if( v1.type_number() != v2.type_number() ) return false;                      // type numbers matches between argument variant_view

   switch( v1.type_number() )
   {
   case arguments::eTypeNumberUnknown: return true;
   case arguments::eTypeNumberBool: return v1.m_unionValue.b == v2.m_V.b;
   case arguments::eTypeNumberInt8: return v1.m_unionValue.v_int8 == v2.m_V.int8;
   case arguments::eTypeNumberUInt8: return v1.m_unionValue.v_uint8 == v2.m_V.uint8;
   case arguments::eTypeNumberInt16: return v1.m_unionValue.v_int16 == v2.m_V.int16;
   case arguments::eTypeNumberUInt16: return v1.m_unionValue.v_uint16 == v2.m_V.uint16;
   case arguments::eTypeNumberInt32: return v1.m_unionValue.v_int32 == v2.m_V.int32;
   case arguments::eTypeNumberUInt32: return v1.m_unionValue.v_uint32 == v2.m_V.uint32;
   case arguments::eTypeNumberInt64: return v1.m_unionValue.v_int64 == v2.m_V.int64;
   case arguments::eTypeNumberUInt64: return v1.m_unionValue.v_uint64 == v2.m_V.uint64;
   case arguments::eTypeNumberPointer: return v1.m_unionValue.v_uint64 == v2.m_V.uint64;
      return v1.m_unionValue.p == v2.m_V.p;
      break;
   case arguments::eTypeNumberGuid: 
      return memcmp(v1.m_unionValue.p, v2.m_V.p, 16) == 0;
      break;
   case arguments::eTypeNumberFloat: return v1.m_unionValue.f == v2.m_V.f;
   case arguments::eTypeNumberDouble: return v1.m_unionValue.d == v2.m_V.d;
   case arguments::eTypeNumberString:
   case arguments::eTypeNumberUtf8String:
      return strcmp(v1.m_unionValue.pbsz, v2.m_V.pbsz) == 0;
      break;
   case arguments::eTypeNumberWString:
      return wcscmp(v1.m_unionValue.pwsz, v2.m_V.pwsz) == 0;
      break;
   }

   return false;
}

/*----------------------------------------------------------------------------- get_param_s */ /**
 * return param for position iterator is pointing at
 * \param it position for returned param
 * \return param  holding value at iterator position
 */
arguments::argument arguments::get_argument_s(arguments::const_pointer pPosition)
{
   uint32_t u_ = *(uint32_t*)pPosition;
   arguments::enumCType eCType = (arguments::enumCType)(u_ >> 24);
   uint32_t uLength = u_ & 0x00FFFFFF;

   pPosition += sizeof(uint32_t);
   
   switch( eCType )
   {
   case arguments::eTypeNumberUnknown: return arguments::argument();
   case arguments::eTypeNumberBool: return arguments::argument(*(bool*)pPosition);
   case arguments::eTypeNumberInt8: return arguments::argument(*(int8_t*)pPosition);
   case arguments::eTypeNumberUInt8: return arguments::argument(*(uint8_t*)pPosition);
   case arguments::eTypeNumberInt16: return arguments::argument(*(int16_t*)pPosition);
   case arguments::eTypeNumberUInt16: return arguments::argument(*(uint16_t*)pPosition);
   case arguments::eTypeNumberInt32: return arguments::argument(*(int32_t*)pPosition);
   case arguments::eTypeNumberUInt32: return arguments::argument(*(uint32_t*)pPosition);
   case arguments::eTypeNumberInt64: return arguments::argument(*(int64_t*)pPosition);
   case arguments::eTypeNumberUInt64: return arguments::argument(*(uint64_t*)pPosition);
   case arguments::eTypeNumberFloat: return arguments::argument(*(float*)pPosition);
   case arguments::eTypeNumberDouble: return arguments::argument(*(double*)pPosition);

   case arguments::eTypeNumberPointer: return arguments::argument((void*)(*(size_t*)pPosition));

   case arguments::eTypeNumberGuid: return arguments::argument((const uint8_t*)pPosition, eTypeGuid);

   // ## values that has value length stored in front of value, this is a four byte value


   case arguments::eTypeNumberString: return arguments::argument(eTypeString, (const uint8_t*)(const char*)(pPosition));
   case arguments::eTypeNumberUtf8String: return arguments::argument(eTypeUtf8String, (const uint8_t*)(const char*)(pPosition));
   case arguments::eTypeNumberWString: return arguments::argument(eTypeWString, (const uint8_t*)(const wchar_t*)(pPosition));
   case arguments::eTypeNumberBinary: return arguments::argument(eTypeGuid, (const uint8_t*)pPosition);

   case (arguments::eTypeNumberString | arguments::eValueLength): return arguments::argument(eTypeString | arguments::eValueLength, (const uint8_t*)(const char*)(pPosition + sizeof(uint32_t)));
   case (arguments::eTypeNumberUtf8String | arguments::eValueLength): return arguments::argument(eTypeUtf8String | arguments::eValueLength, (const uint8_t*)(const char*)(pPosition + sizeof(uint32_t)));
   case (arguments::eTypeNumberWString | arguments::eValueLength): return arguments::argument(eTypeWString | arguments::eValueLength, (const uint8_t*)(const wchar_t*)(pPosition + sizeof(uint32_t)));
   case (arguments::eTypeNumberBinary | arguments::eValueLength): return arguments::argument(eTypeBinary | arguments::eValueLength, (const uint8_t*)pPosition + sizeof(uint32_t));

   case arguments::eType_ParameterName:
   {
      uLength = align32_g( uLength );
      pPosition += uLength;
      return get_argument_s(pPosition);
   }
   break;

   default:
      assert(false);
   }
   return arguments::argument();
}

arguments::argument_edit arguments::get_edit_param_s(arguments* parguments, arguments::const_pointer pPosition)
{
   arguments::argument argumentValue = arguments::get_argument_s( pPosition );
   return arguments::argument_edit( parguments, pPosition, argumentValue );
}

/*----------------------------------------------------------------------------- get_total_param_length_s */ /**
 * Calculate number of bytes param value is using in internal buffer where param values
 * is stored
 * \param pPosition start position for param
 * \return unsigned int number of bytes param value use in buffer
 */
uint64_t arguments::get_total_param_length_s(const_pointer pPosition) noexcept
{
   const_pointer pEnd = next_s( pPosition );
   uint64_t uSize = pEnd - pPosition;
   return uSize;
}

/// return all matching values (same name) in vector
std::vector<arguments::argument> arguments::get_argument_all_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName)
{                                                                                                  assert( pBegin <= pEnd );
   std::vector<argument> vectorArgument;
   if( pBegin != nullptr )
   {
      do
      {
         if( compare_name_s( pBegin, stringName ) == true ) vectorArgument.push_back( get_argument_s( pBegin ) );
      } while( (pBegin = next_s( pBegin )) < pEnd );
   }

   return vectorArgument;
}

/// return all matching values (same name) in vector
std::vector<gd::variant_view> arguments::get_argument_all_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName, tag_view)
{
   std::vector<gd::variant_view> vectorArgument;
   if( pBegin != nullptr )
   {
      do
      {
         if( compare_name_s( pBegin, stringName ) == true ) vectorArgument.push_back( get_argument_s( pBegin ).as_variant_view() );
      } while( (pBegin = next_s( pBegin )) < pEnd );
   }

   return vectorArgument;
}

/// return all values from name and traling values with no name
/// this is handy if you store a list or lists of values in arguments object and they start with some name
std::vector<gd::variant_view> arguments::get_argument_section_s(const_pointer pBegin, const_pointer pEnd, std::string_view stringName, tag_view)
{
   std::vector<gd::variant_view> vectorArgument;
   if( pBegin != nullptr )
   {
      do
      {
         if( compare_name_s( pBegin, stringName ) == true )                    // found name ?
         {
            vectorArgument.push_back( get_argument_s( pBegin ).as_variant_view() );// push value for name

            // push all trailing values that isn't named
            while( (pBegin = next_s( pBegin )) < pEnd && is_name_s( pBegin ) == false ) 
            { 
               vectorArgument.push_back( get_argument_s( pBegin ).as_variant_view() ); 
            }

            return vectorArgument;
         }
      } while( (pBegin = next_s( pBegin )) < pEnd );
   }

   return vectorArgument;
}

/*----------------------------------------------------------------------------- get_total_param_length_s */ /**
 * Calculate number of bytes needed to store value in internal buffer used by arguments object
 * \param argumentValue value store for name
 * \return unsigned int number of bytes needed to store value in internal buffer
 */
unsigned int arguments::get_total_param_length_s(const argument& argumentValue)
{
   unsigned uSize = sizeof_s(argumentValue);
   uSize = align32_g( uSize );
   return uSize;
}


/*----------------------------------------------------------------------------- get_total_param_length_s */ /**
 * Calculate number of bytes needed to store named value in internal buffer used by arguments object
 * \param stringName name for value
 * \param argumentValue value store for name
 * \return unsigned int number of bytes needed to store value in internal buffer
 */
unsigned int arguments::get_total_param_length_s(std::string_view stringName, const argument& argumentValue)
{
   unsigned uSize = sizeof_name_s( (unsigned)stringName.length(), tag_align{});
   uSize += sizeof_s(argumentValue);
   uSize = align32_g( uSize );
   return uSize;
}


/*----------------------------------------------------------------------------- next */ /**
 * Move to next element in binary stream. Make sure that next value is compatible type
 * @see: Read `arguments` documents in header for explanation on how data is placed
 * \param riPosition current position in stream
 * \return arguments::pointer position to next value
 */
arguments::pointer arguments::next_s(pointer pPosition) 
{
#ifndef NDEBUG
   const_pointer pbegin_d = pPosition;
#endif
   pPosition = arguments::move_to_value_s(pPosition);                          // go to value
   uint32_t u_ = *(uint32_t*)pPosition;                                        // get type and length
   uint32_t uType = u_ >> 24;                                                  // get value type
#ifndef NDEBUG
   auto typename_d = gd::types::type_name_g( uType & ~eTypeNumber_MASK );
#endif
   // Total value buffer storage length for value
   uint32_t uLength = u_ & 0x00FFFFFF;                                                             assert( (uLength % 4) == 0 ); // has to be aligend 
   pPosition += sizeof( uint32_t );                                            // move past type and length information
   pPosition += uLength;                                                       // move past value length
#ifndef NDEBUG
   uint64_t uValueSize_d = pPosition - pbegin_d;
#endif
   return pPosition;
}

/*----------------------------------------------------------------------------- next */ /**
 * Move to next element in binary stream. Make sure that next value is compatible type
 * @see: Read `arguments` documents in header for explanation on how data is placed
 * \param riPosition current position in stream
 * \return arguments::const_pointer position to next value
 */
arguments::const_pointer arguments::next_s(const_pointer pPosition) 
{
#ifndef NDEBUG
   const_pointer pbegin_d = pPosition;
#endif
   pPosition = arguments::move_to_value_s(pPosition);
   uint32_t u_ = *(uint32_t*)pPosition;
   uint32_t uType = u_ >> 24;                                                  // get value type
#ifndef NDEBUG
   auto typename_d = gd::types::type_name_g( uType & ~eTypeNumber_MASK );
#endif
   // Total value buffer storage length for value
   uint32_t uLength = u_ & 0x00FFFFFF;                                                             assert( (uLength % 4) == 0 ); // has to be aligend 
   pPosition += sizeof( uint32_t );                                            // move past type and length information
   pPosition += uLength;                                                       // move past value length
#ifndef NDEBUG
   uint64_t uValueSize_d = pPosition - pbegin_d;
#endif

   return pPosition;
}

/** ---------------------------------------------------------------------------
 * @brief goto sub (second) value in named section
 * name section is a value that is named and then non named values after belongs to it
 * @param pPosition first value, could be a name
 * @param uSecondIndex index to value that belongs to first named value
 * @param pEnd last position to search (buffer ends)
 * @return pointer to value if index is within the number of values after name or nullptr if no value
 */
arguments::const_pointer arguments::next_s( const_pointer pPosition, unsigned uSecondIndex, const_pointer pEnd )
{
   pPosition = arguments::move_to_value_s(pPosition);
   uint32_t u_ = *(uint32_t*)pPosition;
   uint32_t uType = u_ >> 24;                                                  // get value type
#ifndef NDEBUG
   auto typename_d = gd::types::type_name_g( uType & ~eTypeNumber_MASK );
#endif
   while( pPosition < pEnd && uSecondIndex > 0 && uType < arguments::CType_MAX )
   {
#ifndef NDEBUG
      const_pointer pbegin_d = pPosition;
#endif
      uint32_t uLength = u_ & 0x00FFFFFF;                                                          assert( (uLength % 4) == 0 ); // has to be aligend 
      uType = u_ >> 24;
      pPosition += sizeof( uint32_t );                                         // move past type and length information
      pPosition += uLength;                                                    // move past value length
#ifndef NDEBUG
      uint64_t uValueSize_d = pPosition - pbegin_d;
#endif
      uSecondIndex--;
   }

   if( uSecondIndex == 0 && uType < arguments::CType_MAX ) return pPosition;

   return nullptr;
}

/// Same as `next_s` above with 
arguments::pointer arguments::next_s( pointer pPosition, unsigned uSecondIndex, const_pointer pEnd )
{
   pPosition = arguments::move_to_value_s(pPosition);
   uint32_t u_ = *(uint32_t*)pPosition;
   uint32_t uType = u_ >> 24;                                                  // get value type
#ifndef NDEBUG
   auto typename_d = gd::types::type_name_g( uType & ~eTypeNumber_MASK );
#endif
   while( pPosition < pEnd && uSecondIndex > 0 && uType < arguments::CType_MAX )
   {
#ifndef NDEBUG
      const_pointer pbegin_d = pPosition;
#endif
      uint32_t uLength = u_ & 0x00FFFFFF;                                                          assert( (uLength % 4) == 0 ); // has to be aligend 
      uType = u_ >> 24;
      pPosition += sizeof( uint32_t );                                         // move past type and length information
      pPosition += uLength;                                                    // move past value length
#ifndef NDEBUG
      uint64_t uValueSize_d = pPosition - pbegin_d;
#endif
      uSecondIndex--;
   }

   if( uSecondIndex == 0 && uType < arguments::CType_MAX ) return pPosition;

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Advances to the next value position in a arguments buffer and returns the new offset.
 *
 * @param pbuffer Pointer to the constant buffer containing the data
 * @param uOffset Initial offset position within the buffer
 * @return size_t New offset position after advancing to the next value in the buffer
 *
 * @warning Ensure pbuffer is valid and properly allocated before calling this method
 */
size_t arguments::next_s(const_pointer pbuffer, size_t uOffset)
{
   const_pointer p_ =  pbuffer + uOffset;
   p_ = next_s( p_ );
   uOffset = p_ - pbuffer;
   return uOffset;
}



/*----------------------------------------------------------------------------- sizeof_s */ /**
 * Return size for argument in bytes
 * \param argumentValue argument value size is returned for
 * \return unsigned int number of bytes for value
 */
unsigned int arguments::sizeof_s(const argument& argumentValue)
{
   unsigned int uSize = sizeof(uint32_t);                                      // start with 32 bit unsigned that store type and value size
   if( argumentValue.ctype() & eValueLength )  uSize += sizeof(uint32_t);      // add size value storing native size
   uSize += argumentValue.size();

   return uSize;
}

/*----------------------------------------------------------------------------- sizeof_s */ /**
 * @brief calculate needed size in bytes to store variant view value
 * @param variantviewValue value to return size in bytes for
 * @return unsigned number of bytes needed to store value
 */
unsigned int arguments::sizeof_s(const gd::variant_view& variantviewValue, tag_view)
{
   auto argumentValue = get_argument_s(variantviewValue);
   return sizeof_s( argumentValue );
}

/** ---------------------------------------------------------------------------
 * @brief calculate needed size in bytes to store name and variant view value
 * @param stringName name for value
 * @param VV_ value to return size in bytes for
 * @return unsigned number of bytes needed to store name and value
 */
unsigned int arguments::sizeof_s(const std::string_view& stringName, const gd::variant_view& VV_, tag_view)
{
   unsigned uSize = 0;
   auto argumentValue = get_argument_s(VV_);

   uSize += sizeof( uint32_t );
   uSize += (unsigned)stringName.length();
   uSize = align32_g( uSize );
   uSize += sizeof_s( argumentValue );

   return uSize;
}

/*----------------------------------------------------------------------------- print_s */ /**
 * Print values into text and return string with values
 * \param pPosition position in arguments object where value is located
 * \param uPairType pair type to print, key and/or value can be printed
 * \return std::string text containing key/value 
 */
std::string arguments::print_s(const_pointer pPosition, uint32_t uPairType)
{
   std::string stringArgument;
   auto uType = type_s( pPosition );
   if( uType == eType_ParameterName )
   {
      if( uPairType & ePairTypeKey )
      {
         stringArgument += "\"";
         stringArgument.append( get_name_s( pPosition ) );
         stringArgument += "\": ";
      }
   }

   pPosition = move_to_value_s( pPosition );                                   // move to value

   if( uPairType & ePairTypeValue )
   {
      auto uType = type_s( pPosition );
      arguments::argument argumentValue( get_argument_s( pPosition ) );
      stringArgument += argumentValue.get_string();
   }

   return stringArgument;
}

void arguments::print_name_s( const_pointer pPosition, std::string& stringPrint )
{
   if( *pPosition == eType_ParameterName )
   {
      stringPrint.append( reinterpret_cast<const char*>( pPosition+ 2), *(pPosition + 1) );
   }
}

void arguments::print_type_s( const_pointer pPosition, std::string& stringPrint )
{
   arguments::argument argumentValue( get_argument_s( pPosition ) );

   auto stringType = type_name_s( argumentValue.type() );
   stringPrint.append( stringType );
}

void arguments::print_value_s( const_pointer pPosition, std::string& stringPrint )
{
   arguments::argument argumentValue( get_argument_s( pPosition ) );
   stringPrint += argumentValue.as_string();
}


/*----------------------------------------------------------------------------- sizeof_s */ /**
 * Calculate length for name value where each part have been counted
 * \param uNameLength name length for argument stored in arguments
 * \param uType type of value
 * \param uValueLength length needed to store value
 * \return unsigned int calculated length needed to store value in arguments buffer
 */
unsigned int arguments::sizeof_s(uint32_t uNameLength, param_type uType, unsigned int uValueLength)
{
   unsigned int uSize = 0;
   if(uNameLength > 0)
   {
      uSize = sizeof( uint32_t ) + uNameLength;
      uSize = align32_g( uSize );
   }

   uSize += sizeof( uint32_t );
   if( uType & eValueLength ) { uSize += sizeof( uint32_t ); }
   uSize +=  uValueLength;
   uSize = align32_g( uSize );

   return uSize;
}

/// return needed aligned size to store name in arguments buffer
unsigned int arguments::sizeof_name_s(uint32_t uNameLength, tag_align) noexcept
{
   unsigned uSize = 0;
   if(uNameLength > 0)
   {
      uSize = sizeof( uint32_t ) + uNameLength;
      uSize = align32_g( uSize );
   }
   return uSize;
}

/// return needed size for name within arguments
unsigned int arguments::sizeof_name_s(const_pointer pPosition) noexcept
{ 
   unsigned uSize;
   auto uType = type_s( pPosition, &uSize );
   if( *pPosition == eType_ParameterName )
   {
      return uSize + sizeof( uint32_t );                                       // return size and unsigned prefix needed to store name information
   }
   return 0;
}

/// return needed size for name within arguments
unsigned int arguments::sizeof_name_s(const_pointer pPosition, tag_align ) noexcept
{ 
   unsigned uSize;
   auto uType = type_s( pPosition, &uSize );
   if( *pPosition == eType_ParameterName )
   {
      uSize = align32_g( uSize );
      return uSize + sizeof( uint32_t );                                       // return size and unsigned prefix needed to store name information
   }
   return 0;
}



/*----------------------------------------------------------------------------- get_variant_s */ /**
 * Return argument value as variant
 * \param argumentValue value that is returned as variant
 * \return gd::variant value with argument value
 */
gd::variant arguments::get_variant_s(const arguments::argument& argumentValue)
{
   const auto& value = argumentValue.get_value();

   switch( type_number_s(argumentValue.m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      return gd::variant();
      break;
   case arguments::eTypeNumberBool:
      return gd::variant(value.b);
      break;
   case arguments::eTypeNumberInt8:
      return gd::variant(value.v_int8);
      break;
   case arguments::eTypeNumberUInt8:
      return gd::variant(value.v_uint8);
      break;
   case arguments::eTypeNumberInt16:
      return gd::variant(value.v_int16);
      break;
   case arguments::eTypeNumberUInt16:
      return gd::variant(value.v_uint16);
      break;
   case arguments::eTypeNumberInt32:
      return gd::variant(value.v_int32);
      break;
   case arguments::eTypeNumberUInt32:
      return gd::variant(value.v_uint32);
      break;
   case arguments::eTypeNumberInt64:
      return gd::variant(value.v_int64);
      break;
   case arguments::eTypeNumberUInt64:
      return gd::variant(value.v_uint64);
      break;
   case arguments::eTypeNumberFloat:
      return gd::variant(value.f);
      break;
   case arguments::eTypeNumberDouble:
      return gd::variant(value.d);
      break;
   case arguments::eTypeNumberString:
      return gd::variant(value.pbsz, (size_t)argumentValue.length() - sizeof(char) );
      break;
   case arguments::eTypeNumberUtf8String:
      return gd::variant( gd::variant::utf8( value.pbsz, (size_t)argumentValue.length() - sizeof(char) ) );
      break;
   case arguments::eTypeNumberWString:
      return gd::variant(value.pwsz, (size_t)argumentValue.length() - sizeof(wchar_t) );
      break;
   default:
      assert(false);
   }

   return variant();
}

/** ---------------------------------------------------------------------------
 * @brief Converts list of argument values into list of variant values
 * @param vectorValue list of argument values
 * @return std::vector<gd::variant> list of variant values
*/
std::vector<gd::variant> arguments::get_variant_s( const std::vector<argument>& vectorValue )
{
   std::vector<gd::variant> vectorResult;

   for( auto it : vectorValue ) { vectorResult.emplace_back( get_variant_s( it ) ); }

   return vectorResult;
}

/*----------------------------------------------------------------------------- get_variant_s */ /**
 * Return argument value as variant, variant value returned do not own the memory so
 * make sure that the value isn't deleted as long as the variant value is in use
 * \param argumentValue value that is returned as variant
 * \return gd::variant value with argument value
 */
gd::variant arguments::get_variant_s(const arguments::argument& argumentValue, bool)
{
   const auto& value = argumentValue.get_value();

   switch( type_number_s(argumentValue.m_eType) )
   {
   case arguments::eTypeNumberBool:
      return gd::variant(value.b);
      break;
   case arguments::eTypeNumberInt8:
      return gd::variant(value.v_int8);
      break;
   case arguments::eTypeNumberUInt8:
      return gd::variant(value.v_uint8);
      break;
   case arguments::eTypeNumberInt16:
      return gd::variant(value.v_int16);
      break;
   case arguments::eTypeNumberUInt16:
      return gd::variant(value.v_uint16);
      break;
   case arguments::eTypeNumberInt32:
      return gd::variant(value.v_int32);
      break;
   case arguments::eTypeNumberUInt32:
      return gd::variant(value.v_uint32);
      break;
   case arguments::eTypeNumberInt64:
      return gd::variant(value.v_int64);
      break;
   case arguments::eTypeNumberUInt64:
      return gd::variant(value.v_uint64);
      break;
   case arguments::eTypeNumberFloat:
      return gd::variant(value.f);
      break;
   case arguments::eTypeNumberDouble:
      return gd::variant(value.d);
      break;
   case arguments::eTypeNumberString:
      return gd::variant(value.pbsz, (size_t)argumentValue.length() - sizeof(char), false);
      break;
   case arguments::eTypeNumberUtf8String:
      return gd::variant( variant::utf8( value.pbsz, (size_t)argumentValue.length() - sizeof(char) ), false );
      break;
   case arguments::eTypeNumberWString:
      return gd::variant(value.pwsz, (size_t)argumentValue.length() - sizeof(wchar_t), false);
      break;
   default:
      assert(false);
   }

   return variant();
}

/*----------------------------------------------------------------------------- get_variant_s */ /**
 * Return argument value as variant_view
 * \param argumentValue value that is returned as variant_view
 * \return gd::variant_view value with argument value
 */
gd::variant_view arguments::get_variant_view_s(const arguments::argument& argumentValue)
{
   const auto& value = argumentValue.get_value();

   switch( type_number_s(argumentValue.m_eType) )
   {
   case arguments::eTypeNumberUnknown:
      return variant_view();
      break;
   case arguments::eTypeNumberBool:
      return gd::variant_view(value.b);
      break;
   case arguments::eTypeNumberInt8:
      return gd::variant_view(value.v_int8);
      break;
   case arguments::eTypeNumberUInt8:
      return gd::variant_view(value.v_uint8);
      break;
   case arguments::eTypeNumberInt16:
      return gd::variant_view(value.v_int16);
      break;
   case arguments::eTypeNumberUInt16:
      return gd::variant_view(value.v_uint16);
      break;
   case arguments::eTypeNumberInt32:
      return gd::variant_view(value.v_int32);
      break;
   case arguments::eTypeNumberUInt32:
      return gd::variant_view(value.v_uint32);
      break;
   case arguments::eTypeNumberInt64:
      return gd::variant_view(value.v_int64);
      break;
   case arguments::eTypeNumberUInt64:
      return gd::variant_view(value.v_uint64);
      break;
   case arguments::eTypeNumberFloat:
      return gd::variant_view(value.f);
      break;
   case arguments::eTypeNumberDouble:
      return gd::variant_view(value.d);
      break;
   case arguments::eTypeNumberGuid:
      return gd::variant_view(value.pbsz, (size_t)argumentValue.length());
      break;
   case arguments::eTypeNumberString: {                                                            assert( (size_t)value.pbsz % 4 == 0 );
         size_t uSize = (size_t)*(uint32_t*)(value.pbsz - sizeof(uint32_t));
         return gd::variant_view(value.pbsz, uSize );
      }
      break;
   case arguments::eTypeNumberUtf8String: {
         size_t uSize = (size_t)*(uint32_t*)(value.pbsz - sizeof(uint32_t));
         return gd::variant_view( variant_type::utf8( value.pbsz, uSize ) );
      }
      break;
   case arguments::eTypeNumberWString: {
         size_t uSize = (size_t)*(uint32_t*)(value.pwsz - sizeof(uint32_t));
         return gd::variant_view(value.pwsz, uSize);
      }
      break;
   default:
      assert(false);
   }

   return variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief Converts list of argument values into list of variant_view values
 * @param vectorValue list of argument values
 * @return std::vector<gd::variant> list of variant_view values
*/
std::vector<gd::variant_view> arguments::get_variant_view_s( const std::vector<argument>& vectorValue )
{
   std::vector<gd::variant_view> vectorResult;

   for( auto it : vectorValue ) { vectorResult.emplace_back( get_variant_view_s( it ) ); }

   return vectorResult;
}



/** ---------------------------------------------------------------------------
 * @brief Convert variant to argument
 * @param variantValue variant to be converted
 * @return argument converted value as argument
*/
arguments::argument arguments::get_argument_s(const gd::variant& variantValue)
{
   switch( variantValue.type_number() )
   {
   case variant_type::eTypeNumberBool:
      return arguments::argument( (bool)variantValue );
      break;
   case variant_type::eTypeNumberInt8:
      return arguments::argument((int8_t)variantValue);
      break;
   case variant_type::eTypeNumberInt16:
      return arguments::argument((int16_t)variantValue);
      break;
   case variant_type::eTypeNumberInt32:
      return arguments::argument((int32_t)variantValue);
      break;
   case variant_type::eTypeNumberInt64:
      return arguments::argument((int64_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt8:
      return arguments::argument((uint8_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt16:
      return arguments::argument((uint16_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt32:
      return arguments::argument((uint32_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt64:
      return arguments::argument((uint64_t)variantValue);
      break;
   case variant_type::eTypeNumberFloat:
      return arguments::argument((float)variantValue);
      break;
   case variant_type::eTypeNumberDouble:
      return arguments::argument((double)variantValue);
      break;
   case variant_type::eTypeNumberPointer:
      return arguments::argument((void*)variantValue);
      break;
   case variant_type::eTypeNumberGuid:
      return arguments::argument(eTypeGuid, (const uint8_t*)variantValue);
      break;
   case variant_type::eTypeNumberString:
      return arguments::argument((const char*)variantValue);
      break;
   case variant_type::eTypeNumberUtf8String:
      return arguments::argument(eTypeUtf8String, (const uint8_t*)variantValue);
      break;
   case variant_type::eTypeNumberWString:
      return arguments::argument((const wchar_t*)variantValue);
      break;
   case variant_type::eTypeNumberBinary:
      return arguments::argument(eTypeBinary, (const uint8_t*)variantValue);
      break;
   default:                                                                   assert( false );
   }

   return arguments::argument();
}

arguments::argument arguments::get_argument_s(const gd::variant_view& variantValue)
{
#ifdef _DEBUG
   auto type_d = gd::variant::get_type_name_s( variantValue.type_number() );
#endif // _DEBUG

   auto uNumberType = variantValue.type_number();
   switch( uNumberType )
   {
   case variant_type::eTypeNumberUnknown:
      return arguments::argument();
      break;
   case variant_type::eTypeNumberBool:
      return arguments::argument((bool)variantValue);
      break;
   case variant_type::eTypeNumberInt8:
      return arguments::argument((int8_t)variantValue);
      break;
   case variant_type::eTypeNumberInt16:
      return arguments::argument((int16_t)variantValue);
      break;
   case variant_type::eTypeNumberInt32:
      return arguments::argument((int32_t)variantValue);
      break;
   case variant_type::eTypeNumberInt64:
      return arguments::argument((int64_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt8:
      return arguments::argument((uint8_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt16:
      return arguments::argument((uint16_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt32:
      return arguments::argument((uint32_t)variantValue);
      break;
   case variant_type::eTypeNumberUInt64:
      return arguments::argument((uint64_t)variantValue);
      break;
   case variant_type::eTypeNumberFloat:
      return arguments::argument((float)variantValue);
      break;
   case variant_type::eTypeNumberDouble:
      return arguments::argument((double)variantValue);
      break;
   case variant_type::eTypeNumberPointer:
      return arguments::argument((void*)variantValue);
      break;
   case variant_type::eTypeNumberGuid:
      return arguments::argument(eTypeGuid, (const uint8_t*)variantValue);
      break;
   case variant_type::eTypeNumberString:
      return arguments::argument((const char*)variantValue);
      break;
   case variant_type::eTypeNumberUtf8String:
#if defined(__cpp_char8_t)
      return arguments::argument((const char8_t*)variantValue);
#else
      return arguments::argument(eTypeUtf8String, (const char*)variantValue);
#endif
      break;
   case variant_type::eTypeNumberWString:
      return arguments::argument((const wchar_t*)variantValue);
      break;
   case variant_type::eTypeNumberBinary:
      return arguments::argument(eTypeBinary, (const uint8_t*)variantValue);
      break;
   default:                                                                   assert( false );
   }

   return arguments::argument();
}

/** ---------------------------------------------------------------------------
 * @brief validates that named values exist in arguments object
 * @param argumentsValidate arguments object where values are checked for existence
 * @param listValue list of named values to check for existence
 * @return std::pair<bool, std::string> true if ok, false and sent name that wasn't found
*/
std::pair<bool, std::string> arguments::exists_s( const arguments& argumentsValidate, const std::initializer_list<std::string_view>& listName, tag_name )
{
   for( auto it = std::begin( listName ), itEnd = std::end( listName ); it != itEnd; it++ )
   {
      if( argumentsValidate.exists( *it ) == false ) return { false, std::string( *it ) };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
* @brief check if any of named values exist in arguments object
* @param argumentsValidate arguments object where values are checked for existence
* @param listValue list of named values to check for existence
* @return std::pair<bool, std::string> true and name for first value found, false if not found
*/
std::pair<bool, std::string> arguments::exists_any_of_s( const arguments& argumentsValidate, const std::initializer_list<std::string_view>& listName, tag_name )
{
   for( auto it = std::begin( listName ), itEnd = std::end( listName ); it != itEnd; it++ )
   {
      if( argumentsValidate.exists( *it ) == true ) return { true, std::string( *it ) };
   }

   return { false, "" };
}



/** ---------------------------------------------------------------------------
 * @brief validates that named values exist in arguments object
 * @param argumentsValidate arguments object where values are checked for existence
 * @param listValue list of named values to check for existence
 * @return std::pair<bool, std::string> true if ok, false and sent error information if error
*/
std::pair<bool, std::string> arguments::exists_s( const arguments& argumentsValidate, const std::initializer_list<std::pair<std::string_view, std::string_view>>& listName, tag_description )
{
   for( auto it = std::begin( listName ), itEnd = std::end( listName ); it != itEnd; it++ )
   {
      if( argumentsValidate.exists( it->first ) == false ) return { false, std::string( it->second ) };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief copy value name into buffer
 * @param pCopyTo buffer value name is copied to
 * @param pbszName pointer to name data that is copied
 * @param uNameLength length of name in bytes to copy
 * @return unber of bytes copied into buffer
 */
uint64_t arguments::memcpy_s(pointer pCopyTo, const char* pbszName, unsigned uNameLength)
{
   auto* pdata_ = pCopyTo;

   uint32_t uTypeAndSize = (eType_ParameterName << 24) | uNameLength;
   *(uint32_t*)(pdata_) = uTypeAndSize;                                        // set name type
   pdata_ += sizeof( uint32_t );                                               // move past prefix information for name
   memcpy(pdata_, pbszName, uNameLength);                                      // copy name into buffer
   uNameLength = align32_g( uNameLength );
   pdata_ += uNameLength;                                                      // move past name content
   uint64_t uSize = pdata_ - pCopyTo;                                                              assert( uSize % 4 == 0 );
   return uSize;
}

/** ---------------------------------------------------------------------------
 * @brief Copy data into buffer and return number of bytes copied
 * @param pCopyTo pointer to position in buffer where data is copied to
 * @param uType type of value, this is set in the prefix to value
 * @param pBuffer points to buffer holding value that is copied into arguments buffer
 * @param uLength number of bytes to copy
 * @return total number of bytes that was copied into arguments buffer
 */
uint64_t arguments::memcpy_s( pointer pCopyTo,  argument_type uType, const_pointer pBuffer, unsigned int uLength )
{
   uint64_t uPosition = 0;

   if( (uType & eValueLength) == 0 )                                           // if type doesn't have specified length flag then just copy data into buffer
   {
      uint32_t uValueLength = uLength;                                         // hold value lenght
      uLength = align32_g( uLength );                                          // align to 32 bit boundary
      uint32_t uTypeAndSize = (uType << 24) | uLength;
      *(uint32_t*)(pCopyTo) = uTypeAndSize;                        // set type and size
      uPosition += sizeof( uint32_t );

      memcpy(&pCopyTo[uPosition], pBuffer, uValueLength);                      assert( uLength >= uValueLength );
      uPosition += uLength;                                                    // add aligned length
   }
   else
   {
      unsigned uTotalLength = uLength;
      uint32_t uValueLength = uLength;                                         // value length in bytes (storage needed to hold data)
      uTotalLength += sizeof( uint32_t );                                      // add value length to total value size
      uTotalLength = align32_g( uTotalLength );                                // align to 32 bit boundary
      uint32_t uTypeAndSize = (uType << 24) | uTotalLength;                    // set value type and length in 32 bit value
      *(uint32_t*)(pCopyTo + uPosition) = uTypeAndSize;                        // set type and size
      uPosition += sizeof( uint32_t );                                         // move past type and size

      uint32_t uCompleteType = gd::types::typenumber_to_type_g( uType & ~eType_MASK );

      // ## fix size to the actual length for value, this is to improve the speed
      //    generating value objects from data
      if(uCompleteType & gd::types::eTypeGroupString)
      {
         if(( uType & eTypeNumber_MASK ) == eTypeNumberWString)
         {                                                                                         assert( (uValueLength % 2) == 0 );
            uValueLength = uValueLength >> 1;                                  // unicode string, length is cut in half
         }
         uValueLength--;                                                       // remove the zero terminator for length
      }

      *(uint32_t*)(pCopyTo + uPosition) = uValueLength;
      memcpy(&pCopyTo[uPosition + sizeof( uint32_t )], pBuffer, uLength);      // copy data
      uPosition += uTotalLength;                                               // move past data for value (length and data)
      //buffer_set_size( uPosition );                                            assert(buffer_size() < buffer_buffer_size());
   }
   return uPosition;
}




_GD_ARGUMENT_SHARED_END

_GD_ARGUMENT_SHARED_BEGIN
namespace debug {

   /** ------------------------------------------------------------------------
    * @brief print data in argument to view what it contains, usefull in debug
    * @param argumentPrint argument to print
    * @return string with argument information
   */
   std::string print( const arguments::argument& argumentPrint )
   {
      std::string stringPrint;
      stringPrint += argumentPrint.as_string();
      stringPrint += " : ";
      stringPrint += arguments::type_name_s( argumentPrint.type() );

      return stringPrint;
   }

   /** ------------------------------------------------------------------------
    * @brief print data in arguments to view what it contains, usefull in debug
    * @param argumentsToPrint arguments to print
    * @return string with arguments information
   */
   std::string print( const arguments& argumentsToPrint, const std::string_view& stringDivide )
   {
      std::string stringPrint;
      for( auto pPosition = argumentsToPrint.next(); pPosition != nullptr; pPosition = argumentsToPrint.next( pPosition ) )
      {
         if( stringPrint.empty() == false ) stringPrint += stringDivide;

         if( arguments::is_name_s( pPosition ) == true )
         {
            arguments::print_name_s(pPosition, stringPrint);
            stringPrint += " = ";
         }
         arguments::print_value_s(pPosition, stringPrint);
         stringPrint += " : ";
         arguments::print_type_s(pPosition, stringPrint);
      }

      return stringPrint;
   }



   /** ------------------------------------------------------------------------
    * @brief print data in arguments to view what it contains, usefull in debug
    * @param argumentsToPrint arguments to print
    * @return string with arguments information
   */
   std::string print( const arguments& argumentsToPrint )
   {
      return print( argumentsToPrint, "\n" );
   }

   /** ------------------------------------------------------------------------
    * @brief print vector with arguments items
    * @param vectorToPrint values in vector to print
    * @return string with printed arguments values found in vector
   */
   std::string print( const std::vector<arguments>& vectorToPrint )
   {
      std::string stringPrint;

      for( auto it : vectorToPrint )
      {
         if( stringPrint.empty() == false ) stringPrint += "\n";

         stringPrint += "[ ";
         stringPrint += print( it );
         stringPrint += " ]";
      }

      return stringPrint;
   }

}
_GD_ARGUMENT_SHARED_END

#endif // GD_COMPILER_HAS_CPP20_SUPPORT