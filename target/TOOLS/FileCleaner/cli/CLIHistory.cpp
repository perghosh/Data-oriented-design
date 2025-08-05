/**
* @file CLIHistory.cpp
* @brief Implements history management for the cleaner console application.
*
* @section history_file_format History File Format
*
* The history information for the cleaner console application is stored in either XML or JSON format.
* Each entry records an operation performed by the cleaner, including a timestamp, operation type, and optional details.
*
*
* @section history_file_fields Fields
* - @b timestamp: ISO 8601 formatted date and time of the operation.
* - @b operation: The type of operation performed (e.g., "delete", "scan").
* - @b details: (Optional) Additional information about the operation.
*/

// @TAG #cli #history

#include <fstream>
#include <filesystem>

#include "gd/gd_file.h"

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#include <pugixml/pugixml.hpp>

#include "gd/expression/gd_expression_runtime.h"


#include "../Application.h"
#include "../Document.h"

#include "CLI_Shared.h"

#include "CLIHistory.h"




NAMESPACE_CLI_BEGIN


// ## History operations

// ## Forward declarations

static std::pair<bool, std::string> PrepareXml_s(const gd::argument::arguments& argumentsXml);

static std::pair<bool, std::string> AppendEntry_s(const gd::argument::arguments& argumentsEntry, CDocument* pdocument);

static std::pair<bool, std::string> ReadFile_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable);

static std::unique_ptr<gd::table::dto::table> CreateTable_s(const gd::argument::arguments& argumentsTable);

static std::string FilePath();

static std::string CurrentTime_s();

static std::filesystem::path GetHistoryPath_s();

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory, CDocument* pdocument)
{
   const gd::cli::options& options_ = *poptionsHistory;
   if( options_.exists("create") == true )
   {
      gd::argument::arguments argumentsCreate( {"create", options_["create"].as_string()} );
      auto result_ = HistoryCreate_g(argumentsCreate, pdocument);
   }
   else if( options_.exists("delete") == true )
   {
      gd::argument::arguments argumentsDelete( {"delete", options_["delete"].as_string()} );
      auto result_ = HistoryDelete_g(argumentsDelete);
   }
   else if( options_.exists("print") == true )
   {
      gd::argument::arguments argumentsPrint({ "print", options_["print"].as_string() });
      auto result_ = HistoryPrint_g(argumentsPrint, pdocument);
   }
   else if( options_.exists("remove") == true )
   {
      gd::argument::arguments argumentsRemove({ "remove", options_["remove"].as_string() });
      auto result_ = HistoryRemove_g(argumentsRemove);
   }
   else if( options_.exists("edit") == true )
   {
      gd::argument::arguments argumentsEdit({ "edit", options_["edit"].as_string() });
      auto result_ = HistoryEdit_g();
   }

   return { true, "" };
}


