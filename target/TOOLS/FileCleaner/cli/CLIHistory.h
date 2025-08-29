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

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory, gd::cli::options* poptionsApplication, CDocument* pdocument);

/// \brief Create logic to manage hisory for cleaner, if windows or linux this differ some based on different filesystem
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate, CDocument* pdocument);

/// \brief Append entry to history file
std::pair<bool, std::string> HistoryAppend_g( std::string_view stringFile, gd::cli::options* poptionsHistory, std::string_view stringSection );
inline std::pair<bool, std::string> HistoryAppend_g( std::string_view stringFile, gd::cli::options* poptionsHistory) { return HistoryAppend_g(stringFile, poptionsHistory, ""); }

/// \brief Delete history file or folder
std::pair<bool, std::string> HistoryDelete_g( const gd::argument::arguments& argumentsDelete);

/// \brief Remove entry or entries in history file
std::pair<bool, std::string> HistoryRemove_g(const gd::argument::arguments& argumentsRemove, CDocument* pdocument);

/// \brief Print history file
std::pair<bool, std::string> HistoryPrint_g( const gd::argument::arguments& argumentsPrint, CDocument* pdocument);

/// \brief get row history table
std::pair<bool, std::string> HistoryGetRow_g( const gd::argument::arguments& argumentsRow, CDocument* pdocument);

/// \brief Save XML file for history
std::pair<bool, std::string> HistorySave_g(const gd::argument::arguments& argumentsSave, CDocument* pdocument);

std::pair<bool, std::string> HistoryRun_g(const gd::argument::arguments& argumentsRun, gd::cli::options* poptionsApplication, CDocument* pdocument);

/// \brief Edit history file
std::pair<bool, std::string> HistoryEdit_g( const gd::argument::arguments& argumentsSave );

NAMESPACE_CLI_END

/*
@TASK [name: history] [task: delete row] [priority: high] [state: open] [assigned_to: kevin] [description: Delete history row, with row index]
[sample: 'cleaner history --delete 5 ']
*/

/*
@TASK [name: history] [task: prune] [priority: high] [state: open] [assigned_to: kevin] [description: Shrink number of history items in save, add option "prune"]
[sample: 'cleaner history --prune "50%" ']
*/

/*
@TASK [name: history] [task: pin] [priority: high] [state: open] [assigned_to: kevin] [description: Pin history command, copy command from save to pinned]
[sample: "cleaner history --pin 23" - Pin history command with number 23 in save]
*/



