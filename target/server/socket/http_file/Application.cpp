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

#include "gd/gd_file.h"

#include "Server.h"

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
}


std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{
   int iResult = Main_s( iArgumentCount, ppbszArgument );

   return application::basic::CApplication::Main( iArgumentCount, ppbszArgument, nullptr );
}

std::pair<bool, std::string> CApplication::Initialize()
{

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

/** ------------------------------------------------------------------------
* @brief 
* @param iArgumentCount 
* @param ppbszArgument 
* @return 
* https://www.boost.org/doc/libs/1_87_0/libs/beast/example/advanced/server/advanced_server.cpp
*/
int CApplication::Main_s(int iArgumentCount, char* ppbszArgument[])
{
   unsigned short uPort = 8080;
   std::string stringPort("8080");

   // ## Prepare ip address
   std::string stringIp("127.0.0.1");
   if( papplication_g->PROPERTY_Get("ip").empty() == false ) stringIp = papplication_g->PROPERTY_Get("ip").as_string();

   // ## Prepare root folder for site on local disk
   std::string stringRootFolder = FOLDER_GetRoot_g( "temp__/" );
   if( papplication_g->PROPERTY_Get("folder-root").empty() == false ) stringRootFolder = papplication_g->PROPERTY_Get("folder-root").as_string();

   unsigned uThreadCount = 4;
   if( papplication_g->PROPERTY_Get("system-treadcount").empty() == false ) uThreadCount = papplication_g->PROPERTY_Get("system-treadcount").as_uint();

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



