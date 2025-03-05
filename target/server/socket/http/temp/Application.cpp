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




void fail(boost::beast::error_code ec, char const* what)
{
   std::cerr << what << ": " << ec.message() << "\n";
}


// Handles an HTTP server connection
class http_session : public std::enable_shared_from_this<http_session>
{
   boost::beast::tcp_stream stream_;
   boost::beast::flat_buffer buffer_;
   std::shared_ptr<std::string const> doc_root_;

   static constexpr std::size_t queue_limit = 8; // max responses
   std::queue<boost::beast::http::message_generator> response_queue_;

   // The parser is stored in an optional container so we can
   // construct it from scratch it at the beginning of each new message.
   boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

public:
   // Take ownership of the socket
   http_session( tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root)
      : stream_(std::move(socket))
      , doc_root_(doc_root)
   {
      static_assert(queue_limit > 0,"queue limit must be positive");
   }

   // Start the session
   void run()
   {
      // We need to be executing within a strand to perform async operations
      // on the I/O objects in this session. Although not strictly necessary
      // for single-threaded contexts, this example code is written to be
      // thread-safe by default.
      boost::asio::dispatch( stream_.get_executor(), boost::beast::bind_front_handler( &http_session::do_read, this->shared_from_this()));
   }

private:
   void do_read()
   {
      // Construct a new parser for each message
      parser_.emplace();

      // Apply a reasonable limit to the allowed size
      // of the body in bytes to prevent abuse.
      parser_->body_limit(10000);

      // Set the timeout.
      stream_.expires_after(std::chrono::seconds(30));

      // Read a request using the parser-oriented interface
      boost::beast::http::async_read(
         stream_,
         buffer_,
         *parser_,
         boost::beast::bind_front_handler(
            &http_session::on_read,
            shared_from_this()));
   }

   void on_read(boost::beast::error_code ec, std::size_t uBytesTransferred)
   {
      boost::ignore_unused(uBytesTransferred);

      // This means they closed the connection
      if(ec == boost::beast::http::error::end_of_stream) return do_close();

      if(ec) return fail(ec, "read");

      // See if it is a WebSocket Upgrade
      /*
      if(boost::beast::websocket::is_upgrade(parser_->get()))
      {
         // Create a websocket session, transferring ownership
         // of both the socket and the HTTP request.
         std::make_shared<websocket_session>( stream_.release_socket())->do_accept(parser_->release());
         return;
      }
      */

      // Send the response
      //queue_write(handle_request(*doc_root_, parser_->release()));           // TODO : handle_request

      // If we aren't at the queue limit, try to pipeline another request
      if (response_queue_.size() < queue_limit)
         do_read();
   }

   void queue_write(http::message_generator response)
   {
      // Allocate and store the work
      response_queue_.push(std::move(response));

      // If there was no previous work, start the write loop
      if (response_queue_.size() == 1)
         do_write();
   }

   // Called to start/continue the write-loop. Should not be called when
   // write_loop is already active.
   void
      do_write()
   {
      if(! response_queue_.empty())
      {
         bool keep_alive = response_queue_.front().keep_alive();

         boost::beast::async_write(
            stream_,
            std::move(response_queue_.front()),
            boost::beast::bind_front_handler( &http_session::on_write, shared_from_this(), keep_alive));
      }
   }

   void on_write( bool keep_alive, boost::beast::error_code ec, std::size_t bytes_transferred)
   {
      boost::ignore_unused(bytes_transferred);

      if(ec)
         return fail(ec, "write");

      if(! keep_alive)
      {
         // This means we should close the connection, usually because
         // the response indicated the "Connection: close" semantic.
         return do_close();
      }

      // Resume the read if it has been paused
      if(response_queue_.size() == queue_limit)
         do_read();

      response_queue_.pop();

      do_write();
   }

   void do_close()
   {
      // Send a TCP shutdown
      boost::beast::error_code ec;
      stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

      // At this point the connection is closed gracefully
   }
};


// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
   boost::asio::io_context& ioc_;
   boost::asio::ip::tcp::acceptor acceptor_;
   std::shared_ptr<std::string const> doc_root_;

public:
   listener( boost::asio::io_context& iocontext_, boost::asio::ip::tcp::endpoint endpoint_, std::shared_ptr<std::string const> const& doc_root)
      : ioc_(iocontext_)
      , acceptor_(boost::asio::make_strand(iocontext_))
      , doc_root_(doc_root)
   {
      boost::beast::error_code ec;

      // Open the acceptor
      acceptor_.open(endpoint_.protocol(), ec);
      if(ec)
      {
         fail(ec, "open");
         return;
      }

      // Allow address reuse
      acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
      if(ec)
      {
         fail(ec, "set_option");
         return;
      }

      // Bind to the server address
      acceptor_.bind(endpoint_, ec);
      if(ec)
      {
         fail(ec, "bind");
         return;
      }

      // Start listening for connections
      acceptor_.listen( boost::asio::socket_base::max_listen_connections, ec);
      if(ec)
      {
         fail(ec, "listen");
         return;
      }
   }

   // Start accepting incoming connections
   void run()
   {
      // We need to be executing within a strand to perform async operations
      // on the I/O objects in this session. Although not strictly necessary
      // for single-threaded contexts, this example code is written to be
      // thread-safe by default.
      boost::asio::dispatch( acceptor_.get_executor(), boost::beast::bind_front_handler( &listener::do_accept, this->shared_from_this()));
   }

private:
   void
      do_accept()
   {
      // The new connection gets its own strand
      acceptor_.async_accept(
         boost::asio::make_strand(ioc_),
         boost::beast::bind_front_handler(
            &listener::on_accept,
            shared_from_this()));
   }

   void
      on_accept(beast::error_code ec, tcp::socket socket)
   {
      if(ec)
      {
         fail(ec, "accept");
      }
      else
      {
         // Create the http session and run it
         std::make_shared<http_session>(
            std::move(socket),
            doc_root_)->run();
      }

      // Accept another connection
      do_accept();
   }
};

/** ------------------------------------------------------------------------
* @brief 
* @param iArgumentCount 
* @param ppbszArgument 
* @return 
* https://www.boost.org/doc/libs/1_87_0/libs/beast/example/advanced/server/advanced_server.cpp
*/
int CApplication::Main_s(int iArgumentCount, char* ppbszArgument[])
{
   std::string stringIp("127.0.0.1");
   unsigned short uPort = 8080;

   std::string stringRootFolder = FOLDER_GetRoot_g( "temp__/" );


   auto const paddressIp = boost::asio::ip::make_address(stringIp);
   auto const pstringRoot = std::make_shared<std::string>(stringRootFolder);

   unsigned uThreadCount = 4;
   boost::asio::io_context iocontext_(uThreadCount);

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
   
   return 0;
}




namespace ip = boost::asio::ip;         // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

namespace my_program_state
{
   std::size_t
      request_count()
   {
      static std::size_t count = 0;
      return ++count;
   }

   std::time_t
      now()
   {
      return std::time(0);
   }
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
   http_connection(tcp::socket socket)
      : socket_(std::move(socket))
   {
   }

   // Initiate the asynchronous operations associated with the connection.
   void
      start()
   {
      read_request();
      check_deadline();
   }

private:
   // The socket for the currently connected client.
   tcp::socket socket_;

   // The buffer for performing reads.
   boost::beast::flat_buffer buffer_{8192};

   // The request message.
   boost::beast::http::request<http::dynamic_body> request_;

   // The response message.
   boost::beast::http::response<http::dynamic_body> response_;

   // The timer for putting a deadline on connection processing.
   boost::asio::basic_waitable_timer<std::chrono::steady_clock> deadline_{
      socket_.get_executor().context(), std::chrono::seconds(60)};

