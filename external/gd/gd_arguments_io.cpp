// @FILE [tag: arguments, print, output] [description: Generate output from tables in different formats] [type: source] [name: gd_arguments_io.cpp]

#include <numeric>

#include "gd_parse.h"
#include "gd_sql_value.h"

#include "gd_utf8.h"

#include "gd_arguments_io.h"

_GD_ARGUMENT_BEGIN

/// Generate json formated  string from arguments object ---------------------
void to_string( const arguments& arguments_, std::string& stringOut, tag_io_json )
{
   std::string stringResult;
   std::string stringEscaped;
   std::string_view stringValue;

   stringResult += "{";

   unsigned uIndex = 0;
   for( auto it = arguments_.named_begin(); it != arguments_.named_end(); ++it )
   {
      if( uIndex > 0 )  { stringResult += ","; }                              // Add , separator
      std::string_view stringName = it->first;
      gd::variant_view value_ = it->second.as_variant_view();

      if( value_.is_string() == true )
      {
         stringResult += "\"" + std::string(stringName) + "\":\"";
         if( value_.is_char_string() == true )
         {
            stringValue = value_.as_string_view();
            if( stringValue.empty() == false && gd::utf8::json::find_character_to_escape( stringValue ) != nullptr )
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
      else if( value_.is_binary() == true )
      {
         stringResult += "\"" + std::string(stringName) + "\": \"" + value_.as_string() + "\"";
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

/// Generate uri formated string from arguments object -----------------------
void to_string( const arguments& arguments_, std::string& stringOut, tag_io_uri )
{
   std::string stringResult;
   std::string stringEscaped;
   std::string_view stringValue;

   unsigned uIndex = 0;
   for( auto it = arguments_.named_begin(); it != arguments_.named_end(); ++it )
   {
      if( uIndex > 0 ) { stringResult += "&"; }                               // Add & separator
      
      std::string_view stringName = it->first;
      gd::variant_view value_ = it->second.as_variant_view();

      // Encode the name
      gd::utf8::uri::convert_utf8_to_uri( stringName, stringEscaped );
      stringResult += stringEscaped;
      stringResult += "=";

      // Handle the value based on type
      if( value_.is_string() == true )
      {
         if( value_.is_char_string() == true )
         {
            stringValue = value_.as_string_view();
            gd::utf8::uri::convert_utf8_to_uri( stringValue, stringEscaped );
         }
         else
         {
            stringEscaped = value_.as_string();
            gd::utf8::uri::convert_utf8_to_uri( stringEscaped, stringEscaped );
         }
         stringResult += stringEscaped;
      }
      else if( value_.is_number() == true ) { stringResult += value_.as_string();  }
      else if( value_.is_bool() == true ) { stringResult += ( (bool)value_ == true ? "true" : "false" ); }
      else if( value_.is_binary() == true ) {
         stringEscaped = value_.as_string();
         gd::utf8::uri::convert_utf8_to_uri( stringEscaped, stringEscaped );
         stringResult += stringEscaped;
      }
      else if( value_.is_null() == true )
      {
         // Null values are represented as empty strings in URI format
      }

      uIndex++;
   }

   if( stringOut.empty() == false ) { stringOut += stringResult; }
   else stringOut = std::move( stringResult );
}

/// Generate yaml formated string from arguments object ----------------------
void to_string( const arguments& arguments_, std::string& stringOut, tag_io_yaml )
{
   std::string stringResult;
   std::string stringEscaped;
   std::string_view stringValue;

   unsigned uIndex = 0;
   for( auto it = arguments_.named_begin(); it != arguments_.named_end(); ++it )
   {
      if( uIndex > 0 ) { stringResult += "\n"; }                              // Add newline separator
      
      std::string_view stringName = it->first;
      gd::variant_view value_ = it->second.as_variant_view();

      stringResult += std::string(stringName);
      stringResult += ":";

      if( value_.is_string() == true )
      {
         stringResult += " ";
         if( value_.is_char_string() == true )
         {
            stringValue = value_.as_string_view();
            // Check if string needs quoting (contains special characters or is empty)
            if( stringValue.empty() == false )
            {
               // Use JSON escaping as YAML uses similar escaping rules for quoted strings
               if( gd::utf8::json::find_character_to_escape( stringValue ) != nullptr )
               {
                  stringEscaped = gd::utf8::json::convert_utf8_to_json( stringValue );
                  stringResult += "\"" + stringEscaped + "\"";
               }
               else
               {
                  stringResult += stringValue;
               }
            }
            else
            {
               // Empty string
               stringResult += "\"\"";
            }
         }
         else
         {
            stringEscaped = value_.as_string();
            stringEscaped = gd::utf8::json::convert_utf8_to_json( stringEscaped );
            stringResult += "\"" + stringEscaped + "\"";
         }
      }
      else if( value_.is_number() == true ) { stringResult += " " + value_.as_string(); }
      else if( value_.is_bool() == true ) { stringResult += std::string(" ") + ( (bool)value_ == true ? std::string("true") : std::string("false") ); }
      else if( value_.is_binary() == true ) { stringResult += " \"" + value_.as_string() + "\""; }
      else if( value_.is_null() == true ) { stringResult += " null"; }

      uIndex++;
   }

   if( stringOut.empty() == false ) { stringOut += stringResult; }
   else stringOut = std::move( stringResult );
}

_GD_ARGUMENT_END
