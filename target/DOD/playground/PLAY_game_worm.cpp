#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_console_print.h"


#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[game_worm] 01", "[game_worm]" ) {
   gd::console::device deviceWorm( 30, 80 );
   deviceWorm.create();

   deviceWorm[10][20] = 'X';
   deviceWorm[20][79] = 'X';

   std::string stringPrint;
   deviceWorm.render( stringPrint );

   std::cout << stringPrint;


}