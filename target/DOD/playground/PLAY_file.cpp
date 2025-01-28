#include <filesystem>

#include "gd/console/gd_console_style.h"
#include "gd/console/gd_console_print.h"
#include "gd/gd_file.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_utf8.h"

#include "Windows.h"

#undef min
#undef max

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[console] lines", "[console]" ) {
   //console console_( ::GetStdHandle( STD_OUTPUT_HANDLE ) );

   gd::console::device deviceTest( 5, 100 ); 
   deviceTest.create();
   gd::console::draw::line line_( 0, 5,  0, 95 );
   line_.print( &deviceTest, '*', gd::console::color_g("cyan1") );
   line_.move_down();
   line_.print( &deviceTest, '*', gd::console::color_g("gold1") );
   line_.move_down();
   line_.print( &deviceTest, '*', gd::console::color_g("grey35") );
   auto stringOut = deviceTest.render( gd::console::tag_format_cli{} );
   std::cout << stringOut;

   using namespace gd::file;

   path pathTest("C:\\Users\\Public\\Documents");                                                  REQUIRE(pathTest.count() == 4);
   pathTest += "my_text.txt";                                                                      REQUIRE(pathTest.count() == 5);

   auto pathTest2 = pathTest / ".." / "test2.txt";

   std::cout << pathTest2 << "\n";
   std::cout << pathTest2.filename() << "\n";
   std::cout << pathTest2.extension() << "\n";

   pathTest2.erase( 1 );
   std::cout << pathTest2 << "\n";

   std::filesystem::path pathTest3("/1/2/3");
   pathTest3 = std::filesystem::path("test") / "/gggg";
   std::cout << pathTest3 << "\n";


   /*
   gd::console::device deviceTest( 250, 100 );
   deviceTest.create();
   gd::console::draw::line lineColorTest( 0, 30 ,  0, 99 );

   for( uint8_t uColor = uint8_t(16); uColor < 255; uColor++ )
   {
      deviceTest.select( uColor, gd::console::tag_color{});
      //lineColorTest.print(&deviceTest, '+', '-', '+');
      lineColorTest.print(&deviceTest, { '+', '-', '+' }) ;
      deviceTest.print( { lineColorTest.r1(), 0 }, std::to_string(uColor), 255);
      lineColorTest.move_down();
   }

   auto stringOut = deviceTest.render( gd::console::tag_format_cli{} );
   std::cout << stringOut;

   deviceTest.create( 20, 100 );


   gd::console::draw::line line_( 0, 0,  5, 90 );
   line_.print( &deviceTest, '*' );
   line_.move_down(3);
   line_.print( &deviceTest, '+' );

   gd::console::draw::line lineCopy(line_);

   lineCopy.move_down(3);
   lineCopy.print(&deviceTest, 'p');


   stringOut = deviceTest.render( gd::console::tag_format_cli{} );
   std::cout << stringOut;
   */

}