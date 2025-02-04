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
class CRouterDatabase
{
// ## construction -------------------------------------------------------------
public:
   CRouterDatabase() {}
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

/** \name OPERATION
*///@{
   std::pair<bool, std::string> Execute(const std::string_view& stringCommand, gd::com::server::command_i* pCommand, gd::com::server::response_i* presponse );
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


// ## free functions ------------------------------------------------------------
public:



};