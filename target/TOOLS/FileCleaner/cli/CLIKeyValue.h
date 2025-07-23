/**
* @file CLIRun.h
*  
*/

/* 
@TASK[date:250723][name: task-list]
[description:"""Decide a good state list for tasks:
Sample one - todo, doing, stuck, done, canceled, etc.
Sample two - backlog, ready, in development, waiting, in review, deployed, archived.
"""] [state: todo] [priority:high]
*/

/*
@TASK[date:250723][name: task-list]
[description:"""Decide a good priority list for tasks:
Sample one - critical, high, medium, low.
Sample two - urgent, important, normal, low.
"""] [state: todo] [priority:low]

@TASK[date:250723][name: feature-list]
[description:"""Decide a good feature list:
Sample one - must, should, could.
Sample two - high, medium, low.
"""] [state: todo] [priority:low]

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

#include "../Application.h"

#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> KeyValue_g(const gd::cli::options* poptionsKeyValue, CDocument* pdocument); // updated parameter types
std::pair<bool, std::string> KeyValue_g(const std::vector<std::string>& vectorSource, const gd::argument::arguments* pargumentsKeyValue, CDocument* pdocument);

NAMESPACE_CLI_END
