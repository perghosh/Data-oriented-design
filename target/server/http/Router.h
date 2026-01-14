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
class CDocument;


/** @CLASS [tag: router, http] [description: Router class for http server] [name: CRouter]
 * \brief
 *
 * Router is used to parse query string from http request and route to proper command handler inside in http application.
 * There are a number of built in commands inside http application that can be used to manage lots of operations.
 * 
 * Router are also able to run endpoints that goes to scripts. (not implemented yet)
 * 
 * The router holds the response dto object that is used to store response data that will be sent back to client.
 * Command handlers will add/transfer data to response dto object. The logic for this is optimized to handle large data sets with minimal memory usage and copying
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

   enum enumResultFormat
   {
      eResultFormatXml        = 0x00000000,
      eResultFormatJson       = 0x00000100,
   };

public:
   CRouter() {}
   CRouter(CApplication* pApplication): m_pApplication(pApplication) {}
   CRouter(CApplication* pApplication, CDocument* pDocument): m_pApplication(pApplication), m_pDocument(pDocument) {}
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

   bool IsXml() const { return ( m_uFlags & eResultFormatJson ) == 0; }
   bool IsJson() const { return ( m_uFlags & eResultFormatJson ) == eResultFormatJson; }
   
   CRouter& operator=( CApplication* pApplication ) { m_pApplication = pApplication; return *this; }
   CRouter& operator=( CDocument* pDocument ) { m_pDocument = pDocument; return *this; }


// ## methods ------------------------------------------------------------------
public:
   std::pair<bool, std::string> Parse();
   std::pair<bool, std::string> Run( const std::vector<std::string_view>& vectorCommand, gd::argument::arguments& argumentsParameter );
   std::pair<bool, std::string> Run();

   /// Check if router has result to deliver to client
   bool HasResult();

   std::pair<bool, std::string> PrintResponseXml( std::string& stringXml, const gd::argument::arguments* parguments_ );

// ## attributes ----------------------------------------------------------------
public:
   CApplication* m_pApplication{};     ///< application instance
   CDocument* m_pDocument{};           ///< document instance
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

/// @brief Check if router has result to deliver to client
inline bool CRouter::HasResult()
{
   if( m_pdtoresponse != nullptr && m_pdtoresponse->Empty() == false  ) { return true; }
   return false;
}

inline std::pair<bool, std::string> CRouter::PrintResponseXml( std::string& stringXml, const gd::argument::arguments* parguments_ )
{
   if( m_pdtoresponse == nullptr )
   {
      return { false, "No response dto object in router" };
   }
   return m_pdtoresponse->PrintXml( stringXml, parguments_ );
}