/**
 * @brief Set up folders for cleaner and creates history file if not exists
 */
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate, CDocument* pdocument)
{
   //auto result_ = papplication_g->CreateDirectory();                                               if( result_.first == false ) { return result_; }

   std::string stringName = "history.xml";
   std::filesystem::path pathDirectory = GetHistoryPath_s();

/*#ifdef _WIN32
   //std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   // Windows: C:\Users\<username>\AppData\Local\cleaner\history.xml
   char* piAppData = nullptr;
   size_t uLength = 0;
   if( _dupenv_s(&piAppData, &uLength, "LOCALAPPDATA") == 0 && piAppData != nullptr )
   {
      //stringPath = std::string(piAppData) + "\\cleaner";
      pathDirectory = std::filesystem::path(piAppData) / "cleaner";
      free(piAppData);
   }
   else { return { false, "Failed to get LOCALAPPDATA environment variable" }; }
#else
   // Linux: ~/.local/share/cleaner/history.xml
   const char* piDir = getenv("HOME");
   if( piDir == nullptr )
   {
      struct passwd* pw = getpwuid(getuid());
      if( pw == nullptr ) {
         return { false, "Failed to get home directory" };
      }
      piDir = pw->pw_dir;
      pathDirectory = std::filesystem::path(piDir) / ".local" / "share" / "cleaner";
   }
#endif*/

   



   std::string stringFilePath = (pathDirectory / stringName).string();
   gd::argument::arguments argumentsFile({ {"file", stringFilePath} , {"create", true}, {"command", "command1"}, {"line", "line1"}, {"line", "line2"}, {"date", CurrentTime_s()}, { "print", false } , { "index", 0 } } );     // TODO: This is just temporary, we need to edit this later
    // To create a string representing the full path to "history.xml" in pathDirectory:

   if( std::filesystem::exists(pathDirectory) == false )
   {
      std::filesystem::create_directory(pathDirectory); 
      std::ofstream ofstreamFile(pathDirectory / stringName);
      ofstreamFile.close();
      PrepareXml_s(argumentsFile); // Prepare the XML file if it does not exist

      AppendEntry_s(argumentsFile, pdocument); // TODO: This is just temporary, we need to remove this later

      HistoryPrint_g(argumentsFile, pdocument); // Print the history file to console, this is just for debug purposes

      HistoryGetRow_g(argumentsFile); // Get the first row of the history table, this is just for debug purposes

      //HistoryDelete_g(argumentsCreate); // TODO: This is just temporary, we need to remove this later
   }
   //else
   //{
   //   HistoryDelete_g(argumentsCreate); // TODO: This is just temporary, we need to remove this later
   //}

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

std::pair<bool, std::string> HistoryRemove_g(const gd::argument::arguments& argumentsRemove)
{
   std::string stringRemoveCommand = argumentsRemove["remove"].as_string();

   /*std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   if( std::filesystem::exists(pathCurrentDirectory) == true )
   {
      //std::filesystem::remove_all(pathCurrentDirectory); // remove the history folder
   }*/

   int iIndex = std::stoi(stringRemoveCommand) - 1;

   std::string stringFileName = FilePath();
                                                                               assert(!stringFileName.empty());

   pugi::xml_document xmldocument;
   pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.c_str());
   if( !result_ ) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if entries exist
   pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("entries");
   if( xmlnodeEntries.empty() ) { return { false, "No entries node found in XML file: " + stringFileName }; }

   int iRowCount = 0;

   // Iterate through each entry  
   for( auto entry : xmlnodeEntries.children("entry") )
   {
      std::ostringstream oss;
      entry.print(oss, "  ", pugi::format_default);
      std::string entryXml = oss.str();
      std::cout << entryXml << "\n";
      if( iRowCount == iIndex )
      {
         xmlnodeEntries.remove_child(entry);
         break;
      }
      ++iRowCount;
   }

   /*for( auto it = xmlnodeEntries.begin(); it != xmlnodeEntries.end(); ++it )
   {
      if( std::string(it->name()) == "entry" )
      {
         if(iRowCount == iIndex)
         {
            xmlnodeEntries.remove_child(*it); // Remove the entry with the specified index
            break;
         }
         ++iRowCount;
      }
   }*/

   xmldocument.save_file(stringFileName.c_str(), "  ", pugi::format_default );

   std::cout << stringRemoveCommand << "\n";

   return {true, ""};
}

std::unique_ptr<gd::table::dto::table> CreateTable_s(const gd::argument::arguments& argumentsTable)
{
   if( argumentsTable.exists("print") == true && argumentsTable["print"].as_bool() == true )
   {
      auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));
      return std::move(ptable);
   }
   else
   {
      auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"int32", 0, "index"}, {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));
      return std::move(ptable);
   }
}

std::string FilePath()
{
   std::string stringName = "history.xml";
   std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   std::string stringFilePath = (pathCurrentDirectory / stringName).string();

   return stringFilePath;
}

std::pair<bool, std::string> HistoryPrint_g(const gd::argument::arguments& argumentsPrint, CDocument* pdocument)
{
   //std::string stringFileName = argumentsPrint["file"].as_string();

   std::filesystem::path pathDirectory = GetHistoryPath_s();
   std::string stringFileName = ( pathDirectory / "history.xml" ).string();
                                                                               assert(!stringFileName.empty());

   //auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));
   //auto ptable = CreateTable_s(argumentsPrint); // Create a table to hold the history data                                                                               

   //std::unique_ptr<gd::table::dto::table>* ptable;
   //CDocument document;

   //pdocument.CACHE_Prepare("history"); // Prepare the cache for history table

   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache


   ReadFile_s(*ptable, argumentsPrint); // Create the table from the XML file

   std::string stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   std::cout << "\n" << stringTable << "\n";


   return { true, "" };
}

std::pair<bool, std::string> HistoryGetRow_g(const gd::argument::arguments& argumentsRow)
{
   std::string stringFileName = argumentsRow["file"].as_string();
                                                                               assert(!stringFileName.empty());

   //auto ptable = CreateTable_s(argumentsRow);
   CDocument document;
   auto ptable = document.CACHE_Get("history"); // Get the history table from the cache
   ReadFile_s(*ptable, argumentsRow); // Read the history file into the table

   std::string stringCommand = ptable->cell_get_variant_view(argumentsRow["index"].as_uint64(), "command").as_string();
   std::string stringLine = ptable->cell_get_variant_view(argumentsRow["index"].as_uint64(), "line").as_string();

   std::cout << stringCommand << " " << stringLine << "\n";
   
   return { true, "" };
}

