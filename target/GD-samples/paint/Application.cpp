#include "conio.h"


#include <format>
#include <random>

#ifndef NDEBUG
   #include "gd/gd_debug.h"
#endif

#include "Application.h"

std::pair<bool, std::string> CApplication::Initialize()
{
   //::srand((unsigned)::time(NULL));                                           // init random numbers

   m_deviceGame.create(20, 80);

   BRUSH_Reset();

   return application::basic::CApplication::Initialize();
}

void CApplication::Move()
{
   auto [uDeviceHeight, uDeviceWidth] = m_deviceGame.size();

   uint32_t uBrushRow = m_argumentsBrush("row");
   uint32_t uBrushColumn = m_argumentsBrush("column");

   int32_t iBrushMoveY = m_argumentsBrush("move_row");
   int32_t iBrushMoveX = m_argumentsBrush("move_column");

   m_argumentsBrush.set("row", (uBrushRow + iBrushMoveY));
   m_argumentsBrush.set("column", (uBrushColumn + iBrushMoveX));
   m_argumentsBrush.set("move_row", int32_t(0)); 
   m_argumentsBrush.set("move_column", int32_t(0));
}

void CApplication::Draw()
{
   m_deviceGame.fill(' ');

   uint32_t uBrushRow = m_argumentsBrush("row");
   uint32_t uBrushColumn = m_argumentsBrush("column");

   m_deviceGame.print(uBrushRow, uBrushColumn, 177);

   std::cout << m_caretTopLeft.render(gd::console::tag_format_cli{});
   std::cout << m_deviceGame.render(gd::console::tag_format_cli{});
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
         m_argumentsBrush.set("move_row", int32_t(-1));
         break;
      case 's':
         m_stringState = "down";
         m_argumentsBrush.set("move_row", int32_t(1));
         break;
      case 'a':
         m_stringState = "left";
         m_argumentsBrush.set("move_column", int32_t(-1));
         break;
      case 'd':
         m_stringState = "right";
         m_argumentsBrush.set("move_column", int32_t(1));
         break;
      default:
         break;
      }
   }
   return { true, "" };
}

void CApplication::PAINT_Add()
{

}

void CApplication::BRUSH_Reset()
{
   m_argumentsBrush.clear();
   m_argumentsBrush.append("row", uint32_t(5));
   m_argumentsBrush.append("column", uint32_t(5));

   m_argumentsBrush.append("move_row", int32_t(0));
   m_argumentsBrush.append("move_column", int32_t(0));

}
