#include <iterator>
#include <cwchar>

#include "gd_utf8.h"  

#include "gd_arguments_shared.h"  



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
   enumCType eType = (enumCType)*pPosition;                                    // get section type
   if( eType == eType_ParameterName )
   {
      pPosition++;
      uint8_t uNameLength = *pPosition + 1;                                    // get name length and add one for byte that holds how many characters name has (it is not zero based)
      pPosition += uNameLength;
   }
   return pPosition;
}


arguments::pointer arguments::move_to_value_data_s(pointer pPosition)
{                                                                                                  assert( pPosition != nullptr );
   uint32_t u_ = *(uint32_t*)pPosition;
   enumCType eType = (enumCType)(u_ >> 24);
   pPosition += sizeof(uint32_t);

   return pPosition;
}

arguments::const_pointer arguments::move_to_value_data_s(const_pointer pPosition)
{                                                                                                  assert( pPosition != nullptr );
   uint32_t u_ = *(uint32_t*)pPosition;
   enumCType eType = (enumCType)(u_ >> 24);
   pPosition += sizeof(uint32_t);

   return pPosition;
}


/*----------------------------------------------------------------------------- move_to_value */ /**
 * Moves pointer to value part in argument
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
      if( (uLength % 4) != 0 ) uLength = (uLength + 3) & ~3;

      pPosition += sizeof( uint32_t ) + uLength;
   }
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
         auto uSize = *(uint32_t*)(m_unionValue.puch - 4);                                         assert(uSize < 0x00A00000); // realistic
         return uSize;
      }
      else if( eTypeNumber == eTypeNumberWString ) return (unsigned int)wcslen(m_unionValue.pwsz) * 2 + 2;

      return (unsigned int)strlen(m_unionValue.pbsz) + 1;
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

void arguments::argument_edit::set(argument argumentSet)
{
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
}


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



arguments::arguments(std::initializer_list<std::pair<std::string_view, gd::variant>> listPair)
{
   zero();
   for( auto it : listPair ) append_argument(it);
}

arguments::arguments( std::initializer_list<std::pair<std::string_view, gd::variant_view>> listPair, tag_view )
{
   zero();
   for( auto it : listPair ) append_argument( it, view_tag{} );
}

arguments::arguments( std::vector<std::pair<std::string_view, gd::variant_view>> listPair, tag_view )
{
   zero();
   for( auto it : listPair ) append_argument( it, view_tag{} );
}

arguments::arguments(const std::string_view& stringName, const gd::variant& variantValue, arguments::tag_no_initializer_list)
{
   zero();
   append_argument(stringName, variantValue);
}

/*
arguments::arguments(std::pair<std::string_view, gd::variant> pairArgument, pair_tag)
{
   zero();
   append_argument(pairArgument);
}
*/

