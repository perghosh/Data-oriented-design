/**
 * @file gd_parse_format_string.h
 * @brief 
 * 
 * Parse logic for know formats
 */


#pragma once

#include "gd/gd_compiler.h"

#include <array>
#include <cassert>
#include <cstring>
#if GD_COMPILER_HAS_CPP20_SUPPORT
#include <span>
#endif
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_FORMAT_STRING_BEGIN
#  define _GD_PARSE_FORMAT_STRING_BEGIN namespace gd { namespace parse { namespace format {
#  define _GD_PARSE_FORMAT_STRING_END } } }
#endif

_GD_PARSE_FORMAT_STRING_BEGIN

_GD_PARSE_FORMAT_STRING_END
