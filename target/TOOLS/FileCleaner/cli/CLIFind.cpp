/**
* @file CLIFind.cpp
*/

// @TAG #cli #find

#include <cstdint>
#include <format>

#include "../Command.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


#include "CLIFind.h"




NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> Find_g(const gd::cli::options* poptionsFind, CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);assert(poptionsFind != nullptr);
   const gd::cli::options& options_ = *poptionsFind; // get the options from the command line arguments

   auto vectorSourceToPrepare = options_.get_all("source");                            // get all source arguments, this is used to find files in the source directory

   std::vector<std::string> vectorSource; // vector of source paths

   // ## Source preparation
   //    if source is one then check if it may contain multiple sources that are separated by ':'
   if( vectorSourceToPrepare.size() == 1 )
   {
      std::string stringSource = vectorSourceToPrepare[0].as_string();
      CApplication::PreparePath_s(stringSource, ':');                         // if source is empty then set it to current path, otherwise prepare it
      vectorSource = CApplication::Split_s(stringSource, ':');                // split source string into vector of sources
   }
   else if( vectorSourceToPrepare.size() > 1 )
   {
      for( const auto& source : vectorSourceToPrepare )
      {
         std::string stringSource = source.as_string();
         CApplication::PreparePath_s(stringSource, 0);                       // if source is empty then set it to current path, otherwise prepare it
         vectorSource.push_back(stringSource);                                // add prepared source to vector
      }
   }
   else
   {
      std::string stringSource;
      CApplication::PreparePath_s(stringSource, 0);
      vectorSource.push_back(stringSource);
   }

   // ## Ignore patterns
   std::string stringIgnore = options_["ignore"].as_string();
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }

   const gd::argument::arguments* pargumentsFind = &options_.get_arguments(); // get the arguments from the command line options
   auto result_ = Find_g(vectorSource, pargumentsFind, pdocument);            // find files in the source directory based on the find arguments
   if( result_.first == false ) return result_;                               // if find failed, return the error

   return { true, "" }; 
}

std::pair<bool, std::string> Find_g( const std::vector<std::string>& vectorSource, const gd::argument::arguments* pargumentsFind, CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);assert(pargumentsFind != nullptr);
   const gd::argument::arguments& options_ = *pargumentsFind; // get the options from the command line arguments

   int iRecursive = options_["recursive"].as_int();
   if (iRecursive == 0 && options_.exists("R") == true) iRecursive = 16;      // set to 16 if R is set, find all files

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" || stringFilter == "." || stringFilter == "**" )   // if filter is set to * or . or ** then we want all files, so clear the filter and deep recursion
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( iRecursive == 0 ) iRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

   pdocument->GetApplication()->UpdateApplicationState();                     // update the application state to reflect the current state of the application

   // ## Harvest files from the source paths
   for( const auto& stringSource : vectorSource )
   {
      gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
      auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);    // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if (result_.first == false) return result_;
   }

   if( options_.exists("rpattern") == true )
   {
      auto vectorRPattern = options_.get_argument_all("rpattern", gd::types::tag_view{}); // get all regex patterns
      std::vector<std::string> vectorPattern; // store regex patterns as strings
      for( auto& rpattern : vectorRPattern ) { vectorPattern.push_back(rpattern.as_string()); }
      auto uSearchPatternCount = vectorPattern.size(); // count the number of patterns to search for
      std::vector< std::pair<std::regex, std::string> > vectorRegexPattern;   // vector of regex patterns and their string representation

      // ## convert string to regex and put it into vectorRegexPatterns

      gd::argument::shared::arguments argumentsFind; // prepare arguments for the file update

      for( auto& stringPattern : vectorPattern )
      {
         try
         {
            std::regex regexPattern(stringPattern);
            vectorRegexPattern.push_back({ regexPattern, stringPattern });
         }
         catch (const std::regex_error& e)
         {                                                                      
            std::string stringError = "Invalid regex pattern: '" + stringPattern + "'. Error: " + e.what();
            return { false, stringError };
         }
      }

      auto result_ = pdocument->FILE_UpdatePatternFind(vectorRegexPattern, &argumentsFind); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false) return result_;

      /*
      if( options_["match-all"].is_true() == true )
      {
         result_ = ListMatchAllPatterns_g( vectorRegexPattern, pdocument );
         if (result_.first == false) return result_;
      }
      */
   }



   return { true, "" }; 
}

NAMESPACE_CLI_END
