/**
* @file CLIHistory.cpp
*/

#include <format>

#include "gd/gd_uuid.h"
#include "gd/gd_file.h"
#include "gd/math/gd_math_string.h"
#include "gd/table/gd_table_formater.h"

#include "CLI_Shared.h"


#include "../Command.h"
#include "../Application.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


#include "CLIDir.h"


// @TAG #cli #dir


NAMESPACE_CLI_BEGIN

void CountLevel_s(gd::table::dto::table* ptable_);

// ## Dir operations

std::pair<bool, std::string> Dir_g(const gd::cli::options* poptionsDir, CDocument* pdocument)
{                                                                                                  assert( poptionsDir != nullptr );

   if(pdocument->PROPERTY_Exists("detail") == false) { pdocument->PROPERTY_UpdateFromApplication(); }

   const gd::cli::options& options_ = *poptionsDir;

   gd::argument::shared::arguments argumentsFileHarvest;
   SHARED_ReadHarvestSetting_g( options_, argumentsFileHarvest, pdocument );
   argumentsFileHarvest.append( options_.get_arguments(), {"segment"});
   argumentsFileHarvest.append("size", true);                                 // always get size

	// ## perform the pattern operation if found ..............................

   if( options_.exists("pattern") == true )                                    // 
   {
      std::vector<std::string> vectorPattern; // vector to store patterns
      vectorPattern = options_.get_arguments().get_all<std::string>("pattern"); // get all patterns from options and put them into vectorPattern
      auto result_ = DirPattern_g( vectorPattern, argumentsFileHarvest, pdocument );
      if( result_.first == false ) return result_;
   }
   else if( options_.exists("rpattern") == true )
   {
      auto vectorRPattern = options_.get_all("rpattern"); // get all regex patterns
      std::vector<std::string> vectorPattern; // store regex patterns as strings
      for( auto& rpattern : vectorRPattern ) { vectorPattern.push_back(rpattern.as_string()); }

      vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
      if( vectorPattern.size() == 0 ) return {false, "No regex patterns provided."}; // if no patterns are provided, return an error

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

      auto result_ = DirPattern_g( vectorRegexPattern, argumentsFileHarvest, pdocument );
      if( result_.first == false ) return result_;
   }
   else if( options_.exists("vs") == true || options_.exists("script") == true )
   {
      gd::argument::shared::arguments arguments_(argumentsFileHarvest); // start with harvest arguments
      if( options_.exists("vs") == true ) arguments_.append( "vs", true );
      if( options_.exists("script") == true ) arguments_.append( "script", options_["script"].as_string() );

      auto result_ = DirFilter_g( arguments_, pdocument );
      if( result_.first == false ) return result_;
   }
   else
   {
      auto result_ = DirFilter_g( argumentsFileHarvest, pdocument );
      if( result_.first == false ) return result_;
   }

	// ## if where and sort then filter and sort result .......................

   auto value_ = options_.get_variant_view( "where", gd::cli::options::tag_optional{});
   if( value_.has_value() == true )
   {
      auto result_ = pdocument->CACHE_Where("file-dir", value_.value().as_string_view());// filter table based on where condition
                                                                                                   LOG_ERROR_RAW_IF( result_.first == false, std::format("Error in where: {}, error: {}", value_.value().as_string_view(), result_.second) );
      if(result_.first == false) return result_;
   }

   value_ = options_.get_variant_view( "sort", gd::cli::options::tag_optional{});
   if( value_.has_value() == true )
   {
      auto result_ = pdocument->CACHE_Sort("file-dir", value_.value().as_string_view());// sort the table by the given sort value
      if(result_.first == false) return result_;
   }

   // ## print the result ....................................................

   value_ = options_.get_variant_view( "compact", gd::cli::options::tag_optional{});
   if( value_.has_value() == true && value_.value() == true )
   {
      gd::argument::shared::arguments arguments_(options_.get_arguments(), {"parents"});
      DirPrintCompact_g(pdocument, arguments_);                               // print the table similar to ls
   }
   else
   {
      DirPrint_g(pdocument);                                                  // print the table to the console
   }

#ifdef _WIN32
   // ## VS specific output .................................................
   if( options_.exists("vs") == true )
   {
      auto result_ = DirPrintToVS_g(  pdocument, options_.get_arguments() );
      if( result_.first == false ) return result_;
   }
#endif


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief
 *   Performs a directory search using the specified source path and filter, then applies one or more patterns to the results.
 *
 *   - Harvests files from the given directory path using the provided filter and search depth.
 *   - For each file found, checks if it matches any of the specified patterns.
 *   - Removes files from the result set that do not match any pattern.
 *   - Displays the filtered result table to the user.
 *
 * @param vectorPattern The vector of patterns to match against.
 * @param arguments_   Arguments containing "filter", "depth", and "pattern" keys.
 * @param pdocument    Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> DirPattern_g( const std::vector<std::string>& vectorPattern, const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{                                                                                                  assert( vectorPattern.empty() == false );
   auto ptable = pdocument->CACHE_Get( "file-dir", true );
   auto result_ = FILES_Harvest_g( arguments_, ptable);      if( result_.first == false ) return result_;
   CountLevel_s(ptable);

   std::string stringSegment = arguments_["segment"].as_string();

   gd::table::dto::table* ptableFile = ptable;                                // get the table pointer
   std::vector<uint64_t> vectorCount; // vector storing results from COMMAND_CollectPatternStatistics
   std::vector<uint64_t> vectorDeleteRow; 

   for( const auto& itRowFile : *ptableFile )
   {
      std::string stringFile = itRowFile.cell_get_variant_view("path").as_string(); // get the file path

      // ## Match the pattern/patterns with the file
      
      gd::argument::shared::arguments argumentsDir( {{"source", stringFile} } );
      if( stringSegment.empty() == false ) argumentsDir.append("segment", stringSegment); // if segment is set, add it to the arguments
      auto result_ = COMMAND_CollectPatternStatistics( argumentsDir, vectorPattern, vectorCount );
      if( result_.first == false ) { pdocument->ERROR_Add(result_.second); }

      // ## Check for pattern matches, vector contains the number of matches for each pattern

      bool bPatternMatch = false;
      for( unsigned u = 0; u < vectorCount.size(); u++ )
      {
         if( vectorCount[u] > 0 ) { bPatternMatch = true; break; }
      }

      if( bPatternMatch == false )
      {
         vectorDeleteRow.push_back(itRowFile.get_row());                       // add the row index to the delete vector
      }

      vectorCount.resize(vectorPattern.size(), 0);                             // set counters to 0 in vector
   }

   if( vectorDeleteRow.empty() == false )                                      // if the vector is not empty, delete the rows
   {
      ptableFile->erase(vectorDeleteRow);                                      // delete the rows from the table
      // ## fix the row numbers
      for( uint64_t uRow = 0; uRow < ptableFile->get_row_count(); uRow++ )
      {
         ptableFile->cell_set(uRow, "key", uRow + 1);                          // set new key numbers to make key work for user, like index for each file
      }
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief
 *   Performs a directory search using the specified source path and filter, then applies one or more patterns to the results.
 *
 *   - Harvests files from the given directory path using the provided filter and search depth.
 *   - For each file found, checks if it matches any of the specified patterns.
 *   - Removes files from the result set that do not match any pattern.
 *   - Displays the filtered result table to the user.
 *
 * @param vectorRegexPattern The vector of regex patterns to match against.
 * @param arguments_   Arguments containing "filter", "depth", and "pattern" keys.
 * @param pdocument    Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> DirPattern_g( const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPattern, const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{
   auto ptable = pdocument->CACHE_Get( "file-dir", true );
   auto result_ = FILES_Harvest_g( arguments_, ptable);                                            if( result_.first == false ) return result_;

   CountLevel_s(ptable);

   std::string stringSegment = arguments_["segment"].as_string();

   gd::table::dto::table* ptableFile = ptable;                                // get the table pointer
   std::vector<uint64_t> vectorCount; // vector storing results from COMMAND_CollectPatternStatistics
   std::vector<uint64_t> vectorDeleteRow; 

   for( const auto& itRowFile : *ptableFile )
   {
      std::string stringFile = itRowFile.cell_get_variant_view("path").as_string(); // get the file path

      // ## Match the pattern/patterns with the file
      
      gd::argument::shared::arguments argumentsDir( {{"source", stringFile} } );
      if( stringSegment.empty() == false ) argumentsDir.append("segment", stringSegment); // if segment is set, add it to the arguments
      auto result_ = COMMAND_CollectPatternStatistics( argumentsDir, vectorRegexPattern, vectorCount );
      if( result_.first == false ) { pdocument->ERROR_Add(result_.second); }

      // ## Check for pattern matches, vector contains the number of matches for each pattern

      bool bPatternMatch = false;
      for( unsigned u = 0; u < vectorCount.size(); u++ )
      {
         if( vectorCount[u] > 0 ) { bPatternMatch = true; break; }
      }

      if( bPatternMatch == false )
      {
         vectorDeleteRow.push_back(itRowFile.get_row());                       // add the row index to the delete vector
      }

      vectorCount.resize(vectorRegexPattern.size(), 0);                             // set counters to 0 in vector
   }

   if( vectorDeleteRow.empty() == false )                                      // if the vector is not empty, delete the rows
   {
      ptableFile->erase(vectorDeleteRow);                                      // delete the rows from the table
      // ## fix the row numbers
      for( uint64_t uRow = 0; uRow < ptableFile->get_row_count(); uRow++ )
      {
         ptableFile->cell_set(uRow, "key", uRow + 1);                          // set new key numbers to make key work for user, like index for each file
      }
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Similar to the standard dir command with a filter and optional script execution
 * @param stringSource source path or paths, separated by a ;
 * @param arguments_ arguments containing "filter", "depth", and optional "script" keys
 * @param pdocument pointer to the document object where data is stored
 * @return a pair of bool and string, where the bool indicates success or failure, and the string contains the error message or result
 */
std::pair<bool, std::string> DirFilter_g( const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{
   auto ptable = pdocument->CACHE_Get( "file-dir", true );

   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();
   std::string stringSource = arguments_["source"].as_string();

   auto result_ = FILES_Harvest_g( stringSource, stringFilter, ptable, uDepth, true);
   if( result_.first == false ) return result_;

	CountLevel_s(ptable);

#ifdef _WIN32
   if( arguments_.exists("script") == true )
   {
      std::string stringScript = arguments_["script"].as_string();
      VS::CVisualStudio visualstudio;
      result_ = visualstudio.Connect();
      if( result_.first == true )
      {
         visualstudio.AddTable( ptable );
         result_ = visualstudio.ExecuteExpression( stringScript );
      }
      if( result_.first == false ) return result_;
   }
#endif // _WIN32

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Prints the directory table to the console
 * @param pdocument pointer to the document object where data is stored
 * @return a pair of bool and string, where the bool indicates success or failure, and the string contains the error message or result
 */
std::pair<bool, std::string> DirPrint_g(CDocument* pdocument)
{
   const auto* ptable_ = pdocument->CACHE_Get("file-dir");                                         assert(ptable_ != nullptr);

   auto stringTable = gd::table::to_string( *ptable_, { {"verbose", true} }, gd::table::tag_io_cli{});
   pdocument->MESSAGE_Display(stringTable);

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prints the directory table in a compact format similar to the 'ls' command.
 * 
 * @verbatim
 * folder...
 * file    file    file    file
 * file    file    file    file
 * @endverbatim
 * @param pdocument pointer to the document object where data is stored
 * @param arguments_ additional arguments for formatting
 * @return a pair of bool and string, where the bool indicates success or failure, and the string contains the error message or result
 */
std::pair<bool, std::string> DirPrintCompact_g( CDocument* pdocument, const gd::argument::shared::arguments& arguments_ )
{
   using namespace gd::table::dto;
   unsigned uParents = 0;
   std::string stringCurrentFolder; // hold the current folder path

   auto* ptable_ = pdocument->CACHE_Get("file-dir");                                         assert(ptable_ != nullptr);

   if( arguments_.exists("parents") == true ) { uParents = arguments_["parents"].as_uint(); }
   uParents = std::min( (unsigned)10, uParents );

   // ## Create new table to match what to print that work as ls command .............
   gd::table::dto::table tableLS( ( table::eTableFlagNull32 | table::eTableFlagRowStatus ), { { "string", 200, "name"}, { "uint64", 0, "size"}, { "rstring", 0, "folder"} }, gd::table::tag_prepare{} );
   gd::file::path pathFile;
   for( const auto& itRow : *ptable_ )
   {
      std::string stringPath = itRow.cell_get_variant_view("path").as_string();
      pathFile = stringPath;
      auto uRow = tableLS.row_add_one();
      std::string stringName = pathFile.filename().string();
      if( uParents > 0 )
      {
         /// ## prepend parent folders to the name ...........................
         gd::file::path pathTemp = pathFile;
         std::vector<std::string> vectorParentFolders;

         // Collect parent folders
         for( unsigned uCount = 0; uCount < uParents; ++uCount )
         {
            gd::file::path pathParent = pathTemp.parent_path();
            if( pathParent == pathTemp ) break;
            pathTemp = std::move( pathParent );
            vectorParentFolders.push_back( pathTemp.filename().string() );
         }

         // ## Prepend in reverse order (closest parent first)
         for( auto it = vectorParentFolders.begin(); it != vectorParentFolders.end(); ++it )
         {
            stringName = *it + "/" + stringName;
         }
      }
      tableLS.cell_set(uRow, 0, stringName);

      stringPath = pathFile.parent_path().string();
      if( stringPath != stringCurrentFolder )
      {
         stringCurrentFolder = stringPath;
         tableLS.cell_set(uRow, 2, stringCurrentFolder);
      }
   }
   

   std::string stringFiles;
   std::string stringFolder;
   std::array<std::byte, 64> array_; // array to hold data for arguments

   uint64_t uRowFrom = 0;
   for( const auto& itRow : tableLS )
   {
      uint64_t uRow = itRow.get_row();
      if( uRow != (tableLS.size() - 1) && itRow.cell_get_variant_view("folder").is_null() == true ) continue;

      // ## if no folder then just get folder and continue ....................
      if( stringFolder.empty() == true ) 
      { 
         stringFolder = itRow.cell_get_variant_view("folder").as_string(); 
         if( ( uRow + 1 ) < tableLS.size() ) continue;                          // if not last row then continue
      }

      uint64_t uCount = uRow - uRowFrom + 1;

      // ## print files ......................................................
      pdocument->MESSAGE_Display(stringFolder, { array_, {{"color", "header"}}, gd::types::tag_view{} });

      auto uTerminalWidth = (unsigned)SHARED_GetTerminalWidth();

      stringFiles = gd::table::format::to_string(tableLS, uRowFrom, uCount, { "name" }, uTerminalWidth, gd::argument::arguments{ { "border", false }, { "row-space", 0 } }, gd::types::tag_card{});
      pdocument->MESSAGE_Display(stringFiles, { array_, {{"color", "body"}}, gd::types::tag_view{} });
      uRowFrom = uRow;
      stringFolder = itRow.cell_get_variant_view("folder").as_string();
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Prints the directory table to the Visual Studio output window
 * @param pdocument pointer to the document object where data is stored
 * @return a pair of bool and string, where the bool indicates success or failure, and the string contains the error message or result
 */
std::pair<bool, std::string> DirPrintToVS_g(CDocument* pdocument, const gd::argument::arguments& arguments_)
{
#ifdef _WIN32
   VS::CVisualStudio visualstudio;
   auto result_ = visualstudio.Connect();
   if( result_.first == false ) return result_;


   auto* ptable_ = pdocument->CACHE_Get("file-dir");                                         assert(ptable_ != nullptr);

   std::string stringPrintToVSOutput;
   for( const auto& itRow : *ptable_ )
   {
      std::string stringPath = itRow.cell_get_variant_view("path").as_string();
      stringPrintToVSOutput += stringPath + "\n";
   }

   result_ = visualstudio.Print(stringPrintToVSOutput, VS::tag_vs_output{});
   if( result_.first == false ) return result_;
#endif
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Counts the level (number of parent folders) of each file in the directory structure.
 * @param ptable_ pointer to the table object containing file information
 */
void CountLevel_s(gd::table::dto::table* ptable_)
{
   if( ptable_ == nullptr ) return;
   char iCharacter = 0;
   for(auto it : *ptable_)
   {
      std::string_view stringPath = it.cell_get_variant_view("path").as_string_view();
      if(iCharacter == 0)
      {
         // try to find out character used for path separator
         if (gd::math::string::count_character(stringPath, '\\') > 0) iCharacter = '\\';
         else if (gd::math::string::count_character(stringPath, '/') > 0) iCharacter = '/';
      }
      auto uCount = gd::math::string::count_character(stringPath, iCharacter);
      it.cell_set("level", (int32_t)uCount );                                    // set the level of the file in the directory structure
   }
}



NAMESPACE_CLI_END