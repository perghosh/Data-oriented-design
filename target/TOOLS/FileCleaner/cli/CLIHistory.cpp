/**
* @file CLIHistory.cpp
* @brief Implements history management for the cleaner console application.
*
* @section history_file_format History File Format
*
* The history information for the cleaner console application is stored in either XML or JSON format.
* Each entry records an operation performed by the cleaner, including a timestamp, operation type, and optional details.
*
* @par XML Format Example
* @code{.xml}
* <?xml version="1.0" encoding="UTF-8"?>
 <history>
   <entries>
     <entry>
       <timestamp>2025-07-01T12:34:56Z</timestamp>
       <operation>delete</operation>
       <arguments>Removed 5 files from temp directory</arguments>
     </entry>
     <entry>
       <timestamp>2025-07-01T13:00:00Z</timestamp>
       <operation>count</operation>
       <arguments><![CDATA[ --source  "C:\\dev\\home\\DOD\\external\\gd\\" --recursive 3 --pattern "std::string_view,std::string,std::list -print]]></arguments>
     </entry>
     <entry>
       <timestamp>2025-07-01T14:00:00Z</timestamp>
       <operation>count</operation>
       <arguments><![CDATA[ --source  "*" -R --pattern "@TAG" --segment string -print]]></arguments>
     </entry>
   </entries>
   <entries>
    <entry sequence="001">
      <when>2025-07-01T11:17:00Z</when>
      <what>
        <operation>count</operation>
        <parameters><![CDATA[--source "C:\\dev\\home\\DOD\\external\\gd\\" --recursive 3 --pattern "std::string_view,std::string,std::list" --print]]></parameters>
      </what>
      <outcome>
        <status>success</status>
        <duration>1.25s</duration>
        <matches-found>42</matches-found>
        <files-scanned>156</files-scanned>
      </outcome>
      <annotations>
        <user-note>String analysis for GD library refactoring</user-note>
        <bookmarked>yes</bookmarked>
        <project>gd-modernization</project>
      </annotations>
    </entry>
    </entries>
* </history>
* 
<CommandHistory>
   <Commands>
       <Command>
           <CommandString><![CDATA[ --source  "C:\\dev\\home\\DOD\\external\\gd\\" --recursive 3 --pattern "std::string_view,std::string,std::list -print]]></CommandString>
           <Date>2025-07-01T11:17:00Z</Date>
           <Type>count</Type>
           <Favorite>true</Favorite>
           <Description>Count occurrences of std::string_view in specified directory</Description>
       </Command>
       <Command>
           <CommandString><![CDATA[ --source  "*" -R --pattern "@TAG" --segment string -print]]></CommandString>
           <Date>2025-07-01T10:30:00Z</Date>
           <Type>count</Type>
           <Favorite>false</Favorite>
           <Description>Count occurrences of @TAG in all files</Description>
       </Command>
       <Command>
           <CommandString><![CDATA[ --source "C:\\Users\\NewUser\\Documents" --recursive 2 --pattern "example" -print]]></CommandString>
           <Date>2025-07-02T09:15:00Z</Date>
           <Type>delete</Type>
           <Favorite>false</Favorite>
           <Description>Delete files matching example pattern in specified directory</Description>
       </Command>
   </Commands>
</CommandHistory> 
* @endcode
*
* @par JSON Format Example
* @code{.json}
* {
*   "history": [
*     {
*       "timestamp": "2025-07-01T12:34:56Z",
*       "operation": "delete",
*       "details": "Removed 5 files from temp directory"
*     },
*     {
*       "timestamp": "2025-07-01T13:00:00Z",
*       "operation": "scan",
*       "details": "Scanned C:\\Users\\Example\\Downloads"
*     }
*   ]
* }
* @endcode
*
* @section history_file_fields Fields
* - @b timestamp: ISO 8601 formatted date and time of the operation.
* - @b operation: The type of operation performed (e.g., "delete", "scan").
* - @b details: (Optional) Additional information about the operation.
*/

// @TAG #cli #history

#include <fstream>
#include <pugixml/pugixml.hpp>

#include "gd/expression/gd_expression_runtime.h"


