// @FILE {tag: parse, uri, read} [summary: URI parsing and manipulation utilities] [type: header] [name: gd_parse_uri.h]

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
#include "../gd_arguments_shared.h"


#ifndef _GD_PARSE_JSON_BEGIN
#  define _GD_PARSE_JSON_BEGIN namespace gd { namespace parse { namespace json {
#  define _GD_PARSE_JSON_END } } }
#endif

_GD_PARSE_JSON_BEGIN

std::pair<bool, std::string> parse_shallow_object_g( std::string_view stringJson, gd::argument::arguments& argumentsJson );
std::pair<bool, std::string> parse_shallow_object_g( std::string_view stringJson, gd::argument::shared::arguments& argumentsJson );

_GD_PARSE_JSON_END
