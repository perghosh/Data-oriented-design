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

std::vector<std::string> Test(const std::string& stringPath)
{
   std::vector<std::string> vectorFiles;
   std::string stringSource = stringPath;

   for( )
}