/**
 * @file CLIHistory.h
 *  
 */

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <filesystem>

#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"



#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

// ## History operations

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory);

/// \brief Create logic to manage hisory for cleaner, if windows or linux this differ some based on different filesystem
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate );

/// \brief Delete history file or folder
std::pair<bool, std::string> HistoryDelete_g( const gd::argument::arguments& argumentsDelete);

/// \brief Print history file
std::pair<bool, std::string> HistoryPrint_g( const gd::argument::arguments& argumentsPrint);

/// \brief get row history table
std::pair<bool, std::string> HistoryGetRow_g( const gd::argument::arguments& argumentsRow);

NAMESPACE_CLI_END
