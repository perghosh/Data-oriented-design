/**
* @file CLIList.cpp
*/


#include "../Command.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


#include "CLIList.h"




NAMESPACE_CLI_BEGIN

// ## Dir operations

std::pair<bool, std::string> List_g(const gd::cli::options* poptionsList, CDocument* pdocument )
{
   const gd::cli::options& options_ = *poptionsList;

   std::string stringCommandName = options_.name();
   if( stringCommandName == "list" )
   {
      if( options_.exists("explain") == true )
      {
         //std::string stringExplain = ListGetExplain_g(( *poptionsCount )["explain"].as_string());
         //pdocument->MESSAGE_Display(stringExplain);
      }
      else
      {
         auto result_ = ListPattern_g(poptionsList, pdocument);
         if( result_.first == false ) return result_;
      }

   }


   return { true, "" };
}

/**
 * @brief Processes the 'list' command by harvesting files, applying filters, searching for patterns, and outputting results.
 *
 * This function performs the following steps:
 * 1. Prepares the source path and determines recursion settings.
 * 2. Harvests files from the specified source, optionally applying a filter.
 * 3. Applies additional file filtering if requested.
 * 4. Splits the pattern string into a vector of patterns to search for.
 * 5. Limits the number of result lines based on the 'max' option.
 * 6. Searches for the specified patterns in the harvested files, optionally restricting to a segment (e.g., code, comment, string).
 * 7. Outputs the results either to the CLI, Visual Studio output, or saves them to a file, depending on the options provided.
 *
 * @param poptionsList Pointer to a gd::cli::options object containing command-line options such as source, filter, pattern, max, segment, and output.
 * @param pdocument Pointer to a CDocument object used for file harvesting, filtering, and result output.
 * @return A std::pair where the first element is a boolean indicating success (true) or failure (false), and the second element is a string containing an error message if the operation failed, or an empty string on success.
 */
std::pair<bool, std::string> ListPattern_g(const gd::cli::options* poptionsList, CDocument* pdocument)
{                                                                                                  assert( poptionsList != nullptr );
   const gd::cli::options& options_ = *poptionsList;
   std::string stringSource = options_["source"].as_string();
   CApplication::PathPrepare_s(stringSource);                                 // if source is empty then set it to current path, otherwiss prepare it

   int iRecursive = options_["recursive"].as_int();
   if (iRecursive == 0 && options_.exists("R") == true) iRecursive = 16; // set to 16 if R is set, find all files

   gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
   std::string stringFilter = options_["filter"].as_string();

   auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);       // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
   if (result_.first == false) return result_;

   
   if( options_["filter"].is_true() == true )                                 // Apply file filters if specified
   {
      std::string stringFilter = options_["filter"].as_string();
      result_ = pdocument->FILE_Filter(stringFilter);                                              if( !result_.first ) { return result_; }
   }

   std::string stringPattern = options_["pattern"].as_string();                                    //LOG_INFORMATION_RAW("== --pattern: " & stringPattern);
   auto vectorPattern = CApplication::Split_s(stringPattern);                 // split pattern string into vector

   uint64_t uMax = options_["max"].as_uint64(); // max number of lines to be printed
   if (uMax == 0) uMax = 512; // default to 512 lines

   gd::argument::shared::arguments argumentsList({ {"max", uMax} });
   std::string stringSegment = options_["segment"].as_string(); // type of segment to search in, code, comment or string, maybe all
   if (stringSegment.empty() == false) argumentsList.set("state", stringSegment.c_str());

   result_ = pdocument->FILE_UpdatePatternList(vectorPattern, argumentsList); // count rows in harvested files
   if (result_.first == false) return result_;

   auto* ptableLineList = pdocument->CACHE_Get("file-linelist");

   auto tableResultLineList = pdocument->RESULT_PatternLineList();
   // LOG_INFORMATION_RAW("== Lines in result: " & tableResultLineList.get_row_count() & " breaks if above: " & uMax );

   std::string stringOutput = options_["output"].as_string();
   if (stringOutput.empty() == true)
   {
      std::string stringCliTable;

#ifdef _WIN32
      if (poptionsList->exists("vs") == false)
      {
         stringCliTable = gd::table::to_string(tableResultLineList, gd::table::tag_io_cli{});
         pdocument->MESSAGE_Display( stringCliTable );
      }
      else
      {
         //stringCliTable = "\n-- Result from search  --\n";
         stringCliTable = "\n";
         CDocument::RESULT_VisualStudio_s(tableResultLineList, stringCliTable);
         VS::CVisualStudio visualstudio;
         result_ = visualstudio.Connect();
         if (result_.first == true) result_ = visualstudio.Print(stringCliTable, VS::tag_vs_output{});
         if (result_.first == false)
         {
            std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
            std::cerr << stringError << "\n";
            return result_;
         }
      }
#else
      stringCliTable = gd::table::to_string(tableResultLineList, gd::table::tag_io_cli{});
      pdocument->MESSAGE_Display( stringCliTable );
#endif // _WIN32
   }
   else
   {
      auto result_ = pdocument->RESULT_Save({ {"type", "LIST"}, {"output", stringOutput} }, &tableResultLineList);
      if (result_.first == false) return result_;
   }

   return { true, "" };
}



NAMESPACE_CLI_END

