#include <stdlib.h>

#include "gd_utf8.h"
#include "gd_parse.h"

#include "gd_sql_value.h"


#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wswitch"
   #pragma clang diagnostic ignored "-Wformat"
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wswitch"
   #pragma GCC diagnostic ignored "-Wformat"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996 6001 6255 )
#endif



_GD_SQL_QUERY_BEGIN

char iBeginBrace_g   = '{';
char iEndBrace_g     = '}';
char iQuestion_g     = '?';
char iSemicolon_g    = ';';

const int iSprintfBufferSize_g = 32;

/** ---------------------------------------------------------------------------
 * @brief Append ascii text to string as utf8
 * @param pbszAscii ascii text appended to string as utf8
 * @param stringSql string that gets ascii text formated as utf8
*/
void append_ascii( const uint8_t* pbszAscii, std::string& stringSql )
{
   uint8_t pbUtf8[2];

   while( *pbszAscii )
   {
      if( *pbszAscii < 0x80 )
      {
         if( *pbszAscii != '\'' ) stringSql += (char)*pbszAscii;
         else                     stringSql += std::string_view{"''"};
      }
      else
      {
          gd::utf8::convert( *pbszAscii, pbUtf8 );
          stringSql.append( (const char*)pbUtf8, 2 );
      }
      pbszAscii++;
   }
}

/** ---------------------------------------------------------------------------
 * @brief Append ascii text to string as utf8
 * @param pbszAscii ascii text appended to string as utf8
 * @param uLength ascii text length
 * @param stringSql string that gets ascii text formated as utf8
*/
void append_ascii( const uint8_t* pbszAscii, size_t uLength, std::string& stringSql )
{
   uint8_t pbUtf8[2];

   decltype( pbszAscii ) puEnd = pbszAscii + uLength;

   while( pbszAscii < puEnd )
   {
      if( *pbszAscii < 0x80 )
      {
         if( *pbszAscii != '\'' ) stringSql += (char)*pbszAscii;
         else                     stringSql += std::string_view{"''"};
      }
      else
      {
          gd::utf8::convert( *pbszAscii, pbUtf8 );
          stringSql.append( (const char*)pbUtf8, 2 );
      }
      pbszAscii++;
   }
}


/** ---------------------------------------------------------------------------
 * @brief Append utf8 (that is the default) text to string object
 * @param pbszUft8 utf8 formated text appended to text
 * @param stringSql utf8 text is appended to string
*/
void append_utf8( const uint8_t* pbszUft8, std::string& stringSql )
{
   while( *pbszUft8 )
   {
      if( *pbszUft8 != '\'' ) stringSql += (char)*pbszUft8;
      else                    stringSql += std::string_view{"''"};

      pbszUft8++;
   }
}

