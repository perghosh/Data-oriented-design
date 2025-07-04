/**
* @file CLIList.cpp
*/

// @TAG #cli #list

#include <cstdint>
#include <format>

#include "../Command.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


#include "CLIList.h"




NAMESPACE_CLI_BEGIN

// ## Dir operations

/** --------------------------------------------------------------------------- @TAG #cli #list
 * @brief Processes the 'list' command and performs file harvesting and pattern matching.
 * 
 * This function checks if the command name is 'list' and if the 'explain' option is set. If so, it would display an explanation (currently commented out).
 * If the 'explain' option is not set, it calls the ListPattern_g function to perform file harvesting and pattern matching.
 *
 * @param poptionsList Pointer to a gd::cli::options object containing command-line options.
 * @param pdocument Pointer to a CDocument object used for file harvesting and pattern matching.
 * @return A std::pair where the first element is a boolean indicating success (true) or failure (false), and the second element is a string containing an error message if the operation failed, or an empty string on success.
 */
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

/** ---------------------------------------------------------------------------
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
{                                                                                                  assert( poptionsList != nullptr ); assert( pdocument != nullptr );
   size_t uSearchPatternCount = 0; // count of patterns to search for
   const gd::cli::options& options_ = *poptionsList;
   std::string stringSource = options_["source"].as_string();
   CApplication::PreparePath_s(stringSource);                                 // if source is empty then set it to current path, otherwiss prepare it

   std::string stringIgnore = options_["ignore"].as_string();
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }


   int iRecursive = options_["recursive"].as_int();
   if (iRecursive == 0 && options_.exists("R") == true) iRecursive = 16; // set to 16 if R is set, find all files

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" || stringFilter == "." || stringFilter == "**" )   // if filter is set to * or . or ** then we want all files, so clear the filter and deep recursion
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( iRecursive == 0 ) iRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

   pdocument->GetApplication()->UpdateApplicationState();                     // update the application state to reflect the current state of the application

   gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
   auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);       // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
   if (result_.first == false) return result_;

   
   if( options_["filter"].is_true() == true )                                 // Apply file filters if specified
   {
      std::string stringFilter = options_["filter"].as_string();
      // If the filter is empty, we do not apply any filter and remove files that do not match the filter
      result_ = pdocument->FILE_Filter(stringFilter);                                              if( !result_.first ) { return result_; }
   }

   uint64_t uMax = options_["max"].as_uint64(); // max number of lines to be printed
   if (uMax == 0) uMax = 512; // default to 512 lines

   gd::argument::shared::arguments argumentsList({ {"max", uMax} });
   std::string stringSegment = options_["segment"].as_string(); // type of segment to search in, code, comment or string, maybe all
   if (stringSegment.empty() == false) argumentsList.set("state", stringSegment.c_str());

   // ## check for pattern that 
   if( options_.exists("pattern") == true )
   {
      std::vector<std::string> vectorPattern; // vector to store patterns
      auto vector_ = options_.get_all("pattern"); // get all patterns from options and put them into argumentsList
      if( vector_.size() == 1 )
      {
         auto stringPattern = vector_[0].as_string();
         if( stringPattern.empty() == true )                                  // if pattern is empty, read from clipboard
         {
            OS_ReadClipboard_g( stringPattern );
            if( stringPattern.empty() == true ) { pdocument->MESSAGE_Display( std::format( "Use clipboard: {}", stringPattern ) ); }
            vectorPattern.push_back(stringPattern); 
         }
         else { vectorPattern = CApplication::Split_s(stringPattern, ';'); }
      }
      // put all patterns into vectorPattern
      else { for( auto& pattern : vector_ ) { vectorPattern.push_back(pattern.as_string()); } }

      // remove empty patterns
      vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
      if( vectorPattern.size() == 0 ) return {false, "No patterns provided."}; // if no patterns are provided, return an error

      uSearchPatternCount = vectorPattern.size();                              // count the number of patterns to search for
      result_ = pdocument->FILE_UpdatePatternList(vectorPattern, argumentsList); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false) return result_;

      if( options_["match-all"].is_true() == true )
      {
         result_ = ListMatchAllPatterns_g( vectorPattern, pdocument );
         if (result_.first == false) return result_;
      }
   }
   else if( options_.exists("rpattern") == true )
   {
      auto vectorRPattern = options_.get_all("rpattern"); // get all regex patterns
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

      uSearchPatternCount = vectorPattern.size(); // count the number of patterns to search for
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

      result_ = pdocument->FILE_UpdatePatternList(vectorRegexPattern, argumentsList); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false) return result_;

      if( options_["match-all"].is_true() == true )
      {
         result_ = ListMatchAllPatterns_g( vectorRegexPattern, pdocument );
         if (result_.first == false) return result_;
      }
   }
   else { return { false, "No pattern specified" }; }                          // no pattern specified

   // ## context handling, if context is specified then we will bring some context to the found code

   int64_t iContextOffset = 0, iContextCount = 0; // variables used to bring context to found code

   if( options_.exists("context") == true )
   {
      std::string stringContext = options_["context"].as_string();             // context is in the format offset,count like -2,6 to get lines from 2 before and 6 lines

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

   auto* ptableLineList = pdocument->CACHE_Get("file-linelist");

   // ## check for expression to do some preprocessing on the result table

   if( options_["expression"].is_true() == true )
   {
      std::string stringExpression = options_["expression"].as_string();
      if( stringExpression.empty() == false )
      {
         std::vector<std::string> vectorExpression;
         vectorExpression.push_back(stringExpression);                        // put the expression into a vector
         result_ = EXPRESSION_FilterOnColumn_g(ptableLineList, ptableLineList->column_get_index("line"), vectorExpression); // filter the result table based on the expression
         if(result_.first == false) return result_;
      }
   }

   gd::argument::arguments argumentsOption( { { "pattern-count", (unsigned)uSearchPatternCount } } );
   if( iContextOffset != 0 || iContextCount != 0 ) { argumentsOption.append( "offset", iContextOffset ); argumentsOption.append( "count", iContextCount ); }
   auto tableResultLineList = pdocument->RESULT_PatternLineList( argumentsOption );// generate the result table for pattern line list


   std::string stringOutput = options_["output"].as_string();
   if (stringOutput.empty() == true)
   {
      std::string stringCliTable;

#ifdef _WIN32
      if (poptionsList->exists("vs") == false)
      {
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
               stringLine += "\n";                                            // add a newline to the line text
               std::string stringContext = itRow.cell_get_variant_view("context").as_string(); // get the context code
               gd::utf8::indent(stringContext, "-- ");                         // indent the context code by 3 spaces
               auto uRow = table_.row_add_one();

               // ## mark the line with that has the matched pattern

               auto uLeadingRow = itRow.cell_get_variant_view( "row-leading" ).as_uint();
               auto piPosition = gd::ascii::strchr(stringContext, '\n', uLeadingRow);// find the leading row in the context code
               if( piPosition != nullptr && piPosition[1] != 0 && piPosition[2] != 0 ) // if the leading row is found
               {
                  auto uIndex = piPosition - stringContext.data();            // get the index of the leading row
                  if( uLeadingRow > 0 ) uIndex++;                             // if the leading row is greater than 0, then we have a leading row, so increment the index past new line character is needed
                  stringContext[uIndex] = '>';                                // mark the leading row with a '>' character
                  uIndex++;
                  stringContext[uIndex] = '>';                                // mark the leading row with a '>' character
               }

               stringLine += stringContext;                                   // add the context code to the line text

               table_.cell_set(uRow, "line", stringLine);                     // set the line text in the result table
            }

            stringCliTable = gd::table::to_string(table_, gd::table::tag_io_raw{});
         }
         
         pdocument->MESSAGE_Display( stringCliTable );
      }
      else
      {                                                                        // @TAG #script
         if( options_["script"].is_true() == true )                            
         {
            std::string stringScript = options_["script"].as_string();
            VS::CVisualStudio visualstudio;
            result_ = visualstudio.Connect();
            if( result_.first == true )
            {
               visualstudio.AddTable( &tableResultLineList );
               result_ = visualstudio.ExecuteExpression( stringScript );
            }
            if( result_.first == false ) return result_;
         }


         //stringCliTable = "\n-- Result from search  --\n";
         stringCliTable = "\n";
         CDocument::RESULT_VisualStudio_s(tableResultLineList, stringCliTable);
         VS::CVisualStudio visualstudio;
         result_ = visualstudio.Connect();
         if(result_.first == true) result_ = visualstudio.Print(stringCliTable, VS::tag_vs_output{});
         if(result_.first == false)
         {
            std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
            std::cerr << stringError << "\n";
            return result_;
         }
         else
         {
            std::string stringPrint = std::format("Printed to Visual Studio output: {} rows", tableResultLineList.get_row_count());
            std::cerr << stringPrint << "\n";
         }
      }
#else
      // ## Just print the "line" column 
      gd::table::dto::table table_(0, { {"rstring", 0, "line"} }, gd::table::tag_prepare{});
      table_.plant(tableResultLineList, "line", 0, tableResultLineList.get_row_count() ); // plant the table into the result table

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
            stringLine += "\n";                                            // add a newline to the line text
            std::string stringContext = itRow.cell_get_variant_view("context").as_string(); // get the context code
            gd::utf8::indent(stringContext, "-- ");                         // indent the context code by 3 spaces
            auto uRow = table_.row_add_one();

            // ## mark the line with that has the matched pattern

            auto uLeadingRow = itRow.cell_get_variant_view( "row-leading" ).as_uint();
            auto piPosition = gd::ascii::strchr(stringContext, '\n', uLeadingRow);// find the leading row in the context code
            if( piPosition != nullptr && piPosition[1] != 0 && piPosition[2] != 0 ) // if the leading row is found
            {
               auto uIndex = piPosition - stringContext.data();            // get the index of the leading row
               if( uLeadingRow > 0 ) uIndex++;                             // if the leading row is greater than 0, then we have a leading row, so increment the index past new line character is needed
               stringContext[uIndex] = '>';                                // mark the leading row with a '>' character
               uIndex++;
               stringContext[uIndex] = '>';                                // mark the leading row with a '>' character
            }

            stringLine += stringContext;                                   // add the context code to the line text

            table_.cell_set(uRow, "line", stringLine);                     // set the line text in the result table
         }

         stringCliTable = gd::table::to_string(table_, gd::table::tag_io_raw{});
      }
      pdocument->MESSAGE_Display( stringCliTable );
#endif // _WIN32

      // Print number of lines found
      std::string stringMessage = std::format("Found {} lines", tableResultLineList.get_row_count() );
      pdocument->MESSAGE_Display(stringMessage);
   }
   else
   {
      auto result_ = pdocument->RESULT_Save({ {"type", "LIST"}, {"output", stringOutput} }, &tableResultLineList);
      if (result_.first == false) return result_;
   }

   return { true, "" }; // return success
}

std::pair<bool, std::string> ListMatchAllPatterns_g(const std::vector<std::string>& vectorPattern, CDocument* pdocument, int iMatchCount )
{                                                                                                  assert( pdocument != nullptr ); assert( vectorPattern.size() > 0 ); // at least one pattern must be specified
   std::vector<uint64_t> vectorRowDelete; // vector of row numbers to delete

   auto ptableLineList = pdocument->CACHE_Get("file-linelist"); // get the file line list from the cache

   if( iMatchCount == -1 ) iMatchCount = (int)vectorPattern.size();                     // if iMatchCount is -1 then set it to the size of the vectorPattern, so we check all patterns

   // ## iterate over all rows in table and check the line text for all patterns
   for( size_t uRow = 0; uRow < ptableLineList->get_row_count(); ++uRow )
   {
      std::string_view stringLineText = ptableLineList->cell_get_variant_view(uRow, "line").as_string_view(); // get the line text
      int iMatch = iMatchCount;                                               // number of patterns to match, if -1 then match all
      
      for( const auto& stringPattern : vectorPattern )                        // check if all patterns match the line text
      {
         if( stringLineText.find(stringPattern) != std::string::npos )        // if pattern is not found in line text
         {
            iMatch--;                                                         // decrement the match count 
            if( iMatch <= 0 ) break;                                          // if we have matched all patterns, break the loop
         }
      }
      
      if( iMatch != 0 ) vectorRowDelete.push_back(uRow);                      // if not all patterns match, add row number to delete vector
   }

   // ## delete all rows that do not match all patterns
   if( vectorRowDelete.empty() == false )
   {
      ptableLineList->erase( vectorRowDelete );
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Matches all specified regex patterns against the lines in the file line list.
 *
 * This function iterates over each row in the file line list and checks if the line text matches all specified regex patterns.
 * If a line does not match all patterns, it is marked for deletion.
 *
 * @param vectorRegexPattern A vector of pairs containing regex patterns and their string representations.
 * @param pdocument Pointer to a CDocument object used for accessing the file line list.
 * @param iMatchCount The number of patterns to match. If -1, all patterns are matched.
 * @return A pair where the first element is a boolean indicating success (true) or failure (false), and the second element is an empty string on success or an error message on failure.
 */
