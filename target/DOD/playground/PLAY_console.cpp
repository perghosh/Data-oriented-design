#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/console/gd_console_style.h"
#include "gd/console/gd_console_print.h"

#include "explore/gd_test.h"

#include "Windows.h"

#undef min
#undef max

#include "main.h"

#include "catch2/catch_amalgamated.hpp"



/// ---------------------------------------------------------------------------            
/// \brief 
struct console
{
   enum enumConsoleColor {
      BLACK = 30,
      RED = 31,
      GREEN = 32,
      YELLOW = 33,
      BLUE = 34,
      MAGENTA = 35,
      CYAN = 36,
      WHITE = 37
   };

// ## construction ------------------------------------------------------------
   console(): m_hConsole( nullptr ) {}
   console( HANDLE hConsole ): m_hConsole( hConsole ) {}
   // copy
   console(const console& o) { common_construct(o); }
   console(console&& o) noexcept { common_construct(std::move(o)); }
   // assign
   console& operator=(const console& o) { common_construct(o); return *this; }
   console& operator=(console&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~console() {}
   // common copy
   void common_construct(const console& o) { m_hConsole = o.m_hConsole; }
   void common_construct(console&& o) noexcept {
      m_hConsole = o.m_hConsole;
      o.m_hConsole = nullptr;
   }

// ## methods -----------------------------------------------------------------
   void move( uint32_t uRow, uint32_t uColumn ) { m_pairPosition = { uRow, uColumn }; position_update(); }

   void print( const std::string_view& stringText ) { std::puts( stringText.data() ); }
   void print_at( uint32_t uRow, uint32_t uColumn, const std::string_view& stringText );

   void position_set( uint32_t uRow, uint32_t uColumn ) { printf("\033[%d;%dH", uRow, uColumn); }
   void position_update() { printf("\033[%d;%dH", m_pairPosition.first, m_pairPosition.second); }

   void color() { std::puts("\033[0m" ); }
   void color( unsigned uColor ) { printf("\033[%dm", uColor ); }



// ## attributes --------------------------------------------------------------
   HANDLE m_hConsole;
   std::pair<uint32_t, uint32_t> m_pairPosition;

};

inline void console::print_at(uint32_t uRow, uint32_t uColumn, const std::string_view& stringText) {
   position_set( uRow, uColumn );
   print( stringText );
   position_update();
}

TEST_CASE( "[console] get console information", "[console]" ) {
   gd::console::console console_;

   auto result_ = console_.initialize();                                                           REQUIRE( result_.first );
}


TEST_CASE( "[console] 01", "[console]" ) {
   console console_( ::GetStdHandle( STD_OUTPUT_HANDLE ) );

   console_.color( console::GREEN );
   for( unsigned u = 0; u < 10; u++ )
   {
      console_.move( u, u );
      console_.print( "XXXXXXXXXXX" );
   }
   console_.color();
   console_.print( "\nReady\n" );
   console_.print_at( 1, 40, "SCORE: 100" );
   console_.print( "\n//////////////////////////////////////////////////////////" );
}

TEST_CASE( "[console] lines", "[console]" ) {
   //console console_( ::GetStdHandle( STD_OUTPUT_HANDLE ) );
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


}