/*
* @brief `gd_utf8_2.h` iextends methods found in `gd_utf8.hpp`
* 
## Overview
| Name | Description |
| - | - |


*/

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "gd_utf8.hpp"
#include "gd_arguments.h"
#include "gd_variant.h"
#include "gd_variant_view.h"


_GD_UTF8_BEGIN

void split( const std::string_view& stringText, const std::string_view& stringSplit, std::vector<gd::variant>& vectorPart, gd::variant_type::enumType eDefaultType );

inline void split( const std::string_view& stringText, const std::string_view& stringSplit, std::vector<gd::variant>& vectorPart ) { split( stringText, stringSplit, vectorPart, gd::variant_type::eTypeUtf8String ); }
inline void split( const std::string_view& stringText, std::vector<gd::variant>& vectorPart ) { split( stringText, std::string_view{","}, vectorPart, gd::variant_type::eTypeUtf8String); }
std::vector<std::string> split( const std::string_view& stringText, const std::string_view& stringSplit );

namespace regex {
   int64_t find( const std::string_view& stringText, const std::string_view& stringFind );
   void replace( std::string& stringText, const std::string_view& stringMatch, const std::string_view& stringInsert );
}

_GD_UTF8_END