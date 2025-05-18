/**
* @file CLIHistory.cpp
*/

// @TAG #cli


#include "../Command.h"
#include "../Application.h"

#ifdef _WIN32
#  include "../win/VS_Command.h"
#endif


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

   }
   else if( options_.exists("rpattern") == true )
   {

   }
   else
   {
      std::string stringFilter = options_["filter"].as_string();
      auto result_ = DirFilter_g( stringSource, stringFilter, uRecursive, pdocument );
   }
/*

   auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "path"} }, gd::table::tag_prepare{}));

   // Add to table
   for( const auto& it : std::filesystem::directory_iterator(stringSource) )
   {
      if( it.is_regular_file() || it.is_directory() )
      {
         std::string stringFilePath = it.path().string();

         if( (*poptionsDir)["filter"].is_true() == true )
         {
            std::string stringFilter = (*poptionsDir)["filter"].as_string();
            std::string stringExtension = it.path().extension().string();

            if( stringFilter == stringExtension )
            {
               ptable->row_add();
               ptable->cell_set(ptable->get_row_count() - 1, "path", stringFilePath);
            }
         }
         else
         {
            ptable->row_add();
            ptable->cell_set(ptable->get_row_count() - 1, "path", stringFilePath);
         }
      }
   }

   auto stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   application.PrintMessage(stringTable, gd::argument::arguments());
   */

   return { true, "" };
}

std::pair<bool, std::string> DirFilter_g(const std::string& stringSource, const std::string& stringFilter, unsigned uDepth, CDocument* pdocument )
{                                                                                                  assert( stringSource != "" );
   std::unique_ptr<gd::table::dto::table> ptable;
   pdocument->CACHE_Prepare( "file-dir", &ptable );

   FILES_Harvest_g( stringSource, stringFilter, ptable.get(), uDepth);
   auto stringTable = gd::table::to_string(*ptable.get(), { {"verbose", true} }, gd::table::tag_io_cli{});
   pdocument->MESSAGE_Display(stringTable);

   return { false, "" };
}


NAMESPACE_CLI_END