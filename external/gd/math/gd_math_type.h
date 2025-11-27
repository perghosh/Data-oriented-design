// @FILE [tag: math, type] [description: Type checking functions] [type: header]

/**
 * @file gd_math_type.h
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>


#ifndef _GD_MATH_TYPE_BEGIN

#define _GD_MATH_TYPE_BEGIN namespace gd { namespace math { namespace type {
#define _GD_MATH_TYPE_END } } }

#endif


_GD_MATH_TYPE_BEGIN

// @API [tag: math, string, type] [summary: Type checking functions for strings] [description: Functions to check if strings represent specific data types like numbers, integers, hex, etc.]

bool is_number(const std::string_view& stringText) noexcept;
bool is_integer(const std::string_view& stringText) noexcept;
bool is_unsigned(const std::string_view& stringText) noexcept;
bool is_decimal(const std::string_view& stringText) noexcept;
bool is_hex(const std::string_view& stringText) noexcept;
bool is_binary(const std::string_view& stringText) noexcept;
bool is_octal(const std::string_view& stringText) noexcept;

bool is_alpha(const std::string_view& stringText) noexcept;

bool is_alphanumeric(const std::string_view& stringText) noexcept;
bool is_lowercase(const std::string_view& stringText) noexcept;
bool is_uppercase(const std::string_view& stringText) noexcept;
bool is_whitespace(const std::string_view& stringText) noexcept;
bool is_printable(const std::string_view& stringText) noexcept;
bool is_ascii(const std::string_view& stringText) noexcept;
bool is_utf8(const std::string_view& stringText) noexcept;


// Complex numbers
bool is_complex(const std::string_view& stringText) noexcept;  // "3+4i"

// Boolean representations
bool is_boolean(const std::string_view& stringText) noexcept;  // "true", "false", "0", "1"

// Base conversions
bool is_base_n(const std::string_view& stringText, int base) noexcept;

// Mathematical expressions
bool is_expression(const std::string_view& stringText) noexcept;  // Contains operators
bool is_balanced_parentheses(const std::string_view& stringText) noexcept;

_GD_MATH_TYPE_END