/**                                                                            @FILE [tag: cli, find] [description: definition for methods used for find]
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

#include "gd/console/gd_console_console.h"
#include "gd/math/gd_math_string.h"

#include "../Command.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif

#include "../automation/code-analysis/Run.h"

#include "CLI_Shared.h"
#include "CLIFind.h"

NAMESPACE_CLI_BEGIN

/** ---------------------------------------------------------------------------
 * @brief Structure to hold formatting parameters for key-value printing.
 *
 * This structure groups all formatting-related parameters used when printing
 * key-value table rows, reducing parameter passing complexity.
 */
struct kv_print
{
   const std::vector<std::string>* pvectorHeader;                             // keys to include in header
   const std::vector<std::string>* pvectorBrief;                              // keys to include in brief section
   const std::vector<std::string>* pvectorBody;                               // keys to include in body section
   const std::vector<std::string>* pvectorFooter;                             // keys to include in footer section
   unsigned uWidth;                                                           // width for formatting output lines
   unsigned uTextWidth;                                                       // width for formatting text content
   unsigned uKeyMarginWidth;                                                  // margin width for aligning keys
   std::string_view stringHeaderFormat;                                       // format string for header line
   std::string_view stringBriefFormat;                                        // format string for brief line
   std::string_view stringFooterFormat;                                       // format string for footer line
   std::array<std::byte, 64>* parray;                                         // array to hold color codes

   kv_print() : pvectorHeader(nullptr), pvectorBrief(nullptr), pvectorBody(nullptr), pvectorFooter(nullptr), uWidth(80), uTextWidth(0), uKeyMarginWidth(0), parray(nullptr) {}
   kv_print( const std::vector<std::string>* pvectorHeader_, const std::vector<std::string>* pvectorBrief_, const std::vector<std::string>* pvectorBody_, const std::vector<std::string>* pvectorFooter_, unsigned uWidth_, unsigned uTextWidth_, unsigned uKeyMarginWidth_, std::string_view stringHeaderFormat_, std::string_view stringBriefFormat_, std::string_view stringFooterFormat_)
    : pvectorHeader(pvectorHeader_), pvectorBrief(pvectorBrief_), pvectorBody(pvectorBody_), pvectorFooter(pvectorFooter_),uWidth(uWidth_), uTextWidth(uTextWidth_), uKeyMarginWidth(uKeyMarginWidth_), stringHeaderFormat(stringHeaderFormat_), stringBriefFormat(stringBriefFormat_), stringFooterFormat(stringFooterFormat_) {}

};

std::pair<bool, std::string> PrintKeyValueRowsBasic_s( CDocument* pdocument, gd::table::arguments::table* ptableKeyValue, const kv_print& kv_);
std::pair<bool, std::string> PrintKeyValueRows_s( CDocument* pdocument, gd::table::arguments::table* ptableKeyValue, const kv_print& kv_);


