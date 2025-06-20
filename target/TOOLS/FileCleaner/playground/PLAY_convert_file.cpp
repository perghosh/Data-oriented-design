#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/parse/gd_parse_window_line.h"
#include "gd/parse/gd_parse_match_pattern.h"
#include "gd/expression/gd_expression_parse_state.h"

#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"


std::vector<uint8_t> strip( std::string stringFileName )
{
   std::vector<uint8_t> vectorBuffer;

   gd::expression::parse::state state_;
   state_.add(std::string_view("LINECOMMENT"), "//", "\n");
   state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
   state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   state_.add(std::string_view("RAWSTRING"), "R\"(", ")\"");


   std::ifstream file_(stringFileName, std::ios::binary);                      REQUIRE(file_.is_open() == true);
   gd::parse::window::line windowLine_( 1024 );
   windowLine_.create();

   auto uAvailable = windowLine_.available();
   file_.read((char*)windowLine_.buffer(), uAvailable);
   auto uSize = file_.gcount();                                                                     REQUIRE(uSize > 0);
   windowLine_.update(uSize);                                                                    
   // ## Scan file and read lines found in table
   while( windowLine_.eof() == false )
   {
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});         // get range of valid data in buffer
      for( const auto* it = first_; it < last_; it++ ) 
      {
         if( state_.in_state() == false )                                     // not in a state? that means we are reading source code
         {
            // ## check if we have found state
            if( state_[*it] != 0 && state_.exists( it ) == true )
            {
               state_.activate(it);                                           // activate state
            }
            else
            {
               vectorBuffer.push_back(*it);                                   // add character to buffer
            }
         }
         else
         {
            // ## check if we have found end of state
            unsigned uLength;
            if( state_.deactivate( it, &uLength ) == true ) 
            {
               if( uLength > 1 ) it += (uLength - 1);                         // skip to end of state marker and if it is more than 1 character, skip to end of state
               // check for ending linebreak 
               if( *it == '\n' ) vectorBuffer.push_back(*it);                 //
               continue;
            }

            if( *it == '\n' ) vectorBuffer.push_back(*it);
         }
      }

      windowLine_.rotate();                                                   // rotate buffer

      auto uAvailable = windowLine_.available();                              // get available space in buffer to be filled
      file_.read((char*)windowLine_.buffer(), windowLine_.available());       // read more data into available space in buffer
      uSize = file_.gcount();
      windowLine_.update(uSize);                                              // update valid size in line buffer
   }

   file_.close();

   return vectorBuffer;
}

TEST_CASE("[convert-file] remove comments", "[convert-file]")
{
   auto vectorBuffer = strip("D:\\dev\\main.c");

   std::cout << "Buffer size: " << vectorBuffer.size() << " bytes\n";
   std::cout << "Buffer content: \n";
   for( auto it : vectorBuffer )
   {
       std::cout << (char)it;
   }

   // Save the stripped content to temp.c file
   std::ofstream fileWrite_("D:\\dev\\temp.c", std::ios::binary);            REQUIRE(fileWrite_.is_open() == true);
   fileWrite_.write((char*)vectorBuffer.data(), vectorBuffer.size());
   fileWrite_.close();
   std::cout << "\nStripped content saved to D:\\dev\\temp.c\n";

}