/**
 * @file CLIPaste.cpp
 * @brief Implementation file for CLI paste operations.
 */

#include "../Command.h"
#include "../Application.h"

#include "CLIPaste.h"

NAMESPACE_CLI_BEGIN 

// count  --source "C:\dev\home\DOD\external\gd" -R --sort count --stats "sum"


std::pair<bool, std::string> Paste_g( const gd::cli::options* poptionsPaste, gd::cli::options* poptionsRoot )
{
   std::string stringCommandLine;

   auto result_ = OS_ReadClipboard_g( stringCommandLine );                                         if( result_.first == false )  { return result_; }

   if( stringCommandLine.empty() == false )
   {                                                                                               LOG_DEBUG_RAW("== Paste command line: " & stringCommandLine);
      papplication_g->PrintMessage("> Paste command line: " + stringCommandLine, gd::argument::arguments()); // print the command line from clipboard
      poptionsRoot->clear();                                                  // clear the options root to prepare for new command
      poptionsRoot->set_first( 0 );
      result_ = poptionsRoot->parse_terminal(stringCommandLine);                       // parse the command line from clipboard
      if( result_.first == false ) { return result_; }

      result_ = papplication_g->Initialize( *poptionsRoot);                   // initialize the application with parsed options
      if( result_.first == false ) { return result_; }
   }


   return { true, "" };
}

NAMESPACE_CLI_END