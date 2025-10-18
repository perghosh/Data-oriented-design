/**
 * @file gd_table_formater.h
 *
 * @brief 
 *
 *
 *
 *
 *
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include "../gd_types.h"
#include "../gd_variant.h"
#include "../gd_variant_view.h"
#include "../gd_table.h"
#include "../gd_table_column-buffer.h"
#include "../gd_table_table.h"


#ifndef _GD_TABLE_BEGIN
#define _GD_TABLE_BEGIN namespace gd { namespace table {
#define _GD_TABLE_END } }
#endif

_GD_TABLE_BEGIN

namespace format
{ 

void to_string( const gd::table::dto::table& table_, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, gd::types::tag_card );

inline std::string to_string(const gd::table::dto::table& table_, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, gd::types::tag_card) {
    std::string stringResult;
    to_string(table_, uBegin, uCount, vectorColumn, uBoxWidth, uTotalWidth, argumentOption, stringResult, gd::types::tag_card{});
    return stringResult;
}

inline std::string to_string(const gd::table::dto::table& table_, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, gd::types::tag_card) {
   std::string stringResult;
   to_string(table_, 0, table_.size(), vectorColumn, uBoxWidth, uTotalWidth, argumentOption, stringResult, gd::types::tag_card{});
   return stringResult;
}

inline std::string to_string(const gd::table::dto::table& table_, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, gd::types::tag_card) {
   std::string stringResult;
   to_string(table_, 0, table_.size(), vectorColumn, uBoxWidth, uTotalWidth, {}, stringResult, gd::types::tag_card{});
   return stringResult;
}

std::string to_string(const gd::table::dto::table& table_, const std::vector<unsigned>& vectorColumn, unsigned uBoxCount, const gd::argument::arguments& argumentOption, gd::types::tag_card);

} // namespace format

_GD_TABLE_END
