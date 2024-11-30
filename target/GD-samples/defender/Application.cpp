#include "conio.h"


#include <format>
#include <random>

#ifndef NDEBUG
   #include "gd/gd_debug.h"
#endif

#include "Application.h"


std::pair<bool, std::string> CApplication::Initialize()
{
   ::srand( (unsigned)::time(NULL));                                           // init random numbers

   m_deviceGame.create(20, 80);

   SHIP_Reset();

   return application::basic::CApplication::Initialize();
}

void CApplication::Move()
{
   auto [uDeviceHeight, uDeviceWidth] = m_deviceGame.size();

   uint32_t uShipRow = m_argumentsShip("row");
   uint32_t uShipColumn = m_argumentsShip("column");

   int32_t iShipMoveY = m_argumentsShip("move_row");
   int32_t iShipMoveX = m_argumentsShip("move_column");

   m_argumentsShip("row") = uShipRow += iShipMoveY;
   m_argumentsShip("column") = uShipColumn += iShipMoveX;
   m_argumentsShip.set("move_row", int32_t(0));
   m_argumentsShip.set("move_column", int32_t(0));

   for(auto& itBomb : m_vectorBomb)
   {
      if( itBomb["show"] == true )
      {
         uint32_t uColumn = itBomb["column"];
         int32_t iMoveX = itBomb["move-x"];

         uColumn = uColumn + iMoveX;

         if(uColumn == 0 || uColumn == uDeviceWidth)
         {
            itBomb.set( "show", false );
         }
         else
         {
            itBomb("column") = uColumn;
            //itBomb.set( "column", uColumn );
         }
      }
   }
}

std::pair<bool, std::string> CApplication::Input_Update()
{
   if( _kbhit() != 0 )
   {
      char iKey = _getch();
      switch( iKey )
      {
      case 'q':
         m_stringState = "quit";
         break;
      case 'w':
         m_stringState = "up";
         m_argumentsShip.set("move_row", int32_t(-1));
         break;
      case 's':
         m_stringState = "down";
         m_argumentsShip.set("move_row", int32_t(1));
         break;
      case 'a':
         m_stringState = "left";
         m_argumentsShip.set("move_column", int32_t(-1));
         break;
      case 'd':
         m_stringState = "right";
         m_argumentsShip.set("move_column", int32_t(1));
         break;
      default:
         break;
      }
   }
   return { true, "" };
}

void CApplication::Draw()
{
   m_deviceGame.fill(' ');
   
   auto pairSize = m_deviceGame.size();

   for( const auto& itBomb : m_vectorBomb)
   {
      if( itBomb["show"] == true )
      {
         uint32_t uRow = itBomb["row"];
         uint32_t uColumn = itBomb["column"];
         uint8_t uColor = itBomb["color"];
         m_deviceGame.print( uRow, uColumn, "#", uColor);
      }
   }

   uint32_t uShipRow = m_argumentsShip("row");
   uint32_t uShipColumn = m_argumentsShip("column");

   m_deviceGame.print(uShipRow, uShipColumn, "P", 44);
   
   std::cout << m_caretTopLeft.render( gd::console::tag_format_cli{});  
   std::cout << m_deviceGame.render(gd::console::tag_format_cli{});
}

void CApplication::BOMB_Add()
{
   gd::argument::arguments argumentsBomb;

   auto uHeight = m_deviceGame.height();

   uint32_t uRow = rand() % uHeight;

   uint8_t uColor = rand() % ((255 - 16) + 1 ) + 16;
   
   bool bBombReuse = false;

   for(auto& itBomb : m_vectorBomb)
   {
      if(itBomb["show"] == false)
      {
         itBomb.set("show", true);
         itBomb.set("row", uRow);
         itBomb.set("column", uint32_t(0));
         itBomb.set("color", gd::console::enumColor(uColor));
         bBombReuse = true;
         break;
      }
   }

   if(bBombReuse == false)
   {
      argumentsBomb.append("row", uRow);
      argumentsBomb.append("column", uint32_t(0));
      argumentsBomb.append("move-x", int32_t(1));
      //argumentsBomb.append("color", gd::console::color_g("blue3"));
      argumentsBomb.append("color", gd::console::enumColor(uColor));
      argumentsBomb.append("show", true);
      m_vectorBomb.push_back(std::move(argumentsBomb));
   }

}

void CApplication::SHIP_Reset()
{
   m_argumentsShip.clear();
   m_argumentsShip.append("row", uint32_t(5));
   m_argumentsShip.append("column", uint32_t(5));

   m_argumentsShip.append("move_row", int32_t(0));
   m_argumentsShip.append("move_column", int32_t(0));
   
}

void CApplication::GAME_Start()
{

}

void CApplication::GAME_End()
{

}

void CApplication::Update()
{
   if (m_iCount % 5 == 0)
   {
      BOMB_Add();
   }

   for( auto& itBomb : m_vectorBomb )
   {
      if( itBomb["show"] == true )
      {
         uint32_t uColumn = itBomb["column"];
         uint32_t uRow = itBomb["row"];

         uint32_t uShipColumn = m_argumentsShip("column");
         uint32_t uShipRow = m_argumentsShip("row");

         if( (uColumn + 1) == uShipColumn && uRow == uShipRow )
         {
            m_stringState = "quit";
         }
      }
   }

}