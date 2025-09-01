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

bool is_number(const std::string_view& stringText) noexcept;
bool is_integer(const std::string_view& stringText) noexcept;
bool is_unsigned(const std::string_view& stringText) noexcept;
bool is_decimal(const std::string_view& stringText) noexcept;
bool is_hex(const std::string_view& stringText) noexcept;
bool is_binary(const std::string_view& stringText) noexcept;
bool is_octal(const std::string_view& stringText) noexcept;

bool is_alpha(const std::string_view& stringText) noexcept;

_GD_MATH_TYPE_END