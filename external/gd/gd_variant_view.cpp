// @FILE [tag: variant, view] [description: General variant item that owns its data, manages all primitive types and some common] [type: source]

#include <charconv>
#include <cmath>
#include <cwchar>

#include "gd_utf8.h"
#include "gd_variant_view.h"



#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #pragma clang diagnostic ignored "-Wswitch"
   #pragma clang diagnostic ignored "-Wformat"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
   #pragma GCC diagnostic ignored "-Wswitch"
   #pragma GCC diagnostic ignored "-Wformat"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996 26812 )
#endif



#ifndef _GD_BEGIN
namespace gd {
#else
_GD_BEGIN
#endif

using namespace variant_type;


bool variant_view::get_bool() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return false;
   case eTypeNumberBit         : return m_V.int8 != 0 ? true : false;
   case eTypeNumberBool        : return m_V.b;
   case eTypeNumberInt8        : return m_V.int8 != 0 ? true : false;
   case eTypeNumberInt16       : return m_V.int16 != 0 ? true : false;
   case eTypeNumberInt32       : return m_V.int32 != 0 ? true : false;
   case eTypeNumberInt64       : return m_V.int64 != 0 ? true : false;
   case eTypeNumberUInt8       : return m_V.uint8 != 0 ? true : false;
   case eTypeNumberUInt16      : return m_V.uint16 != 0 ? true : false;
   case eTypeNumberUInt32      : return m_V.uint32 != 0 ? true : false;
   case eTypeNumberUInt64      : return m_V.uint64 != 0 ? true : false;
   case eTypeNumberFloat       : return m_V.f != 0.0 ? true : false;
   case eTypeNumberDouble      : return m_V.d != 0.0 ? true : false;
   case eTypeNumberGuid        : return false;
   case eTypeNumberBinary      : return false;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : return *m_V.pbsz != '\0' ? true : false;
   case eTypeNumberWString     : return *m_V.pwsz != L'\0' ? true : false;
   default:                                                                      assert( false );
   }

   return false;
}


int variant_view::get_int() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return 0;
   case eTypeNumberBit         : return (m_V.int8 & 0x01 ? 1 : 0);
   case eTypeNumberBool        : return (m_V.b == true ? 1 : 0);
   case eTypeNumberInt8        : return (int)m_V.int8;
   case eTypeNumberInt16       : return (int)m_V.int16;
   case eTypeNumberInt32       : return (int)m_V.int32;
   case eTypeNumberInt64       : return (int)m_V.int64;
   case eTypeNumberUInt8       : return (int)m_V.uint8;
   case eTypeNumberUInt16      : return (int)m_V.uint16;
   case eTypeNumberUInt32      : return (int)m_V.uint32;
   case eTypeNumberUInt64      : return (int)m_V.uint64;
   case eTypeNumberFloat       : return (int)m_V.f;
   case eTypeNumberDouble      : return (int)m_V.d;
   case eTypeNumberGuid        : return 0;
   case eTypeNumberBinary      : return 0;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : return atoi( m_V.pbsz );
#if defined(_MSC_VER)
   case eTypeNumberWString     : return _wtoi( m_V.pwsz );
#else   
   case eTypeNumberWString     : return wcstol( m_V.pwsz, 0, 10 );
#endif   
   default:                                                                      assert( false );
   }

   return 0;
}

unsigned int variant_view::get_uint() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return 0;
   case eTypeNumberBit         : return (m_V.int8 & 0x01 ? 1 : 0);
   case eTypeNumberBool        : return (m_V.b == true ? 1 : 0);
   case eTypeNumberInt8        : return (unsigned int)m_V.int8;
   case eTypeNumberInt16       : return (unsigned int)m_V.int16;
   case eTypeNumberInt32       : return (unsigned int)m_V.int32;
   case eTypeNumberInt64       : return (unsigned int)m_V.int64;
   case eTypeNumberUInt8       : return (unsigned int)m_V.uint8;
   case eTypeNumberUInt16      : return (unsigned int)m_V.uint16;
   case eTypeNumberUInt32      : return (unsigned int)m_V.uint32;
   case eTypeNumberUInt64      : return (unsigned int)m_V.uint64;
   case eTypeNumberFloat       : return (unsigned int)m_V.f;
   case eTypeNumberDouble      : return (unsigned int)m_V.d;
   case eTypeNumberGuid        : return 0;
   case eTypeNumberBinary      : return 0;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : return (unsigned int)atoi( m_V.pbsz );
#if defined(_MSC_VER)
   case eTypeNumberWString     : return (unsigned int)_wtoi( m_V.pwsz );
#else   
   case eTypeNumberWString     : return (unsigned int)wcstol( m_V.pwsz, 0, 10 );
#endif   

   default:                                                                      assert( false );
   }

   return 0;
}


