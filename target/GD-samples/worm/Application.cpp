#include "conio.h"

#include "gd/gd_console_print.h"
#include "gd/gd_arguments_shared.h"

#ifndef NDEBUG
#include "gd/gd_debug.h"
#endif

#include "Application.h"

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------- Worm
// ----------------------------------------------------------------------------



std::pair< bool, std::string > Worm::Create()
{

   m_argumentsWorm.clear();

   m_argumentsWorm.append( "move_row", 0 );                                    // vertical movement
   m_argumentsWorm.append( "move_column", 1 );                                 // horizontal movement
   m_argumentsWorm.append( "head", ToBodyPart_s( 5, 10 ) );                    // head at position 5,5

   m_argumentsWorm.append( "body", ToBodyPart_s( 5, 9 ) );
   m_argumentsWorm.append_many( ToBodyPart_s( 5, 8 ), ToBodyPart_s( 5, 7 ), ToBodyPart_s( 5, 6 ), ToBodyPart_s( 5, 5 ) );
   m_argumentsWorm.append( "dummy", false );

   //uint64_t uMoveSize = (m_argumentsWorm.buffer_data() + m_argumentsWorm.buffer_size()) - m_argumentsWorm.find("dummy");

   return { true, "" };
}

std::pair<bool, std::string> Worm::ReadInput()
{
   if(_kbhit() != 0 ) 
   {
      char iKey = _getch();
      switch(iKey)
      {
      case 'w':
         m_argumentsWorm.set("move_row", -1 );
         m_argumentsWorm.set("move_column", 0 );
         break;
      case 'x':
         m_argumentsWorm.set("move_row", 1 );
         m_argumentsWorm.set("move_column", 0 );
         break;
      case 'a':
         m_argumentsWorm.set("move_row", 0 );
         m_argumentsWorm.set("move_column", -1 );
         break;
      case 'd':
         m_argumentsWorm.set("move_row", 0 );
         m_argumentsWorm.set("move_column", 1 );
         break;
      }
   }

   return { true, "" };
}

/// Generate body parts to write in terminal
std::vector<gd::console::rowcolumn> Worm::ToList( const std::string_view& stringType ) const
{
   std::vector<gd::console::rowcolumn> vectorList;

   if( stringType == "body" )
   {
      auto vector_ = m_argumentsWorm.get_argument_section("body", gd::argument::shared::arguments::tag_view{});
      for( auto it : vector_ )
      {
         vectorList.push_back( it.as_uint64() );
      }
   }

   return vectorList;
}

/** ---------------------------------------------------------------------------
 * @brief move worm to next position based on what direction that is active
 */
void Worm::Move()
{
   m_uMoveCounter++;

   int32_t iMoveRow = m_argumentsWorm["move_row"];
   int32_t iMoveColumn = m_argumentsWorm["move_column"];

   auto vectorBody = m_argumentsWorm.get_argument_section("body", gd::argument::shared::arguments::tag_view{});

   if( (m_uMoveCounter % 10) == 0 ) 
   { 
      vectorBody.push_back( ToBodyPart_s(0,0) ); 
   }

   std::rotate(vectorBody.rbegin(), vectorBody.rbegin() + 1, vectorBody.rend());

   uint64_t uHead = m_argumentsWorm["head"];                                   // get head position
   vectorBody[0] = uHead;                                                      // move head position to first body position

#ifndef NDEBUG
   std::string string_d = gd::debug::print( vectorBody, []( const auto& v_ ) -> std::string { return std::to_string( v_.as_uint64() >> 32 ) + "-" + std::to_string( v_.as_uint() ); });
#endif


   m_argumentsWorm.set_argument_section( "body", vectorBody );

   //auto size_ = m_argumentsWorm.size();

   // ## move head to next position and update head position
   uHead = Worm::Move_s( uHead, iMoveRow, iMoveColumn );
   m_argumentsWorm.set( "head", uHead );
}

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- Application
// ----------------------------------------------------------------------------


/** ---------------------------------------------------------------------------
 * @brief 
 * @return 
 */
std::pair<bool, std::string> Application::Initialize()
{
   // ## Get information about terminal size


   // ## create device
   // ### Get the terminal size and set device from that
   auto rowcolumn_ = gd::console::device::terminal_get_size_s();
   if( rowcolumn_.row() > 25 ) rowcolumn_.row( 25 );
   m_deviceGame = rowcolumn_;
   // ### create device
   m_deviceGame.create();

   // ## create start worm that user moves in game
   m_worm.Create();

   return application::basic::CApplication::Initialize();
}

/// Draw application to terminal
void Application::Draw()
{
   // ## draw the game plan
   DrawFrame();

   // ## draw the game objects
   auto vectorWorm = m_worm.ToList( "body" );
   m_deviceGame.print( vectorWorm, '*' );


   std::cout << m_caretTopLeft.render( gd::console::tag_format_cli{});
   std::cout << m_deviceGame.render( gd::console::tag_format_cli{});
}

/// Draw the game plan
void Application::DrawFrame()
{
   const char chFrame = '/';
   auto [uRowCount, uColumnCount] = m_deviceGame.size();
   for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) m_deviceGame[0][uColumn] = chFrame;
   for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) m_deviceGame[uRowCount-1][uColumn] = chFrame;

   for( unsigned uRow = 0; uRow < uRowCount; uRow++ ) m_deviceGame[uRow][0] = chFrame;
   for( unsigned uRow = 0; uRow < uRowCount; uRow++ ) m_deviceGame[uRow][uColumnCount - 1] = chFrame;

   m_deviceGame.fill( 1,1, uRowCount - 2, uColumnCount - 2, ' ' );
}

void Application::PrepareFrame()
{
   m_worm.ReadInput();
   m_worm.Move();
}
