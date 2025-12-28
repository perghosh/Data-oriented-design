#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <queue>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"


#include "gd/gd_file.h"
#include "gd/gd_file_rotate.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"
#include "gd/gd_database_sqlite.h"

#include "gd/console/gd_console_console.h"

#if defined(_MSC_VER)
#   include "windows.h"   
#endif

#include "dto/DTOResponse.h"

#include "Server.h"
//#include "HttpServer.h"

#include "Application.h"

// https://medium.com/@AlexanderObregon/building-restful-apis-with-c-4c8ac63fe8a7
// https://www.youtube.com/watch?v=gVmwrnhkybk
// https://github.com/LegalizeAdulthood/asio-http-websocket/tree/master
//
// https://github.com/dfleury2/beauty/blob/master/include/beauty/server.hpp


// WSL https://learn.microsoft.com/en-us/cpp/build/walkthrough-build-debug-wsl2?view=msvc-170

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace net = boost::asio;                    // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

constexpr std::string_view ROOT_MARKER = "__root";

/// Global pointer to application object
CApplication* papplication_g = nullptr;


std::string FOLDER_GetRoot_g( const std::string_view& stringSubfolder );

CApplication::~CApplication() 
{
   // ## release server if it is set
   if( m_pserver )
   {
      m_pserver->release();
      m_pserver = nullptr;
   }

   if( m_phttpserver != nullptr )
   {
      m_phttpserver->release();
      m_phttpserver = nullptr;
   }

   if( m_pserverBoost ) delete m_pserverBoost;
}