int64_t variant_view::get_int64() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return 0;
   case eTypeNumberBit         : return (m_V.int8 & 0x01 ? 1 : 0);
   case eTypeNumberBool        : return (m_V.b == true ? 1 : 0);
   case eTypeNumberInt8        : return (int64_t)m_V.int8;
   case eTypeNumberInt16       : return (int64_t)m_V.int16;
   case eTypeNumberInt32       : return (int64_t)m_V.int32;
   case eTypeNumberInt64       : return (int64_t)m_V.int64;
   case eTypeNumberUInt8       : return (int64_t)m_V.uint8;
   case eTypeNumberUInt16      : return (int64_t)m_V.uint16;
   case eTypeNumberUInt32      : return (int64_t)m_V.uint32;
   case eTypeNumberUInt64      : return (int64_t)m_V.uint64;
   case eTypeNumberFloat       : return (int64_t)m_V.f;
   case eTypeNumberDouble      : return (int64_t)m_V.d;
   case eTypeNumberGuid        : return 0;
   case eTypeNumberBinary      : return 0;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : return atoll( m_V.pbsz );// _atoi64( m_V.pbsz ); #GCC
   case eTypeNumberWString     : return std::wcstoll( m_V.pwsz, 0, 10 );
   //case eTypeNumberUtf32String : return _wtoi64( m_V.pwsz );                   // #TODO fix this
   default:                                                                                        assert( false );
   }

   return 0;
}

uint64_t variant_view::get_uint64() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return 0;
   case eTypeNumberBit         : return (m_V.int8 & 0x01 ? 1 : 0);
   case eTypeNumberBool        : return (m_V.b == true ? 1 : 0);
   case eTypeNumberInt8        : return (uint64_t)m_V.int8;
   case eTypeNumberInt16       : return (uint64_t)m_V.int16;
   case eTypeNumberInt32       : return (uint64_t)m_V.int32;
   case eTypeNumberInt64       : return (uint64_t)m_V.int64;
   case eTypeNumberUInt8       : return (uint64_t)m_V.uint8;
   case eTypeNumberUInt16      : return (uint64_t)m_V.uint16;
   case eTypeNumberUInt32      : return (uint64_t)m_V.uint32;
   case eTypeNumberUInt64      : return (uint64_t)m_V.uint64;
   case eTypeNumberFloat       : return (uint64_t)m_V.f;
   case eTypeNumberDouble      : return (uint64_t)m_V.d;
   case eTypeNumberGuid        : return 0;
   case eTypeNumberBinary      : return 0;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      :
   {
      uint64_t v_;      
      std::from_chars( m_V.pbsz, m_V.pbsz + length(), v_ );
      return v_;
   }
   case eTypeNumberWString     : return (uint64_t)std::wcstoll( m_V.pwsz, 0, 10 );
   //case eTypeNumberUtf32String : return _wtoi64( m_V.pwsz );                   // #TODO fix this
   default:                                                                                        assert( false );
   }

   return 0;
}



double variant_view::get_decimal() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return 0.0;
   case eTypeNumberBit         : return (m_V.int8 & 0x01 ? 1.0 : 0.0);
   case eTypeNumberBool        : return (m_V.b == true ? 1.0 : 0.0);
   case eTypeNumberInt8        : return (double)m_V.int8;
   case eTypeNumberInt16       : return (double)m_V.int16;
   case eTypeNumberInt32       : return (double)m_V.int32;
   case eTypeNumberInt64       : return (double)m_V.int64;
   case eTypeNumberUInt8       : return (double)m_V.uint8;
   case eTypeNumberUInt16      : return (double)m_V.uint16;
   case eTypeNumberUInt32      : return (double)m_V.uint32;
   case eTypeNumberUInt64      : return (double)m_V.uint64;
   case eTypeNumberFloat       : return (double)m_V.f;
   case eTypeNumberDouble      : return m_V.d;
   case eTypeNumberGuid        : return 0.0;
   case eTypeNumberBinary      : return 0.0;
   case eTypeNumberUtf8String  :
   case eTypeNumberString: {
      double dValue = std::strtod( c_str(), nullptr );
      return dValue;
      }
      break;
   case eTypeNumberWString     :
      break;
   }

   return 0.0;
}

