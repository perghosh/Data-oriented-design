/**
 * @file Command.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * 
 */

// @TAG #command.refactor

#pragma once


 // ## STL

#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <regex>


 // ## GD

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/expression/gd_expression.h"
#include "gd/expression/gd_expression_parse_state.h"
#include "gd/parse/gd_parse_match_pattern.h"


#include "Application.h"

/*
int CountRowsInFile(const gd::table::dto::table& table_);

int RowCount(const std::string& stringFile);
*/




/// @brief Harvests files from the specified path and populates a table with their details.
std::pair<bool, std::string> FILES_Harvest_g(const std::string& stringPath, const std::string& stringWildcard, gd::table::dto::table* ptable_, unsigned uDepth, bool bSize = false );
/// @brief Harvests files from the specified path and populates a table with their details.
std::pair<bool, std::string> FILES_Harvest_g(const gd::argument::shared::arguments& argumentsPath, gd::table::dto::table* ptable_ );

/// @brief Read lines from a file, starting at the specified row and offset, and returning the specified number of lines.
std::pair<bool, std::string> FILES_ReadLines_g( const std::string& stringPath, uint64_t uRow, int64_t iOffset, uint64_t uCount, std::string& stringLines, int64_t* piLeadingLineCount = nullptr );

/// @brief Counts the number of rows in a file. 
std::pair<bool, std::string> COMMAND_CountRows(const gd::argument::shared::arguments& argumentsPath, gd::argument::shared::arguments& argumentsResult );
std::pair<bool, std::string> COMMAND_CollectFileStatistics(const gd::argument::shared::arguments& argumentsPath, gd::argument::shared::arguments& argumentsResult );
std::pair<bool, std::string> COMMAND_CollectPatternStatistics(const gd::argument::shared::arguments& argumentsPath, const std::vector<std::string>& vectorPattern, std::vector<uint64_t>& vectorCount );
/// @brief Collects the number of lines in a file that match a specific pattern, this is case sensetive matching.
std::pair<bool, std::string> COMMAND_ListLinesWithPattern(const gd::argument::shared::arguments& argumentsPath, const gd::parse::patterns& patternsFind, gd::table::dto::table* ptable_ );
/// @brief Collects the number of lines in a file that match a specific pattern, this use regex matching.
std::pair<bool, std::string> COMMAND_ListLinesWithPattern(const gd::argument::shared::arguments& argumentsPath, const std::vector< std::pair<std::regex, std::string> >& vectorRegexPatterns, gd::table::dto::table* ptable_);

std::pair<bool, std::string> TABLE_AddSumRow(gd::table::dto::table* ptable_, const std::vector<unsigned>& vectorColumnIndex);

std::pair<bool, std::string> TABLE_RemoveZeroRow(gd::table::dto::table* ptable_, const std::vector<unsigned>& vectorColumnIndex);

/// @brief Filters the table based on the expression provided for a specific column. To keep the row, the expression must evaluate to true.
std::pair<bool, std::string> EXPRESSION_FilterOnColumn_g( gd::table::dto::table* ptable_, unsigned uColumn, const std::vector<std::string> vectorExpression );


