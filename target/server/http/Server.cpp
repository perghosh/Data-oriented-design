/*
 * @file Server.cpp
 */

/*
 * NAVIGATE ==================================================================
 * - `0TAG0handle_request` - Handle incoming HTTP requests and generate responses
 */

#include <array>
#include <cctype>
#include <fstream>
#include <filesystem>

#include "gd/parse/gd_parse_window_line.h"
#include "gd/expression/gd_expression_parse_state.h"

#include "api/APIView.h"

#include "dto/DTOResponse.h"
#include "render/RENDERHtml.h"

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

/// @brief Get the listener for the server ----------------------------------
std::shared_ptr<listener> CServer::GetListener() const { return m_plistener; }

/// @brief Set the listener for the server ----------------------------------
void CServer::SetListener( std::shared_ptr<listener> plistener )
{
   m_plistener = plistener;
}

std::pair<bool, std::string> CServer::Initialize( const gd::argument::arguments& arguments_ )
{
   std::string stringValue;

   // ## read file extensions to block .......................................
   stringValue = arguments_["ignore-extension"].as_string();
   if(stringValue.empty() == false)
   { 
      m_argumentSettings.append("ignore-extension", stringValue); 
      AddFlags(CServer::eFlagIgnore);
   }

   // ## read folder settings ................................................
   stringValue = arguments_["webroot"].as_string();
   if(stringValue.empty() == false)
   {
      m_argumentSettings.append("webroot", stringValue);
      AddFlags(CServer::eFlagRoot);
   }

   // ## read folder settings ................................................
   stringValue = arguments_["path"].as_string();
   if(stringValue.empty() == false)
   {
      m_argumentSettings.append("path", stringValue);
      AddFlags(CServer::eFlagPath);

   }

   // ## read SSR comment settings ..........................................
   stringValue = arguments_["ssr-comment"].as_string();
   if(stringValue.empty() == false)
   {
      m_argumentSettings.append("ssr-comment", stringValue);
      AddFlags(CServer::eFlagSSR);
   }

   // ## read SSR extension settings ..........................................
   stringValue = arguments_["ssr-extension"].as_string();
   if(stringValue.empty() == false)
   {
      m_argumentSettings.append("ssr-extension", stringValue);
      AddFlags(CServer::eFlagSSR);
   }


   // ## Build index for values in argument settings .........................
   m_argumentIndexSettings.build(m_argumentSettings);
   m_puIndexSettings[ValueIndex_s("ignore-extension")] = m_argumentIndexSettings.get_index("ignore-extension"); // set index for ignore-extension setting
   m_puIndexSettings[ValueIndex_s("webroot")] = m_argumentIndexSettings.get_index("webroot"); // set index for webroot setting
   m_puIndexSettings[ValueIndex_s("path")] = m_argumentIndexSettings.get_index("path"); // set index for path setting
   m_puIndexSettings[ValueIndex_s("ssr-comment")] = m_argumentIndexSettings.get_index("ssr-comment"); // set index for ssr-comment setting
   m_puIndexSettings[ValueIndex_s("ssr-extension")] = m_argumentIndexSettings.get_index("ssr-extension"); // set index for ssr-extension setting   

   return { true, "" };
}

namespace
{
   /// Returns a bad request response
   inline boost::beast::http::response<boost::beast::http::string_body> server_error_s( const boost::beast::http::request<boost::beast::http::string_body>& request_, boost::beast::string_view stringWhat)
   {
      boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::internal_server_error, request_.version()};
      response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      response.set(boost::beast::http::field::content_type, "text/html");
      response.keep_alive(request_.keep_alive());
      response.body() = "An error occurred: '" + std::string(stringWhat) + "'";
      response.prepare_payload();
      return response;
   }
} // namespace

