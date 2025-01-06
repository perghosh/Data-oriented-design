#pragma once

#include "../command/Router.h"

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CHttpServer : public gd::com::server::router::server
{
// ## construction -------------------------------------------------------------
public:
   CHttpServer() {}
   // copy
   CHttpServer(const CHttpServer& o) { common_construct(o); }
   CHttpServer(CHttpServer&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CHttpServer& operator=(const CHttpServer& o) { common_construct(o); return *this; }
   CHttpServer& operator=(CHttpServer&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CHttpServer() {}
private:
   // common copy
   void common_construct(const CHttpServer& o) {}
   void common_construct(CHttpServer&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> get( const std::string_view* stringCommandList, const gd::argument::arguments* pargumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse ) override ;
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