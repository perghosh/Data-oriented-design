/**
* \file gd_debug.h
* 
* \brief 
* 
* Information about console 
* Format switching between different colors
* `\033[<style>;<foreground_color>;<background_color>m`
* 
* \033[38;5;<color_code>m  // Set foreground (text) color
* \033[48;5;<color_code>m  // Set background color

* 
*/



#pragma once
#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>



#ifndef _GDD_BEGIN
   #define _GDD_BEGIN namespace gdd {
   #define _GDD_END }
   _GDD_BEGIN
#else
   _GDD_BEGIN
#endif

// ## Buffer checks

bool buffer_find( const uint8_t* pubuffer, const std::vector<uint8_t>& vectorFind, size_t uEnd );
bool buffer_find( const uint8_t* pubuffer, uint32_t uFind, size_t uEnd );

/// This struct template always evaluates to false for any type T. 
/// It's useful in SFINAE (Substitution Failure Is Not An Error) contexts to 
/// disable overloads or specializations when they should not be considered.
template<typename T> struct always_false : std::false_type {};


_GDD_END