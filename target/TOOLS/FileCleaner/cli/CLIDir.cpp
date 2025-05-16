/**
* @file CLIHistory.cpp
*/


#include "../Application.h"

#include "CLIDir.h"




NAMESPACE_CLI_BEGIN

// ## Dir operations

std::pair<bool, std::string> Dir_g(const gd::cli::options* poptionsDir)
{
   std::string stringSource = (*poptionsDir)["source"].as_string(); 

   // ## Check if the source is empty and if so set it to the current directory
   if( stringSource.empty() == true ) { stringSource = std::filesystem::current_path().string(); }

   if( std::filesystem::exists(stringSource) == false )
   {
      return { false, std::format( "The source \"{}\" path does not exist", stringSource ) };
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
   std::cout << stringTable << "\n";

   return { true, "" };
}


NAMESPACE_CLI_END