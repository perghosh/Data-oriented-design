#include "conio.h"

#include <format>
#include <random>

#include "gd/gd_math.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"
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
   Clear();

   m_argumentsWorm.append( "move_row", 0 );                                    // vertical movement
   m_argumentsWorm.append( "move_column", 1 );                                 // horizontal movement
   m_argumentsWorm.append( "head", ToBodyPart_s( 5, 10 ) );                    // head at position 5,5

   m_argumentsWorm.append( "body", ToBodyPart_s( 5, 9 ) );
   m_argumentsWorm.append_many( ToBodyPart_s( 5, 8 ), ToBodyPart_s( 5, 7 ), ToBodyPart_s( 5, 6 ), ToBodyPart_s( 5, 5 ) );
   m_argumentsWorm.append( "dummy", false );

   //uint64_t uMoveSize = (m_argumentsWorm.buffer_data() + m_argumentsWorm.buffer_size()) - m_argumentsWorm.find("dummy");

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

   // ## update worm position
   int32_t iMoveRow = m_argumentsWorm["move_row"];
   int32_t iMoveColumn = m_argumentsWorm["move_column"];

   auto vectorBody = m_argumentsWorm.get_argument_section("body", gd::argument::shared::arguments::tag_view{});

   if( (m_uMoveCounter % 10) == 0 ) 
   { 
      vectorBody.push_back( ToBodyPart_s(0,0) ); 
   }

   std::rotate(vectorBody.rbegin(), vectorBody.rbegin() + 1, vectorBody.rend()); // rotate

   uint64_t uHead = m_argumentsWorm["head"];                                   // get head position
   vectorBody[0] = uHead;                                                      // move head position to first body position

#ifndef NDEBUG
   // std::string string_d = gd::debug::print( vectorBody, []( const auto& v_ ) -> std::string { return std::to_string( v_.as_uint64() >> 32 ) + "-" + std::to_string( v_.as_uint() ); });
#endif

   m_argumentsWorm.set_argument_section( "body", vectorBody );

   //auto size_ = m_argumentsWorm.size();

   // ## move head to next position and update head position
   uHead = Worm::Move_s( uHead, iMoveRow, iMoveColumn );
   m_argumentsWorm.set( "head", uHead );                                       // update head value
}

/// Check if position is on any part of worm
bool Worm::Exists(uint64_t uPosition) const
{
   // ## check if position is on worm body
   auto pposition_ = m_argumentsWorm.find( {"body", uPosition } , gd::argument::shared::tag_section{} );
   if( pposition_ != nullptr ) return true;

   return false;
}

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- Application
// ----------------------------------------------------------------------------


