// @FILE [tag: xml, pugixml, playground] [description: Playground for testing XML functionality]

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


std::pair<bool, std::string> ConfigurationRead_s(const std::string_view stringFile, gd::types::tag_xml )
{
   pugi::xml_document xmldocument;
   
   // Load the XML file
   pugi::xml_parse_result result = xmldocument.load_file(stringFile.data());
   if(!result) { return { false, std::string("Failed to load XML file: ") + result.description() }; }
   
   // Get the root templates node
   pugi::xml_node xmlnodeTemplates = xmldocument.child("templates");
   if(!xmlnodeTemplates) { return { false, "No 'templates' root node found in XML" }; }
   
   // ## Iterate through each template
   for(pugi::xml_node xmlnodeTemplate = xmlnodeTemplates.child("template"); xmlnodeTemplate; xmlnodeTemplate = xmlnodeTemplate.next_sibling("template"))
   {
      // ## Read template attributes
      std::string stringTemplateName = xmlnodeTemplate.attribute("name").value();
      std::string stringTemplateDescription = xmlnodeTemplate.attribute("description").value();

      // ## `metadata` element
      pugi::xml_node xmlnodeMetadata = xmlnodeTemplate.child("metadata");
      if(xmlnodeMetadata)
      {
         std::string stringAutor = xmlnodeMetadata.child("autor").text().get();
         std::string stringVersion = xmlnodeMetadata.child("version").text().get();
         std::string stringApplication = xmlnodeMetadata.child("application").text().get();
      }

      
      // Read command node
      pugi::xml_node xmlnodeCommand = xmlnodeTemplate.child("command");
      if(xmlnodeCommand)
      {
         std::string stringCommandName = xmlnodeCommand.attribute("name").value();
         std::string stringCommandDescription = xmlnodeCommand.attribute("description").value();
         std::string stringCommandData = xmlnodeCommand.text().get();

         if( stringTemplateName.empty() == true ) stringTemplateName = stringCommandName; // Use command name as template name if not specified
         
         // Process command data (remove CDATA wrapper if present)
         // Command data is stored in CDATA section
      }
      
      
      // Read configuration node
      pugi::xml_node xmlnodeConfiguration = xmlnodeTemplate.child("configuration");
      if(xmlnodeConfiguration)
      {
         pugi::xml_node xmlnodeOptions = xmlnodeConfiguration.child("options");
         if(xmlnodeOptions)
         {
            // Iterate through all options
            for(pugi::xml_node xmlnodeOption = xmlnodeOptions.child("option"); xmlnodeOption; xmlnodeOption = xmlnodeOption.next_sibling("option"))
            {
               std::string stringOptionName = xmlnodeOption.attribute("name").value();
               std::string stringOptionType = xmlnodeOption.attribute("type").value();
               std::string stringOptionRequired = xmlnodeOption.attribute("required").value();
               std::string stringOptionDefault = xmlnodeOption.attribute("default").value();
               std::string stringOptionDesc = xmlnodeOption.attribute("description").value();
               
               if(stringOptionName.empty() == true) { return { false, "Option missing required 'name' attribute" }; }
               
               // ## Validate option type
               if(stringOptionType.empty() == false &&
                  stringOptionType != "boolean" && 
                  stringOptionType != "integer" && 
                  stringOptionType != "decimal" && 
                  stringOptionType != "string") { return { false, "Invalid option type: " + stringOptionType }; }
               
               // Here you would typically store the parsed data in your configuration structure
               // For example: m_vectorOptions.push_back({stringOptionName, stringOptionType, ...});
            }
         }
      }
   }
   
   return { true, "" };                                                           // Success
}



static std::pair<bool, std::string> HistorySaveArguments_s(const std::string_view& stringArguments)
{
   // Create file
   wchar_t cProgramDataPath[MAX_PATH];

   if( !GetEnvironmentVariableW(L"ProgramData", cProgramDataPath, MAX_PATH) )
   {
      return { false, "" };
   }

   std::wstring stringDirectory = std::wstring(cProgramDataPath) + L"\\history";
   if( !std::filesystem::exists(stringDirectory) )
   {
      if( !std::filesystem::create_directory(stringDirectory) )
      {
         return { false, "" };
      }
   }

   std::wstring stringFilePath = stringDirectory + L"\\history.xml";

   pugi::xml_document xmldocument;
   pugi::xml_node commands_nodeAppend = xmldocument.append_child("commands");

   if( !xmldocument.save_file(stringFilePath.c_str()) )
   {
      return { false, "" };
   }

   // Append command
   pugi::xml_node commands_nodeChild = xmldocument.child("commands");
   if( !commands_nodeChild )
   {
      commands_nodeChild = xmldocument.append_child("commands");
   }

   commands_nodeChild.append_child("command").append_child(pugi::node_pcdata).set_value(stringArguments);
   xmldocument.save_file(stringFilePath.c_str());
}

//------------------------------------------------------------

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
   //pugi::xml_document xmldocument;

   //pugi::xml_parse_result result_ = xmldocument.load_file("C:\\temp\\kevin\\example.xml");          REQUIRE(result_ == true);
   //pugi::xml_parse_result result_ = xmldocument.load_file("D:\\kevin\\example.xml");               REQUIRE(result_ == true);

   //std::wstring filePath = "D:\\kevin\\example.xml";
   std::string stringText = "test_2";

   /*std::wstring stringName = L"history";
   std::wstring stringfilePath = CreateXMLFile(stringName);   */

   //HistoryPrint_s("1");

   /*Append(stringText, xmldocument, stringfilePath);
   Print(xmldocument, stringfilePath);
   Clear(xmldocument, stringfilePath);
   Print(xmldocument, stringfilePath);*/

}



/*

<templates>
   <template name="template-name" description="optional description">
      <command name="command name" description="optional description"><![CDATA[ raw command line string {option name needed to be filled in} ]]></command>
      <metadata>
         <autor></autor>
         <version></version>
         <application></application>
      </metadata>
      <configuration>
         <options>
            <option name="option-name" type="boolean|integer|decimal|string" required="true" default="value" description="optional description"></option>   
            <option name="option-name" type="boolean|integer|decimal|string" required="true" default="value" description="optional description"></option>   
         </options>
      </configuration>
   </template>
</templates>

*/