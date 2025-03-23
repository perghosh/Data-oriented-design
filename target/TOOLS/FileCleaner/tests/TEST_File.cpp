#include <fstream>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"

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
   gd::cli::options optionsApplication;
   optionsApplication.set_first(0);                                         // set first argument as command, because we are not using executable then first argument is command
   CApplication::Prepare_s(optionsApplication);

   std::string stringDataFolder = GetDataFolder();

   auto result_ = optionsApplication.parse( { "copy", "-s", stringDataFolder + "/python.txt", "-d", stringDataFolder + "/python_copy.txt" } );
                                                                                                   REQUIRE(result_.first == true);

   auto poptions = optionsApplication.find_active();
   auto stringSource = ( *poptions )["source"].as_string();                                        REQUIRE(stringSource.find("python.txt") != -1);
   auto stringDestination = ( *poptions )["destination"].as_string();                              REQUIRE(stringDestination.find("python_copy.txt") != -1);
}

TEST_CASE( "[file] serialize", "[file]" ) {
   using namespace gd::io::stream;

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   int iValue = 10, iValue2 = 20, iValue3 = 30;
   archiveStream.write(iValue).write(iValue2).write(iValue3);
   archiveStream.write_all(iValue, iValue2, iValue3);
   archiveStream << iValue << iValue2 << iValue3;
   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});
   int iValueRead = 0, iValue2Read = 0, iValue3Read = 0;
   archiveStream.read(iValueRead).read(iValue2Read).read(iValue3Read);
   int iValueReadAll[3] = { 0, 0, 0 };
   archiveStream.read_all(iValueReadAll[0], iValueReadAll[1], iValueReadAll[2]);

   std::cout << "Read values: " << iValueRead << ", " << iValue2Read << ", " << iValue3Read << std::endl;
   std::cout << "Read all values: " << iValueReadAll[0] << ", " << iValueReadAll[1] << ", " << iValueReadAll[2] << std::endl;

   archiveStream.close();
   std::filesystem::remove(pathFile);

   {
      gd::file::path pathFile(stringDataFolder + "/archive1.bin");
      if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);
      archive archiveStream(pathFile, gd::io::tag_io_write{});

      std::string s1 = "1111111", s2 = "2222222", s3 = "3333333";
      archiveStream.write_all(s1, s2, s3);
      archiveStream.write_all(s1, s2, s3);
      archiveStream.close();

      archiveStream.open(pathFile, gd::io::tag_io_read{});
      std::string s1Read, s2Read, s3Read;
      archiveStream.read_all(s1Read, s2Read, s3Read);
      std::string s1ReadAll, s2ReadAll, s3ReadAll;
      archiveStream >> s1ReadAll >> s2ReadAll >> s3ReadAll;

      std::cout << "Read strings: " << s1Read << ", " << s2Read << ", " << s3Read << std::endl;
      std::cout << "Read all strings: " << s1ReadAll << ", " << s2ReadAll << ", " << s3ReadAll << std::endl;

      archiveStream.close();
      std::filesystem::remove(pathFile);
   }

   {
      gd::file::path pathFile(stringDataFolder + "/archive.bin");
      if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);
      archive archiveStream(pathFile, gd::io::tag_io_write{});

      gd::argument::arguments arguments( {{"one", "one"}, {"one", "one"}, {"one", "one"}, {"one", "one"}, {"one", "one"}});
      archiveStream.write_block( (uint64_t)arguments.buffer_size(), arguments.data() );
      archiveStream.close();

      archiveStream.open(pathFile, gd::io::tag_io_read{});
      gd::argument::arguments argumentsRead;
      uint64_t uSize = 0;
      archiveStream.read_size( uSize );
      argumentsRead.reserve(uSize);
      argumentsRead.buffer_set_size( uSize );
      archiveStream.read(argumentsRead.data(), uSize );

      std::cout << "Read arguments: " << argumentsRead.print() << std::endl;


      archiveStream.close();
      std::filesystem::remove(pathFile);
   }

   {
      gd::file::path pathFile(stringDataFolder + "/archive.bin");
      if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);
      archive archiveStream(pathFile, gd::io::tag_io_write{});

      gd::argument::shared::arguments arguments( {{"one", "one"}, {"one", "one"}, {"one", "one"}, {"one", "one"}, {"one", "one"}});
      archiveStream.write_block( (uint64_t)arguments.buffer_size(), arguments.data() );
      archiveStream.close();

      archiveStream.open(pathFile, gd::io::tag_io_read{});
      gd::argument::shared::arguments argumentsRead;
      uint64_t uSize = 0;
      archiveStream.read_size( uSize );
      argumentsRead.reserve(uSize);
      argumentsRead.buffer_set_size( uSize );
      archiveStream.read(argumentsRead.data(), uSize );

      std::cout << "Read arguments: " << argumentsRead.print() << std::endl;


      archiveStream.close();
      std::filesystem::remove(pathFile);
   }

   {
      gd::file::path pathFile(stringDataFolder + "/archive.bin");
      if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);
      archive archiveStream(pathFile, gd::io::tag_io_write{});

      gd::argument::shared::arguments arguments( {{"two", "two"}, {"two", "two"}, {"two", "two"}, {"two", "two"}, {"two", "two"}});
      archiveStream.write_block( (uint64_t)arguments.buffer_size(), arguments.data() );
      archiveStream.close();

      archiveStream.open(pathFile, gd::io::tag_io_read{});
      gd::argument::shared::arguments argumentsRead;
      archiveStream.read_block64( [&argumentsRead](uint64_t uSize) -> void* { argumentsRead.reserve( uSize ); argumentsRead.buffer_set_size( uSize ); return (void*)argumentsRead.data(); });

      std::cout << "Read arguments: " << argumentsRead.print() << std::endl;


      archiveStream.close();
      std::filesystem::remove(pathFile);
   }



}