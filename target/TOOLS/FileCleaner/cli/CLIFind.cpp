/**                                                                            @TAG #ui.cli #command.find [description: definitions for find methods]
 * @file CLIFind.cpp
 * @brief This file contains the definitions methods used for the cli find command
 * 
 * The find command is used to search for patterns in files based on various options and arguments provided by the user.
 * It supports searching in multiple source paths, applying ignore patterns, and printing results in different formats.
 *
 * 
 */

#include <cstdint>
#include <format>

#include "gd/math/gd_math_string.h"

#include "../Command.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


#include "CLIFind.h"

NAMESPACE_CLI_BEGIN

/** --------------------------------------------------------------------------- @TAG #ui.cli #command.find [description: Main find method, this is the start for all find related logic]
 * @brief Processes the 'find' command and performs file searching based on provided options.
 *
 * @param poptionsFind Pointer to a gd::cli::options object containing command-line options.
 * @param poptionsFind.source A vector of source paths to search for files.
 * @param poptionsFind.ignore A string containing ignore patterns to exclude from the search.
 * @param poptionsFind.context An optional string specifying the context around matched row.
 * @param pdocument Pointer to a CDocument object used for file searching.
 * @return A std::pair where the first element is a boolean indicating success (true) or failure (false), and the second element is a string containing an error message if the operation failed, or an empty string on success.
 * 
 * @code
// Example usage of Find_g function
gd::cli::options options_; // add command line options to this object
auto* pdocument = DOCUMENT_Get("find", true );
auto result_ = CLI::Find_g( poptionsActive, pdocument );
if( result_.first == false ) return result_;
 * @endcode
 * 
 * @code
 // Example usage of Find_g function
 gd::cli::options optionsApplication; // Assume this is populated with command line arguments
 auto* pdocument = DOCUMENT_Get("find", true );
 std::vector<std::string> argv_ = { "find", "--source", "target/TOOLS/FileCleaner", "-R", "--pattern", "@code", "--segment", "comment", "-vs", "-verbose", "--rule", "\"select-between:begin_text,end_text\"" }; 
 std::vector<const char*> argv;
 argv.push_back("application"); // application name, can be anything
 for(const auto& arg : argv_) { argv.push_back(arg.c_str()); }
 auto result = optionsApplication.parse(argv.size(), argv.data());
 const gd::cli::options* poptionsActive = optionsApplication.find_active();
 auto result_ = CLI::Find_g( poptionsActive, pdocument );
 if( result_.first == false ) return result_;
 * @endcode
 */
