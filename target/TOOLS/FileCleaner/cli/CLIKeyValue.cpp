/**
 * @file CLIKeyValue.cpp
 */


#include "../Command.h"

#include "CLI_Shared.h"


#include "CLIKeyValue.h"

NAMESPACE_CLI_BEGIN 

std::pair<bool, std::string> KeyValue_g(const gd::cli::options* poptionsKeyValue, CDocument* pdocument)
{
   const gd::cli::options& options_ = *poptionsKeyValue; // get the options from the command line arguments

   std::vector<std::string> vectorSource = SHARED_GetSourcePaths(options_); // prepare the source paths, this is a shared function that prepares the source paths based on the command line options

   // ## Ignore patterns
   std::string stringIgnore = options_["ignore"].as_string();
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }

   // ## Call the more generic Find_g function, this is to make it possible to call Find_g with different environments, like CLI or GUI

   const gd::argument::arguments* pargumentsKeyValue = &options_.get_arguments(); // get the arguments from the command line options
   auto result_ = KeyValue_g(vectorSource, pargumentsKeyValue, pdocument);        // find files in the source directory based on the find arguments
   if( result_.first == false ) return result_;                               // if find failed, return the error

   return { true, "" }; 
}

std::pair<bool, std::string> KeyValue_g(const std::vector<std::string>& vectorSource, const gd::argument::arguments* pargumentsKeyValue, CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);assert(pargumentsKeyValue != nullptr);
   const gd::argument::arguments& options_ = *pargumentsKeyValue; // get the options from the command line arguments

   uint64_t uPatternCount = 0; // count of patterns to search for

   int iRecursive = options_["recursive"].as_int();
   if (iRecursive == 0 && options_.exists("R") == true) iRecursive = 16;      // set to 16 if R is set, find all files

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" || stringFilter == "." || stringFilter == "**" )   // if filter is set to * or . or ** then we want all files, so clear the filter and deep recursion
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( iRecursive == 0 ) iRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

   pdocument->GetApplication()->UpdateApplicationState();                     // update the application state to reflect the current state of the application

   gd::argument::shared::arguments argumentsFind; // prepare arguments for the file update

   if( options_.exists("max") == true )
   {
      uint64_t uMax = options_["max"].as_uint64();
      argumentsFind.append("max", uMax);                                      // set the maximum number of matches to find
   }

   if( options_.exists("segment") == true ) { argumentsFind.append("segment", options_["segment"].as_string()); }

   // ## Harvest files from the source paths
   for( const auto& stringSource : vectorSource )
   {
      gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
      auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);    // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if (result_.first == false) return result_;
   }

   if( options_.exists("pattern") == true )
   {
      auto vector_ = options_.get_argument_all("pattern", gd::types::tag_view{}); // get all patterns
      std::vector<std::string> vectorPattern; // store patterns as views
      std::vector<std::string> vectorPatternString;                           // store patterns as strings

      for( auto& pattern_ : vector_ ) {  vectorPattern.push_back(pattern_.as_string()); } // convert views to strings

      if( vectorPattern.size() == 1 )
      { 
         auto stringPattern = vectorPattern[0];
         vectorPattern = CApplication::Split_s(stringPattern, ';'); }

      for( auto& pattern_ : vectorPattern ) { vectorPatternString.push_back(pattern_); }

      if( vectorPatternString.size() == 1 ) 
      { 
         // check for empty pattern, if empty then try to read from clipboard
         if( vectorPatternString[0].empty() == true )
         {
            std::string stringPattern;
            OS_ReadClipboard_g( stringPattern );
            if( stringPattern.empty() == true ) { pdocument->MESSAGE_Display( std::format( "Use clipboard: {}", stringPattern ) ); }
            vectorPatternString[0] = std::move(stringPattern);                // move the string from clipboard to the pattern vector
         }
      }

      // remove empty patterns
      vectorPatternString.erase(std::remove_if(vectorPatternString.begin(), vectorPatternString.end(), [](const std::string& str) { return str.empty(); }), vectorPatternString.end());

      uPatternCount = vectorPatternString.size();                             // count the number of patterns to search for
      if( uPatternCount == 0 ) return { false, "No patterns provided." };     // if no patterns are provided, return an error

      auto result_ = pdocument->FILE_UpdatePatternFind(vectorPatternString, &argumentsFind); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false) return result_;

      if( options_["match-all"].is_true() == true )
      {
         result_ = SHARED_MatchAllPatterns_g( vectorPattern, pdocument );
         if (result_.first == false) return result_;
      }

   }
   else if( options_.exists("rpattern") == true )
   {
      auto vectorRPattern = options_.get_argument_all("rpattern", gd::types::tag_view{}); // get all regex patterns
      std::vector<std::string> vectorPattern; // store regex patterns as strings
      for( auto& rpattern : vectorRPattern ) { vectorPattern.push_back(rpattern.as_string()); }

      if( vectorPattern.size() == 1 ) 
      { 
         // check for empty pattern, if empty then try to read from clipboard
         if( vectorPattern[0].empty() == true )
         {
            std::string stringPattern;
            OS_ReadClipboard_g( stringPattern );
            if( stringPattern.empty() == true ) { pdocument->MESSAGE_Display( std::format( "Use clipboard: {}", stringPattern ) ); }
            vectorPattern[0] = std::move(stringPattern);                // move the string from clipboard to the pattern vector
         }
      }

      // remove empty patterns
      vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
      if( vectorPattern.size() == 0 ) return {false, "No regex patterns provided."}; // if no patterns are provided, return an error

      uPatternCount = vectorPattern.size(); // count the number of patterns to search for
      if( uPatternCount == 0 ) return { false, "No patterns provided." };     // if no patterns are provided, return an error

      std::vector< std::pair<boost::regex, std::string> > vectorRegexPattern;   // vector of regex patterns and their string representation

      // ## convert string to regex and put it into vectorRegexPatterns

      for( auto& stringPattern : vectorPattern )
      {
         try
         {
            boost::regex regexPattern(stringPattern);
            vectorRegexPattern.push_back({ regexPattern, stringPattern });
         }
         catch (const boost::regex_error& e)
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
