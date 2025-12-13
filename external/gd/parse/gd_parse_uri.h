// @FILE {tag: uri, read} [summary: URI parsing and manipulation utilities] [type: header]

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

#include "../gd_types.h"
#include "../gd_arguments.h"


#ifndef _GD_PARSE_URI_BEGIN
#  define _GD_PARSE_URI_BEGIN namespace gd { namespace parse { namespace uri {
#  define _GD_PARSE_URI_END } } }
#endif

_GD_PARSE_URI_BEGIN
std::pair<bool, std::string> parse( std::string_view stringUri, gd::argument::arguments& argumentsUri );

_GD_PARSE_URI_END
