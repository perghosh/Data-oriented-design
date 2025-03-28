#include <fstream>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"
#include "gd/io/gd_io_repository_stream.h"

#include "main.h"

#include "../Document.h"
#include "../Application.h"

#include "catch2/catch_amalgamated.hpp"

/// Generate path to data folder where files are located for tests
std::string GetDataFolder()
{
   return FOLDER_GetRoot_g("target/TOOLS/FileCleaner/tests/data");
}

/// Test to create and delete documents
TEST_CASE( "[repository] create repository01", "[repository]" ) {
   CApplication application;
   application.Initialize();
   std::string stringDataFolder = GetDataFolder();

   std::string stringFile = stringDataFolder + "/repository.repo";
   gd::file::path pathFile(stringFile);
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   gd::io::stream::repository repositoryStream;

   auto result_ = repositoryStream.create(pathFile);                                               REQUIRE(result_.first == true);

   stringFile = stringDataFolder + "/readme.md";
   result_ = repositoryStream.add(stringFile);                                                     REQUIRE(result_.first == true);
   result_ = repositoryStream.add(stringFile);                                                     REQUIRE(result_.first == true);
   result_ = repositoryStream.add(stringFile);                                                     REQUIRE(result_.first == true);
   result_ = repositoryStream.add(stringFile);                                                     REQUIRE(result_.first == true);
   result_ = repositoryStream.add(stringFile);                                                     REQUIRE(result_.first == true);

   repositoryStream.remove( 1 );
   repositoryStream.remove( 2 );
   repositoryStream.remove( 3 );

   repositoryStream.remove_entry_from_file();

   auto list_ = repositoryStream.list();
   for( auto& stringName : list_ )
   {
      std::cout << "File: " << stringName << std::endl;
   }

   repositoryStream.flush();
}