/** --------------------------------------------------------------------------- @API [tag: cli, command, find] [description: Searches for patterns in files based on various options, differ from list that this can do multiline searches]
 * @brief Processes the 'find' command and performs file searching based on provided options.
 *
 *  - Reads settings from command line options to collect.
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
   std::vector<std::string> vectorSource; // vector of source paths

   // ## Check if source is provided in clipboard and it should look there

   if( options_.exists("clip") == true && options_["clip"].is_true() == true ) // test for clip argument
   {
      std::string stringFile;
      OS_ReadClipboard_g(stringFile);                                          // read clipboard content

      // ### Check if clipboard content is a valid file path and informtion user if found
      if( stringFile.empty() == false && std::filesystem::exists( stringFile ) == true )
      {
         pdocument->MESSAGE_Display(std::format("File from clipboard as source: {}", stringFile));
         poptionsFind->set_value("source", stringFile);                       // set source to the file from clipboard
      }
   }

   gd::argument::arguments argumentsFileHarvest;
   SHARED_ReadHarvestSetting_g( options_, argumentsFileHarvest, pdocument );  // Harvest files to find information in, settings to read from options
   options_.get_arguments().append( argumentsFileHarvest, { "depth" } );

   auto vectorSourceToPrepare = argumentsFileHarvest.get_argument_all("source", gd::types::tag_view{}); // get all source arguments, this is used to find files in the source directory

   // ## Source preparation
   //    if source is one then check if it may contain multiple sources that are separated by ':'
   if( vectorSourceToPrepare.size() == 1 )
   {
      std::string stringSource = vectorSourceToPrepare[0].as_string();
      auto uCount = CApplication::PreparePath_s(stringSource, ';');           // if source is empty then set it to current path, otherwise prepare it
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

   // ## FIND
   // ## Call the more generic Find_g function
   //    this is to make it possible to call Find_g with different environments, like CLI or GUI

   gd::argument::arguments* pargumentsFind = &options_.get_arguments();       // get the arguments from the command line options
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

   // ## Print results

   if( options_.exists("print") == false || options_["print"].is_true() == true )// default is to print result
   {
      papplication_g->Print("background", gd::types::tag_background{} );

      // ### Print results from rule
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

      // ### Print results from key-value pairs
      if( options_.exists("keys") == true || options_.exists("kv") == true )
      {
         gd::argument::shared::arguments argumentsPrint;
         argumentsPrint.append( options_.get_arguments(), {"context", "keys", "header", "footer", "brief", "width"});
         if( options_.exists("kv-where") == true ) argumentsPrint.append("where", options_["kv-where"].as_string_view()); // if kv-where is set, add it to the print arguments
         result_ = FindPrintKeyValue_g(pdocument, &argumentsPrint);                             // Print the key-value pairs found in the files
         if( result_.first == false ) return result_;                          // if print failed, return the error

#ifdef _WIN32
         if( options_.exists("vs") == true ) 
         { 
            argumentsPrint.append("vs", true); 

            auto* ptableKeyValue = pdocument->CACHE_GetTableArguments( "keyvalue" );                 assert( ptableKeyValue != nullptr );
            gd::table::dto::table tableVS( 0, { {"rstring", 0u, "line"} }, gd::table::tag_prepare{} );
            std::string stringLine;
            for( auto row : *ptableKeyValue )
            {
               stringLine = row.cell_get_variant_view( "filename" ).as_string_view(); // get full filename
               uint64_t uLineNumber = row.cell_get_variant_view("row").as_uint64(); // get the line number from the line list table
               auto stringPreview = row.cell_get_variant_view( "preview" ).as_string_view();
               uLineNumber++;                                                     // add one because lines in table are zero based
               stringLine += std::format("({}) : {}", uLineNumber, stringPreview );

               tableVS.row_add( { stringLine } );
            }

            result_ = FindPrintVS_g( tableVS );                                // Print to visual studio output
            if( result_.first == false ) return result_;                       // if print failed, return the error
         }
#endif // _WIN32

         bPrint = true;                                                        // set print to true, we have printed the results
         return { true, "" };
      }


      if( bPrint == false || (options_.exists("print") == true || options_.exists("vs") == true ))
      {
         gd::argument::shared::arguments argumentsPrint({ { "pattern-count", uint64_t(2u) } }); // hardcode pattern count to 2 for printing results and allways print patterns
         if( options_.exists("context") == true ) argumentsPrint.append("context", options_["context"].as_string_view()); // if context is set, add it to the print arguments

         // check if vs flag is set, if so then print to Visual Studio output
         if( options_.exists("vs") == true ) { argumentsPrint.append("vs", true); }

         result_ = FindPrint_g(pdocument, argumentsPrint);                     // Print the results of the find operation
         if( result_.first == false ) return result_;                          // if print failed, return the error
      }

      papplication_g->Print("", gd::types::tag_background{} );
   }

   return { true, "" }; 
}

/** ---------------------------------------------------------------------------
 * @brief Finds files based on the provided source paths and find arguments.
 *
 * This function searches for files in the specified source paths, applies regex patterns if provided,
 * and prints the results of the find operation.
 * 
 * *steps*
 * 1. Update the global application state with `UpdateApplicationState()`
 * 2. Harvest files with `FILE_Harvest()`
 * 2.1 If keys or header are provided for key-value pairs, merge them into a unified list. Keys are separated by semicolons, so extract each key name between semicolons and combine them into the final merged keys string.
 * 3. Based on the provided patterns, search for matches in the harvested files in `FILE_UpdatePatternFind`.
 *
 * @param vectorSource A vector of source paths to search for files.
 * @param pargumentsFind The arguments containing options such as recursive search, filter, and regex patterns.
 * @param pargumentsFind.pattern patterns to search for in the files.
 * @param pargumentsFind.rpattern regex patterns to search for in the files.
 * @param pargumentsFind.keys Used to specify for key-value pairs reading.
 * @param pargumentsFind.* Lots of other options that can be used to control the find operation, like max, segment, kv, kv-format, etc.
 * @param pdocument Pointer to the CDocument instance where the results will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> Find_g( const std::vector<std::string>& vectorSource, gd::argument::arguments* pargumentsFind, CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);assert(pargumentsFind != nullptr);
   gd::argument::arguments& options_ = *pargumentsFind; // get the options from the command line arguments

   uint64_t uPatternCount = 0; // count of patterns to search for

   int iRecursive = options_["depth"].as_int();

   std::string stringFilter = options_["filter"].as_string();

   gd::argument::shared::arguments argumentsFind; // prepare arguments for the file update

   if( options_.exists("max") == true )
   {
      uint64_t uMax = options_["max"].as_uint64();
      argumentsFind.append("max", uMax);                                      // set the maximum number of matches to find
   }

   bool bUseKeyValue = false; // flag to indicate if key-value pairs should be used

   if( options_.exists("segment") == true ) { argumentsFind.append("segment", options_["segment"].as_string()); }
   if( options_.exists_any( { "keys", "brief", "header", "footer" } ) == true )  // if keys or header or brief or footer is set, we want to use key-value pairs
   {
      bUseKeyValue = true;                                                    // if keys are set, we want to use key-value pairs
      // ## merge header, footer, brief and  keys with key-value pairs
      if( options_.exists_any({ "header", "brief", "footer" }) == true )
      {
         std::vector<std::string_view> vectorKeys; // vector to store all keys used for key-value pairs
         vectorKeys.push_back(options_["keys"].as_string_view());             // add header to the keys
         vectorKeys.push_back(options_["header"].as_string_view());           // add header to the keys
         vectorKeys.push_back(options_["brief"].as_string_view());            // add brief to the keys
         vectorKeys.push_back(options_["footer"].as_string_view());           // add footer to the keys

         char iSeparator = ';'; // separator for keys
         for( const auto& keys_ : vectorKeys )
         {
            if( keys_.find(',') != std::string_view::npos ) { iSeparator = ','; break; } // if comma is found, use it as separator
            else if( keys_.find(';') != std::string_view::npos ) { iSeparator = ';'; break; } // if semicolon is found, use it as separator
         }

         auto stringMergedKeys = gd::math::string::merge_delimited(vectorKeys, iSeparator); // merge keys and header into the keys argument
         options_.set("keys", std::string_view( stringMergedKeys ));
      }
                                                                                                   LOG_DEBUG_RAW("== keys: " & argumentsFind["keys"].as_string());
      argumentsFind.append("keys", options_.get_argument_all("keys", gd::types::tag_view{}));  
      if( options_.exists("kv-format") == true ) argumentsFind.append("kv-format", options_.get_argument_all("kv-format", gd::types::tag_view{})); 
      else
      {  // ## Get format for key-value pairs from configuration
         auto format_ = papplication_g->CONFIG_Get("format", { "kv","keyvalue" }); // get the format for key-value pairs from application configuration
         if( format_.is_true() == true ) { argumentsFind.append("kv-format", format_.as_string_view()); } // if kv-format is set, use it
      }
                                                                                                   LOG_DEBUG_RAW( "== keyvalue format: " & argumentsFind["kv-format"].as_string() );
   }

   // ## Check if kv or keys are provided in kv argument, this is special (hack) for quick editing sending arguments
   if( options_.exists("kv") == true ) 
   { 
      std::string string_ = options_["kv"].as_string();
      // Extract keys, they are before @ character
      auto uPosition = string_.find('@');
      if( uPosition != std::string::npos )
      {
         std::string stringKeys = string_.substr(0, uPosition);
         argumentsFind.append("keys", stringKeys);
         options_.set("keys", std::string_view( stringKeys ));
         //options_.append("keys", stringKeys);
         // add kv-format, characters after @ character are used as kv-format
         std::string stringKVFormat = string_.substr(uPosition + 1);
         argumentsFind.append("kv-format", stringKVFormat);
         options_.set("kv-format", std::string_view( stringKVFormat ));
         //options_.append("kv-format", stringKVFormat);
      }
      else
      {
         argumentsFind.append("keys", string_);                               // if no @ character is found, then use the whole string as keys
      }
      bUseKeyValue = true; 
   }


   // ## Harvest files from the source paths ..................................

   for( const auto& stringSource : vectorSource )
   {
      gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
      auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);    // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if (result_.first == false) return result_;
   }

   auto* ptableFile = pdocument->CACHE_Get("file");                                                assert(ptableFile != nullptr); LOG_DEBUG_RAW("== number of files: " & ptableFile->size());

   // ## Search for patterns in the harvested files ...........................

   if( options_.exists("pattern") == true )
   {                                                                                               LOG_INFORMATION_RAW( "== search pattern: " & options_["pattern"].as_string() );
      argumentsFind.append( options_, {"icase", "word"});
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

      auto result_ = pdocument->FILE_UpdatePatternFind(vectorPatternString, &argumentsFind, 0); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false)
      {
         if (pdocument->ERROR_Empty() == false)
         {
            pdocument->ERROR_Print();                                         // print any errors that occurred during the pattern search)
         }
         return result_;                                                      // if error occurred, return the error
      }

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
 * Unconnected rows are removed from the dependent tables. Only connected rows are kept.
 *
 * @param pdocument Pointer to the CDocument instance containing the file line list and key-value table.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> SynchronizeResult_g(CDocument* pdocument)
{                                                                                                  assert(pdocument != nullptr);
   const auto* ptableLineList = pdocument->CACHE_Get("file-linelist");                             assert(ptableLineList != nullptr); // ensure the "file-linelist" table is in cache
   auto* ptableKeyValue = pdocument->CACHE_GetTableArguments("keyvalue"); 

   if( ptableKeyValue != nullptr )
   {
      std::vector<uint64_t> vectorRowDelete; // vector of row numbers to delete

      // ## iterate over all rows in key-value table and check if the file-linelist-key exists in the file line list

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
   std::array<std::byte, 64> array_; // array to hold the color codes for the output
   std::string stringCliTable; // string to hold the CLI table output
   size_t uSearchPatternCount = argumentsPrint.get_argument<uint64_t>("pattern-count", 1u); // count of patterns to search for

   pdocument->MESSAGE_Display("\n");

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
      pdocument->MESSAGE_Display(stringCliTable, { array_, {{"color", "default"}}, gd::types::tag_view{} });
      stringCliTable.clear();
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

   if( stringCliTable.empty() == false ) pdocument->MESSAGE_Display( stringCliTable );// display the result table to the user
   // Print number of lines found
   std::string stringMessage = std::format("\nFound {} lines", uRowCount);
   pdocument->MESSAGE_Display(stringMessage);                                 // display the number of lines found

#ifdef _WIN32
   if( argumentsPrint.exists("vs") == true ) // if vs flag is set, then we want to print to Visual Studio output
   {
      auto result_ = FindPrintVS_g( tableResultLineList );
      if( result_.first == false ) return result_;
   }
#endif // _WIN32

   pdocument->MESSAGE_Display();                                              // reset the message display

   return { true, "" }; 
}
#ifdef _WIN32
std::pair<bool, std::string> FindPrintVS_g( const gd::table::dto::table& table_ )
{
   std::string stringCliTable; // string to hold the CLI table output
   // ## Print to Visual Studio output
   CDocument::RESULT_VisualStudio_s(table_, stringCliTable);
   VS::CVisualStudio visualstudio;
   auto result_ = visualstudio.Connect();
   if(result_.first == true) result_ = visualstudio.Print(stringCliTable, VS::tag_vs_output{});
   if(result_.first == false)
   {
      std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
      return { false, stringError };
   }
                                                                                                   LOG_INFORMATION_RAW( std::format("Printed to Visual Studio output: {} rows", table_.get_row_count()) );
   return { true, "" };
}
#endif // _WIN32


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
 * @param pargumentsPrint The arguments specifying how to generate the print output.
 * @param pargumentsPrint.header Optional argument to specify which keys to include in the header of the output.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 * 
 * @verbatim
 * Example output: Note that there are code in this method to enable this, read carefully.
 ┌─ key-value ────────────────────────────────────────────────────────────────┐ header and header-line
 │>> write documentation                                                        brief
 C:\dev\home\DOD\target\TOOLS\FileCleaner\Document.cpp(833)
 description: description how to write documentation                            keys
 └────────────────────────────────────────────────────────────────────────────┘ footer and footer-line
 * @endverbatim
 */
