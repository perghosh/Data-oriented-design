#include <iostream>

#include "Application.h"

#include "main.h"

#include <fstream>

/// Global pointer to application object
CApplication* papplication_g = nullptr;

/*void PrintFile(std::string stringFile)
{
   std::fstream fstreamFile(stringFile);
   std::string stringText;
   while( std::getline(fstreamFile, stringText) )
   {
      std::cout << stringText << std::endl;
   }

   fstreamFile.close();
}

void CountFile(std::string stringFile)
{
   std::fstream fstreamFile(stringFile);
   std::string stringText;
   int iCount = 0;
   while( std::getline(fstreamFile, stringText) )
   {
      iCount++;
   }

   std::cout << iCount << " Rows" << std::endl;

   fstreamFile.close();
} */

int main(int iArgumentCount, char** ppbszArgument)
{
   if( iArgumentCount > 1 )
   {
      for( auto i = 0; i < iArgumentCount; i++ )
      {
         std::cout << "Argument: " <<  ppbszArgument[i] << "\n";
      }
   }

   CApplication application;

   /*std::string stringCommand = ppbszArgument[1];
   if( stringCommand == "print" )
   {
      PrintFile(ppbszArgument[2]);
   }
   else if( stringCommand == "count" )
   {
      CountFile(ppbszArgument[2]);
   }
   else
   {
      std::cout << "no command\n";
   }*/


   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();


   auto result_ =  papplication_->Main( iArgumentCount, ppbszArgument, nullptr );
   if( result_.first == false ) {  std::cout << "Error: " << result_.second << "\n"; return -1; }

   result_ = papplication_->Initialize();                                                     assert( result_.first );
   if( result_.first == false ) {  std::cout << "Error: " << result_.second << "\n"; return -1; }

   return 0;
}