/**
* @file CLIHistory.cpp
* @brief Implements history management for the cleaner console application.
*
* Fileformat for history file is XML, with root element `<history>` and multiple `<entry>` child elements.
* @verbatim
* <history>
*   <entry>
*     <pinned>...</pinned>
*     <saved>...</saved>
*     <history>...</history>
*   </entry>
* </history>
* @endverbatim
* 
*/

// @TAG #cli #history

#include <fstream>
#include <filesystem>
#include <format>
#include <chrono>
#include <functional>

#include "gd/gd_file.h"
#include "gd/math/gd_math_string.h"

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

// @TASK [project: history] 

NAMESPACE_CLI_BEGIN


// ## History operations

// ## Forward declarations

/// Prepare an empty XML file with root element
static std::pair<bool, std::string> PrepareEmptyXml_s(std::string_view stringHistoryFile); 

static std::pair<bool, std::string> PrepareXml_s(const gd::argument::arguments& argumentsXml);

// ## XML* methods work on XML nodes and XML documents

static std::pair<bool, std::string> XML_AppendEntry_s(const gd::argument::arguments& argumentsEntry, CDocument* pdocument);
bool XML_EntryExists_s( const pugi::xml_node* pxmlnodeEntries, std::string_view stringCommand );

static std::pair<bool, std::string> XML_ReadFile_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable, std::function<void(std::string_view)> callback_);
inline std::pair<bool, std::string>  XML_ReadFile_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable) {
   return XML_ReadFile_s( tableHistory, argumentsTable, [](std::string_view){ } ); 
}

static std::pair<bool, std::string> XML_Write_s(std::string_view stringHistoryFile, const gd::table::dto::table& tableHistory, std::string_view stringSection);

static std::unique_ptr<gd::table::dto::table> CreateTable_s(const gd::argument::arguments& argumentsTable);

static std::string FILE_GetHistoryFile_s( const gd::argument::arguments& arguments_ );

static std::string DATE_CurrentTime_s();

static std::filesystem::path GetHistoryPath_s();

static std::filesystem::path CurrentDirectory_s();

static std::string FolderGetHome_s();

std::pair<bool, std::string> History_g(const gd::cli::options* poptionsHistory, gd::cli::options* poptionsApplication, CDocument* pdocument)
{                                                                                                  assert( poptionsHistory != nullptr );
#ifndef NDEBUG
   auto stringOptions_d = gd::argument::debug::print( poptionsHistory->get_arguments() );
#endif // !NDEBUG

   std::pair<bool, std::string> result_;
   const gd::cli::options& options_ = *poptionsHistory;
   //const gd::cli::options& optionsApplication = *poptionsApplication;
   if( options_.exists("create") == true )
   {
      gd::argument::arguments argumentsCreate;
      argumentsCreate.append( options_.get_arguments(), { "create", "local"} );

      result_ = HistoryCreate_g(argumentsCreate, pdocument);
   }
   else if( options_.exists("delete") == true )
   {
      gd::argument::arguments argumentsDelete( {"delete", options_["delete"].as_string()} );
      result_ = HistoryDelete_g(argumentsDelete);
   }
   else if( options_.exists("print") == true )
   {
      gd::argument::arguments argumentsPrint;
      argumentsPrint.append( options_.get_arguments(), { "print", "local"} );
      result_ = HistoryPrint_g(argumentsPrint, pdocument);
   }
   else if( options_.exists("remove") == true )
   {
      gd::argument::arguments argumentsRemove;
      argumentsRemove.append( options_.get_arguments(), { "remove", "local"} );
      result_ = HistoryRemove_g(argumentsRemove, pdocument);
      auto stringHistoryFile = FILE_GetHistoryFile_s( argumentsRemove );
      if( result_.first == true && std::filesystem::exists(stringHistoryFile) == true )
      {
         const auto* ptable_ = pdocument->CACHE_Get("history", false );                             assert( ptable_ != nullptr );
         result_ = XML_Write_s(stringHistoryFile, *ptable_, "");               // Write an empty history table to the file
         if( result_.first == false ) { return result_; }
      }
   }
   else if( options_.exists("edit") == true )
   {
      gd::argument::arguments argumentsEdit;
      argumentsEdit.append( options_.get_arguments(), { "edit", "local"} );
      result_ = HistoryEdit_g( argumentsEdit );
   }
   else if( options_.exists("run") == true )
   {
      gd::argument::arguments argumentsRun;
      argumentsRun.append( options_.get_arguments(), { "run", "local"} );
      result_ = HistoryRun_g(argumentsRun, poptionsApplication, pdocument);
   }
   else if( options_.exists("index") == true )
   {
      gd::argument::arguments argumentsIndex;
      argumentsIndex.append(options_.get_arguments(), { "index", "set-alias" ,"local"});
      result_ = HistoryIndex_g(argumentsIndex, pdocument);
   }

   return result_;
}

