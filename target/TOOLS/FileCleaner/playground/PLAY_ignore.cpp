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


// Clean the extension list by removing spaces and dots
std::vector<std::string> CleanExtension(std::vector<std::string> vectorList)
{
   std::vector<std::string> vectorExtensions;
   for( int i = 0; i < vectorList.size(); i++ )
   {
      std::string stringExtension = vectorList[i];
      std::string stringTemp;

      for( int i2 = 0; i2 < stringExtension.size(); i2++ )
      {
         if( stringExtension[i2] != ' ' && stringExtension[i2] != '*' )      //stringExtension[i2] != '.' &&
         {
            stringTemp += stringExtension[i2];
         }
      }
      vectorExtensions.push_back(stringTemp);
   }
   return vectorExtensions;
}

// Read the ignore list from a file
std::vector<std::string> ReadIgnoreList(const std::string& stringPath)
{
   std::vector<std::string> vectorExtensions;
   std::ifstream ifstreamFile;
   ifstreamFile.open(stringPath);

   // Check if the string is a single character
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
      return CleanExtension(vectorExtensions);
   }
   else
   {
      std::cout << "Failed to open file: " << stringPath << "\n";
   }
}

// Print the files in a directory that are not in the ignore list
void PrintFiles(const std::string& stringPath, const std::vector<std::string> vectorList)
{
   auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "path"} }, gd::table::tag_prepare{}));

   for( const auto& it : std::filesystem::directory_iterator(stringPath) )
   {
      if( it.is_regular_file() || it.is_directory() )
      {
         bool bFound = false;
         for( int i = 0; i < vectorList.size(); i++ )
         {
            if( it.path().extension() == vectorList[i] )
            {
               bFound = true;
               break;
            }
         }
         if( bFound == false )
         {
            std::string stringFilePath = it.path().string();
            ptable->row_add();
            ptable->cell_set(ptable->get_row_count() - 1, "path", stringFilePath);
         }
      }
   }

   auto stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   std::cout << stringTable << "\n";
}

TEST_CASE("[file] test", "[file]")
{
   std::cout << "h\n";
   std::string stringPath = "C:\\dev\\work\\DOD\\.gitignore";
   //std::string stringPath = "D:\\dev\\testfiles\\testignore.txt";
   std::vector<std::string> vectorList = ReadIgnoreList(stringPath);
   //std::string stringDirectory = "D:\\dev\\work\\DOD\\target\\TOOLS\\FileCleaner\\cli";
   std::string stringDirectory = "D:\\dev\\work\\DOD\\target\\TOOLS\\FileCleaner\\cli";

   for( std::string stringExtension : vectorList )
   {
      std::cout << stringExtension << std::endl;
   }

   PrintFiles(stringDirectory, vectorList);
}