/**
* @file CLIHistory.cpp
*/

#include <format>

#include "gd/gd_uuid.h"

#include "../Command.h"
#include "../Application.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


#include "CLIDir.h"


// @TAG #cli #dir


NAMESPACE_CLI_BEGIN

// ## Dir operations

std::pair<bool, std::string> Dir_g(const gd::cli::options* poptionsDir, CDocument* pdocument)
{                                                                                                  assert( poptionsDir != nullptr );
   const gd::cli::options& options_ = *poptionsDir;
   std::string stringSource = (*poptionsDir)["source"].as_string(); 
   CApplication::PreparePath_s(stringSource);                                  // if source is empty then set it to current path, otherwiss prepare it

   std::string stringIgnore = options_["ignore"].as_string();
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }


   // std::string stringTableResultId = gd::uuid(gd::uuid::tag_random{}).to_string(); // create a random id for the table result

   unsigned uRecursive = options_["recursive"].as_uint();
   if(uRecursive == 0 && options_.exists("R") == true) uRecursive = 16;        // set to 16 if R is set, find all files

   pdocument->GetApplication()->UpdateApplicationState();

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" || stringFilter == "." || stringFilter == "**" ) 
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( uRecursive == 0 ) uRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

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

   if( options_.exists("sort") == true )
   {
      pdocument->CACHE_Sort("file-dir", options_["sort"]);                    // sort the table by the given sort value
   }

   DirPrint_g(pdocument);                                                     // print the table to the console


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
   //std::unique_ptr<gd::table::dto::table> ptable;
   auto ptable = pdocument->CACHE_Get( "file-dir", true );
   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();
   auto result_ = FILES_Harvest_g( stringSource, stringFilter, ptable, uDepth, true);        if( result_.first == false ) return result_;

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




NAMESPACE_CLI_END