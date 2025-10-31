/**
 * @file gd_parse_format_string.h
 * @brief 
 * 
 * Parse logic for know formats
 */


#pragma once

#include "gd/gd_compiler.h"

#include <array>
#include <cassert>
#include <cstring>
#if GD_COMPILER_HAS_CPP20_SUPPORT
#include <span>
#endif
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_FORMAT_STRING_BEGIN
#  define _GD_PARSE_FORMAT_STRING_BEGIN namespace gd { namespace parse { namespace format {
#  define _GD_PARSE_FORMAT_STRING_END } } }
#endif

_GD_PARSE_FORMAT_STRING_BEGIN

std::string format_string( const std::string_view& stringFormat, const gd::argument::arguments& argumentsValue );

/** ---------------------------------------------------------------------------
 * @brief Formats a string using Python-style placeholders with variadic arguments.
 *
 * This convenience function allows formatting with explicit arguments instead of
 * requiring an arguments object. It accepts any number of arguments of various types
 * and constructs an arguments object internally before calling format_string.
 *
 * @param stringFormat The format string containing placeholders.
 * @param args Variadic template arguments to be used for substitution.
 * @return std::string The formatted string with placeholders replaced.
 * 
 * @code
 * std::string result = format("Hello {0}, you are {1} years old!", "Alice", 30);
 * // result: "Hello Alice, you are 30 years old!"
 * 
 * std::string result2 = format("Values: {}, {}, {}", 100, 200.5, "text");
 * // result2: "Values: 100, 200.5, text"
 * @endcode
 */
template<typename... Args>
std::string format( const std::string_view& stringFormat, Args&&... args )
{
   gd::argument::arguments arguments;
   (arguments.append(std::forward<Args>(args)), ...); // Fold expression to append all arguments
   return format_string(stringFormat, arguments);
}

/** ---------------------------------------------------------------------------
 * @brief Formats a string using named arguments from an initializer list.
 *
 * This function provides a convenient way to format strings with named placeholders
 * by accepting an initializer list of key-value pairs. It's particularly useful
 * for ad-hoc formatting without constructing an arguments object manually.
 *
 * @param stringFormat The format string containing named placeholders.
 * @param namedArgs Initializer list of name-value pairs.
 * @return std::string The formatted string with placeholders replaced.
 * 
 * @code
 * std::string result = format_named("Hello {name}, you are {age} years old!",
 *                                    {{"name", "Alice"}, {"age", 30}});
 * // result: "Hello Alice, you are 30 years old!"
 * 
 * std::string result2 = format_named("{greeting} {name}! Today is {day}.",
 *                                     {{"greeting", "Good morning"}, 
 *                                      {"name", "Bob"}, 
 *                                      {"day", "Monday"}});
 * // result2: "Good morning Bob! Today is Monday."
 * @endcode
 */
template<typename TYPE>
std::string format_named( const std::string_view& stringFormat, std::initializer_list<std::pair<std::string, TYPE>> namedArgs )
{
   gd::argument::arguments arguments;
   for(const auto& pair : namedArgs)
   {
      arguments.append(pair.first, pair.second);
   }
   return format_string(stringFormat, arguments);
}

_GD_PARSE_FORMAT_STRING_END