/** @CRITICAL [tag: server, http, request] [summary: Handle incoming HTTP requests and generate responses]
 * @brief Handles an incoming HTTP request and generates an appropriate HTTP response.
 *
 * This function processes HTTP GET and HEAD requests for static files and special commands.
 * It validates the request, routes commands, checks for illegal paths, and serves files from disk.
 * If the request is invalid or the file is not found, it returns the appropriate HTTP error response.
 *
 * @param stringRoot The root directory from which files should be served.
 * @param request_ The HTTP request to handle (moved in).
 * @param psession_ A pointer to the session associated with the request (optional, can be used for session management).
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
boost::beast::http::message_generator                                          // 0TAG0handle_request
   handle_request( boost::beast::string_view stringRoot,  boost::beast::http::request<boost::beast::http::string_body>&& request_, const session* psession_ )
{
   // Returns a bad request response
   auto const error_ = [&request_]( boost::beast::string_view stringWhy )
   {
      boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::bad_request, request_.version()};
      response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      response.set(boost::beast::http::field::content_type, "text/html");
      response.keep_alive(request_.keep_alive());
      response.body() = std::string(stringWhy);
      response.prepare_payload();
      return response;
   };

   boost::beast::http::verb const eVerb = request_.method();

   // ## Make sure we can handle the method
   if( eVerb > boost::beast::http::verb::trace  ) { return error_("Unknown HTTP-method"); }

   if( eVerb == boost::beast::http::verb::options )
   {
      boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::ok, request_.version()};
      response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      response.set(boost::beast::http::field::content_type, "text/html");
      response.set(boost::beast::http::field::access_control_allow_origin, "*");   
      response.set(boost::beast::http::field::access_control_allow_methods, "GET, POST, HEAD, OPTIONS");
      response.set(boost::beast::http::field::access_control_allow_headers, "content-type, authorization, x-requested-with, accept, origin");
      response.keep_alive(request_.keep_alive());
      response.prepare_payload();
      return response;
   }

   std::string_view stringPage = request_.target();
   std::string_view stringTarget = stringPage;
   if( stringTarget.empty() == true ) {  return error_("Empty request-target, server version: 0.9.0"); }
   else if( stringTarget.size() > 0 && stringTarget[0] == '/' ) { stringTarget.remove_prefix(1); }

   std::string_view stringBody = request_.body();

   // ## Route command if target begins with '!' ............................. @API [tag: server, uri, route-command] [summary: Investigate and route command requests]

   CServer* pserver = papplication_g->GetServer();

   if(stringTarget.empty() == false && stringTarget.front() == '!')          // @CRITICAL [tag: command] [description: Route command requests that start with '!' to the command handler]
   {
      if(pserver)
      {
         return pserver->RouteCommand(stringTarget, stringBody, std::move(request_), psession_);
      }
      else
      {  // @DEPRECATED [tag: server, application] [description: Handle case where server pointer is null, should not happen if application is properly initialized]
         CServer server_(papplication_g);
         return server_.RouteCommand(stringTarget, stringBody, std::move(request_), psession_);
      }
   }

   // ## Request path must be absolute and not contain ".."...................
   if( stringPage.empty() || stringPage[0] != '/' || stringPage.find("..") != boost::beast::string_view::npos) { return error_("Illegal request-target"); }

   // ## Build the path to the requested file
   std::string stringPath = path_cat_g(stringRoot, request_.target());
   if(request_.target().back() == '/') { stringPath.append("index.html"); }
   else
   {                                                                                               //LOG_DEBUG_RAW( stringPath );
   }

   // ## Check for ending ? and remove it ....................................
   auto uPosition = stringPath.find('?');
   if( uPosition != std::string::npos ) { stringPath = stringPath.substr( 0, uPosition ); }

   // ------------------------------------------------------------------------
   // ## Check if server is in blocking mode, if it is, check if file is blocked
   if( pserver != nullptr && pserver->IsIgnore() == true )
   {
      if( pserver->IsIgnored(stringPath) == true )
      {
         return error_( std::format( "The resource '{}' is blocked.", stringTarget ) );
      }
   }

   // ## Attempt to open the file
   boost::beast::error_code errorcode_;
   boost::beast::http::file_body::value_type body_;
   body_.open(stringPath.c_str(), boost::beast::file_mode::scan, errorcode_);

   // ------------------------------------------------------------------------
   // ## Check if server is in SSR mode, if it is, check if file is SSR extension and render it
   if(pserver != nullptr && pserver->IsSSR() == true && pserver->IsSSRExtension(stringPath) )
   {
      bool bRender = false;
      std::string stringSSR;
      if( pserver->PeekSSRComment(stringPath, stringSSR) ) { bRender = true; }

      if(bRender == true)
      {
         std::string stringRendered;
         auto message_ = pserver->RenderPage(stringTarget, stringBody, stringPath, stringSSR, std::move(request_), psession_ );

      }
   }



   if(errorcode_ == boost::beast::errc::no_such_file_or_directory) 
   {                                                                                               LOG_DEBUG_RAW( std::format( "File not found: '{}'", stringPath ) );
      return error_(  std::format( "The resource '{}' was not found.", stringTarget ) ); 
   }
   if(errorcode_) { return error_( std::format( "An error occurred: {}", errorcode_.message()) ); }
   
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

// @CRITICAL [tag: router, uri] [description: Routes and processes HTTP commands based on the target path]
/** --------------------------------------------------------------------------
 * @brief Routes and processes HTTP commands based on the target path
 * 
 * This method handles the routing of HTTP requests to appropriate command handlers.
 * It parses the target path to identify commands and parameters, executes the command,
 * and generates an appropriate HTTP response.
 * 
 * @param stringTarget The target path from the HTTP request (e.g., "/api/command")
 * @param stringBody The body of the HTTP request
 * @param request_ The HTTP request object (moved into the method)
 * @return boost::beast::http::message_generator A message generator that will produce the HTTP response
 * 
 * The method follows these steps:
 * 1. Creates a router object for the target path
 * 2. Parses the target to extract command and parameters
 * 3. Executes the command via the router
 * 4. Generates an XML or JSON response based on the router's output
 * 5. Prepares and returns an HTTP response with appropriate headers
 * 
 * If any step fails, it returns an internal server error response with the error message.
 */