/** ---------------------------------------------------------------------------
 * @brief Creates a history file in either the current directory or the user's local application data folder, preparing the necessary folder and initializing the file with empty XML if needed.
 * @param argumentsCreate The argument list used to determine the location for the history file (checks for the 'local' key).
 * @param pdocument Pointer to a CDocument object used for displaying messages to the user.
 * @return A pair where the first element is true if the history file was successfully created or already exists, and false otherwise; the second element is an error message if creation failed, or an empty string on success.
 */
std::pair<bool, std::string> HistoryCreate_g( const gd::argument::arguments& argumentsCreate, CDocument* pdocument)
{
   std::string_view stringHistoryFileName;
   std::filesystem::path pathDirectory;
   bool bCurrentDirectory = argumentsCreate.exists("local"); // If true, use current directory for history file, otherwise user local app data folder


   // ## Prepare folder for history file ......................................

   if( bCurrentDirectory == true )
   {
      stringHistoryFileName = ".cleaner-history.xml";                         // Local history file name, note that it is hidden file on Unix systems
      pathDirectory = CurrentDirectory_s();                                   // Get the local history path based on the current directory
   }
   else
   {
      stringHistoryFileName = "cleaner-history.xml";                          // Default history file name
      std::string stringPath = FolderGetHome_s();                               // Get the user home folder
      pathDirectory = std::filesystem::path(stringPath);                                     // Get the history path based on the operating system
   }
                                                                                                   LOG_DEBUG_RAW( "==> History file path: " + (pathDirectory / stringHistoryFileName).string() );

   std::string stringHistoryFile = ( pathDirectory / stringHistoryFileName ).string(); // Full path to the history file

   // ## Check if history file exists .........................................

   if( std::filesystem::exists(stringHistoryFile) == true )
   {                                                                           // No need to create the folder if it already exists, inform the user
      pdocument->MESSAGE_Display("History folder already exists: " + pathDirectory.string());
      return { true, "" };
   }


   // ## Prepare history file .................................................

   if( bCurrentDirectory == false )
   {
      // ### Create the cleaner user folder if it does not exist
      if( std::filesystem::exists(pathDirectory) == false )
      {  
         std::filesystem::create_directories(pathDirectory);                   // Create the cleaner folder if it does not exist
         pdocument->MESSAGE_Display("Cleaner folder created: " + pathDirectory.string());
      }
   }

   std::ofstream ofstreamFile(stringHistoryFile);
   if( ofstreamFile.is_open() == false ) {  return { false, "Failed to create history file in current directory: " + stringHistoryFile }; }
   else
   {
      ofstreamFile.close();
      pdocument->MESSAGE_Display("History file created: " + stringHistoryFile);
   }

   // ### Prepare the XML in the history file

   auto result_ = PrepareEmptyXml_s( stringHistoryFile );                     // Prepare the XML file if it does not exist
   if( result_.first == false ) { return result_; }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Appends a new command entry to the history section of an XML file, excluding the 'history' option from the saved command.
 *
 * The function first checks if the specified XML file exists, then loads it and appends a new `<entry>` node under the specified section or `<saved>` that is default.
 *
 * @param stringFile The path to the XML history file.
 * @param poptionsHistory Pointer to the options object representing the history; used to find the active command options.
 * @param stringSection The section name in the XML file (not directly used in the function body).
 * @return A pair where the first element is true if the entry was successfully appended, and false otherwise; the second element is an error message if the operation failed, or an empty string on success.
 */
std::pair<bool, std::string> HistoryAppend_g( std::string_view stringFile, gd::cli::options* poptionsHistory, std::string_view stringSection )
{                                                                                                  assert( std::filesystem::exists(stringFile) == true );
   return HistoryAppend_g(stringFile, poptionsHistory->name(),  & poptionsHistory->get_arguments(), stringSection);
}

std::pair<bool, std::string> HistoryAppend_g( std::string_view stringFile, std::string_view stringName, gd::argument::arguments* parguments, std::string_view stringSection )
{                                                                                                  assert( std::filesystem::exists(stringFile) == true );
   parguments->remove("history");                                             // remove history option if it exists, we do not want to save this in history
   auto stringCommand = gd::cli::options::to_string_s(*parguments);           // get the full command line for the active command
   
   std::string stringDateTime = DATE_CurrentTime_s(); // Generate date string

   // ## Open xml document and append to save entry ...........................

   if( std::filesystem::exists(stringFile) == true )
   {
      pugi::xml_document xmldocument;
      pugi::xml_parse_result result_ = xmldocument.load_file(stringFile.data());                  if( !result_ ) { return { false, std::format("Failed to load XML file: {}", stringFile) }; }

      // ### Append node to "save"
      pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("saved");
      if( xmlnodeEntries.empty() ) { return { false, std::format("No save node found in XML file: {}", stringFile) }; }
      pugi::xml_node xmlnodeEntry = xmlnodeEntries.append_child("entry");

      // ### Check if command already exists
      if( XML_EntryExists_s(&xmlnodeEntries, stringCommand) == true ) { return { false, "Command already exists in history, command: " + stringCommand }; }

      xmlnodeEntry.append_child("name").text().set( stringName );             // save command name
      xmlnodeEntry.append_child("line").text().set(stringCommand.c_str());    // save command line
      xmlnodeEntry.append_child("date").text().set( stringDateTime );         // save date

      papplication_g->PrintMessage("Appended command to history: " + stringCommand, gd::argument::arguments{});

      // ### Save the XML document back to the file
      xmldocument.save_file(stringFile.data(), "  ", pugi::format_default);
   }
   else
   {
      return { false, "History file does not exist: " + std::string(stringFile) };
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

/** ---------------------------------------------------------------------------
 * @brief Removes a specified row from the history table in the document cache, displaying a message upon successful removal.
 * @param argumentsRemove The arguments containing the row index to remove, accessed via the 'remove' key.
 * @param argumentsRemove.remove is expected to be a string representing a 1-based index of the row to remove.
 * @param pdocument Pointer to the document object containing the history table and message display functionality.
 * @return A pair where the first element is a boolean indicating success (true if the row was removed, false otherwise), and the second element is a string containing an error message if the operation failed, or an empty string on success.
 */
std::pair<bool, std::string> HistoryRemove_g(const gd::argument::arguments& argumentsRemove, CDocument* pdocument)
{
   std::string stringRemoveCommand = argumentsRemove["remove"].as_string();
   int64_t iRow = std::stoi(stringRemoveCommand) - 1;

   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache

   // ## Read the history file into the table

   auto result_ = XML_ReadFile_s(*ptable, argumentsRemove, [pdocument](std::string_view message_) {
      if( pdocument ) { pdocument->MESSAGE_Display(message_); }
   }); 

   if( result_.first == false ) { return result_; }

   if( iRow < 0 || iRow >= (int)ptable->size() ) { return { false, "Invalid row index: " + stringRemoveCommand }; } // Ensure the row index is valid

   std::string stringLine = ptable->cell_get_variant_view(iRow, "line").as_string();
   ptable->erase( iRow );                                                     // Remove the specified row from the table
   
   pdocument->MESSAGE_Display("Removed command line: " + stringLine);

   return { true, "" };
}


/// Open xml file with pugixml and remove all entries from node "saved" and then add nodes from table
std::pair<bool, std::string> XML_Write_s(std::string_view stringHistoryFile, const gd::table::dto::table& tableHistory, std::string_view stringSection)
{
   if( stringSection.empty() == true ) { stringSection = "saved"; }

   if( std::filesystem::exists(stringHistoryFile) == true )
   {
      pugi::xml_document xmldocument;
      pugi::xml_parse_result result_ = xmldocument.load_file(stringHistoryFile.data());            if( !result_ ) { return { false, std::format("Failed to load XML file: {}", stringHistoryFile) }; }
      // ### Get node "saved"
      pugi::xml_node xmlnodeEntries = xmldocument.child("history").child(stringSection);
      if( xmlnodeEntries.empty() ) { return { false, std::format("No save node found in XML file: {}", stringHistoryFile) }; }
      // ### Remove all existing entries
      xmlnodeEntries.remove_children();
      // ### Add entries from table
      for( size_t uRow = 0; uRow < tableHistory.size(); ++uRow )
      {
         auto stringName = tableHistory.cell_get_variant_view(uRow, "name").as_string_view();
         auto stringLine = tableHistory.cell_get_variant_view(uRow, "line").as_string_view();
         auto stringDate = tableHistory.cell_get_variant_view(uRow, "date").as_string_view();
         pugi::xml_node xmlnodeEntry = xmlnodeEntries.append_child("entry");
         xmlnodeEntry.append_child("name").text().set( stringName.data() );    // save command name
         xmlnodeEntry.append_child("line").text().set( stringLine.data() );    // save command line
         xmlnodeEntry.append_child("date").text().set( stringDate.data() );    // save date
      }
      // ### Save the XML document back to the file
      xmldocument.save_file(stringHistoryFile.data(), "  ", pugi::format_default);
   }
   else
   {
      return { false, "History file does not exist: " + std::string(stringHistoryFile) };
   }
   return { true, "" };
}




/*
std::pair<bool, std::string> XML_ClearFile_s(const gd::argument::arguments& argumentsClear)
{
   return { true, "" };
}
*/

std::unique_ptr<gd::table::dto::table> CreateTable_s(const gd::argument::arguments& argumentsTable)
{
   if( argumentsTable.exists("print") == true && argumentsTable["print"].as_bool() == true )
   {
      auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "date"}, {"rstring", 0, "name"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));
      return std::move(ptable);
   }
   else
   {
      auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"int32", 0, "index"}, {"rstring", 0, "date"}, {"rstring", 0, "name"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));
      return std::move(ptable);
   }
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves the full path to the history file based on the provided arguments, checking for its existence.
 * @param arguments_ The argument list used to determine the location for the history file (checks for the 'local' key).
 * @return The full path to the history file if it exists; otherwise, an empty string.
 */
