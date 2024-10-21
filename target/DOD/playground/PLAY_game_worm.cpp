#include <random>
#include <thread>
#include <chrono>

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"



#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[game_worm] 01", "[game_worm]" ) {
   const unsigned uRowCount = 15;
   const unsigned uColumnCount = 80;
   gd::console::caret caretLeftTop;
   gd::console::device deviceWorm( uRowCount, uColumnCount );
   deviceWorm.create();

   std::random_device randomdevice;
   std::mt19937 mt19937RandomNumber(randomdevice()); 
   std::uniform_int_distribution<> UIDRow(0, uRowCount - 1);
   std::uniform_int_distribution<> UIDColumn(0, uColumnCount - 1);
   std::uniform_int_distribution<> UIDColor(16, 255);

   deviceWorm[0][0] = "** Code sample showing how to draw on device **";

   unsigned uCount = 100;

   while( uCount > 0 )
   {
      unsigned uRow = UIDRow( mt19937RandomNumber );                                               assert( uRow < uRowCount );
      unsigned uColumn = UIDColumn( mt19937RandomNumber );                                         assert( uColumn < uColumnCount );
      unsigned uColor = UIDColor( mt19937RandomNumber );

      deviceWorm[uRow][uColumn] = 'X';
      deviceWorm.set_color( uRow, uColumn, uColor );

      std::string stringPrint;

      caretLeftTop.render( stringPrint );
      std::cout << stringPrint;

      stringPrint = "";

      deviceWorm.render( stringPrint );
      std::cout << stringPrint;

      uCount--;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
   }

   gd::console::device deviceWorm2( deviceWorm );

   uCount = 15;
   while( uCount > 0 )
   {
      deviceWorm.scroll_y( -1 );
      std::string stringPrint;

      caretLeftTop.render( stringPrint );
      std::cout << stringPrint;
      stringPrint = "";
      deviceWorm.render( stringPrint );
      std::cout << stringPrint;

      uCount--;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   std::cout << caretLeftTop.render( gd::console::tag_format_cli{});
   std::cout << deviceWorm2.render( gd::console::tag_format_cli{});
   deviceWorm = deviceWorm2;

   uCount = 15;
   while( uCount > 0 )
   {
      deviceWorm.scroll_y( 1 );
      std::cout << caretLeftTop.render( gd::console::tag_format_cli{} )
                << deviceWorm.render( gd::console::tag_format_cli{});

      uCount--;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }


   std::cout << caretLeftTop.render( gd::console::tag_format_cli{});
   std::cout << deviceWorm2.render( gd::console::tag_format_cli{});
}