#include "../Application.h"

#include "CLIHistory.h"




NAMESPACE_CLI_BEGIN


// ## History operations

// ## Forward declarations

static std::pair<bool, std::string> HistoryPrepareXml_s(const gd::argument::arguments& argumentsXml);

static std::pair<bool, std::string> HistoryAppendEntry_s(const gd::argument::arguments& argumentsEntry);

static std::pair<bool, std::string> CreateTable_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable);

static std::string CurrentTime_s();

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory)
{
   const gd::cli::options& options_ = *poptionsHistory;
   if( options_.exists("create") == true )
   {
      gd::argument::arguments argumentsCreate( {"create", options_["create"].as_string()} );
      auto result_ = HistoryCreate_g(argumentsCreate);
   }

   return { true, "" };
}


/**
 * @brief Set up folders for cleaner and creates history file if not exists
 */
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate )
{
   //auto result_ = papplication_g->CreateDirectory();                                               if( result_.first == false ) { return result_; }

   std::string stringName = "history.xml";
   std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   std::string stringFilePath = (pathCurrentDirectory / stringName).string();
   gd::argument::arguments argumentsFile({ {"file", stringFilePath} , {"create", true}, {"command", "command1"}, {"line", "line1"}, {"line", "line2"}, {"date", CurrentTime_s()} } );     // TODO: This is just temporary, we need to edit this later
    // To create a string representing the full path to "history.xml" in pathCurrentDirectory:

   if( std::filesystem::exists(pathCurrentDirectory) == false )
   {
      std::filesystem::create_directory(pathCurrentDirectory); 
      std::ofstream ofstreamFile(pathCurrentDirectory / stringName);
      ofstreamFile.close();
      HistoryPrepareXml_s(argumentsFile); // Prepare the XML file if it does not exist

      HistoryAppendEntry_s(argumentsFile); // TODO: This is just temporary, we need to remove this later

      HistoryPrint_g(argumentsFile); // Print the history file to console, this is just for debug purposes

      HistoryDelete_g(argumentsCreate); // TODO: This is just temporary, we need to remove this later
   }
   else
   {
      HistoryDelete_g(argumentsCreate); // TODO: This is just temporary, we need to remove this later
   }

   return { true, "" };
}

std::pair<bool, std::string> HistoryDelete_g(const gd::argument::arguments& argumentsDelete)
{
   std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   if( std::filesystem::exists(pathCurrentDirectory) == true  )
   {
      std::filesystem::remove_all(pathCurrentDirectory); // remove the history folder
   }

   return { true, "" };
}

std::pair<bool, std::string> HistoryPrint_g(const gd::argument::arguments& argumentsPrint)
{
   std::string stringFileName = argumentsPrint["file"].as_string();
                                                                               assert(!stringFileName.empty());
   auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));

   CreateTable_s(*ptable, argumentsPrint); // Create the table from the XML file

   std::string stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   std::cout << "\n" << stringTable << "\n";


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prepare XML file for history
 * 
 * This function checks if the specified XML file exists, and if not, it creates a new XML document with a root node "history".
 * If the file exists, it loads the XML document and ensures that the root node is "history".
 * 
 * @param argumentsXml The arguments containing the file name.
 * @param argumentsXml.file {string}  The file name of the XML document to prepare.
 * @return A pair containing a boolean indicating success or failure, and a string with an error message if applicable.
 * 
 * @code
 * // Example usage: HistoryPrepareXml_s
 * gd::argument::arguments argumentsXml({{"file", "history.xml"}});
 * auto [success, message] = HistoryPrepareXml_s(argumentsXml);
 * if (!success) { std::cerr << "Error: " << message << std::endl; } 
 * else { std::cout << "History XML prepared successfully." << std::endl; }
 * @endcode
 */
