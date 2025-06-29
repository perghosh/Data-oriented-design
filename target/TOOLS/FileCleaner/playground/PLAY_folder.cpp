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

std::string CombineDirectory(const std::string_view& stringPath, const std::string_view& stringName)
{
   std::filesystem::path pathDirectory = std::filesystem::path(stringPath) / stringName;

   return pathDirectory.string();
}

void Create(const std::string_view& stringPath, const std::string_view& stringName)
{

   std::filesystem::path pathDirectory = std::filesystem::path(stringPath) / stringName;
   //std::string stringDirectory = CombineDirectory(stringPath, stringName);

   if( std::filesystem::exists(pathDirectory) == false )
   {
      std::ofstream ofstreamFile(pathDirectory);
      ofstreamFile << "Hello\n";
      ofstreamFile.close();
   }
   else
   {
      std::cout << "File already exist" << std::endl;
   }
}

void CreateFolder(const std::string_view& stringPath, const std::string_view& stringName)
{
   std::filesystem::path pathDirectory = std::filesystem::path(stringPath) / stringName;

   if( std::filesystem::create_directory(pathDirectory) )
   {
      std::cout << pathDirectory << std::endl;
   }
   else
   {
      std::cout << "Folder already exist" << std::endl;
   }
}

TEST_CASE("[folder] test", "[folder]")
{
   std::string stringDirectory = "D:\\dev\\testfiles";
   std::string stringName = ".cleaner";

   std::string stringPath = CombineDirectory(stringDirectory, stringName);
   std::string stringFileName = "TestFile.txt";

   CreateFolder(stringDirectory, stringName);
   Create(stringPath, stringFileName);
}