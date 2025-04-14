#include "Command.h"

int RowCount( const std::string& stringFile )
{
   if( std::filesystem::is_regular_file(stringFile) == true )
   {
      std::ifstream ifstreamFile(stringFile);
      std::string stringText;
      int iCount = 0;

      while( std::getline(ifstreamFile, stringText) )
      {
         iCount++;
      }

      ifstreamFile.close();

      return iCount;
   }

   return 0;
}