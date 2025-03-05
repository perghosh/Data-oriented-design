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
class CRouterApplication : public gd::com::server::router::server
{
   // ## construction -------------------------------------------------------------
public:
   CRouterApplication() {}
   /// create database with name
   CRouterApplication(const std::string_view& stringName) : m_stringName(stringName) {}
   CRouterApplication(const std::string_view& stringName, const std::string_view& stringShort) : m_stringName(stringName), m_stringShortName( stringShort ) {}
   // copy
   CRouterApplication(const CRouterApplication& o) { common_construct(o); }
   CRouterApplication(CRouterApplication&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CRouterApplication& operator=(const CRouterApplication& o) { common_construct(o); return *this; }
   CRouterApplication& operator=(CRouterApplication&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CRouterApplication() {}
private:
   // common copy
   void common_construct(const CRouterApplication& o) {}
   void common_construct(CRouterApplication&& o) noexcept {}

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
   //@}

public:
   /** \name DEBUG
   *///@{

   //@}


   // ## attributes ----------------------------------------------------------------
public:
   std::string m_stringName; ///< Name of the application server
   std::string m_stringShortName; ///< Short  name for the application server


   // ## free functions ------------------------------------------------------------
public:



};