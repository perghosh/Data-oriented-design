// @FILE [tag: json] [summary: JSON parsing functions] [type: source] [name: gd_parse_json.cpp]

#include "../gd_variant.h"
#include "../gd_utf8.h"

#include "gd_parse_json.h"


_GD_PARSE_JSON_BEGIN

/**
 * \brief Parse a JSON object into key-value pairs (shallow copy).
 * 
 * Parses a JSON object string into key-value pairs stored in an arguments object.
 * This is a shallow copy implementation - values are stored as strings without
 * recursively parsing nested objects or arrays.
 * 
 ** Sample JSON objects to parse:**
 * - {"name":"John","age":30}
 * - {"key":"value","nested":{"inner":"value"}}
 * - name=John&age=30 (without braces, like query string format)
 * - {"string":"Hello \"World\"","escaped":"Line\\nBreak"}
 * - {"empty":"","number":123.45}
 * 
 ** Structure of a JSON object:**
 { ┬─ "key1" : "value1" , ── "key2" : "value2" ─┬ }
   │   ├────┘  │  ├────┘   │  ├────┘  │  ├────┘   │
   │   │       │  │        │  │       │  │        │
   │   │       │  │        │  │       │  │        └─ optional closing brace
   │   │       │  │        │  │       │  └───────── value (stored as string)
   │   │       │  │        │  └───────── colon separator
   │   │       │  └───────── value (stored as string)
   │   │       └─────────── colon separator
   │   └─────────────────── key (quoted string)
   └─────────────────────── optional opening brace
 * 
 * @param stringJson The JSON string to parse (with or without surrounding braces)
 * @param argumentsJson Output arguments object to store parsed key-value pairs
 * @return Pair of success flag and error message
 */
