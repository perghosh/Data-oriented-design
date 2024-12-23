#pragma once

/*
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>

https://www.boost.io/doc/libs/1_68_0/libs/beast/example/http/client/async/http_client_async.cpp
*/

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

// https://www.boost.io/doc/libs/1_68_0/libs/beast/example/http/client/async/http_client_async.cpp
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

//------------------------------------------------------------------------------

// Report a failure
inline void fail(boost::system::error_code ec, char const* what)
{
   std::cerr << what << ": " << ec.message() << "\n";
}

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
   tcp::resolver resolver_;
   tcp::socket socket_;
   boost::beast::flat_buffer buffer_; // (Must persist between reads)
   http::request<http::empty_body> req_;
   http::response<http::string_body> res_;

public:
   // Resolver and socket require an io_context
   explicit
      session(boost::asio::io_context& ioc)
      : resolver_(ioc)
      , socket_(ioc)
   {
   }

   // Start the asynchronous operation
   void run( char const* host, char const* port, char const* target, int version)
   {
      // Set up an HTTP GET request message
      req_.version(version);
      req_.method(http::verb::get);
      req_.target(target);
      req_.set(http::field::host, host);
      req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      // Look up the domain name
      resolver_.async_resolve(
         host,
         port,
         std::bind(
            &session::on_resolve,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
   }

   void on_resolve( boost::system::error_code ec, tcp::resolver::results_type results)
   {
      if(ec)
         return fail(ec, "resolve");

      // Make the connection on the IP address we get from a lookup
      boost::asio::async_connect(
         socket_,
         results.begin(),
         results.end(),
         std::bind(
            &session::on_connect,
            shared_from_this(),
            std::placeholders::_1));
   }

   void on_connect(boost::system::error_code ec)
   {
      if(ec)
         return fail(ec, "connect");

      // Send the HTTP request to the remote host
      http::async_write(socket_, req_,
         std::bind(
            &session::on_write,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
   }

   void on_write( boost::system::error_code ec, std::size_t bytes_transferred)
   {
      boost::ignore_unused(bytes_transferred);

      if(ec)
         return fail(ec, "write");

      // Receive the HTTP response
      http::async_read(socket_, buffer_, res_,
         std::bind(
            &session::on_read,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
   }

   void on_read( boost::system::error_code ec, std::size_t bytes_transferred )
   {
      boost::ignore_unused(bytes_transferred);

      if(ec)
         return fail(ec, "read");

      // Write the message to standard out
      std::cout << res_ << std::endl;

      // Gracefully close the socket
      socket_.shutdown(tcp::socket::shutdown_both, ec);

      // not_connected happens sometimes so don't bother reporting it.
      if(ec && ec != boost::system::errc::not_connected)
         return fail(ec, "shutdown");

      // If we get here then the connection is closed gracefully
   }
};


/*

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
public:
   listener(State &state, boost::asio::ip::tcp::endpoint endpoint):
      state_(state),
      m_acceptor(make_strand(state.ioc))
   {
      boost::beast::error_code errorcode_;

      // Open the acceptor
      m_acceptor.open(endpoint.protocol(), errorcode_);
      if(errorcode_)
      {
         fail(errorcode_, "open");
         return;
      }

      // Allow address reuse
      m_acceptor.set_option(net::socket_base::reuse_address(true), errorcode_);
      if(errorcode_)
      {
         fail(errorcode_, "set_option");
         return;
      }

      // Bind to the server address
      m_acceptor.bind(endpoint, errorcode_);
      if(errorcode_)
      {
         fail(errorcode_, "bind");
         return;
      }

      // Start listening for connections
      std::cout << "Listening for connections on endpoint " << endpoint << '\n';
      m_acceptor.listen( net::socket_base::max_listen_connections, errorcode_);
      if(errorcode_)
      {
         fail(errorcode_, "listen");
         return;
      }
   }

   // Start accepting incoming connections
   void run()
   {
      do_accept();
   }


   void do_accept();

   void on_accept(beast::error_code ec, boost::asio::ip::tcp::socket socket)
   {
      if(ec)
      {
         fail(ec, "accept");
         return; // To avoid infinite loop
      }
      else
      {
         // Create the session and run it
         std::make_shared<detect_session>(state_, std::move(socket))->run();
      }

      // Accept another connection
      do_accept();
   }

   State        &state_;
   boost::asio::ip::tcp::acceptor m_acceptor;
};
*/