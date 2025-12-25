/*
 * @file Server.cpp
 */

#include "dto/DTOResponse.h"

#include "Router.h"

#include "Server.h"


CServer::CServer() {}
CServer::CServer(CApplication* ppapplication) : m_ppapplication(ppapplication) {}

CServer::CServer(const CServer& o) { common_construct(o); }
CServer::CServer(CServer&& o) noexcept { common_construct(std::move(o)); }
// assign
CServer& CServer::operator=(const CServer& o) { common_construct(o); return *this; }
CServer& CServer::operator=(CServer&& o) noexcept { common_construct(std::move(o)); return *this; }


CServer::~CServer()
{
}

void CServer::common_construct(const CServer& o) {}
void CServer::common_construct(CServer&& o) noexcept {}


std::pair<bool, std::string> CServer::Initialize()
{
   // Implementation of the Initialize method
   // For now, just return a dummy response
   return { true, "" };
}

/**
 * @brief Handles an incoming HTTP request and generates an appropriate HTTP response.
 *
 * This function processes HTTP GET and HEAD requests for static files and special commands.
 * It validates the request, routes commands, checks for illegal paths, and serves files from disk.
 * If the request is invalid or the file is not found, it returns the appropriate HTTP error response.
 *
 * @param stringRoot The root directory from which files should be served.
 * @param request_ The HTTP request to handle (moved in).
 * @return boost::beast::http::message_generator The generated HTTP response.
 *
 * Steps performed:
 * 1. Validates the HTTP method (only GET and HEAD are allowed).
 * 2. Normalizes the request target path.
 * 3. If the target starts with '!', routes the request as a command.
 * 4. Checks for illegal request targets (must be absolute and not contain "..").
 * 5. Calls the application core to process the request.
 * 6. Builds the full filesystem path to the requested file.
 * 7. Attempts to open the file from disk.
 *    - If not found, returns a 404 Not Found response.
 *    - If another error occurs, returns a 500 Server Error response.
 * 8. If the request is a HEAD, returns headers only.
 * 9. If the request is a GET, returns the file contents.
 */