std::string FILE_GetHistoryFile_s( const gd::argument::arguments& arguments_ )
{
   std::string stringHistoryFile;
   std::string_view stringHistoryFileName;
   std::filesystem::path pathDirectory;
   bool bCurrentDirectory = arguments_.exists("local"); // If true, use current directory for history file, otherwise user local app data folder


   // ## Prepare folder for history file ......................................

   if( bCurrentDirectory == true )
   {
      auto result_ = CApplication::HistoryFindLocal_s(pathDirectory);         // Find the local history folder based on the current directory
      if( result_.first == false ) { return ""; }                             // Failed to find the local history folder
      stringHistoryFile = pathDirectory.string();
   }
   else
   {
      stringHistoryFileName = "cleaner-history.xml";                          // Default history file name
      pathDirectory = GetHistoryPath_s();                                     // Get the history path based on the operating system
      stringHistoryFile = ( pathDirectory / stringHistoryFileName ).string(); // Full path to the history file
   }
                                                                                                   LOG_DEBUG_RAW( "==> History file path: " + stringHistoryFile );
   if( std::filesystem::exists(stringHistoryFile) == false )
   {
      return "";
   }

   return stringHistoryFile;
}

/** ---------------------------------------------------------------------------
 * @brief Prints the history table from a cached XML file associated with the given document and arguments.
 * @param argumentsPrint The arguments used for printing the history table.
 * @param pdocument Pointer to the document object containing the cache and history data.
 * @return A pair where the first element is true if the history was printed successfully, and false otherwise; the second element is an error message if unsuccessful, or an empty string if successful.
 */
