// @FILE tag: session] [description: Session management for HTTP server, manage user sessions] [type: header]

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_table_arguments.h"

/**
 * \brief Manages user sessions for the HTTP server.
 * 
 * Session is used to check users and they are stored inside table member called m_tableSession.
 *
 * This class is responsible for creating, copying, moving, and destroying session objects.
 *
 * \code
 * \endcode
 */
class CSession
{
public:
   CSession() {}
   // copy
   CSession(const CSession& o) { common_construct(o); }
   CSession(CSession&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CSession& operator=(const CSession& o) { common_construct(o); return *this; }
   CSession& operator=(CSession&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CSession() {}
private:
   void construct();
   // common copy
   void common_construct(const CSession& o) {}
   void common_construct(CSession&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:

protected:
   /** \name INTERNAL
   *///@{

   //@}

public:
   /// Create session session object and initialize internal data to start working with sessions
   std::pair<bool, std::string> Create( std::size_t uMaxSessionCount );


   // ## attributes ----------------------------------------------------------------
public:
   gd::table::arguments::table m_tableSession;  ///< Table holding session data


   // ## free functions ------------------------------------------------------------
public:



};