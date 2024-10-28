#include "gd/gd_arguments_shared.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"


#include "worm_main.h"

/// Global pointer to application object
application::basic::CApplication* papplication_g = nullptr;

std::random_device randomdevice_g;
std::mt19937 mt19937RandomNumber_g(randomdevice_g()); 


int main( int iArgumentCount, char* ppbszArgument[] )
{
   using namespace application::basic;
   // ## Initialize application and configure to get the server running
   std::unique_ptr< Application > papplication_ = std::make_unique<Application>();
   papplication_g = papplication_.get();
   papplication_g->Initialize();

   //std::cout << "\n\n##Game worm is starting...\n\n";

   papplication_->Main( iArgumentCount, ppbszArgument, nullptr );

   return 0;
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------- Worm
// ----------------------------------------------------------------------------



std::pair< bool, std::string > Worm::Create()
{

   m_argumentsWorm.clear();

   m_argumentsWorm.append( "head", ToBodyPart_s( 5, 10 ) );                     // head at position 5,5

   m_argumentsWorm.append( "body", ToBodyPart_s( 5, 9 ) );
   m_argumentsWorm.append_many( ToBodyPart_s( 5, 8 ), ToBodyPart_s( 5, 7 ), ToBodyPart_s( 5, 6 ), ToBodyPart_s( 5, 5 ) );
      
   return { true, "" };
}


std::pair<bool, std::string> Application::Initialize()
{
   // ## Get information about terminal size

   auto rowcolumn_ = gd::console::device::terminal_get_size_s();

   // ## create device
   m_deviceGame = rowcolumn_;
   m_deviceGame.create();

   return application::basic::CApplication::Initialize();
}