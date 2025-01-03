/**
* \file gd_variant_common.h
* 
* \brief general code used for variant and variant_view objects
* 
*/

#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_variant.h"
#include "gd_variant_view.h"

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

template <typename TYPE, typename VARIANT>
TYPE get(const VARIANT& v_)
{
   return (TYPE)v_;
}

// ## specialization for variant
template <>
bool get(const gd::variant& v_) { return v_.as_bool(); }

template <>
uint32_t get(const gd::variant& v_) { return v_.as_uint(); }
template <>
uint64_t get(const gd::variant& v_) { return v_.as_uint64(); }

template <>
int32_t get(const gd::variant& v_) { return v_.as_int(); }
template <>
int64_t get(const gd::variant& v_) { return v_.as_int64(); }

template <>
std::string get(const gd::variant& v_) { return v_.as_string(); }
template <>
std::string_view get(const gd::variant& v_) { return v_.as_string_view(); }

// ## specialization for variant_view
template <>
bool get(const gd::variant_view& v_) { return v_.as_bool(); }

template <>
uint32_t get(const gd::variant_view& v_) { return v_.as_uint(); }
template <>
uint64_t get(const gd::variant_view& v_) { return v_.as_uint64(); }

template <>
int32_t get(const gd::variant_view& v_) { return v_.as_int(); }
template <>
int64_t get(const gd::variant_view& v_) { return v_.as_int64(); }

template <>
std::string get(const gd::variant_view& v_) { return v_.as_string(); }
template <>
std::string_view get(const gd::variant_view& v_) { return v_.as_string_view(); }

_GD_END