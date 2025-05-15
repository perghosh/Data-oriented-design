/**
* @file CLICount.h
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
#include "../Application.h"


#ifndef NAMESPACE_CLI_BEGIN

#  define NAMESPACE_CLI_BEGIN namespace CLI {
#  define NAMESPACE_CLI_END  }

#endif

NAMESPACE_CLI_BEGIN

// ## Count operations

std::pair<bool, std::string> Count_g( const gd::cli::options* poptionsCount, CDocument* pdocument );

std::pair<bool, std::string> CountLine_g(const gd::cli::options* poptionsCount, CDocument* pdocument);

/// \brief Get explain for selected count result
std::string CountGetExplain_g( const std::string_view& stringType );

/*
inline std::string CountGetExplain_g( const gd::cli::options* poptionsCount ) {                       assert( poptionsCount != nullptr );
   return CountGetExplain_g( ( *poptionsCount )["information"].as_string() );
}
*/

NAMESPACE_CLI_END