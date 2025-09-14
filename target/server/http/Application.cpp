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

   papplication_g->PROPERTY_Add("folder-application", stringApplicationFolder);

   gd::cli::options optionsApplication;
   Prepare_s(optionsApplication);

   // ## Parse arguments if sent
   if( iArgumentCount > 1 )											           // do we have arguments ? (first is application)
   {
	   auto [bOk, stringError] = optionsApplication.parse(iArgumentCount, ppbszArgument); // @BOOKMARK [title: options] [description: parse command line arguments] 
      if( bOk == false ) { return { bOk, stringError }; }
   }



   int iResult = Main_s( iArgumentCount, ppbszArgument );

   // ## Process the command-line arguments
   auto [bOk, stringError] = Initialize(optionsApplication);
   if (bOk == false) { return { false, stringError }; }


   return application::basic::CApplication::Main( iArgumentCount, ppbszArgument, nullptr );
}


std::pair<bool, std::string> CApplication::Initialize()
{
   {
      gd::file::path pathFolderApplication( papplication_g->PROPERTY_Get("folder-application").as_string() );
      pathFolderApplication += "configuration.xml";
      if( std::filesystem::exists( pathFolderApplication ) == true )
      {
         auto result_ = CONFIGURATION_Read(pathFolderApplication);
         if( result_.first == false ) { return result_; }
      }

   }

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

std::pair<bool, std::string> CApplication::Configure(const gd::cli::options& optionsApplication)
{
   return std::pair<bool, std::string>();
}

/** ---------------------------------------------------------------------------
 * @brief Start the web server
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> CApplication::SERVER_Start()
{
   unsigned short uPort = 8080;
   std::string stringPort("8080");

   // ## Prepare ip address
   std::string stringIp("127.0.0.1");
   if( PROPERTY_Get("ip").empty() == false ) stringIp = papplication_g->PROPERTY_Get("ip").as_string();

   // ## Prepare root folder for site on local disk
   std::string stringRootFolder = FOLDER_GetRoot_g( "temp__/" );
   if( PROPERTY_Get("folder-root").empty() == false ) stringRootFolder = papplication_g->PROPERTY_Get("folder-root").as_string();

   unsigned uThreadCount = 4;
   if( PROPERTY_Get("system-treadcount").empty() == false ) uThreadCount = papplication_g->PROPERTY_Get("system-treadcount").as_uint();

   int iVersion = 11;
   boost::asio::io_context iocontext_( uThreadCount );

   auto const address = net::ip::make_address(stringIp);
   auto const doc_root = std::make_shared<std::string>(stringRootFolder);

   // Create and launch a listening port
   std::make_shared<listener>( iocontext_,  tcp::endpoint{address, uPort}, doc_root)->run();

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


/** ------------------------------------------------------------------------
* @brief 
* @param iArgumentCount 
* @param ppbszArgument 
* @return 
* https://www.boost.org/doc/libs/1_87_0/libs/beast/example/advanced/server/advanced_server.cpp
*/
int CApplication::Main_s(int iArgumentCount, char* ppbszArgument[])
{

   /*
   // Launch the client
   std::make_shared<session>(iocontext_)->run(stringIp.c_str(), stringPort.c_str(), stringRootFolder.c_str(), iVersion);
   iocontext_.run();
   */


/*
   auto const paddressIp = boost::asio::ip::make_address(stringIp);
   auto const pstringRoot = std::make_shared<std::string>(stringRootFolder);

   boost::asio::io_context iocontext_(uThreadCount);

   // Create and launch a listening port
   std::make_shared<listener>(state, tcp::endpoint{address, port})->run();


   // Create and launch a listening port
   std::make_shared<listener>( iocontext_, boost::asio::ip::tcp::endpoint{paddressIp, uPort}, pstringRoot)->run();

   // ## Capture SIGINT and SIGTERM to perform a clean shutdown
   boost::asio::signal_set signalset_(iocontext_, SIGINT, SIGTERM);
   signalset_.async_wait( [&](boost::beast::error_code const&, int) {
      // Stop the `io_context`. This will cause `run()`
      // to return immediately, eventually destroying the
      // `io_context` and all of the sockets in it.
      iocontext_.stop();
      });


   // ## Run the I/O service on the requested number of threads
   std::vector<std::thread> vectorThread;
   vectorThread.reserve(uThreadCount);
   for( auto itIndex = 0; itIndex < uThreadCount; itIndex++ )
   {
      vectorThread.emplace_back( [&iocontext_] {
         return iocontext_.run();
         });
   }

   // (If we get here, it means we got a SIGINT or SIGTERM)

   // Block until all the threads exit
   for(auto& itThread : vectorThread) { itThread.join(); }
   */

   return 0;
}

std::pair<bool, std::string> CApplication::Initialize(gd::cli::options& optionsApplication)
{
    // ## Read port number if found
	if(optionsApplication["port"].is_true() == true)
	{
		int iPort = optionsApplication["port"].as_int();
		if (iPort < 1 || iPort > 65535) { return { false, "Port number must be between 1-65535" }; }
		PROPERTY_Add("port", iPort);                                                               LOG_INFORMATION_RAW("== Port number: " & iPort);
	}

    return { true, std::string() };
}


/** --------------------------------------------------------------------------- @BOOKMARK [title: options] [description: Prepare application specific arguments]
 * @brief Prepare application specific arguments
 */
void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   // ## Log settings
   optionsApplication.add_flag( {"logging", "Turn on logging"} );              // logging is turned on using this flag
   optionsApplication.add_flag( {"logging-csv", "Add csv logger, prints log information using the csv format"} );
   
   optionsApplication.add({"logging-severity", "Set specific severity for logger, severity acts as a filter"});
   optionsApplication.add({"logging-tags", "set active log-tags to filter log messages"});
   optionsApplication.add({"logging-show", "Default is to log messages (non tagged), with this you can turn them on or off. If setting tag/tags then this turns off if not set to be on, value 0|1"});

   optionsApplication.add({ "port", "Set port number" });

   // ## Folder settings
   optionsApplication.add({"path", "Global path variable used to find files in any of the folders if not found in selected folder, folders are separated by semicolon"});
   optionsApplication.add({"folder-configuration", "Folder where to read configuration files"});
   optionsApplication.add({"folder-logging", "Set folder where logger places log files"});
}

