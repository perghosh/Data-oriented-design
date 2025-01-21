#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/console/gd_console_print.h"
#include "gd/console/gd_console_style.h"

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
   
   box(unsigned uHeight, unsigned uWidth): m_uRow(0), m_uColumn(0), m_uHeight(uHeight), m_uWidth(uWidth) {}
   box(unsigned uRow, unsigned uColumn, unsigned uHeight, unsigned uWidth) : m_uRow(uRow), m_uColumn(uColumn), m_uHeight(uHeight), m_uWidth(uWidth) {}

   box& move_up() { m_uRow--; return *this; }
   box& move_up(unsigned uMove) { m_uRow -= uMove; return *this; }
   box& move_down() { m_uRow++; return *this; }
   box& move_down(unsigned uMove) { m_uRow += uMove; return *this; }
   
   box& move_left() { m_uColumn--; return *this; }
   box& move_left(unsigned uMove) { m_uColumn -= uMove; return *this; }
   box& move_right() { m_uColumn++; return *this; }
   box& move_right(unsigned uMove) { m_uColumn += uMove; return *this; }

   box& increase_width() { m_uWidth++; return *this; }
   box& increase_width(unsigned uChange) { m_uWidth += uChange; return *this; }
   box& decrease_width() { m_uWidth--; return *this; }
   box& decrease_width(unsigned uChange) { m_uWidth -= uChange; return *this; }

   box& increase_height() { m_uHeight++; return *this; }
   box& increase_height(unsigned uChange) { m_uHeight += uChange; return *this; }
   box& decrease_height() { m_uHeight--; return *this; }
   box& decrease_height(unsigned uChange) { m_uHeight -= uChange; return *this; }

   void print(gd::console::device* pdevice, char iCharacter)
   {
      unsigned uR = m_uRow, uC = m_uColumn;

      for (unsigned int ui = 0; ui < m_uHeight; ui++)
      {
         for (unsigned int uj = 0; uj < m_uWidth; uj++)
         {
            pdevice->print(ui + uR, uj + uC, iCharacter);
         }
      }
   }

   char m_iCharacter = 0;
   unsigned int m_uRow;
   unsigned int m_uColumn;

   unsigned int m_uHeight;
   unsigned int m_uWidth;
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

   box boxTest(1, 1, 10, 10);

   triangle triangleTest(1, 1);

   deviceTest.create(20, 100);

   auto stringOut = deviceTest.render(gd::console::tag_format_cli{});
   std::cout << stringOut;
   
   boxTest.print(&deviceTest, '*');
   boxTest.decrease_height(5);
   boxTest.decrease_width(5);

   boxTest.move_right(20);

   boxTest.print(&deviceTest, '*');

   /*triangleTest.move_right(30);
   triangleTest.print(&deviceTest, 10, '#');*/

   //std::cout << " ";

   stringOut = deviceTest.render(gd::console::tag_format_cli{});
   std::cout << stringOut;

   std::cout << "Hello\n";


   

}