/**
 * @file CLIHistory.cpp
 */

#include "gd/expression/gd_expression_runtime.h"


#include "../Application.h"

#include "CLIHistory.h"




NAMESPACE_CLI_BEGIN

// ## History operations

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory)
{
   return { true, "" };
}


/**
 * @brief Set up folders for cleaner and creates history file if not exists
 */
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate )
{
   auto result_ = papplication_g->CreateDirectory();                                               if( result_.first == false ) { return result_; }

   return { true, "" };
}

NAMESPACE_CLI_END