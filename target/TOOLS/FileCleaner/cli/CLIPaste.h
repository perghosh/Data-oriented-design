/**                                                                            @TAG #ui.cli #command.paste[description:declaration for methods used for paste command]
* @file CLIPaste.h
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

std::pair<bool,std::string> Paste_g( const gd::cli::options* poptionsCount, gd::cli::options* poptionsRoot );

NAMESPACE_CLI_END


/*
@TASK #history.paste #user.per
[name: history] [priority: high] [state: open] [owner: per]
[brief: "Paste command for history"]
[description: "## paste to use clipboard and figure out command from that." ]
[sample: '
- `cleaner paste
']
[idea: "Main method for history is called `Paste_g'."]
*/