std::pair<bool, std::string> HistoryPrepareXml_s(const gd::argument::arguments& argumentsXml)
{ 
   std::string stringFileName = argumentsXml["file"].as_string();
   bool bCreate = argumentsXml["create"].is_true();                                      // TODO: What if create is not set? Default to false?
   if( std::filesystem::exists(stringFileName) == false ) {  return { false, "History file does not exist: " + stringFileName }; }


   pugi::xml_document xmldocument;

   if( bCreate != true )
   {
      pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.c_str());
   }
   
   //if (!result_) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if the root node is "history"
   pugi::xml_node xmlnodeRoot = xmldocument.child("history");

   // If the root node is not "history", create it
   if( xmlnodeRoot.empty() == true ) { xmlnodeRoot = xmldocument.append_child("history"); }

   // Ensure the "entries" node exists under the root
   pugi::xml_node xmlnodeEntries = xmlnodeRoot.child("entries");

   // If the "entries" node does not exist, create it
   if( xmlnodeEntries.empty() == true ) { xmlnodeEntries = xmlnodeRoot.append_child("entries"); }

   // save the modified XML document back to the file
   xmldocument.save_file(stringFileName.c_str(), "  ", pugi::format_default );

   return { true, "" };
}

std::pair<bool, std::string> HistoryAppendEntry_s(const gd::argument::arguments& argumentsEntry)
{
   std::string stringFileName = argumentsEntry["file"].as_string();
                                                                               assert(!stringFileName.empty());
   std::string stringDate = argumentsEntry["date"].as_string();
   std::string stringCommand = argumentsEntry["command"].as_string();
   //std::string stringLine = argumentsEntry["line"].as_string();

   std::vector<gd::argument::arguments::argument> vectorLines = argumentsEntry.get_argument_all("line");

   //if( std::filesystem::exists(stringFileName) == false ) { return { false, "History file does not exist: " + stringFileName }; }


   pugi::xml_document xmldocument;
   pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.c_str());
   if( !result_ ) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if entries exist
   pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("entries");
   if( xmlnodeEntries.empty() ) { return { false, "No entries node found in XML file: " + stringFileName }; }

   // Create a new entry node
   pugi::xml_node xmlnodeEntry = xmlnodeEntries.append_child("entry");

   xmlnodeEntry.append_child("date").text().set(stringDate.c_str());
   xmlnodeEntry.append_child("command").text().set(stringCommand.c_str());
   
   std::string stringLine;

   for( auto it : vectorLines )
   {
      std::string stringTemp = it.as_string();
      stringLine += stringTemp + " ";
      //xmlnodeEntry.append_child("line").text().set(stringLine.c_str());
   }

   xmlnodeEntry.append_child("line").text().set(stringLine.c_str());

   //xmlnodeEntry.append_child("line").text().set(stringLine.c_str());

   // save the modified XML document back to the file
   xmldocument.save_file(stringFileName.c_str(), "  ", pugi::format_default);


   return { true, "" };
}

std::pair<bool, std::string> CreateTable_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable)
{
   std::string stringFileName = argumentsTable["file"].as_string();
   assert(!stringFileName.empty());
   //auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));

   pugi::xml_document xmldocument;
   pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.c_str());
   if( !result_ ) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if entries exist
   pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("entries");
   if( xmlnodeEntries.empty() ) { return { false, "No entries node found in XML file: " + stringFileName }; }

   // Iterate through each entry  
   for( auto entry : xmlnodeEntries.children("entry") )
   {
      std::string stringDate = entry.child("date").text().get();
      std::string stringCommand = entry.child("command").text().get();
      std::string stringLine = entry.child("line").text().get();
      // Add the entry to the table  
      tableHistory.row_add();
      tableHistory.cell_set(tableHistory.get_row_count() - 1, "date", stringDate);
      tableHistory.cell_set(tableHistory.get_row_count() - 1, "command", stringCommand);
      tableHistory.cell_set(tableHistory.get_row_count() - 1, "line", stringLine);
   }
   return { true, "" };
}

std::string CurrentTime_s()
{
   auto now_ = std::chrono::system_clock::now();
   auto nowTime_ = std::chrono::system_clock::to_time_t(now_);
   auto tm_ = *std::localtime(&nowTime_);

   std::ostringstream ostringstreamTime;
   ostringstreamTime << std::put_time(&tm_, "%Y-%m-%d %H:%M:%S");

   return ostringstreamTime.str();
}

NAMESPACE_CLI_END