/** ---------------------------------------------------------------------------
 * @brief Initialize game
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> Application::Initialize()
{
   ::srand( (unsigned)::time(NULL));                                           // init random numbers

   // ## Get information about terminal size


   // ## create device
   // ### Get the terminal size and set device from that
   auto rowcolumn_ = gd::console::device::terminal_get_size_s();
   rowcolumn_.row( rowcolumn_.row() - 4 );
   if( rowcolumn_.row() > 24 ) rowcolumn_.row( 24 );
   rowcolumn_.column( rowcolumn_.column() - 2 );
   m_deviceGame = rowcolumn_;
   // ### create device
   m_deviceGame.create();

   m_devicePanel = gd::console::rowcolumn( 3, rowcolumn_.column() );           // set device used to print score and game information
   m_devicePanel.create();

   // ## create start worm that user moves in game
   m_worm.Create();
   m_argumentsGame.append("meat",uint64_t(0))
                  .append("shrink",uint64_t(0))
                  .append("score",uint64_t(0))
                  .append("hiscore",uint64_t(0));

   return application::basic::CApplication::Initialize();
}

/// ---------------------------------------------------------------------------
/// read key stroke
std::pair<bool, std::string> Application::GAME_Update( tag_key )
{
   if(_kbhit() != 0 ) 
   {
      char iKey = _getch();
      switch(iKey)
      {
      case 'w':
         m_worm.SetProperty("move_row", -1 );
         m_worm.SetProperty("move_column", 0 );
         break;
      case 'x':
         m_worm.SetProperty("move_row", 1 );
         m_worm.SetProperty("move_column", 0 );
         break;
      case 'a':
         m_worm.SetProperty("move_row", 0 );
         m_worm.SetProperty("move_column", -1 );
         break;
      case 'd':
         m_worm.SetProperty("move_row", 0 );
         m_worm.SetProperty("move_column", 1 );
         break;
      case 'q':
         m_stringState = "quit";
         break;
      case '\r':
         m_stringState = "play";
         break;
      }
   }

   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// read key stroke
std::pair<bool, std::string> Application::GAME_Update( tag_loop )
{
   if( GetState() == "play" )
   {
      // ## move worm to next position
      m_worm.Move();
   }

   return { true, "" };
}


/// ---------------------------------------------------------------------------
/// Test positions if there is changes in game state, could be that the snake has hit something
std::pair<bool, std::string> Application::GAME_Update( tag_state )
{
   if( GetState() == "wait") return {true, ""};

   // ## If snake head has moved into something then set the proper state for it
   auto pairHeadPosition = m_worm.GetHeadPosition();                               // get head position

   // ### Check if head is outside game plan
   {
      gd::console::rowcolumn RC = m_deviceGame.size();

      RC = gd::math::increase_pair( -3, std::pair<unsigned,unsigned>(RC) );                                  // decrease border size
      bool bInside = gd::math::area::is_inside_box( pairHeadPosition, {1,1}, std::pair<unsigned,unsigned>(RC) );
      if(bInside == false) 
      {  // We are outside the game area and game has ended
         SetState( "crash" ); 
         return { true, "" };
      }
   }

   // ## check so that worm head is not moving ihto its own body
   {
      uint64_t uHead = m_worm.GetProperty("head");
      if(m_worm.Exists(uHead) == true)
      {
         SetState( "crash" ); 
         return { true, "" };
      }
   }

   // ### test if snake head has moved into food
   {
      bool bNewMeat = false;
      uint64_t uMeat = m_argumentsGame["meat"];
      auto pairMeat = gd::math::algebra::split_to_pair( uMeat );
      if(pairHeadPosition == pairMeat)
      {
         uint64_t uScore = m_argumentsGame["score"];
         uScore += 10;
         m_argumentsGame.set( "score", uScore );
         bNewMeat = true;
      }
      else if(pairMeat == decltype(pairMeat){ 0ull, 0ull } )
      {
         bNewMeat = true;
      }

      if(bNewMeat == true)                                                        // create new meat?
      {
         bool bFreeSpot = false;
         auto pairSize = m_deviceGame.size();

         while( bFreeSpot == false )
         {
            uint64_t uRow = (rand() % (pairSize.first - 2)) + 1;                                   assert( uRow > 0 );
            uint64_t uColumn = (rand() % (pairSize.second - 2)) + 1;                               assert( uColumn > 0 );
            auto uNewMeat = gd::math::algebra::join_from_pair( std::pair<uint64_t,uint64_t>{ uRow, uColumn } );
            if( m_worm.Exists( uNewMeat ) == false && m_worm.IsOnHead( uNewMeat ) == false )
            {
               m_argumentsGame.set("meat", uNewMeat);
               bFreeSpot = true;
            }
         }
      }
   }


   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// prepare frame before drawing it to terminal
void Application::PrepareFrame()
{
   GAME_Update( tag_key{} );
   m_worm.Move();
}



/// ---------------------------------------------------------------------------
/// Draw application to terminal
void Application::Draw()
{
   // ## draw game frame
   //DrawGameFrame2();
   //DrawGamePanel2();

   DrawBorder(m_deviceGame);
   DrawBorder(m_devicePanel);
   DrawGamePanel2();

   if( GetState() == "play")
   {
      // ## draw the game objects
      // ### draw worm
      auto pairPosition = m_worm.GetHeadPosition();
      m_deviceGame.print( pairPosition, 'O' );
      auto vectorWorm = m_worm.ToList( "body" );
      m_deviceGame.print( vectorWorm, 'X' );

      // ### draw meat
      uint64_t uMeat = m_argumentsGame["meat"];
      auto pairMeat = gd::math::algebra::split_to_pair( uMeat );                                   assert( pairMeat.first != 0 );
      m_deviceGame.print( (unsigned)pairMeat.first, (unsigned)pairMeat.second , char(254) );// print food, ascii 254
   }
   else if(GetState() == "crash")
   {
      // ## Update hiscore if above
      uint64_t uScore = m_argumentsGame["score"];
      uint64_t uHiScore = m_argumentsGame["hiscore"];
      if(uHiScore < uScore)
      {
         uHiScore = uScore;
         m_argumentsGame.set( "hiscore", uHiScore );
      }
      m_argumentsGame.set("meat", 0ull );

      // ## Worm crashed, update and prepare for new game
      m_worm.Create();
      SetState( "wait" );

      if( m_argumentsGame["ready"].is_true() == false)
      {
         auto pairSize = m_deviceGame.size();
         uint64_t uRow = (rand() % (pairSize.first - 2)) -1;
         uint64_t uColumn = (rand() % (pairSize.second - 2)) -1;
         auto uMeat = gd::math::algebra::join_from_pair( std::pair<uint64_t,uint64_t>{ uRow, uColumn } );

         // ## test if meat spot is on empty place
         bool b_ = m_worm.Exists( uMeat );

         m_argumentsGame.set( "meat", uMeat );
         m_argumentsGame.set("ready", true);
      }
   }
   else
   {
      // ## draw the game plan
      DrawGameInformation();
   }


   std::cout << m_caretTopLeft.render( gd::console::tag_format_cli{});
   std::cout << m_deviceGame.render( gd::console::tag_format_cli{});
   std::cout << m_devicePanel.render( gd::console::tag_format_cli{});
}


/// ---------------------------------------------------------------------------
/// Draw the game plan
void Application::DrawGameFrame()
{
   const char chFrame = '#';
   auto [uRowCount, uColumnCount] = m_deviceGame.size();
   
   m_deviceGame.select( gd::console::enumColor::eColorSteelBlue3, gd::console::tag_color{});// select frame color

   for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) m_deviceGame.print( 0, uColumn, chFrame );
   for( unsigned uColumn = 0; uColumn < uColumnCount; uColumn++ ) m_deviceGame.print( uRowCount-1, uColumn, chFrame );

   for( unsigned uRow = 0; uRow < uRowCount; uRow++ ) m_deviceGame.print( uRow, 0, chFrame );
   for( unsigned uRow = 0; uRow < uRowCount; uRow++ ) m_deviceGame.print( uRow, uColumnCount - 1, chFrame ); 
   
   m_deviceGame.select( gd::console::enumColor::eColorNavajoWhite1, gd::console::tag_color{}); // select active color
   m_deviceGame.fill( 1,1, uRowCount - 2, uColumnCount - 2, ' ' );             // clear inner part
}

void Application::DrawBorder(gd::console::device& deviceBorder)
{
   const char iFrameCornerTopLeft = (char)201;
   const char iFrameCornerTopRight = (char)187;
   const char iFrameCornerDownLeft = (char)200;
   const char iFrameCornerDownRight = (char)188;
   const char iFrameSide = (char)186;
   const char iFrameRow = (char)205;

   auto [uRowCount, uColumnCount] = deviceBorder.size();

   deviceBorder.select(gd::console::enumColor::eColorSteelBlue3, gd::console::tag_color{});// select frame color


   for (unsigned uColumn = 0; uColumn < uColumnCount; uColumn++) deviceBorder.print(0, uColumn, iFrameRow);
   for (unsigned uColumn = 0; uColumn < uColumnCount; uColumn++) deviceBorder.print(uRowCount - 1, uColumn, iFrameRow);

   deviceBorder.print(0, 0, iFrameCornerTopLeft);
   deviceBorder.print(uRowCount - 1, 0, iFrameCornerDownLeft);

   deviceBorder.print(0, uColumnCount - 1, iFrameCornerTopRight);
   deviceBorder.print(uRowCount - 1, uColumnCount - 1, iFrameCornerDownRight);

   for (unsigned uRow = 1; uRow < uRowCount - 1; uRow++) deviceBorder.print(uRow, 0, iFrameSide);
   for (unsigned uRow = 1; uRow < uRowCount - 1; uRow++) deviceBorder.print(uRow, uColumnCount - 1, iFrameSide);

   deviceBorder.select(gd::console::enumColor::eColorNavajoWhite1, gd::console::tag_color{}); // select active color
   deviceBorder.fill(1, 1, uRowCount - 2, uColumnCount - 2, ' ');             // clear inner part
   
}

/// ---------------------------------------------------------------------------
/// Draw the game panel information like hiscore and score
void Application::DrawGamePanel()
{
   // ## Generate game information
   m_devicePanel.fill( ' ' );
   auto uHiScore = m_argumentsGame["hiscore"].as_uint64();
   auto uScore = m_argumentsGame["score"].as_uint64();
   std::string stringPanel = std::format( "HISCORE: {}         SCORE: {}", uHiScore, uScore );
   m_devicePanel[1][10] = stringPanel;
}

/// ---------------------------------------------------------------------------
/// Draw the game panel information like hiscore and score


void Application::DrawGamePanel2()
{
   // ## Generate game information
   //m_devicePanel.fill(' ');
   auto uHiScore = m_argumentsGame["hiscore"].as_uint64();
   auto uScore = m_argumentsGame["score"].as_uint64();
   std::string stringPanel = std::format("HISCORE: {}         SCORE: {}", uHiScore, uScore);
   m_devicePanel[1][10] = stringPanel;
}

/// ---------------------------------------------------------------------------
/// draws start frame for game
void Application::DrawGameInformation()
{
   m_deviceGame.select( gd::console::enumColor::eColorSteelBlue3, gd::console::tag_color{});

   m_deviceGame.print(5, 20, "W = Up" );
   m_deviceGame.print(7, 20, "A = Left" );
   m_deviceGame.print(9, 20, "D = Right" );
   m_deviceGame.print(11, 20, "X = Down" );
   m_deviceGame.print(13, 70, "Press enter to start game" );
   m_deviceGame.print(14, 70, "Q = Quit" );

   auto _size_ = m_deviceGame.size();
}