boost::beast::http::message_generator CServer::RouteCommand( std::string_view stringTarget, std::string_view stringBody, boost::beast::http::request<boost::beast::http::string_body>&& request_, const session* psession_ )
{
	CRouter router_(papplication_g, stringTarget, stringBody);                 // create router for the target, router is a simple command router to handle commands
   router_.SetSession( psession_ );

   if( stringBody.empty() == false )
   {
      // ## prepare content type from request header
      std::string stringContentType = request_[boost::beast::http::field::content_type];
      if( stringContentType.find("xml") != std::string::npos ) { router_.SetFlag( CRouter::eRequestFormatXml ); }
      else if( stringContentType.find( "json" ) != std::string::npos ) { router_.SetFlag( CRouter::eRequestFormatJson ); }
      else { router_.SetFlag( CRouter::eRequestFormatNone ); }
   }

	auto result_ = router_.Parse();                                            // parse the target to get command and parameters
   if( result_.first == false ) { return server_error_s( request_, result_.second ); }
   

   result_ = router_.Run();
   if( result_.first == false ) 
   { 
      std::string& stringError = result_.second;
      auto response_ = PrepareResponse_s( request_, int(boost::beast::http::status::bad_request), "text/plain", stringError );
      return response_;
   }

   std::string stringResponse;

   // ## print response as XML or JSON, this will be used as response body
   if( router_.HasResult() == true )
   {
      router_.PrintResponseXml( stringResponse, nullptr );                    // print response as XML, this will be used as response body
   }

   if( stringResponse.empty() == true ) { stringResponse = "<response status=\"ok\" />"; } // if response is empty, set it to a default response

   std::array<std::byte, 128> array_; // array to hold data for arguments
   gd::argument::arguments argumentHeader( (gd::argument::arguments::pointer)array_.data(), (unsigned)array_.size() );

   if( router_.IsJson() == true ){ argumentHeader["format"] = "json"; }
   else { argumentHeader["format"] = "xml; charset=utf-8"; }                  // set format of response, this will be added to response header

   boost::beast::http::file_body::value_type body_;

   // 1. Create a response object using string_body
   boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::ok, request_.version()};

   // 2. Set the body of the response
   response.body() = std::move( stringResponse );                             // set response body, body is pased to client

   // 3. Set other response parameters
   PrepareResponseHeader_s( argumentHeader, response );                       // prepare response header

   return response;
}

