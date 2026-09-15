/** @FILE [tag: cli, command, log] [description:implementation for CLI log operations] [type: source]
 * @file CLILog.cpp
 * @brief Implementation file for CLI log operations.
 */

#include <cstdint>
#include <format>

#include "../Command.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif

#include "CLI_Shared.h"

#include "CLILog.h"

NAMESPACE_CLI_BEGIN

// count  --source "C:\dev\home\DOD\external\gd" -R --sort count --stats "sum"


std::pair<bool, std::string> Log_g(gd::cli::options* poptionsLog, CDocument* pdocument)
{
   const gd::cli::options& options_ = *poptionsLog;

#ifndef NDEBUG
   [[maybe_unused]] std::string string_d = gd::argument::debug::print(poptionsLog->get_arguments());
#endif // NDEBUG


   if(options_.exists("clip") == true && options_["clip"].is_true() == true) // test for clip argument
   {
      std::string stringFile;
      OS_ReadClipboard_g(stringFile);                                          // read clipboard content

      // ### Check if clipboard content is a valid file path and informtion user if found
      if(stringFile.empty() == false && std::filesystem::exists(stringFile) == true)
      {
         pdocument->MESSAGE_Display(std::format("File from clipboard as source: {}", stringFile));
         poptionsLog->set_value("source", stringFile);                       // set source to the file from clipboard
      }
   }

   std::string stringCommandName = options_.name();
   if(stringCommandName == "log")
   {
      auto result_ = LogPattern_g(poptionsLog, pdocument);
      if(result_.first == false) return result_;
   }

   return { true, "" };
}

/*
std::pair<bool, std::string> SHARED_Harvest(const gd::argument::arguments& argumentsHarvest, CDocument* pdocument)
{
   std::string stringFilter = argumentsHarvest["filter"].as_string();
   std::string stringSource = argumentsHarvest["source"].as_string();
   auto result_ = pdocument->FILE_Harvest(argumentsHarvest);                  // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
   if(result_.first == false) return result_;
}
*/

std::pair<bool, std::string> LogPattern_g(gd::cli::options* poptionsLog, CDocument* pdocument)
{
   assert(poptionsLog != nullptr); assert(pdocument != nullptr);
   size_t uSearchPatternCount = 0; // count of patterns to search for
   const gd::cli::options& options_ = *poptionsLog;

   gd::argument::arguments argumentsFileHarvest;
   SHARED_ReadHarvestSetting_g(options_, argumentsFileHarvest, pdocument);

   int iRecursive = argumentsFileHarvest["depth"].as_int();

   std::string stringSource = argumentsFileHarvest["source"].as_string();
   CApplication::PreparePath_s(stringSource);                                 // if source is empty then set it to current path, otherwise prepare it

   std::string stringFilter = options_["filter"].as_string();

   gd::argument::shared::arguments argumentsPath({ { "source", stringSource },{ "recursive", iRecursive } });
   std::string stringPathFilter = options_["path-filter"].as_string();
   if(stringPathFilter.empty() == false) argumentsPath.append("path-filter", stringPathFilter);
   auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);       // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
   if(result_.first == false) return result_;

   if(options_["filter"].is_true() == true)                                   // Apply file filters if specified
   {
      std::string stringFilter = options_["filter"].as_string();
      // If the filter is empty, we do not apply any filter and remove files that do not match the filter
      result_ = pdocument->FILE_Filter(stringFilter);                                              if(!result_.first) { return result_; }
   }

   gd::argument::shared::arguments argumentsList;
   argumentsList.append(options_.get_arguments(), { "max", "match-all", "icase", "word" });
   if(argumentsList.exists("max") == false) { argumentsList.set("max", 512); } // default to 512 lines

   if(options_.exists("pattern") == true)
   {
      std::vector<std::string> vectorPattern; // vector to store patterns
      vectorPattern = options_.get_arguments().get_all<std::string>("pattern"); // get all patterns from options and put them into vectorPattern
      if(vectorPattern.size() == 1)
      {
         auto stringPattern = vectorPattern[0];
         if(stringPattern.empty() == true)                                  // if pattern is empty, read from clipboard
         {
            OS_ReadClipboard_g(stringPattern);
            if(stringPattern.empty() == true) { pdocument->MESSAGE_Display(std::format("Use clipboard: {}", stringPattern)); }
            vectorPattern.push_back(stringPattern);
         }
         else { vectorPattern = CApplication::Split_s(stringPattern, ';'); }
      }

      vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
      if(vectorPattern.size() == 0) return { false, "No patterns provided." }; // if no patterns are provided, return an error

      uSearchPatternCount = vectorPattern.size();                              // count the number of patterns to search for

      result_ = pdocument->FILE_UpdatePatternLog(vectorPattern, argumentsList); // Search for patterns in harvested files and place them into the result table

   }


}


NAMESPACE_CLI_END
