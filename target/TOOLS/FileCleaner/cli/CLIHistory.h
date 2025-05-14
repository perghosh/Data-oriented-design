/**
 * @file CLIHistory.h
 *  
 */

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>

#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"



#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

// ## History operations

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory);

/// \brief Create logic to manage hisory for cleaner, if windows or linux this differ some based on different filesystem
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate );

NAMESPACE_CLI_END