std::pair<bool, std::string> ListMatchAllPatterns_g(const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPattern, CDocument* pdocument, int iMatchCount)
{                                                                                                  assert( pdocument != nullptr ); assert( vectorRegexPattern.size() > 0 ); // at least one pattern must be specified
   std::vector<uint64_t> vectorRowDelete; // vector of row numbers to delete

   auto ptableLineList = pdocument->CACHE_Get("file-linelist"); // get the file line list from the cache

   if( iMatchCount == -1 ) iMatchCount = (int)vectorRegexPattern.size();                // if iMatchCount is -1 then set it to the size of the vectorPattern, so we check all patterns

   // ## iterate over all rows in table and check the line text for all patterns
   for( size_t uRow = 0; uRow < ptableLineList->get_row_count(); ++uRow )
   {
      std::string_view stringLineText = ptableLineList->cell_get_variant_view(uRow, "line").as_string_view(); // get the line text
      int iMatch = iMatchCount;                                               // number of patterns to match, if -1 then match all

      for(size_t u = 0; u < vectorRegexPattern.size(); ++u)
      {
         if( boost::regex_search(stringLineText.begin(), stringLineText.end(), vectorRegexPattern[u].first) )
         {
            iMatch--;                                                         // decrement the match count 
            if( iMatch <= 0 ) break;                                          // if we have matched all patterns, break the loop
         }
      }
      
      if( iMatch != 0 ) vectorRowDelete.push_back(uRow);                      // if not all patterns match, add row number to delete vector
   }

   // ## delete all rows that do not match all patterns

   if( vectorRowDelete.empty() == false )
   {
      ptableLineList->erase( vectorRowDelete );
   }

   return { true, "" };
}




NAMESPACE_CLI_END

