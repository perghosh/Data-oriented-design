/**
 * @file Application.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0DATABASE.Application` - database operations
 * 
 */

#include <filesystem>
#include <format>

#include "pugixml/pugixml.hpp"

#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"

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
    for(const auto& document_ : o.m_vectorDocument) 
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

   if( iArgumentCount > 1 )
   {
      gd::cli::options optionsApplication;
      CApplication::Prepare_s(optionsApplication);
      // Parse the command-line arguments
      auto [bOk, stringError] = optionsApplication.parse(iArgumentCount, ppbszArgument);
      if( bOk == false ) { return { false, stringError }; }

      std::tie(bOk, stringError) = Initialize(optionsApplication);
      if( bOk == false ) { return { false, stringError }; }
   }


   // Process the command-line arguments
   return { true, "" };
}


std::pair<bool, std::string> CApplication::Initialize()
{
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

   // If cleanup is successful
   return {true, ""};

   // If cleanup fails, return an appropriate error message
   // return {false, "Exit failed: <error details>"};
}

std::pair<bool, std::string> CApplication::Initialize( gd::cli::options& optionsApplication )
{
   const gd::cli::options* poptionsActive = optionsApplication.find_active();
   if( poptionsActive == nullptr ) { return { false, "No active options found" }; }

   /// ## prepare command

#ifndef NDEBUG
   auto stringName_d = poptionsActive->name();
#endif // !NDEBUG

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
      std::string stringSource = (*poptionsActive)["source"].as_string();                          
     
      if( stringSource.empty() == true )
      {
         std::filesystem::path pathFile = std::filesystem::current_path();
         stringSource = pathFile.string();
      }                                                                                            LOG_INFORMATION_RAW("== --source: " & stringSource);

      int iRecursive = ( *poptionsActive )["recursive"].as_int();                                  LOG_INFORMATION_RAW("== --recursive: " & iRecursive);
      gd::argument::shared::arguments argumentsPath({ {"source", stringSource}, {"recursive", iRecursive} });

      auto result_ = pdocument->FILE_Harvest(argumentsPath);                   // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if( result_.first == false ) return result_;


      if( (*poptionsActive)["filter"].is_true() == true )
      {
         std::string stringFilter = ( *poptionsActive )["filter"].as_string();                     LOG_INFORMATION_RAW("== --filter: " & stringFilter);
         auto result_ = pdocument->FILE_Filter(stringFilter);
         if( result_.first == false ) return result_;
      }
      else
      {                                                                        // filter not set, then we need to investigate that harvested files contains text, those that dont will be removed
         auto result_ = pdocument->FILE_FilterBinaries();                      // remove files that are not text
         if( result_.first == false ) return result_;
      }

      result_ = pdocument->FILE_UpdateRowCounters();                           // count rows in harvested files
      if( result_.first == false ) return result_;

      bool bSaved = false;                                                     // variable to store if result was saved

      std::string stringOutput = ( *poptionsActive )["output"].as_string();                        LOG_INFORMATION_RAW("== --output: " & stringOutput);
      bool bOutput = ( *poptionsActive )["output"].is_true();

      // if option "print" was specified, then print the result or not if, ignore the bSaved
      if( bSaved == false && poptionsActive->exists("print") == true ) bSaved = true;
      if( bSaved == false || bOutput == true || stringOutput.empty() == false )
      {
         auto tableResult = pdocument->RESULT_RowCount();
         if( stringOutput.empty() == false )
         {
            auto result_ = pdocument->RESULT_Save( { {"type", "COUNT"}, {"output", stringOutput} }, &tableResult );
            if( result_.first == false ) return result_;
         }

         auto result_ = TABLE_AddSumRow( &tableResult, {2, 3, 4, 5, 6});
         if( result_.first == false ) return result_;

         tableResult.cell_set(tableResult.get_row_count() - 1, "folder", "Total:");

         std::string stringCliTable = gd::table::to_string(tableResult, gd::table::tag_io_cli{});  LOG_INFORMATION_RAW("count = total number of lines\ncode = number of code lines\ncharacters = number of code characters\ncomment = number of comments in code\nstring = number of strings in code");
         std::cout << "\n" << stringCliTable << "\n\n";
      }
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
   else if( stringCommandName == "help" )                                      // command = "help"
   {
      std::string stringDocumentation;
      optionsApplication.print_documentation( stringDocumentation );
      std::cout << stringDocumentation << "\n";
   }
   else if( stringCommandName == "version" )
   {
      std::cout << "version 0.9.0" << "\n";
   }
   else
   {
      return { false, "Unknown command: " + stringCommandName };
   }

   return { true, "" };
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


// 0TAG0DATABASE.Application

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



void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   optionsApplication.add_flag( {"logging", "Turn on logging"} );              // logging is turned on using this flag
   optionsApplication.add_flag( {"logging-csv", "Add csv logger, prints log information using the csv format"} );
   optionsApplication.add_flag({ "print", "Reults from command should be printed" });
   optionsApplication.add({ "recursive", "Operation should be recursive, by settng number decide the depth" });
   optionsApplication.add({ "output", 'o', "Save output to the specified file. Overwrites the file if it exists. Defaults to stdout if not set."});
   optionsApplication.add({"database", "Set folder where logger places log files"});
   optionsApplication.add({"statements", "file containing sql statements"});

   {  // ## `copy` command, copies file from source to target
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "count", "count lines in file" );
      optionsCommand.add({"source", 's', "File to count lines in"});
      optionsCommand.add({"comment", "Pair of characters marking start and end for comments"});
      optionsCommand.add({"string", "Pair of characters marking start and end for strings"});
      optionsCommand.add({ "filter", "Filter to use, if empty then all found files are counted" });
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `count` command, count number of lines in file
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "copy", "Copy file from source to target" );
      optionsCommand.add({"source", 's', "File to copy"});
      optionsCommand.add({"destination", 'd', "Destination, where file is copied to"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `db` command, use database and configure settings for that
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "db", "Configure database" );
      optionsCommand.add({"file", 'f', "Where to place database file (used for sqlite databases)"});
      optionsCommand.add({"settings", "Where to write configuration file"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
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

void CApplication::PrepareLogging_s()
{
#ifdef GD_LOG_SIMPLE
   using namespace gd::log;
   gd::log::logger<0>* plogger = gd::log::get_s();                          // get pointer to logger 0

   plogger->append( std::make_unique<gd::log::printer_console>() );         // append printer to logger, this prints to console

   // ## set margin for log messages, this to make it easier to read. a bit hacky 
   auto* pprinter_console = (gd::log::printer_console*)plogger->get( 0 );
   // ## color console messages in debug mode
   pprinter_console->set_margin( 8 );                                       // set log margin
   pprinter_console->set_margin_color( eColorBrightBlack );

   unsigned uSeverity = unsigned(eSeverityNumberVerbose) | unsigned(eSeverityGroupDebug);
   plogger->set_severity( uSeverity ); 

#endif // GD_LOG_SIMPLE
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

