/**
 * @file Application.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0Initialize.Application` - Initialize the application from command line 
 * - `0TAG0RUN.Application` - run commands, there are a number of commands that can be run
 * - `0TAG0Database.Application` - database operations
 * - `0TAG0OPTIONS.Application` - prepare command line options
 * 
 */

#include <filesystem>
#include <format>

#include "pugixml/pugixml.hpp"

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"
#include "gd/gd_file.h"


#ifdef _WIN32
#  include <windows.h>
#  include <shlobj_core.h> // For SHGetFolderPathW
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  include <pwd.h>
#  include <cstdlib>
#endif


#ifdef _WIN32
#  include "win/VS_Command.h"
#endif

#include "cli/CLICount.h"
#include "cli/CLIDir.h"
#include "cli/CLIHistory.h"

#include "Command.h"

#include "Application.h"


/// Global pointer to application object
CApplication* papplication_g = nullptr;



/** ---------------------------------------------------------------------------
 * @brief Common construction logic for copy constructor and copy assignment operator.
 * 
 * @param o The source object to copy from.
 */
void CApplication::common_construct(const CApplication& o)
{
   // Copy the document vector
   m_vectorDocument.clear();
   for( const auto& document_ : o.m_vectorDocument )
   {
      m_vectorDocument.push_back(std::make_unique<CDocument>(*document_));
   }
}

/** ---------------------------------------------------------------------------
 * @brief Common construction logic for move constructor and move assignment operator.
 * 
 * @param o The source object to move from.
 */
void CApplication::common_construct(CApplication&& o) noexcept 
{
   // Move the document vector
   m_vectorDocument = std::move(o.m_vectorDocument);
}

/** ---------------------------------------------------------------------------
 * @brief Returns the string representation of the UI type.
 *
 * This method converts the enum value of the UI type to its corresponding string representation.
 *
 * @return std::string The string representation of the UI type.
 */
std::string CApplication::GetUITypeAsString() const
{
   switch( m_eUIType )
   {
   case    eUITypeUnknown: return "unknown";
   case    eUITypeConsole: return "console";
   case    eUITypeWeb    : return "web";
   case    eUITypeWIMP   : return "wimp";
   case    eUITypeVSCode : return "vscode";
   case    eUITypeVS     : return "vs";
   case    eUITypeSublime: return "sublime";
   }
   return  "unknown";
}


/** ---------------------------------------------------------------------------
 * @brief Prepares the application by setting up command-line options.
 *
 * Main in application is similar to main in application, but it is used to prepare
 * based on command line arguments. Here tha actual work is done.
 * 
 * @param iArgumentCount The number of command-line arguments.
 * @param ppbszArgument The command-line arguments.
 * @param process_ A function to process the command-line arguments.
 */
std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{
   PrepareLogging_s();

   auto result_ = Initialize();
   if( result_.first == false ) { return result_; }


   if( iArgumentCount > 1 )
   {
      std::string stringArgument = gd::cli::options::to_string_s(iArgumentCount, ppbszArgument, 1);
      PROPERTY_Add("arguments", stringArgument);                                                   LOG_INFORMATION_RAW("== Arguments: " & stringArgument);

      gd::cli::options optionsApplication;
      CApplication::Prepare_s(optionsApplication);                             // prepare command-line options

      // ## Parse the command-line arguments
      auto [bOk, stringError] = optionsApplication.parse(iArgumentCount, ppbszArgument);
      if( bOk == false ) 
      { 
         std::string stringHelp;
         const gd::cli::options* poptionsActive = optionsApplication.find_active();
         if( poptionsActive != nullptr )
         {
            poptionsActive->print_documentation(stringHelp, gd::cli::options::tag_documentation_dense{});// print help for active command

            stringError += "\n\n" + stringHelp;
         }
         
         return { false, stringError }; 
      }

      // ## Process the command-line arguments
      std::tie(bOk, stringError) = Initialize(optionsApplication);
      if( bOk == false ) { return { false, stringError }; }
   }

   return { true, "" };
}


std::pair<bool, std::string> CApplication::Initialize()
{
#ifdef _WIN32
   // Initialize Windows-specific functionality, such as COM
   auto result_ = PrepareWindows_s();
   if( result_.first == false ) return result_;
#endif

   // Perform initialization tasks here
   // For example, you might want to initialize documents or other resources

   // Example: Initialize documents
   // DOCUMENT_Add("example_document");

   // If initialization is successful
   return {true, ""};

   // If initialization fails, return an appropriate error message
   // return {false, "Initialization failed: <error details>"};
}

std::pair<bool, std::string> CApplication::Exit()
{
   // Perform cleanup tasks here
   // For example, you might want to clear documents or release resources

   // Example: Clear documents
   DOCUMENT_Clear();
   
   std::string stringArguments = PROPERTY_Get("arguments").as_string();

   HistorySaveArguments_s(stringArguments);

#ifdef _WIN32
   ExitWindows_s();
#endif


   // If cleanup is successful
   return {true, ""};

   // If cleanup fails, return an appropriate error message
   // return {false, "Exit failed: <error details>"};
}

// 0TAG0Initialize.Application

