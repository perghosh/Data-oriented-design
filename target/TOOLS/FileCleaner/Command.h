/**
 * @file Command.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * 
 */

#pragma once


 // ## STL

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <filesystem>

 // ## GD

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"


#include "Application.h"

/*
int CountRowsInFile(const gd::table::dto::table& table_);

int RowCount(const std::string& stringFile);
*/


/// @brief Harvests files from the specified path and populates a table with their details.
std::pair<bool, std::string> FILES_Harvest_g(const std::string& stringPath, gd::table::dto::table* ptable_, unsigned uDepth );
/// @brief Harvests files from the specified path and populates a table with their details.
std::pair<bool, std::string> FILES_Harvest_g(const gd::argument::shared::arguments& argumentsPath, gd::table::dto::table* ptable_ );

/// @brief Counts the number of rows in a file. 
std::pair<bool, std::string> COUNT_Row(const gd::argument::shared::arguments& argumentsPath, gd::argument::shared::arguments& argumentsResult );