boost::beast::http::message_generator handle_request( boost::beast::string_view stringRoot, boost::beast::http::request<boost::beast::http::string_body>&& request_)
{
   // Returns a bad request response
   auto const bad_request_ = [&request_](boost::beast::string_view stringWhy)
      {
         boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::bad_request, request_.version()};
         response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
         response.set(boost::beast::http::field::content_type, "text/html");
         response.keep_alive(request_.keep_alive());
         response.body() = std::string(stringWhy);
         response.prepare_payload();
         return response;
      };

   // Returns a not found response
   auto const not_found_ = [&request_](boost::beast::string_view stringTarget)
      {
         boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::not_found, request_.version()};
         response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
         response.set(boost::beast::http::field::content_type, "text/html");
         response.keep_alive(request_.keep_alive());
         response.body() = "The resource '" + std::string(stringTarget) + "' was not found.";
         response.prepare_payload();
         return response;
      };

   // Returns a server error response
   auto const server_error_ = [&request_](boost::beast::string_view stringWhat)
      {
         boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::internal_server_error, request_.version()};
         response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
         response.set(boost::beast::http::field::content_type, "text/html");
         response.keep_alive(request_.keep_alive());
         response.body() = "An error occurred: '" + std::string(stringWhat) + "'";
         response.prepare_payload();
         return response;
      };


   boost::beast::http::verb const eVerb = request_.method();

   // ## Make sure we can handle the method
   if( eVerb > boost::beast::http::verb::trace  )
   { 
      return bad_request_("Unknown HTTP-method"); 
   }

   std::string_view stringTarget = request_.target();
   if( stringTarget.empty() == true )
   { 
      return bad_request_("Empty request-target, server version: 0.9.0"); 
   }
   else if( stringTarget.size() > 0 && stringTarget[0] == '/' ) { stringTarget.remove_prefix(1); }

   // ## Route command if target begins with '!' .............................

   if( stringTarget.empty() == false && stringTarget.front() == '!' )
   {
      CServer server_( papplication_g );
      return server_.RouteCommand( stringTarget, std::move( request_ ) );
   }

   // ## Request path must be absolute and not contain "..".
   if( request_.target().empty() || request_.target()[0] != '/' || request_.target().find("..") != boost::beast::string_view::npos) 
   { 
      return bad_request_("Illegal request-target"); 
   }

   /*
   {
      // ## Process request by calling core method in application
      std::vector<std::pair<std::string, std::string>> vectorResponse;
      auto resulut_ = papplication_g->GetServer()->ProcessRequest( eVerb, stringTarget, vectorResponse );
      if ( resulut_.first == false ) { return server_error_(resulut_.second); }
   }
   */

   // ## Build the path to the requested file
   std::string stringPath = path_cat_g(stringRoot, request_.target());
   if(request_.target().back() == '/') { stringPath.append("index.html"); }
   else
   {                                                                                               LOG_DEBUG_RAW( stringPath );
   }

   // ## Attempt to open the file
   boost::beast::error_code errorcode;
   boost::beast::http::file_body::value_type body_;
   body_.open(stringPath.c_str(), boost::beast::file_mode::scan, errorcode);
   if(errorcode == boost::beast::errc::no_such_file_or_directory) { return not_found_(request_.target()); }
  
   if(errorcode) { return server_error_(errorcode.message()); }

   
   auto const uSize = body_.size();

   // ## Respond to HEAD request
   if(request_.method() == boost::beast::http::verb::head)
   {
      boost::beast::http::response<boost::beast::http::empty_body> response_{boost::beast::http::status::ok, request_.version()};
      response_.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      response_.set(boost::beast::http::field::content_type, mime_type_g(stringPath));
      response_.content_length(uSize);
      response_.keep_alive(request_.keep_alive());
      return response_;
   }

   // ## Respond to GET request
   boost::beast::http::response<boost::beast::http::file_body> response{ std::piecewise_construct, std::make_tuple(std::move(body_)), std::make_tuple(boost::beast::http::status::ok, request_.version())};
   response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   response.set(boost::beast::http::field::content_type, mime_type_g(stringPath));
   response.content_length(uSize);
   response.keep_alive(request_.keep_alive());
   return response;
}


boost::beast::http::message_generator CServer::RouteCommand( std::string_view stringTarget, boost::beast::http::request<boost::beast::http::string_body>&& request_ )
{
   // Returns a server error response
   auto const server_error_ = [&request_](boost::beast::string_view stringWhat)
      {
         boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::internal_server_error, request_.version()};
         response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
         response.set(boost::beast::http::field::content_type, "text/html");
         response.keep_alive(request_.keep_alive());
         response.body() = "An error occurred: '" + std::string(stringWhat) + "'";
         response.prepare_payload();
         return response;
      };


	CRouter router_(papplication_g, stringTarget);                             // create router for the target, router is a simple command router to handle commands
	auto result_ = router_.Parse();                                            // parse the target to get command and parameters
   if( result_.first == false ) { return server_error_( result_.second ); }

   result_ = router_.Run();
   if( result_.first == false ) { return server_error_( result_.second ); }

   std::string stringBody;

   if( router_.HasResult() == true )
   {
      router_.PrintResponseXml( stringBody, nullptr );
   }

   if( stringBody.empty() == true )
   {
      stringBody = "<response status=\"ok\" />";
   }

   std::array<std::byte, 128> array_; // array to hold data for arguments
   gd::argument::arguments argumentHeader( (gd::argument::arguments::pointer)array_.data(), (unsigned)array_.size() );

   if( router_.IsJson() == true ){ argumentHeader["format"] = "json"; }
   else { argumentHeader["format"] = "xml"; }

   boost::beast::http::file_body::value_type body_;

   // 1. Create a response object using string_body
   boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::ok, request_.version()};

   // 2. Set the body of the response
   response.body() = std::move(stringBody);

   PrepareResponseHeader_s( argumentHeader, response );

   /*
   // 3. Set essential headers
   response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   response.set(boost::beast::http::field::content_type, "text/plain");

   // 4. Manage the connection state (keep-alive or close)
   response.keep_alive(request_.keep_alive());

   // 5. Prepare the payload. This is crucial as it finalizes headers,
   //    for example, it automatically sets the Content-Length header for string_body.
   response.prepare_payload();

   // 6. Return the response. It will be implicitly converted to message_generator.
   */
   return response;
}