arguments& arguments::operator=(std::initializer_list<std::pair<std::string_view, gd::variant>> listPair)
{
   for( auto it : listPair ) append_argument(it);
   return *this;
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

/*-----------------------------------------------------------------------------
 * Add typed argument to binary stream of bytes
 * \param uType type of parameter
 * \param pBuffer pointer to binary data
 * \param uLength number of bytes added to stream
 * \return arguments& reference to arguments object
 */
arguments& arguments::append( argument_type uType, const_pointer pBuffer, unsigned int uLength)
{                                                                                                  assert( (uLength > 0 && pBuffer != nullptr) || (uLength == 0 && pBuffer == nullptr) );
   reserve(m_uLength + uLength + sizeof(argument_type) + sizeof(uint32_t));
   m_pBuffer[m_uLength] = uType;                                               // type marker
   m_uLength++;
   if( (uType & eValueLength) == 0 )                                           // fixed length?
   {
      memcpy(&m_pBuffer[m_uLength], pBuffer, uLength);
      m_uLength += uLength;                                                                        assert(m_uLength < m_uBufferLength);

      return *this;
   }

   *(uint32_t*)&m_pBuffer[m_uLength] = (unsigned long)uLength;                 // set length in byte count
   m_uLength += sizeof(uint32_t);
   memcpy(&m_pBuffer[m_uLength], pBuffer, uLength);
   m_uLength += uLength;                                                                           assert(m_uLength < m_uBufferLength);

   return *this;
}


/*----------------------------------------------------------------------------- append */ /**
 * Add typed value to arguments
 * typed value is a name in ascii format with beginning marker that it is a name, then length for name
 * and the name. After name type of value is added, then the value or if value needs length the length is added.
 * Lastly data for value is added
 * [name type][name length][name][value type][value length?][value]
 * \param pbszName name for value
 * \param uNameLength character count for name
 * \param uType type of value
 * \param pBuffer data, pointer to bytes holding data
 * \param uLength length for data
 * \return arguments& argument object for nested calls
 */
arguments& arguments::append(const char* pbszName, uint32_t uNameLength, argument_type uType, const_pointer pBuffer, unsigned int uLength)
{                                                                                                  assert(m_pbuffer->get_reference_count() <= 1); 
#ifdef _DEBUG
   enumCType debug_eCType = (enumCType)(uType & ~eTypeNumber_MASK);            // get absolute variable type
#endif // _DEBUG

   uint64_t uReserveLength = buffer_size();                                    // current buffer size
   uReserveLength += uNameLength + (sizeof( uint16_t ) * 2);                   // name if any and length value for name
   uReserveLength += uLength;                                                  // value length
   uReserveLength += sizeof( uint16_t ) + sizeof(uint32_t);                    // value type and value length if needed
   uReserveLength = (uReserveLength + 3) & ~3;                                 // align
      

   // reserve data for name, length for name, name type and data type
   // [current length][data length][name length][type + length + zero ending (=argument_type*3)][length value if data isn't primitive type]
   reserve( uReserveLength );

   auto uPosition = buffer_size();                                             assert( uPosition % 4 == 0 );
   auto pdata_ = buffer_data();

   uint32_t uTypeAndSize = (eType_ParameterName << 24) | uNameLength;
   *(uint32_t*)(pdata_ + uPosition) = uTypeAndSize;                            // set name type
   uPosition += sizeof( uint32_t );
   memcpy(&pdata_[uPosition], pbszName, uNameLength);                          // copy name
   uPosition += uNameLength;                                                   // add length for name
   uPosition = (uPosition + 3) & ~3;                                           // align to four byte boundary 
                                                                               assert( uPosition % 4 == 0 );
   uTypeAndSize = (uType << 24) | uLength;
   *(uint32_t*)(pdata_ + uPosition) = uTypeAndSize;                            // set type and size
   uPosition += sizeof( uint32_t );

   if( (uType & eValueLength) == 0 )                                           // if type doesn't have specified length flag then just copy data into buffer
   {
      memcpy(&pdata_[uPosition], pBuffer, uLength);
      if( (uLength % sizeof( uint32_t )) == 0 ) uPosition += uLength;
      else uPosition += (uLength + 3) & ~3;                                    // add length and align
      buffer_set_size( uPosition );
                                                                                                   assert(m_uLength <= m_uBufferLength);
      return *this;
   }

   //*(uint32_t*)&m_pBuffer[m_uLength] = (uint32_t)uLength;                        // set length for data
   //m_uLength += sizeof(uint32_t);
   memcpy(&pdata_[uPosition], pBuffer, uLength);                               // copy data
   uPosition += (uLength + 3) & ~3;                                            // add and align
   buffer_set_size( uPosition );

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

/*----------------------------------------------------------------------------- append_argument */ /**
 * Add argument from variant_view
 * \param stringName argument name
 * \param variantValue argument value added
 * \return arguments::arguments& reference to this if nested operations is wanted
 */
arguments& arguments::append_argument(const std::string_view& stringName, const gd::variant_view& variantValue)
{
   auto argumentValue = get_argument_s(variantValue);
   const_pointer pData = (argumentValue.type_number() <= eTypeNumberPointer ? (const_pointer)&argumentValue.m_unionValue : (const_pointer)argumentValue.get_raw_pointer());
   unsigned uType = argumentValue.type_number();
   unsigned uLength;
   if( uType > ARGUMENTS_NO_LENGTH )
   {
      unsigned uZeroEnd = 0;
      if( uType >= eTypeNumberString && uType <= eTypeNumberBinary ) { uType |= eValueLength; }

      uLength = variantValue.length() + get_string_zero_terminate_length_s(uType);

      return append(stringName, uType, pData, uLength);
   }
   return append(stringName, uType, pData, argumentValue.length());
}

arguments& arguments::append_argument(const std::string_view stringName, const std::string_view& stringValue, tag_parse_type )
{
   gd::variant_view v_ = stringValue;
   unsigned uTypeGroup = gd::types::detect_ctypegroup_g( stringValue );
   if( uTypeGroup == gd::types::eTypeGroupInteger )
   {
      auto to_ = v_.convert_to( gd::types::type_g( "int64" ) );
      append_argument( stringName, to_ );
   }
   else if( uTypeGroup == gd::types::eTypeGroupInteger )
   {
      auto to_ = v_.convert_to( gd::types::type_g( "double" ) );
      append_argument( stringName, to_ );
   }
   else { append_argument( stringName, v_ ); }

   return *this;
}

arguments& arguments::append_argument( const std::initializer_list< std::pair<std::string_view, gd::variant_view> >& vectorArgument, tag_view )
{
   for( const auto& it : vectorArgument )
   {
      append_argument( it, tag_view{});
   }

   return *this;
}


arguments& arguments::append_argument( const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorArgument, tag_view )
{
   for( const auto& it : vectorArgument )
   {
      append_argument( it, tag_view{});
   }

   return *this;
}

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
      pPosition = move_to_value_data_s( pPosition );                           assert(pPosition < get_buffer_end());
      memcpy(pPosition, pBuffer, uLength);
   }
   else
   {
      auto uOldSize = arguments::sizeof_name_s( uNameLength ) +  arguments::sizeof_s( argumentOld ); // calculate total size for old value
      auto uNewSize = arguments::sizeof_s( uNameLength, uType, uLength );      // calculate total size for new value

      

      if( uOldSize != uNewSize ) 
      { 
         if( uNewSize > uOldSize ) 
         { 
            size_t uOffset = pPosition - m_pBuffer;                            // offset value from start of buffer, needed to set new position if new block is allocated
            
            // reserv memory if needed and if memory is reserved, reset position
            if( reserve( m_uLength + (uNewSize - uOldSize) ) == true ) { pPosition = m_pBuffer + uOffset; }
         }

         resize(pPosition, uOldSize, uNewSize); 
      }

      pPosition += arguments::sizeof_name_s(uNameLength);
      m_uLength += int(uNewSize - uOldSize);

      *pPosition = uType;
      pPosition++;

      if( (uType & eValueLength) == 0 )                                          // if type doesn't have specified length flag then just copy data into buffer
      {
         memcpy(pPosition, pBuffer, uLength);
      }
      else
      {
         *(uint32_t*)pPosition = (uint32_t)uLength;                              // set length for data
         pPosition += sizeof(uint32_t);
         memcpy(pPosition, pBuffer, uLength);                                    // copy data
      }
   }



   return *this;
}

