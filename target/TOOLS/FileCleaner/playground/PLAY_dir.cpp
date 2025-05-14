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

void Directory(const std::string& stringPath)
{

   auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "path"} }, gd::table::tag_prepare{}));

   for( const auto& it : std::filesystem::directory_iterator(stringPath) )
   {
      if( it.is_regular_file() || it.is_directory() )
      {
         std::string stringFilePath = it.path().string();
         ptable->row_add();
         ptable->cell_set(ptable->get_row_count() - 1, "path", stringFilePath);
      }
   }

   auto stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   std::cout << stringTable << "\n";
}

TEST_CASE("[file] test", "[file]")
{
   std::string stringPath = "C://temp//kevin";
   Directory(stringPath);
}