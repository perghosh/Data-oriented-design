/**
* @file gd_expression_method_01.h
* 
* @brief 
* 
*/

#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "gd_expression.h"
#include "gd_expression_value.h"
#include "gd_expression_runtime.h"

#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 

std::pair<bool, std::string> average_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> max_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> min_g(const std::vector< value >& vectorArgument, value* pvalueResult);  
std::pair<bool, std::string> sum_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> abs_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> round_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> floor_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> ceil_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> if_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> is_null_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> is_not_null_g(const std::vector<value>& vectorArgument, value* pvalueResult);

std::pair<bool, std::string> tolower_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> toupper_g(const std::vector< value >& vectorArgument, value* pvalueResult);

std::pair<bool, std::string> length_g( const std::vector< value >& vectorArgument, value* pvalueResult );
std::pair<bool, std::string> count_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> find_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> has_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> has_tag_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> list_tags_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> missing_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> starts_with_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> ends_with_g(const std::vector< value >& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> trim_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> ltrim_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> rtrim_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> substring_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> replace_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> reverse_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> repeat_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> is_numeric_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> is_alpha_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> is_empty_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> char_at_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> left_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> right_g(const std::vector<value>& vectorArgument, value* pvalueResult);
std::pair<bool, std::string> mid_g(const std::vector<value>& vectorArgument, value* pvalueResult);


//============================================================================
// Global method registry for mathematical and logical functions
// Format: { function_pointer, "method_name", parameter_count, version }
//============================================================================

// Core mathematical and logical operations
// Arguments format: method_name(arg1, arg2, ...)
const method pmethodDefault_g[] = {
   { (void*)&abs_g, "abs", 1, 1},                    // abs(number) - absolute value
   { (void*)&average_g, "average", 2, 1},           // average(a, b) - mean of two numbers
   { (void*)&ceil_g, "ceil", 1, 1},                 // ceil(number) - round up to integer
   { (void*)&floor_g, "floor", 1, 1},               // floor(number) - round down to integer
   { (void*)&if_g, "if", 3, 1 },                    // if(condition, true_value, false_value)
   { (void*)&is_not_null_g, "is_not_null", 1, 1 },  // is_not_null(value) - check not null
   { (void*)&is_null_g, "is_null", 1, 1 },          // is_null(value) - check if null
   { (void*)&max_g, "max", 2, 1 },                  // max(a, b) - greater of two values
   { (void*)&min_g, "min", 2, 1 },                  // min(a, b) - lesser of two values
   { (void*)&round_g, "round", 1, 1 },              // round(number) - round to nearest integer
   { (void*)&sum_g, "sum", 2, 1 }                   // sum(a, b) - add two numbers
};

// Calculate array size at compile time
constexpr size_t uMethodDefaultSize_g = sizeof(pmethodDefault_g) / sizeof(method);

// String manipulation and text processing functions
// Arguments format: method_name(arg1, arg2, ...)
const method pmethodString_g[] = {
   { (void*)&char_at_g, "char_at", 2, 1 },           // char_at(text, index) - get character at position
   { (void*)&count_g, "count", 2, 1 },               // count(haystack, needle) - count occurrences
   { (void*)&ends_with_g, "ends_with", 2, 1 },       // ends_with(haystack, suffix) - check string ending
   { (void*)&find_g, "find", 3, 1 },                 // find(text, word, offset) - find substring position
   { (void*)&has_g, "has", 2, 1 },                   // has(haystack, needle) - check if contains substring
   { (void*)&has_tag_g, "has_tag", 2, 1 },           // has_tag(text, tag) - check if text contains tag
   { (void*)&is_alpha_g, "is_alpha", 1, 1 },         // is_alpha(text) - check if only alphabetic chars
   { (void*)&is_empty_g, "is_empty", 1, 1 },         // is_empty(text) - check if empty or whitespace
   { (void*)&left_g, "left", 2, 1 },                 // left(text, count) - get leftmost characters
   { (void*)&length_g, "length", 1, 1 },             // length(text) - get string length
   { (void*)&list_tags_g, "list_tags", 1, 1 },       // list_tags(text) - extract unique tags as CSV
   { (void*)&ltrim_g, "ltrim", 1, 1 },               // ltrim(text) - remove leading whitespace
   { (void*)&mid_g, "mid", 3, 1 },                   // mid(text, start, length) - substring (1-based start)
   { (void*)&missing_g, "missing", 2, 1 },           // missing(haystack, needle) - check if lacks substring
   { (void*)&repeat_g, "repeat", 2, 1 },             // repeat(text, count) - repeat string N times
   { (void*)&replace_g, "replace", 3, 1 },           // replace(text, old, new) - replace all occurrences
   { (void*)&reverse_g, "reverse", 1, 1 },           // reverse(text) - reverse character order
   { (void*)&right_g, "right", 2, 1 },               // right(text, count) - get rightmost characters
   { (void*)&rtrim_g, "rtrim", 1, 1 },               // rtrim(text) - remove trailing whitespace
   { (void*)&starts_with_g, "starts_with", 2, 1 },   // starts_with(haystack, prefix) - check string start
   { (void*)&substring_g, "substring", 3, 1 },       // substring(text, start, length) - extract substring
   { (void*)&tolower_g, "tolower", 1, 1},            // tolower(text) - convert to lowercase
   { (void*)&toupper_g, "toupper", 1, 1 },           // toupper(text) - convert to uppercase
   { (void*)&trim_g, "trim", 1, 1 },                 // trim(text) - remove leading/trailing whitespace
};

// Calculate array size at compile time
constexpr size_t uMethodStringSize_g = sizeof(pmethodString_g) / sizeof(method);


_GD_EXPRESSION_END