/** --------------------------------------------------------------------------- @TAG #option #print
 * @brief Initializes the application based on the provided command-line options.
 *
 * This method processes the command-line options and performs initialization tasks
 * based on the active subcommand. It supports various commands such as `count`, `db`,
 * `history`, `list`, `help`, and `version`.
 *
 * ### Steps:
 * 1. Retrieve the active subcommand from the provided options.
 * 2. Perform initialization tasks based on the subcommand:
 *    - **count**: Harvest files, apply filters, count rows, and optionally save or print results.
 *    - **db**: Open or create a database and update its schema.
 *    - **history**: Print the command history.
 *    - **list**: Harvest files, apply patterns, and list matching rows.
 *    - **help**: Display help information for the application.
 *    - **version**: Display the application version.
 * 3. Handle errors and return appropriate success or failure messages.
 *
 * @param optionsApplication The parsed command-line options.
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Initialize( gd::cli::options& optionsApplication )
{
   const gd::cli::options* poptionsActive = optionsApplication.find_active();
   if( poptionsActive == nullptr ) { return { false, "No active options found" }; }

   if( poptionsActive->exists("help") == true ) 
   {
      std::string stringDocumentation;
      poptionsActive->print_documentation( stringDocumentation, gd::cli::options::tag_documentation_dense{});
      PrintMessage( stringDocumentation, gd::argument::arguments() );
      return { true, "" };
   }

   /// ## prepare command

#ifndef NDEBUG
   auto stringName_d = poptionsActive->name();
#endif // !NDEBUG

   // ## set editor
   std::string stringEditor = ( *poptionsActive )["editor"].as_string();
   PROPERTY_Set("editor", stringEditor);

   // ## set command name
   std::string stringCommandName = poptionsActive->name();
   PROPERTY_Set("command", stringCommandName);                                                     LOG_INFORMATION_RAW("== Command: " & stringCommandName);

   // ## check for sql statements
   if( poptionsActive->exists("statements") == true )
   {
      std::string stringFileName = ( *poptionsActive )["statements"].as_string();
      auto result_ = STATEMENTS_Load(stringFileName);
      if( result_.first == false ) return result_;
   }

   auto* pdocument = DOCUMENT_Add(stringCommandName);
   if( pdocument == nullptr ) { return { false, "Failed to add document" }; }

   if( stringCommandName == "count" )                                          // command = "count"
   {
      // Add a document for the "count" command
      auto* pdocument = DOCUMENT_Get("count");
      if( pdocument == nullptr ) 
      {
         pdocument = DOCUMENT_Add("count");
         if( pdocument == nullptr ) { return { false, "Failed to add document" }; }
      }

      return CLI::Count_g(poptionsActive, pdocument);                         // count lines in file or directory

      //return RUN_Count( poptionsActive );
   }
   else if( stringCommandName == "db" )
   {
      std::string stringDatabaseFile = (*poptionsActive)["file"].as_string();
      if( stringCommandName.empty() == false )
      {
         auto result_ = DATABASE_Open({ {"file", stringDatabaseFile} });       // open or create database (create is default, not creating set "create" to false)
         if( result_.first == false ) return result_;
         result_ = DATABASE_Update();                                          // update database to match latest design
         if( result_.first == false ) return result_;
      }
   }
   else if( stringCommandName == "dir" )
   {
      auto result_ = CLI::Dir_g(poptionsActive);
      if( result_.first == false ) return result_;
   }
   else if( stringCommandName == "history" )
   {
      auto result_ = CLI::History_g( poptionsActive );
      //HistoryPrint_s();
   }
   else if( stringCommandName == "list" )
   {
      std::string stringSource = (*poptionsActive)["source"].as_string();                          
      PathPrepare_s(stringSource);                                   // if source is empty then set it to current path

      int iRecursive = ( *poptionsActive )["recursive"].as_int();
      if( iRecursive == 0 && poptionsActive->exists("R") == true ) iRecursive = 16;// set to 16 if D is set, find all files
      gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });
      std::string stringFilter = ( *poptionsActive )["filter"].as_string();

      

      auto result_ = pdocument->FILE_Harvest(argumentsPath, stringFilter);     // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if( result_.first == false ) return result_;

      std::string stringPattern = ( *poptionsActive )["pattern"].as_string();                      //LOG_INFORMATION_RAW("== --pattern: " & stringPattern);
      auto vectorPattern = Split_s(stringPattern);                             // split pattern string into vector

      uint64_t uMax = ( *poptionsActive )["max"].as_uint64();                  // max number of lines to be printed
      if( uMax == 0 ) uMax = 512;                                              // default to 512 lines

      gd::argument::shared::arguments argumentsList({  {"max", uMax} });
      std::string stringSegment = ( *poptionsActive )["segment"].as_string();  // type of segment to search in, code, comment or string, maybe all
      if( stringSegment.empty() == false ) argumentsList.set("state", stringSegment.c_str());

      result_ = pdocument->FILE_UpdatePatternList( vectorPattern, argumentsList );      // count rows in harvested files
      if( result_.first == false ) return result_;

      auto* ptableLineList = pdocument->CACHE_Get("file-linelist");

      auto tableResultLineList = pdocument->RESULT_PatternLineList();                              LOG_INFORMATION_RAW("== Lines in result: " & tableResultLineList.get_row_count() & " breaks if above: " & uMax );

      std::string stringOutput = ( *poptionsActive )["output"].as_string();
      if( stringOutput.empty() == true )
      {
         std::string stringCliTable;

#ifdef _WIN32
         if( poptionsActive->exists("vs") == false )
         {
            stringCliTable = gd::table::to_string(tableResultLineList, gd::table::tag_io_cli{});
            std::cout << "\n" << stringCliTable << "\n\n";
         }
         else
         {
            //stringCliTable = "\n-- Result from search  --\n";
            stringCliTable = "\n";
            CDocument::RESULT_VisualStudio_s(tableResultLineList, stringCliTable);
            result_ = VS::CVisualStudio::Print_s( stringCliTable, VS::tag_vs_output{});            
            if( result_.first == false ) 
            { 
               std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
               std::cerr << stringError << "\n";
               return result_; 
            }
         }
#else
         stringCliTable = gd::table::to_string(tableResultLineList, gd::table::tag_io_cli{});
         std::cout << "\n" << stringCliTable << "\n\n";
#endif // _WIN32
      }
      else
      {
         auto result_ = pdocument->RESULT_Save({ {"type", "LIST"}, {"output", stringOutput}}, &tableResultLineList);
         if( result_.first == false ) return result_;
      }
   }
   else if( stringCommandName == "help" )                                      // command = "help"
   {
      std::string stringDocumentation;
      optionsApplication.print_documentation( stringDocumentation, gd::cli::options::tag_documentation_verbose{});
      std::cout << stringDocumentation << "\n";
   }
   else if( stringCommandName == "version" )
   {
      std::cout << "version 0.9.3" << "\n";
   }
   else
   {
      return { false, "Unknown command: " + stringCommandName };
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------- @TAG #directory
 * @brief Creates application specific directory where files used for cleaner are stored
 * 
 * Cleaner can be configured with configuration file and it also handles history
 * On Windows this is stored in %APPDATA%/tools/cleaner and on Linux in ~/.config/cleaner
 * 
 * @return std::pair<bool, std::string> - (success, error message)
 *         - success: true if directory was created or already exists, false on failure
 *         - error message: empty string on success, descriptive error on failure
 */
