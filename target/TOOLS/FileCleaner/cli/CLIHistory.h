/**                                                                            @FILE [tag: cli, history ] [description:declaration for methods used for history command]
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
std::pair<bool, std::string> HistoryAppend_g( std::string_view stringFile, std::string_view stringName, gd::argument::arguments* parguments, std::string_view stringSection );
inline std::pair<bool, std::string> HistoryAppend_g( std::string_view stringFile, gd::cli::options* poptionsHistory) { return HistoryAppend_g(stringFile, poptionsHistory, ""); }

/// \brief Set attributes for history entry (sets any value)
std::pair<bool, std::string> HistorySetAttributes_g(uint64_t uRow, const gd::argument::arguments& argumentsSet, CDocument* pdocument); 

/// \brief Delete history file or folder
std::pair<bool, std::string> HistoryDelete_g( const gd::argument::arguments& argumentsDelete, CDocument* pdocument);

/// \brief Edit history file
std::pair<bool, std::string> HistoryEdit_g(const gd::argument::arguments& argumentsEdit);

/// \brief Edit history file
std::pair<bool, std::string> HistoryList_g(const gd::argument::arguments& argumentsList, CDocument* pdocument);

/// \brief Remove entry or entries in history file
std::pair<bool, std::string> HistoryRemove_g(const gd::argument::arguments& argumentsRemove, CDocument* pdocument);

/// \brief Print history file
std::pair<bool, std::string> HistoryPrint_g( const gd::argument::arguments& argumentsPrint, CDocument* pdocument);

/// \brief get row history table
std::pair<bool, std::string> HistoryGetRow_g( const gd::argument::arguments& argumentsRow, CDocument* pdocument);

/// \brief Save XML file for history
std::pair<bool, std::string> HistorySave_g(const gd::argument::arguments& argumentsSave, CDocument* pdocument);

std::pair<bool, std::string> HistoryRun_g(const gd::argument::arguments& argumentsRun, const gd::cli::options* poptionsApplication, CDocument* pdocument);

/// \brief Index history entries from table
std::pair<bool, std::string> HistoryIndex_g(const gd::argument::arguments& argumentsIndex, CDocument* pdocument);

NAMESPACE_CLI_END

/*
@TASK [project: history] [task: delete row] [priority: high] [state: open] [owner: kevin] [description: Delete history row, with row index]
[sample: 'cleaner history --delete 5 ']
[summary: 1 23 4 5 6 7 8 9 10 -> cleaner history --delete 5 -> 1 2 3 4 6 7 8 9 10 1 23 4 5 6 7 8 9 10 -> cleaner history --delete 5 -> 1 2 3 4 6 7 8 9 10 ]
*/

/*
@TASK [project: history] [task: prune] [priority: high] [state: open] [owner: kevin] [description: Shrink number of history items in save, add option "prune"]
[sample: 'cleaner history --prune "50%" ']
*/

/*
@TASK [project: history] [task: pin] [priority: high] [state: open] [owner: kevin] [description: Pin history command, copy command from save to pinned]
[sample: "cleaner history --pin 23" - Pin history command with number 23 in save]
*/

/*
@TASK [project: history] [title: history printing width] [priority: high] [state: open] [owner: per] [description: Pin history command, copy command from save to pinned]
[sample: "cleaner history --pin 23" - Pin history command with number 23 in save]
*/