static const uint8_t binary_pszHEX_s[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
                                  

//inline void append_binary( uint8_t* puPosition, uint8_t* puBytes, unsigned uLength )
inline void append_binary( const uint8_t* puBinary, unsigned uLength, std::string& stringSql )
{
   const unsigned STACK_SIZE = 1024;

   uint8_t* puBuffer; // temporary storage for hexadecimal binary text
   const uint8_t* puBytes = puBinary + uLength;// set to end
   const uint8_t* puBytesStop = puBinary; 

   unsigned uSize = uLength * 2;                                               // two characters for each byte !!!

   if( uLength > STACK_SIZE ) puBuffer = new uint8_t[uSize];
   else                       puBuffer = (uint8_t*)alloca( uSize );

   uint8_t* puPosition = puBuffer + uSize;                                     // set to buffer end

   // optimize, align for 4 byte sections
   for( unsigned int u = uLength % 4; u > 0; u--  )
   {
      puBytes--;
      puPosition--;
      *puPosition = binary_pszHEX_s[*puBytes & 0x0F];
      puPosition--;
      *puPosition = binary_pszHEX_s[(*puBytes & 0xF0) >> 4];
   }

   // ## copy four bytes for each loop iteration to speed up and simplify for
   //    compiler and cpu to do it's optimization.
   while( puBytes != puBytesStop )
   {
      puBytes--;
      puPosition--;
      *puPosition = binary_pszHEX_s[*puBytes & 0x0F];
      puPosition--;
      *puPosition = binary_pszHEX_s[(*puBytes & 0xF0) >> 4];

      puBytes--;
      puPosition--;
      *puPosition = binary_pszHEX_s[*puBytes & 0x0F];
      puPosition--;
      *puPosition = binary_pszHEX_s[(*puBytes & 0xF0) >> 4];

      puBytes--;
      puPosition--;
      *puPosition = binary_pszHEX_s[*puBytes & 0x0F];
      puPosition--;
      *puPosition = binary_pszHEX_s[(*puBytes & 0xF0) >> 4];

      puBytes--;
      puPosition--;
      *puPosition = binary_pszHEX_s[*puBytes & 0x0F];
      puPosition--;
      *puPosition = binary_pszHEX_s[(*puBytes & 0xF0) >> 4];
   }

   stringSql.append( (const char*)puBuffer, uSize );

   if( uLength > STACK_SIZE ) delete [] puBuffer;                              // delete heap memory if more than allowed stack size
}


/** ---------------------------------------------------------------------------
 * @brief Append variant value to string in a format that works for sql
 * @param variantValue value added to string as string
 * @param stringSql string value is appended to with sql rules
*/
void append_g( const gd::variant& variantValue, std::string& stringSql )
{
   using namespace gd::variant_type;

   size_t uSize = 0; // if value is added from buffer then size has number of bytes to copy
   //char pbszBuffer[sizeof "-9223372036854775808_"]; // buffer used to convert numbers
   char pbszBuffer[iSprintfBufferSize_g];

   const gd::variant::value& value = variantValue.m_V; // get union from variant value

   auto uType = variantValue.type_number();
   switch( uType )
   {
   case eTypeNumberUnknown: 
      memcpy( pbszBuffer, "NULL", sizeof "NULL");
      uSize = 4;
      break;
   case eTypeNumberBit:
   case eTypeNumberBool:
      *pbszBuffer = value.b != 0 ? '1' : '0';
      uSize = 1;
      break;
   case eTypeNumberInt8: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int8, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt8: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.uint8, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt16: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int16, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt16: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.uint16, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt32: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int32, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt32: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.int32, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt64: 
   {
      auto p_ = gd::utf8::itoa( (int64_t)value.int64, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt64: 
   {
      auto p_ = gd::utf8::utoa( (uint64_t)value.uint64, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;
   }
   break;
   case eTypeNumberFloat: 
   {
      uSize = ::sprintf( pbszBuffer, "%f", value.f );
   }
   break;
   case eTypeNumberDouble: 
   {
      uSize = ::sprintf( pbszBuffer, "%.17g", value.d );
   }
   break;
   case eTypeNumberGuid: 
   {
      append_binary( (uint8_t*)value.pb, 16, stringSql );
   }
   break;
   case eTypeNumberString: 
   {
      stringSql += '\'';
      append_ascii( (uint8_t*)value.pb, variantValue.length(), stringSql );
      stringSql += '\'';
   }
   break;
   case eTypeNumberUtf8String: 
   {
      stringSql += '\'';
      append_utf8( (uint8_t*)value.pb, stringSql );
      stringSql += '\'';
   }
   break;
   case eTypeNumberBinary: 
   {
      append_binary( (uint8_t*)value.pb, variantValue.length(), stringSql );
   }
   break;

      
/*
   case eTypeNumberWString     : return *m_V.pwsz != L'\0' ? true : false;
*/
   default:                                                                      assert( false );
   }

   if( uSize != 0 ) { stringSql.append( pbszBuffer, uSize ); };

}


/** ---------------------------------------------------------------------------
 * @brief Append variant_view value to string in a format that works for sql
 * @param variantValue value added to string as string
 * @param stringSql string value is appended to with sql rules
*/
void append_g( const gd::variant_view& variantValue, std::string& stringSql )
{
   using namespace gd::variant_type;

   size_t uSize = 0; // if value is added from buffer then size has number of bytes to copy
   //char pbszBuffer[sizeof "-9223372036854775808_"]; // buffer used to convert numbers
   char pbszBuffer[iSprintfBufferSize_g];

   const gd::variant_view::value& value = variantValue.m_V; // get union from variant value

   auto uType = variantValue.type_number();
   switch( uType )
   {
   case eTypeNumberUnknown: 
      memcpy( pbszBuffer, "NULL", sizeof "NULL");
      uSize = 4;
      break;
   case eTypeNumberBit:
   case eTypeNumberBool:
      *pbszBuffer = value.b != 0 ? '1' : '0';
      uSize = 1;
      break;
   case eTypeNumberInt8: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int8, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt8: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.uint8, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt16: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int16, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt16: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.uint16, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt32: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int32, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt32: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.int32, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt64: 
   {
      auto p_ = gd::utf8::itoa( (int64_t)value.int64, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt64: 
   {
      auto p_ = gd::utf8::utoa( (uint64_t)value.uint64, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;
   }
   break;
   case eTypeNumberFloat: 
   {
      uSize = ::sprintf( pbszBuffer, "%f", value.f );
   }
   break;
   case eTypeNumberDouble: 
   {
      uSize = ::sprintf( pbszBuffer, "%.17g", value.d );
   }
   break;
   case eTypeNumberGuid: 
   {
      append_binary( (uint8_t*)value.pb, 16, stringSql );
   }
   break;
   case eTypeNumberString: 
   {
      stringSql += '\'';
      append_ascii( (uint8_t*)value.pb, variantValue.length(), stringSql );
      stringSql += '\'';
   }
   break;
   case eTypeNumberUtf8String: 
   {
      stringSql += '\'';
      append_utf8( (uint8_t*)value.pb, stringSql );
      stringSql += '\'';
   }
   break;
   case eTypeNumberBinary: 
   {
      append_binary( (uint8_t*)value.pb, variantValue.length(), stringSql );
   }
   break;
      
/*
   case eTypeNumberWString     : return *m_V.pwsz != L'\0' ? true : false;
*/
   default:                                                                    assert( false );
   }

   if( uSize != 0 ) { stringSql.append( pbszBuffer, uSize ); };

}

/** ---------------------------------------------------------------------------
 * @brief Append variant_view value to string in a format that works for sql
 * @param variantValue value added to string as string
 * @param stringSql string value is appended to with sql rules
*/
void append_g( const gd::variant_view& variantValue, std::string& stringSql, tag_raw )
{
   using namespace gd::variant_type;

   size_t uSize = 0; // if value is added from buffer then size has number of bytes to copy
   //char pbszBuffer[sizeof "-9223372036854775808_"]; // buffer used to convert numbers
   char pbszBuffer[iSprintfBufferSize_g];

   const gd::variant_view::value& value = variantValue.m_V; // get union from variant value

   auto uType = variantValue.type_number();
   switch( uType )
   {
   case eTypeNumberUnknown: 
      memcpy( pbszBuffer, "NULL", sizeof "NULL");
      uSize = 4;
      break;
   case eTypeNumberBit:
   case eTypeNumberBool:
      *pbszBuffer = value.b != 0 ? '1' : '0';
      uSize = 1;
      break;
   case eTypeNumberInt8: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int8, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt8: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.uint8, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt16: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int16, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt16: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.uint16, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt32: 
   {
      auto p_ = gd::utf8::itoa( (int32_t)value.int32, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt32: 
   {
      auto p_ = gd::utf8::utoa( (uint32_t)value.int32, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberInt64: 
   {
      auto p_ = gd::utf8::itoa( (int64_t)value.int64, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;                                       // calculate filled buffer size
   }
   break;
   case eTypeNumberUInt64: 
   {
      auto p_ = gd::utf8::utoa( (uint64_t)value.uint64, (uint8_t*)pbszBuffer );
      uSize = p_ - (uint8_t*)pbszBuffer;
   }
   break;
   case eTypeNumberFloat: 
   {
      uSize = ::sprintf( pbszBuffer, "%f", value.f );
   }
   break;
   case eTypeNumberDouble: 
   {
      uSize = ::sprintf( pbszBuffer, "%.17g", value.d );
   }
   break;
   case eTypeNumberGuid: 
   {
      append_binary( (uint8_t*)value.pb, 16, stringSql );
   }
   break;
   case eTypeNumberString: 
   {
      append_ascii( (uint8_t*)value.pb, variantValue.length(), stringSql );
   }
   break;
   case eTypeNumberUtf8String: 
   {
      append_utf8( (uint8_t*)value.pb, stringSql );
   }
   break;
   case eTypeNumberBinary: 
   {
      append_binary( (uint8_t*)value.pb, variantValue.length(), stringSql );
   }
   break;
      
/*
   case eTypeNumberWString     : return *m_V.pwsz != L'\0' ? true : false;
*/
   default:                                                                    assert( false );
   }

   if( uSize != 0 ) { stringSql.append( pbszBuffer, uSize ); };

}




/** ---------------------------------------------------------------------------
 * @brief Prepare two sql queries used for doing some sort of database bulk operation
 * @param stringFixed fixed string part that is only added once
 * @param stringParameter parameter part is the part that is copied bulk count times
 * @param uCount total number of rows to execute
 * @param uBulkCount max number of rows in each bulk command
 * @return std::tuple<uint64_t, std::string, std::string> tuple with number of full bulk strings needed
           to bulk all data and then the full bulk string, also rest bulk to execute tail rows
*/
std::tuple<uint64_t, std::string, std::string> make_bulk_g( const std::string_view& stringFixed, const std::string_view& stringParameter, uint64_t uCount, uint64_t uBulkCount )
{
   // ## local method used to append repeated string count number of times.
   auto repeat_ = []( const std::string_view& stringAppend, uint64_t uCount, std::string& stringSql ) -> void {  assert( uCount > 0 );
      stringSql.reserve( stringSql.length() + uCount * stringAppend.length() );// allocate needed space in string to avoid relocation
      stringSql += stringAppend;                                               // add first row
      if( uCount == 1 ) return;                                                // only one row, no need to add more with comma separation

      // ## append rest, generate string with comma to avoid multiple copy
      std::string stringAppendWitComma{ ',' };
      stringAppendWitComma += stringAppend;
      while( --uCount > 0 ) { stringSql += stringAppendWitComma; }
   };

   std::string stringSqlFull;
   std::string strinSqlRest;

   uint64_t uSectionCount = uCount / uBulkCount;                               // Number of bulk sections with max bulk count
   uint64_t uTailCount = uCount % uBulkCount;                                  // Tail rows
   uint64_t uActiveRow = 0;                                                    // Row index for row value in table that is processed

   if( uSectionCount > 0 )
   {
      stringSqlFull = stringFixed;                                             // fixed part
      repeat_( stringParameter, uBulkCount, stringSqlFull );                   // build sql to bulk max amount of rows
   }

   if( uTailCount > 0 )
   {
      strinSqlRest = stringFixed;                                              // fixed part
      repeat_( stringParameter, uTailCount, strinSqlRest );                    // build sql to bulk tail amount of rows
   }

   return { uSectionCount, std::move(stringSqlFull), std::move(strinSqlRest) };
}

namespace {
}

/** ---------------------------------------------------------------------------
 * @brief Replaces placeholders in a source string with values from an argument list, producing a new string.
 *
 * This function processes a source string, replacing placeholders (denoted by curly braces `{}`) with corresponding
 * values from the provided `argumentsValue`. Placeholders can reference arguments by index (e.g., `{0}`) or name
 * (e.g., `{name}`). Special syntax like `{*name}` indicates a required value, and `{=name}` indicates raw value
 * insertion without SQL-specific escaping. The resulting string is built in `stringNew`. The function handles
 * quoted strings and skips them appropriately.
 *
 * @param stringSource The input string containing placeholders to be replaced (as a string_view).
 * @param argumentsValue The collection of arguments providing values for placeholder substitution.
 * @param[out] stringNew The output string where the result is constructed.
 * @param tag_brace A tag indicating this function processes brace-style placeholders (implementation-specific).
 *
 * @return A pair containing:
 *         - `bool`: `true` if the replacement succeeded, `false` if an error occurred (e.g., missing required value or unclosed quote).
 *         - `std::string`: An error message if the operation failed; empty if successful.
 *
 * @note The function assumes the input string is well-formed except where errors are explicitly checked (e.g., unclosed quotes).
 * @note If a placeholder starts with '=', the value is appended raw without SQL escaping.
 * @note The function modifies `stringNew` incrementally and clears it only implicitly via assignment.
 * 
 * @par Examples
 * @parblock
 * **Example 1: Basic indexed placeholder replacement**  
 * @code
 * gd::argument::arguments args({"Alice", 42});
 * std::string result;
 * auto [success, error] = replace_g("Hello {0}, age {1}", args, result, tag_brace{});
 * // success = true, result = "Hello Alice, age 42", error = ""
 * @endcode
 *
 * **Example 2: Named placeholders with SQL escaping**  
 * @code
 * gd::argument::arguments args({{"name", "O'Reilly"}, {"id", 123}});
 * std::string result;
 * auto [success, error] = replace_g("WHERE name = '{name}' AND id = {id}", args, result, tag_brace{});
 * // success = true, result = "WHERE name = 'O''Reilly' AND id = 123", error = ""
 * @endcode
 *
 * **Example 3: Required value with missing argument**  
 * @code
 * gd::argument::arguments args({{"id", 123}});
 * std::string result;
 * auto [success, error] = replace_g("WHERE name = {*name}", args, result, tag_brace{});
 * // success = false, result unchanged, error = "required value not found: name"
 * @endcode
 *
 * **Example 4: Raw value insertion**  
 * @code
 * gd::argument::arguments args({{"table", "users"}});
 * std::string result;
 * auto [success, error] = replace_g("SELECT * FROM {=table}", args, result, tag_brace{});
 * // success = true, result = "SELECT * FROM users", error = ""
 * @endcode
 *
 * **Example 5: Unclosed quote error**  
 * @code
 * gd::argument::arguments args;
 * std::string result;
 * auto [success, error] = replace_g("WHERE name = 'test", args, result, tag_brace{});
 * // success = false, result unchanged, error = "no quote ending: 'test"
 * @endcode
 * @endparblock
 */
std::pair<bool,std::string> replace_g(const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, std::string& stringNew, tag_brace)
{
   using namespace gd::types;

   unsigned uArgumentIndex = 0;
   std::string stringName;       // current variable name that is replaced

   for(auto it = std::begin( stringSource ), itEnd = std::end( stringSource ); it != itEnd; it++ )
   {
      if(*it != '{')
      {
         if(*it != '\'' ) stringNew += *it;                                    // no quote then copy character
         else
         {                                                                     // string is found, when in string we need to copy until end of string is found
            const char* pbszFind = &(*it) + 1; 
            pbszFind = gd::parse::strchr( pbszFind, '\'', gd::parse::sql{} );  // method used to find last quote, this method knows how to skip double quoutes
            if(pbszFind != nullptr && pbszFind <= &(*(itEnd - 1)))
            {
               auto uSize = (pbszFind - &(*it));
               stringNew.append( &(*it), uSize + 1);                           // append text including first quote (note + 1)
               it += uSize;                                                    // move iterator to end of string
            }
            else
            {
               auto uSizeToEnd = std::distance(it, itEnd);                     // get distance to end of string
               if( uSizeToEnd > 20 ) uSizeToEnd = 20;                          // limit to 20 characters
               const auto* pbszPosition = &( *it );
               return { false, std::string("no quote ending: ") + std::string( pbszPosition, uSizeToEnd) }; // end of string not found, return text and the first 20 characters
            }
         }
      }
      else
      {
         bool bRequired = false;                                               // mark if value is required
         it++;
         if( *it == '*' ) { bRequired = true; it++; }                          // check if value is required

         // ## copy name of variable
         stringName.clear();
         while(*it != '}' && it != itEnd)
         {
            stringName += *it;
            it++;
         }

         if(*it == '}')
         {
            bool bRaw = false;
            gd::variant_view v_;

            if(stringName.empty() == false)
            {
               char chFirst = stringName.at( 0 );                              // get first character
               if(chFirst == '=')
               {
                  bRaw = true;
                  stringName.erase( stringName.begin() );                      // remove equal charact
               }
            }

            if(stringName.empty() == true)
            {
               v_ = argumentsValue[uArgumentIndex].as_variant_view();
               uArgumentIndex++;
            }
            else
            {
               // ## investigate type of name
               char chFirst = stringName.at( 0 );                              // get first character


               if( is_ctype_g( chFirst, "digit"_ctype ) == true)               // is value a number
               {
                  unsigned uIndex = std::stoul( stringName );                                   assert( uIndex < 0xffff ); //realistic ??
                  v_ = argumentsValue[uIndex].as_variant_view();
               }
               else
               {
                  v_ = argumentsValue[stringName].as_variant_view();
               }
            }

            // ## check if value is required and if value is found return error
            if( bRequired == true && v_.is_null() == true )
            {
               return { false, std::string("required value not found: ") + stringName };
            }

            if( bRaw == false ) append_g( v_, stringNew );                     // add value to work in sql
            else                append_g( v_, stringNew, gd::sql::tag_raw{});  // add value to string without fix for quotes if needed for value
         }// if(*it == '}') {
      }
   }// for(auto it = std::begin( stringSource ...

   return { true, std::string{} };
}

/** --------------------------------------------------------------------------- format `replace_g`
 * @brief replace value with sql formating, if argument to replace is not found then old replacement identifier is kept
 * @code
std::string stringTemplate = "...{=one}...{=one1}...{=one}...{=one}..";
auto string1 = gd::sql::replace_g( stringTemplate, {{"one1","111"}}, gd::sql::tag_brace{}, gd::sql::tag_keep_not_found{} );
REQUIRE( string1 == "...{=one}...111...{=one}...{=one}.." );
 * @endcode
 * @param stringSource string with values to replace
 * @param argumentsValue arguments that holds values to replace with
 * @return  std::string string with replaced values
 */
std::string replace_g(const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, tag_brace, tag_keep_not_found )
{
   using namespace gd::types;

   unsigned uArgumentIndex = 0;
   std::string stringName;       // current variable name that is replaced
   std::string stringNew;        // new created string

   for(auto it = std::begin( stringSource ), itEnd = std::end( stringSource ); it != itEnd; it++ )
   {
      if(*it != '{')
      {
         if(*it != '\'' ) stringNew += *it;                                    // no quote then copy character
         else
         {                                                                     // string is found, when in string we need to copy until end of string is found
            const char* pbszFind = &(*it) + 1; 
            pbszFind = gd::parse::strchr( pbszFind, '\'', gd::parse::sql{} );  // method used to find last quote, this method knows how to skip double quoutes
            if(pbszFind != nullptr && pbszFind <= &(*(itEnd - 1)))
            {
               auto uSize = (pbszFind - &(*it));
               stringNew.append( &(*it), uSize + 1);                           // append text including first quote (note + 1)
               it += uSize;
            }
            else
            {
               return stringNew;                                               // end of string not found, return text
            }
         }
      }
      else
      {
         const char* pbszFromKeep = &(*it);                                    // keep start position if name for template argument isnt found and therefore kept
         stringName.clear();
         it++;
         while(*it != '}' && it != itEnd)
         {
            stringName += *it;
            it++;
         }

         // store old part if no replace value is found for name, then this should be kept
         std::string_view stringKeepOld( pbszFromKeep, stringName.length() + 2 ); // 2 = sizeof("{}") - zero terminator

         if(*it == '}')
         {
            bool bRaw = false;
            gd::variant_view v_;

            if(stringName.empty() == false)
            {
               char chFirst = stringName.at( 0 );                              // get first character
               if(chFirst == '=')
               {
                  bRaw = true;
                  stringName.erase( stringName.begin() );                      // remove equal charact
               }
            }

            if(stringName.empty() == true)
            {
               v_ = argumentsValue[uArgumentIndex].as_variant_view();
               uArgumentIndex++;
            }
            else
            {
               // ## investigate type of name
               char chFirst = stringName.at( 0 );                              // get first character


               if( is_ctype_g( chFirst, "digit"_ctype ) == true)               // is value a number
               {
                  unsigned uIndex = std::stoul( stringName );                                   assert( uIndex < 0xffff ); //realistic ??
                  v_ = argumentsValue[uIndex].as_variant_view();
               }
               else
               {
                  v_ = argumentsValue[stringName].as_variant_view();
               }
            }

            if( v_.is_null() == false )
            {
               if( bRaw == false ) append_g( v_, stringNew );                  // add value to work in sql
               else                append_g( v_, stringNew, gd::sql::tag_raw{});// add value to string without fix for quotes if needed for value
            }
            else
            {
               stringNew += stringKeepOld;
            }
         }// if(*it == '}') {
      }
   }// for(auto it = std::begin( stringSource ...

   return stringNew;
}


 /** --------------------------------------------------------------------------
 * @brief Processes a string with SQL-style conditional replacement expressions.
 * 
 * This function parses and replaces templated expressions in the input string with values determined by a provided callback.
 * The expressions follow the format `{?name,true_value,false_value}`, where:
 * - `name` is evaluated via the `find_` callback to determine a boolean condition.
 * - `true_value` is inserted if the condition is true.
 * - `false_value` (optional) is inserted if the condition is false.
 * 
 * The function handles SQL string literals (single-quoted text) by preserving them, including proper handling of escaped quotes.
 * Non-expression text is copied as-is, except for single quotes, which trigger string literal parsing.
 * 
 * @param stringSource The input string containing text and optional replacement expressions.
 * @param find_ A callback function that takes a `std::string_view` (the `name` in the expression) and returns a `gd::variant_view` 
 *              indicating the condition's truth value.
 * @param pbError Pointer to a boolean flag (optional). Set to `true` if a parsing error occurs; `false` otherwise. If `nullptr`, errors are silently ignored.
 * @param tag_preprocess .
 * @return A new `std::string` with all expressions replaced according to the `find_` callback results.
 * 
 * @example
 * Input: "SELECT * FROM users WHERE active = {?status,1,0}"
 * If find_("status") returns true: "SELECT * FROM users WHERE active = 1"
 * If find_("status") returns false: "SELECT * FROM users WHERE active = 0"
 */
std::string replace_g(const std::string_view& stringSource, std::function<gd::variant_view (const std::string_view&)> find_, bool* pbError, tag_preprocess )
{
   using namespace gd::types;

   unsigned uArgumentIndex = 0;
   std::string stringName;       // current variable name that is replaced
   std::string stringNew;        // new created string
   std::string_view stringExpression; // current variable name that is replaced

   if( pbError != nullptr ) *pbError = false;

   for( const char* pit = stringSource.data(), * pitEnd = stringSource.data() + stringSource.length(); pit < pitEnd; pit++ )
   //for(auto it = std::begin( stringSource ), itEnd = std::end( stringSource ); it != itEnd; it++ )
   {
      if(*pit != iBeginBrace_g || (*pit == iBeginBrace_g && *(pit + 1) != iQuestion_g) )
      {
         if(*pit != '\'' ) stringNew += *pit;                                  // no quote then copy character
         else
         {                                                                     // string is found, when in string we need to copy until end of string is found
            const char* pbszFind = pit + 1; 
            pbszFind = gd::parse::strchr( pbszFind, '\'', gd::parse::sql{} );  // method used to find last quote, this method knows how to skip double quoutes
            if( pbszFind != nullptr && pbszFind <= pitEnd )
            {
               auto uSize = (pbszFind - pit);
               stringNew.append( pit, uSize + 1);                              // append text including first quote (note + 1)
               pit += uSize;
            }
            else
            {
               return stringNew;                                               // end of string not found, return text
            }
         }
      }
      else
      {
         stringExpression = std::string_view();
         pit += 2;                                                             // move past "{?"

         const char* pbszBegin = pit;
         const char* pbszEnd = gd::parse::strchr( pbszBegin, stringSource.data() + stringSource.length(), '}', '{', gd::parse::tag_scope{});

         // ## parse first part to found out what to do
         const char* pbszSemicolon = gd::parse::strchr( pbszBegin, pbszEnd, iSemicolon_g );
         if(*pbszSemicolon != iSemicolon_g)
         {
            if( pbError != nullptr ) *pbError = true;
            // ERROR: TODO fix code for error
            return std::string();
         }

         // ## check name
         stringExpression = std::string_view( pbszBegin, pbszSemicolon - pbszBegin );
         //std::string_view string_( pbszBegin, pbszSemicolon - pbszBegin );
         bool bTrue = find_( stringExpression ).is_true();

         // ### extract rest
         pbszBegin = pbszSemicolon + 1;
         stringExpression = std::string_view( pbszBegin, pbszEnd );
         std::vector<std::string_view> vectorPart;
         gd::utf8::split( stringExpression, iSemicolon_g, vectorPart );

         if(vectorPart.empty() == false)
         {
            if(bTrue == true)
            {
               stringNew.append( vectorPart[0] );
            }
            else if(vectorPart.size() > 1)
            {
               stringNew.append( vectorPart[1] );
            }
         }

         pit = pbszEnd;
      }
   }// for(auto it = std::begin( stringSource ...

   return stringNew;
}



_GD_SQL_QUERY_END