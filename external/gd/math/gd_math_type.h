// @FILE [tag: math, type] [description: Type checking functions] [type: header] [name: gd_math_type.h]

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

bool is_number(std::string_view stringText) noexcept;
bool is_integer(std::string_view stringText) noexcept;
bool is_unsigned(std::string_view stringText) noexcept;
bool is_decimal(std::string_view stringText) noexcept;
bool is_hex(std::string_view stringText) noexcept;
bool is_binary(std::string_view stringText) noexcept;
bool is_octal(std::string_view stringText) noexcept;

bool is_alpha(std::string_view stringText) noexcept;

bool is_alphanumeric(std::string_view stringText) noexcept;
bool is_lowercase(std::string_view stringText) noexcept;
bool is_uppercase(std::string_view stringText) noexcept;
bool is_whitespace(std::string_view stringText) noexcept;
bool is_printable(std::string_view stringText) noexcept;
bool is_ascii(std::string_view stringText) noexcept;
bool is_utf8(std::string_view stringText) noexcept;


// Complex numbers
bool is_complex(std::string_view stringText) noexcept;  // "3+4i"

// Boolean representations
bool is_boolean(std::string_view stringText) noexcept;  // "true", "false", "0", "1"

// Base conversions
bool is_base_n(std::string_view stringText, int base) noexcept;

// Mathematical expressions
bool is_expression(std::string_view stringText) noexcept;  // Contains operators
bool is_balanced_parentheses(std::string_view stringText) noexcept;

_GD_MATH_TYPE_END