   // Asynchronously receive a complete request message.
   void
      read_request()
   {
      auto self = shared_from_this();

      boost::beast::http::async_read(
         socket_,
         buffer_,
         request_,
         [self](boost::beast::error_code ec,
            std::size_t bytes_transferred)
         {
            boost::ignore_unused(bytes_transferred);
            if(!ec)
               self->process_request();
         });
   }

   // Determine what needs to be done with the request message.
   void
      process_request()
   {
      response_.version(request_.version());
      response_.keep_alive(false);

      switch(request_.method())
      {
      case boost::beast::http::verb::get:
         response_.result(http::status::ok);
         response_.set(http::field::server, "Beast");
         create_response();
         break;

      default:
         // We return responses indicating an error if
         // we do not recognize the request method.
         response_.result(http::status::bad_request);
         response_.set(http::field::content_type, "text/plain");
         boost::beast::ostream(response_.body())
            << "Invalid request-method '"
            << request_.method_string()
            << "'";
         break;
      }

      write_response();
   }

   // Construct a response message based on the program state.
   void
      create_response()
   {
      if(request_.target() == "/count")
      {
         response_.set(http::field::content_type, "text/html");
         boost::beast::ostream(response_.body())
            << "<html>\n"
            <<  "<head><title>Request count</title></head>\n"
            <<  "<body>\n"
            <<  "<h1>Request count</h1>\n"
            <<  "<p>There have been "
            <<  my_program_state::request_count()
            <<  " requests so far.</p>\n"
            <<  "</body>\n"
            <<  "</html>\n";
      }
      else if(request_.target() == "/time")
      {
         response_.set(http::field::content_type, "text/html");
         boost::beast::ostream(response_.body())
            <<  "<html>\n"
            <<  "<head><title>Current time</title></head>\n"
            <<  "<body>\n"
            <<  "<h1>Current time</h1>\n"
            <<  "<p>The current time is "
            <<  my_program_state::now()
            <<  " seconds since the epoch.</p>\n"
            <<  "</body>\n"
            <<  "</html>\n";
      }
      else
      {
         response_.result(http::status::not_found);
         response_.set(http::field::content_type, "text/plain");
         boost::beast::ostream(response_.body()) << "File not found\r\n";
      }
   }

   // Asynchronously transmit the response message.
   void
      write_response()
   {
      auto self = shared_from_this();

      //response_.set(http::field::content_length, response_.body().size());

      boost::beast::http::async_write(
         socket_,
         response_,
         [self](boost::beast::error_code ec, std::size_t)
         {
            self->socket_.shutdown(tcp::socket::shutdown_send, ec);
            self->deadline_.cancel();
         });
   }

   // Check whether we have spent enough time on this connection.
   void
      check_deadline()
   {
      auto self = shared_from_this();

      deadline_.async_wait(
         [self](boost::beast::error_code ec)
         {
            if(!ec)
            {
               // Close socket to cancel any outstanding operation.
               self->socket_.close(ec);
            }
         });
   }
};

// "Loop" forever accepting new connections.
void
http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
   acceptor.async_accept(socket,
      [&](boost::beast::error_code ec)
      {
         if(!ec)
            std::make_shared<http_connection>(std::move(socket))->start();
         http_server(acceptor, socket);
      });
}

int main(int argc, char* argv[])
{
   try
   {
      // Check command line arguments.
      if(argc != 3)
      {
         std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
         std::cerr << "  For IPv4, try:\n";
         std::cerr << "    receiver 0.0.0.0 80\n";
         std::cerr << "  For IPv6, try:\n";
         std::cerr << "    receiver 0::0 80\n";
         return EXIT_FAILURE;
      }

      auto const address = boost::asio::ip::make_address(argv[1]);
      unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

      boost::asio::io_context ioc{1};

      tcp::acceptor acceptor{ioc, {address, port}};
      tcp::socket socket{ioc};
      http_server(acceptor, socket);

      ioc.run();
   }
   catch(std::exception const& e)
   {
      std::cerr << "Error: " << e.what() << std::endl;
      return EXIT_FAILURE;
   }
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



