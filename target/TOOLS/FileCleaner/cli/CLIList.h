/**                                                                            @TAG #ui.cli #command.list [description:declaration for methods used for list command]
 * @file CLIList.h
 *  
 * @brief Header file for CLI list operations. "list" command passes arguments to these methods.
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

#include "../Document.h"

#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

// ## Dir operations

std::pair<bool, std::string> List_g(gd::cli::options* poptionsList, CDocument* pdocument);
std::pair<bool, std::string> ListPattern_g(const gd::cli::options* poptionsList, CDocument* pdocument);
std::pair<bool, std::string> ListMatchAllPatterns_g( const std::vector<std::string>& vectorPattern, CDocument* pdocument, int iMatchCount = -1);
std::pair<bool, std::string> ListMatchAllPatterns_g(const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPattern, CDocument* pdocument, int iMatchCount = -1);
// std::pair<bool, std::string> ListRPattern_g(const gd::cli::options* poptionsList, CDocument* pdocument);

/// Print found lines in list command
std::pair<bool, std::string> ListPrintLine_g( CDocument* pdocument, gd::argument::arguments& argumentsPrint );

NAMESPACE_CLI_END