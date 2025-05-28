#include <format>
#include <thread>

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/console/gd_console_style.h"
#include "gd/console/gd_console_print.h"

#include "gd/console/gd_console_console.h"

#ifdef _WIN32
#include "windows.h"
#endif

#undef min
#undef max

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

void PrintProgressbar( unsigned uPercent, gd::console::progress& progress_, gd::console::console& console_ ) 
{
   //console_.move_to( progress_.row(), progress_.column(), gd::types::tag_row_column{});


   std::string stringProgress = std::format("[{:3d}%] ", uPercent);

   progress_.update(uPercent, gd::types::tag_percent{});

   progress_.print_to( "[ ", "=", ">", " ]", stringProgress );
   console_.print( stringProgress);

   std::cout << std::endl;

/*
   int barWidth = 70;
   float progress = (float)i / total;
   std::cout << "[ ";
   int pos = barWidth * progress;
   for (int j = 0; j < barWidth; ++j) {
      if (j < pos) std::cout << "=";
      else if (j == pos) std::cout << ">";
      else std::cout << " ";
   }
   std::cout << " ] " << int(progress * 100.0) << " %\r";
   std::cout.flush();
   */
}


TEST_CASE( "[console] get console information", "[console]" ) {
   using namespace gd::console;
   console console_;

   auto result_ = console_.initialize();                                                           REQUIRE( result_.first );
   progress progressBar;
   progressBar.set_position( console_.yx( gd::types::tag_type_unsigned{}));
   progressBar.set_width(50);

   PrintProgressbar(50, progressBar, console_);


   /*
   auto color_ = console_.query_foreground_color();

   console_.move_to( 10, 5 );
   console_.set_foreground_color( 0, 150, 0 );
   //console_.set_background_color( 25, 0, 0 );
   std::cout << "Console position: " << console_.xy().first << ", " << console_.xy().second << std::endl;
   std::cout << "XXXXXXXXX\n";
   console_.set_foreground_color( color_.second );
   std::cout << "XXXXXXXXX\n";
   */
}

TEST_CASE( "[console] progressbar", "[console]" ) {
   /*
   using namespace gd::console;
   console console_;
   auto result_ = console_.initialize();                                                           REQUIRE( result_.first );
   auto color_ = console_.query_foreground_color();

   {
      progress progressBar(5, 100, 100);
      progressBar.update( 20 );

      gd::math::algebra::point<unsigned> positionOffsetLeft(3u, 0u);

      // simulate incrementing the progress bar
      for (int i = 0; i <= 100; i += 1) {
         progressBar.update(i);
         console_.print( progressBar.position(), "X" );

         if( i > 3 ) { console_.print( progressBar.position() - positionOffsetLeft, "O"); }
      
         std::this_thread::sleep_for(std::chrono::milliseconds(100)); // simulate work
      }
   }

   */

}



#ifdef _WIN32x

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
#endif