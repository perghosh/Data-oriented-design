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

#include "api/API_Base.h"

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
      eFlagPrepared           = 0x00000002,
   };

   enum enumResultFormat
   {
      eResultFormatXml        = 0x00000100,  // Xml formatde data
      eResultFormatJson       = 0x00000200,  // Json formated data
      eResultFormatBinary     = 0x00000400,  // For images, files, etc.
      eResultFormatMultipart  = 0x00000800,  // For multipart/form-data
   };

   enum enumRequestFormat
   {
      eRequestFormatNone      = 0x00000000,
      eRequestFormatXml       = 0x00010000,
      eRequestFormatJson      = 0x00020000,
      eRequestFormatAny       = 0x00030000,
      eRequestFormatMask      = 0x00030000,
   };

public:
   CRouter() {}
   CRouter(CApplication* pApplication): m_context(pApplication) {}
   CRouter(CApplication* pApplication, CDocument* pDocument): m_context(pApplication, pDocument) {}
   CRouter( const std::string_view& stringQueryString ) : m_stringQueryString( stringQueryString ) {}
   CRouter( CApplication* pApplication, const std::string_view& stringQueryString ): m_context(pApplication), m_stringQueryString(stringQueryString) {}
   CRouter( CApplication* pApplication, const std::string_view& stringQueryString, std::string_view stringBody ): m_context(pApplication), m_stringQueryString(stringQueryString), m_stringBody(stringBody) {}
   // copy
   CRouter( const CRouter& o ) { common_construct( o ); }
   CRouter( CRouter&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CRouter& operator=( const CRouter& o ) { common_construct( o ); return *this; }
   CRouter& operator=( CRouter&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CRouter();
private:
   // common copy
   void common_construct( const CRouter& o ) {}
   void common_construct( CRouter&& o ) noexcept {}

// @API [tag: operator] [description: operators for router]
public:

// @API [tag: getter, setter] [description: get and set router attributes]
public:
   void SetFlag( unsigned uFlag ) { m_uFlags |= uFlag; }
   void ClearFlag( unsigned uFlag ) { m_uFlags &= ~uFlag; }
   void SetFlags( unsigned uFlags ) { m_uFlags = uFlags; }
   void SetFlags( unsigned uSet, unsigned uClear ) { m_uFlags = ( m_uFlags | uSet ) & ~uClear; }

   bool IsCommand() const { return (m_uFlags & eFlagCommand) != 0; }
   bool IsPrepared() const { return ( m_uFlags & eFlagPrepared ) != 0; }

   bool IsBody() const { return m_stringBody.empty() == false; }
   bool IsRequestFormatNone() const { return ( m_uFlags & eRequestFormatMask ) == 0; }
   bool IsRequestFormatXml() const { return ( m_uFlags & eRequestFormatMask ) == eRequestFormatXml; }
   bool IsRequestFormatJson() const { return ( m_uFlags & eRequestFormatMask ) == eRequestFormatJson; }
   void* GetRequestData() const { return m_pairRequestData.second; }

   bool IsXml() const { return ( m_uFlags & eResultFormatXml ) == eResultFormatXml; }
   bool IsJson() const { return ( m_uFlags & eResultFormatJson ) == eResultFormatJson; }
   bool IsBinary() const { return ( m_uFlags & eResultFormatBinary ) == eResultFormatBinary; }
   bool IsMultipart() const { return ( m_uFlags & eResultFormatMultipart ) == eResultFormatMultipart; }

   void SetResponseData( unsigned uType, void* pData ) { m_pairRequestData = { uType, pData }; }
   
   // Convenience accessors that forward to m_context so existing call-sites
   // in Run() / Prepare() / PrintResponseXml() do not need to change.
   CApplication* GetApplication()             { return m_context.GetApplication(); }
   const CApplication* GetApplication() const { return m_context.GetApplication(); }   
   CDocument*    GetDocument()                { return m_context.GetDocument(); }
   const CDocument* GetDocument() const       { return m_context.GetDocument(); }
 

// ## methods ------------------------------------------------------------------
public:
   std::pair<bool, std::string> Parse();
   std::pair<bool, std::string> Run( const std::vector<std::string_view>& vectorCommand, gd::argument::arguments& argumentsParameter );
   std::pair<bool, std::string> Run();

   /// Check if router has result to deliver to client
   bool HasResult();

   std::pair<bool, std::string> Prepare();

   std::pair<bool, std::string> PrintResponseXml( std::string& stringXml, const gd::argument::arguments* parguments_ );

   template<typename APIObject>
   std::pair<bool, std::string> ExecuteCommand_( const std::vector<std::string_view>& vectorPath, const gd::argument::arguments& arguments_, unsigned& uCommandIndex);

// ## attributes ----------------------------------------------------------------
public:
   CAPIContext  m_context;             ///< context object that holds application and document pointers, response data and other useful information for command handlers

   unsigned m_uFlags{};                ///< router flags
   unsigned m_uUserIndex{};            ///< user index for user in session table for users logged in

   std::string m_stringQueryString;    ///< query string from url
   std::string_view m_stringBody;      ///< body from http request
   std::vector<std::string_view> m_vectorCommand; ///< list of commands parsed from query string
   std::unique_ptr<CDTOResponse> m_pdtoresponse;  ///< response dto object
   std::mutex m_mutexRouter;         ///< mutex for response dto object
   std::pair<unsigned, void*> m_pairRequestData{ 0, nullptr }; ///< pair of request data, first is type and second is pointer to data

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
