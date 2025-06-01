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
   gd::expression::parse::state state_;
   state_.add(std::string_view("LINECOMMENT"), "#", "\n");
   gd::parse::window::line lineBuffer(256, gd::types::tag_create{});          // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   {
      std::string stringTest = "  1 2 3 4 5 6 7  # Test string";
      auto [ iRule, piPosition ] = state_.find_first(stringTest); // Find first comment in line
      assert( iRule == -1 );
      assert( *piPosition == '1');

      auto [ iRule2, stringValue ] = state_.read_first( stringTest );
      assert(stringValue == "1 2 3 4 5 6 7  ");

      std::string stringTest1 = "# Test string";
      auto [ iRule3, stringValue3 ] = state_.read_first( stringTest1 );
      assert(stringValue3 == " Test string");
   }

   std::string stringFileGitignore = FOLDER_GetRoot_g(".gitignore");                               REQUIRE(std::filesystem::exists(stringFileGitignore) == true);
   std::ifstream file_(stringFileGitignore, std::ios::binary);                                     REQUIRE( file_.is_open() );


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
         
         auto [ iRule, piPosition ] = state_.find_first(stringLine); // Find first comment in line

         if( iRule == -1 && piPosition != nullptr )
         {
            auto [ iRule, stringValue ] = state_.read_first( stringLine );
            std::cout << stringValue << std::endl;
         }
      }

      lineBuffer.rotate();
      file_.read((char*)lineBuffer.buffer(), lineBuffer.available());
      uReadSize = file_.gcount();                                             // get number of valid bytes read
      lineBuffer.update(uReadSize);
   }
}