/** --------------------------------------------------------------------------- RenderPage
 * @brief Renders an HTML page and returns it as an HTTP response message.
 *
 * Creates a `CRouter` context for request-scoped routing data, executes HTML
 * rendering through `CRENDERHtml`, and maps the render result to an HTTP response.
 * If rendering fails, the method returns **500 Internal Server Error** with the
 * renderer error text as `text/plain`.
 *
 * @param stringTarget Request target used to initialize routing context.
 * @param stringBody Request body used to initialize routing context.
 * @param stringPath Physical/virtual page path to render.
 * @param stringHeader Header input for page rendering context.
 * @param request_ Incoming HTTP request moved into response preparation path.
 * @param psession_ Optional session pointer for request context (currently unused in this method).
 * @return boost::beast::http::message_generator Generated HTTP response containing rendered HTML or an error response if rendering fails.
 */
boost::beast::http::message_generator CServer::RenderPage(
   std::string_view stringTarget, 
   std::string_view stringBody, 
   std::string_view stringPath, 
   std::string_view stringHeader, 
   boost::beast::http::request<boost::beast::http::string_body>&& request_,
   const session* psession_)
{
   CRouter router_(papplication_g, stringTarget, stringBody);                 // create router for the target, router is a simple command router to handle commands
   router_.SetSession(psession_);

   std::string stringSSRPage;
   CRouter::Configure_call configure_ = [&]( CAPI_Base* papiObject, std::string_view stringObjectType, std::string_view stringEventStage  ) {
      if(stringEventStage == "before")
      {
         if(stringObjectType == "view")
         {
            CAPIView* papiview = reinterpret_cast<CAPIView*>(papiObject);
            papiview->SetPath(stringPath);
         }
      }
      else if(stringEventStage == "after")
      {
         if(stringObjectType == "view")
         {
            CAPIView* papiview = reinterpret_cast<CAPIView*>(papiObject);
            stringSSRPage = std::move(papiview->GetSSRPage());
         }
      }
   };

   router_.SetConfigureCallback(std::move( configure_ ));
   auto result_ = router_.Parse();                                            // parse the target to get command and parameters
   if(result_.first == false) { return server_error_s(request_, result_.second); }

   router_.SetFlag(CRouter::eFlagCommand | CRouter::eFlagNoResponse);
   result_ = router_.Run( "view/ssr" );
   if(result_.first == false) { return server_error_s(request_, result_.second); }

   std::size_t uSize = stringSSRPage.size();

   // 1. Create a response object using string_body
   boost::beast::http::response<boost::beast::http::string_body> response{ boost::beast::http::status::ok, request_.version() };

   // 2. Set the body to page
   response.body() = std::move(stringSSRPage);                                // set response body, body is pased to client

   // 3. Set other response parameters
   response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   response.set(boost::beast::http::field::content_type, mime_type_g(stringPath));
   response.content_length(uSize);
   response.keep_alive(request_.keep_alive());

   return response;
}



/** --------------------------------------------------------------------------- @API [tag: server, http, block] [summary: Checks if a given file path is blocked based on the server's ignore-extension settings]
 * @brief Checks if a given file path is blocked based on the server's ignore-extension settings
 * This method determines if a file should be blocked from being served based 
 * on its extension. It retrieves the list of ignored extensions from the
 * server's settings, extracts the extension from the provided file path, and
 * checks if it matches any of the ignored extensions.
 * 
 * @param stringPath The file path to check for blocking
 * @return true if the file is blocked (i.e., its extension is in the ignore list)
 */
