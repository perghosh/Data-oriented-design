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

void Append(std::string& stringCommand, pugi::xml_document& xmldocument, const std::string& filePath)
{
   pugi::xml_node commands_node = xmldocument.child("commands");
   if( !commands_node )
   {
      commands_node = xmldocument.append_child("commands");
   }

   commands_node.append_child("command").append_child(pugi::node_pcdata).set_value(stringCommand);
   
   xmldocument.save_file(filePath.c_str());
}

TEST_CASE("[file] test", "[file]")
{
   pugi::xml_document xmldocument;

   //pugi::xml_parse_result result_ = xmldocument.load_file("C:\\temp\\kevin\\example.xml");          REQUIRE(result_ == true);
   pugi::xml_parse_result result_ = xmldocument.load_file("D:\\kevin\\example.xml");               REQUIRE(result_ == true);

   std::string filePath = "D:\\kevin\\example.xml";

   std::string stringPath = "test";
   Append(stringPath, xmldocument, filePath);

}