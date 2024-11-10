#include <thread>
#include <chrono>


#include "gd/gd_arguments_shared.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"

#include "worm/Application.h"

#include "worm_main.h"

/// Global pointer to application object
application::basic::CApplication* papplication_g = nullptr;

std::random_device randomdevice_g;
std::mt19937 mt19937RandomNumber_g(randomdevice_g()); 

std::pair<bool, std::string> Play();

// ## `main`

int main( int iArgumentCount, char* ppbszArgument[] )
{
   using namespace application::basic;
   // ## Initialize application and configure to get the server running
   std::unique_ptr< Application > papplication_ = std::make_unique<Application>();
   papplication_g = papplication_.get();
   papplication_g->Initialize();

   //std::cout << "\n\n##Game worm is starting...\n\n";

   papplication_->Main( iArgumentCount, ppbszArgument, nullptr );

   auto [bOk, stringError] = Play();

   return 0;
}

/// game loop
std::pair<bool, std::string> Play()
{
   Application* papplication = (Application*)papplication_g;

   papplication->Draw();

   while( papplication->GetState() != "quit" )
   {
      // papplication->PrepareFrame();

      //if( papplication->GetState() == "quit") { return { true, "quit" }; }

      papplication->GAME_Update( tag_key{} );
      papplication->GAME_Update( tag_loop{} );
      papplication->GAME_Update( tag_state{} );
      
      papplication->Draw();
      std::this_thread::sleep_for(std::chrono::milliseconds(75));
   }

   return { true, papplication->GetState() };
}



