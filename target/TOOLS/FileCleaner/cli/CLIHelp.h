/** @FILE [tag: cli, command, help] [description:implementation for CLI help operations] [type: source]
* @file CLIHelp.h
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

#include "../Document.h"

#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> Help_g(CDocument* pdocument, const gd::cli::options* poptionsHelp, gd::cli::options* poptionsApplication);

/// \brief Print help for specific command
std::pair<bool, std::string> HelpCommand_g(std::string_view stringCommand, bool bLLM, std::string& stringDocumentation);

NAMESPACE_CLI_END
