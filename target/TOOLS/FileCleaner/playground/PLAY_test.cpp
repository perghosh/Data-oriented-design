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

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"

// - take directories
//   - input string
//   - input string seperated with semicolon
//   - split string
//   - for every string read files
//       - place in table

//   -return vector

std::vector<std::string> Split(const std::string& stringPath)
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

   return vectorPath;
}

std::vector<std::pair<bool, std::string>> CheckPath(const std::vector<std::string>& vectorPath)
{
   std::vector<std::pair<bool, std::string>> vectorCheck;

   for( const auto& it : vectorPath )
   {
      std::pair<bool, std::string> pairCheck;

      std::filesystem::path pathCheck(it);

      pairCheck.second = pathCheck.string();

      if( !std::filesystem::is_regular_file(pathCheck) || !std::filesystem::is_directory(pathCheck) )
      {
         pairCheck.first = false;

         vectorCheck.push_back(pairCheck);
      }
      else
      {
         pairCheck.first = true;

         vectorCheck.push_back(pairCheck);
      }
   }

   return vectorCheck;

}

std::vector<std::string> Test(const std::vector<std::string>& vectorPath)
{
   std::vector<std::string> vectorFiles;
   std::vector<std::string> vectorTemp;

   for( int i = 0; i < vectorPath.size(); i++ )
   {
      for( const auto& it : std::filesystem::directory_iterator(vectorPath[i]) )
      {
         if( it.is_regular_file() )
         {
            std::string stringFilePath = it.path().string();
            vectorFiles.push_back(stringFilePath);
         }
         else if( it.is_directory() )
         {
            std::string stringFilePath = it.path().string();
            vectorTemp.push_back(stringFilePath);

            auto files = Test(vectorTemp);
            for( const auto& it : files )
            {
               vectorFiles.push_back(it);
            }
         }
      }
   }

   return vectorFiles;
}

TEST_CASE("[file] test", "[file]")
{
   //std::string stringPath = "D://dev//testfiles";
   std::string stringPath = "C://temp//kevin";
   
   std::vector<std::string> vectorPath = Split(stringPath);

   std::vector<std::pair<bool, std::string>> vectorCheck = CheckPath(vectorPath);

   for( auto& it : vectorCheck )
   {
      std::cout << it.first << " " << it.second << "\n";
   }

   std::cout << std::endl;

   std::vector<std::string> vectorFile = Test(vectorPath);
   int iCount = 0;

   for( auto& it : vectorFile )
   {
      std::cout << "Rows: " << RowCount(it) << " " << it << "\n";
      iCount += RowCount(it);
   }

   std::cout << std::endl;

   std::cout << iCount << " " << "Rows" << "\n";

}