template<typename ARGUMENTS>
std::pair<bool, std::string> parse_shallow_object_implementation( std::string_view stringJson, ARGUMENTS& argumentsJson, bool bEncode = true )
{
   // Clear any existing arguments 
   argumentsJson.clear();
   
   // Check if the JSON string is empty
   if( stringJson.empty() ) { return { false, "No JSON provided" }; }
   
   const char* piPosition = stringJson.data();                                // current position in JSON string
   const char* piBegin = stringJson.data();                                   // start of JSON string
   const char* piEnd = stringJson.data() + stringJson.size();                  // end of JSON string
   const char* piPartStart = piPosition;                                      // start of current part being parsed
   
   // ## Skip whitespace at start ............................................
   
   piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );
   //while( piPosition < piEnd && ( *piPosition == ' ' || *piPosition == '\t' || *piPosition == '\n' || *piPosition == '\r' ) ) { piPosition++; }
   
   // ## Check for optional opening brace .....................................
   
   bool bHasBraces = false;                                                   // flag indicating if object has braces
   if( piPosition < piEnd && *piPosition == '{' )
   {
      bHasBraces = true;
      piPosition++;                                                           // skip '{'
      
      // Skip whitespace after opening brace
      piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );
      //while( piPosition < piEnd && ( *piPosition == ' ' || *piPosition == '\t' || *piPosition == '\n' || *piPosition == '\r' ) ) { piPosition++; }
   }
   
   // ## Parse key-value pairs ..............................................
   
   while( piPosition < piEnd )
   {
      // ### Skip whitespace before key .....................................
      
      piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );
      
      // ### Check for closing brace or end of string .......................
      
      if( piPosition < piEnd && *piPosition == '}' )
      {
         piPosition++;                                                        // skip '}'
         break;
      }
      
      if( piPosition >= piEnd ) { break; }
      
      // ### Parse key ......................................................
      
      const char* piKeyStart = nullptr;                                       // start of key string
      const char* piKeyEnd = nullptr;                                         // end of key string
      std::string stringKey;                                                  // decoded key string
      bool bKeyQuoted = false;                                                // flag indicating if key was quoted
      
      if( *piPosition == '"' )
      {
         // #### Key is quoted string .......................................
         
         bKeyQuoted = true;
         piPosition++;                                                        // skip opening quote
         piKeyStart = piPosition;
         
         bool bEscape = false;                                                // escape flag for processing escape sequences
         while( piPosition < piEnd )
         {
            if( bEscape ) { bEscape = false; }
            else if( *piPosition == '\\' ) { bEscape = true; }
            else if( *piPosition == '"' ) { break; }
            piPosition++;
         }
         
         if( piPosition >= piEnd ) { return { false, "Unclosed key string" }; }
         
         piKeyEnd = piPosition;
         piPosition++;                                                        // skip closing quote
         
         // ## Decode JSON escape sequences in key ..........................
         
         if( bEncode == true )
         {
            std::string_view stringKeyEncoded( piKeyStart, piKeyEnd - piKeyStart );
            stringKey = gd::utf8::convert_json( stringKeyEncoded );
         }
         else { stringKey = std::string_view( piKeyStart, piKeyEnd - piKeyStart ); }
      }
      else
      {
         // #### Key is unquoted (query string style) .......................
         
         piKeyStart = piPosition;
         
         while( piPosition < piEnd && *piPosition != '=' && *piPosition != ',' && *piPosition != '}' && 
                *piPosition != '&' && *piPosition != '\n' && *piPosition != '\r' )
         {
            piPosition++;
         }
         
         piKeyEnd = piPosition;
         
         if( piKeyEnd <= piKeyStart ) { return { false, "Empty key" }; }
         
         stringKey = std::string( piKeyStart, piKeyEnd - piKeyStart );
      }
      
      // ### Skip whitespace after key .........................................
      
      piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );
      
      // ### Check for colon or equals separator ................................
      
      if( piPosition >= piEnd || ( *piPosition != ':' && *piPosition != '=' ) )
      {
         // If no separator and we have braces, it's an error. Without braces, key without value is allowed.
         if( bHasBraces ) { return { false, "Expected ':' or '=' after key" }; }
         else { return { false, "Expected '=' after key" }; }
      }
      
      piPosition++;                                                           // skip ':' or '='
      
      piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );       // skip space
      
      // ### Parse value ........................................................
      
      const char* piValueStart = nullptr;                                     // start of value string
      const char* piValueEnd = nullptr;                                       // end of value string
      std::string stringValue;                                                // decoded value string
      
      if( piPosition < piEnd && *piPosition == '"' )
      {
         // #### Value is quoted string ........................................
         
         piPosition++;                                                        // skip opening quote
         piValueStart = piPosition;
         
         bool bEscape = false;                                                // escape flag for processing escape sequences
         while( piPosition < piEnd )
         {
            if( bEscape ) { bEscape = false; }
            else if( *piPosition == '\\' ) { bEscape = true; }
            else if( *piPosition == '"' ) { break; }
            piPosition++;
         }
         
         if( piPosition >= piEnd ) { return { false, "Unclosed value string" }; }
         
         piValueEnd = piPosition;
         piPosition++;                                                        // skip closing quote
         
         // ## Decode JSON escape sequences in value ............................
         if( bEncode == true )
         {
            std::string_view stringValueEncoded( piValueStart, piValueEnd - piValueStart );
            stringValue = gd::utf8::convert_json( stringValueEncoded );
         }
         else { stringValue = std::string_view( piValueStart, piValueEnd - piValueStart ); }

         argumentsJson.push_back( { stringKey, gd::variant_type::utf8( stringValue ) } );
      }
      else
      {
         // #### Value is unquoted (number, boolean, null, etc.) ..............
         
         piValueStart = piPosition;
         
         while( piPosition < piEnd && *piPosition != ',' && *piPosition != '}' && 
                *piPosition != '&' && *piPosition != '\n' && *piPosition != '\r' )
         { piPosition++; }
         
         piValueEnd = piPosition;
         
         // Trim trailing whitespace from value
         while( piValueEnd > piValueStart && ( *( piValueEnd - 1 ) == ' ' || *( piValueEnd - 1 ) == '\t' ) ) { piValueEnd--; }
         
         stringValue = std::string( piValueStart, piValueEnd - piValueStart );
         bool bSuccess;
         gd::variant variantValue( stringValue, &bSuccess, gd::types::tag_json{} );

         // Check or error, and you shouldn't find string here
         if( bSuccess == false || variantValue.is_string() == true ) { return { false, "Invalid JSON value" }; }
         
         argumentsJson.append_argument( stringKey, variantValue );
      }
      
      
      // ### Skip whitespace after value .......................................
      
      piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );       // skip space
      
      // ### Check for separator (comma, ampersand) or closing brace ............
      
      if( piPosition < piEnd && *piPosition == ',' ) { piPosition++; }        // skip ','
      else if( piPosition < piEnd && *piPosition == '&' && false ==bHasBraces ) { piPosition++; } // skip '&' (query string separator)
      else if( piPosition < piEnd && *piPosition == '}' ) { }                 // Closing brace will be handled in next iteration
      else if( piPosition < piEnd )
      {
         // If we have braces and no separator, it's an error
         if( bHasBraces ) { return { false, "Expected ',' or '}' after value" }; }
      }
   }
   
   // ## If we had opening brace, expect closing brace ..........................
   
   if( bHasBraces )
   {
      // Skip trailing whitespace
      piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );       // skip space
      
      if( piPosition < piEnd && *piPosition == '}' ) { piPosition++; }        // skip '}'
   }
   
   return { true, "" };
}

