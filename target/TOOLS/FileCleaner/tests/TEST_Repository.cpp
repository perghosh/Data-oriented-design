#include <chrono>
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

std::string get_current_time_as_string() 
{
   auto now = std::chrono::system_clock::now();
   std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
   std::tm now_tm = *std::localtime(&now_time_t);

   std::ostringstream oss;
   oss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
   return oss.str();
}


TEST_CASE( "[repository] create and read", "[repository]" ) {
   CApplication application;
   application.Initialize();
   std::string stringDataFolder = GetDataFolder();

   std::string stringFile = stringDataFolder + "/repository.repo";
   gd::file::path pathFile(stringFile);
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);


   gd::io::stream::repository repositoryStream(stringFile, 20);

   repositoryStream.create();
   for( auto i = 0; i < 15; i++ )
   {
      std::string stringFile = stringDataFolder + "/readme.md";
      std::string stringName = "readme" + std::to_string(i) + ".md";
      auto result_ = repositoryStream.add(stringFile, stringName);                                 REQUIRE(result_.first == true);
   }

   {
      std::string stringTemporary;
      gd::io::stream::repository::file_new_tempoary_s(repositoryStream, stringTemporary, false);
      std::cout << "Temporary file: " << stringTemporary << std::endl;
   }

   repositoryStream.flush();
   repositoryStream.close();
   bool bOpen = repositoryStream.is_open();                                                        REQUIRE(repositoryStream.is_open() == false);

   std::cout << "Repository file: " << repositoryStream.dump() << std::endl;

   /*
   {
      gd::io::stream::repository repositoryRead(repositoryStream.get_path());
      repositoryRead.open();

      for( const auto& it : repositoryRead )
      {
         repositoryRead.read_to_file(it.get_name(), stringDataFolder + "/" + it.get_name().data());
      }
      // repositoryRead
   }
   */

   {
      gd::io::stream::repository repositoryRead(repositoryStream.get_path());
      repositoryRead.open();

      repositoryRead.remove("readme5.md" );
      repositoryRead.remove("readme6.md" );
      repositoryRead.remove("readme7.md" );
      repositoryRead.remove("readme8.md" );
      repositoryRead.remove_entry_from_file();

//      std::cout << "Repository file: " << repositoryRead.dump() << std::endl;
   }

   {
      gd::io::stream::repository repositoryRead(repositoryStream.get_path());
      repositoryRead.open();

      std::cout << "Repository file: " << repositoryRead.dump() << std::endl;
   }

   {
      gd::io::stream::repository repositoryRead(repositoryStream.get_path());
      repositoryRead.open();

      for( const auto& it : repositoryRead )
      {
         repositoryRead.read_to_file(it.get_name(), stringDataFolder + "/" + it.get_name().data());
      }
      // repositoryRead
   }
}

/// Test to create and delete documents
/*
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

   result_ = repositoryStream.add(stringFile);                                                     REQUIRE(result_.first == true);
   list_ = repositoryStream.list();
   for( auto& stringName : list_ )
   {
      std::cout << "File: " << stringName << std::endl;
   }
}
*/