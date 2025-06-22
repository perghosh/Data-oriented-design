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
      auto uCount = CApplication::PreparePath_s(stringSource, ':');           // if source is empty then set it to current path, otherwise prepare it
      if( uCount == 1 ) vectorSource.push_back(stringSource);                 // if there is only one source then add it to the vector
      else if( uCount > 1 )  vectorSource = CApplication::Split_s(stringSource, ':');// if there are multiple sources then split them by ':'
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

   // ## Call the more generic Find_g function, this is to make it possible to call Find_g with different environments, like CLI or GUI

   const gd::argument::arguments* pargumentsFind = &options_.get_arguments(); // get the arguments from the command line options
   auto result_ = Find_g(vectorSource, pargumentsFind, pdocument);            // find files in the source directory based on the find arguments
   if( result_.first == false ) return result_;                               // if find failed, return the error

   if( options_.exists("print") == false || options_["print"].is_true() == true )  // default is to print result
   {
      gd::argument::shared::arguments argumentsPrint({ { "pattern-count", uint64_t(2u) } }); // hardcode pattern count to 2 for printing results and allways print patterns
      if( options_.exists("context") == true ) argumentsPrint.append("context", options_["context"].as_string_view()); // if context is set, add it to the print arguments

      // check if vs flag is set, if so then print to Visual Studio output
      if( options_.exists("vs") == true ) { argumentsPrint.append("vs", true); }

      FindPrint_g(pdocument, argumentsPrint); // Print the results of the find operation
   }


   return { true, "" }; 
}

