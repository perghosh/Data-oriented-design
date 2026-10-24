// @FILE [tag: compiler] [description: Compiler detection and feature support] [type: header]

/**
* @file gd_compiler.h
* @brief Provide macros to identify the compiler and its C++ standard support.
*
* It includes:
* - Macros to detect the compiler being used (GCC, Clang, MSVC).
* - Checks for C++17, C++20, C++23, and C++26 language modes.
* - Platform-specific constants to identify the target platform.
*
* Supported compilers:
* - GCC, Clang, and MSVC, using the language-version macro reported by the compiler.
*
* These macros indicate the selected language mode; they do not guarantee support for every
* feature in that C++ standard.
*/

#pragma once

#ifdef __APPLE__
#  ifndef __has_extension
#    define __has_extension(x) 0
#  endif
#  include <TargetConditionals.h>
#  if (defined(TARGET_OS_OSX) && TARGET_OS_OSX == 1) || \
      (defined(TARGET_OS_MAC) && TARGET_OS_MAC == 1)
#    define GD_COMPILER_PLATFORM_MAC
#  elif (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1)
#    define GD_COMPILER_PLATFORM_IPHONE
#  endif

#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define GD_COMPILER_PLATFORM_LINUX

#elif defined(WIN32) || defined(__WIN32__) || defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#  define GD_COMPILER_PLATFORM_WINDOWS

#  if defined( WINAPI_FAMILY ) && ( WINAPI_FAMILY == WINAPI_FAMILY_APP )
#      define GD_COMPILER_PLATFORM_WINDOWS_UWP
#  endif

#elif defined(__ORBIS__) || defined(__PROSPERO__)
#  define GD_COMPILER_PLATFORM_PLAYSTATION

#endif

#ifdef __cplusplus

#  if (__cplusplus >= 201703L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#    define GD_COMPILER_HAS_CPP17_SUPPORT 1
#  endif

#  if (__cplusplus >= 202002L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#    define GD_COMPILER_HAS_CPP20_SUPPORT 1
#  endif

#  if (__cplusplus >= 202302L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202302L)
#    define GD_COMPILER_HAS_CPP23_SUPPORT 1
#  endif

#  if (__cplusplus >= 202400L) || (defined(_MSVC_LANG) && _MSVC_LANG >= 202400L)
#    define GD_COMPILER_HAS_CPP26_SUPPORT 1
#  endif

#endif