std::pair<bool, std::string> Find_g(gd::cli::options* poptionsFind, CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);assert(poptionsFind != nullptr);
   gd::cli::options& options_ = *poptionsFind; // get the options from the command line arguments

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

   // ## FIND
   // ## Call the more generic Find_g function
   //    this is to make it possible to call Find_g with different environments, like CLI or GUI

   gd::argument::arguments* pargumentsFind = &options_.get_arguments(); // get the arguments from the command line options
   auto result_ = Find_g(vectorSource, pargumentsFind, pdocument);            // find files in the source directory based on the find arguments
   if( result_.first == false ) return result_;                               // if find failed, return the error

   if( pargumentsFind->exists("rule") == true )                                
   {
      auto vector_ = pargumentsFind->get_argument_all("rule");
      std::vector<std::string> vectorRule;
      for( auto& rule : vector_ ) { vectorRule.push_back(rule.as_string()); }

      result_ = ReadSnippet_g( vectorRule, pdocument );
      if( result_.first == false ) return result_;                            // if find failed, return the error
   }

   if( options_.exists("print") == false || options_["print"].is_true() == true )  // default is to print result
   {
      bool bPrint = false;
      if( options_.exists("rule") == true )
      {
         gd::argument::shared::arguments argumentsPrint;
         result_ = FindPrintSnippet_g(pdocument, argumentsPrint);             // Print the results of the find operation
         if( options_.exists("vs") == true ) 
         { 
            argumentsPrint.append("vs", true); 
            result_ = FindPrint_g(pdocument, argumentsPrint);                  // Print the results of the find operation
            if( result_.first == false ) return result_;                       // if print failed, return the error
         }
         bPrint = true;                                                        // set print to true, we have printed the results
      }

      if( options_.exists("keys") == true || options_.exists("kv") == true )
      {
         result_ = FindPrintKeyValue_g(pdocument);                             // Print the key-value pairs found in the files
         if( result_.first == false ) return result_;                          // if print failed, return the error
         bPrint = true;                                                        // set print to true, we have printed the results
      }


      if( bPrint == false || options_.exists("print") == true )
      {
         gd::argument::shared::arguments argumentsPrint({ { "pattern-count", uint64_t(2u) } }); // hardcode pattern count to 2 for printing results and allways print patterns
         if( options_.exists("context") == true ) argumentsPrint.append("context", options_["context"].as_string_view()); // if context is set, add it to the print arguments

         // check if vs flag is set, if so then print to Visual Studio output
         if( options_.exists("vs") == true ) { argumentsPrint.append("vs", true); }

         result_ = FindPrint_g(pdocument, argumentsPrint);                     // Print the results of the find operation
         if( result_.first == false ) return result_;                          // if print failed, return the error
      }
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
std::pair<bool, std::string> Find_g( const std::vector<std::string>& vectorSource, gd::argument::arguments* pargumentsFind, CDocument* pdocument)
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

   bool bUseKeyValue = false; // flag to indicate if key-value pairs should be used

   if( options_.exists("segment") == true ) { argumentsFind.append("segment", options_["segment"].as_string()); }
   if( options_.exists("kv") == true ) { argumentsFind.append( "kv", options_.get_argument_all("kv", gd::types::tag_view{})); bUseKeyValue = true; }
   if( options_.exists("keys") == true ) 
   { 
      argumentsFind.append("keys", options_.get_argument_all("keys", gd::types::tag_view{})); bUseKeyValue = true; 
      if( options_.exists("kv-format") == true ) argumentsFind.append("kv-format", options_.get_argument_all("kv-format", gd::types::tag_view{})); 
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
      auto vector_ = options_.get_argument_all("pattern", gd::types::tag_view{}); // get all patterns
      std::vector<std::string> vectorPattern; // store patterns as views
      std::vector<std::string> vectorPatternString;                           // store patterns as strings

      for( auto& pattern_ : vector_ ) {  vectorPattern.push_back(pattern_.as_string()); } // convert views to strings

      const std::string_view stringPattern = vectorPattern[0];                // get the pattern as a string view
      if( stringPattern.size() > sizeof("&c-") && stringPattern[2] == '-' )
      {
         // ## Special case for patterns starting with "&--", "&c-" or "&s-" or ohther combinations
         //    this is a hack to allow users to specify patterns rules with special characters like '&' or 'c' or 's' and save typing

         // found '&', 'c', 's' at start then match all patterns.             @TAG #ui.cli #command.find #hack [description: if first pattern character starts with & and then space, this will be like specify AND between all patterns, 'c' = comment segment and 's' = string segment]
         std::string_view string_( stringPattern.data(), 2);
         if( string_.find( '&' ) != std::string_view::npos ) pargumentsFind->append("match-all", true); 
         if( string_.find( 'c' ) != std::string_view::npos ) argumentsFind.append("segment", "comment"); 
         if( string_.find( 's' ) != std::string_view::npos ) argumentsFind.append("segment", "string"); 
         vectorPattern[0] = stringPattern.substr(3);                          // remove the first 3 characters '&c-' from the pattern
      }

      if( vectorPattern.size() == 1 )
      { 
         auto stringPattern = vectorPattern[0];
         vectorPattern = CApplication::Split_s(stringPattern, ';'); 
      }

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
         // ## Match all patterns, remove all rows that do not match all patterns

         result_ = MatchAllPatterns_g( vectorPattern, pdocument );
         if (result_.first == false) return result_;

         if( bUseKeyValue == true )
         {
            result_ = SynchronizeResult_g(pdocument);                         // Print the key-value pairs found in the files
            if (result_.first == false) return result_;                       // if print failed, return the error
         }
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
   }

   return { true, "" }; 
}