std::pair<bool, std::string> FindPrintKeyValue_g(CDocument* pdocument, const gd::argument::shared::arguments* pargumentsPrint )
{                                                                                                  assert(pdocument != nullptr); assert(pargumentsPrint != nullptr);
   std::array<std::byte, 64> array_; // array to hold the color codes for the output
   auto* ptableKeyValue = pdocument->CACHE_GetTableArguments("keyvalue");     // get table for key-value pairs from the cache
   if( ptableKeyValue == nullptr || ptableKeyValue->size() == 0 ) { pdocument->MESSAGE_Display("\nNo key-value pairs found."); return { true, "" }; } // if no key-value pairs found, return

   unsigned uWidth = 80; // default width for the output, can be changed by configuration
   unsigned uTextWidth = 0; // width of the text, used to align the text in the output
   std::string stringPrint; // string to hold the formatted output, this is used to format the output for terminal application in rest of method

   // ## Prepare some special output formats for parts in the output ..........
   //    line and brief sections are formatted to simplify the output

   unsigned uKeyMarginWidth = 0; // margin width for the key, used to align the keys in the output
   std::string stringHeaderFormat = pdocument->GetApplication()->CONFIG_Get("format", "header-line").as_string(); // get the header line format from the configuration
   if( stringHeaderFormat.empty() == false && stringHeaderFormat[0] == '0' && stringHeaderFormat[1] == 'x'  )
   { 
      // convert hex character codes to string characters for each hex code
      stringHeaderFormat = gd::math::string::convert_hex_to_ascii(stringHeaderFormat.substr(2)); // remove the "0x" prefix and convert hex to string
   } 

   std::string stringBriefFormat = pdocument->GetApplication()->CONFIG_Get("format", "brief").as_string(); // get the brief line format from the configuration
   if( stringBriefFormat.empty() == false && stringBriefFormat[0] == '0' && stringBriefFormat[1] == 'x'  )
   { 
      // convert hex character codes to string characters for each hex code
      stringBriefFormat = gd::math::string::convert_hex_to_ascii(stringBriefFormat.substr(2)); // remove the "0x" prefix and convert hex to string
   } 

   std::string stringFooterFormat = pdocument->GetApplication()->CONFIG_Get("format", "footer-line").as_string(); // get the footer line format from the configuration
   if( stringFooterFormat.empty() == false && stringFooterFormat[0] == '0' && stringFooterFormat[1] == 'x'  )
   { 
      // convert hex character codes to string characters for each hex code
      stringFooterFormat = gd::math::string::convert_hex_to_ascii(stringFooterFormat.substr(2)); // remove the "0x" prefix and convert hex to string
   } 

   if( pargumentsPrint->exists("width") == true ) { uWidth = pargumentsPrint->get_argument("width").as_uint(); uTextWidth = uWidth; }
   else if( pdocument->GetApplication()->CONFIG_Exists("format", "width") == true )
   {
      uWidth = pdocument->GetApplication()->CONFIG_Get("format", "width").as_uint();
      uTextWidth = uWidth;                                                    // use the same width for text
   }

   if( uWidth < 40 ) { uWidth = 40; uTextWidth = uWidth; }                    // ensure the width is at least 40 characters, this is to avoid too narrow output

   stringPrint += "\n\n"; 
   stringPrint += gd::math::string::format_header_line("RESULT", uWidth, '#', '=', '#');// add a header line to the output
   stringPrint += "\n"; 
   pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "default"}}, gd::types::tag_view{} });
   stringPrint.clear(); 

   // ## check for where filter in argumentsPrint, if exists then filter the key-value pairs       @TAG #expression.where
   if( pargumentsPrint->exists("where") == true )
   {
      auto stringKeys = pargumentsPrint->get_argument("keys").as_string();
      auto vectorColumn = CApplication::Split_s( stringKeys, 0 );

      std::string stringWhere = pargumentsPrint->get_argument("where").as_string();   // get the where filter from the arguments


      auto result_ = pdocument->CACHE_Where( "keyvalue", stringWhere, vectorColumn );
      if( result_.first == false ) { return result_; }                            // if where filter failed, return the error
   }

   if( pargumentsPrint->exists("context") == true )
   {
      gd::argument::arguments argumentsContext;
      argumentsContext.append(*pargumentsPrint, { "context" });               // add context
      pdocument->CACHE_Context("keyvalue", argumentsContext);
   }


   // ## Extract keys ......................................................... 

   std::vector<std::string> vectorBody;
   std::vector<std::string> vectorHeader;
   std::vector<std::string> vectorBrief;
   std::vector<std::string> vectorFooter;
   std::vector<std::string> vectorAll;

   char iSeparator = ';'; // separator used to split the keys
   {
      auto string_ = pargumentsPrint->get_argument("keys").as_string_view();
      if( string_.find(',') != std::string::npos ) { iSeparator = ','; }      // if the keys are separated by commas, use comma as separator
   }

   if( pargumentsPrint->exists("keys") == true ) { vectorBody =  gd::utf8::split( pargumentsPrint->get_argument("keys").as_string(), iSeparator, gd::types::tag_string{}); }
   if( pargumentsPrint->exists("header") == true ) { vectorHeader = gd::utf8::split( pargumentsPrint->get_argument("header").as_string(), iSeparator, gd::types::tag_string{}); }
   if( pargumentsPrint->exists("brief") == true ) { vectorBrief = gd::utf8::split( pargumentsPrint->get_argument("brief").as_string(), iSeparator, gd::types::tag_string{}); }
   if( pargumentsPrint->exists("footer") == true ) { vectorFooter = gd::utf8::split( pargumentsPrint->get_argument("footer").as_string(), iSeparator, gd::types::tag_string{}); }

   vectorAll = vectorBody;

   // ### Remove keys from vectorBody found in other vectors
   if( vectorHeader.empty() == false )
   {
      // remove keys from vectorBody that are in vectorHeader
      vectorBody.erase(std::remove_if(vectorBody.begin(), vectorBody.end(), [&vectorHeader](const std::string& key_) {
         return std::find(vectorHeader.begin(), vectorHeader.end(), key_) != vectorHeader.end();
      }), vectorBody.end());
   }
   if( vectorBrief.empty() == false )
   {
      // remove keys from vectorBody that are in vectorBrief
      vectorBody.erase(std::remove_if(vectorBody.begin(), vectorBody.end(), [&vectorBrief](const std::string& key_) {
         return std::find(vectorBrief.begin(), vectorBrief.end(), key_) != vectorBrief.end();
      }), vectorBody.end());
   }
   if( vectorFooter.empty() == false )
   {
      // remove keys from vectorBody that are in vectorFooter
      vectorBody.erase(std::remove_if(vectorBody.begin(), vectorBody.end(), [&vectorFooter](const std::string& key_) {
         return std::find(vectorFooter.begin(), vectorFooter.end(), key_) != vectorFooter.end();
      }), vectorBody.end());
   }

   // ## Calculate width of the longest key used in body, this to format the output nicely

   for( const auto& it : vectorBody ) // iterate over body keys
   {
      if( it.size() > uKeyMarginWidth ) uKeyMarginWidth = (unsigned)it.size();// update the key width if the current key is longer
   }

   kv_print kv_{ &vectorHeader, &vectorBrief, &vectorBody, &vectorFooter, 
                 uWidth, uTextWidth, uKeyMarginWidth, 
                 stringHeaderFormat, stringBriefFormat, stringFooterFormat };

   if( papplication_g->GetDetail() == CApplication::eDetailBasic )
   {
      PrintKeyValueRowsBasic_s( pdocument, ptableKeyValue, kv_ );
   }
   else
   { 
      PrintKeyValueRows_s( pdocument, ptableKeyValue, kv_ );
   }


   // ## Print values in the key-value table ..................................
   //    Print order for each row is:
   //    - header with header line
   //    - brief with brief line 
   //    - Source code file name with row number
   //    - body with key-value pairs
   //    - footer with footer line