std::pair<bool, std::string> CApplication::Read_s(CApplication* papplication_, gd::cli::options& optionsApplication)
{
   const auto* poptions_ = optionsApplication.sub_find_active();               // find active sub parser for arguments
   if( poptions_ == nullptr ) poptions_ = &optionsApplication;                 // set to root options

   // Set path property, it holds paths to folders that may be used as path in os system
   if( (*poptions_)["path"].is_true() == true ) { papplication_->PROPERTY_Add( "path", (*poptions_)["path"].as_string() ); }

   poptions_->iif( std::string_view("folder-configuration"), [papplication_]( const std::string_view& s_ ) { papplication_->PROPERTY_Add( "folder-configuration", s_ ); } );
   //if( (*poptions_)["folder-configuration"].is_true() == true ) { papplication_->PROPERTY_Add( "folder-configuration", (*poptions_)["folder-configuration"].as_string() ); }


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

/*
std::vector<std::string_view> CApplication::ROUTER_Resolv_s(const std::string_view& stringCommand, size_t* puOffset) 
{

   return { true, "" };
}
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
               const char* pbszName = xmlnode.attribute( "name" ).value();
               const char* pbszValue = xmlnode.attribute( "value" ).value();

               if( *pbszName != '\0' && *pbszValue != '\0' )
               {
                  if( PROPERTY_Has( pbszName ) == false )                      // check if property is added, if not then it is ok to add from configuration
                  {
                     PROPERTY_Add( pbszName, pbszValue );                      // add property
                  }
               }
            }
            xmlnode = xmlnode.next_sibling();
         }

         xmlnodeProperties = xmlnodeProperties.next_sibling( "properties" );
      }
   }

   return { true, "" };
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


