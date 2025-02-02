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

using namespace gd::file;
TEST_CASE("Path Constructors and Assignment", "[path]") {
   { path p; REQUIRE(p.empty()); }
   { path p("test/path"); REQUIRE(p == "test/path"); }
   { std::string_view sv = "test/path"; path p(sv); REQUIRE(p == "test/path"); }
   { std::string s = "test/path"; path p(s); REQUIRE(p == "test/path"); }
   { std::string s = "test/path"; path p(std::move(s)); REQUIRE(p == "test/path"); }
   { path p1("test/path"); path p2(p1); REQUIRE(p2 == "test/path"); }
   { path p1("test/path"); path p2(std::move(p1)); REQUIRE(p2 == "test/path"); }
   { path p1("test/path"); path p2; p2 = p1; REQUIRE(p2 == "test/path"); }
   { path p1("test/path"); path p2; p2 = std::move(p1); REQUIRE(p2 == "test/path"); }
}

TEST_CASE("Path Methods", "[path]") {
   { path p("test/path/file.txt"); REQUIRE(p.has_filename()); }
   { path p("test/path/"); REQUIRE(p.has_separator()); }
   { path p("/test/path"); REQUIRE(p.has_begin_separator()); }
   { path p("test/path/file.txt"); REQUIRE(p.filename().string() == "file.txt"); }
   { path p("test/path/file.txt"); REQUIRE(p.extension().string() == ".txt"); }
   { path p("test"); p.add("path"); REQUIRE(p == "test/path"); }
   { path p("test"); p.add({"path", "to", "file"}); REQUIRE(p == "test/path/to/file"); }
   { path p("test"); std::vector<std::string_view> vec = {"path", "to", "file"}; p.add(vec); REQUIRE(p == "test/path/to/file"); }
   { path p1("test"); path p2("path"); path p3 = p1 / p2; REQUIRE(p3 == "test/path"); }
   { path p1("test"); path p2 = p1 / "path"; REQUIRE(p2 == "test/path"); }
   { path p("test/path"); p.erase_end(); REQUIRE(p == "test"); }
   { path p("test/path/file.txt"); p.remove_filename(); REQUIRE(p == "test/path/"); }
   { path p("test/path/file.txt"); p.replace_filename("newfile.txt"); REQUIRE(p == "test/path/newfile.txt"); }
   { path p("test/path/file.txt"); p.replace_extension(".md"); REQUIRE(p == "test/path/file.md"); }
   { path p("test/path"); p.clear(); REQUIRE(p.empty()); }
   { path p("test/path"); std::string result; for (auto it = p.begin(); it != p.end(); ++it) { result += *it; } REQUIRE(path(result) == "test/path"); }
}