/*
   for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow )
   {
      std::string stringContext; // context code if any, this is used to print the context code if any
      // ### Get context if set
      if( ptableKeyValue->cell_is_null( uRow, "context" ) == false )
      {
         stringContext = ptableKeyValue->cell_get_variant_view(uRow, "context").as_string();
      }

      // ### Prepare header and print line if vectorHeader is not empty

      const auto* pargumentsRow = ptableKeyValue->row_get_arguments_pointer(uRow); // get the arguments object from the row
      if( vectorHeader.empty() == false )
      {
         stringPrint.clear();                                                // clear the stringPrint for the next row
         for( const auto& key_ : vectorHeader ) 
         { 
            auto stingValue = pargumentsRow->get_argument( key_ ).as_string_view(); 
            if( stringPrint.empty() == false && stingValue.empty() == false ) stringPrint += ", "; // add a separator if the stringPrint is not empty

            stringPrint += stingValue; 
         }

         if( stringHeaderFormat.empty() == true ) { stringPrint = gd::math::string::format_header_line(stringPrint, uWidth); }
         else                                     { stringPrint = gd::math::string::format_header_line(stringPrint, gd::math::string::enumAlignment::eAlignmentLeft , uWidth, stringHeaderFormat); }

         pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "header"}}, gd::types::tag_view{} });
      }

      // ### Prepare and print brief line if vectorBrief is not empty

      if( vectorBrief.empty() == false )
      {
         stringPrint.clear();                                                // clear the stringPrint for the next row
         for( const auto& key_ : vectorBrief ) 
         { 
            if( stringPrint.empty() == false ) stringPrint += "\n";           // add a separator if the stringPrint is not empty

            std::string stringBrief = pargumentsRow->get_argument( key_ ).as_string(); 
            if( uTextWidth > 0 && stringBrief.empty() == false)
            {
               // format the value to fit in the width, with a margin for the key and separator
               auto uWidth = uTextWidth - 2 - stringBriefFormat.length();
               if( uWidth < 40 ) uWidth = 40;                                  // ensure the width is at least 40 characters, this is to avoid too narrow output
               stringBrief = gd::math::string::format_text_width(stringBrief, uWidth);
               stringBrief = gd::math::string::format_indent(stringBrief, stringBriefFormat.length(), false); // indent the value with the key margin width + 2 spaces because adds for separator and space after that
            }

            stringPrint += stringBrief;
         }

         if( stringPrint.empty() == false )
         {
            pdocument->MESSAGE_Display( stringBriefFormat + stringPrint, {array_, {{"color", "brief"}}, gd::types::tag_view{}});
         }
      }

      // ### Prepare and print file name

      stringPrint = ptableKeyValue->cell_get_variant_view(uRow, "filename").as_string(); // get the filename from the key-value table
      uint64_t uRowNumber = ptableKeyValue->cell_get_variant_view(uRow, "row").as_uint64(); // get the row number from the key-value table
      uRowNumber++;                                                           // add one because rows in table are zero based
      stringPrint += std::format("({})", uRowNumber);                         // add the row number to the filename

      // If no header line then line is used as header line
      if( vectorHeader.empty() == true ) { stringPrint += std::format("{:-<80}", stringPrint + "  "); } // add the filename to the stringPrint, with a separator before it

      pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "line"}}, gd::types::tag_view{} });

      // ### Prepare and print body

      if( pargumentsRow != nullptr )
      {
         stringPrint.clear();
         for( const auto& key_ : vectorBody )                                 // iterate over the keys in the vectorBody
         {
            if( pargumentsRow->exists(key_) == false ) continue;              // if the key does not exist in the arguments object, skip this key-value pair

            if( stringPrint.empty() == false ) stringPrint += '\n';

            auto stringValue_ = pargumentsRow->get_argument(key_).as_string();// get the value of the argument

            if( uTextWidth > 0 )
            {
               // format the value to fit in the width, with a margin for the key and separator
               stringValue_ = gd::math::string::format_text_width(stringValue_, uTextWidth - uKeyMarginWidth - 2); 
            }

            if( stringValue_.find('\n') != std::string::npos ) // if the value contains a newline, indent it
            {
               stringValue_ = gd::math::string::format_indent(stringValue_, uKeyMarginWidth + 2, false); // indent the value with the key margin width + 2 spaces because adds for separator and space after that
            }
            // Print name with padding
            stringPrint += std::format("{:>{}}: {}", key_, uKeyMarginWidth, stringValue_); // format the key-value pair as "key: value" with padding
         }

         if( stringPrint.empty() == false ) pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "body"}}, gd::types::tag_view{} });
      }

      // ### Prepare and print footer

      if( vectorFooter.empty() == false )
      {
          stringPrint.clear();                                                // clear the stringPrint for the next row
         for( const auto& key_ : vectorFooter ) 
         { 
            auto stingValue = pargumentsRow->get_argument( key_ ).as_string_view(); 
            if( stringPrint.empty() == false && stingValue.empty() == false ) stringPrint += ", "; // add a separator if the stringPrint is not empty

            stringPrint += stingValue; 
         }
         if( stringFooterFormat.empty() == true ) { stringPrint = gd::math::string::format_header_line(stringPrint, gd::math::string::enumAlignment::eAlignmentRight, uWidth); }
         else                                     { stringPrint = gd::math::string::format_header_line(stringPrint, gd::math::string::enumAlignment::eAlignmentRight, uWidth, stringFooterFormat); }
         pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "footer"}}, gd::types::tag_view{} });
      }

      // ## Prepare preview part

      std::string stringPreview;
      for( auto it = pargumentsRow->begin(); it != pargumentsRow->end() && stringPreview.length() < 60; ++it )
      {
         if( stringPreview.empty() == false ) stringPreview += ", ";
         stringPreview += it.get_argument(). as_string_view();
      }

      ptableKeyValue->cell_set( uRow, "preview", stringPreview );

      

      if( stringContext.empty() == false )
      {
         pdocument->MESSAGE_Display(stringContext, { array_, {{"color", "disabled"}}, gd::types::tag_view{} });
      }

      pdocument->MESSAGE_Display("");                                         // add a newline after each row to separate the key-value pairs
   }// for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow ) {

   // ## Final summary ........................................................

   // ### print keys used in the output to remind the user what keys are available when querying key-value pairs
   
   */


   {
      bool bFirst = true;
      std::string stringKeys( "Keys used: " );
      for( auto it = std::begin( vectorAll ); it != std::end( vectorAll ); it++ )
      {
         if( bFirst == false ) stringKeys += ", "; 
         bFirst = false;
         stringKeys += *it;
      }

      pdocument->MESSAGE_Display(stringKeys + "\n", {array_, {{"color", "default"}}, gd::types::tag_view{}});
   }


   // ### print summary of key-value pairs
   std::string stringSummary = std::format("Found {} sections with key-value pairs", ptableKeyValue->get_row_count());
   stringSummary = gd::math::string::format_header_line(stringSummary, uWidth, '#', '=', '#');
   pdocument->MESSAGE_Display(stringSummary, { array_, {{"color", "default"}}, gd::types::tag_view{} });

   pdocument->MESSAGE_Display();                                              // reset color to default


   // ## check if Visual Studio output is requested ..........................
   if( (*pargumentsPrint)["vs"].as_bool() == true )
   {

      //gd::table::dto::table tableVS( *ptableKeyValue );

      //gd::table::dto::table tableVS = gd::table::make_table<gd::table::dto::table>( *ptableKeyValue );
      
   }

   return { true, "" };                                                       // return success
}


