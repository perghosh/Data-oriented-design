// @FILE [tag: cli, http] [description: Handle http configuration from terminal] [type: header] [name: CLIHttp.h]

/**
 * @file CLIHttp.h
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

#include "../Document.h"

#pragma once

#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

// ## Http operations

std::pair<bool, std::string> Http_g(const gd::cli::options* poptionsCopy, CDocument* pdocument);


NAMESPACE_CLI_END