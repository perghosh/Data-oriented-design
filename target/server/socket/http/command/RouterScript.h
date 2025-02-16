/**
* \file command/RouterScript.h
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
class CRouterScript : public gd::com::server::router::server
{
   // ## construction -------------------------------------------------------------
public:
   CRouterScript() {}
   /// create database with name
   CRouterScript(const std::string_view& stringName) : m_stringName(stringName) {}
   // copy
   CRouterScript(const CRouterScript& o) { common_construct(o); }
   CRouterScript(CRouterScript&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CRouterScript& operator=(const CRouterScript& o) { common_construct(o); return *this; }
   CRouterScript& operator=(CRouterScript&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CRouterScript() {}
private:
   // common copy
   void common_construct(const CRouterScript& o) {}
   void common_construct(CRouterScript&& o) noexcept {}

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
   bool is_endpoint(const std::string_view& stringCommand) override { return m_stringName == stringCommand; }
//@}


/** \name OPERATION
*///@{
   /// Execute resolvs the command and executes it
   // std::pair<bool, std::string> Execute(const std::string_view& stringCommand, gd::com::server::command_i* pCommand, gd::com::server::response_i* presponse );
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
   std::string m_stringName; ///< Name of the database server


   // ## free functions ------------------------------------------------------------
public:



};