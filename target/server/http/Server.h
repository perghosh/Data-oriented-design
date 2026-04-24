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

#include "gd/gd_arguments_shared.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"

#include "Application.h"

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type_g(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat_g( boost::beast::string_view base,  boost::beast::string_view path);

class session; // Forward declaration of session class to avoid circular dependency
class listener;

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
   CServer();
   /// Constructor that sets application pointer
   CServer(CApplication* ppapplication);
   // copy
   CServer(const CServer& o);
   CServer(CServer&& o) noexcept;
   // assign
   CServer& operator=(const CServer& o);
   CServer& operator=(CServer&& o) noexcept;

   ~CServer();

private:
   // common copy
void common_construct(const CServer& o);
void common_construct(CServer&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
   // @API [tag: get, set]
      /// Get application pointer
   CApplication* GetApplication() const { return m_ppapplication; }
   void SetApplication( CApplication* ppapplication ) { m_ppapplication = ppapplication; }

   std::shared_ptr<listener> GetListener() const;
   void SetListener( std::shared_ptr<listener> plistener );

/** \name OPERATION
*///@{
   std::pair<bool, std::string> Initialize();

   /// Route command
   boost::beast::http::message_generator RouteCommand( std::string_view stringTarget, std::string_view stringBody, boost::beast::http::request<boost::beast::http::string_body>&& request_, const session* psession_ );

   std::pair<bool, std::string> Execute( const std::vector<std::string_view>& vectorCommand, gd::com::server::command_i* pcommand );

   
//@}

/** \name ROUTER
*///@{
   //gd::com::server::server_i* ROUTER_GetServer( const std::string_view& stringServer ) { return m_router.GetServer( stringServer ); }
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
   std::shared_ptr<listener> m_plistener; ///< listener object that accepts incoming connections and launches sessions to handle them

// ## free functions ------------------------------------------------------------
public:
   /// Prepares response header for request
   void PrepareResponseHeader_s( gd::argument::arguments& argumentHeader, boost::beast::http::response<boost::beast::http::string_body>& response );
   boost::beast::http::response<boost::beast::http::string_body> PrepareResponse_s( const boost::beast::http::request<boost::beast::http::string_body>& request_, int iType, std::string_view stringContentType, std::string& stringBody );


};

boost::beast::http::message_generator handle_request( boost::beast::string_view stringRoot, boost::beast::http::request<boost::beast::http::string_body>&& request_);


//------------------------------------------------------------------------------

/**  ======================================================================== session
 * @brief **HTTP session** handling one TCP connection with asynchronous Beast operations.
 *
 * Lifecycle:
 * 1. `run()` starts processing.
 * 2. `do_read()` / `on_read(...)` receive and parse request data.
 * 3. `send_response(...)` / `on_write(...)` send reply and evaluate keep-alive.
 * 4. `do_close()` shuts down the socket when needed.
 *
 * `m_argument` is backed by `m_array_` to reduce dynamic allocations for per-request argument handling.
 */
class session : public std::enable_shared_from_this<session>
{
public:
   // Transfer ownership of the stream
   session( boost::asio::ip::tcp::socket&& socket, std::shared_ptr<std::string const> const& pstringFolderRoot);

// ## methods -----------------------------------------------------------------
 
   void read( uint64_t uRequestItems );
 
   // Start the asynchronous request
   void run();

   void do_read();
   void on_read( boost::beast::error_code errorcode, std::size_t uBytesTransferred);
   void send_response(boost::beast::http::message_generator&& messagegenerator);
   void on_write( bool bKeepAlive, boost::beast::error_code errorcode, std::size_t uBytesTransferred);
   void do_close();

   bool exists( std::string_view stringName ) const { return m_argument.exists( stringName ); }
   std::string as_string( std::string_view stringName ) const { return m_argument[stringName].as_string(); }

// ## attributes --------------------------------------------------------------
public:
   boost::beast::tcp_stream m_tcpstream;        ///< Stream data using socket
   boost::beast::flat_buffer m_flatbuffer;      ///< Buffer to store data used in request
   std::shared_ptr<std::string const> m_pstringFolderRoot;///< root folder on disk where to find files
   boost::beast::http::request<boost::beast::http::string_body> m_request;///< Handle parts in http message
   std::array<std::byte, 64> m_array_; ///< buffer to avoid dynamic memory allocation for arguments
   gd::argument::arguments m_argument{m_array_}; ///< arguments for session, "ip", "session" are common
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
   void stop(); ///< Stop accepting incoming connections and close acceptor

private:
   void do_accept();
   void on_accept(boost::beast::error_code errorcode, boost::asio::ip::tcp::socket socket);

// ## attributes --------------------------------------------------------------
public:
   boost::asio::io_context& m_iocontext; ///< composite class for boost I/O network classes 
   boost::asio::ip::tcp::acceptor m_acceptor; ///< Handle new socket connections
   std::shared_ptr<std::string const> m_pstringFolderRoot;///< root folder on disk where to find files
};