/** --------------------------------------------------------------------------- @CODE [tag: main] [description: main entry point for application]
 * @brief Entry point for the application, handling initialization, argument parsing, and main execution logic.
 * @param iArgumentCount The number of command-line arguments.
 * @param ppbszArgument Array of pointers to the command-line argument strings.
 * @param process_ A function to process each command-line argument, taking a string view and a variant view, and returning a boolean indicating success.
 * @return A pair consisting of a boolean indicating success or failure, and a string containing an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{
   std::string stringApplicationFolder;   // folder where application is stored
   std::string stringRootFolder;          // root folder for site
   char pbszPathBuffer[MAX_PATH];         // buffer to store path


   // ## Store application folder
#if defined(_MSC_VER)
   ::GetModuleFileNameA( nullptr, pbszPathBuffer, MAX_PATH );
   stringApplicationFolder = pbszPathBuffer;
#else
   stringApplicationFolder = ppbszArgument[0];
#endif
   auto position_ = stringApplicationFolder.find_last_of("\\/");
   if( position_ != std::string::npos ) { stringApplicationFolder = stringApplicationFolder.substr( 0, position_ + 1 ); }
                                                                                                   assert( std::filesystem::exists( stringApplicationFolder ) == true && "Application folder need to be valid!" );


   papplication_g->PROPERTY_Add("folder-application", stringApplicationFolder);

   gd::cli::options optionsApplication;
   Prepare_s(optionsApplication);

   // ## Initialize application
   auto result_ = Initialize();                                                                    assert( result_.first );

   // ## Parse arguments if sent
   if( iArgumentCount > 1 )											           // do we have arguments ? (first is application)
   {
       auto result_ = optionsApplication.parse(iArgumentCount, ppbszArgument); // @BOOKMARK [title: options] [description: parse command line arguments] 
      if( result_.first == false ) { return result_; }

      result_ = Read_s(this, optionsApplication);
      if( result_.first == false ) { return result_; }

      // ## Check active command and execute
      auto poptionsActive = optionsApplication.sub_find_active();
      if( poptionsActive != nullptr )
      {
         result_ = Configure( optionsApplication );                                                if( result_.first == false ) { return result_; }

         result_ = Execute( *poptionsActive);
         if( result_.first == false ) { return result_; }
      }
   }

   return application::basic::CApplication::Main( iArgumentCount, ppbszArgument, nullptr );
}


/** --------------------------------------------------------------------------- @CODE [tag: initialize] [description: Initialize application]
 * @brief Initializes the application by setting up base data, reading configuration files, and configuring logging settings.
 * @return A pair consisting of a boolean indicating success or failure, and a string containing an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Initialize()
{
   { // ## Initialize base data used in application
      using namespace gd::table::arguments;
      unsigned uTableFlags = table::eTableFlagNull32|table::eTableFlagArguments;

      // Create table with columns : key (uint32), ip (string 32), root (rstring)
      m_ptableSite = std::make_unique<gd::table::arguments::table>(uTableFlags, 
         std::initializer_list< std::tuple< std::string_view, unsigned, std::string_view > >( { { "uint32", 0u, "key"  }, { "string", 32u, "ip"}, { "uint32", 0, "port"}, { "rstring", 0u, "root"} } ), gd::table::tag_prepare{}); 
   }

   {  // ## Read configuration file
      std::string stringConfigurationFile = papplication_g->PROPERTY_Get("configuration").as_string();

      if( stringConfigurationFile.empty() == true )
      {
         // ## try to find configuration file in application folder
         gd::file::path pathFolderApplication(papplication_g->PROPERTY_Get("folder-application").as_string());
         pathFolderApplication += "configuration.xml";
      }

      if( stringConfigurationFile.empty() == false )
      {
         if( std::filesystem::exists(stringConfigurationFile) == true )
         {
            auto result_ = CONFIGURATION_Read(stringConfigurationFile);
            if( result_.first == false ) { return result_; }
         }
         else
         {
            return std::pair<bool, std::string>( false, "Configuration file '" + stringConfigurationFile + "' does not exist!" );
         }
      }
   } // ## end read configuration file

   // ## Configure log settings
   {
      using namespace gd::log;
      gd::log::logger<0>* plogger = gd::log::get_s();                          // get pointer to logger 0
      std::string stringLogFile = papplication_g->PROPERTY_Get("folder-log").as_string();
      std::string stringDate = gd::file::rotate::backup_history::date_now_s();
      // replace dashes in string
      std::replace( stringDate.begin(), stringDate.end(), '-', '_');
      stringLogFile += stringDate;
      stringLogFile += ".log";
      papplication_g->PROPERTY_Set("file-log", stringLogFile);
#ifndef NDEBUG
#     ifdef _WIN32
      //plogger->append( std::make_unique<gd::log::printer_console>() );         // append printer to logger, this prints to console
#     endif
      plogger->append( std::make_unique<gd::log::printer_console>() );         // append printer to logger, this prints to console
      // ## set margin for log messages, this to make it easier to read. a bit hacky 
      auto* pprinter_console = (gd::log::printer_console*)plogger->get( 0 );
      // ## color console messages in debug mode
      pprinter_console->set_margin( 8 );                                       // set log margin
      pprinter_console->set_margin_color( eColorBrightBlack );

      plogger->append( std::make_unique<gd::log::printer_file>(stringLogFile) );// append printer to logger, prints to file
#else
      plogger->set_severity( unsigned(eSeverityNumberVerbose) | unsigned(eSeverityGroupDebug) );   // set severity filter, messages within this filter is printed

      // ## if logging or ignore-error option is set then turn on console logging
      //if( papplication_g->PROPERTY_Get( "log-console" ).is_null() == false || papplication_g->PROPERTY_Get( "ignore-error" ).is_true() )
      if( papplication_g->PROPERTY_Get( { "log-console", "ignore-error" } ).is_null() == false  )
      {
         plogger->append( std::make_unique<gd::log::printer_console>() );      // append printer to logger, this prints to console
         // ## set margin for log messages, this to make it easier to read. a bit hacky 
         auto* pprinter_console = (gd::log::printer_console*)plogger->get( 0 );
         // ## color console messages in debug mode
         pprinter_console->set_margin( 8 );                                    // set margin for 
         pprinter_console->set_margin_color( eColorBrightBlack );

         unsigned uSeverity = unsigned(eSeverityNumberVerbose) | unsigned(eSeverityGroupDebug);
         if( papplication_g->PROPERTY_Get( "log-console" ).is_null() == false )
         {
            uSeverity = papplication_g->PROPERTY_Get( "log-console" ).as_uint();
            if((uSeverity & 0xff) >= eSeverityNumberMAX)
            {
               return { false, "Log console severity level is invalid, max level is " + std::to_string( eSeverityNumberMAX - 1 ) };
            }
         }
      }

      plogger->append( std::make_unique<gd::log::printer_file>(stringLogFile) );// append printer to logger, prints to file
#endif

#ifndef NDEBUG
      unsigned uSeverity = unsigned(eSeverityNumberVerbose) | unsigned(eSeverityGroupDebug);
#else
      unsigned uSeverity = unsigned(eSeverityNumberVerbose) | unsigned(eSeverityGroupDebug);
#endif
      plogger->set_severity( uSeverity );                                      // set severity filter, messages within this filter is printed

      if( papplication_g->PROPERTY_Get( "log-level" ).is_null() == false )
      {
         unsigned uSeverityLevel = papplication_g->PROPERTY_Get( "log-level" ).as_uint();
         plogger->set_severity_Level( uSeverityLevel );                        // set severity filter level
      }
   }

   // ## create main document .................................................
   //    this holds main application data

   auto pdocumentMain = std::make_unique<CDocument>( this );
   m_pdocumentActive = pdocumentMain.get(); 
   m_vectorDocument.push_back( std::move( pdocumentMain ) );

   // ## create server boost object to handle http requests

   m_pserverBoost = new CServer(this);                                        // create server object, used to handle and isolate http requests

   /*
   // ## Add default servers to router
   CHttpServer* phttpserver = new CHttpServer;
   
   auto result_ = phttpserver->Initialize();                                   // initialize http server, connect all routes
   if ( result_.first == false )
   {
      phttpserver->release();   
      return result_;
   }

   m_phttpserver = phttpserver;                                               // set http server
   */

   { // ## add main site if values are set ....................................
      std::string stringIp = papplication_g->PROPERTY_Get("ip").as_string(); // get ip address
      uint32_t uPort = papplication_g->PROPERTY_Get("port").as_uint(); // get port number
      std::string stringRoot = papplication_g->PROPERTY_Get("folder-root").as_string(); // get root folder
      if( stringRoot.empty() == false )
      {
         if( uPort == 0 ) uPort = 80;
         if( stringIp.empty() == true ) stringIp = "127.0.0.1";
         SITE_Add( stringIp, uPort, stringRoot );
      }
   }

   return application::basic::CApplication::Initialize();
}