std::pair<bool, std::string> CServer::ProcessRequest(boost::beast::http::verb eVerb, std::string_view stringCommand, std::vector<std::pair<std::string, std::string>>& vectorResponse)
{                                                                                                     LOG_INFORMATION_RAW("Command: " + std::string(stringCommand));
   using namespace gd::com::server::router;
   // ## Create command object from request
   //gd::com::server::server_i* pserver = m_ppapplication->ROUTER_GetActiveServer();
   //gd::com::pointer< command > pcommand = gd::com::pointer< gd::com::server::router::command >( new gd::com::server::router::command( pserver ) );

   //auto result_ = pcommand->append(stringCommand, gd::types::tag_uri{});
   //if( result_.first == false ) { return { false, "Failed to append command: " + std::string(stringCommand)  + " - " + result_.second }; }


   //std::vector< std::string_view > vectorCommand = static_cast<gd::com::server::router::command*>( pcommand )->add_querystring( stringCommand );

   /*
   vectorCommand.erase(std::remove_if(vectorCommand.begin(), vectorCommand.end(), [](const auto& string_) {
      return string_.empty();
   }), vectorCommand.end());
   */

   //pserver->get( pcommand, nullptr );
   // pserver->get( vectorCommand, nullptr, pcommand, nullptr);


   if(eVerb == boost::beast::http::verb::get)
   {
      //auto result_ = Execute( pcommand );
      // Handle GET request
      vectorResponse.push_back({"Content-Type", "text/plain"});
      return {true, "GET request processed for target: " + std::string(stringCommand)};
   }
   else if(eVerb == boost::beast::http::verb::head)
   {
      // Handle HEAD request
      vectorResponse.push_back({"Content-Type", "text/plain"});
      return {true, "HEAD request processed for target: " + std::string(stringCommand)};
   }
   else
   {
      return {false, "Unsupported HTTP verb"};
   }
}

std::pair<bool, std::string> CServer::Execute(gd::com::server::command_i* pcommand)
{
   CHttpServer* phttpserver = m_ppapplication->GetHttpServer();

   gd::com::server::response_i* presponse = nullptr;

   auto result_ = phttpserver->Execute( pcommand, &presponse );


   //std::string_view stringCommand = vectorCommand[0];

   //gd::com::server::server_i* pserver = m_router.GetServer( stringCommand );
   // CHttpServer* phttpserver = m_ppapplication->GetHttpServer();

   // auto result_ = phttpserver->Get( vectorCommand, pcommand );

   // if( pserver == nullptr ) { return { false, "No server found for command: " + std::string(stringCommand) }; }

   // pserver->get( vectorCommand, nullptr, pcommand, nullptr);



   return { true, "" };
}


std::pair<bool, std::string> CServer::Execute(const std::vector<std::string_view>& vectorCommand, gd::com::server::command_i* pcommand)
{                                                                                                  assert( vectorCommand.empty() == false );
   CHttpServer* phttpserver = m_ppapplication->GetHttpServer();

   gd::com::server::response_i* presponse = nullptr;

   auto result_ = phttpserver->Execute( vectorCommand, pcommand, &presponse );

   return { true, "" };
}


void CServer::PrepareResponseHeader_s( gd::argument::arguments& argumentHeader, boost::beast::http::response<boost::beast::http::string_body>& response )
{
   using namespace boost::beast::http;
   response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   response.set(field::access_control_allow_origin, "*");   
   response.set(field::access_control_allow_methods, "GET, POST, HEAD, OPTIONS");

   auto stringFormat = argumentHeader["format"].as_string_view();
   if( stringFormat.empty() == false )
   {
      std::string stringContentType("application/");
      stringContentType += stringFormat;
      response.set(field::content_type, stringContentType);
   }
   else 
   {
      response.set( field::content_type, "text/plain" );
   }

   response.prepare_payload();
}