std::pair<bool, std::string> CApplication::CreateDirectory()
{
   std::filesystem::path pathTarget;

#ifdef _WIN32
   // ## Get %APPDATA% path using SHGetFolderPathW

   wchar_t puAppdataPath[MAX_PATH];
   HRESULT iResult = ::SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, puAppdataPath);
   if (FAILED(iResult)) { return { false, "Failed to retrieve APPDATA path" }; }

   // Convert wide string to UTF-8 string
   char utf8_path[MAX_PATH * 4]; // Buffer for UTF-8 conversion
   int iConverted = ::WideCharToMultiByte(CP_UTF8, 0, puAppdataPath, -1, utf8_path, sizeof(utf8_path), NULL, NULL);
   if(iConverted == 0) { return { false, "Failed to convert APPDATA path to UTF-8" }; }

   // Construct full path: %APPDATA%/tools/cleaner
   pathTarget = std::filesystem::path(utf8_path) / "tools" / "cleaner";

#else // Linux
   // ## Check for $XDG_CONFIG_HOME, fallback to ~/.config

   const char* pbszXDG_CONFIG_HOME = std::getenv("XDG_CONFIG_HOME");
   std::string stringConfigurationBase;
   if( pbszXDG_CONFIG_HOME != nullptr && *pbszXDG_CONFIG_HOME != '\0' )
   {
      stringConfigurationBase = pbszXDG_CONFIG_HOME;
   } 
   else 
   {
      const char* pbszHome = std::getenv("HOME");                             // Get home directory
      if( pbszHome != nullptr ) 
      {
         struct passwd* pw = getpwuid(getuid());                              // Fallback to getpwuid if $HOME is not set
         if(!pw || !pw->pw_dir) { return { false, "Failed to retrieve home directory" }; }
         pbszHome = pw->pw_dir;
      }
      stringConfigurationBase = std::string(pbszHome) + "/.config";
   }

   pathTarget = std::filesystem::path(stringConfigurationBase) / "cleaner";    // Construct full path: ~/.config/cleaner
#endif

   try 
   {
      if(!std::filesystem::exists(pathTarget) == false )                       // Create directory if it doesn't exist (recursive creation for 'tools' on Windows)
      {
         if( std::filesystem::create_directories(pathTarget) == false )
         {
            return { false, "Failed to create directory: " + pathTarget.string() };
         }
      }
   } 
   catch (const std::filesystem::filesystem_error& e) 
   {
      return { false, "Failed to create directory: " + std::string(e.what()) };
   } 
   catch (const std::exception& e) 
   {
      return { false, "Unexpected error: " + std::string(e.what()) };
   }

   return { true, "" };
}


std::pair<bool, std::string> CApplication::PrintMessage(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat)
{
   // Optionally extract formatting options from argumentsFormat
   // For example: severity, color, output target, etc.
   // Example: std::string_view severity = argumentsFormat["severity"].as_string_view();

   enumUIType eUIType = m_eUIType; // Get the UI type from the application instance

   if( argumentsFormat.exists("ui") == true ) 
   {
      std::string stringUIType = argumentsFormat["ui"].as_string();
      eUIType = GetUITypeFromString_s( stringUIType );
   }

   switch(eUIType)
   {
   case eUITypeConsole:
      std::cout << stringMessage << std::endl;
      break;
   case eUITypeWeb:
      // Implement web output logic here
      // e.g., send message to web UI log
      break;
   case eUITypeWIMP:
      // Implement desktop UI message box or log
      break;
   case eUITypeVSCode:
#ifdef _WIN32
   case eUITypeVS:
      {
         auto result_ = VS::CVisualStudio::Print_s( stringMessage, VS::tag_vs_output{});            
         if( result_.first == false ) 
         { 
            std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
            std::cerr << stringError << "\n";
            return result_; 
         }
      }
      break;
#endif // _WIN32
   case eUITypeSublime:
      // Implement extension output logic here
      break;
   default:
      // Fallback to console
      std::cout << stringMessage << std::endl;
      break;
   }

   return {true, ""};
}


