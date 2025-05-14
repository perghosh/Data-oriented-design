/**
 * @file gd_compiler.h
 * @brief This header file provides macros to identify the compiler and its support for the C++20 standard.
 *
 * It includes:
 * - Macros to detect the compiler being used (GCC, Clang, MSVC).
 * - Checks for C++20 standard support based on the compiler version.
 * - Platform-specific constants to identify the target platform.
 *
 * Supported compilers:
 * - GCC (GNU Compiler Collection) version 10 or higher for C++20 support.
 * - Clang version 10 or higher for C++20 support.
 * - MSVC (Microsoft Visual C++) version 1928 or higher for C++20 support.
 *
 * This file ensures compatibility and feature detection for projects targeting C++20.
 */


#pragma once

/**
 * Platform-specific constants to identify the target platform.
 * These macros define the platform on which the code is being compiled.
 * Supported platforms include:
 * - macOS
 * - iPhone
 * - Linux
 * - Windows
 * - PlayStation
 */
#ifdef __APPLE__

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

#elif defined(__ORBIS__) || defined(__PROSPERO__)

#  define GD_COMPILER_PLATFORM_PLAYSTATION

#endif



/**
 * C++ Compiler Detection Macros
 *
 * This header defines macros to identify the compiler (GCC, Clang, MSVC) and check
 * for C++20 standard support. These macros allow conditional compilation based on
 * compiler type and C++20 compatibility, ensuring portable code across different
 * compilers and versions.
 *
 * Macro Descriptions:
 * - GD_COMPILER_IS_GCC: Identifies GNU Compiler Collection (GCC). Defined when
 *   __GNUC__ is present but neither __clang__ nor _MSC_VER are defined, ensuring
 *   exclusion of Clang (which also defines __GNUC__) and MSVC.
 * - GD_COMPILER_IS_CLANG: Identifies Clang compiler. Defined when __clang__ is
 *   present, as provided by Clang.
 * - GD_COMPILER_IS_MSVC: Identifies Microsoft Visual C++ (MSVC). Defined when
 *   _MSC_VER is present, as provided by MSVC.
 * - GD_COMPILER_IS_CPP20: Checks if the C++ standard is C++20 or later. True when
 *   __cplusplus >= 202002L, the standard value for C++20 as defined by the C++
 *   Standard.
 * - GD_COMPILER_GCC_CPP20_SUPPORT: Checks if GCC supports C++20. True when the
 *   compiler is GCC and the version (__GNUC__) is 10 or higher, as GCC 10 introduced
 *   full C++20 support.
 * - GD_COMPILER_CLANG_CPP20_SUPPORT: Checks if Clang supports C++20. True when the
 *   compiler is Clang and the major version (__clang_major__) is 10 or higher, as
 *   Clang 10 introduced full C++20 support.
 * - GD_COMPILER_MSVC_CPP20_SUPPORT: Checks if MSVC supports C++20. True when the
 *   compiler is MSVC and the version (_MSC_VER) is 1928 or higher, corresponding to
 *   Visual Studio 2019 version 16.8 or later, which introduced full C++20 support.
 * - GD_COMPILER_HAS_CPP20_SUPPORT: Combines checks to confirm C++20 support. True
 *   when the C++ standard is C++20 or later (GD_COMPILER_IS_CPP20) and the compiler
 *   version supports C++20 (GCC, Clang, or MSVC specific checks).
 * - GD_COMPILER_BELOW_CPP20: Indicates the compiler does not support C++20. True
 *   when GD_COMPILER_HAS_CPP20_SUPPORT is false, useful for fallback code paths.
 *
 * Why These Macros:
 * These macros ensure robust detection of the compiler and its C++20 support by
 * leveraging standard predefined macros (__GNUC__, __clang__, _MSC_VER, __cplusplus)
 * and version checks. The version thresholds (GCC 10, Clang 10, MSVC 1928) are chosen
 * based on when each compiler achieved full C++20 compliance. The prefix
 * "GD_COMPILER_" avoids naming conflicts in larger projects. These macros enable
 * conditional compilation to adapt code for C++20 features or provide fallbacks for
 * older standards, improving portability across GCC, Clang, and MSVC.
 *
 * Usage:
 * Include this header to use the macros in conditional compilation, e.g.:
 *   #if GD_COMPILER_HAS_CPP20_SUPPORT
 *       // Use C++20 features
 *   #elif GD_COMPILER_BELOW_CPP20
 *       // Fallback for older standards
 *   #endif
 */


// Compiler identification
#define GD_COMPILER_IS_GCC   defined(__GNUC__) && !defined(__clang__) && !defined(_MSC_VER)
#define GD_COMPILER_IS_CLANG defined(__clang__)
#define GD_COMPILER_IS_MSVC  defined(_MSC_VER)

// C++20 standard check
#define GD_COMPILER_IS_CPP20 (__cplusplus >= 202002L)

// Compiler version checks for C++20 support
#define GD_COMPILER_GCC_CPP20_SUPPORT   (IS_GCC && __GNUC__ >= 10)
#define GD_COMPILER_CLANG_CPP20_SUPPORT (IS_CLANG && __clang_major__ >= 10)
#define GD_COMPILER_MSVC_CPP20_SUPPORT  (IS_MSVC && _MSC_VER >= 1928)

// Check if the compiler supports C++20
#define GD_COMPILER_HAS_CPP20_SUPPORT (IS_CPP20 && (GCC_CPP20_SUPPORT || CLANG_CPP20_SUPPORT || MSVC_CPP20_SUPPORT))

// Check if the compiler is below C++20
#define GD_COMPILER_BELOW_CPP20 (!HAS_CPP20_SUPPORT)
