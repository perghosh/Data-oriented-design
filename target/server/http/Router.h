// @FILE [tag: router, http] [summary: Router class for http server] [type: header]

#pragma once

#include <cassert>

#include <print>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>


#include "gd/gd_arguments.h"

#include "dto/DTOResponse.h"

class CApplication;


/** @CLASS [tag: router, http] [description: Router class for http server] [name: CRouter]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CRouter
{
public:
   enum enumFlag
   {
      eFlagNone               = 0x00000000,
      eFlagCommand            = 0x00000001,
   };

public:
   CRouter() {}
   CRouter(CApplication* pApplication): m_pApplication(pApplication) {}
   CRouter( const std::string_view& stringQueryString ) : m_stringQueryString( stringQueryString ) {}
   CRouter( CApplication* pApplication, const std::string_view& stringQueryString ): m_pApplication(pApplication), m_stringQueryString(stringQueryString) {}
   // copy
   CRouter( const CRouter& o ) { common_construct( o ); }
   CRouter( CRouter&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CRouter& operator=( const CRouter& o ) { common_construct( o ); return *this; }
   CRouter& operator=( CRouter&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CRouter() {}
private:
   // common copy
   void common_construct( const CRouter& o ) {}
   void common_construct( CRouter&& o ) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   bool IsCommand() const { return (m_uFlags & eFlagCommand) != 0; }


// ## methods ------------------------------------------------------------------
public:
   std::pair<bool, std::string> Parse();
   std::pair<bool, std::string> Run( const std::vector<std::string_view>& vectorCommand, gd::argument::arguments& argumentsParameter );
   std::pair<bool, std::string> Run();
public:


// ## attributes ----------------------------------------------------------------
public:
   CApplication* m_pApplication;       ///< application instance
   unsigned m_uFlags{};                ///< router flags
   unsigned m_uUserIndex{};            ///< user index for user in session table for users logged in
   std::string m_stringQueryString;    ///< query string from url
   std::vector<std::string_view> m_vectorCommand; ///< list of commands parsed from query string
   std::unique_ptr<CDTOResponse> m_pdtoresponse;  ///< response dto object


// ## free functions ------------------------------------------------------------
public:

   /// Encode values in arguments for specified names in vectorName
   static std::pair<bool, std::string> Encode_s( gd::argument::arguments& arguments_, const std::vector<std::string>& vectorName );


};