std::pair<bool, std::string> HistoryPrint_g(const gd::argument::arguments& argumentsPrint, CDocument* pdocument)
{
   constexpr size_t uMaxLineLength = 70; // Maximum length of a line before it is cut
   std::array<std::byte, 64> array_; // array to hold the color codes for the output

   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache


   // ## Create the table from the XML file
   XML_ReadFile_s(*ptable, argumentsPrint, [pdocument](std::string_view message_) {
      if( pdocument ) { pdocument->MESSAGE_Display(message_); }
   });   

   // ## Move to table that is used for printing ..............................

   gd::table::dto::table tablePrint(0u, { {"int32", 0, "index"}, {"rstring", 0, "name"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{});

   for( size_t i = 0; i < ptable->size(); ++i )
   {
      auto variantName = ptable->cell_get_variant_view(i, "name");
      auto variantLine = ptable->cell_get_variant_view(i, "line");

      std::string stringLine = variantLine.as_string();

      stringLine = gd::math::string::format_text_width(stringLine, uMaxLineLength);// cut long lines

      tablePrint.row_add({ (int32_t)( i + 1 ), variantName, stringLine });
   }



   //std::string stringTable = gd::table::to_string(*ptable, gd::table::tag_io_cli{});
   std::string stringTable = gd::table::to_string(tablePrint, {{"divide", true}}, gd::table::tag_io_cli{}, gd::table::tag_text{});
   pdocument->MESSAGE_Display(stringTable, { array_, {{"color", "default"}}, gd::types::tag_view{} });

   return { true, "" };
}

std::pair<bool, std::string> HistoryGetRow_g(const gd::argument::arguments& argumentsRow, CDocument* pdocument)
{
   std::string stringFileName = argumentsRow["file"].as_string();
                                                                               assert(!stringFileName.empty());

   //auto ptable = CreateTable_s(argumentsRow);
   //CDocument document;
   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache
   XML_ReadFile_s(*ptable, argumentsRow); // Read the history file into the table

   std::string stringCommand = ptable->cell_get_variant_view(argumentsRow["index"].as_uint64(), "name").as_string();
   std::string stringLine = ptable->cell_get_variant_view(argumentsRow["index"].as_uint64(), "line").as_string();

   std::cout << stringCommand << " " << stringLine << "\n";
   
   return { true, "" };
}

std::pair<bool, std::string> HistorySave_g(const gd::argument::arguments& argumentsSave, CDocument* pdocument)
{

   auto ptable = pdocument->CACHE_Get("history"); // Ensure the history table is prepared in the cache
   std::string stringFileName = argumentsSave["file"].as_string();

   pugi::xml_document xmldocument;
   pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.c_str());
   if( !result_ ) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if entries exist
   pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("saved");
   if( xmlnodeEntries.empty() ) { return { false, "No entries node found in XML file: " + stringFileName }; }

   // Create a new entry node
   pugi::xml_node xmlnodeEntry = xmlnodeEntries.append_child("entry");

   auto uRowCount = ptable->size();

   std::string stringDate = ptable->cell_get_variant_view(uRowCount - 1, "date").as_string();
   std::string stringCommand = ptable->cell_get_variant_view(uRowCount - 1, "command").as_string();
   std::string stringLine = ptable->cell_get_variant_view(uRowCount - 1, "line").as_string();

   xmlnodeEntry.append_child("date").text().set(stringDate.c_str());
   xmlnodeEntry.append_child("command").text().set(stringCommand.c_str());
   xmlnodeEntry.append_child("line").text().set(stringLine.c_str());

   xmldocument.save_file(stringFileName.c_str(), "  ", pugi::format_default);

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Executes a command from the history table based on the specified row index.
 * @param argumentsRun The set of arguments containing the row index and other parameters.
 * @param poptionsApplication Pointer to the application's CLI options, which will be configured and used for command execution.
 * @param pdocument Pointer to the document object, used to access the cached history table.
 * @return A pair where the first element is a boolean indicating success or failure, and the second element is a string containing an error message if the operation failed.
 */
std::pair<bool, std::string> HistoryRun_g(const gd::argument::arguments& argumentsRun, gd::cli::options* poptionsApplication, CDocument* pdocument)
{
   // ## Read the history file into the table

   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache
   auto result_ = XML_ReadFile_s(*ptable, argumentsRun, [pdocument](std::string_view message) {
      if( pdocument ) { pdocument->MESSAGE_Display(message); }
   }); 

   if( result_.first == false ) { return result_; }                           

   std::string stringCommand;
   std::string stringRun = argumentsRun["run"].as_string();                                        LOG_DEBUG_RAW( "==> Index/name to run: " + stringRun );
   int64_t iRow = std::stoi(stringRun) - 1;

   if( iRow < 0 || iRow >= (int)ptable->size() ) { return { false, std::format( "Invalid row index: {} max is: {} (did you forget -local)", stringRun, ptable->size() ) }; } // Ensure the row index is valid, note that is 1-based index

   // ## Get the command from the specified row and execute it

   std::string stringName = ptable->cell_get_variant_view(iRow, "name").as_string();
   std::string stringLine = ptable->cell_get_variant_view(iRow, "line").as_string();

   stringCommand = stringName + " " + stringLine; // Construct the full command line

   if( stringCommand.empty() == false )
   {                                                                                               LOG_DEBUG_RAW( "==> Running history command: " + stringCommand );
      poptionsApplication->clear();
      poptionsApplication->set_first(0);

      result_ = poptionsApplication->parse_terminal(stringCommand);           // Parse the command line from the history entry
      if( result_.first == false ) { return result_; }

      auto result_ = CApplication::CliPrompt_s(poptionsApplication);          // If prompt values exist, ask the user for values
      if( result_.first == false ) { return result_; }

      result_ = papplication_g->Initialize(*poptionsApplication);             // Initialize the application with parsed options
      if( result_.first == false ) { return result_; }
   }

   return { true, "" };
}

/* ---------------------------------------------------------------------------
 * @brief Edit a command in the history
 */
std::pair<bool, std::string> HistoryEdit_g( const gd::argument::arguments& argumentsEdit )
{
   std::string stringHistoryFile = FILE_GetHistoryFile_s( argumentsEdit );
   if( stringHistoryFile.empty() == true ) return { false, "Failed to get history file path." };   

   return SHARED_OpenFile_g(stringHistoryFile);
}

std::pair<bool, std::string> HistoryIndex_g(const gd::argument::arguments& argumentsIndex, CDocument* pdocument)
{

   // ## Read the history file into the table

   auto ptable = pdocument->CACHE_Get("history"); // Get the history table from the cache
   auto result_ = XML_ReadFile_s(*ptable, argumentsIndex, [pdocument](std::string_view message) {
      if( pdocument ) { pdocument->MESSAGE_Display(message); }
      });

   if( result_.first == false ) { return result_; }

   std::string stringIndex = argumentsIndex["index"].as_string();                                        LOG_DEBUG_RAW("==> Index/name to run: " + stringRun);
   int64_t iRow = std::stoi(stringIndex) - 1;

   if( iRow < 0 || iRow >= (int)ptable->size() ) { return { false, std::format("Invalid row index: {} max is: {} (did you forget -local)", stringIndex, ptable->size()) }; } // Ensure the row index is valid, note that is 1-based index


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prepare an empty XML file for history
 * 
 * This function creates a new XML document with a root node "history" and a child node "entries".
 * It saves the XML document to the specified file.
 * 
 * @param stringHistoryFile The file name of the XML document to prepare.
 * @return A pair containing a boolean indicating success or failure, and a string with an error message if applicable.
 * 
 * @code
 * // Example usage: PrepareEmptyXml_s
 * auto [bOk, stringError] = PrepareEmptyXml_s("cleaner-history.xml");
 * if (!bOk) { std::cerr << "Error: " << stringError << std::endl; } 
 * else { std::cout << "Empty history XML prepared successfully." << std::endl; }
 * @endcode
 */
std::pair<bool, std::string> PrepareEmptyXml_s( std::string_view stringHistoryFile )
{                                                                                                  assert( std::filesystem::exists(stringHistoryFile) == true ); // Ensure the file does not already exist
   pugi::xml_document xmldocument; // Create a new XML document

   // ## Prepare the XML structure ............................................

   pugi::xml_node xmlnodeRoot = xmldocument.append_child("history");           // "history" = root

   xmlnodeRoot.append_child("named");                                          // "named" = child and used to name important entries
   xmlnodeRoot.append_child("pinned");                                         // "pinned" = child and used to pin important entries
   xmlnodeRoot.append_child("saved");                                          // "saved" = child and used to save important entries
   xmlnodeRoot.append_child("recent");                                         // "recent" = child and used for recent entries

   // Save the XML document to the specified file
   bool bSucceeded = xmldocument.save_file(stringHistoryFile.data(), "  ", pugi::format_default);
   if( bSucceeded == false ) { return { false, "Failed to save XML file: " + std::string(stringHistoryFile) }; }

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

std::pair<bool, std::string> XML_AppendEntry_s(const gd::argument::arguments& argumentsEntry, CDocument* pdocument)
{
   std::string stringFileName = argumentsEntry["file"].as_string();
                                                                               assert(!stringFileName.empty());
   std::string stringDate = argumentsEntry["date"].as_string();
   std::string stringName = argumentsEntry["name"].as_string();
   //std::string stringLine = argumentsEntry["line"].as_string();

   std::vector<gd::argument::arguments::argument> vectorLines = argumentsEntry.get_argument_all("line");

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
   ptable->cell_set(uRow, "name", stringName);
   ptable->cell_set(uRow, "line", stringLine);

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Checks if an XML node contains an entry with a specified command.
 * @param pxmlnodeEntries Pointer to the XML node containing entry children to search.
 * @param stringCommand The command string to look for among the entries.
 * @return True if an entry with the specified command exists; otherwise, false.
 */
bool XML_EntryExists_s( const pugi::xml_node* pxmlnodeEntries, std::string_view stringCommand )
{
   for( auto entry : pxmlnodeEntries->children("entry") )
   {
      std::string stringExistingCommand = entry.child("command").text().get();
      if( stringExistingCommand == stringCommand ) { return true; } // Entry with the same command already exists
   }
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief Reads history entries from an XML file and populates a table with the data.
 * 
 * This function loads an XML file containing history entries, parses the entries, and populates a provided table with the data.
 * Each entry in the XML file is expected to have child nodes for "date", "name", and "line".
 * 
 * @note The function checks for the existence of the history file in both the local directory and the user's home directory.
 * 
 * @param tableHistory Reference to a table object where the history entries will be stored.
 * @param argumentsTable The arguments containing options for reading the history, such as the file name.
 * @param callback_ Optional callback function to receive status messages during the reading process.
 * @return A pair containing a boolean indicating success or failure, and a string with an error message if applicable.
 */
std::pair<bool, std::string> XML_ReadFile_s(gd::table::dto::table& tableHistory, const gd::argument::arguments& argumentsTable, std::function<void(std::string_view)> callback_)
{
   std::string stringFileName = FILE_GetHistoryFile_s(argumentsTable);

   if( stringFileName.empty() == true ) { return { false, "No history file found." }; } // No history file found

   if( callback_ ) { callback_("Reading history file: " + stringFileName); }
   //auto ptable = std::make_unique<gd::table::dto::table>(gd::table::dto::table(0u, { {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));

   pugi::xml_document xmldocument;
   pugi::xml_parse_result xmlparseresult_ = xmldocument.load_file(stringFileName.c_str());
   if( !xmlparseresult_ ) { return { false, "Failed to load XML file: " + stringFileName }; }

   // Check if entries exist
   pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("saved");
   if( xmlnodeEntries.empty() ) { return { false, "No entries node found in XML file: " + stringFileName }; }
   
   int iRowCount = 0;

   // Iterate through each entry  
   for( auto entry : xmlnodeEntries.children("entry") )
   {
      std::string stringDate = entry.child("date").text().get();
      std::string stringName = entry.child("name").text().get();
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
      tableHistory.cell_set(uRow, "name", stringName);
      tableHistory.cell_set(uRow, "line", stringLine);
   }
   return { true, "" };
}

/// ---------------------------------------------------------------------------
/// Generate time string in format "YYYY-MM-DD HH:MM:SS"
std::string DATE_CurrentTime_s()
{
   auto now_ = std::chrono::system_clock::now();
   std::time_t time_ = std::chrono::system_clock::to_time_t(now_);
#if defined(_WIN32)
   std::tm tm_;
   localtime_s(&tm_, &time_);
#else
   std::tm tm_;
   localtime_r(&time_, &tm_);
#endif
   char buffer[20];
   std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm_);
   return std::string(buffer);
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
      piDir = getenv("XDG_DATA_HOME");
      if(piDir != nullptr) 
      {
         piDir = getenv("USERPROFILE"); // Windows compatibility
      }
   }

   if( piDir != nullptr )
   {
      pathDirectory = std::filesystem::path(piDir) / ".local" / "share" / "cleaner";
   }
#endif   
   return pathDirectory;
}

std::filesystem::path CurrentDirectory_s()
{
   std::filesystem::path pathDirectory = std::filesystem::current_path();

   return pathDirectory;
}

std::string FolderGetHome_s()
{
   std::string stringHome;
   CApplication::FolderGetHome_s(stringHome);

   return stringHome;
}

NAMESPACE_CLI_END