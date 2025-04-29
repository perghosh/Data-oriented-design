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

std::wstring CreateXMLFile(const std::wstring& stringName)
{
   wchar_t cProgramDataPath[MAX_PATH];

   if( !GetEnvironmentVariableW(L"ProgramData", cProgramDataPath, MAX_PATH) )
   {
      return L"";
   }

   std::wstring stringDirectory = std::wstring(cProgramDataPath) + L"\\" + stringName;
   if( !std::filesystem::exists(stringDirectory) )
   {
      if( !std::filesystem::create_directory(stringDirectory) )
      {
         return L"";
      }
   }

   std::wstring stringFilePath = stringDirectory + L"\\history.xml";

   pugi::xml_document xmldocument;
   pugi::xml_node commands_node = xmldocument.append_child("commands");

   if( !xmldocument.save_file(stringFilePath.c_str()) )
   {
      return L"";
   }

   return stringFilePath;

}

void Append(std::string& stringCommand, pugi::xml_document& xmldocument, const std::wstring& filePath)
{
   pugi::xml_node commands_node = xmldocument.child("commands");
   if( !commands_node )
   {
      commands_node = xmldocument.append_child("commands");
   }

   commands_node.append_child("command").append_child(pugi::node_pcdata).set_value(stringCommand);
   xmldocument.save_file(filePath.c_str());
}

void Print(pugi::xml_document& xmldocument, const std::wstring& filePath)
{
   auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "command"} }, gd::table::tag_prepare{}));
   pugi::xml_node commands_node = xmldocument.child("commands");

   for( auto command : commands_node.children("command") )
   {
      std::string stringCommand = command.child_value();
      ptable->row_add();
      ptable->cell_set(ptable->get_row_count() - 1, "command", stringCommand);
   }

   auto stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});

   std::cout << "\n" << stringTable << "\n";
}

void Clear(pugi::xml_document& xmldocument, const std::wstring& filePath)
{
   pugi::xml_node commands_node = xmldocument.child("commands");
   commands_node.remove_children();
   xmldocument.save_file(filePath.c_str());
}

TEST_CASE("[file] test", "[file]")
{
   pugi::xml_document xmldocument;

   //pugi::xml_parse_result result_ = xmldocument.load_file("C:\\temp\\kevin\\example.xml");          REQUIRE(result_ == true);
   pugi::xml_parse_result result_ = xmldocument.load_file("D:\\kevin\\example.xml");               REQUIRE(result_ == true);

   //std::wstring filePath = "D:\\kevin\\example.xml";
   std::string stringText = "test_2";

   std::wstring stringName = L"history";
   std::wstring stringfilePath = CreateXMLFile(stringName);

   Append(stringText, xmldocument, stringfilePath);
   Print(xmldocument, stringfilePath);
   Clear(xmldocument, stringfilePath);
   Print(xmldocument, stringfilePath);

}