std::string variant_view::get_string() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return std::string();
   case eTypeNumberBool        : return m_V.b == false ? "0" : "1" ;
   case eTypeNumberInt8        : return std::to_string( m_V.int8 );
   case eTypeNumberInt16       : return std::to_string( m_V.int16 );
   case eTypeNumberInt32       : return std::to_string( m_V.int32 );
   case eTypeNumberInt64       : return std::to_string( m_V.int64 );
   case eTypeNumberUInt8       : return std::to_string( m_V.uint8 );
   case eTypeNumberUInt16      : return std::to_string( m_V.uint16 );
   case eTypeNumberUInt32      : return std::to_string( m_V.uint32 );
   case eTypeNumberUInt64      : return std::to_string( m_V.uint64 );
   case eTypeNumberFloat       : return std::to_string(m_V.f);
   case eTypeNumberDouble: {
      //return std::to_string( m_V.d );
      char pbszBuffer[32];
      if( variant::is_exponent_s( m_V.d, 15 ) == false )
      {
         sprintf( pbszBuffer, "%.9f", m_V.d );
         char* pLastZero = pbszBuffer;
         while( *pLastZero ) pLastZero++;
         pLastZero--;
         if( pLastZero != pbszBuffer )
         {
            while( *pLastZero == '0' && pLastZero > pbszBuffer ) pLastZero--;
            if( *pLastZero != '.' ) pLastZero++;
         }
      
         return std::string( pbszBuffer, pLastZero - pbszBuffer );
      }
      else sprintf( pbszBuffer, "%.9g", m_V.d );
      return std::string( pbszBuffer );
   }
   case eTypeNumberGuid        : {  
      std::string stringHex;
      gd::utf8::print_hex( m_V.pb, m_V.pb + 16, stringHex );
      return stringHex;
      }
      break;
   case eTypeNumberBinary      : {
      std::string stringHex;
      gd::utf8::print_hex( m_V.pb, m_V.pb + m_uSize, stringHex );
      return stringHex;
      }
      break;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : return std::string( m_V.pbsz, m_uSize ); 
   case eTypeNumberWString     : 
   {
      std::string s;
      gd::utf8::convert_utf16_to_uft8( reinterpret_cast<const uint16_t*>(m_V.pwsz), s);
      return std::move(s);
   }
      /*
   case eTypeNumberJson        : return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ); 
   case eTypeNumberXml         : return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ); 
      break;
   */
   }

   return std::string();
}

std::string variant_view::get_string( gd::variant_type::tag_scientific ) const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return std::string();
   case eTypeNumberBool        : return std::to_string( m_V.b );
   case eTypeNumberInt8        : return std::to_string( m_V.int8 );
   case eTypeNumberInt16       : return std::to_string( m_V.int16 );
   case eTypeNumberInt32       : return std::to_string( m_V.int32 );
   case eTypeNumberInt64       : return std::to_string( m_V.int64 );
   case eTypeNumberUInt8       : return std::to_string( m_V.uint8 );
   case eTypeNumberUInt16      : return std::to_string( m_V.uint16 );
   case eTypeNumberUInt32      : return std::to_string( m_V.uint32 );
   case eTypeNumberUInt64      : return std::to_string( m_V.uint64 );
   case eTypeNumberFloat       : {
      char pbszBuffer[32];
      sprintf( pbszBuffer, "%e", static_cast< double >(m_V.f) );
      return std::string( pbszBuffer );
   }
   break;
   case eTypeNumberDouble      : {
      char pbszBuffer[40];
      bool bExponent = false;
      if(m_V.d != 0.0)
      {
         if( m_V.d > 10000 ) bExponent = true;
         else
         {
            double dTest = std::round(m_V.d * 100000) / 100000;
            if( dTest != m_V.d ) bExponent = true;
         }
      }
      if( bExponent == true ) { sprintf( pbszBuffer, "%e", m_V.d ); }
      else                    { sprintf( pbszBuffer, "%.9g", m_V.d ); }

      return std::string( pbszBuffer );
   }
   break;
   case eTypeNumberGuid        : {  
      std::string stringHex;
      gd::utf8::print_hex( m_V.pb, m_V.pb + 16, stringHex );
      return stringHex;
   }
   break;
   case eTypeNumberBinary      : {
      std::string stringHex;
      gd::utf8::print_hex( m_V.pb, m_V.pb + m_uSize, stringHex );
      return stringHex;
   }
   break;
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : return std::string( m_V.pbsz, m_uSize ); 
   case eTypeNumberWString     : 
   {
      std::string s;
      gd::utf8::convert_utf16_to_uft8( reinterpret_cast<const uint16_t*>(m_V.pwsz), s);
      return std::move(s);
   }
      /*
   case eTypeNumberJson        : return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ); 
   case eTypeNumberXml         : return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ); 
      break;
   */
   }

   return std::string();
}


/** ---------------------------------------------------------------------------
 * @brief return value as string_view
 * @return std::string_view value as string_view
*/
std::string_view variant_view::get_string_view() const
{
   if( is_char_string() == true ) return std::string_view(c_str(), length());
   return std::string_view();
}


