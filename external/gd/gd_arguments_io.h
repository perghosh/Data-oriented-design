// @FILE [tag: arguments, print, output] [description: Generate output from tables in different formats] [type: header] [name: gd_arguments_io.h]

/**
 * \file gd_arguments_io.h
 * 
 * \brief "io" means that argument data are streamed from or to other storage items
 * 
 * 
 * 
 * 
 * 
 */

 #pragma once

#include <cassert>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include "gd_types.h"
#include "gd_utf8.hpp"
#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_arguments.h"


#if defined( __clang__ )
   #pragma GCC diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wreorder-ctor"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 6387 26495 26812 )
#endif

_GD_ARGUMENT_BEGIN

/// tag dispatcher used for json formatting
struct tag_io_json {};

/// tag dispatcher used for uri formatting
struct tag_io_uri {};

/// tag dispatcher used for yaml formatting
struct tag_io_yaml {};

// ## JSON IO -----------------------------------------------------------------

void to_string( const arguments& arguments_, std::string& stringOut, tag_io_json );

// ## URI IO ------------------------------------------------------------------

void to_string( const arguments& arguments_, std::string& stringOut, tag_io_uri );

// ## YAML IO -----------------------------------------------------------------

/// Generate yaml formated string from arguments object
void to_string( const arguments& arguments_, std::string& stringOut, tag_io_yaml );

_GD_ARGUMENT_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
