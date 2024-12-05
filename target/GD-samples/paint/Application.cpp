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



   for( const auto& itPaint : m_vectorPaint )
   {

      uint32_t uRow = itPaint["row"];
      uint32_t uColumn = itPaint["column"];
      uint8_t uColor = itPaint["color"];
     
      std::string stringCharacter = itPaint["character"];

      m_deviceGame.print(uRow, uColumn, stringCharacter, uColor);

   }

   m_deviceGame.print(uBrushRow, uBrushColumn, 176);

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

      if( iKey == ' ' && m_uInputCount <= 1 )
      {
         m_bPaintState = true;
         m_uInputCount++;

         std::cout << "paint";
      }
      else if( m_uInputCount == 2)
      {
         m_bPaintState = false;
         m_uInputCount = 0;
         std::cout << "     ";
      }

   }
   return { true, "" };
}

void CApplication::PAINT_Add()
{
   gd::argument::arguments argumentsPaint;

   uint32_t uRow = m_argumentsBrush("row");
   uint32_t uColumn = m_argumentsBrush("column");

   uint8_t uColor = 46;

   if( m_bPaintState == true )
   {
      argumentsPaint.append("row", uRow);
      argumentsPaint.append("column", uColumn);
      argumentsPaint.append("color", gd::console::enumColor(uColor));
      argumentsPaint.append("character", "#");

      m_vectorPaint.push_back(std::move(argumentsPaint));
   }



}

void CApplication::BRUSH_Reset()
{
   m_argumentsBrush.clear();
   m_argumentsBrush.append("row", uint32_t(5));
   m_argumentsBrush.append("column", uint32_t(5));

   m_argumentsBrush.append("move_row", int32_t(0));
   m_argumentsBrush.append("move_column", int32_t(0));

}