bool CServer::IsIgnored(std::string_view stringPath) const
{
   std::string_view stringIgnoreExtension = GetPropertyValue(eIndexSettingsIgnoreExtension);       assert(stringIgnoreExtension.empty() == false);

   // ## extract file extension from path
   auto uPosition = stringPath.rfind('.');
   if(uPosition == std::string_view::npos) { return false; }

   std::string_view stringExtension = stringPath.substr(uPosition + 1);

   std::array<std::string_view, 32> arrayString;   // adjust 32 as needed
   auto uCount = gd::utf8::split(stringIgnoreExtension, ',', arrayString, gd::utf8::tag_stack{});

   // ## check if file extension is in ignore list
   for(std::size_t u = 0; u < uCount; ++u)
   {
      if(stringExtension == arrayString[u]) { return true; }
   }
   
   return false;
}

/** --------------------------------------------------------------------------- @API [tag: server, http, ssr] [summary: Checks if a file path has an SSR (Server-Side Rendering) extension]
 * @brief Checks if a file path has an SSR (Server-Side Rendering) extension.
 * 
 * If no specific extensions is set only html pages are processed as SSR, otherwise the extensions set in settings are processed as SSR.
 * 
 * @param stringPath The file path to check for SSR extension.
 * @return True if the file extension should be processed as SSR.
 */
bool CServer::IsSSRExtension(std::string_view stringPath) const                // @TODO [tag: server, http, ssr] [description: Implement SSR extension checking based on settings, currently only checks for .html extension]
{
   if(IsProperty(eIndexSettingsSSRExtension) == false)
   {
      // If no specific SSR extensions are set, default to treating .html files as SSR
      auto uPosition = stringPath.rfind('.');
      if(uPosition == std::string_view::npos) { return false; }
      std::string_view stringExtension = stringPath.substr(uPosition + 1);
      return stringExtension == "html";
   }

   std::string_view stringSSRExtension = GetPropertyValue(eIndexSettingsSSRExtension);             assert(stringSSRExtension.empty() == false);

   // ## extract file extension from path ...................................
   auto uPosition = stringPath.rfind('.');
   if(uPosition == std::string_view::npos) { return false; }
   std::string_view stringExtension = stringPath.substr(uPosition + 1);

   std::array<std::string_view, 32> arrayString;   // adjust 32 as needed
   auto uCount = gd::utf8::split(stringSSRExtension, ',', arrayString, gd::utf8::tag_stack{});
   // ## check if file extension is in SSR list
   for(std::size_t u = 0; u < uCount; ++u)
   {
      if(stringExtension == arrayString[u]) { return true; }
   }

   return false;
}

/** --------------------------------------------------------------------------- @API [tag: server, http, ssr, file] [summary: Detects SSR marker comments near the start of a file]
 * @brief Reads the file head and checks whether any configured SSR identifier exists.
 *
 * Uses the `eIndexSettingsSSRComment` property as a comma-separated identifier list.
 * The method reads only a small fixed-size prefix from the file to keep detection cheap.
 *
 * @param stringPath Path to the file that may contain an SSR marker comment.
 * @param stringComment Output value set to the matched SSR identifier when found.
 * @return `true` if a configured SSR identifier is found in the file head; otherwise `false`.
 */
bool CServer::PeekSSRComment(std::string_view stringPath, std::string& stringComment) const
{
   constexpr std::size_t uMaxCommentLength = 16;
   stringComment.clear();

   std::string_view stringSSRComment = GetPropertyValue(eIndexSettingsSSRComment);                 assert( stringSSRComment.empty() == false );
   if(stringSSRComment.empty() == true) { return false; }

   // ## split ssr comment setting into array of comments ....................
   std::array<std::string_view, 16> arrayIdentifier;
   std::size_t uCommentCount = gd::utf8::split(stringSSRComment, ',', arrayIdentifier, gd::utf8::tag_stack{});
   if(uCommentCount == 0) { return false; }

   // ## Read the first 16 characters in file and check for ssr comment ......
   std::string stringHead;
   stringHead.resize(uMaxCommentLength);
   std::ifstream ifstreamFile(std::string(stringPath), std::ios::binary);
   if(ifstreamFile.is_open() == false) { return false; }
   ifstreamFile.read(stringHead.data(), static_cast<std::streamsize>(stringHead.size()));

   std::size_t uReadCount = static_cast<std::size_t>(ifstreamFile.gcount());
   if(uReadCount == 0) { return false; }

   std::string_view stringHeadView(stringHead.data(), uReadCount);

   for(std::size_t u = 0; u < uCommentCount; ++u)
   {
      const std::string_view stringIdentifier = arrayIdentifier[u];
      if(stringIdentifier.empty() == true) { continue; }
      if(stringHeadView.find(stringIdentifier) != std::string_view::npos)
      {
         stringComment.assign(stringIdentifier);
         return true;
      }
   }

   return false;
}


