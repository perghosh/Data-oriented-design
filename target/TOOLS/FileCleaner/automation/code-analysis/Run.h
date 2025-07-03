/**
* @file Run.h
* @brief Header file for run-related functionality and utilities.
*
* This file contains declarations for functions and classes that are responsible for managing and executing runs.
*/

#pragma once
#include <string>
#include <utility>
#include <vector>


#include "gd/gd_types.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"



std::pair<bool, std::string> RunExpression_g(const std::string_view& stringExpression, const gd::argument::shared::arguments& argumentsCode, const gd::table::dto::table* ptableLineList, gd::table::dto::table* ptableSnippet );