/*
std::string variant_view::get_utf8string() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return std::string();
   case eTypeNumberBit         : return std::string( gd_std::string_buffer::c_type( m_V.int8 ).c_str() );
   case eTypeNumberBool        : return std::string( string_buffer::c_type( m_V.b ).c_str() );
   case eTypeNumberInt8        : return std::string( string_buffer::c_type( m_V.int8 ).c_str() );
   case eTypeNumberInt16       : return std::string( string_buffer::c_type( m_V.int16 ).c_str() );
   case eTypeNumberInt32       : return std::string( string_buffer::c_type( m_V.int32 ).c_str() );
   case eTypeNumberInt64       : return std::string( string_buffer::c_type( m_V.int64 ).c_str() );
   case eTypeNumberUInt8       : return std::string( string_buffer::c_type( m_V.uint8 ).c_str() );
   case eTypeNumberUInt16      : return std::string( string_buffer::c_type( m_V.uint16 ).c_str() );
   case eTypeNumberUInt32      : return std::string( string_buffer::c_type( m_V.uint32 ).c_str() );
   case eTypeNumberUInt64      : return std::string( string_buffer::c_type( m_V.uint64 ).c_str() );
   case eTypeNumberCFloat      : return std::string( string_buffer::c_type( m_V.f,  gd_std::string::eConvertNoExponent ).c_str() );
   case eTypeNumberCDouble     : return std::string( string_buffer::c_type( m_V.d,  gd_std::string::eConvertNoExponent ).c_str() );
   case eTypeNumberUtf8String  : return std::string( m_V.pbsz, m_uSize ); 
   case eTypeNumberString      : 
      {
         auto _length = string::utf8_encode( m_V.pbsz, m_V.pbsz + m_uSize, nullptr );
         std::vector<unsigned char> vectorString;

         vectorString.reserve( _length + 1 );
         vectorString.resize( _length );
         string::utf8_encode( m_V.pbsz, m_V.pbsz + m_uSize, vectorString.data() );
         return std::string( vectorString.begin(), vectorString.end() ); 
      }
   case eTypeNumberWString : 
      {
         auto _length = string::utf8_encode( m_V.pwsz, m_V.pwsz + m_uSize, nullptr );
         std::vector<unsigned char> vectorString;

         vectorString.reserve( _length + 1 );
         vectorString.resize( _length );

         //string::utf8_encode( m_V.pwsz, m_V.pwsz + m_uSize, vectorString.data() );
         auto _size = string::utf8_encode_validate( m_V.pwsz, m_V.pwsz + m_uSize, vectorString.data() );
         if( _size < _length ) { vectorString.resize( _size ); }

         return std::string( vectorString.begin(), vectorString.end() ); 
      }
   case eTypeNumberJson        : return std::string( m_V.pbsz, m_uSize ); 
   case eTypeNumberXml         : return std::string( m_V.pbsz, m_uSize );
      break;
   }

   return std::string();

}
*/


std::wstring variant_view::get_wstring() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown: return std::wstring();
   case eTypeNumberBit: return std::to_wstring(m_V.int8);
   case eTypeNumberBool: return std::to_wstring(m_V.b);
   case eTypeNumberInt8: return std::to_wstring(m_V.int8);
   case eTypeNumberInt16: return std::to_wstring(m_V.int16);
   case eTypeNumberInt32: return std::to_wstring(m_V.int32);
   case eTypeNumberInt64: return std::to_wstring(m_V.int64);
   case eTypeNumberUInt8: return std::to_wstring(m_V.uint8);
   case eTypeNumberUInt16: return std::to_wstring(m_V.uint16);
   case eTypeNumberUInt32: return std::to_wstring(m_V.uint32);
   case eTypeNumberUInt64: return std::to_wstring(m_V.uint64);
   case eTypeNumberFloat: return std::to_wstring(m_V.f); 
   case eTypeNumberDouble: return std::to_wstring(m_V.d); 
   //case eTypeNumberUtf8String: return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ) ).get_wstring();
   //case eTypeNumberString: return gd_std::wstring( m_V.pbsz ).get_wstring();
   case eTypeNumberWString: return std::wstring( m_V.pwsz, m_V.pwsz + m_uSize );
   //case eTypeNumberJson: return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ).get_wstring();
   //case eTypeNumberXml: return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ).get_wstring();
   default:                                                                      assert(false);

   }

   return std::wstring();
}

