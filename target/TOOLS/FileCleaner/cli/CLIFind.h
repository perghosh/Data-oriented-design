/**                                                                            @TAG #ui.cli #command.find [description:declaration for methods used for find]
* @file CLIFind.h
*  
* @brief Header file for CLI find operations.
* 
* Find operations are used to search for patterns in files based on all that may be in the file. 
* Compare with `List` operations, which are more focused on finding in one single line. Line based and treat files as list of rows.
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

std::pair<bool, std::string> Find_g(gd::cli::options* poptionsFind, CDocument* pdocument);
std::pair<bool, std::string> Find_g(const std::vector<std::string>& vectorSource, gd::argument::arguments* pargumentsFind, CDocument* pdocument);

std::pair<bool, std::string> MatchAllPatterns_g( const std::vector<std::string>& vectorPattern, CDocument* pdocument, int iMatchCount = -1);
std::pair<bool, std::string> SynchronizeResult_g( CDocument* pdocument );

std::pair<bool, std::string> ReadSnippet_g( const std::vector<std::string>& vectorRule, CDocument* pdocument );

/// @brief Core print method
std::pair<bool, std::string> FindPrint_g( CDocument* pdocument, const gd::argument::shared::arguments& argumentsPrint );
/// @brief Print method for snippets
std::pair<bool, std::string> FindPrintSnippet_g( CDocument* pdocument, const gd::argument::shared::arguments& argumentsPrint );
/// @brief Print method for key-value pairs
std::pair<bool, std::string> FindPrintKeyValue_g(CDocument* pdocument, const gd::argument::shared::arguments* pargumentsPrint = nullptr );
/// @brief Print method for visual studio format
std::pair<bool, std::string> FindPrintVS_g( const gd::table::dto::table& table_ );

NAMESPACE_CLI_END

