/**                                                                            @TAG #ui.cli #command.history [description:declaration for methods used for history command]
 * @file CLIHistory.h
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

// ## History operations

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory, CDocument* pdocument);

/// \brief Create logic to manage hisory for cleaner, if windows or linux this differ some based on different filesystem
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate, CDocument* pdocument);

/// \brief Delete history file or folder
std::pair<bool, std::string> HistoryDelete_g( const gd::argument::arguments& argumentsDelete);

/// \brief Remove entry or entries in history file
std::pair<bool, std::string> HistoryRemove_g(const gd::argument::arguments& argumentsRemove);

/// \brief Print history file
std::pair<bool, std::string> HistoryPrint_g( const gd::argument::arguments& argumentsPrint, CDocument* pdocument);

/// \brief get row history table
std::pair<bool, std::string> HistoryGetRow_g( const gd::argument::arguments& argumentsRow, CDocument* pdocument);

std::pair<bool, std::string> HistorySave_g(const gd::argument::arguments& argumentsSave, CDocument* pdocument);

/// \brief Edit history file
std::pair<bool, std::string> HistoryEdit_g();

NAMESPACE_CLI_END


/*
@TASK #history.list #user.kevin
[name: history] [priority: high] [state: open] [assigned_to: kevin]
[description: "## list history items.
Items in history are stored in a file, to make it possible to use previous commands.
List history items is needed to view what has been done before, and to make it possible to repeat commands." ]
[sample: '
- `cleaner history -list` - lists all history items
- `cleaner history -list --filter "count"` - lists all history "count" items
- `cleaner history -list --filter "find"` - lists all history "find" items
']
[idea: "Main method for history is called `History_g'. If the flag `-list` is set, then call method `ReadHistory_s`into history table stored in document cache.
When history is read, apply filter if any was sent and the print. Method to print should be called `HistoryPrint_g`"]
*/

/*
@TASK #history.create #user.kevin
[name: history] [priority: high] [state: open] [assigned_to: kevin]
[description: "## create history file. 
   For windows this file should be placed in `C:\Users\<username>\AppData\Local\cleaner\history.xml`.
   For linux this file should be placed in `~/.local/share/cleaner/history.xml`.
   If history files exists then just print that it does exist and exit." ]
[sample: """- `cleaner history -create` - Creates history file"""]
[idea: "Call method `CreateHistory_s'. If the flag `-create` is set and create the history file"]
*/

/*
@TASK #history.create #user.kevin [name:create local history file]
[description:"   
- `cleaner history -create -local
- This will create a local history file in the current folder.
"]
*/



/*
@TASK #history.add #user.kevin [name:add command to history]
[description:"   
   - Add global option command called `--history` that marks if the command should be added to history.
   - If `--history` is set then: save the command string to hisory file and the value set for --history is the alias name.
"]
[state:open]   
*/

/*
@TASK #history.load #user.kevin [name:load history]
[description:"   
- Loading history should try to check if there are any history files in the current folder or parent folder to current folder.
- - If local history file is found then load it into the document cache.
- - If no local history file is found then try to load the global history file.
"]
[state:open]   
*/

/*
@TASK #history.backup #user.kevin
[name: history] [priority: high] [state: open] [assigned_to: kevin]
[description: "## create backup of history file
if No folder than place it it in same folder as the history file are located, if full path is given then place it in that folder." ]
[sample: '
- `cleaner history --backup "history-file-name"` - Creates backup of history file
']
[idea: "Call method `BackupHistory_s'. If the option `--backup` is set"]
*/

/*
@TASK #history.prune #user.kevin
[name: history] [priority: high] [state: open] [assigned_to: kevin]
[description: "## Prune history file.
This will remove entries from the history file based on a given filter.
" ]
[sample: '
- `cleaner history --prune "50%"` - Prunes history file to keep only the last 50% of entries
- `cleaner history --prune "100"` - Prunes history file to keep only the last 100 entries
- `cleaner history --prune "list"` - Prunes all list entries from the history file
']
[idea: "Call method `HistoryPrune_g`. If the option `--prune` is set, prune is an advanced command so it will need some logic and filtering."]
*/

/*
@TASK #history.run #user.kevin
[name: history] [priority: high] [state: open] [assigned_to: kevin]
[description: "## Run history command.
This will execute a history command based on a given filter.
" ]
[sample: '
- `cleaner history --run "23"` - Runs history command with number 23
- `cleaner history --run "alias-name"` - Runs history that have alias name
- `cleaner history --run "-"` - Runs last history command
']
*/

/*
@TASK #history.pin #user.kevin
[name: history] [priority: high] [state: open] [assigned_to: kevin]
[description: "## Pin history command.
This will pin a history command based on a given filter. Pinning a command means that it will not be pruned or removed from the history file.
" ]
[sample: '
- `cleaner history --pin "23"`- Pins history command with number 23
- `cleaner history --pin "alias-name"` - Pins history that have alias name
']
*/


