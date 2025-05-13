/**
* @file CLIHistory.cpp
*/


#include "../Application.h"

#include "CLICount.h"




NAMESPACE_CLI_BEGIN

// ## History operations

std::pair<bool, std::string> Count_g(const gd::cli::options* poptionsHistory)
{
   return { true, "" };
}


/**
* @brief 
*/
std::string CountGetExplain_g( const std::string_view& stringType )
{
   std::string stringHelp;
   if( stringType == "count-lines" )
   {
      stringHelp = 
R"(
Count lines in file/files.
   columns:
      - folder - The name of the folder containing the file.
      - filename - The name of the file being analyzed.
      - count - Total number of lines in the file.
      - code - Number of lines containing actual code (excluding comments and whitespace).
      - characters - Total count of code characters (excluding comments and strings).
      - comment - The number of comment segments (not lines—counts).
      - string - The number of string segments (not lines—counts).

)";
   }

   return stringHelp;
}

NAMESPACE_CLI_END