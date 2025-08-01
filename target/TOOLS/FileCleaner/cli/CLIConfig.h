/**                                                                            @TAG #ui.cli #command.config [description:declaration for methods used for config command]
* @file CLIConfig.h
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



#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

std::pair<bool,std::string> Configuration_g(const gd::cli::options* poptionsConfiguration);

std::pair<bool,std::string> CreateConfiguration();

NAMESPACE_CLI_END






/*
@TASK #configuration.create #user.per
[name: config] [priority: high] [state: open] [assigned_to: per]
[description: "## create configuration file if it doesn't exist.
For windows this file should be placed in `C:\Users\<username>\AppData\Local\cleaner\configuration.json`.
For linux this file should be placed in `~/.local/share/cleaner/configuration.json`.
If configuration file exists then just print that it does exist and exit." ]

[sample: '
- `cleaner config -create` - Creates configuration file
']
[idea: "Main method for history is called `Configuration_g'."]
*/

/*
@TASK #configuration.load #user.per
[name: config] [priority: high] [state: open] [assigned_to: per]
[description: "## load configuration file if it exists.
For windows this file should be placed in `C:\Users\<username>\AppData\Local\cleaner\configuration.json`.
For linux this file should be placed in `~/.local/share/cleaner/configuration.json`.
" ]
[idea: "In start, try to find configuration file and load it if found, this is done in the `CApplication` class'."]
*/
