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



#ifndef _GDB_BEGIN
#define _GDB_BEGIN namespace gdd {
#define _GDB_END }
_GDB_BEGIN
#else
_GDB_BEGIN
#endif

_GDB_BEGIN

// ## Buffer checks

bool buffer_find( const uint8_t* pubuffer, const std::vector<uint8_t>& vectorFind, size_t uEnd );
bool buffer_find( const uint8_t* pubuffer, uint32_t uFind, size_t uEnd );

_GDB_END