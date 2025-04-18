#include <iostream>
#include <filesystem>
#include <fstream>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "gd/parse/gd_parse_window_line.h"



#include "main.h"

#include "catch2/catch_amalgamated.hpp"

/*

TEST_CASE("[read-file] test", "[read-file]")
{
   unsigned uNewLineCount = 0;
   std::string stringFile = R"(D:\temp\Metadata_Statements.cpp)";
   std::ifstream file_(stringFile, std::ios::binary);                                               REQUIRE(file_.is_open() == true);

   gd::parse::window::line windowLine_( 1024 );
   windowLine_.create();

   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);
   auto uSize = file_.gcount();                                                                     REQUIRE(uSize > 0);
   windowLine_.update(uSize);                                                                    
   while( uSize > 0 )
   {
      //std::string_view stringLine = windowLine_;
      //std::cout << stringLine;

      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});
      for(auto it = first_; it != last_; it++ ) 
      {
         if( *it == '\n' ) { uNewLineCount++; }
      }

      auto uAvailable = windowLine_.available();
      windowLine_.rotate();
      uAvailable = windowLine_.available();
      file_.read((char*)windowLine_.buffer(), windowLine_.available());
      uSize = file_.gcount();
      windowLine_.update(uSize);
      auto uSizeSummary = windowLine_.size_summary();
      std::cout << "size: " << uSize << "  " << uSizeSummary << " bytes\n";
   }

   std::cout << "line count: " << uNewLineCount << "\n";

   file_.close();

}
*/

TEST_CASE("[read-file] find name", "[read-file]")
{
   //unsigned uHitCount = 0;
   std::string stringFile = R"(D:\temp\sqlite3.c)";
   std::string stringToFile = R"(D:\temp\sqlite3_to.c)";
   std::string_view stringCompare = "static";
   std::ifstream file_(stringFile, std::ios::binary);                                               REQUIRE(file_.is_open() == true);

   if( std::filesystem::exists(stringToFile) == true ) { std::filesystem::remove(stringToFile); }

   std::ofstream fileWrite_(stringToFile, std::ios::binary);                                          REQUIRE(fileWrite_.is_open() == true);

   gd::parse::window::line windowLine_( 1024 );
   windowLine_.create();

   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);
   auto uSize = file_.gcount();                                                                     REQUIRE(uSize > 0);
   windowLine_.update(uSize);                                                                    
   while( windowLine_.eof() == false )
   {
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});
      fileWrite_.write((char*)first_, last_ - first_);

      auto uAvailable = windowLine_.available();
      windowLine_.rotate();
      uAvailable = windowLine_.available();
      file_.read((char*)windowLine_.buffer(), windowLine_.available());
      uSize = file_.gcount();
      windowLine_.update(uSize);
      if( uSize != 1024 )
      {
         std::cout << "size: " << uSize << "  " << windowLine_.size_summary() << " bytes\n";
      }
   }

   //std::cout << "line count: " << uHitCount << "\n";

   file_.close();
   fileWrite_.close();

}

/*
TEST_CASE("[read-file] find name", "[read-file]")
{
   unsigned uHitCount = 0;
   std::string stringFile = R"(D:\temp\sqlite3.c)";
   std::string_view stringCompare = "static";
   std::ifstream file_(stringFile, std::ios::binary);                                               REQUIRE(file_.is_open() == true);

   gd::parse::window::line windowLine_( 1024 );
   windowLine_.create();

   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);
   auto uSize = file_.gcount();                                                                     REQUIRE(uSize > 0);
   windowLine_.update(uSize);                                                                    
   while( uSize > 0 )
   {
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});
      for(auto it = first_; it != last_; it++ ) 
      {
         if( *it == stringCompare[0] && std::memcmp(it, stringCompare.data(), stringCompare.length()) == 0 )
         {
            uHitCount++;
         }
      }

      auto uAvailable = windowLine_.available();
      windowLine_.rotate();
      uAvailable = windowLine_.available();
      file_.read((char*)windowLine_.buffer(), windowLine_.available());
      uSize = file_.gcount();
      windowLine_.update(uSize);
   }

   std::cout << "line count: " << uHitCount << "\n";

   file_.close();

}
*/