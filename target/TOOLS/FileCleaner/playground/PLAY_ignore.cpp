#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "pugixml/pugixml.hpp"

#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"

std::vector<std::string> ReadIgnoreList(const std::string& stringPath)
{
   std::vector<std::string> vectorExtensions;
   std::ifstream ifstreamFile;
   ifstreamFile.open(stringPath);

   auto bSingleChar = [](const std::string& stringText, char iType)
   {
      for( int i = 0; i < stringText.size(); i++ )
      {
         if( stringText[i] != iType )
         {
            return false;
         }
      }
      return true;
   };

   if( ifstreamFile.is_open() == true )
   {
      std::string stringLine;
      while( std::getline(ifstreamFile, stringLine) )
      {
         for( int i = 0; i < stringLine.size(); i++ )
         {
            if( stringLine[i] == '#' )
            {
               break;
            }
            else if( bSingleChar(stringLine, ' ') == true )
            {
               break;
            }
            else
            {
               vectorExtensions.push_back(stringLine);
               break;
            }
         }
      }
      return vectorExtensions;
   }
   else
   {
      std::cout << "Failed to open file: " << stringPath << "\n";
   }
}

TEST_CASE("[file] test", "[file]")
{
   std::cout << "h\n";
   //std::string stringPath = "C:\\dev\\work\\DOD\\.gitignore";
   std::string stringPath = "D:\\dev\\testfiles\\testignore.txt";
   std::vector<std::string> vectorList = ReadIgnoreList(stringPath);

   for( std::string stringExtension : vectorList )
   {
      std::cout << stringExtension << std::endl;
   }
}