/**
 * \brief Parse a JSON object into key-value pairs (shallow copy).
 * 
 * Parses a JSON object string into key-value pairs stored in a regular arguments object.
 * Values are stored as strings without recursively parsing nested objects or arrays.
 * 
 * @param stringJson The JSON string to parse (with or without surrounding braces)
 * @param argumentsJson Output arguments object to store parsed key-value pairs
 * @return Pair of success flag and error message
 * 
 * Example:
 *   "{\"name\":\"John\",\"age\":30}" -> {{"name", "John"}, {"age", "30"}}
 *   "key=value&another=data" -> {{"key", "value"}, {"another", "data"}}
 */
std::pair<bool, std::string> parse_shallow_object_g( std::string_view stringJson, gd::argument::arguments& argumentsJson, bool bEncode )
{
   return parse_shallow_object_implementation( stringJson, argumentsJson, bEncode );
}

/**
 * \brief Parse a JSON object into key-value pairs (shallow copy).
 * 
 * Parses a JSON object string into key-value pairs stored in a shared arguments object.
 * Values are stored as strings without recursively parsing nested objects or arrays.
 * 
 * @param stringJson The JSON string to parse (with or without surrounding braces)
 * @param argumentsJson Output shared arguments object to store parsed key-value pairs
 * @return Pair of success flag and error message
 * 
 * Example:
 *   "{\"name\":\"John\",\"age\":30}" -> {{"name", "John"}, {"age", "30"}}
 *   "key=value&another=data" -> {{"key", "value"}, {"another", "data"}}
 */
std::pair<bool, std::string> parse_shallow_object_g( std::string_view stringJson, gd::argument::shared::arguments& argumentsJson, bool bEncode )
{
   return parse_shallow_object_implementation( stringJson, argumentsJson, bEncode );
}


/** --------------------------------------------------------------------------
 * \brief Determine the JSON value type from a string view
 * 
 * Analyzes a string to determine what type of JSON value it represents.
 * Handles null, boolean (true/false), decimal (integer and floating-point),
 * string (quoted), array, and object types.
 * 
 * @param stringValue The string value to analyze
 * @return The detected value type
 * 
 * Examples:
 *   "null" -> value_type::null
 *   "true" -> value_type::boolean
 *   "false" -> value_type::boolean
 *   "123" -> value_type::decimal
 *   "-123.45" -> value_type::decimal
 *   "\"hello\"" -> value_type::string
 *   "[1,2,3]" -> value_type::array
 *   "{\"key\":\"value\"}" -> value_type::object
 */
