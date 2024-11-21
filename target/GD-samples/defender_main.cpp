#include <thread>
#include <chrono>


#include "gd/gd_arguments_shared.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"

#include "defender/Application.h"

#include "defender_main.h"

int main()
{
   std::pair<int, int> pairTest(5, 5);

   Application applicationTest(pairTest);
   applicationTest.Initialize();

   while (true)
   {
      applicationTest.Draw();

      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   }
   
   return 0;
}