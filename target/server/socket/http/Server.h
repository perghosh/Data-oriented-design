/** 
 * @file Server.h
 * 
 * @brief Logic to handle ip trafic to and from http server
 *
 *
 *
 \code
 \endcode
 */

#pragma once

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include "gd/gd_com.h"
#include "gd/com/gd_com_server.h"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"

#include "command/Router.h"

#include "Application.h"


// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type_g(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat_g( boost::beast::string_view base,  boost::beast::string_view path);

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CServer
{
// ## construction -------------------------------------------------------------
public:
   CServer() {}
   /// Constructor that sets application pointer
   CServer(CApplication* ppapplication) : m_ppapplication(ppapplication) {}
   // copy
   CServer(const CServer& o) { common_construct(o); }
   CServer(CServer&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CServer& operator=(const CServer& o) { common_construct(o); return *this; }
   CServer& operator=(CServer&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CServer() {}
private:
   // common copy
   void common_construct(const CServer& o) {}
   void common_construct(CServer&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> Initialize();
   std::pair<bool, std::string> ProcessRequest(boost::beast::http::verb eVerb, std::string_view stringTarget, std::vector<std::pair<std::string, std::string>>& vectorResponse);
   std::pair<bool, std::string> Execute( gd::com::server::command_i* pcommand );
   std::pair<bool, std::string> Execute( const std::vector<std::string_view>& vectorCommand, gd::com::server::command_i* pcommand );
//@}

/** \name ROUTER
*///@{
   gd::com::server::server_i* ROUTER_GetServer( const std::string_view& stringServer ) { return m_router.GetServer( stringServer ); }
//@}


protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   CApplication* m_ppapplication{}; ///< application pointer, access application that is used as object root for server
   CRouter m_router;                ///< command router


// ## free functions ------------------------------------------------------------
public:



};

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
boost::beast::http::message_generator handle_request( boost::beast::string_view stringRoot, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& request_)
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
   if( eVerb != boost::beast::http::verb::get && eVerb != boost::beast::http::verb::head) 
   { 
      return bad_request_("Unknown HTTP-method"); 
   }

   std::string_view stringTarget = request_.target();
   if( stringTarget.size() > 0 && stringTarget[0] == '/' ) { stringTarget.remove_prefix(1); }

   // ## Resolve target

   // ## Request path must be absolute and not contain "..".
   if( request_.target().empty() || request_.target()[0] != '/' || request_.target().find("..") != boost::beast::string_view::npos) 
   { 
      return bad_request_("Illegal request-target"); 
   }

   {
      // ## Process request by calling core method in application
      std::vector<std::pair<std::string, std::string>> vectorResponse;
      auto resulut_ = papplication_g->GetServer()->ProcessRequest( eVerb, stringTarget, vectorResponse );
      if ( resulut_.first == false ) { return server_error_(resulut_.second); }
      // copilot: implement ProcessRequest method in CApplication

   }

   // ## Build the path to the requested file
   std::string stringPath = path_cat_g(stringRoot, request_.target());
   if(request_.target().back() == '/') { stringPath.append("index.html"); }
   else
   {                                                                                               LOG_DEBUG_RAW( stringPath );
      
   }

   // TODO: rewrite request logic
   // papplication_g->GetRouter()->Get( stringPath );

   // ## Attempt to open the file
   boost::beast::error_code errorcode;
   boost::beast::http::file_body::value_type body_;
   body_.open(stringPath.c_str(), boost::beast::file_mode::scan, errorcode);
   if(errorcode == boost::beast::errc::no_such_file_or_directory) { return not_found_(request_.target()); } // Handle the case where the file doesn't exist
  
   if(errorcode) { return server_error_(errorcode.message()); }                // Handle an unknown error

   
   auto const uSize = body_.size();                                            // Cache the size since we need it after the move

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

//------------------------------------------------------------------------------

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
public:
   // Transfer ownership of the stream
   session( boost::asio::ip::tcp::socket&& socket, std::shared_ptr<std::string const> const& pstringFolderRoot);

// ## methods -----------------------------------------------------------------
   // Start the asynchronous request
   void run();

   void do_read();
   void on_read( boost::beast::error_code errorcode, std::size_t uBytesTransferred);
   void send_response(boost::beast::http::message_generator&& messagegenerator);
   void on_write( bool bKeepAlive, boost::beast::error_code errorcode, std::size_t uBytesTransferred);
   void do_close();

// ## attributes --------------------------------------------------------------
public:
   boost::beast::tcp_stream m_tcpstream;        ///< Stream data using socket
   boost::beast::flat_buffer m_flatbuffer;      ///< Buffer to store data used in request
   std::shared_ptr<std::string const> m_pstringFolderRoot;///< root folder on disk where to find files
   boost::beast::http::request<boost::beast::http::string_body> m_request;///< Handle parts in http message
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
public:
   listener( boost::asio::io_context& iocontext_, boost::asio::ip::tcp::endpoint endpoint_, std::shared_ptr<std::string const> const& pstringFolderRoot );

// ## methods -----------------------------------------------------------------
   // Start accepting incoming connections
   void run() { do_accept(); }

private:
   void do_accept();
   void on_accept(boost::beast::error_code errorcode, boost::asio::ip::tcp::socket socket);

// ## attributes --------------------------------------------------------------
public:
   boost::asio::io_context& m_iocontext; ///< composite class for boost I/O network classes 
   boost::asio::ip::tcp::acceptor m_acceptor; ///< Handle new socket connections
   std::shared_ptr<std::string const> m_pstringFolderRoot;///< root folder on disk where to find files
};

