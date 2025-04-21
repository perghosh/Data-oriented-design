#include <iostream>

#include "Application.h"

#include "main.h"

#include <fstream>


int main(int iArgumentCount, char** ppbszArgument)
{
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

   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();


   auto result_ =  papplication_->Main( iArgumentCount, ppbszArgument, nullptr );
   if( result_.first == false ) { std::cout << "Error: " << result_.second << "\n"; assert( false ); return -1; }

   result_ = papplication_->Initialize();                                                     assert( result_.first );
   if( result_.first == false ) { assert( false ); std::cout << "Error: " << result_.second << "\n"; assert( false ); return -1; }

   return 0;
}