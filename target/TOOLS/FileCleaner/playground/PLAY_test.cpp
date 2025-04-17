#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

// - take directories
//   - input string
//   - input string seperated with semicolon
//   - split string
//   - for every string read files
//       - place in table

std::string Test(const std::string& stringPath)
{
   std::vector<std::string> vectorPath;
   std::string stringTemp;
   std::string stringFiles;
   int iCount = 0;

   for( int i = 0; i <= stringPath.size(); i++ )
   {
      if( stringPath[i] == ';' || i == stringPath.size() )
      {
         for( int i2 = iCount; i2 < i; i2++ )
         {
            stringTemp += stringPath[i2];
         }
         vectorPath.push_back(stringTemp);
         stringTemp.clear();
         iCount = i + 1;
      }
   }

   for( int i = 0; i < vectorPath.size(); i++ )
   {
      for( const auto& it : std::filesystem::directory_iterator(vectorPath[i]) )     // i get error here
      {
         if( it.is_regular_file() )
         {
            std::string stringFilePath = it.path().string();
            stringFiles += stringFilePath + ";";
         }
      }
   }


   return stringFiles;
}

TEST_CASE("[file] test", "[file]")
{
   std::string stringPath = "D://dev//testfiles";
   std::string stringFiles = Test(stringPath);

   std::cout << stringFiles << "\n";

}