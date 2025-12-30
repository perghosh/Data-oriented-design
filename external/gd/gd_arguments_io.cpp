// @FILE [tag: arguments, print, output] [description: Generate output from tables in different formats] [type: source] [name: gd_arguments_io.cpp]

#include <numeric>

#include "gd_parse.h"
#include "gd_sql_value.h"

#include "gd_utf8.h"

#include "gd_arguments_io.h"

_GD_ARGUMENT_BEGIN

void to_string( const arguments& arguments_, std::string& stringOut, tag_io_json )
{
   std::string stringResult;
   std::string stringEscaped;
   std::string_view stringValue;

   stringResult += "{";

   unsigned uIndex = 0;
   for( auto it = arguments_.named_begin(); it != arguments_.named_end(); ++it )
   {
      if( uIndex > 0 )
      {
         stringResult += ",";
      }
      std::string_view stringName = it->first;
      gd::variant_view value_ = it->second.as_variant_view();

      if( value_.is_string() == true )
      {
         stringResult += "\"" + std::string(stringName) + "\":\"";
         if( value_.is_char_string() == true )
         {
            stringValue = value_.as_string_view();
            if( gd::utf8::json::find_character_to_escape( stringValue ) != nullptr )
            {
               stringEscaped = gd::utf8::json::convert_utf8_to_json( stringValue );
               stringValue = stringEscaped;
            }
         }
         else
         {
            stringEscaped = value_.as_string();
            stringEscaped = gd::utf8::json::convert_utf8_to_json( stringValue );
            stringValue = stringEscaped;
         }
         stringResult += stringValue;
         stringResult += + "\"";
      }
      else if( value_.is_number() == true )
      {
         stringResult += "\"" + std::string(stringName) + "\": " + value_.as_string();
      }
      else if( value_.is_bool() == true )
      {
         stringResult += "\"" + std::string(stringName) + "\":" + ( (bool)value_ == true ? "true" : "false" );
      }
      else if( value_.is_null() == true )
      {
         stringResult += "\"" + std::string(stringName) + "\":null";
      }

      uIndex++;
   }

   stringResult += "}";

   if( stringOut.empty() == false ) { stringOut += stringResult; }
   else stringOut = std::move( stringResult );
}

_GD_ARGUMENT_END