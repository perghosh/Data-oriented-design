#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// https://www.boost.org/doc/libs/1_87_0/libs/beast/example/http/server/async/http_server_async.cpp

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type_g(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat_g( boost::beast::string_view base,  boost::beast::string_view path);

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
boost::beast::http::message_generator handle_request( boost::beast::string_view doc_root, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& request_)
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

   // ## Make sure we can handle the method
   if( request_.method() != boost::beast::http::verb::get &&
       request_.method() != boost::beast::http::verb::head) { return bad_request_("Unknown HTTP-method"); }

   // ## Request path must be absolute and not contain "..".
   if( request_.target().empty() ||
       request_.target()[0] != '/' ||
       request_.target().find("..") != boost::beast::string_view::npos) { return bad_request_("Illegal request-target"); }

   // ## Build the path to the requested file
   std::string stringPath = path_cat_g(doc_root, request_.target());
   if(request_.target().back() == '/') { stringPath.append("index.html"); }

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
      boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::ok, request_.version()};
      res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
      res.set(boost::beast::http::field::content_type, mime_type_g(stringPath));
      res.content_length(uSize);
      res.keep_alive(request_.keep_alive());
      return res;
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