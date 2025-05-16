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

   try
   {
      // ## Initialize application and configure to get the server running
      std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
      papplication_g = papplication_.get();


      auto result_ =  papplication_->Main( iArgumentCount, ppbszArgument, nullptr );
      if( result_.first == false ) { std::cout << "Error: " << result_.second << "\n"; return -1; }

      result_ = papplication_->Initialize();                                                     assert( result_.first );
      if( result_.first == false ) { assert( false ); std::cout << "Error: " << result_.second << "\n"; assert( false ); return -1; }

      std::string stringError = papplication_g->ERROR_Report();
      if( stringError.empty() == false )
      {
         std::cout << "\n\nFound internal errors: " << stringError << "\n";
      }

      papplication_->Exit();
   }
   catch( const std::exception& e_ )
   {
      std::cout << "\n\nError: " << e_.what() << "\n";                                             assert(false);
   }
   catch( const std::string& s_ )
   {
      std::cout << "\n\nError: " << s_ << "\n";                                                    assert(false);
   }
   catch( const char* p_ )
   {
      std::cout << "\n\nError: " << p_ << "\n";                                                    assert(false);
   }
   catch( ... )
   {
      std::cout << "\n\nError: Unknown exception\n";                                               assert(false);
   }

   return 0;
}