/**                                                                            @TAG #ui.cli #command.config [description:declaration for methods used for config command]
* @file CLIConfig.h
* @brief Implementation file for CLI configuration operations.
*
* This file contains the implementation of functions related to CLI configuration.
* Configuration file is store in user folder for that os and in that a folder named `cleaner` should exist.
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

std::pair<bool,std::string> Configuration_g(const gd::cli::options* poptionsConfiguration);

std::pair<bool,std::string> ConfigurationCreate_g();
std::pair<bool,std::string> ConfigurationEdit_g();

NAMESPACE_CLI_END






/*
@TASK #configuration.create #user.per
[name: config] [priority: high] [state: ongoing] [assigned_to: per] [todo: "test in linux"]
[description: "## create configuration file if it doesn't exist.
For windows this file should be placed in `C:\Users\<username>\AppData\Local\cleaner\cleaner-configuration.json`.
For linux this file should be placed in `~/.local/share/cleaner/cleaner-   configuration.json`.
If configuration file exists then just print that it does exist and exit." ]

[sample: '
- `cleaner config -create` - Creates configuration file
']
[idea: "Main method for history is called `Configuration_g'."]
*/

