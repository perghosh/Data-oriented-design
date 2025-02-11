#pragma once

#include "gd/com/gd_com_server.h"

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CRouter
{
// ## construction -------------------------------------------------------------
public:
   CRouter() {}
   // copy
   CRouter(const CRouter& o) { common_construct(o); }
   CRouter(CRouter&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CRouter& operator=(const CRouter& o) { common_construct(o); return *this; }
   CRouter& operator=(CRouter&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CRouter();
private:
   // common copy
   void common_construct(const CRouter& o) {}
   void common_construct(CRouter&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   std::pair< bool, std::string > Connect( gd::com::server::server_i* pserver );

   std::pair< bool, std::string > Get( const std::string_view& stringArgument );

   std::pair< bool, std::string > Get( std::vector<std::string_view>& vectorCommand );

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
   /// Connected servers
   std::vector<gd::com::server::server_i*> m_vectorServer;

// ## free functions ------------------------------------------------------------
public:

};


