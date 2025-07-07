/**
 * @file gd_math_string.h
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


#ifndef _GD_MATH_STRING_BEGIN

#define _GD_MATH_STRING_BEGIN namespace gd { namespace math { namespace string {
#define _GD_MATH_STRING_END } } }

#endif


_GD_MATH_STRING_BEGIN

/// Counts the number of occurrences of iCharacter in stringText.
size_t count_character(const std::string_view& stringText, char iCharacter) noexcept;

/// Extracts substring from the first occurrence of stringFrom to the end.
std::string select_from(const std::string_view& stringText, const std::string_view& stringFrom);

/// Extracts substring up to (not including) the first occurrence of stringTo.
std::string select_until(const std::string_view& stringText, const std::string_view& stringTo);

/// Extracts substring between stringFrom and stringTo.
std::string select_between(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo);

/// Extracts substring between the nth occurrence of stringFrom and the next stringTo.
std::string select_between_nth(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo, size_t uOccurrence);

/// Removes prefix and suffix if present.
std::string select_unwrap(const std::string_view& stringText, const std::string_view& stringPrefix, const std::string_view& stringSuffix);

/// Extracts all substrings between pairs of delimiters.
std::vector<std::string> select_between_all(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo);


_GD_MATH_STRING_END