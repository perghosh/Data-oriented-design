#include <iostream>

#include "Application.h"

/// Global pointer to application object
CApplication* papplication_g = nullptr;


int main( int iArgumentCount, char* ppbszArgument[] )
{
   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();

   std::cout << "we have a message";

   //papplication_->Main( iArgumentCount, ppbszArgument, nullptr );
   //auto result_ = papplication_->Initialize();                                                     assert( result_.first );
   //result_ = papplication_->SERVER_Start();

   return 0;
}