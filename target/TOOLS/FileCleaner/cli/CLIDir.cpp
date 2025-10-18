/**
* @file CLIHistory.cpp
*/

#include <format>

#include "gd/gd_uuid.h"
#include "gd/gd_file.h"
#include "gd/math/gd_math_string.h"
#include "gd/table/gd_table_formater.h"


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
   std::string stringSource = ( *poptionsDir )["source"].as_string();                              LOG_DEBUG_RAW_IF( stringSource.empty() == false, "Source: " & stringSource);
   CApplication::PreparePath_s(stringSource);                                  // if source is empty then set it to current path, otherwiss prepare it

   std::string stringIgnore = options_["ignore"].as_string();                                      LOG_DEBUG_RAW_IF( stringIgnore.empty() == false, "Ignore: " & stringIgnore);
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }

	// ## get recursive value .................................................

   unsigned uRecursive = options_["recursive"].as_uint();
   if(uRecursive == 0 && options_.exists("R") == true)
   {
      if(options_["R"].is_bool() == true) uRecursive = 16;                    // set to 16 if R is set, find all files
      else { uRecursive = options_["R"].as_uint(); }
   }

   pdocument->GetApplication()->UpdateApplicationState();

	// ## get filter value ....................................................

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" || stringFilter == "." || stringFilter == "**" ) 
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( uRecursive == 0 ) uRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

	// ## perform the pattern operation if found ..............................

   if( options_.exists("pattern") == true )                                    // 
   {
      gd::argument::shared::arguments arguments_( { { "depth", uRecursive }, { "filter", stringFilter }, { "pattern", options_["pattern"].as_string() }, { "segment", options_["segment"].as_string() } });
      auto result_ = DirPattern_g( stringSource, arguments_, pdocument );
   }
   else if( options_.exists("rpattern") == true )
   {

   }
   else if( options_.exists("vs") == true || options_.exists("script") == true )
   {
      gd::argument::shared::arguments arguments_( { { "depth", uRecursive }, { "filter", stringFilter }});

      if( options_.exists("vs") == true ) arguments_.append( "vs", true );
      if( options_.exists("script") == true ) arguments_.append( "script", options_["script"].as_string() );

      auto result_ = DirFilter_g( stringSource, arguments_, pdocument );
      if( result_.first == false ) return result_;
   }
   else
   {
      auto result_ = DirFilter_g( stringSource, stringFilter, uRecursive, pdocument );
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
      gd::argument::shared::arguments arguments_;
      DirPrintCompact_g(pdocument, arguments_);                               // print the table similar to ls
   }
   else
   {
      DirPrint_g(pdocument);                                                  // print the table to the console
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
 * @param stringSource The source directory path to search.
 * @param arguments_   Arguments containing "filter", "depth", and "pattern" keys.
 * @param pdocument    Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> DirPattern_g( const std::string& stringSource, const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{                                                                                                  assert( stringSource != "" );
   auto ptable = pdocument->CACHE_Get( "file-dir", true );
   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();
   auto result_ = FILES_Harvest_g( stringSource, stringFilter, ptable, uDepth, true);        if( result_.first == false ) return result_;

   CountLevel_s(ptable);

   std::string stringSegment = arguments_["segment"].as_string();

   gd::table::dto::table* ptableFile = ptable;                                // get the table pointer
   std::vector<uint64_t> vectorCount; // vector storing results from COMMAND_CollectPatternStatistics
   std::vector<uint64_t> vectorDeleteRow; 

   // ## Filter the table with the pattern or patterns sent
   auto stringPattern = arguments_["pattern"].as_string();
   auto vectorPattern = CApplication::Split_s(stringPattern);

   for( const auto& itRowFile : *ptableFile )
   {
      std::string stringFile = itRowFile.cell_get_variant_view("path").as_string(); // get the file path

      // ## Match the pattern/patterns with the file
      
      gd::argument::shared::arguments argumentsdir( {{"source", stringFile} } );
      if( stringSegment.empty() == false ) argumentsdir.append("segment", stringSegment); // if segment is set, add it to the arguments
      auto result_ = COMMAND_CollectPatternStatistics( argumentsdir, vectorPattern, vectorCount );
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

std::pair<bool, std::string> DirFilter_g( const std::string& stringSource, const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{                                                                                                  assert( stringSource != "" );
   auto ptable = pdocument->CACHE_Get( "file-dir", true );

   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();

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
 * @brief Similar to the standard dir command with a filter
 * @param stringSource source path or paths, separated by a ;
 * @param stringFilter filter string, e.g. *.txt;*.docx
 * @param uDepth depth of the search, 0 = current directory, 1 = subdirectory, 2 = sub-subdirectory, etc.
 * @param pdocument pointer to the document object where data is stored
 * @return a pair of bool and string, where the bool indicates success or failure, and the string contains the error message or result
 */
std::pair<bool, std::string> DirFilter_g(const std::string& stringSource, const std::string& stringFilter, unsigned uDepth, CDocument* pdocument )
{                                                                                                  assert( stringSource != "" );
   std::unique_ptr<gd::table::dto::table> ptable;
   pdocument->CACHE_Prepare( "file-dir", &ptable );

   auto result_ = FILES_Harvest_g( stringSource, stringFilter, ptable.get(), uDepth, true );
   if( result_.first == false ) return result_;

	// count level if column exists
   if(ptable->column_exists("level") == true) { CountLevel_s(ptable.get()); }

   pdocument->CACHE_Add( std::move( ptable ) );
   
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

std::pair<bool, std::string> DirPrintCompact_g( CDocument* pdocument, const gd::argument::shared::arguments& arguments_ )
{
   auto* ptable_ = pdocument->CACHE_Get("file-dir");                                         assert(ptable_ != nullptr);

   // ## Create new table to match what to print that work as ls command .............
   gd::table::dto::table tableLS( 0, { { "string", 200, "name"}, { "uint64", 0, "size"} }, gd::table::tag_prepare{} );
   gd::file::path pathFile;
   for( const auto& itRow : *ptable_ )
   {
      std::string stringPath = itRow.cell_get_variant_view("path").as_string();
      pathFile = stringPath;
      auto uRow = tableLS.row_add_one();
      tableLS.cell_set(uRow, 0, pathFile.filename().string());
      //tableLS.cell_set(uRow, "size", uSize);
   }
   

   unsigned uColumnPath = ptable_->column_get_index("path");

   std::string stringOutput;
   stringOutput = gd::table::format::to_string(tableLS, {0}, 3, gd::argument::arguments{ { "border", false }, { "row-space", 0 } }, gd::types::tag_card{});

   pdocument->MESSAGE_Display(stringOutput);

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