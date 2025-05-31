#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "gd/parse/gd_parse_window_line.h"



#include "pugixml/pugixml.hpp"

#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[file] read .gitignore", "[file]")
{
   std::string stringFileGitignore = FOLDER_GetRoot_g(".gitignore");                               REQUIRE(std::filesystem::exists(stringFileGitignore) == true);
   std::ifstream file_(stringFileGitignore, std::ios::binary);                                     REQUIRE( file_.is_open() );

   gd::expression::parse::state state_;
   state_.add(std::string_view("LINECOMMENT"), "//", "\n");
   gd::parse::window::line lineBuffer(1024, gd::types::tag_create{});         // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   file_.read((char*)lineBuffer.buffer(), lineBuffer.available());
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   // ## Process the file
   while(lineBuffer.eof() == false)
   {
      std::string_view stringLine;
      while( lineBuffer.getline( stringLine ) == true )
      {
         // Process the line here, e.g., print it or store it
         std::cout << stringLine << std::endl;
      }
      while
      auto [first_, last_] = lineBuffer.range(gd::types::tag_pair{});
      for( const auto* it = first_; it != last_; ++it )                       // For each character in the buffer
      {
         std::string_view stringLine;
         it = lineBuffer.getline( it, stringLine );
      }
   }



}