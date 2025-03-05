/**
* \file command/Database.h
* 
* \brief Handle database actions
* Metods to handle database actions like open, close, execute and ask databases for information.
* 
*/

#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>

#include "gd/gd_strings.h"
#include "gd/com/gd_com_server.h"

#include "Router.h"

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CRouterDatabase : public gd::com::server::router::server
{
// ## construction -------------------------------------------------------------
public:
   CRouterDatabase() {}
   /// create database with name
   CRouterDatabase(const std::string_view& stringName) : m_stringName(stringName) {}
   CRouterDatabase(const std::string_view& stringName, const std::string_view& stringShort) : m_stringName(stringName), m_stringShortName( stringShort ) {}
   // copy
   CRouterDatabase(const CRouterDatabase& o) { common_construct(o); }
   CRouterDatabase(CRouterDatabase&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CRouterDatabase& operator=(const CRouterDatabase& o) { common_construct(o); return *this; }
   CRouterDatabase& operator=(CRouterDatabase&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CRouterDatabase() {}
private:
   // common copy
   void common_construct(const CRouterDatabase& o) {}
   void common_construct(CRouterDatabase&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name INTERFACE
*///@{
   /// Check if command is endpoint
   bool is_endpoint(const std::string_view& stringCommand) override { return (m_stringShortName == stringCommand || m_stringName == stringCommand); }
   std::pair<bool, std::string> get( gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse) override;
//@}

/** \name OPERATION
*///@{
   /// Execute resolvs the command and executes it
   std::pair<bool, std::string> Execute(const std::string_view& stringCommand, gd::com::server::command_i* pCommand, gd::com::server::response_i* presponse );
   //std::pair<bool, std::string> Execute( const gd::strings32& strings32Command, gd::com::server::command_i* pCommand, gd::com::server::response_i* presponse );
//@}

protected:
/** \name INTERNAL
*///@{
   std::pair<bool, std::string> CreateDatabase( const gd::argument::arguments& arguments_ );
   std::pair<bool, std::string> RemoveDatabase( const gd::argument::arguments& arguments_ );
//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   std::string m_stringName; ///< Name of the database server
   std::string m_stringShortName; ///< Short mame for the database server


// ## free functions ------------------------------------------------------------
public:



};