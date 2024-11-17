#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"

#include <chrono>
#include <thread>

#include "main.h"


#include "catch2/catch_amalgamated.hpp"

void MoveCursor(int iRow, int iColumn)
{
   std::cout << "\033[" << iRow << ";" << iColumn << "H";
}

struct box
{
public:
   
   box(unsigned int uRow, unsigned int uColumn): m_uRow(uRow), m_uColumn(uColumn) {}

   box& move_up() { m_uRow--; return *this; }
   box& move_up(unsigned uMove) { m_uRow -= uMove; return *this; }
   box& move_down() { m_uRow++; return *this; }
   box& move_down(unsigned uMove) { m_uRow += uMove; return *this; }
   
   box& move_left() { m_uColumn--; return *this; }
   box& move_left(unsigned uMove) { m_uColumn -= uMove; return *this; }
   box& move_right() { m_uColumn++; return *this; }
   box& move_right(unsigned uMove) { m_uColumn += uMove; return *this; }

   void print(gd::console::device* pdevice, unsigned int uLength, unsigned int uWidth, char iCharacter)
   {
      unsigned uR = m_uRow, uC = m_uColumn;

      for (unsigned int ui = 0; ui < uLength; ui++)
      {
         for (unsigned int uj = 0; uj < uWidth; uj++)
         {
            pdevice->print(ui + uR, uj + uC, iCharacter);
         }
      }
   }

   char m_iCharacter = 0;
   unsigned int m_uRow;
   unsigned int m_uColumn;
};

struct triangle
{
public:

   triangle(unsigned int uRow, unsigned int uColumn) : m_uRow(uRow), m_uColumn(uColumn) {}

   triangle& move_up() { m_uRow--; return *this; }
   triangle& move_up(unsigned uMove) { m_uRow -= uMove; return *this; }
   triangle& move_down() { m_uRow++; return *this; }
   triangle& move_down(unsigned uMove) { m_uRow += uMove; return *this; }

   triangle& move_left() { m_uColumn--; return *this; }
   triangle& move_left(unsigned uMove) { m_uColumn -= uMove; return *this; }
   triangle& move_right() { m_uColumn++; return *this; }
   triangle& move_right(unsigned uMove) { m_uColumn += uMove; return *this; }

   void print(gd::console::device* pdevice, unsigned int uSize, char iCharacter)
   {
      unsigned uR = m_uRow, uC = m_uColumn;

      for (int i = 0; i < uSize; i++)
      {
         for (int j = 0; j < i + 1; j++)
         {
            pdevice->print(i + uR, j + uC, iCharacter);
         }
      }

   }

   char m_iCharacter = 0;
   unsigned int m_uRow;
   unsigned int m_uColumn;
};

TEST_CASE(" [kevin] 02", "[kevin] ")
{
   gd::console::device deviceTest(250, 100);
   deviceTest.create();

   box boxTest(1, 1);

   triangle triangleTest(1, 1);

   deviceTest.create(20, 100);

   auto stringOut = deviceTest.render(gd::console::tag_format_cli{});
   std::cout << stringOut;
   
   boxTest.print(&deviceTest, 5, 10, '*');

   boxTest.move_right(10);

   boxTest.print(&deviceTest, 5, 15, '-');

   boxTest.move_down(5);
   boxTest.move_left(10);

   boxTest.print(&deviceTest, 5, 25, '-');


   triangleTest.move_right(30);
   triangleTest.print(&deviceTest, 10, '#');

   //std::cout << " ";

   stringOut = deviceTest.render(gd::console::tag_format_cli{});
   std::cout << stringOut;

   std::cout << "Hello\n";


   

}