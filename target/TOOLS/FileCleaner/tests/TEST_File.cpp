#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"

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
TEST_CASE( "[file] load file into document", "[file]" ) {
   CApplication application;
   application.Initialize();
   std::string stringDataFolder = GetDataFolder();

   for( auto i = 0; i < 10; i++ ) 
   {
      auto stringName = GENERATE_RandomName( 10 + i );
      application.DOCUMENT_Add( stringName );

      std::string stringFile = stringDataFolder + "/python.txt";
      gd::file::path pathFile(stringFile);
      auto* pdocument = application.DOCUMENT_Get(stringName);
      pdocument->Load( pathFile );
   }

   for( auto it = application.DOCUMENT_Begin(); it != application.DOCUMENT_End(); it++ )
   {
      std::cout << "Document: " << ( *it )->GetName() << " and number of lines are: " << ( *it )->Count('\n') << std::endl;
   }

   application.DOCUMENT_Clear();
}

TEST_CASE( "[file] passing arguments", "[file]" ) {

}