// Report a failure
void fail_g(boost::beast::error_code errorcode, char const* piWhat)
{
   std::cerr << piWhat << ": " << errorcode.message() << "\n";
}

/// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type_g(boost::beast::string_view stringPath)
{
   using boost::beast::iequals;
   auto const stringExtension = [&stringPath] {
      auto const uPosition = stringPath.rfind(".");
      if(uPosition == boost::beast::string_view::npos) { return boost::beast::string_view{}; }
      return stringPath.substr(uPosition);
   }();

   if(iequals(stringExtension, ".htm"))  return "text/html";
   if(iequals(stringExtension, ".html")) return "text/html";
   if(iequals(stringExtension, ".php"))  return "text/html";
   if(iequals(stringExtension, ".css"))  return "text/css";
   if(iequals(stringExtension, ".txt"))  return "text/plain";
   if(iequals(stringExtension, ".js"))   return "application/javascript";
   if(iequals(stringExtension, ".json")) return "application/json";
   if(iequals(stringExtension, ".xml"))  return "application/xml";
   if(iequals(stringExtension, ".swf"))  return "application/x-shockwave-flash";
   if(iequals(stringExtension, ".flv"))  return "video/x-flv";
   if(iequals(stringExtension, ".png"))  return "image/png";
   if(iequals(stringExtension, ".jpe"))  return "image/jpeg";
   if(iequals(stringExtension, ".jpeg")) return "image/jpeg";
   if(iequals(stringExtension, ".jpg"))  return "image/jpeg";
   if(iequals(stringExtension, ".gif"))  return "image/gif";
   if(iequals(stringExtension, ".bmp"))  return "image/bmp";
   if(iequals(stringExtension, ".ico"))  return "image/vnd.microsoft.icon";
   if(iequals(stringExtension, ".tiff")) return "image/tiff";
   if(iequals(stringExtension, ".tif"))  return "image/tiff";
   if(iequals(stringExtension, ".svg"))  return "image/svg+xml";
   if(iequals(stringExtension, ".svgz")) return "image/svg+xml";
   return "application/text";
}

/// Append an HTTP relative-path to a local filesystem path.
/// The returned path is normalized for the platform.
std::string path_cat_g( boost::beast::string_view stringBase,  boost::beast::string_view stringPath)
{
   if( stringBase.empty() == true ) return std::string(stringPath);

   std::string result(stringBase);
#ifdef BOOST_MSVC
   char constexpr iPathSeparator = '\\';
   if(result.back() == iPathSeparator) { result.resize(result.size() - 1); }
   result.append(stringPath.data(), stringPath.size());
   for(auto& c : result) { if(c == '/') { c = iPathSeparator; } }
#else
   char constexpr iPathSeparator = '/';
   if(result.back() == iPathSeparator) { result.resize(result.size() - 1); }
   result.append(stringPath.data(), stringPath.size());
#endif
   return result;
}


// ----------------------------------------------------------------------------
// ------------------------------------------------------------------  listener
// ----------------------------------------------------------------------------


/** ---------------------------------------------------------------------------
 * @brief construct listerner that is used to listen for incoming connections
 * @param iocontext_ core input and output logic, acts as a composite class for
                     specialized boost classes that has logic for computer networking.
 * @param endpoint_
 * @param pstringFolderRoot
 */