value_type get_value_type_g( std::string_view stringValue )
{
   if( stringValue.empty() ) { return value_type::string; }
   
   const char* piStart = stringValue.data();                                 // start of value string
   const char* piEnd = stringValue.data() + stringValue.size();              // end of value string
   const char* piPosition = piStart;                                         // current position
   
   // ## Skip leading whitespace ..............................................
   
   piPosition = gd::utf8::move::next_non_space( piPosition, piEnd );
   
   if( piPosition >= piEnd ) { return value_type::string; }
   
   // ## Check for structural types (array, object, string) ...................
   
   if( *piPosition == '[' ) { return value_type::array; }
   if( *piPosition == '{' ) { return value_type::object; }
   if( *piPosition == '"' ) { return value_type::string; }
   
   // ## Check for null .........................................................
   
   if( piEnd - piPosition >= 4 && 
       piPosition[0] == 'n' && piPosition[1] == 'u' && 
       piPosition[2] == 'l' && piPosition[3] == 'l' )
   {
      // Verify it's exactly "null" (not followed by alphanumeric char)
      const char* piAfterNull = piPosition + 4;
      if( piAfterNull >= piEnd || !std::isalnum( static_cast<unsigned char>( *piAfterNull ) ) )
      {
         return value_type::null;
      }
   }
   
   // ## Check for boolean (true) ..............................................
   
   if( piEnd - piPosition >= 4 && 
       piPosition[0] == 't' && piPosition[1] == 'r' && 
       piPosition[2] == 'u' && piPosition[3] == 'e' )
   {
      // Verify it's exactly "true" (not followed by alphanumeric char)
      const char* piAfterTrue = piPosition + 4;
      if( piAfterTrue >= piEnd || !std::isalnum( static_cast<unsigned char>( *piAfterTrue ) ) )
      {
         return value_type::boolean;
      }
   }
   
   // ## Check for boolean (false) .............................................
   
   if( piEnd - piPosition >= 5 && 
       piPosition[0] == 'f' && piPosition[1] == 'a' && 
       piPosition[2] == 'l' && piPosition[3] == 's' && piPosition[4] == 'e' )
   {
      // Verify it's exactly "false" (not followed by alphanumeric char)
      const char* piAfterFalse = piPosition + 5;
      if( piAfterFalse >= piEnd || !std::isalnum( static_cast<unsigned char>( *piAfterFalse ) ) )
      {
         return value_type::boolean;
      }
   }
   
   // ## Check for decimal number ..............................................
   
   bool bIsDecimal = false;                                                  // flag indicating if valid decimal
   const char* piNumberStart = piPosition;                                   // start of potential number
   
   // ### Check for optional negative sign ......................................
   
   if( *piPosition == '-' ) { piPosition++; }
   
   // ### Check for digits before decimal point .................................
   
   bool bHasDigits = false;                                                  // flag indicating if we have at least one digit
   while( piPosition < piEnd && std::isdigit( static_cast<unsigned char>( *piPosition ) ) )
   {
      bHasDigits = true;
      piPosition++;
   }
   
   // ### Check for decimal point and fractional digits ..........................
   
   if( piPosition < piEnd && *piPosition == '.' )
   {
      piPosition++;
      
      // Must have at least one digit after decimal point
      while( piPosition < piEnd && std::isdigit( static_cast<unsigned char>( *piPosition ) ) )
      {
         bHasDigits = true;
         piPosition++;
      }
   }
   
   // ### Check for exponent notation ...........................................
   
   if( piPosition < piEnd && ( *piPosition == 'e' || *piPosition == 'E' ) )
   {
      piPosition++;
      
      // Optional sign in exponent
      if( piPosition < piEnd && ( *piPosition == '+' || *piPosition == '-' ) ) { piPosition++; }
      
      // Must have at least one digit in exponent
      bool bHasExponentDigits = false;
      while( piPosition < piEnd && std::isdigit( static_cast<unsigned char>( *piPosition ) ) )
      {
         bHasExponentDigits = true;
         piPosition++;
      }
      
      if( !bHasExponentDigits ) { bHasDigits = false; }
   }
   
   // ### Determine if valid number ..............................................
   
   if( bHasDigits && piPosition > piNumberStart )
   {
      // Check if we consumed the entire value (or reached non-alphanumeric)
      if( piPosition >= piEnd || !std::isalnum( static_cast<unsigned char>( *piPosition ) ) )
      {
         bIsDecimal = true;
      }
   }
   
   if( bIsDecimal ) { return value_type::decimal; }
   
   // ## Default to string type .................................................
   
   return value_type::string;
}


_GD_PARSE_JSON_END
