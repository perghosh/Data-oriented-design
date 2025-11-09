/**
* @file Run.h
* @brief Header file for run-related functionality and utilities.
*
* This file contains declarations for functions and classes that are responsible for managing and executing runs.
*/

#pragma once
#include <string>
#include <utility>
#include <functional>
#include <vector>


#include "gd/gd_types.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_arguments.h"

// ## @FILE [tag: where, run, expression] [description: Methods used perform actions on tables, mostly filtering]

std::pair<bool, std::string> RunExpression_g(const std::string_view& stringExpression, const gd::argument::shared::arguments& argumentsCode, const gd::table::dto::table* ptableLineList, gd::table::dto::table* ptableSnippet );
/// @brief Executes a general expression with provided arguments and tables. And filters the rows based on the expression.
std::pair<bool, std::string> RunExpression_Where_g(const std::string_view& stringExpression, gd::table::arguments::table* ptableKeyValue, std::function<void(const std::vector<uint64_t>&, const gd::table::arguments::table*)> callback_ );
/// @brief Overloaded function to execute a "where" expression without a callback.
inline std::pair<bool, std::string> RunExpression_Where_g(const std::string_view& stringExpression, gd::table::arguments::table* ptableKeyValue ) {
    return RunExpression_Where_g(stringExpression, ptableKeyValue, nullptr);
}
/// @brief Executes a "where" expression on a table, filtering rows based on the expression.
std::pair<bool, std::string> RunExpression_Where_g(const std::string_view& stringExpression, gd::table::dto::table* ptable_);
