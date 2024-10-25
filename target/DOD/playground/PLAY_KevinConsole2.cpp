#include <random>
#include <thread>
#include <chrono>

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"



#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE(" [kevin] 03", "[kevin] ")
{
   const unsigned uRowCount = 5;
   const unsigned uColumnCount = 20;

   std::random_device randomdevice;
   std::mt19937 mt19937RandomNumber(randomdevice());
   std::uniform_int_distribution<> UIDColor(16, 255);

   gd::console::caret caretLeftTop;
   gd::console::device deviceWorm(uRowCount, uColumnCount);

   deviceWorm.create();

   for (int i = 0; i < uRowCount - 1; i++)
   {
      for (int j = 0; j < uColumnCount; j++)
      {
         deviceWorm[i][j] = "X";
         unsigned uColor = UIDColor(mt19937RandomNumber);
         deviceWorm.set_color(i, j, uColor);
      }
      std::string stringPrint;

      caretLeftTop.render(stringPrint);
      std::cout << stringPrint;

      stringPrint = "";

      deviceWorm.render(stringPrint);
      std::cout << stringPrint;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   std::cout << deviceWorm.render(gd::console::tag_format_cli{});

   std::cout << "Hello\n";
   std::cout << "Hello\n";
}