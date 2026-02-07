// // @FILE [tag: json] [summary: JSON parsing functions] [type: header] [name: gd_parse_json.h]

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

#include "../gd_types.h"
#include "../gd_arguments.h"
#include "../gd_arguments_shared.h"


#ifndef _GD_PARSE_JSON_BEGIN
#  define _GD_PARSE_JSON_BEGIN namespace gd { namespace parse { namespace json {
#  define _GD_PARSE_JSON_END } } }
#endif

_GD_PARSE_JSON_BEGIN


std::pair<bool, std::string> parse_shallow_object_g( std::string_view stringJson, gd::argument::arguments& argumentsJson, bool bEncode = true );
std::pair<bool, std::string> parse_shallow_object_g( std::string_view stringJson, gd::argument::shared::arguments& argumentsJson, bool bEncode = true );


/**
 * \brief Enum representing JSON value types
 */
enum class value_type
{
   null = gd::types::eTypeUnknown,             /// null value
   boolean = gd::types::eTypeBool,             /// true or false
   decimal = gd::types::eTypeCDouble,          /// integer or floating-point number
   string = gd::types::eTypeUtf8String,        /// quoted string or unquoted text
   array = gd::types::eTypeArray,              /// array (starts with '[')
   object = gd::types::eTypeObject,            /// object (starts with '{')
};

/**
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
value_type get_value_type_g( std::string_view stringValue );


_GD_PARSE_JSON_END