/** ---------------------------------------------------------------------------
* @brief call this before application is exited, place last cleanup in this
* @return true if ok, false and error information on error
*/
std::pair<bool, std::string> CApplication::Exit()
{
   CDTOResponse::Destroy_s();                                                 // Destroy the CDTOResponse static objects

   return application::basic::CApplication::Exit();
}

/** --------------------------------------------------------------------------- @API [tag: options, configure] [description: Configure application from command-line options]
 * @brief Configure application specific options
 * @param optionsApplication The command-line options to configure.
 * @return A pair consisting of a boolean indicating success or failure, and a string containing an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Configure(const gd::cli::options& optionsActive)
{
   std::string stringCommand = optionsActive.name();                                               assert( stringCommand.empty() == false );



   return std::pair<bool, std::string>();
}

std::pair< bool, std::string > CApplication::Execute( gd::cli::options& optionsCommand )
{
   std::string stringCommandName = optionsCommand.name(); 

   if( stringCommandName == "http" )
   {
      auto stringIp = optionsCommand["ip"].as_string();
      if( stringIp.empty() == false ) { PROPERTY_Set("ip", stringIp ); }
      else { stringIp = PROPERTY_Get( "ip" ).as_string(); }

      auto uPort = optionsCommand["port"].as_uint();
      if( uPort != 0 ) { PROPERTY_Set("port", uPort ); }
      else { uPort = PROPERTY_Get( "port" ).as_uint(); }

      auto stringSite = optionsCommand["site"].as_string();
      if( stringSite.empty() == false ) { PROPERTY_Set( "folder-root", stringSite ); }
      else { stringSite = PROPERTY_Get( "folder-root" ).as_string(); }



      SITE_Add( stringIp, uPort, stringSite );
      
      // ## add site
      /*
      std::string stringRoot = papplication_g->PROPERTY_Get("folder-root").as_string();
      if( stringRoot.empty() == false )
      {
         SITE_Add( stringIp, uPort, stringRoot );
      }
      */

      return SERVER_Start( 0 );
   }

   return std::pair<bool, std::string>( true, "" );
}