/** ---------------------------------------------------------------------------
 * @brief Finds files based on the provided source paths and find arguments.
 *
 * This function searches for files in the specified source paths, applies regex patterns if provided,
 * and prints the results of the find operation.
 *
 * @param vectorSource A vector of source paths to search for files.
 * @param pargumentsFind The arguments containing options such as recursive search, filter, and regex patterns.
 * @param pdocument Pointer to the CDocument instance where the results will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> Find_g( const std::vector<std::string>& vectorSource, const gd::argument::arguments* pargumentsFind, CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);assert(pargumentsFind != nullptr);
   const gd::argument::arguments& options_ = *pargumentsFind; // get the options from the command line arguments

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


   // ## Harvest files from the source paths
   for( const auto& stringSource : vectorSource )
   {
      gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
      auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);    // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if (result_.first == false) return result_;
   }

   if( options_.exists("pattern") == true )
   {
      auto vectorPattern = options_.get_argument_all("pattern", gd::types::tag_view{}); // get all patterns
      std::vector<std::string> vectorPatternString;                           // store patterns as strings
      for( auto& pattern : vectorPattern ) { vectorPatternString.push_back(pattern.as_string()); }
      
      uPatternCount = vectorPatternString.size();                             // count the number of patterns to search for
      if( uPatternCount == 0 ) return { false, "No patterns provided." };     // if no patterns are provided, return an error

      auto result_ = pdocument->FILE_UpdatePatternFind(vectorPatternString, &argumentsFind); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false) return result_;
   }
   if( options_.exists("rpattern") == true )
   {
      auto vectorRPattern = options_.get_argument_all("rpattern", gd::types::tag_view{}); // get all regex patterns
      std::vector<std::string> vectorPattern; // store regex patterns as strings
      for( auto& rpattern : vectorRPattern ) { vectorPattern.push_back(rpattern.as_string()); }
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

std::pair<bool, std::string> FindPrint_g( CDocument* pdocument, const gd::argument::shared::arguments& argumentsPrint )
{                                                                                                  assert(pdocument != nullptr);
   std::string stringCliTable; // string to hold the CLI table output
   size_t uSearchPatternCount = argumentsPrint.get_argument<uint64_t>("pattern-count", 1u); // count of patterns to search for

   int64_t iContextOffset = 0, iContextCount = 0; // variables used to bring context to found code

   if( argumentsPrint.exists("context") == true )
   {
      std::string stringContext = argumentsPrint["context"].as_string();       // context is in the format offset,count like -2,6 to get lines from 2 before and 6 lines

      // parse context to numbers
      auto vectorOffsetCount = CApplication::SplitNumber_s( stringContext );   // get offset value and count
      size_t position_; // not used
      if( vectorOffsetCount.size() == 1 )
      {
         iContextCount = std::stoll(vectorOffsetCount[0].data(), &position_);
      }
      else if( vectorOffsetCount.size() > 1 )
      {
         iContextOffset = std::stoll(vectorOffsetCount[0].data(), &position_);
         iContextCount = std::stoll(vectorOffsetCount[1].data(), &position_);
      }

      iContextOffset = iContextOffset % 100;                                  // limit the offset to 100 lines, so we do not get too much context
      iContextCount = iContextCount % 1000;                                   // limit the count to 1000 lines, so we do not get too much context
   }


   gd::argument::arguments argumentsOption( { { "pattern-count", (unsigned)uSearchPatternCount } } );
   if( iContextOffset != 0 || iContextCount != 0 ) { argumentsOption.append( "offset", iContextOffset ); argumentsOption.append( "count", iContextCount ); }
   auto tableResultLineList = pdocument->RESULT_PatternLineList( argumentsOption );// generate the result table for pattern line list

   auto uRowCount = tableResultLineList.get_row_count(); // get the number of rows in the result table

   if( iContextCount == 0 )
   {
      // ## Just print the "line" column 
      gd::table::dto::table table_(0, { {"rstring", 0, "line"} }, gd::table::tag_prepare{});
      table_.plant(tableResultLineList, "line", 0, tableResultLineList.get_row_count() ); // plant the table into the result table
      stringCliTable = gd::table::to_string(table_, gd::table::tag_io_raw{});
   }
   else
   {
      // ## Print the "line" with context
      gd::table::dto::table table_(0, { {"rstring", 0, "line"} }, gd::table::tag_prepare{});
      for( auto itRow : tableResultLineList )
      {
         std::string stringLine = itRow.cell_get_variant_view("line").as_string(); // get the line text
         stringLine += "\n";                                                  // add a newline to the line text
         std::string stringContext = itRow.cell_get_variant_view("context").as_string(); // get the context code
         gd::utf8::indent(stringContext, "-- ");                              // indent the context code by 3 spaces
         auto uRow = table_.row_add_one();

         // ## mark the line with that has the matched pattern

         auto uLeadingRow = itRow.cell_get_variant_view( "row-leading" ).as_uint();
         auto piPosition = gd::ascii::strchr(stringContext, '\n', uLeadingRow);// find the leading row in the context code
         if( piPosition != nullptr && piPosition[1] != 0 && piPosition[2] != 0 ) // if the leading row is found
         {
            auto uIndex = piPosition - stringContext.data();                  // get the index of the leading row
            if( uLeadingRow > 0 ) uIndex++;                                   // if the leading row is greater than 0, then we have a leading row, so increment the index past new line character is needed
            stringContext[uIndex] = '>';                                      // mark the leading row with a '>' character
            uIndex++;
            stringContext[uIndex] = '>';                                      // mark the leading row with a '>' character
         }

         stringLine += stringContext;                                         // add the context code to the line text

         table_.cell_set(uRow, "line", stringLine);                           // set the line text in the result table
      }

      stringCliTable = gd::table::to_string(table_, gd::table::tag_io_raw{});
   }

   pdocument->MESSAGE_Display( stringCliTable );                              // display the result table to the user
   // Print number of lines found
   std::string stringMessage = std::format("\nFound {} lines", uRowCount);
   pdocument->MESSAGE_Display(stringMessage);                                 // display the number of lines found

#ifdef _WIN32
   if( argumentsPrint.exists("vs") == true ) // if vs flag is set, then we want to print to Visual Studio output
   {
      stringCliTable.clear();                                                 // clear the stringCliTable, we will use it to print to Visual Studio output
      CDocument::RESULT_VisualStudio_s(tableResultLineList, stringCliTable);
      VS::CVisualStudio visualstudio;
      auto result_ = visualstudio.Connect();
      if(result_.first == true) result_ = visualstudio.Print(stringCliTable, VS::tag_vs_output{});
      if(result_.first == false)
      {
         std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
         pdocument->MESSAGE_Display(stringError);
      }
      else
      {
         std::string stringPrint = std::format("Printed to Visual Studio output: {} rows", tableResultLineList.get_row_count());
         pdocument->MESSAGE_Display(stringPrint);
      }
   }
#endif // _WIN32

   return { true, "" }; 
}

NAMESPACE_CLI_END
