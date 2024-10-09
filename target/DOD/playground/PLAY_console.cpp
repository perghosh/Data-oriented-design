#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"

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
   void move( uint32_t uRow, uint32_t uColumn ) { m_pairPosition = { uRow, uColumn }; update(); }

   void print( const std::string_view& stringText ) { std::puts( stringText.data() ); }

   void update() { printf("\033[%d;%dH", m_pairPosition.first, m_pairPosition.second); }

   void color() { std::puts("\033[0m" ); }
   void color( unsigned uColor ) { printf("\033[%dm", uColor ); }



// ## attributes --------------------------------------------------------------
   HANDLE m_hConsole;
   std::pair<uint32_t, uint32_t> m_pairPosition;

};


TEST_CASE( "[console] 01", "[console]" ) {
   console consoleRow( ::GetStdHandle( STD_OUTPUT_HANDLE ) );

   consoleRow.color( console::GREEN );
   for( unsigned u = 0; u < 10; u++ )
   {
      consoleRow.move( u, u );
      consoleRow.print( "XXXXXXXXXXX" );
   }
   consoleRow.color();
   consoleRow.print( "\nReady\n" );
}