/** --------------------------------------------------------------------------- @API [tag: server] [summary: start server] [description: Start the web server] [type: member method]
 * @brief Start the web server
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> CApplication::SERVER_Start(unsigned uIndex)
{
   assert(uIndex < m_ptableSite->size() && "You need at least one site to start server");
   unsigned short uPort = 80;
   std::string stringDefaultIp("127.0.0.1");
   std::string stringIp;
   std::string stringRoot;

   // ## site
   uPort = (unsigned short)m_ptableSite->cell_get_variant_view(uIndex, "port").as_uint();
   stringIp = m_ptableSite->cell_get_variant_view(uIndex, "ip").as_string();
   stringRoot = m_ptableSite->cell_get_variant_view(uIndex, "root").as_string();


   // ## Prepare ip address

   if (PROPERTY_Get("ip").empty() == false) { stringIp = papplication_g->PROPERTY_Get("ip").as_string(); }
   if( stringIp.empty() == true) { stringIp = stringDefaultIp; }

   // ## Prepare root folder for site on local disk
   std::string stringRootFolder = FOLDER_GetRoot_s( "temp__/" );
   if( PROPERTY_Get("folder-root").empty() == false ) stringRootFolder = papplication_g->PROPERTY_Get("folder-root").as_string();

   unsigned uThreadCount = 4;
   if( PROPERTY_Get("system-treadcount").empty() == false ) uThreadCount = papplication_g->PROPERTY_Get("system-treadcount").as_uint();

#ifndef NDEBUG
   LOG_INFORMATION_RAW("== Starting server in DEBUG mode ==");
   auto ptable_d = PROPERTY_ToTable();
   auto stringTable = gd::table::to_string(*ptable_d, gd::table::tag_io_cli{});
   LOG_INFORMATION_RAW( stringTable );
#endif

   int iVersion = 11;
   boost::asio::io_context iocontext_( uThreadCount );

                                                                                                   assert( stringIp.empty() == false && "IP address can not be empty!" );

   const boost::asio::ip::address addressIP = net::ip::make_address(stringIp);
   auto const doc_root = std::make_shared<std::string>(stringRootFolder);
                                                                                                   LOG_DEBUG_RAW("Starting server on http://" & stringIp + ":" & uPort & " with root folder '" & stringRootFolder & "'");
   // Create and launch a listening port
   std::make_shared<listener>( iocontext_,  tcp::endpoint{addressIP, uPort}, doc_root)->run();

   std::vector<std::thread> vectorThread;
   vectorThread.reserve(uThreadCount - 1);
   for(auto i = uThreadCount - 1; i > 0; --i)
      vectorThread.emplace_back(
         [&iocontext_]
         {
            iocontext_.run();
         });
   iocontext_.run();

   return { true, "" };
}




/** --------------------------------------------------------------------------- @API [tag: options] [title: configure options] [description: Prepare application specific arguments]
 * @brief Prepare application specific arguments
 */
void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   // ## Log settings
   optionsApplication.add_flag( {"logging", "Turn on logging"} );              // logging is turned on using this flag
   optionsApplication.add_flag( {"logging-csv", "Add csv logger, prints log information using the csv format"} );

   optionsApplication.add({"configuration", "File with configuration settings for web server, json or xml file"});
   
   optionsApplication.add({"logging-severity", "Set specific severity for logger, severity acts as a filter"});
   optionsApplication.add({"logging-tags", "set active log-tags to filter log messages"});
   optionsApplication.add({"logging-show", "Default is to log messages (non tagged), with this you can turn them on or off. If setting tag/tags then this turns off if not set to be on, value 0|1"});

   optionsApplication.add({ "port", "Set port number" });

   // ## Folder settings
   optionsApplication.add({"path", "Global path variable used to find files in any of the folders if not found in selected folder, folders are separated by semicolon"});
   optionsApplication.add({"folder-configuration", "Folder where to read configuration files"});
   optionsApplication.add({"folder-logging", "Set folder where logger places log files"});

   {  // ## `http` command, manage settings for http server
      gd::cli::options optionsCommand( 0, "http", "Webserver configuration" );
      optionsCommand.add({ "ip", "IP address to bind the server to" });
      optionsCommand.add( { "port", "Port number to bind the server to" } );
      optionsCommand.add( { "site", "Folder on disk where to find files"});
      optionsCommand.add( { "add-session", "Adds session values at start, usefull for testing"});
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }
}