char* variant_view::get_string( char* pbszBuffer ) const
{
   char* pbszStart = pbszBuffer;
   switch( type_number() )
   {
   case eTypeNumberUnknown     : return pbszBuffer;
   case eTypeNumberBool        :
      *pbszBuffer = m_V.b == true ? '1' : '0'; 
      pbszBuffer++;
      break;
   case eTypeNumberInt8        :
      return (char*)gd::utf8::itoa( static_cast<int32_t>(m_V.int8), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberUInt8       : 
      return (char*)gd::utf8::utoa( static_cast<uint32_t>(m_V.uint8), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberInt16       : 
      return (char*)gd::utf8::itoa( static_cast<int32_t>(m_V.int16), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberUInt16      : 
      return (char*)gd::utf8::utoa( static_cast<uint32_t>(m_V.uint16), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberInt32       : 
      return (char*)gd::utf8::itoa( static_cast<int32_t>(m_V.int32), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberUInt32      : 
      return (char*)gd::utf8::utoa( static_cast<uint32_t>(m_V.uint32), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberInt64       : 
      return (char*)gd::utf8::itoa( static_cast<int64_t>(m_V.int64), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberUInt64      : 
      return (char*)gd::utf8::utoa( static_cast<uint64_t>(m_V.uint64), reinterpret_cast<uint8_t*>(pbszBuffer) );
   case eTypeNumberFloat       : 
      sprintf(pbszBuffer, "%f", static_cast<double>(m_V.f));
      pbszBuffer += strlen( pbszBuffer );
      break;
   case eTypeNumberDouble      : 
      sprintf(pbszBuffer, "%f", m_V.d);
      pbszBuffer += strlen( pbszBuffer );
      break;
      /*
   case eTypeNumberGuid        : 
      {  assert( false );
      gd_std::wstring s;
      s.assign_hex( m_V.pb, m_uSize );
      return s;
      }
   case eTypeNumberBinary      :
      {
         gd_std::wstring s;
         s.assign_hex( m_V.pb, m_uSize );
         return s;
      }
      */
   case eTypeNumberUtf8String  : 
   case eTypeNumberString      : 
      strncpy(pbszBuffer, m_V.pbsz, m_uSize);
      pbszBuffer += m_uSize;
      break;
   case eTypeNumberWString     : 
   {
      std::string s;
      gd::utf8::convert_unicode_to_ascii( m_V.pwsz, pbszBuffer, pbszBuffer + m_uSize);
      pbszBuffer += m_uSize;
   }
      /*
   case eTypeNumberJson        : return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ); 
   case eTypeNumberXml         : return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ); 
      break;
   */
   }
   
   *pbszBuffer = '\0';
   return pbszBuffer;
}



/** ---------------------------------------------------------------------------
 * @brief checks if we have a "true" value
 * A true value is a valid type with content, If variant is a number then 0 is
 * false, all other numbers are true. If string then empty string is false, string
 * with characters is true. If binary then empty binary is false, binary with content
 * is true.
 * 
*Sample code to show how `is_true` behaves.*
@code
gd::variant_view variantviewTest;

variantviewTest = 0;     assert( variantviewTest.is_true() == false );
variantviewTest = 1;     assert( variantviewTest.is_true() == true );
variantviewTest = -1;    assert( variantviewTest.is_true() == true );
variantviewTest = 321;   assert( variantviewTest.is_true() == true );
variantviewTest = "";    assert( variantviewTest.is_true() == false );
variantviewTest = L"";   assert( variantviewTest.is_true() == false );
variantviewTest = (void*)nullptr;   assert( variantviewTest.is_true() == false );
@endcode

 * @return true if variant has content, false if not
*/
bool variant_view::is_true() const
{
   if( (unsigned int)m_uType & variant_type::eGroupInteger ) 
   { 
      if( m_uType & variant_type::eGroupSize32 ) return m_V.int32 != 0;
      else if(m_uType & variant_type::eGroupSize64 ) return m_V.uint64 != 0; 
      else if(m_uType & variant_type::eGroupSize16 ) return m_V.uint16 != 0; 
      return m_V.uint8 != 0;
   }
   else if( (unsigned int)m_uType & variant_type::eGroupBoolean ) { return m_V.b; }
   else if( m_uType & variant_type::eGroupDecimal ) 
   { 
      if( m_uType & variant_type::eGroupSize64 ) return m_V.d != 0;
      return m_V.f != 0.0;
   }
   else if( (unsigned int)m_uType & variant_type::eGroupString ) 
   {                                                                             assert( m_V.pwsz != nullptr );
      if( type_number() == eTypeNumberWString )
      {
         if(m_V.pwsz[0] != L'\0') return true; 
      }
      else if(m_V.pbsz[0] != '\0') return true; 
   }
   else if( ( unsigned int )m_uType & variant_type::eGroupBinary )
   {
      return m_uSize > 0;                                                      // has binary content ?
   }
   else if( m_uType == eTypePointer )
   {
      return m_V.p != nullptr;
   }
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief adjust size of string or binary
 * @param uMemorySize new size of string or binary
 */
void variant_view::adjust(unsigned uMemorySize)
{
   if( is_string() == true || is_binary() == true )
   {
      switch( type_number() )
      {
      case eTypeNumberGuid: 
      case eTypeNumberBinary: 
         m_uSize = uMemorySize;
         break;
      case eTypeNumberString:
      case eTypeNumberUtf8String:
         m_uSize = uMemorySize - 1;                                            // decrease zero terminator
         break;
      case eTypeNumberWString:                                                                     assert( uMemorySize % 2 == 0 );
         m_uSize = ( uMemorySize >> 1 ) - 1;                                   // decrease zero terminator
         break;
      case eTypeNumberUtf32String:                                                                 assert( uMemorySize % 4 == 0 );
         m_uSize = ( uMemorySize >> 2 );
         break;
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief convert internal value to new type
 * @param eType new type to convert to
*/
void variant_view::convert( variant_type::enumType eType )
{
   if( (unsigned int)eType & variant_type::eGroupInteger )
   {
      if( (m_uType & variant_type::eGroupInteger) & variant_type::eGroupInteger ) return;
      auto _value = get_int64();
      switch( eType )
      {
      case eTypeInt8   : *this = (int8_t)_value; break;
      case eTypeInt16  : *this = (int16_t)_value; break;
      case eTypeInt32  : *this = (int32_t)_value; break;
      case eTypeInt64  : *this = (int64_t)_value; break;
      case eTypeUInt8  : *this = (uint8_t)_value; break;
      case eTypeUInt16 : *this = (uint16_t)_value; break;
      case eTypeUInt32 : *this = (uint32_t)_value; break;
      case eTypeUInt64 : *this = (uint64_t)_value; break;
      }
   }
   else if( (unsigned int)eType & variant_type::eGroupDecimal )
   {
      if( (m_uType & variant_type::eGroupDecimal) & variant_type::eGroupDecimal ) return;
      auto _value = get_decimal();
      switch( eType )
      {
      case eTypeCFloat : *this = (float)_value; break;
      case eTypeCDouble : *this = (double)_value; break;
      }
   }
   else if( (unsigned int)eType & variant_type::eGroupString )
   {                                                                             assert(false);
   /*
      if( (m_uType & variant_type::eGroupString) == variant_type::eGroupString ) return;
      auto _value = get_wstring();
      switch( eType )
      {
      case eTypeUtf8String : { auto _s = _value.get_utf8string(); this->_set_value( utf8( _s.c_str(), _s.length() ) ); } break;
      case eTypeString : { auto _s = _value.get_string(); this->_set_value( _s.c_str(), _s.length() ); } break;
      case eTypeWString : this->_set_value( _value.c_str(), _value.length() ); break;
      }
      */
   }
   else if( (unsigned int)eType & variant_type::eGroupBoolean )
   {
      if( (m_uType & variant_type::eGroupBoolean) & variant_type::eGroupBoolean ) return;
      *this = get_bool();
   }
   else                                                                          assert( false );
}

/** ---------------------------------------------------------------------------
 * @brief convert take internal value and into new variant
 * @param uType new type to convert to
 * @param variantTo variant getting new value
 * @return true if ok, false if not
*/
bool variant_view::convert_to( unsigned uType, variant& variantTo ) const
{
   return variant::convert_to_s( (variant*)this, &variantTo, uType );
}

/** --------------------------------------------------------------------------- * @brief get address to value
 * @return pointer to value as uint8_t*
*/
const uint8_t* variant_view::data() const noexcept
{
   switch( type_number() )
   {
   case eTypeNumberUnknown: return nullptr;
   case eTypeNumberBool: return (const uint8_t*)&m_V.b;
   case eTypeNumberInt8: return (const uint8_t*)&m_V.int8;
   case eTypeNumberInt16: return (const uint8_t*)&m_V.int16;
   case eTypeNumberInt32: return (const uint8_t*)&m_V.int32;
   case eTypeNumberInt64: return (const uint8_t*)&m_V.int64;
   case eTypeNumberUInt8: return (const uint8_t*)&m_V.uint8;
   case eTypeNumberUInt16: return (const uint8_t*)&m_V.uint16;
   case eTypeNumberUInt32: return (const uint8_t*)&m_V.uint32;
   case eTypeNumberUInt64: return (const uint8_t*)&m_V.uint64;
   case eTypeNumberFloat: return (const uint8_t*)&m_V.f; 
   case eTypeNumberDouble: return (const uint8_t*)&m_V.d; 
   case eTypeNumberPointer: return (const uint8_t*)&m_V.p; 
   case eTypeNumberGuid: 
   case eTypeNumberString:
   case eTypeNumberUtf8String:
   case eTypeNumberWString: 
   case eTypeNumberUtf32String:
   case eTypeNumberBinary: return (const uint8_t*)m_V.pb_const;
   case eTypeNumberBit: return (const uint8_t*)&m_V.int8;
   default:                                                                      assert(false);

   }

   return nullptr;
}


bool variant_view::compare( const variant_view& v ) const
{
   if( v.type_number() != type_number() ) return false;

   switch( type_number() )
   {
   case eTypeNumberUnknown: return true;
   case eTypeNumberBool: return m_V.b == v.m_V.b;
   case eTypeNumberInt8: return m_V.int8 == v.m_V.int8;
   case eTypeNumberInt16: return m_V.int16 == v.m_V.int16;
   case eTypeNumberInt32: return m_V.int32 == v.m_V.int32;
   case eTypeNumberInt64: return m_V.int64 == v.m_V.int64;
   case eTypeNumberUInt8: return m_V.uint8 == v.m_V.uint8;
   case eTypeNumberUInt16: return m_V.uint16 == v.m_V.uint16;
   case eTypeNumberUInt32: return m_V.uint32 == v.m_V.uint32;
   case eTypeNumberUInt64: return m_V.uint64 == v.m_V.uint64;
   case eTypeNumberFloat: return m_V.f == v.m_V.f; 
   case eTypeNumberDouble: return m_V.d == v.m_V.d; 
   case eTypeNumberPointer: return m_V.p == v.m_V.p; 
   case eTypeNumberGuid: 
      return (memcmp( m_V.pbsz, v.m_V.pbsz, 16 ) == 0);
   case eTypeNumberString:
   case eTypeNumberUtf8String:
      return (m_uSize == v.m_uSize && memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize ) == 0);
   case eTypeNumberWString: 
      return (m_uSize == v.m_uSize && memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize * sizeof( wchar_t ) ) == 0);
   case eTypeNumberUtf32String:
      return (m_uSize == v.m_uSize && memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize * sizeof( int32_t ) ) == 0);
   case eTypeNumberBinary:
      return (m_uSize == v.m_uSize && memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize ) == 0);
   case eTypeNumberBit: return m_V.int8 == v.m_V.int8;
   default:                                                                      assert(false);

   }

   return false;
}

/** ---------------------------------------------------------------------------
 * @brief compares if this is less than passed value
 * @param v value to check if this is less than
 * @return true if this is less than passed value
*/
bool variant_view::less( const variant_view& v ) const
{
   if( v.type_number() != type_number() ) return false;

   switch( type_number() )
   {
   case eTypeNumberUnknown: return true;
   case eTypeNumberBool: return m_V.b < v.m_V.b;
   case eTypeNumberInt8: return m_V.int8 < v.m_V.int8;
   case eTypeNumberInt16: return m_V.int16 < v.m_V.int16;
   case eTypeNumberInt32: return m_V.int32 < v.m_V.int32;
   case eTypeNumberInt64: return m_V.int64 < v.m_V.int64;
   case eTypeNumberUInt8: return m_V.uint8 < v.m_V.uint8;
   case eTypeNumberUInt16: return m_V.uint16 < v.m_V.uint16;
   case eTypeNumberUInt32: return m_V.uint32 < v.m_V.uint32;
   case eTypeNumberUInt64: return m_V.uint64 < v.m_V.uint64;
   case eTypeNumberFloat: return m_V.f < v.m_V.f; 
   case eTypeNumberDouble: return m_V.d < v.m_V.d; 
   case eTypeNumberPointer: return m_V.p < v.m_V.p; 
   case eTypeNumberGuid: 
      return (memcmp( m_V.pbsz, v.m_V.pbsz, 16 ) < 0);
   case eTypeNumberString:
   case eTypeNumberUtf8String: {
#ifndef NDEBUG
      auto p1_d = m_V.pbsz;
      auto p2_d = v.m_V.pbsz;
#endif // !NDEBUG
      auto iCompare = memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize );
      if( iCompare != 0 ) return iCompare < 0;
      return m_uSize < v.m_uSize;
   }
   case eTypeNumberWString: 
      return memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize * sizeof( wchar_t ) ) < 0;
   case eTypeNumberUtf32String:
      return memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize * sizeof( int32_t ) ) < 0;
   case eTypeNumberBinary:
      return memcmp( m_V.pbsz, v.m_V.pbsz, m_uSize ) < 0;
   case eTypeNumberBit: return m_V.int8 < v.m_V.int8;
   default:                                                                      assert(false);

   }

   return false;
}

/*
      auto iCompare = memcmp( m_V.pbsz, v.m_V.pbsz, 16 );
      return iCompare < 0;


*/

uint32_t variant_view::length_in_bytes() const
{
   switch( type_number() )
   {
   case eTypeNumberUnknown: return 0;
   case eTypeNumberBool: return 1;
   //case eTypeNumberInt8: return std::to_wstring(m_V.int8);
   //case eTypeNumberInt16: return std::to_wstring(m_V.int16);
   //case eTypeNumberInt32: return std::to_wstring(m_V.int32);
   //case eTypeNumberInt64: return std::to_wstring(m_V.int64);
   //case eTypeNumberUInt8: return std::to_wstring(m_V.uint8);
   //case eTypeNumberUInt16: return std::to_wstring(m_V.uint16);
   //case eTypeNumberUInt32: return std::to_wstring(m_V.uint32);
   //case eTypeNumberUInt64: return std::to_wstring(m_V.uint64);
   //case eTypeNumberFloat: return std::to_wstring(m_V.f); 
   //case eTypeNumberGuid: return 16;
   // case eTypeNumberDouble: return std::to_wstring(m_V.d); 
   case eTypeNumberString: return m_uSize;
   case eTypeNumberUtf8String: return m_uSize;
   case eTypeNumberWString: return m_uSize * 2;
   //case eTypeNumberJson: return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ).get_wstring();
   //case eTypeNumberXml: return gd_std::wstring( gd_std::string::utf8( m_V.pbsz ), m_uSize ).get_wstring();
   case eTypeNumberBit: return 1;
   default:                                                                      assert(false);

   }

    return 0;
}

/** ---------------------------------------------------------------------------
 * @brief copy value in `variant_view` into variant
 * @param variantviewFrom value to copy from
 * @param variantTo copy to this variant
*/
void variant_view::copy_s( const variant_view& variantviewFrom, variant& variantTo )
{
   variantTo.clear();
   
   if( variantviewFrom.type_number() <= eTypeTipRegister )
   {
      memcpy( static_cast<void*>(&variantTo), &variantviewFrom, sizeof(variant));
   }
   else
   {
      switch( variantviewFrom.type_number() )
      {
      case eTypeNumberGuid:
         variantTo.assign( (const uint8_t*)variantviewFrom.get_void(), variant_type::eTypeGuid, variantviewFrom.length(), variant_type::tag_raw{});
         break;
      case eTypeNumberString:
         variantTo.assign( (const char*)variantviewFrom, variantviewFrom.length() );
         break;
      case eTypeNumberUtf8String:
         variantTo.assign( (const char8_t*)variantviewFrom, variantviewFrom.length() );
         break;
      case eTypeNumberWString:
         variantTo.assign( (const wchar_t*)variantviewFrom, variantviewFrom.length() );
         break;
      case eTypeNumberBinary:
         variantTo.assign( (const uint8_t*)variantviewFrom.get_void(), variant_type::eTypeBinary, variantviewFrom.length(), variant_type::tag_raw{});
         break;
      default:                                                                                     assert(false);

      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief copy variant to variant_view
 * @param variantFrom variant that is copied into variant_view
 * @param variantviewTo variant_view that get data from variant
*/
void variant_view::copy_s( const variant& variantFrom, variant_view& variantviewTo )
{
   memcpy(static_cast<void*>(&variantviewTo), &variantFrom, sizeof(variant) );
}

/// variant values in list are converted to string and concatenated with each other and returned
std::string variant_view::format_s(const std::initializer_list<variant_view>& listValue)
{
   std::string string_;
   format_s( listValue, string_ );
   return string_;
}

/// variant_view values in list are converted to string and added to string sent as argument
void variant_view::format_s(const std::initializer_list<variant_view>& listValue, std::string& stringFormat)
{
   for(auto it : listValue)
   {
      stringFormat += it.as_string();
   }
}

variant_view variant_view::parse_to_primitive_s( const std::string_view& stringValue ) 
{
   variant_view v_;

   return v_;
}

namespace debug {
   /** ------------------------------------------------------------------------
    * @brief print data in variant_view to view what it contains, usefull in debug
    * @param argumentsToPrint value to print
    * @return string value as string
   */
   std::string print( const variant_view& v )
   {
      std::string stringPrint; 
      stringPrint = gd::variant::get_type_name_s( v.type() );
      stringPrint += ": ";
      stringPrint += v.as_string();

      return stringPrint;
   }

   /** ------------------------------------------------------------------------
    * @brief print value, usefull in debug
    * @param argumentsToPrint value to print
    * @return string value as string
   */
   std::string print_value( const variant_view& v )
   {
      std::string stringPrint; 
      stringPrint += v.as_string();

      return stringPrint;
   }

   /** ------------------------------------------------------------------------
    * @brief print vector values
    * @param argumentsToPrint value to print
    * @return string value as string
   */
   std::string print( const std::vector<variant_view>& v_ )
   {
      std::string stringPrint; 
      for( const auto& it : v_ )
      {
         if( stringPrint.empty() == false ) stringPrint.append(std::string_view{ ", " });
         stringPrint += gd::variant::get_type_name_s( it.type() );
         stringPrint += ": ";
         stringPrint += it.as_string();
      }

      return stringPrint;
   }

   /** ------------------------------------------------------------------------
    * @brief print vector values
    * @param argumentsToPrint value to print
    * @return string value as string
   */
   std::string print( const std::vector<variant_view>& v_, std::function< std::string( const variant_view& ) > callback_ )
   {
      std::string stringPrint; 
      for( const auto& it : v_ )
      {
         if( stringPrint.empty() == false ) stringPrint.append(std::string_view{ ", " });
         stringPrint += gd::variant::get_type_name_s( it.type() );
         stringPrint += ": ";
         stringPrint += callback_( it );
      }

      return stringPrint;
   }

}

} /* namespace gd */

