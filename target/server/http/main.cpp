#include <iostream>

#include "Application.h"

/// Global pointer to application object
//CApplication* papplication_g = nullptr;


/** --------------------------------------------------------------------------- @API [tag: main] [description: start application] [type: method]
 * @brief man is as you know where the application starts
 */
int main( int iArgumentCount, char* ppbszArgument[] )
{
   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();

   //std::cout << "we have a message";

   auto result_ =  papplication_->Main( iArgumentCount, ppbszArgument, nullptr );

   if( result_.first == false )
   {
      std::cout << "## Server exit with error: "  << result_.second << std::endl;
   }
   
   //result_ = papplication_->SERVER_Start();

   return 0;
}
