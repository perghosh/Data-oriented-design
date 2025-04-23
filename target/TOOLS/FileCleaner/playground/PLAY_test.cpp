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


/** ---------------------------------------------------------------------------
 * @brief Splits a semicolon-separated string into individual paths.
 *
 * This function takes a single string containing multiple paths separated by semicolons (`;`),
 * splits it into individual paths, and returns them as a vector of strings.
 *
 * @param stringPath A semicolon-separated string containing multiple paths.
 * @return A vector of strings, where each string is an individual path.
 */
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

/** ---------------------------------------------------------------------------
 * @brief Checks a list of paths and returns those that are neither regular files nor directories.
 *
 * This method iterates through a vector of file paths, checks each path to determine if it is 
 * either a regular file or a directory, and collects paths that do not satisfy either condition.
 *
 * @param vectorPath A vector of strings representing file or directory paths to be checked.
 * @return A vector of strings containing paths that are neither regular files nor directories.
 */
std::vector<std::string> CheckPath(const std::vector<std::string>& vectorPath)
{
   std::vector<std::string> vectorCheck;

   for( const auto& it : vectorPath )
   {
      std::filesystem::path pathCheck(it);

      if( std::filesystem::is_regular_file(pathCheck) == false && std::filesystem::is_directory(pathCheck) == false )
      {
         vectorCheck.push_back( pathCheck.string() );
      }
   }

   return vectorCheck;

}


/** ---------------------------------------------------------------------------
 * @brief Recursively collects all regular files from the given directories and their subdirectories.
 *
 * This function takes a vector of directory paths, iterates through each directory, and collects
 * all regular files. If a subdirectory is encountered, it recursively processes the subdirectory
 * to collect files from it as well.
 *
 * @param vectorPath A vector of strings representing directory paths to process.
 * @return A vector of strings containing the paths of all regular files found.
 */
void Test( const gd::table::dto::table& table_ )
{
   std::vector<std::string> vectorFiles;
   std::vector<std::string> vectorTemp;

   

   for( int i = 0; i < table_.get_row_count(); i++ )
   {
      const std::string& stringPath = vectorPath[i];
      for( const auto& it : std::filesystem::directory_iterator( stringPath ) )
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
}

TEST_CASE("[file] test", "[file]")
{
   //std::string stringPath = "D://dev//testfiles";
   std::string stringPath = "C://temp//kevin";

   auto ptable = std::make_unique<gd::table::dto::table>( gd::table::dto::table( 0u, { {"rstring", 0, "path"}, {"uint64", 0, "count"}, {"uint64", 0, "comment"}, {"uint64", 0, "space"} }, gd::table::tag_prepare{} ) );
   
   std::vector<std::string> vectorPath = Split(stringPath);

   std::vector<std::string> vectorCheck = CheckPath(vectorPath);

   for( auto& it : vectorCheck )
   {
      std::cout << it << "\n";
   }

   std::cout << std::endl;

   //std::vector<std::string> vectorFile = Test(vectorPath);
   int iCount = 0;

   /*for( auto& it : vectorFile )
   {
      std::cout << "Rows: " << RowCount(it) << " " << it << "\n";
      iCount += RowCount(it);
   }*/

   std::cout << std::endl;

   std::cout << iCount << " " << "Rows" << "\n";

}