/** ---------------------------------------------------------------------------
 * @brief Matches all patterns in the vectorPattern against the lines in the file line list.
 *
 * This function iterates over all rows in the file line list and checks if all patterns in vectorPattern match the line text.
 * If a line does not match all patterns, it is marked for deletion.
 *
 * @param vectorPattern A vector of patterns to match against the line text.
 * @param pdocument Pointer to the CDocument instance containing the file line list.
 * @param iMatchCount The number of patterns to match, if -1 then match all patterns.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> MatchAllPatterns_g(const std::vector<std::string>& vectorPattern, CDocument* pdocument, int iMatchCount )
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
 * @brief Synchronizes tables used in find operation.
 *
 * Tables involved in the find operation is in this method checked and synchronized.
 * The file-linelist table is the core and dependent tables are checked against it.
 *
 * @param pdocument Pointer to the CDocument instance containing the file line list and key-value table.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> SynchronizeResult_g(CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);
   auto* ptableLineList = pdocument->CACHE_Get("file-linelist");                                   assert(ptableLineList != nullptr); // ensure the "file-linelist" table is in cache
   auto* ptableKeyValue = pdocument->CACHE_GetTableArguments("keyvalue"); 

   if( ptableKeyValue != nullptr )
   {
      std::vector<uint64_t> vectorRowDelete; // vector of row numbers to delete
      for( size_t uRow = 0; uRow < ptableKeyValue->get_row_count(); ++uRow )
      {
         uint64_t uLineListKey = ptableKeyValue->cell_get_variant_view(uRow, "file-linelist-key");

         // check if the key exists in the file line list
         auto iRow = ptableLineList->find( "key", uLineListKey );
         if( iRow == -1 ) { vectorRowDelete.push_back(uRow); }                 // if the key does not exist in the file line list, add row to delete vector
      }

      // ## delete all rows that do not match the file line list

      if( vectorRowDelete.empty() == false )
      {
         ptableKeyValue->erase(vectorRowDelete);                              // delete all rows that do not match the file line list
      }
   }
 
   return { true, "" };
   
}

/** --------------------------------------------------------------------------- @TAG #options.rule [description: Process rule for matched secton in file, rule is converted to scripting and script is executed]
 * @brief Reads a snippet based on the file line where find did find the pattern.
 *
 * This function processes the rules to select/process specific lines or ranges from the matched position in file,
 * and retrieves the corresponding code (snippets).
 * 
 * rules used are:
 * - `select-line:<line_number>`: Select a specific line.
 * - `select-between:<start_line>,<end_line>`: Select a range of lines.
 * - `select-all`: Select all lines for that multiline comment/string/code.
 *
 * @param vectorRule A vector of rules to apply for selecting snippets.
 * @param pdocument Pointer to the CDocument instance containing the file line list and snippet table.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> ReadSnippet_g( const std::vector<std::string>& vectorRule, CDocument* pdocument )
{                                                                                                  assert( vectorRule.empty() == false ); assert(pdocument != nullptr); 
   auto* ptableLineList = pdocument->CACHE_Get("file-linelist", true);        // ensure the "file-linelist" table is in cache

   if( ptableLineList->size() == 0 ) { return { true, ""}; }

   auto* ptableSnippet = pdocument->CACHE_Get("file-snippet", true);          // ensure the "file-snippet" table is in cache

   // ## convert rules to source code patterns used to collect snippets

   for( const auto& stringRule : vectorRule )
   {
      std::string stringRuleName;

      std::string stringPattern;
      auto uPosition = stringRule.find(':');
      if( uPosition == std::string::npos ) { stringRuleName = stringRule; } // if no colon is found, then the whole rule is the name
      else
      {
         stringRuleName = stringRule.substr(0, uPosition); // get rule name
         stringPattern = stringRule.substr(uPosition + 1); // get the pattern after the colon
      }

      if( stringRuleName == "select-all" )
      {
         std::string stringCode("source::select_all( source )");
         gd::argument::shared::arguments argumentsPattern({}); // create arguments for the pattern
         auto result_ = COMMAND_ReadSnippet_g(stringCode, argumentsPattern, ptableLineList, ptableSnippet); // read snippet from the source code using the pattern
         if(result_.first == false) return result_;
      }
      else if( stringRuleName == "select-between" )
      {
         std::string stringArguments = stringRule.substr(uPosition + 1);      // get the pattern after the colon
         auto vector_ = gd::utf8::split(stringArguments, ',');                // split the arguments by comma

         if( vector_.size() != 2 )
         {
            std::string stringError = "Invalid rule: '" + stringRule + "'. Expected format is 'select-between:from,to'.";
            return { false, stringError }; // if the rule is not recognized, return an error
         }

         std::string stringCode("source::select_between( source, from, to )");

         auto v = gd::variant(vector_[0]);

         gd::argument::shared::arguments argumentsPattern({ {"from", gd::variant(vector_[0])}, {"to", gd::variant(vector_[1])} }); // create arguments for the pattern
         auto result_ = COMMAND_ReadSnippet_g(stringCode, argumentsPattern, ptableLineList, ptableSnippet); // read snippet from the source code using the pattern
         if(result_.first == false) return result_;
      }
      else if( stringRuleName == "select-line" )
      {
         std::string stringCode("source::select_line( source, from )"); // updated to use the correct function for select-line
         gd::argument::shared::arguments argumentsPattern({}); // create arguments for the pattern
         if( stringPattern.empty() == false ) { argumentsPattern.append("from", stringPattern); } // add the line number to the arguments
         
         auto result_ = COMMAND_ReadSnippet_g(stringCode, argumentsPattern, ptableLineList, ptableSnippet); // read snippet from the source code using the pattern
         if(result_.first == false) return result_;
      }
      else
      {
         std::string stringError = "Unknown rule: '" + stringRule + "'.";
         return { false, stringError }; // if the rule is not recognized, return an error
      }


   }

   return { true, "" }; 
}

// @TASK [date: 250723] [name: key-value] [description: "print result for key value logic, like harvested values for selected keys"] [state: open] [priority: 1]

/** ---------------------------------------------------------------------------
 * @brief Prints the results of the find operation based on the provided arguments.
 *
 * This function retrieves the results of the find operation from the document,
 * formats them according to the specified options, and displays them to the user.
 *
 * @param pdocument Pointer to a CDocument instance containing the results of the find operation.
 * @param argumentsPrint The arguments specifying how to format and display the results.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
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


/** --------------------------------------------------------------------------- @TAG #cli #snippet
 * @brief Finds and prints snippets from the document based on the provided arguments.
 *
 * This function retrieves snippets from the document's cache, formats them into a string,
 * and displays them to the user. It also handles the case where no snippets are found.
 *
 * @param pdocument Pointer to a CDocument instance containing the snippets.
 * @param argumentsPrint The arguments specifying how to format and display the snippets.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> FindPrintSnippet_g( CDocument* pdocument, const gd::argument::shared::arguments& argumentsPrint )
{                                                                                                  assert(pdocument != nullptr);
   std::string stringCliTable; // string to hold the CLI table output
   auto* ptableLineList = pdocument->CACHE_Get("file-linelist");
   auto ptableSnippet = pdocument->CACHE_Get("file-snippet"); 

#ifndef NDEBUG
   std::string stringTableLineList_d = gd::table::debug::print(*ptableLineList); // debug print the line list table
   std::string stringTableSnippet_d = gd::table::debug::print(*ptableSnippet); // debug print the snippet table
#endif // NDEBUG

   if( ptableSnippet->size() == 0 ) { pdocument->MESSAGE_Display("\nNo snippets found."); return { true, "" }; } // if no snippets found, return

   pdocument->MESSAGE_Display( std::format("\n\nSnippets found: {}", ptableSnippet->size())); // display the number of snippets found

   for( auto uRowLine = 0u; uRowLine < ptableLineList->size(); ++uRowLine )
   {
      uint64_t uFileKey = ptableLineList->cell_get_variant_view(uRowLine, "key").as_uint64();

      bool bFound = false; // flag to check if we found snippet for the line

      for( auto uRowSnippet = 0u; uRowSnippet < ptableSnippet->get_row_count(); ++uRowSnippet )
      {
         uint64_t uForeignKeyFile = ptableSnippet->cell_get_variant_view(uRowSnippet, "file-key").as_uint64(); // get the foreign key from the snippet table
         if( uForeignKeyFile != uFileKey ) continue;                          // if the foreign key does not match the file key, skip this row

         if( bFound == false )
         {
            if( stringCliTable.empty() == false ) stringCliTable += "\n===\n"; // if the stringCliTable is not empty, add a newline before the next snippet   
            stringCliTable += ptableLineList->cell_get_variant_view(uRowLine, "filename").as_string_view(); // get the filename from the line list table
            uint64_t uLineNumber = ptableLineList->cell_get_variant_view(uRowLine, "row").as_uint64(); // get the line number from the line list table
            uLineNumber++;                                                     // add one because lines in table are zero based
            stringCliTable += std::format("({})", uLineNumber );
            stringCliTable += "\n";                                            // add a newline after the filename
            bFound = true;                                                     // mark that we have found a snippet for this file key
         }

         stringCliTable += "--\n";                                             // add a separator after the filename
         stringCliTable += ptableSnippet->cell_get_variant_view(uRowSnippet, "snippet").as_string_view(); // get the snippet from the snippet table
         stringCliTable += "\n";                                            // add a newline after the snippet
      }
   }

   pdocument->MESSAGE_Display(stringCliTable);                                 // display the snippet table to the user
   return { true, "" };                                                        // return success
}

/** ---------------------------------------------------------------------------
 * @brief Finds and prints key-value pairs from the table "keyvalue" in document.
 *
 * This function retrieves key-value pairs from the document's cached table "keyvalue", formats them into a string,
 * and displays them to the user. It also handles the case where no key-value pairs are found.
 * 
 * When used as terminal application, this function will print the key-value pairs in a column format where keys are aligned, values are indented,
 *
 * @param pdocument Pointer to a CDocument instance containing the key-value pairs.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> FindPrintKeyValue_g(CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);
   auto* ptableKeyValue = pdocument->CACHE_GetTableArguments("keyvalue");     // get table for key-value pairs from the cache
   if( ptableKeyValue->size() == 0 ) { pdocument->MESSAGE_Display("\nNo key-value pairs found."); return { true, "" }; } // if no key-value pairs found, return

   unsigned uKeyMarginWidth = 0; // margin width for the key, used to align the keys in the output
   std::string stringCliTable; // string to hold the CLI table output

   // ## Calculate width of the longest key, this to format the output nicely

   const auto* pargumentsRow = ptableKeyValue->row_get_arguments_pointer(0);
   for( auto it = std::begin( *pargumentsRow ); it != std::end( *pargumentsRow ); ++it ) // iterate over the arguments object
   {
      auto name_ = it.name();
      if( name_.size() > uKeyMarginWidth ) uKeyMarginWidth = (unsigned)name_.size();// update the key width if the current key is longer
   }

   // ## Print values in the key-value table

   for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow )
   {
      // ## Prepare file name

      std::string stringFilename = ptableKeyValue->cell_get_variant_view(uRow, "filename").as_string(); // get the filename from the key-value table
      uint64_t uRowNumber = ptableKeyValue->cell_get_variant_view(uRow, "row").as_uint64(); // get the row number from the key-value table
      uRowNumber++; // add one because rows in table are zero based
      stringFilename += std::format("({})", uRowNumber); // add the row number to the filename
      stringCliTable += std::format("\n{:-<80}\n", stringFilename + "  ");  // add the filename to the stringCliTable, with a separator before it

      // ## Get the arguments object from row
      const auto* pargumentsRow = ptableKeyValue->row_get_arguments_pointer(uRow); // get the arguments object from the row
      if( pargumentsRow == nullptr ) continue;                                  // if the arguments object is null, skip this row

      // ### Iterator over values in the arguments object
      for( auto it = std::begin( *pargumentsRow ); it != std::end( *pargumentsRow ); ++it ) // iterate over the arguments object
      {
         //stringCliTable += std::format("{}: {}", argument.name(), argument.as_string()); // format the key-value pair as "key: value"
         auto name_ = it.name();
         auto stringValue_ = it.get_argument().as_string();                          // get the name and value of the argument
         if( stringValue_.find('\n') != std::string::npos ) // if the value contains a newline, replace it with a space
         {
            stringValue_ = gd::math::string::format_indent(stringValue_, uKeyMarginWidth + 2, false); // indent the value with the key margin width + 2 spaces because adds for separator and space after that
         }

         // Print name with padding
         stringCliTable += std::format("{:>{}}: {}", name_, uKeyMarginWidth, stringValue_); // format the key-value pair as "key: value" with padding
         stringCliTable += "\n";
      }
   }

   pdocument->MESSAGE_Display(stringCliTable);                                 // display the key-value pairs to the user

   // ## print summary of key-value pairs
   std::string stringSummary = std::format("\nFound {} areas with key-value pairs", ptableKeyValue->get_row_count());
   pdocument->MESSAGE_Display(stringSummary);   
   return { true, "" };                                                        // return success
}

NAMESPACE_CLI_END