/** --------------------------------------------------------------------------- @API [tag: options] [title: read options] [description: Read command-line options]
 * @brief Reads and processes application options, updating application properties based on the provided command-line options.
 * @param papplication_ Pointer to the CApplication instance whose properties will be updated.
 * @param optionsApplication Reference to the root options object containing command-line arguments and sub-options.
 * @return A pair consisting of a boolean indicating success (always true in this implementation) and a string containing an error message (empty if successful).
 */
std::pair<bool, std::string> CApplication::Read_s(CApplication* papplication_, gd::cli::options& optionsApplication)
{
   const auto* poptions_ = optionsApplication.sub_find_active();               // find active sub parser for arguments
   if( poptions_ == nullptr ) poptions_ = &optionsApplication;                 // set to root options

   // Set path property, it holds paths to folders that may be used as path in os system
   if( (*poptions_)["path"].is_true() == true ) { papplication_->PROPERTY_Add( "path", (*poptions_)["path"].as_string() ); }

   poptions_->iif( std::string_view("folder-configuration"), [papplication_]( const std::string_view& s_ ) { papplication_->PROPERTY_Add( "folder-configuration", s_ ); } );
   poptions_->iif( std::string_view("configuration"), [papplication_]( const std::string_view& s_ ) { papplication_->PROPERTY_Add( "configuration", s_ ); } );

   if(optionsApplication["port"].is_true() == true)
   {
      int iPort = optionsApplication["port"].as_int();
      if (iPort < 1 || iPort > 65535) { return { false, "Port number must be between 1-65535" }; }
      papplication_->PROPERTY_Add("port", iPort);
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------- @API [tag: database, open] [description: Open database connection based on arguments]
 * @brief Open database connection based on arguments
 * 
 * @param argumentsOpen The arguments used to open the database connection.
 *  
 */
std::pair<bool, std::string> CApplication::OpenDatabase_s(const gd::argument::arguments& argumentsOpen, gd::database::database_i*& pdatabase_)
{
   std::string stringType = argumentsOpen["type"].as_string();
   if(stringType == "sqlite")
   {
      std::string stringName = argumentsOpen["name"].as_string();
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i("sqlite");  // create database interface for sqlite
      auto [bOk, stringError] = pdatabase->open(stringName);              // open sqlite database
      if(bOk == true)
      {
         pdatabase_ = pdatabase;
         pdatabase_->set("dialect", "sqlite");                            // only sqlite is file database, we can set the dialect directly
      }
      else return { bOk, stringError };
   }

   return { true, "" };
}


/// ---------------------------------------------------------------------------
/// Set active database based on name or index
void CApplication::DATABASE_SetActive(const std::variant<std::size_t, std::string_view>& index_) 
{
   std::lock_guard<std::mutex> lock(m_mutexDatabase);                          // thread safety
   DATABASE_SetNull();

   if (std::holds_alternative<std::size_t>(index_)) 
   {  
      std::size_t uIndex = std::get<std::size_t>(index_);                                          assert(uIndex < m_vectorDatabase.size());
      // Set the active database by index (e.g., store the index)
      m_pdatabase = m_vectorDatabase[uIndex];
      m_pdatabase->add_reference();
   } 
   else if( std::holds_alternative<std::string_view>(index_) ) 
   {
      std::string_view stringName = std::get<std::string_view>(index_);
      for (std::size_t u = 0; u < m_vectorDatabase.size(); u++ ) 
      {
         if(  m_vectorDatabase[u]->name() == stringName)
         { 
            m_pdatabase = m_vectorDatabase[u];                                 // Set the active database by name
            m_pdatabase->add_reference();
            break; 
         }
      }
      // Handle the case where no database with the given name is found
   }
}


/** ---------------------------------------------------------------------------
 * @brief Read configuration file, reads settings from xml file
 * 
 * Format for xml:
 * <configuration>
 *    <properties>
 *       <property key="port" value="8080"/>
 *       <property key="folder-root" value="C:\dev\home\DOD\www"/>
 *       <property key="log_file" value="C:\dev\home\DOD\logs\http.log"/>
 *    </properties>
 *</configuration>
 */
std::pair<bool, std::string> CApplication::CONFIGURATION_Read( const std::string_view& stringFileName )
{
   using namespace pugi;
   xml_document xmldocument;   // xml document used to load xml from file

   if( std::filesystem::exists( stringFileName ) == false ) return { true, "" };

   // ## Load xml data
   xml_parse_result xmlparseresult = xmldocument.load_file( stringFileName.data() );
   if( (bool)xmlparseresult == false ) return { false, xmlparseresult.description() };

   {
      // ## get all properties nodes found in xml
      auto xmlnodeProperties = xmldocument.document_element().child( "properties" );
      while( xmlnodeProperties.empty() == false )
      {
         pugi::xml_node xmlnode = xmlnodeProperties.first_child();
         while( xmlnode.empty() == false )
         {
            // ## check for property value, if found read value and store in application
            if( std::string_view( xmlnode.name() ) == "property" )
            {
               const char* pbszKey = xmlnode.attribute( "key" ).value();
               const char* pbszValue = xmlnode.attribute( "value" ).value();

               if( *pbszKey != '\0' && *pbszValue != '\0' )
               {
                  if( PROPERTY_Has( pbszKey ) == false )                      // check if property is added, if not then it is ok to add from configuration
                  {
                     PROPERTY_Add( pbszKey, pbszValue );                      // add property
                  }
               }
            }
            xmlnode = xmlnode.next_sibling();
         }

         xmlnodeProperties = xmlnodeProperties.next_sibling( "properties" );
      }
   }
                                                                                                   LOG_DEBUG_RAW( "Configuration read from file: " & stringFileName );
   return { true, "" };
}

void CApplication::SITE_Add(std::string_view stringIp, uint32_t uPort, std::string_view stringFolder)
{                                                                                                  LOG_DEBUG_RAW( "Add site - ip: " & stringIp & " port: " & uPort & " directory: " & stringFolder);
   auto uRow = m_ptableSite->row_add_one();
   m_ptableSite->row_set(uRow, gd::table::tag_variadic{}, gd::table::tag_convert{}, uRow + 1, stringIp, uPort, stringFolder);
}

/** --------------------------------------------------------------------------- @API [tag: configuration, load] [description: Load configuration from a JSON file into the config table]
 * @brief Load configuration from a JSON file into the config table
 *
 * @param stringFileName The path to the configuration file
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::CONFIG_Load(const std::string_view& stringFileName)
{                                                                                                 assert( (bool)m_ptableConfig == false );
   using namespace jsoncons;
   using namespace gd::table;

   if( m_ptableConfig != nullptr ) return { true, "" }; // If config table is already set, return success

   constexpr std::string_view stringConfigurationFileName = ".http-configuration.json"; // Default configuration file name

   std::string stringFolder( stringFileName );


   // ## Try to find local configuration file


   if( stringFolder.empty() == true )
   {
      stringFolder = PROPERTY_Get("folder-home").as_string();                 // Get home folder from properties
   }

   if( stringFolder.empty() == true ) return { false, "No home folder set" }; // If no home folder is set, return error

   // ## Prepare configuration path
   gd::file::path pathConfiguration(stringFolder);
   if( pathConfiguration.has_extension() == false ) { pathConfiguration += stringConfigurationFileName; } // Add filename if not provided

   if( std::filesystem::exists(pathConfiguration) == false ) { return { false, std::format("configuration file not found: {}", pathConfiguration.string()) }; } // Check if configuration file exists

   // ## Load settings from file
   try
   {
      // Create a new config table with the path to the configuration file
      m_ptableConfig = std::make_unique<table>( table( table::eTableFlagNull32, {{"rstring", 0, "group"}, {"rstring", 0, "name"}, {"rstring", 0, "value"}, {"string", 6, "type"} }, tag_prepare{} ) );
      // Open the JSON file
      std::ifstream ifstreamJson(pathConfiguration);
      if( ifstreamJson.is_open() == false ) { return { false, std::format("Failed to open configuration file: {}", pathConfiguration.string()) }; } // Check if file opened successfully

      // Parse JSON data from the file into a json object
      json jsonDocument = json::parse( ifstreamJson );

      // ## Iterate through the JSON object and populate the config table
      for( const auto& keyvalueRoot : jsonDocument.object_range() )
      {
         if( keyvalueRoot.value().is_object() == false ) continue;            // Skip if value is not an object

         std::string stringKey = keyvalueRoot.key();

         // ## split string between group and name, splitting by '.' and set stringGroup and stringName

         auto vectorSplit = gd::utf8::split(stringKey, '.');                  // Split the string by '.'
         if( vectorSplit.size() < 2 ) continue;                               // Skip if less than 2 parts after splitting

         auto stringCleaner = vectorSplit[0];                                 // First part is the 'cleaner'  marker, all config values belongs to key objects where first part is "cleaner"

         if( stringCleaner != "cleaner" ) continue;                           // Skip if group is not "cleaner"

         auto stringGroup = vectorSplit[1];                                   // Second part is the group name
         for( const auto& value_ : keyvalueRoot.value().object_range() )
         {
            if( value_.value().is_null() ) continue;                          // Skip if value is null

            auto stringName = value_.key();

            if( value_.value().is_array() == true )
            {
               // ## Handle array values
               gd::argument::shared::arguments argumentsArray;
               for( const auto& arrayValue : value_.value().array_range() )
               {
                  argumentsArray.append( arrayValue.as_string() );            // Convert each array element to string and add to arguments
               }
               CONFIG_HandleArray(stringGroup, stringName, argumentsArray);
               continue;
            }

            auto stringValue = value_.value().as_string();                    // Convert JSON value to string
            auto uRow = m_ptableConfig->row_add_one();
            m_ptableConfig->row_set(uRow, { stringGroup, stringName, stringValue }); // Set value, each value is store as string and belongs to group and name
         }
      }

      // Use the json object (e.g., print it)
      //std::cout << pretty_print(j) << std::endl;
   }
   catch (const std::exception& e)
   {
      std::string stringError = std::format("Error: {}", e.what());
      return { false, stringError };
   }

   return { true, "" }; // Placeholder for settings loading logic
}

/** ---------------------------------------------------------------------------
 * @brief Get configuration value from the config table
 *
 * @param stringGroup The group name of the configuration
 * @param stringName The name of the configuration
 * @return gd::variant_view The value of the configuration, or an empty variant view if not found
 */
gd::variant_view CApplication::CONFIG_Get(std::string_view stringGroup, std::string_view stringName) const
{
   if( m_ptableConfig == nullptr ) return gd::variant_view(); // If no config table is set, return empty variant view

   auto iRow = m_ptableConfig->find({ {"group", stringGroup }, {"name", stringName} }); // Find the row with the specified group and name

   if( iRow != -1 )                                                           // If row is found
   {
      auto value_ = m_ptableConfig->cell_get_variant_view(iRow, "value");     // Return the value from the found row
      return value_;
   }

   return gd::variant_view();
}

void CApplication::CONFIG_Set(std::string_view stringGroup, std::string_view stringName, const gd::variant_view& value_)
{
}

/** ---------------------------------------------------------------------------
 * @brief Check if configuration exists in the config table
 *
 * @param stringGroup The group name of the configuration
 * @param stringName The name of the configuration
 * @return bool True if the configuration exists, false otherwise
 */
bool CApplication::CONFIG_Exists( std::string_view stringGroup, std::string_view stringName ) const
{
   if( m_ptableConfig == nullptr ) return false;

   auto iRow = m_ptableConfig->find({ {"group", stringGroup }, {"name", stringName} }); // Find the row with the specified group and name
   if( iRow != -1 ) { return true; }

   return false;
}

/** ---------------------------------------------------------------------------
 * @brief Handles array values in the configuration file by converting them to comma-separated strings.
 *
 * This function is called when a configuration value is an array. It converts each array element
 * to a string and joins them with commas, storing the result as a single string in the configuration table.
 *
 * @param stringGroup The group name of the configuration
 * @param stringName The name of the configuration parameter
 * @param arguments_ The arguments containing the array values to process
 */
void CApplication::CONFIG_HandleArray( std::string_view stringGroup, std::string_view stringName, const gd::argument::shared::arguments& arguments_ )
{
   for( const auto& argument : arguments_ )
   {
      // Convert each array element to a string and join them with commas
      std::string stringValue = argument.as_string();

      if( stringValue.empty() == false )
      {
         CONFIG_Set( stringGroup, stringName, stringValue );
         continue;
      }
   }
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
 * @brief Retrieves a document by its name, creating it if it doesn't exist.
 *
 * @param stringName The name of the document to retrieve or create.
 * @param bCreate Whether to create the document if it doesn't exist and bCreate is true.
 * @return CDocument* Pointer to the document if found or created, otherwise nullptr.
 */
CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName, bool bCreate)
{
   auto pdocument = DOCUMENT_Get(stringName);
   if( pdocument == nullptr && bCreate == true )
   {
      pdocument = DOCUMENT_Add(stringName);
   }
   return pdocument;
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


/** --------------------------------------------------------------------------- @TAG #print
 * @brief Prints a message to the console or other output based on the UI type.
 *
 * This method handles printing messages to different output targets based on the application's UI type.
 * It supports console, web, WIMP (Windows, macOS, Linux), Visual Studio Code, and Sublime Text.
 *
 * @param stringMessage The message to print.
 * @param argumentsFormat Optional formatting arguments for the message.
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CApplication::PrintMessage(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat)
{
   // Optionally extract formatting options from argumentsFormat
   // For example: severity, color, output target, etc.
   // Example: std::string_view severity = argumentsFormat["severity"].as_string_view();

   std::unique_lock<std::mutex> lock_( m_mutex );

   return {true, ""};
}

/**
 * @brief Prints a progress message to the console or other output based on the UI type.
 */
std::pair<bool, std::string> CApplication::PrintProgress(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat) // @TAG #progress.Application
{
   std::unique_lock<std::mutex> lock_( m_mutex );


   return {true, ""};
}

std::pair<bool, std::string> CApplication::PrintError(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat)
{
   std::cout << "\n##\n## ERROR \n## ------\n" << stringMessage << std::flush;
   return {true, ""};
}

void CApplication::Print( std::string_view stringColor,  gd::types::tag_background )
{
   std::string stringColorCode = CONFIG_Get("color", stringColor).as_string();
   if( stringColorCode.empty() == false )
   {
      stringColorCode = gd::console::rgb::print(stringColorCode, gd::types::tag_background{});
      std::cout << stringColorCode;                                           // Print the color code before the message
      std::cout << "\033[2J";
      std::cout << "\033[H";
   }
   else
   {
      // ## Reset all attributes and clear the screen to return to the default state
      std::cout << "\033[0m";
   }
}







/** ---------------------------------------------------------------------------
 * @brief Add error to internal list of errors
 * @param stringError error information
 */
void CApplication::ERROR_Add( const std::string_view& stringError )
{
   std::unique_lock<std::mutex> lock_( m_mutexError ); // locks `m_vectorError`
   gd::argument::arguments argumentsError( { {"text", stringError} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsError) );
}

/** ---------------------------------------------------------------------------
 * @brief Add warning to internal list of errors
 * @param stringWarning warning information
 */
void CApplication::ERROR_AddWarning( const std::string_view& stringWarning )
{
   if( PROPERTY_Exists( "verbose" ) == false || PROPERTY_Get( "verbose" ).as_bool() == false ) return; // only add warning in verbose mode
   std::unique_lock<std::mutex> lock_( m_mutexError ); // locks `m_vectorError`
   gd::argument::arguments argumentsWarning( { {"text", stringWarning} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsWarning) );
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





/** ---------------------------------------------------------------------------
* @brief Walk upp the folder tree and try to find folder containing file
* @param stringSubfolder add this folder to found root folder, if empty then root folder is returned
* @return std::string root folder name
*/
std::string CApplication::FOLDER_GetRoot_s( const std::string_view& stringSubfolder )
{
   std::filesystem::path pathCurrentDirecotry = std::filesystem::current_path();
   auto [bFound, stringRootFolder] = gd::file::closest_having_file_g( pathCurrentDirecotry.string(), ROOT_MARKER );

   if( bFound == true ) stringRootFolder += stringSubfolder;

   std::filesystem::path path_( stringRootFolder );
   stringRootFolder = path_.make_preferred().string();
   return stringRootFolder;
}
