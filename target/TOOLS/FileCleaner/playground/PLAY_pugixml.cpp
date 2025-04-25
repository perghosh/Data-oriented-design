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

TEST_CASE("[file] test", "[file]")
{
   pugi::xml_document xmldocument;

   pugi::xml_parse_result result_ = xmldocument.load_file("C:\\temp\\kevin\\example.xml");          REQUIRE(result_ == true);

   pugi::xpath_node john_node = xmldocument.select_node(
      "/Profile/User[Name='John Doe']"
   );



}