/**
 * @file CLI_Shared.h
 * 
 * @brief Header file for shared CLI operations.
 *  
 */

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <filesystem>
#include <iostream>

#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"

#include "../Application.h"

#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

/// Reads harvest settings to configure how files are harvested based on command line options.
std::pair<bool, std::string> SHARED_ReadHarvestSetting_g( const gd::cli::options& options_, gd::argument::shared::arguments& arguments_, CDocument* pdocument);

/// Generates a vector of source paths based on the provided command line options.
std::vector<std::string> SHARED_GetSourcePaths( const gd::cli::options& options_ );
/// match all patterns in the vectorPattern against the file lines in the document
std::pair<bool, std::string> SHARED_MatchAllPatterns_g(const std::vector<std::string>& vectorPattern, CDocument* pdocument, int iMatchCount = -1 );

/// Open file with its associated application.
std::pair<bool, std::string> SHARED_OpenFile_g(const std::string_view& stringFile);

/// Get the width of the terminal window.
int SHARED_GetTerminalWidth();

NAMESPACE_CLI_END


// @TASK #options.folder #user.per [name: options] [description: "## When cleaner is executed from root folder but only should search within some specific sub folders make sure that it is possible to find those folders even if is only the foler name"]