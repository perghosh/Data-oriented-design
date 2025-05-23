/**
* @file CLIList.h
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

std::pair<bool, std::string> List_g(const gd::cli::options* poptionsList, CDocument* pdocument);
std::pair<bool, std::string> ListPattern_g(const gd::cli::options* poptionsList, CDocument* pdocument);
// std::pair<bool, std::string> ListRPattern_g(const gd::cli::options* poptionsList, CDocument* pdocument);

NAMESPACE_CLI_END