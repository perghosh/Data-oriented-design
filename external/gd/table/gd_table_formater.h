// @FILE [tag: table, format, string] [description: Formats a table for different styles, like card style with box drawing.] [type: header] [name: gd_table_formater.h]

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

   /** ------------------------------------------------------------------------
 * @brief Formats a table as a string with box drawing for card style.
 *
 * This function formats a portion of the given table as a string, using box drawing characters
 * to create a "card" style layout. The output is appended to the provided string.
 *
 * @param table_           The table to format.
 * @param uBegin           The starting row index (inclusive).
 * @param uCount           The number of rows to format.
 * @param vectorColumn     The indices of columns to include in the output.
 * @param uBoxWidth        The width of each box (column).
 * @param uTotalWidth      The total width of the formatted table.
 * @param argumentOption   Additional formatting options.
 * @param stringOut        The output string to append the formatted table to.
 * @param tag_card         Tag to select card style formatting.
 */
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

/** ---------------------------------------------------------------------------
 * @brief Format table as string with box drawing for card style, this overload formats entire table
 * @param table_ table to format
 * @param vectorColumn columns to include
 * @param uBoxWidth width of each box
 * @param uTotalWidth total width of the table
 * @return formatted table as string
 */
inline std::string to_string(const gd::table::dto::table& table_, const std::vector<unsigned>& vectorColumn, unsigned uBoxWidth, unsigned uTotalWidth, gd::types::tag_card) {
   std::string stringResult;
   to_string(table_, 0, table_.size(), vectorColumn, uBoxWidth, uTotalWidth, {}, stringResult, gd::types::tag_card{});
   return stringResult;
}

/// @brief Format table as string with box drawing for card style
std::string to_string(const gd::table::dto::table& table_, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, gd::types::tag_card);

/// @brief Format table as string with box drawing for card style, this overload uses column names or indexes for columns to include
inline std::string to_string(const gd::table::dto::table& table_, uint64_t uBegin, uint64_t uCount, const std::vector< std::variant<std::string_view, unsigned> >& vectorColumn, unsigned uBoxCount, const gd::argument::arguments& argumentOption, gd::types::tag_card) {
   std::vector<unsigned> vectorColumnIndex;
   for( const auto& column_ : vectorColumn ) {
      if( std::holds_alternative<unsigned>( column_ ) ) { vectorColumnIndex.push_back( std::get<unsigned>( column_ ) ); }
      else
      {
         const auto& stringName = std::get<std::string_view>( column_ );
         unsigned uIndex = table_.column_get_index(stringName);
         vectorColumnIndex.push_back( uIndex );
      }
   }
   return to_string(table_, uBegin, uCount, vectorColumnIndex, uBoxCount, argumentOption, gd::types::tag_card{});
}

/// @brief Format table as string with box drawing for card style, this overload uses column names or indexes for columns to include and formats entire table
inline std::string to_string(const gd::table::dto::table& table_, const std::vector< std::variant<std::string_view, unsigned> >& vectorColumn, unsigned uTotalWidth, const gd::argument::arguments& argumentOption, gd::types::tag_card) {
   return to_string(table_, 0, table_.size(), vectorColumn, uTotalWidth, argumentOption, gd::types::tag_card{});
}


} // namespace format

_GD_TABLE_END
