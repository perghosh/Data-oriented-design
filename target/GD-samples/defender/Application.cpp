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
   return application::basic::CApplication::Initialize();
}

void CApplication::Move()
{
   auto [uDeviceHeight, uDeviceWidth] = m_deviceGame.size();

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
   
   std::cout << m_caretTopLeft.render( gd::console::tag_format_cli{});  
   std::cout << m_deviceGame.render(gd::console::tag_format_cli{});
}

void CApplication::BOMB_Add()
{
   gd::argument::arguments argumentsBomb;

   auto uHeight = m_deviceGame.height();

   uint32_t uRow = rand() % uHeight;

   uint8_t uRandomColor = rand() % (255 - 16) + 16;
   
   bool bBombReuse = false;

   for(auto& itBomb : m_vectorBomb)
   {
      if(itBomb["show"] == false)
      {
         itBomb.set("show", true);
         itBomb.set("row", uRow);
         itBomb.set("column", uint32_t(0));
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
      argumentsBomb.append("color", gd::console::enumColor(uRandomColor));
      argumentsBomb.append("show", true);
      m_vectorBomb.push_back(std::move(argumentsBomb));
   }

}

void CApplication::Update()
{
   if (m_iCount % 5 == 0)
   {
      BOMB_Add();
   }
}