std::pair<bool, std::string> HistorySave_g(const gd::argument::arguments& argumentsSave, CDocument* pdocument)
{
   return std::pair<bool, std::string>();
}

std::pair<bool, std::string> HistoryEdit_g()
{

   std::string stringHomePath = papplication_g->PROPERTY_Get("folder-home").as_string();

   if( stringHomePath.empty() == true ) return { false, "Home path is not set in the application properties." };

   gd::file::path pathHistoryFile(stringHomePath + "/history.xml");

   if( std::filesystem::exists(pathHistoryFile) == false ) return { false, "History file does not exist: " + pathHistoryFile.string() };

   return SHARED_OpenFile_g(pathHistoryFile);
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
 * // Example usage: PrepareXml_s
 * gd::argument::arguments argumentsXml({{"file", "history.xml"}});
 * auto [success, message] = PrepareXml_s(argumentsXml);
 * if (!success) { std::cerr << "Error: " << message << std::endl; } 
 * else { std::cout << "History XML prepared successfully." << std::endl; }
 * @endcode
 */
std::pair<bool, std::string> PrepareXml_s(const gd::argument::arguments& argumentsXml)
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

std::pair<bool, std::string> AppendEntry_s(const gd::argument::arguments& argumentsEntry, CDocument* pdocument)
{
   std::string stringFileName = argumentsEntry["file"].as_string();
                                                                               assert(!stringFileName.empty());
   std::string stringDate = argumentsEntry["date"].as_string();
   std::string stringCommand = argumentsEntry["command"].as_string();
   //std::string stringLine = argumentsEntry["line"].as_string();

   std::vector<gd::argument::arguments::argument> vectorLines = argumentsEntry.get_argument_all("line");

   //if( std::filesystem::exists(stringFileName) == false ) { return { false, "History file does not exist: " + stringFileName }; }

   /*
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
   */

   std::string stringLine;

   for( auto it : vectorLines )
   {
      std::string stringTemp = it.as_string();
      stringLine += stringTemp + " ";
      //xmlnodeEntry.append_child("line").text().set(stringLine.c_str());
   }

   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache
   auto uRow = ptable->row_add_one(); // Add a new row to the table
   ptable->cell_set(uRow, "date", stringDate);
   ptable->cell_set(uRow, "command", stringCommand);
   ptable->cell_set(uRow, "line", stringLine);


   /*
   xmlnodeEntry.append_child("line").text().set(stringLine.c_str());

   //xmlnodeEntry.append_child("line").text().set(stringLine.c_str());

   // save the modified XML document back to the file
   xmldocument.save_file(stringFileName.c_str(), "  ", pugi::format_default);
   */

   return { true, "" };
}

std::pair<bool, std::string> ReadFile_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable)
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
   
   int iRowCount = 0;

   // Iterate through each entry  
   for( auto entry : xmlnodeEntries.children("entry") )
   {
      std::string stringDate = entry.child("date").text().get();
      std::string stringCommand = entry.child("command").text().get();
      std::string stringLine = entry.child("line").text().get();
      // Add the entry to the table  
      auto uRow = tableHistory.row_add_one();

      if( tableHistory.column_exists("index") == true )
      {
         //std::string stringRowIndex = std::to_string(uRowCount);
         tableHistory.cell_set(uRow, "index", iRowCount); // Set the index column if it exists
         iRowCount++;
      }

      tableHistory.cell_set(uRow, "date", stringDate);
      tableHistory.cell_set(uRow, "command", stringCommand);
      tableHistory.cell_set(uRow, "line", stringLine);
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

std::filesystem::path GetHistoryPath_s()
{

   std::filesystem::path pathDirectory;

#ifdef _WIN32
   //std::filesystem::path pathCurrentDirectory = std::filesystem::current_path() / ".cleaner";

   // Windows: C:\Users\<username>\AppData\Local\cleaner\history.xml
   char* piAppData = nullptr;
   size_t uLength = 0;
   if( _dupenv_s(&piAppData, &uLength, "LOCALAPPDATA") == 0 && piAppData != nullptr )
   {
      //stringPath = std::string(piAppData) + "\\cleaner";
      pathDirectory = std::filesystem::path(piAppData) / "cleaner";
      free(piAppData);
   }
   //else { return { false, "Failed to get LOCALAPPDATA environment variable" }; }
#else
   // Linux: ~/.local/share/cleaner/history.xml
   const char* piDir = getenv("HOME");
   if( piDir == nullptr )
   {
      struct passwd* pw = getpwuid(getuid());
      if( pw == nullptr ) {
         //return { false, "Failed to get home directory" };
      }
      piDir = pw->pw_dir;
      pathDirectory = std::filesystem::path(piDir) / ".local" / "share" / "cleaner";
   }
#endif

   return pathDirectory;
}

NAMESPACE_CLI_END