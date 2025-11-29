#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <queue>
#include <thread>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>

#include "pugixml/pugixml.hpp"

#include "gd/gd_file.h"
#include "gd/gd_file_rotate.h"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"



#if defined(_MSC_VER)
#   include "windows.h"   
#endif


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
               std::cout << "ERROR: `log-console` Sverity value 0-6 is allowed, Not " << int(uSeverity & 0xff) << "\n";
               return 1;
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

   m_pserverBoost = new CServer(this);                                        // create server object, used to handle and isolate http requests

   // ## Add default servers to router
   CHttpServer* phttpserver = new CHttpServer;
   
   auto result_ = phttpserver->Initialize();                                   // initialize http server, connect all routes
   if ( result_.first == false )
   {
      phttpserver->release();   
      return result_;
   }

   m_phttpserver = phttpserver;                                               // set http server

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
   return application::basic::CApplication::Exit();
}

/** --------------------------------------------------------------------------- @API [tag: options, configure] [description: Configure application from command-line options]
 * @brief Configure application specific options
 * @param optionsApplication The command-line options to configure.
 * @return A pair consisting of a boolean indicating success or failure, and a string containing an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Configure(const gd::cli::options& optionsApplication)
{



   return std::pair<bool, std::string>();
}

std::pair< bool, std::string > CApplication::Execute( gd::cli::options& optionsCommand )
{
   std::string stringCommandName = optionsCommand.name(); 

   if( stringCommandName == "http" )
   {
      auto stringIp = optionsCommand["ip"].as_string();
      if( stringIp.empty() == false ) { PROPERTY_Set("ip", stringIp ); }
      auto uPort = optionsCommand["port"].as_uint();
      if( uPort != 0 ) { PROPERTY_Set("port", uPort ); }
      
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

/** --------------------------------------------------------------------------- @API [tag: server] [summary: start server] [description: Start the web server]
 * @brief Start the web server
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> CApplication::SERVER_Start( unsigned uIndex )
{                                                                                                  assert( uIndex < m_ptableSite->size() && "You need at least one site to start server");
   unsigned short uPort = 80;
   std::string stringIp("127.0.0.1");
   std::string stringRoot;

   // ## site
   uPort = (unsigned short)m_ptableSite->cell_get_variant_view( uIndex, "port" ).as_uint();
   stringIp = m_ptableSite->cell_get_variant_view(uIndex, "ip").as_string();
   stringRoot = m_ptableSite->cell_get_variant_view(uIndex, "root").as_string();


   // ## Prepare ip address
   
   if( PROPERTY_Get("ip").empty() == false ) stringIp = papplication_g->PROPERTY_Get("ip").as_string();

   // ## Prepare root folder for site on local disk
   std::string stringRootFolder = FOLDER_GetRoot_g( "temp__/" );
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
{
   auto uRow = m_ptableSite->row_add_one();
   m_ptableSite->row_set(uRow, gd::table::tag_variadic{}, gd::table::tag_convert{}, uRow + 1, stringIp, uPort, stringFolder);
}





/** ---------------------------------------------------------------------------
* @brief Walk upp the folder tree and try to find folder containing file
* @param stringSubfolder add this folder to found root folder, if empty then root folder is returned
* @return std::string root folder name
*/
std::string FOLDER_GetRoot_g( const std::string_view& stringSubfolder )
{
   std::filesystem::path pathCurrentDirecotry = std::filesystem::current_path();
   auto [bFound, stringRootFolder] = gd::file::closest_having_file_g( pathCurrentDirecotry.string(), ROOT_MARKER );

   if( bFound == true ) stringRootFolder += stringSubfolder;

   std::filesystem::path path_( stringRootFolder );
   stringRootFolder = path_.make_preferred().string();
   return stringRootFolder;
}


