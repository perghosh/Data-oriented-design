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

/// create rule for `variant_view` and `variant`
template <typename VARIANT>
concept IsVariant = std::is_same_v<VARIANT, gd::variant> || std::is_same_v<VARIANT, gd::variant_view>;

/// `std::get` like function for `variant` and `variant_view`
template <typename RETURN_TYPE, typename VARIANT>
requires IsVariant<VARIANT>
RETURN_TYPE get(const VARIANT& v_)
{
   if constexpr ( std::is_same_v<RETURN_TYPE, bool> )
      return v_.as_bool();
   else if constexpr ( std::is_same_v<RETURN_TYPE, uint32_t> )
      return v_.as_uint();
   else if constexpr ( std::is_same_v<RETURN_TYPE, uint64_t> )
      return v_.as_uint64();
   else if constexpr ( std::is_same_v<RETURN_TYPE, int32_t> )
      return v_.as_int();
   else if constexpr ( std::is_same_v<RETURN_TYPE, int64_t> )
      return v_.as_int64();
   else if constexpr ( std::is_same_v<RETURN_TYPE, double> )
      return v_.as_double();
   else if constexpr ( std::is_same_v<RETURN_TYPE, std::string> )
      return v_.as_string();
   else if constexpr ( std::is_same_v<RETURN_TYPE, std::string_view> )
      return v_.as_string_view();
   else
      return (RETURN_TYPE)v_;
}

_GD_END

/*

#include <iostream>
#include <type_traits>

template <typename T, typename U>
concept Arithmetic = std::is_arithmetic_v<T> && std::is_arithmetic_v<U>;

template <typename T, typename U>
requires Arithmetic<T, U>
auto add(const T& a, const U& b) -> decltype(a + b) { 
  return a + b; 
}

template <typename T, typename U>
requires (!Arithmetic<T, U>)
std::string add(const T& a, const U& b) { 
  return "Cannot add these types."; 
}

int main() {
  int x = 5;
  double y = 3.14;
  std::string s = "hello";

  std::cout << add(x, y) << std::endl;   // Calls add(int, double)
  std::cout << add(x, s) << std::endl;   // Calls add(int, std::string) 
  // std::cout << add(s, x) << std::endl; // Won't compile

  return 0;
}


#include <iostream>
#include <type_traits>

template <typename T>
constexpr bool is_integral_v = std::is_integral_v<T>;

template <typename T>
constexpr bool is_floating_point_v = std::is_floating_point_v<T>;

template <typename T>
constexpr bool is_pointer_v = std::is_pointer_v<T>;

template <typename T>
constexpr void print_type_info() {
  if constexpr (is_integral_v<T>) {
    std::cout << "T is an integral type" << std::endl;
  } else if constexpr (is_floating_point_v<T>) {
    std::cout << "T is a floating-point type" << std::endl;
  } else if constexpr (is_pointer_v<T>) {
    std::cout << "T is a pointer type" << std::endl;
  } else {
    std::cout << "T is of unknown type" << std::endl;
  }
}

int main() {
  print_type_info<int>();    // Output: T is an integral type
  print_type_info<double>();  // Output: T is a floating-point type
  print_type_info<char*>();   // Output: T is a pointer type
  print_type_info<std::string>(); // Output: T is of unknown type

  return 0;
}


#include <iostream>
#include <type_traits>

template <typename T>
concept Integer32 = std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t>;

template <typename T>
requires Integer32<T>
void print_integer32(const T& value) {
  std::cout << "Value: " << value << std::endl;
}

int main() {
  uint32_t u = 42;
  int32_t i = -123;

  print_integer32(u); 
  print_integer32(i); 

  // print_integer32(1.5); // Compilation error: 1.5 is not uint32_t or int32_t

  return 0;
}

*/