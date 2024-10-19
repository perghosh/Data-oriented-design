#include <random>
#include <thread>
#include <chrono>

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_console_print.h"


#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[game_worm] 01", "[game_worm]" ) {
   gd::console::caret caretLeftTop;
   gd::console::device deviceWorm( 10, 80 );
   deviceWorm.create();

   std::random_device randomdevice;
   std::mt19937 mt19937RandomNumber(randomdevice()); 
   std::uniform_int_distribution<> UIDRow(0, 9);
   std::uniform_int_distribution<> UIDColumn(0, 79);

   unsigned uCount = 100;

   while( uCount > 0 )
   {
      unsigned uRow = UIDRow( mt19937RandomNumber );
      unsigned uColumn = UIDColumn( mt19937RandomNumber );

      deviceWorm[uRow][uColumn] = 'X';

      std::string stringPrint;

      caretLeftTop.render( stringPrint );
      std::cout << stringPrint;

      stringPrint.clear();

      deviceWorm.render( stringPrint );
      std::cout << stringPrint;

      uCount--;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }





}