arguments& arguments::set(pointer pPosition, param_type uType, const_pointer pBuffer, unsigned int uLength)
{
   // get current argument
   argument argumentOld = arguments::get_argument_s(pPosition);
   auto pPositionValue = move_to_value_s(pPosition);
   if( arguments::compare_type_s(argumentOld, uType) == true && (uType & (eValueLength | eValueLengthBig)) == 0 )
   {
      memcpy(pPositionValue, pBuffer, uLength);
   }
   else
   {                                                                                               assert( pPositionValue - pPosition < 255 ); // realistic
      auto uOldSize = get_total_param_length_s(pPosition);
      unsigned uNameLength = arguments::sizeof_name_s( pPosition );
      auto uNewSize = uNameLength + uLength + sizeof_value_prefix( uType );
      if( uOldSize != uNewSize ) 
      { 
         if( uOldSize < uNewSize ) reserve(m_uLength + (uNewSize - uOldSize));  // increase buffer if needed
         resize(pPosition, uOldSize, uNewSize); 
      }

      m_uLength += int(uNewSize - uOldSize);

      pPosition += uNameLength;
      *pPosition = uType;
      pPosition++;

      if( (uType & eValueLength) == 0 )                                          // if type doesn't have specified length flag then just copy data into buffer
      {
         memcpy(pPosition, pBuffer, uLength);
      }
      else
      {
         *(uint32_t*)pPosition = (uint32_t)uLength;                              // set length for data
         pPosition += sizeof(uint32_t);
         memcpy(pPosition, pBuffer, uLength);                                    // copy data
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
 * \param pPosition position where to start from
 * \return gd::argument::arguments::const_pointer position to name if found, otherwise null
 */
arguments::const_pointer arguments::find(std::string_view stringName, const_pointer pPosition) const
{
   for( ; pPosition != nullptr; pPosition = next(pPosition) )
   {
      if( arguments::is_name_s(pPosition) == true )
      {
         auto key_ = arguments::get_name_s(pPosition);
         if( key_ == stringName ) return pPosition;
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

std::string arguments::print(std::string_view stringFormat) const
{
   char pbszBuffer[256];
   std::string stringPrint;
   auto uLength = stringFormat.length();

   auto next = [](auto it, auto itEnd) -> auto {
      while( it != itEnd )
      {
         it++;
         if( *it == '{' )
         {
            it++;
            return it;
         }
      }
      return it;
   };

   auto itPosition = std::begin(stringFormat);
   auto itEnd = std::end(stringFormat);
   while( itPosition != itEnd )
   {
      auto itTo = next( itPosition, itEnd );
      std::copy(itPosition, itTo, std::back_inserter(stringPrint));
      if( *itTo != '{' )
      {
         *pbszBuffer = '\0';
         auto pbszPosition = pbszBuffer;
         while( *itTo != '}' && itTo != itEnd )
         {
            *pbszPosition += *itTo;
            pbszPosition++;
            itTo++;
         }

         *pbszPosition += '\0';

         stringPrint += get_argument(pbszBuffer).get_string();

      }

      itPosition = itTo;
   }

   /*
   for( auto itPosition = std::begin( stringFormat ); itPosition != std::end( stringFormat ); itPosition++ )
   {
      if( *itPosition == '{' )
      {
         itPosition++;
         if( itPosition == std::end(stringFormat) || *(itPosition + 1) == '{' )
         {
            stringPrint += '{';
            if( itPosition != std::end(stringFormat) ) itPosition++;
            continue;
         }
      }
   }
   */

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



/*----------------------------------------------------------------------------- reserve */ /**
 * Make sure internal buffer can hold number of bytes requested
 \note if buffer is increased it is increased with 1.5 * count
 * \param uCount number of bytes to reserve
 * \return bool true if new buffer is allocated, false if not
 */
bool arguments::reserve(uint64_t uCount)
{                                                                                                  assert( (uCount % 4) == 0 );
   if( uCount > buffer_buffer_size() )
   {
      uint64_t uBufferSize = (sizeof( buffer ) + uCount + ( uCount >> 1 ) + 63) & ~63;             // calculate new size and align to 64 byte
      uint8_t* pdata_ = new unsigned char[uBufferSize];

      uint64_t uOldSize = sizeof( buffer );
      uOldSize += buffer_size();

      memcpy(pdata_, m_pbuffer, uOldSize);

      buffer_release();
      m_pbuffer = (buffer*)pdata_;

      uBufferSize -= sizeof( buffer );
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

   m_uLength -= uSize;                                                           assert((int)m_uLength >= 0);
}


// TODO: Implement resize in order to be able to modify a value in arguments object
/*----------------------------------------------------------------------------- resize */ /**
 * Resize buffer, this is used to make space for new value at specific position (replace value)
 * \param pPosition
 * \param iOffset
 * \param iNewOffset
 * \return int
 */
int arguments::resize(pointer pPosition, int iOffset, int iNewOffset)
{
   int iSizeChange = iNewOffset - iOffset;

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
      //memmove( (void*)(pPosition + iNewOffset), (void*)(pPosition + iOffset), get_buffer_end() - (pPosition + iOffset) ); // move memory
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
{
   if( capacity() > size( tag_memory{} ) )
   {
      unsigned char* pBuffer = new unsigned char[m_uLength];                   // new buffer
      memcpy(pBuffer, m_pBuffer, m_uLength);                                   // copy data

      if( is_owner() ) delete m_pBuffer;                                       // clear old buffer

      m_bOwner = true;
      m_pBuffer = pBuffer;

      m_uBufferLength = m_uLength;                                             // set buffer size
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
   m_pbuffer->release();
   m_pbuffer = &m_buffer_s;
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
      auto stringExistsName = it.name( view_tag{} );
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
      if( (uLength % 4) != 0 ) uLength = (uLength + 3) & ~3;
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
unsigned int arguments::get_total_param_length_s(const_pointer pPosition)
{
   const_pointer pEnd = next_s( pPosition );
   return static_cast<unsigned int>(pEnd - pPosition);
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

/*----------------------------------------------------------------------------- get_total_param_length_s */ /**
 * Calculate number of bytes needed to store named value in internal buffer used by arguments object
 * \param stringName name for value
 * \param argumentValue value store for name
 * \return unsigned int number of bytes needed to store value in internal buffer
 */
unsigned int arguments::get_total_param_length_s(std::string_view stringName, const argument argumentValue)
{
   return 1 + (unsigned int)stringName.length() + sizeof_s(argumentValue);
}


/*----------------------------------------------------------------------------- next */ /**
 * Move to next element in binary stream. Make sure that next value is compatible type
 * \param riPosition current position in stream
 * \return arguments::pointer position to next value
 */
arguments::pointer arguments::next_s(pointer pPosition) 
{
   pPosition = arguments::move_to_value_s(pPosition);
   uint8_t uType = *pPosition;                                                   // get value type
   pPosition++;                                                                  // move past type
   if( (uType & eType_MASK) == 0 )
   {
      pPosition += ctype_size[uType];
   }
   else
   {
      if( uType & eValueLength )
      {
         uint32_t uLength = *(uint32_t*)pPosition;
         pPosition += sizeof(uint32_t);
         pPosition += uLength;
      }
   }

   return pPosition;
}

/*----------------------------------------------------------------------------- next */ /**
 * Move to next element in binary stream. Make sure that next value is compatible type
 * \param riPosition current position in stream
 * \return arguments::const_pointer position to next value
 */
arguments::const_pointer arguments::next_s(const_pointer pPosition) 
{
   pPosition = arguments::move_to_value_s(pPosition);
   uint32_t u_ = *(uint32_t*)pPosition;
   uint32_t uType = u_ >> 24;                                                  // get value type
   uint32_t uLength = u_ & 0x00FFFFFF;
   pPosition += sizeof(uint32_t);
   if( (uType & eType_MASK) == 0 )
   {
      if( uType < eTypeNumberString ) 
      { 
         uLength = ctype_size[uType];
      }
      
      if( uLength % 4 != 0 ) uLength = (uLength + 3) & ~3;
      pPosition += uLength;
   }
   else
   {
      if( uType & eValueLength )
      {
         if( uLength % 4 != 0 ) uLength = (uLength + 3) & ~3;
         pPosition += uLength;
      }
      else
      {
         // TODO: implement big length
         assert( false );
      }
   }

   return pPosition;
}



/*----------------------------------------------------------------------------- sizeof_s */ /**
 * Return size for argument in bytes
 * \param argumentValue argument value size is returned for
 * \return unsigned int number of bytes for value
 */
unsigned int arguments::sizeof_s(const argument& argumentValue)
{
   unsigned int uSize = 1;                                                       // start with size for type prefix (1 byte value in front);
   if( argumentValue.ctype() & eValueLength )  uSize += sizeof(uint32_t);        // add size value in front of value

   return uSize + argumentValue.length();
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
   if( *pPosition == eType_ParameterName )
   {
      if( uPairType & ePairTypeKey )
      {
         stringArgument += "\"";
         stringArgument.append( reinterpret_cast<const char*>( pPosition+ 2), *(pPosition + 1) );
         stringArgument += "\": ";
      }
      pPosition += size_t(pPosition[1]) + size_t(2);                             // move to value
   }

   if( uPairType & ePairTypeValue )
   {
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
   unsigned int uSize = 3;                                                       // 3 - one byte for name type, one byte for name length, and one byte for value type
   uSize += uNameLength;                                                         // add name length
   if( uType & eValueLength ) uSize += sizeof(uint32_t); 
   uSize += uValueLength;

   return uSize;
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
   case arguments::eTypeNumberString:
      return gd::variant_view(value.pbsz, (size_t)argumentValue.length() - sizeof(char) );
      break;
   case arguments::eTypeNumberUtf8String:
      return gd::variant_view( variant_type::utf8( value.pbsz, (size_t)argumentValue.length() - sizeof(char)) );
      break;
   case arguments::eTypeNumberWString:
      return gd::variant_view(value.pwsz, (size_t)argumentValue.length() - sizeof(wchar_t));
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

         arguments::print_name_s(pPosition, stringPrint);
         stringPrint += " = ";
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

   /** ------------------------------------------------------------------------
    * @brief print vector with arguments items
    * @param pvectorToPrint values in vector to print
    * @return string with printed arguments values found in vector
   */
   /*
   std::string print( const std::vector<arguments>* pvectorToPrint )
   {
      std::string stringPrint;

      for( auto it = std::begin( *pvectorToPrint ), itEnd = std::end( *pvectorToPrint ); it != itEnd; it++ )
      {
         if( stringPrint.empty() == false ) stringPrint += "\n";

         stringPrint += "[ ";
         stringPrint += print( *it );
         stringPrint += " ]";
      }

      return stringPrint;
   }
   */

}
_GD_ARGUMENT_SHARED_END
