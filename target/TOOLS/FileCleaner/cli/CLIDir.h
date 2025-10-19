/**                                                                            @CODE [tag: cli, command] [description: declaration for dir methods]
* @file CLIDir.h
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

std::pair<bool, std::string> Dir_g(const gd::cli::options* poptionsDir, CDocument* pdocument);
std::pair<bool, std::string> DirPattern_g( const std::string& stringSource, const std::vector<std::string>& vectorPattern, const gd::argument::shared::arguments& arguments_, CDocument* pdocument );
std::pair<bool, std::string> DirPattern_g( const std::string& stringSource, const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPattern, const gd::argument::shared::arguments& arguments_, CDocument* pdocument );
std::pair<bool, std::string> DirFilter_g( const std::string& stringSource, const gd::argument::shared::arguments& arguments_, CDocument* pdocument );
std::pair<bool, std::string> DirFilter_g(const std::string& stringSource, const std::string& stringFilter, unsigned uDepth, CDocument* pdocument );

std::pair<bool, std::string> DirPrint_g( CDocument* pdocument );
std::pair<bool, std::string> DirPrintCompact_g( CDocument* pdocument, const gd::argument::shared::arguments& arguments_ );


NAMESPACE_CLI_END