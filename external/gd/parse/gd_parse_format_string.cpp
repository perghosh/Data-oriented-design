#pragma once

#include "gd_parse_format_string.h"


_GD_PARSE_FORMAT_STRING_BEGIN

/** ---------------------------------------------------------------------------
 * @brief Formats a string using Python-style placeholders with values from an arguments object.
 *
 * This function replaces placeholders in the format string with corresponding values from
 * the arguments object. It supports both positional placeholders (e.g., {0}, {1}) and
 * named placeholders (e.g., {name}, {value}). Empty placeholders {} are filled sequentially
 * starting from index 0. The function handles escaping of braces using {{ and }}.
 *
 * Placeholder formats:
 * - {0}, {1}, {2}, etc. - Positional arguments by index
 * - {name}, {key}, etc. - Named arguments
 * - {} - Auto-incremented positional argument
 * - {{ or }} - Escaped braces (literal { or })
 *
 * @param stringFormat The format string containing placeholders.
 * @param argumentsValue The arguments object containing values to substitute.
 * @return std::string The formatted string with placeholders replaced.
 * 
 * @code
 * gd::argument::arguments args;
 * args.append("name", "Alice");
 * args.append("age", 30);
 * args.append("city", "New York");
 * 
 * std::string result = format_string("Hello {name}, you are {age} years old and live in {city}!", args);
 * // result: "Hello Alice, you are 30 years old and live in New York!"
 * 
 * gd::argument::arguments args2;
 * args2.append(100);
 * args2.append(200);
 * args2.append(300);
 * 
 * std::string result2 = format_string("Values: {0}, {1}, {2}", args2);
 * // result2: "Values: 100, 200, 300"
 * 
 * std::string result3 = format_string("Auto: {}, {}, {}", args2);
 * // result3: "Auto: 100, 200, 300"
 * 
 * std::string result4 = format_string("Escaped: {{not a placeholder}}", args);
 * // result4: "Escaped: {not a placeholder}"
 * @endcode
 */
std::string format_string( const std::string_view& stringFormat, const gd::argument::arguments& argumentsValue )
{
   std::string stringResult;
   stringResult.reserve(stringFormat.size() * 2); // Reserve space to minimize reallocations
   
   size_t uPosition = 0;
   size_t uAutoIndex = 0; // Index for auto-increment placeholders {}
   
   while(uPosition < stringFormat.size())
   {
      // ## Find next opening brace
      size_t uBraceStart = stringFormat.find('{', uPosition);
      
      if(uBraceStart == std::string_view::npos) { stringResult.append(stringFormat.substr(uPosition)); break; } // No more placeholders, append remaining text
      
      stringResult.append(stringFormat.substr(uPosition, uBraceStart - uPosition)); // Append text before the brace
      
      // ## Check for escaped brace {{
      if(uBraceStart + 1 < stringFormat.size() && stringFormat[uBraceStart + 1] == '{')
      {
         stringResult.push_back('{');
         uPosition = uBraceStart + 2;
         continue;
      }
      
      // ## Find closing brace
      size_t uBraceEnd = stringFormat.find('}', uBraceStart);
      if(uBraceEnd == std::string_view::npos)
      {
         // Malformed placeholder, append remaining text as-is
         stringResult.append(stringFormat.substr(uBraceStart));
         break;
      }
      
      // ## Check for escaped closing brace }}
      if(uBraceEnd + 1 < stringFormat.size() && stringFormat[uBraceEnd + 1] == '}')
      {
         stringResult.append(stringFormat.substr(uBraceStart, uBraceEnd - uBraceStart + 1));
         stringResult.push_back('}');
         uPosition = uBraceEnd + 2;
         continue;
      }
      
      // ## Extract placeholder content
      std::string_view stringPlaceholder = stringFormat.substr(uBraceStart + 1, uBraceEnd - uBraceStart - 1);
      
      // ## Handle empty placeholder {} - use auto-increment index
      if(stringPlaceholder.empty())
      {
         if(uAutoIndex < argumentsValue.size())
         {
            auto value = argumentsValue[(unsigned)uAutoIndex];
            stringResult.append(value.as_string());
            uAutoIndex++;
         }
         else
         {
            // Index out of bounds, keep placeholder as-is
            stringResult.append("{").append(stringPlaceholder).append("}");
         }
      }
      // ## Check if placeholder is numeric (positional argument)
      else if(std::isdigit(stringPlaceholder[0]))
      {
         try
         {
            size_t uIndex = std::stoull(std::string(stringPlaceholder));
            if(uIndex < argumentsValue.size())
            {
               auto value_ = argumentsValue[(unsigned)uIndex];
               stringResult.append(value_.as_string());
            }
            else
            {
               // Index out of bounds, keep placeholder as-is
               stringResult.append("{").append(stringPlaceholder).append("}");
            }
         }
         catch(...)
         {
            // Invalid index format, keep placeholder as-is
            stringResult.append("{").append(stringPlaceholder).append("}");
         }
      }
      // ### Named placeholder
      else
      {
         auto value = argumentsValue[stringPlaceholder];
         if(value.is_null() == false)
         {
            // Named argument not found, keep placeholder as-is
            stringResult.append("{").append(stringPlaceholder).append("}");
         }
         else { stringResult.append(value.as_string()); }
      }
      
      uPosition = uBraceEnd + 1;
   }
   
   return stringResult;
}


_GD_PARSE_FORMAT_STRING_END