listener::listener( boost::asio::io_context& iocontext_, boost::asio::ip::tcp::endpoint endpoint_, std::shared_ptr<std::string const> const& pstringFolderRoot )
   : m_iocontext(iocontext_), m_acceptor(boost::asio::make_strand(iocontext_)), m_pstringFolderRoot(pstringFolderRoot)
{
   boost::beast::error_code errorcode;

   m_acceptor.open(endpoint_.protocol(), errorcode);                           // Open the acceptor
   if(errorcode)
   {
      fail_g(errorcode, "open");
      return;
   }

   m_acceptor.set_option(boost::asio::socket_base::reuse_address(true), errorcode);// Allow address reuse
   if(errorcode)
   {
      fail_g(errorcode, "set_option");
      return;
   }

   m_acceptor.bind(endpoint_, errorcode);                                      // Bind to the server address
   if(errorcode)
   {
      fail_g(errorcode, "bind");
      return;
   }

   m_acceptor.listen(boost::asio::socket_base::max_listen_connections, errorcode);// Start listening for connections
   if(errorcode)
   {
      fail_g(errorcode, "listen");
      return;
   }
}

void listener::do_accept()
{
   // The new connection gets its own strand
   auto pstrand = boost::asio::make_strand(m_iocontext);                       // strand = "Sequential Execution", make sure each step is executed in order
   m_acceptor.async_accept( pstrand, boost::beast::bind_front_handler( &listener::on_accept, shared_from_this()));
}

void listener::on_accept(boost::beast::error_code errorcode, boost::asio::ip::tcp::socket socket)
{
   if(errorcode)
   {
      fail_g(errorcode, "accept");
      return;                                                                  // To avoid infinite loop
   }
   else
   {
      // ## Create the session and run it
      auto psession = std::make_shared<session>( std::move(socket), m_pstringFolderRoot);
      psession->run();
   }

   do_accept();                                                                // Accept another connection
}


// ----------------------------------------------------------------------------
// -------------------------------------------------------------------- session
// ----------------------------------------------------------------------------

session::session( boost::asio::ip::tcp::socket&& socket, std::shared_ptr<std::string const> const& pstringFolderRoot)
   : m_tcpstream(std::move(socket))
   , m_pstringFolderRoot(pstringFolderRoot)
{
}

void session::run()
{
   // We need to be executing within a strand to perform async operations
   // on the I/O objects in this session. Although not strictly necessary
   // for single-threaded contexts, this example code is written to be
   // thread-safe by default.
   boost::asio::dispatch(m_tcpstream.get_executor(), boost::beast::bind_front_handler( &session::do_read, shared_from_this()));
}

/// Read data into
void session::do_read()
{
   // Make the request empty before reading,
   // otherwise the operation behavior is undefined.
   m_request = {};

   m_tcpstream.expires_after(std::chrono::seconds(30));                        // Set the timeout.

   // Read a request
   boost::beast::http::async_read(m_tcpstream, m_flatbuffer, m_request, boost::beast::bind_front_handler( &session::on_read, shared_from_this()));
}

void session::on_read( boost::beast::error_code errorcode, std::size_t uBytesTransferred)
{                                                                                                  boost::ignore_unused(uBytesTransferred);
   // This means they closed the connection
   if(errorcode == boost::beast::http::error::end_of_stream) { return do_close(); }

   if(errorcode) { return fail_g(errorcode, "read"); }

   // Send the response
   send_response( handle_request(*m_pstringFolderRoot, std::move(m_request)));
}

void session::send_response(boost::beast::http::message_generator&& messagegenerator)
{
   bool bKeepAlive = messagegenerator.keep_alive();                            // get current keep alive status

   // Write the response
   boost::beast::async_write( m_tcpstream,std::move(messagegenerator), boost::beast::bind_front_handler( &session::on_write, shared_from_this(), bKeepAlive));
}

void session::on_write( bool bKeepAlive, boost::beast::error_code errorcode, std::size_t uBytesTransferred)
{                                                                                                  boost::ignore_unused(uBytesTransferred);
   if(errorcode) { return fail_g(errorcode, "write"); }

   if(bKeepAlive == false)
   {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return do_close();
   }

   do_read();                                                                  // Read another request
}

void session::do_close()
{
   // Send a TCP shutdown
   boost::beast::error_code errorcode;
   m_tcpstream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, errorcode);
                                                                               // At this point the connection is closed gracefully
}