/** -------------------------------------------------------------------------- @API [tag: response, header] [summary: Prepares response header for request]
 * @brief Prepares response header for request
 * 
 * This method sets up the HTTP response headers based on the provided argument header information.
 * 
 * @param argumentHeader additional argument header information
 * @param argumentHeader.format format of response
 * @param response The HTTP response object to prepare, its the boost beast response object
 * 
 * @code
   std::array<std::byte, 128> array_; // array to hold data for arguments
   gd::argument::arguments argumentHeader( (gd::argument::arguments::pointer)array_.data(), (unsigned)array_.size() );
   argumentHeader["format"] = "xml";
   boost::beast::http::response<boost::beast::http::string_body> response{boost::beast::http::status::ok, request_.version()};
   CServer::PrepareResponseHeader_s( argumentHeader, response );
 * @endcode
 */
void CServer::PrepareResponseHeader_s( gd::argument::arguments& argumentHeader, boost::beast::http::response<boost::beast::http::string_body>& response )
{
   using namespace boost::beast::http;
   response.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   response.set(field::access_control_allow_origin, "*");   
   response.set(field::access_control_allow_methods, "GET, POST, HEAD, OPTIONS");
   response.set(field::access_control_allow_headers, "content-type, authorization, x-requested-with, accept, origin");
   response.set(field::access_control_max_age, "86400");

   [[ maybe_unused ]] auto u_ = std::distance( response.begin(), response.end() ); // suppress unused warning

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

boost::beast::http::response<boost::beast::http::string_body> CServer::PrepareResponse_s( const boost::beast::http::request<boost::beast::http::string_body>& request_, int iType, std::string_view stringContentType, std::string& stringBody )
{
   boost::beast::http::response<boost::beast::http::string_body> response_{boost::beast::http::status(iType), request_.version()};
   response_.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
   response_.set(boost::beast::http::field::content_type, stringContentType);
   response_.keep_alive(request_.keep_alive());
   if( stringBody.empty() == false ) { response_.body() = std::move( stringBody ); }
   response_.prepare_payload();

   return response_;
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

void listener::stop()
{
   boost::beast::error_code errorcodeCancel;
   m_acceptor.cancel( errorcodeCancel );

   boost::beast::error_code errorcodeClose;
   m_acceptor.close( errorcodeClose );

   m_iocontext.stop();
}

void listener::do_accept()
{
   // The new connection gets its own strand
   auto pstrand = boost::asio::make_strand(m_iocontext);                       // strand = "Sequential Execution", make sure each step is executed in order
   m_acceptor.async_accept( pstrand, boost::beast::bind_front_handler( &listener::on_accept, shared_from_this()));
}

void listener::on_accept(boost::beast::error_code errorcode, boost::asio::ip::tcp::socket socket)
{
   if( errorcode == boost::asio::error::operation_aborted ) { return; }       // stopped intentionally

   if(errorcode)
   {
      fail_g(errorcode, "accept");
      return;                                                                 // To avoid infinite loop
   }

   // ## Create the session and run it ......................................
   auto psession = std::make_shared<session>( std::move(socket), m_pstringFolderRoot);

   // ## check for proxy mode and set session information based on requested item flags
   const CServer* pserver_ = papplication_g->GetServer();                                          assert( pserver_ != nullptr );
   if( pserver_->IsProxy() == true ) { psession->SetFlags( CServer::eFlagProxy, 0u ); } // if server is in proxy mode, read session information based on requested item flags, for example read IP address of client

   psession->Run();

   do_accept();                                                               // Accept another connection
}


// ----------------------------------------------------------------------------
// -------------------------------------------------------------------- session
// ----------------------------------------------------------------------------

session::session( boost::asio::ip::tcp::socket&& socket, std::shared_ptr<std::string const> const& pstringFolderRoot)
   : m_tcpstream(std::move(socket)), m_pstringFolderRoot(pstringFolderRoot) {}

/** ------------------------------------------------------------------------- read
 * @brief Reads session information based on requested item flags.
 * @param uRequestItems Bit mask specifying which session items to read (e.g., `Types::eRequestItemIp` for IP address).
 */
void session::Read( uint64_t uRequestItems )
{
   // ## Read IP address ?
   if( (uRequestItems & Types::eRequestItemIp) == Types::eRequestItemIp )
   {
      try
      {
         if( IsProxy() == true )                                              // if server is in proxy mode, read IP address of client from request header, for example "X-Forwarded-For" or "X-Real-IP"
         {
            std::string_view stringRealIp = m_request["X-Forwarded-For"];
            if( stringRealIp.empty() == true ) { stringRealIp = m_request["x-forwarded-for"]; }
            
            // Om X-Forwarded-For saknas, kontrollera X-Real-IP
            if( stringRealIp.empty() == true ) { stringRealIp = m_request["X-Real-IP"]; }
            if( stringRealIp.empty() == true ) { stringRealIp = m_request["x-real-ip"]; }

            if( stringRealIp.empty() == false )
            {
               // Extrahera endast den första IP-adressen om det är en kommaseparerad lista
               auto uCommaPos = stringRealIp.find(',');
               if( uCommaPos != std::string_view::npos ) { stringRealIp = stringRealIp.substr( 0, uCommaPos ); }

               m_argument.append( "ip", stringRealIp );
               return;
            }
                                                                                                   // LOG_WARNING( "Proxy mode enabled but no real IP address found." );
         }

         auto endpoint_ = m_tcpstream.socket().remote_endpoint();
         auto stringIp = endpoint_.address().to_string();
         m_argument.append( "ip", stringIp );
      }
      catch( const std::exception& e )
      {  
         std::string stringError = "Failed to read IP address: " + std::string( e.what() );        LOG_ERROR( stringError );
         std::cout << stringError << std::endl;
      }
   }
}

void session::Run()
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

   m_tcpstream.expires_after(std::chrono::seconds(60));                        // Set the timeout.

   // Read a request
   boost::beast::http::async_read(m_tcpstream, m_flatbuffer, m_request, boost::beast::bind_front_handler( &session::on_read, shared_from_this()));
}

void session::on_read( boost::beast::error_code errorcode, std::size_t uBytesTransferred)
{                                                                                                  boost::ignore_unused(uBytesTransferred);
   if( errorcode == boost::beast::http::error::end_of_stream ) { return do_close(); } // This means they closed the connection

   if( errorcode ) { fail_g( errorcode, "read" ); return do_close(); }            // if an error occurs, log the error and close the connection

   // ## Check for what type of information that is read from all incoming connections
   const auto* pdocument = papplication_g->GetDocument();
   if( pdocument != nullptr ) { Read( pdocument->GetRequestFlags() ); }       // read session information based on requested item flags

   // This means they closed the connection
   if(errorcode == boost::beast::http::error::end_of_stream) { return do_close(); }

   if(errorcode) { fail_g(errorcode, "read"); return do_close(); }

   // Send the response
   auto message_ = handle_request(*m_pstringFolderRoot, std::move(m_request), this);
   //auto u_ = std::distance( response.begin(), response.end() ); // suppress unused warning
   send_response( std::move( message_ ) );
}

void session::send_response(boost::beast::http::message_generator&& messagegenerator)
{
   bool bKeepAlive = messagegenerator.keep_alive();                            // get current keep alive status

   // Write the response
   boost::beast::async_write( m_tcpstream,std::move(messagegenerator), boost::beast::bind_front_handler( &session::on_write, shared_from_this(), bKeepAlive));
}

void session::on_write( bool bKeepAlive, boost::beast::error_code errorcode, std::size_t uBytesTransferred)
{                                                                                                  boost::ignore_unused(uBytesTransferred);
   if(errorcode) { fail_g(errorcode, "write"); return do_close(); }

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