std::pair<bool, std::string> CApplication::STATEMENTS_Load(const std::string_view& stringFileName)
{
   using namespace pugi;
   xml_document xmldocument;   // xml document used to load xml from file

   if( std::filesystem::exists( stringFileName ) == false ) return { true, std::string("statements file not found ") + stringFileName.data() };

   m_pstatements = std::make_unique<application::database::metadata::CStatements>();

   // ## Load xml data
   xml_parse_result xmlparseresult = xmldocument.load_file(stringFileName.data()); // load xml file
   if( (bool)xmlparseresult == false ) return { false, xmlparseresult.description() };

   {
      // ## get all statements
      auto xmlnodeStatements = xmldocument.document_element().child( "statements" );
      while( xmlnodeStatements.empty() == false )
      {
         // ## get statement
         pugi::xml_node xmlnode = xmlnodeStatements.first_child();
         while( xmlnode.empty() == false )                                     // if child
         {
            if( std::string_view(xmlnode.name()) == std::string_view{ "statement" } )
            {
               std::string_view stringName = xmlnode.attribute("name").value();
               std::string_view stringType = xmlnode.attribute("type").value();
               std::string_view stringStatement = xmlnode.child_value();
               if( stringName.empty() == false && stringStatement.empty() == false )
               {
                  if( stringType.empty() == true ) { stringType = "select"; }
                  m_pstatements->Append( gd::argument::arguments( { { "name", stringName }, { "type", stringType }, { "sql", stringStatement } }, gd::types::tag_view{}) ); // add statement to list
               }
            }
            xmlnode = xmlnode.next_sibling();                                  // get next statement
         }
         
         xmlnodeStatements = xmlnodeStatements.next_sibling("statements");     // get next statements
      }
   }


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document with the specified name.
 * 
 * @param stringName The name of the document to add.
 */
CDocument* CApplication::DOCUMENT_Add(const std::string_view& stringName) 
{
   auto pdocument = std::make_unique<CDocument>( this, stringName );
   m_vectorDocument.push_back(std::move(pdocument));
   return m_vectorDocument.back().get();
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document based on the provided arguments.
 * 
 * @param arguments_ The arguments used to create the document.
 */
CDocument* CApplication::DOCUMENT_Add(const gd::argument::shared::arguments& arguments_) 
{
   auto pdocument = std::make_unique<CDocument>( arguments_ );
   // Assuming CDocument has a method to initialize from arguments
   // doc->Initialize(arguments_);
   m_vectorDocument.push_back(std::move(pdocument));
   return m_vectorDocument.back().get();
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name.
 * 
 * @param stringName The name of the document to retrieve.
 * @return CDocument* Pointer to the document if found, otherwise nullptr.
 */
const CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName) const 
{
   for( const auto& pdocument : m_vectorDocument ) 
   {
      if(pdocument->GetName() == stringName) 
      {
         return pdocument.get();
      }
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name.
 * 
 * @param stringName The name of the document to retrieve.
 * @return CDocument* Pointer to the document if found, otherwise nullptr.
 */
CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName) 
{
   for( const auto& pdocument : m_vectorDocument ) 
   {
#ifndef NDEBUG
      auto stringName_d = pdocument->GetName();
#endif // !NDEBUG

      if(pdocument->GetName() == stringName) 
      {
         return pdocument.get();
      }
   }
   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief Removes a document by its name.
 * 
 * @param stringName The name of the document to remove.
 */
void CApplication::DOCUMENT_Remove(const std::string_view& stringName) 
{
   m_vectorDocument.erase(
      std::remove_if(m_vectorDocument.begin(), m_vectorDocument.end(),
         [&stringName](const std::unique_ptr<CDocument>& doc) {
            return doc->GetName() == stringName;
         }),
      m_vectorDocument.end());
}

/** ---------------------------------------------------------------------------
 * @brief Gets the number of documents.
 * 
 * @return size_t The number of documents.
 */
size_t CApplication::DOCUMENT_Size() const 
{
   return m_vectorDocument.size();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if there are no documents.
 * 
 * @return bool True if there are no documents, otherwise false.
 */
bool CApplication::DOCUMENT_Empty() const 
{
   return m_vectorDocument.empty();
}

/** -
 * @brief Clears all documents.
 */
void CApplication::DOCUMENT_Clear() 
{
   m_vectorDocument.clear();
}


// 0TAG0Database.Application

/** ---------------------------------------------------------------------------
 * @brief Open a database connection
 *
 * @param argumentsOpen The arguments used to open the database.
 * @param argumentsOpen.file The file name of the database.
 * @param argumentsOpen.create Whether to create the database if it doesn't exist (default: true).
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Open(const gd::argument::shared::arguments& argumentsOpen)
{                                                                                                  assert( argumentsOpen.empty() == false ); // Ensure the arguments are not empty
   DATABASE_CloseActive();

   bool bConnected = false;
   bool bCreate = true;
   if( argumentsOpen.exists("create") == true ) { bCreate = argumentsOpen["create"].as_bool(); }

   std::string stringPath = argumentsOpen["file"].as_string();
   if( stringPath.empty() == false )
   {
      gd::argument::arguments argumentsCreate({ {"file", stringPath}, { "create", bCreate} } );
         
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();     // create database interface
      pdatabase->add_reference();
      auto result_ = pdatabase->open(argumentsCreate);
      if( result_.first == false )
      {
         pdatabase->release();
         return { false, result_.second };
      }
      m_pdatabase = pdatabase;
      bConnected = true;

   }
   return { bConnected, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Check if database needs to be updated and needed then update the database to the latest version
 *
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Update()
{                                                                                                  assert(m_pdatabase != nullptr); // Ensure the active database connection is valid
   // Check if the table "TVersion" exists
   uint64_t uVersion;
   gd::variant value_;
   auto result_ = m_pdatabase->ask("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TVersion';", &value_ );
   if( result_.first == false ) { return { false, result_.second }; }
   uVersion = value_.as_uint64();

   if( uVersion < 1u )
   {
      result_ = DATABASE_Upgrade( uVersion );
      if( result_.first == false ) { return { false, result_.second }; }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Upgrade the database to the latest version
 *
 * @param uVersion The current version of the database
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */ 
std::pair<bool, std::string> CApplication::DATABASE_Upgrade(uint64_t uVersion)
{
   std::string_view stringSql;
   
   if( uVersion == 0 )
   {
      stringSql = R"sql(
CREATE TABLE TVersion( VersionK INTEGER PRIMARY KEY, FVersion INTEGER, FMajor INTEGER, FMinor INTEGER, FBuild INTEGER, FRevision INTEGER );

CREATE TABLE TProject( ProjectK INTEGER PRIMARY KEY, TypeC INTEGER, StateC INTEGER, FName TEXT, FFolder VARCHAR(260), FDescription TEXT, FVersion INTEGER ); 

CREATE TABLE TFile( FileK INTEGER PRIMARY KEY, ProjectK INTEGER, TypeC INTEGER, FName TEXT, FSize INTEGER, FDescription TEXT );
CREATE TABLE TFileProperty( 
   FilePropertyK INTEGER PRIMARY KEY, FileK INTEGER, ValueType INTEGER, FName TEXT, FValue TEXT, FDate REAL,
   FOREIGN KEY( FileK ) REFERENCES TFile( FileK ) ON DELETE CASCADE
);

CREATE TABLE TCodeGroup( CodeGroupK INTEGER PRIMARY KEY, FName TEXT, FDescription TEXT );
CREATE TABLE TCode( 
   CodeK INTEGER PRIMARY KEY, CodeGroupK INTEGER, FName VARCHAR(100), FDescription TEXT, 
   FOREIGN KEY( CodeGroupK ) REFERENCES TCodeGroup( CodeGroupK ) 
);

INSERT INTO TVersion( FVersion, FMajor, FMinor, FBuild, FRevision ) VALUES ( 1, 0, 0, 0, 0 );
INSERT INTO TProject( ProjectK, FName, FDescription, FVersion ) VALUES ( 1, 'demo', 'demo project', 1 );
)sql"; 
      auto result_ = m_pdatabase->execute(stringSql);
      if( result_.first == false ) { return { false, result_.second }; }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Connect to database
 * Connect to database based on string that holds information on how to connect
 * 
 * @param stringConnect connection string
 * @return true if ok, false and error information if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Connect( const std::string_view& stringConnect )
{
   std::string stringConnectionName;
   std::string stringConnectionName2;

   DATABASE_CloseActive();

   if( stringConnect.empty() == true )
   {
      stringConnectionName = PROPERTY_Get("database").as_string();             assert( stringConnectionName.empty() == false );
   }
   else
   {
      stringConnectionName = stringConnect;
   }

   // ## connect main database
   bool bOk = false;
   std::string stringError;
   gd::database::database_i* pdatabaseMain = nullptr;

   
   if(true)
   {
#ifdef GD_DATABASE_ODBC_USE      
      gd::database::odbc::database_i* pdatabase = new gd::database::odbc::database_i();  // create database interface
      pdatabase->add_reference();
      std::tie(bOk, stringError) = pdatabase->open( stringConnectionName );
      pdatabaseMain = pdatabase;
#else
      bOk = false;
      stringError = "Only file connection is enabled, make sure to connect to file database (sqlite)";
#endif      
   }
   else
   {
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();  // create database interface
      pdatabase->add_reference();
      std::tie(bOk, stringError) = pdatabase->open( stringConnectionName );
      pdatabaseMain = pdatabase;
   }

   if( bOk == true )
   {
      m_pdatabase = pdatabaseMain;
   }
   else
   {
      if( pdatabaseMain != nullptr ) pdatabaseMain->release();
      if( stringError.empty() == true ) stringError = "Failed to collect information about database connection";
      return { false, stringError };
   }

   return { true, "" };
}

void CApplication::DATABASE_Append( gd::database::database_i* pdatabase, bool bActivate ) 
{                                                                                                  assert( pdatabase != nullptr );
   pdatabase->add_reference();
   m_vectorDatabase.push_back( pdatabase ); 
   if( bActivate == true )
   {
      if( m_pdatabase != nullptr ) m_pdatabase->release();
      m_pdatabase = pdatabase;
      if( m_pdatabase != nullptr ) m_pdatabase->add_reference();
   }
}


/** ---------------------------------------------------------------------------
 * @brief Close active database
*/
void CApplication::DATABASE_CloseActive()
{
   if( m_pdatabase != nullptr )
   {
      m_pdatabase->release();
      m_pdatabase = nullptr;
   }
}


/** ---------------------------------------------------------------------------
* @brief Add error to internal list of errors
* @param stringError error information
*/
void CApplication::ERROR_Add( const std::string_view& stringError )
{
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexError );            // locks `m_vectorError`
   gd::argument::arguments argumentsError( { {"text", stringError} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsError) );
}

/** ---------------------------------------------------------------------------
 * @brief Get error information
 * 
 * If no errors then empty string is returned
 * @return std::string error information
 */
std::string CApplication::ERROR_Report() const
{
   if( m_vectorError.empty() == false )
   {
      std::string stringError;
      for( const auto& error_ : m_vectorError )
      {
         stringError += error_.print();
         stringError += "\n";
      }
      return stringError;
   }

   return std::string();
}


// 0TAG0OPTIONS.Application

CApplication::enumUIType CApplication::GetUITypeFromString_s(const std::string_view& stringUIType)
{
   if( stringUIType == "console" )   return eUITypeConsole;
   if( stringUIType == "web" )       return eUITypeWeb;
   if( stringUIType == "wimp" )      return eUITypeWIMP;
   if( stringUIType == "vscode" )    return eUITypeVSCode;
   if( stringUIType == "vs" )        return eUITypeVS;
   if( stringUIType == "sublime" )   return eUITypeSublime;

   return eUITypeUnknown; // Default to none if no match found
}

/** --------------------------------------------------------------------------- @TAG #option
 * @brief Prepares the application options for command-line usage.
 *
 * This method sets up the available command-line options for the application,
 * including global options and subcommands. Each subcommand is configured
 * with its specific options and descriptions.
 *
 * @param optionsApplication A reference to the `gd::cli::options` object
 *                           where the options and subcommands will be added.
 *
 * ### Global Options
 * - `editor`        : For editor specific configuration. vs, vscode or sublime is currently supported.
 * - `logging`       : Enables logging.
 * - `logging-csv`   : Adds a CSV logger for log messages.
 * - `print`         : Prints results from commands.
 * - `recursive`     : Specifies recursive operations with a depth value.
 * - `output`        : Saves output to a specified file.
 * - `database`      : Sets the folder for log files.
 * - `statements`    : Specifies a file containing SQL statements.
 *
 * ### Subcommands
 * - `count`    : Counts lines in files or directories.
 * - `copy`     : Copies files from source to destination.
 * - `db`       : Configures database settings.
 * - `history`  : Handles command history.
 * - `list`     : Lists rows matching specified patterns.
 * - `join`     : Joins two or more files.
 * - `help`     : Displays help information.
 * - `version`  : Displays the application version.
 */
void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   optionsApplication.add_flag( {"logging", "Turn on logging"} );              // logging is turned on using this flag
   optionsApplication.add_flag( {"logging-csv", "Add csv logger, prints log information using the csv format"} );
   optionsApplication.add_flag({ "print", "Reults from command should be printed" });
   optionsApplication.add_flag( {"explain", "Print additional context or descriptions about items, which can be especially useful if you need clarification or a deeper understanding"} );
   optionsApplication.add_flag({ "help", "Prints help information about command" });
   optionsApplication.add({ "editor", "type of editor, vs or vscode is currently supported" });
   optionsApplication.add({ "recursive", "Operation should be recursive, by settng number decide the depth" });
   optionsApplication.add({ "output", 'o', "Save output to the specified file. Overwrites the file if it exists. Defaults to stdout if not set."});
   //optionsApplication.add({ "database", "Set folder where logger places log files"});
   //optionsApplication.add({ "statements", "file containing sql statements"});

   {  // ## `copy` command, copies file from source to target
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "count", "count lines in file" );
      optionsCommand.add({ "source", 's', "File/folders to count lines in"});
      optionsCommand.add({ "comment", "Pair of characters marking start and end for comments"});
      optionsCommand.add({ "pattern", 'p', "patterns to search for, multiple values are separated by , or ;"});
      optionsCommand.add({ "string", "Pair of characters marking start and end for strings"});
      optionsCommand.add({ "filter", "Filter to use, if empty then all found files are counted, filter format is wildcard file name matching" });
      optionsCommand.add({ "sort", "Sorts result on selected column name" });
      optionsCommand.add({ "stats", "Add statistics to generated output" });
      optionsCommand.add({ "table", "Table is used based on options set, for example generating sql insert queries will use table name to insort to" });
      optionsCommand.add_flag( {"R", "Set recursive to 16, simple to scan all subfolders"} );
#ifdef _WIN32
      optionsCommand.add_flag( {"vs", "Adapt to visual studio output window format, make files clickable"} );
      optionsCommand.add_flag( {"win", "Windows specific functionality, logic might be using some special for adapting to features used for windows"} );
#endif
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `copy` command, count number of lines in file
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "copy", "Copy file from source to target" );
      optionsCommand.add({"source", 's', "File to copy"});
      optionsCommand.add({"destination", 'd', "Destination, where file is copied to"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `db` command, use database and configure settings for that
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "db", "Configure database" );
      optionsCommand.add({"file", 'f', "Where to place database file (used for sqlite databases)"});
      optionsCommand.add({"settings", "Where to write configuration file"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   { // ## 'dir' command, list files
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "dir", "List files in directory" );
      optionsCommand.add({"source", 's', "Directory to list"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add(std::move(optionsCommand));
   }

   // ## 'history' handle history 
   {
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "history", "Handle command history" );
      optionsCommand.add_flag( {"create", "Initialize history logic, creates folders and files needed to manage history, this also enables configuration settings"} );
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add(std::move(optionsCommand));
      //optionsCommand.add({});
   }

   // ## 'list' list rows with specified patterns
   {
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "list", "list rows with specified patterns" );
      optionsCommand.add({ "source", 's', "File/folders where to search for patterns in"});
      optionsCommand.add({ "pattern", 'p', "patterns to search for, multiple values are separated by , or ;"});
      optionsCommand.add({ "max", "Max list count to avoid too many hits"});
      optionsCommand.add({ "segment", "type of segment in code to searh in"});
      optionsCommand.add_flag( {"R", "Set recursive to 16, simple to scan all subfolders"} );
#ifdef _WIN32
      optionsCommand.add_flag( {"vs", "Adapt to visual studio output window format, make files clickable"} );
      optionsCommand.add_flag( {"win", "Windows specific functionality, logic might be using some special for adapting to features used for windows"} );
#endif
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add(std::move(optionsCommand));
   }

   {  // ## `join` command, joins two or more files
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "join", "join two or more files" );
      optionsCommand.add({"source", 's', "Files to join"});
      optionsCommand.add({"destination", 'd', "Destination, joined files result"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   {  // ## `help` print help about champion
      gd::cli::options optionsCommand( "help", "Print command line help" );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   {  // ## `version` print current version
      gd::cli::options optionsCommand( "version", "Print version" );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }
}

// ----------------------------------------------------------------------------
/// @brief Prepare logging
void CApplication::PrepareLogging_s()
{
#ifdef GD_LOG_SIMPLE
   using namespace gd::log;
   gd::log::logger<0>* plogger = gd::log::get_s();                            // get pointer to logger 0

   plogger->append( std::make_unique<gd::log::printer_console>() );           // append printer to logger, this prints to console

   // ## set margin for log messages, this to make it easier to read. a bit hacky 
   auto* pprinter_console = (gd::log::printer_console*)plogger->get( 0 );
   // ## color console messages in debug mode
   pprinter_console->set_margin( 8 );                                         // set log margin
   pprinter_console->set_margin_color( eColorBrightBlack );

   unsigned uSeverity = unsigned(eSeverityNumberVerbose) | unsigned(eSeverityGroupDebug);
   plogger->set_severity( uSeverity ); 

#endif // GD_LOG_SIMPLE
}



/** --------------------------------------------------------------------------- @TAG #state #parse
 * @brief Prepares the state for parsing based on the file extension.
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @param state_ The state object to be prepared.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> CApplication::PrepareState_s(const gd::argument::shared::arguments& argumentsPath, gd::expression::parse::state& state_)
{
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);
   
   gd::file::path pathFile(stringFile);
   std::string stringExtension = pathFile.extension().string();

   // convert string to lowercase
   std::transform(stringExtension.begin(), stringExtension.end(), stringExtension.begin(), ::tolower);

   if( stringExtension.length() < 2 ) return { false, "File extension is too short: " + stringExtension };

   if( stringExtension[1] == 'c' || stringExtension[1] == 'h' )
   {
      if( stringExtension == ".cpp" || stringExtension == ".c" || stringExtension == ".cc" || stringExtension == ".cxx" || stringExtension == ".h" || stringExtension == ".hpp" || stringExtension == ".hxx" )
      {
         state_.add(std::string_view("LINECOMMENT"), "//", "\n");
         state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
         state_.add(std::string_view("STRING"), "\"", "\"", "\\");
         state_.add(std::string_view("RAWSTRING"), "R\"(", ")\"");
         return { true, "" };
      }
   }
   
   if( stringExtension == ".cs" || stringExtension == ".fs" || stringExtension == ".kt" || stringExtension == ".swift" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "\"\"\"", "\"\"\"");
   }
   else if( stringExtension == ".java" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   }
   else if( stringExtension == ".js" || stringExtension == ".ts" || stringExtension == ".tsx" || stringExtension == ".jsx" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
      state_.add(std::string_view("RAWSTRING"), "`", "`");

      if( stringExtension == ".jsx" || stringExtension == ".tsx" ) { state_.add(std::string_view("BLOCKCOMMENT"), "{/*", "*/}"); }
   }
   else if( stringExtension == ".go" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\"); // Double-quoted
      state_.add(std::string_view("RAWSTRING"), "`", "`");      // Raw string (no escaping)
   }
   else if( stringExtension == ".rs" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   }
   else if( stringExtension == ".html" || stringExtension == ".xml" )
   {
      state_.add(std::string_view("BLOCKCOMMENT"), "<!--", "-->");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".css" )
   {
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".py" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "\"\"\"", "\"\"\"");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".sql" )
   {
      state_.add(std::string_view("LINECOMMENT"), "--", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".php" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".lua" )
   {
      state_.add(std::string_view("LINECOMMENT"), "--", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "--[[", "]]");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
      state_.add(std::string_view("RAWSTRING"), "[[", "]]");
   }
   else if( stringExtension == ".rb" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "=begin", "=end");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".json" )
   {
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".pl" || stringExtension == ".pm" ) 
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".sh" || stringExtension == ".bash" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".yaml" || stringExtension == ".yml" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".toml" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".dart" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "r\"", "\"");
   }
   else if( stringExtension == ".clj" )
   {
      state_.add(std::string_view("LINECOMMENT"), ";", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   }
   else if( stringExtension == ".vim" )
   {
      state_.add(std::string_view("LINECOMMENT"), "\"", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\'");
   }
   else if( stringExtension == ".txt" || stringExtension == ".md" )
   {
      // No special states for text files
   }
   else
   {
      return { false, "Unknown file type: " + stringFile };
   }
   
   return { true, "" };
}




/** ---------------------------------------------------------------------------
 * @brief Ensures the provided path is absolute. If the path is empty, it sets it to the current working directory.
 *
 * This method processes a given path string and ensures it is in an absolute format. If the path is empty, it assigns
 * the current working directory to the path. If the path contains multiple entries separated by `;` or `,`, it processes
 * each entry individually to ensure they are absolute paths.
 *
 * ### Behavior:
 * - If the path is empty, it assigns the current working directory.
 * - If the path is relative, it converts it to an absolute path.
 * - If the path contains multiple entries separated by `;` or `,`, it processes each entry individually.
 *
 * @param stringPath A reference to the path string to process. The string is modified in place.
 *
 * ### Example Usage:
 * ```cpp
 * std::string path = "relative/path";
 * CApplication::PathPrepare_s(path);
 * // path is now absolute
 * ```
 **/
void CApplication::PathPrepare_s(std::string& stringPath)
{
   auto uPosition = stringPath.find_first_of(";,");                           // Find the first occurrence of `;` or `,` if multiple path

   if( uPosition != std::string::npos )
   {
      char iSplitCharacter = stringPath[uPosition]; // split character
      std::string stringNewPath; // new generated path

      auto vectorPath = Split_s(stringPath);                                  // Split string by `;` or `,` and check files to make them absolute
      for( const auto& it : vectorPath )
      {
         if( it.empty() == false )
         {
            if( stringNewPath.empty() == false )  stringNewPath += iSplitCharacter;

            std::string stringCheck = it;
            PathPrepare_s(stringCheck);
            stringNewPath += stringCheck;
         }
      }
      stringPath = stringNewPath;                                             // Update to the fixed path
   }
   else
   {
      if( stringPath.empty() )                                                // no path ?
      {
         std::filesystem::path pathFile = std::filesystem::current_path();    // take current working directory
         stringPath = pathFile.string();
      }
      else
      {
         // ## fix path to make it absolute

         std::filesystem::path pathFile(stringPath);

         if( pathFile.is_absolute() == false )
         {
            if( pathFile.is_relative() == true )                              // Check if path is relative
            {
               // ## make path absolute

               std::filesystem::path pathAbsolute = std::filesystem::absolute(pathFile);
               gd::file::path path_(pathAbsolute);
               stringPath = path_.string();
            }
         } // if( pathFile.is_absolute() == false )
      } // if( stringPath.empty() ) else
   } // if( uPosition != std::string::npos ) else
}

void CApplication::Read_s(const gd::database::record* precord, gd::table::table_column_buffer* ptablecolumnbuffer )
{
   for( unsigned u = 0, uMax = (unsigned)precord->size(); u < uMax; u++ )
   {
      auto pcolumn = precord->get_column( u );
      std::string_view stringName = precord->name_get( u );
      unsigned uType = pcolumn->type();
#ifndef NDEBUG
      auto pbszTypeName_d = gd::types::type_name_g( uType );
#endif // !NDEBUG

      unsigned uSize = 0;
      if( pcolumn->is_fixed() == false ) { uType |= gd::types::eTypeDetailReference; }
      else                               { uSize = pcolumn->size_buffer(); }

      ptablecolumnbuffer->column_add( uType, uSize, stringName );
   }
}

void CApplication::Read_s( gd::database::cursor_i* pcursorSelect, gd::table::table_column_buffer* ptablecolumnbuffer )
{
   const auto* precord = pcursorSelect->get_record();

   if( ptablecolumnbuffer->empty() == true )
   {
      if( ptablecolumnbuffer->get_reserved_row_count() == 0 ) ptablecolumnbuffer->set_reserved_row_count( 10 ); //pre allocate data to hold 10 rows 
      ptablecolumnbuffer->set_flags( gd::table::tag_full_meta{});
      Read_s( precord, ptablecolumnbuffer );
      ptablecolumnbuffer->prepare();

      while( pcursorSelect->is_valid_row() == true )
      {
         auto vectorValue = precord->get_variant_view();
         ptablecolumnbuffer->row_add( vectorValue );
         pcursorSelect->next();
      }
   }
   else
   {
      // ## table contains columns, match against tables in result to know what to add
      auto vectorTableName = ptablecolumnbuffer->column_get_name();
      auto vectorResultName = precord->name_get();
      // Match column names, only fill in columns with matching name in table and result
      auto vectorMatch = gd::table::table_column_buffer::column_match_s( vectorTableName, vectorResultName );

      if( vectorMatch.empty() == false )
      {
         std::vector<unsigned> vectorWriteTable;
         std::vector<unsigned> vectorReadResult;

         for( const auto& it : vectorMatch )
         {
            vectorWriteTable.push_back( it.first );
            vectorReadResult.push_back( it.second );
         }

         while( pcursorSelect->is_valid_row() == true )
         {
            auto vectorValue = precord->get_variant_view( vectorReadResult );
            ptablecolumnbuffer->row_add( vectorValue, vectorWriteTable, gd::table::tag_convert{} );
            pcursorSelect->next();
         }
      }
      else
      {
         while( pcursorSelect->is_valid_row() == true )
         {
            auto vectorValue = precord->get_variant_view();
            ptablecolumnbuffer->row_add( vectorValue, gd::table::tag_convert{} );
            pcursorSelect->next();
         }
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Save command line arguments to history file
 *
 * @param stringArguments The command line arguments to save.
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::HistorySaveArguments_s(const std::string_view& stringArguments)
{
   return { true, "" };
#ifdef WIN32

   // Create file
   wchar_t puProgramDataPath[MAX_PATH];

   if( !GetEnvironmentVariableW(L"ProgramData", puProgramDataPath, MAX_PATH) ) { return { false, "" }; }
   std::wstring stringDirectory = std::wstring(puProgramDataPath) + L"\\tools";
   if( !std::filesystem::exists(stringDirectory) )
   {
      if( std::filesystem::create_directory(stringDirectory) == false ) { return { false, "" }; }
   }

   stringDirectory += L"\\cleaner";
   if( !std::filesystem::exists(stringDirectory) )
   {
      if( std::filesystem::create_directory(stringDirectory) == false ) { return { false, "" }; }
   }

   std::wstring stringFilePath = stringDirectory + L"\\history.xml";

   pugi::xml_document xmldocument;

   pugi::xml_parse_result _result = xmldocument.load_file(stringFilePath.c_str());
   if( !_result )
   {
      return { false, "" };
   }

   //pugi::xml_node commands_nodeAppend = xmldocument.append_child("commands");

   if( !xmldocument.save_file(stringFilePath.c_str()) )    // clears all here
   {
      return { false, "" };
   }

   // Append command
   pugi::xml_node commands_nodeChild = xmldocument.child("commands");
   if( !commands_nodeChild )
   {
      commands_nodeChild = xmldocument.append_child("commands");
   }

   int iCount = 1;
   // Check if command already exists
   for( auto command : commands_nodeChild.children("command") )
   {
      if( command.child_value() == stringArguments )
      {
         commands_nodeChild.remove_child(command); // remove command if it exists
         pugi::xml_node count_node = command.child("count");

         if( count_node )
         {
            iCount = std::stoi(count_node.child_value()) + 1;
         }
      }
   }

   //commands_nodeChild.append_child("command").append_child(pugi::node_pcdata).set_value(stringArguments);
   pugi::xml_node command_node = commands_nodeChild.append_child("command");
   command_node.append_child(pugi::node_pcdata).set_value(stringArguments);
   command_node.append_child("count").append_child(pugi::node_pcdata).set_value(std::to_string(iCount).c_str());

   xmldocument.save_file(stringFilePath.c_str());
#else
#endif

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Print command history
 *
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::HistoryPrint_s()
{
#ifdef WIN32

   // Create file
   wchar_t puProgramDataPath[MAX_PATH];

   if( !GetEnvironmentVariableW(L"ProgramData", puProgramDataPath, MAX_PATH) )
   {
      return { false, "" };
   }

   std::wstring stringDirectory = std::wstring(puProgramDataPath) + L"\\tools\\cleaner";

   std::wstring stringFilePath = stringDirectory + L"\\history.xml";

   pugi::xml_document xmldocument;

   pugi::xml_parse_result _result = xmldocument.load_file(stringFilePath.c_str());
   if( !_result )
   {
      return { false, "" };
   }

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

#else
#endif

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Splits a string into a vector of strings based on the specified delimiter.
 * 
 * *Sample code*
 * ```cpp
 * std::string stringText = "apple;banana;cherry";
 * char iDelimiter = ';';
 * std::vector<std::string> result = Split_s(stringText, iDelimiter);
 * // result will contain {"apple", "banana", "cherry"}
 * ```
 *
 * @param stringText The string to split.
 * @param iDelimiter The delimiter character to use for splitting. If 0, it will try to determine the delimiter.
 * @return std::vector<std::string> A vector of strings obtained by splitting the input string.
 */
std::vector<std::string> CApplication::Split_s(const std::string& stringText, char iDelimiter) 
{
   std::vector<std::string> vectorResult; // vector to hold the split strings

   char iEffectiveDelimiter = iDelimiter;
   if( iEffectiveDelimiter == 0 )
   {
      // ## Determine the effective delimiter

      auto uSemicolon = stringText.find(";");
      auto uComma = stringText.find(",");

      if( uSemicolon != std::string::npos && uComma != std::string::npos )
      {
         iEffectiveDelimiter = ( uSemicolon < uComma ) ? ';' : ',';           // select the first found
      }
      else if( uSemicolon != std::string::npos )
      {
         iEffectiveDelimiter = ';';
      }
      else if( uComma != std::string::npos )
      {
         iEffectiveDelimiter = ',';
      }
   }

   // ## Split the string using the effective delimiter

   vectorResult = gd::utf8::split( stringText, iEffectiveDelimiter, gd::utf8::tag_string{});

   return vectorResult;
}




#ifdef _WIN32

std::pair<bool, std::string> CApplication::PrepareWindows_s() 
{
   // Initialize COM library
   HRESULT iResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
   if( FAILED(iResult) ) { return {false, "Failed to initialize COM library. HRESULT: " + std::to_string(iResult)}; }

   // Additional Windows-specific preparation can go here
   // For example, setting up security, initializing other Windows APIs, etc.

   // Example: Set COM security levels
   /*
   hr = CoInitializeSecurity(
      nullptr,                        // Security descriptor
      -1,                             // COM negotiates authentication services
      nullptr,                        // Authentication services
      nullptr,                        // Reserved
      RPC_C_AUTHN_LEVEL_DEFAULT,      // Default authentication level
      RPC_C_IMP_LEVEL_IMPERSONATE,    // Default impersonation level
      nullptr,                        // Authentication info
      EOAC_NONE,                      // Additional capabilities
      nullptr                         // Reserved
   );

   if (FAILED(hr)) 
   {
      CoUninitialize(); // Clean up COM if security initialization fails
      return {false, "Failed to initialize COM security. HRESULT: " + std::to_string(hr)};
   }
   */

   papplication_g->PROPERTY_Add("WINDOWS", true );

   // If everything succeeds
   return {true, ""};
}

std::pair<bool, std::string> CApplication::ExitWindows_s()
{
   // Uninitialize the COM library
   CoUninitialize();

   papplication_g->PROPERTY_Add("WINDOWS", false );

   return {true, ""};
}


#endif


