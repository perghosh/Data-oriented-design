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

application::basic::CApplication* papplication_g = nullptr;

std::pair<bool, std::string> Play();

int main()
{
   using namespace application::basic;
   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();

   Play();
   
   return 0;
}

std::pair<bool, std::string> Play()
{
   CApplication* papplication = (CApplication*)papplication_g;

   papplication->Initialize();

   papplication->Draw();

   papplication->BOMB_Add();

   

   while ( papplication->GetState() != "quit" )
   {
      // papplication->PrepareFrame();

      //if( papplication->GetState() == "quit") { return { true, "quit" }; }

      /*papplication->GAME_Update(tag_key{});
      papplication->GAME_Update(tag_loop{});
      papplication->GAME_Update(tag_state{});*/                 

      papplication->Count();

      papplication->Input_Update();

      papplication->Update();
      papplication->Move();
      papplication->Draw();
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
   }

   return { true, papplication->GetState() };
}