/** ---------------------------------------------------------------------------
 * @brief Prints all rows from the key-value table in a basic/compact format.
 *
 * This static function formats and displays all rows from the key-value table in a basic/compact format,
 * including header, brief, and filename for each row.
 *
 * @param pdocument Pointer to a CDocument instance containing the key-value pairs.
 * @param ptableKeyValue Pointer to the table containing key-value pairs.
 * @param format Reference to kv_print structure containing formatting parameters.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> PrintKeyValueRowsBasic_s( CDocument* pdocument, gd::table::arguments::table* ptableKeyValue, const kv_print& kv_)
{                                                                                                  assert(pdocument != nullptr); assert(ptableKeyValue != nullptr);
   std::array<std::byte, 64> array_; // array to hold the color codes for the output
      std::string stringPrint; // string to hold the formatted output

   for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow )
   {
      stringPrint.clear();                                                    // clear the stringPrint for the next row

      // ### Prepare header and print line if vectorHeader is not empty

      const auto* pargumentsRow = ptableKeyValue->row_get_arguments_pointer(uRow); // get the arguments object from the row
      if( kv_.pvectorHeader != nullptr && kv_.pvectorHeader->empty() == false )
      {
         for( const auto& key_ : *kv_.pvectorHeader ) 
         { 
            auto stingValue = pargumentsRow->get_argument( key_ ).as_string_view(); 

            stringPrint += stingValue; 
            if( stingValue.empty() == false ) break;
         }
      }

      // ### Prepare and print brief line if vectorBrief is not empty

      if( kv_.pvectorBrief != nullptr && kv_.pvectorBrief->empty() == false )
      {
         for( const auto& key_ : *kv_.pvectorBrief ) 
         { 

            std::string stringBrief = pargumentsRow->get_argument( key_ ).as_string(); 

            // Limit brief to 40 characters
            if( stringBrief.length() > 40 ) stringBrief = stringBrief.substr( 0, 37 ) + "...";

            
            if( stringBrief.empty() == false ) 
            { 
               if( stringPrint.empty() == false ) stringPrint += " :: "; // separator.
               stringPrint += stringBrief;
               break;   
            }
         }
      }

      // ### Prepare and print file name

      std::string stringFile = ptableKeyValue->cell_get_variant_view(uRow, "filename").as_string(); // get the filename from the key-value table

      // Only print filename if different from previous row
      std::filesystem::path path_( stringFile );
      stringFile = path_.filename().string(); // get only the filename without path

      uint64_t uRowNumber = ptableKeyValue->cell_get_variant_view(uRow, "row").as_uint64(); // get the row number from the key-value table
      uRowNumber++;                                                           // add one because rows in table are zero based
      stringFile += std::format("({})", uRowNumber);                          // add the row number to the filename
      if( stringPrint.empty() == false ) stringPrint += " :: "; // separator.

      stringPrint += stringFile;

      pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "line"}}, gd::types::tag_view{} });
   }// for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow )

   pdocument->MESSAGE_Display("");                                            // add a newline after each row to separate the key-value pairs

   return { true, "" };
}



/** ---------------------------------------------------------------------------
 * @brief Prints all rows from the key-value table.
 *
 * This static function formats and displays all rows from the key-value table, 
 * including header, brief, filename, body key-value pairs, footer, and context
 * for each row. The function follows the display order: header, brief, filename, 
 * body, footer.
 *
 * @param pdocument Pointer to a CDocument instance containing the key-value pairs.
 * @param ptableKeyValue Pointer to the table containing key-value pairs.
 * @param format Reference to kv_print structure containing formatting parameters.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> PrintKeyValueRows_s( CDocument* pdocument, gd::table::arguments::table* ptableKeyValue, const kv_print& kv_)
{                                                                                                  assert(pdocument != nullptr); assert(ptableKeyValue != nullptr);
   std::array<std::byte, 64> array_; // array to hold the color codes for the output

   // ## Print values in the key-value table ..................................
   //    Print order for each row is:
   //    - header with header line
   //    - brief with brief line 
   //    - Source code file name with row number
   //    - body with key-value pairs
   //    - footer with footer line

   for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow )
   {
      std::string stringPrint; // string to hold the formatted output

      std::string stringContext; // context code if any, this is used to print the context code if any
      // ### Get context if set
      if( ptableKeyValue->cell_is_null( uRow, "context" ) == false )
      {
         stringContext = ptableKeyValue->cell_get_variant_view(uRow, "context").as_string();
      }

      // ### Prepare header and print line if vectorHeader is not empty

      const auto* pargumentsRow = ptableKeyValue->row_get_arguments_pointer(uRow); // get the arguments object from the row
      if( kv_.pvectorHeader != nullptr && kv_.pvectorHeader->empty() == false )
      {
         stringPrint.clear();                                                // clear the stringPrint for the next row
         for( const auto& key_ : *kv_.pvectorHeader ) 
         { 
            auto stingValue = pargumentsRow->get_argument( key_ ).as_string_view(); 
            if( stringPrint.empty() == false && stingValue.empty() == false ) stringPrint += ", "; // add a separator if the stringPrint is not empty

            stringPrint += stingValue; 
         }

         if( kv_.stringHeaderFormat.empty() == true ) { stringPrint = gd::math::string::format_header_line(stringPrint, kv_.uWidth); }
         else                                            { stringPrint = gd::math::string::format_header_line(stringPrint, gd::math::string::enumAlignment::eAlignmentLeft , kv_.uWidth, kv_.stringHeaderFormat); }

         pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "header"}}, gd::types::tag_view{} });
      }

      // ### Prepare and print brief line if vectorBrief is not empty

      if( kv_.pvectorBrief != nullptr && kv_.pvectorBrief->empty() == false )
      {
         stringPrint.clear();                                                // clear the stringPrint for the next row
         for( const auto& key_ : *kv_.pvectorBrief ) 
         { 
            if( stringPrint.empty() == false ) stringPrint += "\n";           // add a separator if the stringPrint is not empty

            std::string stringBrief = pargumentsRow->get_argument( key_ ).as_string(); 
            if( kv_.uTextWidth > 0 && stringBrief.empty() == false)
            {
               // format the value to fit in the width, with a margin for the key and separator
               auto uWidth = kv_.uTextWidth - 2 - kv_.stringBriefFormat.length();
               if( uWidth < 40 ) uWidth = 40;                                  // ensure the width is at least 40 characters, this is to avoid too narrow output
               stringBrief = gd::math::string::format_text_width(stringBrief, uWidth);
               stringBrief = gd::math::string::format_indent(stringBrief, kv_.stringBriefFormat.length(), false); // indent the value with the key margin width + 2 spaces because adds for separator and space after that
            }

            stringPrint += stringBrief;
         }

         if( stringPrint.empty() == false )
         {
            pdocument->MESSAGE_Display( std::string(kv_.stringBriefFormat) + stringPrint, {array_, {{"color", "brief"}}, gd::types::tag_view{}});
         }
      }

      // ### Prepare and print file name

      stringPrint = ptableKeyValue->cell_get_variant_view(uRow, "filename").as_string(); // get the filename from the key-value table
      uint64_t uRowNumber = ptableKeyValue->cell_get_variant_view(uRow, "row").as_uint64(); // get the row number from the key-value table
      uRowNumber++;                                                           // add one because rows in table are zero based
      stringPrint += std::format("({})", uRowNumber);                         // add the row number to the filename

      // If no header line then line is used as header line
      if( kv_.pvectorHeader == nullptr || kv_.pvectorHeader->empty() == true ) 
      { 
         stringPrint += std::format("{:-<80}", stringPrint + "  ");          // add the filename to the stringPrint, with a separator before it
      }

      pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "line"}}, gd::types::tag_view{} });

      // ### Prepare and print body

      if( pargumentsRow != nullptr && kv_.pvectorBody != nullptr )
      {
         stringPrint.clear();
         for( const auto& key_ : *kv_.pvectorBody )                        // iterate over the keys in the vectorBody
         {
            if( pargumentsRow->exists(key_) == false ) continue;              // if the key does not exist in the arguments object, skip this key-value pair

            if( stringPrint.empty() == false ) stringPrint += '\n';

            auto stringValue_ = pargumentsRow->get_argument(key_).as_string();// get the value of the argument

            if( kv_.uTextWidth > 0 )
            {
               // format the value to fit in the width, with a margin for the key and separator
               stringValue_ = gd::math::string::format_text_width(stringValue_, kv_.uTextWidth - kv_.uKeyMarginWidth - 2); 
            }

            if( stringValue_.find('\n') != std::string::npos ) // if the value contains a newline, indent it
            {
               stringValue_ = gd::math::string::format_indent(stringValue_, kv_.uKeyMarginWidth + 2, false); // indent the value with the key margin width + 2 spaces because adds for separator and space after that
            }
            // Print name with padding
            stringPrint += std::format("{:>{}}: {}", key_, kv_.uKeyMarginWidth, stringValue_); // format the key-value pair as "key: value" with padding
         }

         if( stringPrint.empty() == false ) pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "body"}}, gd::types::tag_view{} });
      }

      // ### Prepare and print footer

      if( kv_.pvectorFooter != nullptr && kv_.pvectorFooter->empty() == false )
      {
          stringPrint.clear();                                                // clear the stringPrint for the next row
         for( const auto& key_ : *kv_.pvectorFooter ) 
         { 
            auto stingValue = pargumentsRow->get_argument( key_ ).as_string_view(); 
            if( stringPrint.empty() == false && stingValue.empty() == false ) stringPrint += ", "; // add a separator if the stringPrint is not empty

            stringPrint += stingValue; 
         }
         if( kv_.stringFooterFormat.empty() == true ) { stringPrint = gd::math::string::format_header_line(stringPrint, gd::math::string::enumAlignment::eAlignmentRight, kv_.uWidth); }
         else                                            { stringPrint = gd::math::string::format_header_line(stringPrint, gd::math::string::enumAlignment::eAlignmentRight, kv_.uWidth, kv_.stringFooterFormat); }
         pdocument->MESSAGE_Display(stringPrint, { array_, {{"color", "footer"}}, gd::types::tag_view{} });
      }

      // ## Prepare preview part

      std::string stringPreview;
      for( auto it = pargumentsRow->begin(); it != pargumentsRow->end() && stringPreview.length() < 60; ++it )
      {
         if( stringPreview.empty() == false ) stringPreview += ", ";
         stringPreview += it.get_argument().as_string_view();
      }

      ptableKeyValue->cell_set( uRow, "preview", stringPreview );

      

      if( stringContext.empty() == false )
      {
         pdocument->MESSAGE_Display(stringContext, { array_, {{"color", "disabled"}}, gd::types::tag_view{} });
      }

      pdocument->MESSAGE_Display("");                                         // add a newline after each row to separate the key-value pairs
   }// for( auto uRow = 0u; uRow < ptableKeyValue->get_row_count(); ++uRow )

   return { true, "" };
}

NAMESPACE_CLI_END
