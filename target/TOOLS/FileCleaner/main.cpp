#include <iostream>
#include <thread>

#include "gd/console/gd_console_style.h"

#include "Application.h"

#include "main.h"

#include <fstream>


int main(int iArgumentCount, char** ppbszArgument)                             // @TAG #main
{
   using namespace gd::console;
#ifndef NDEBUG
   if( iArgumentCount > 1 )
   {
      for( auto i = 0; i < iArgumentCount; i++ )
      {
         std::cout << "Argument: " <<  ppbszArgument[i] << "\n";
      }
   }
#endif // NDEBUG

   CApplication application;

   try
   {
      // ## Initialize application and configure to get the server running
      std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
      papplication_g = papplication_.get();


      auto result_ =  papplication_->Main( iArgumentCount, ppbszArgument, nullptr );
      if( result_.first == false ) { std::cout << to_color( enumColor::eColorRed1 ) << "\nERROR: " << result_.second << "\n"; return -1; } // @TAG #error


      std::string stringError = papplication_g->ERROR_Report();
      if( stringError.empty() == false )
      {
         std::cout << "\n\nFound internal errors: " << stringError << "\n";
      }

      if( papplication_g->IsWork() == true )
      {
         // Wait until application is idle, in loop we check the flag until idle is set
         while( papplication_g->IsWork() == true )
         {
            // Optionally, sleep to avoid busy-waiting, this will lower the cpu work
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
         }
      }

      papplication_->Exit();
   }
   catch( const std::exception& e_ )
   {
      std::cout << to_color( enumColor::eColorRed1 ) << "\n\nError: " << e_.what() << "\n";        assert(false);
   }
   catch( const std::string& s_ )
   {
      std::cout << to_color( enumColor::eColorRed1 ) << "\n\nError: " << s_ << "\n";               assert(false);
   }
   catch( const char* p_ )
   {
      std::cout << to_color( enumColor::eColorRed1 ) << "\n\nError: " << p_ << "\n";               assert(false);
   }
   catch( ... )
   {
      std::cout << to_color( enumColor::eColorRed1 ) << "\n\nError: Unknown exception\n";          assert(false);
   }

   std::cout << to_color_reset() << std::endl;

   return 0;
}