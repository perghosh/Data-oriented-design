#include <cstring>
#include <filesystem>
#include <fstream>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"

#include "gd/expression/gd_expression.h"
#include "gd/parse/gd_parse_window_line.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

/*
TEST_CASE("[rowcouner] count characters", "[rowcouner]") {
   std::string stringFile = FOLDER_GetRoot_g("temp__/sqlite3.c");                                  REQUIRE(std::filesystem::exists(stringFile) == true);

   std::ifstream file_(stringFile, std::ios::binary);                                              REQUIRE(file_.is_open() == true);

   gd::parse::window::line windowLine_(1024, gd::types::tag_create{});         // create line buffer

   unsigned uNewLineCount = 0;
   unsigned uReadCount = 0;
   
   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                            // get number of valid bytes read
   windowLine_.update(uReadSize);                                              // Update valid size in line buffer
   while( windowLine_.eof() == false )
   {
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});
      for(auto it = first_; it != last_; it++ ) 
      {
         if( *it == '\n' ) { uNewLineCount++; }
      }

      windowLine_.rotate();                                                    // rotate buffer

      if( uReadSize > 0 )                                                      // was it possible to read data last read, then more data is available
      {
         uReadCount++;
         auto uAvailable = windowLine_.available();                            // get available space in buffer to be filled
         file_.read((char*)windowLine_.buffer(), windowLine_.available());     // read more data into available space in buffer
         uReadSize = file_.gcount();
         windowLine_.update(uReadSize);                                        // update valid size in line buffer
      }
   }

   file_.close();                                                             // close file
   std::cout << "Read count: " << uReadCount << "and number of lines are: " << uNewLineCount << "\n";
}
*/

TEST_CASE("[rowcouner] count comments", "[rowcouner]") {
   std::string stringFile = FOLDER_GetRoot_g("temp__/sqlite3.c");                                  REQUIRE(std::filesystem::exists(stringFile) == true);

   std::ifstream file_(stringFile, std::ios::binary);                                              REQUIRE(file_.is_open() == true);

   gd::parse::window::line windowLine_(1024, gd::types::tag_create{});         // create line buffer

   bool bInside = false;
   unsigned uCommentCount = 0;
   std::string stringEndComment;

   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                            // get number of valid bytes read
   windowLine_.update(uReadSize);                                              // Update valid size in line buffer
   while( windowLine_.eof() == false )
   {
      // ## count number of comments, comments start with '//' or '/*' and end with '*/'
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});
      for(auto it = first_; it != last_; it++ ) 
      {
         if( bInside == false )
         {
            if( *it == '/' )
            {
               auto iFind = windowLine_.find({ "//", 2 }, it - first_);
               if( iFind != -1 ) { bInside = true; stringEndComment = "\n"; }
               else
               {
                  iFind = windowLine_.find({ "/*", 2 }, it - first_);
                  if( iFind != -1 ) { bInside = true; stringEndComment = "*/"; }
               }
            }
         }
         else
         {
            if( *it == stringEndComment[0] )
            {
               auto iFind = windowLine_.find({ stringEndComment.data(), stringEndComment.length() }, it - first_);
               if( iFind != -1 ) { bInside = false; uCommentCount++; }
            }
         }
      }

      windowLine_.rotate();                                                    // rotate buffer

      if( uReadSize > 0 )                                                      // was it possible to read data last read, then more data is available
      {
         auto uAvailable = windowLine_.available();                            // get available space in buffer to be filled
         file_.read((char*)windowLine_.buffer(), windowLine_.available());     // read more data into available space in buffer
         uReadSize = file_.gcount();
         windowLine_.update(uReadSize);                                        // update valid size in line buffer
      }
   }

   file_.close();                                                             // close file
   std::cout << "Comment count: " << uCommentCount << "\n";

}