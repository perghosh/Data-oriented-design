/**                                                                            @TAG #ui.cli #command.copy [description: declaration for dir methods]
 * @file CLICopy.h
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

// ## Copy operations

std::pair<bool, std::string> Copy_g(const gd::cli::options* poptionsCopy, CDocument* pdocument);

/// @brief Copies files from source to target folder while preserving directory structure.
std::pair<bool, std::string> CopyFiles_g( const std::string& stringSource, const std::string& stringTarget, const gd::argument::shared::arguments& arguments_, CDocument* pdocument);


NAMESPACE_CLI_END