/**
* @file CLIHistory.cpp
*/

#include <format>

#include "../Application.h"

#include "CLIDir.h"




NAMESPACE_CLI_BEGIN

// ## Dir operations

std::pair<bool, std::string> Dir_g(const gd::cli::options* poptionsDir, CDocument* pdocument)
{                                                                                                  assert( poptionsDir != nullptr );
   const gd::cli::options& options_ = *poptionsDir;
   std::string stringSource = (*poptionsDir)["source"].as_string(); 
   CApplication::PathPrepare_s(stringSource);                                  // if source is empty then set it to current path, otherwiss prepare it

   unsigned uRecursive = options_["recursive"].as_uint();
   if(uRecursive == 0 && options_.exists("R") == true) uRecursive = 16;        // set to 16 if R is set, find all files

   if( options_.exists("pattern") == true )
   {
      gd::argument::shared::arguments arguments_( { { "depth", uRecursive }, { "filter", options_["filter"].as_string() }, { "pattern", options_["pattern"].as_string() }});
      
   }
   else if( options_.exists("rpattern") == true )
   {

   }
   else if( options_.exists("vs") == true || options_.exists("script") == true )
   {
      gd::argument::shared::arguments arguments_( { { "depth", uRecursive }, { "filter", options_["filter"].as_string() }});

      if( options_.exists("vs") == true ) arguments_.append( "vs", true );
      if( options_.exists("script") == true ) arguments_.append( "script", options_["script"].as_string() );

      auto result_ = DirFilter_g( stringSource, arguments_, pdocument );
   }
   else
   {
      std::string stringFilter = options_["filter"].as_string();
      auto result_ = DirFilter_g( stringSource, stringFilter, uRecursive, pdocument );
   }

   auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "path"} }, gd::table::tag_prepare{}));

   for( const auto& it : std::filesystem::directory_iterator(stringSource) )
   {
      if( it.is_regular_file() || it.is_directory() )
      {
         std::string stringFilePath = it.path().string();
         ptable->row_add();
         ptable->cell_set(ptable->get_row_count() - 1, "path", stringFilePath);
      }
   }

   auto stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   application.PrintMessage(stringTable, gd::argument::arguments());
   */

   return { true, "" };
}

std::pair<bool, std::string> DirPattern_g( const std::string& stringSource, const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{                                                                                                  assert( stringSource != "" );
   std::unique_ptr<gd::table::dto::table> ptable;
   pdocument->CACHE_Prepare( "file-dir", &ptable );
   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();
   auto result_ = FILES_Harvest_g( stringSource, stringFilter, ptable.get(), uDepth);
   if( result_.first == false ) return result_;

   auto stringPattern = arguments_["pattern"].as_string();

   return { true, "" };
}


NAMESPACE_CLI_END