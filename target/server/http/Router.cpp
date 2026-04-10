// @FILE [tag: router, http] [summary: Router class for http server] [type: source]

#include "gd/parse/gd_parse_uri.h"
#include "gd/gd_utf8.h"

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "api/APIDatabase.h"
#include "api/APISql.h"
#include "api/APISystem.h"


#include "Router.h"

//static std::tuple< gd::com::pointer< gd::com::server::router::command >, gd::com::pointer< gd::com::server::router::response > > make_command_s(gd::com::server::server_i* pserver );

/*
@TASK [tag: url, format, command][status: ongoing] [assigned: per]
[title: design url format for commands]
[description: """
Design a URL format for commands that allows easy parsing and execution of server commands.
sample http://127.0.0.1/!db/create?name=testdb&user=admin
- db/create?name=testdb&user=admin
- db/delete?name=testdb
- db/table/create?name=TUser
- db/column/create?table=TUser&name=FColumnName&type=int32
- db/select?query=SELECT * FROM TUser WHERE FColumnName=100
- db/insert?table=TUser&values=(FColumnName=100,FAnotherColumn='text')
- db/update?table=TUser&set=(FAnotherColumn='newtext')&where=(FColumnName=100)
"""]
*/

CRouter::~CRouter()
{
   // ## clean up request data if any

   if( m_pairRequestData.second != nullptr )
   {
      // @CRITICAL [tag: router, request data] [description: Clean up request data based on its type to prevent memory leaks]
      unsigned uType = m_pairRequestData.first;
      void* pData = m_pairRequestData.second;
      if( uType == eRequestFormatXml )
      {
         pugi::xml_document* pdocument = static_cast<pugi::xml_document*>( pData );
         delete pdocument;
      }
      else if( uType == eRequestFormatJson )
      {
         jsoncons::json* pjson = static_cast<jsoncons::json*>( pData );
         delete pjson;
      }
   }
}


std::pair<bool, std::string> CRouter::Parse()
{                                                                                                   assert( m_stringQueryString.empty() == false );
   std::string_view stringQueryStringView = m_stringQueryString;
   if( stringQueryStringView[0] == '!' )
   {
      stringQueryStringView.remove_prefix( 1 );
      m_stringQueryString = stringQueryStringView;
      m_uFlags |= eFlagCommand;
   }
   else
   {
      m_uFlags &= ~eFlagCommand;
   }

   return { true, "" };
}

/** -------------------------------------------------------------------------
 * @brief Template method that handles the common execution pattern for all API command objects.
 *        Expects APIObject to expose: Execute(), GetObjects(), GetCommandIndex(),
 *        and a constructor matching (application, path, arguments, commandIndex).
 * @return pair of success flag and status message
 */
 /*
template<typename APIObject>
std::pair<bool, std::string> CRouter::ExecuteCommand_( const std::vector<std::string_view>& vectorPath, const gd::argument::arguments& arguments_, unsigned& uCommandIndex)
{
   APIObject apiobject_( m_pApplication, vectorPath, arguments_, uCommandIndex );
   auto result_ = apiobject_.Execute();                                       // execute command based on command

   if( result_.first == true )                                                // if success get objects from api
   {
      Types::Objects* pobjectsResult = apiobject_.GetObjects();                                    assert( pobjectsResult );
      if( pobjectsResult != nullptr && pobjectsResult->Empty() == false )
      {
         result_ = m_pdtoresponse->AddTransfer( pobjectsResult );             // add objects to response dto
      }
   }

   uCommandIndex = apiobject_.GetCommandIndex();                              // get current active command index
   return result_;
}
*/

template<typename APIObject>
std::pair<bool, std::string> CRouter::ExecuteCommand_( const std::vector<std::string_view>& vectorPath,  const gd::argument::arguments& arguments_,   unsigned& uCommandIndex )
{
   // [CONTEXT] Construct handler with shared context instead of raw application pointer.
   APIObject apiobject_( m_context, vectorPath, arguments_, uCommandIndex );
 
   auto result_ = apiobject_.Execute();
 
   if( result_.first == true )
   {
      // Objects are already inside m_context.m_objects via the handler's
      // AddObject() / GetObjects() calls — no separate transfer step needed
      // when the context is shared.  The check below handles the legacy path
      // where a handler that does not yet use context-based construction still
      // fills its own m_objects.
      Types::Objects* pobjectsResult = apiobject_.GetObjects();                                    assert( pobjectsResult );
      if( pobjectsResult != nullptr && pobjectsResult->Empty() == false )
      {
         result_ = m_pdtoresponse->AddTransfer( pobjectsResult );
      }
   }
 
   uCommandIndex = apiobject_.GetCommandIndex();
   return result_;
}


/** @CRITICAL [tag: router, command] [description: Execute command from parsed query string]
 * 
 * @brief 
 * @return 
 */
std::pair<bool, std::string> CRouter::Run()
{
   std::pair<bool, std::string> result_;



   result_ = Prepare();                                                       // prepare for running command to ensure that all necessary data is ready and in correct format
   if( result_.first == false ) { return result_; }

   if( IsCommand() == true )
   {
      // ## parse command path and query arguments and prepare important variables
      auto [vectorPath, arguments_] = gd::parse::uri::parse_path_and_query( m_stringQueryString ); 
      if( vectorPath.empty() == true ) { return { false, "No command found in query string: " + m_stringQueryString }; }
#ifndef NDEBUG
      std::string stringArguments_d = gd::argument::debug::print( arguments_ );
#endif // NDEBUG

      if( IsBody() == true )
      {
         if( IsRequestFormatXml() == true )
         {
            pugi::xml_document* pdocument = static_cast<pugi::xml_document*>( m_pairRequestData.second );
            arguments_.append( "xml", (void*)pdocument );                    // add parsed xml document to arguments for command execution
         }
         else if( IsRequestFormatJson() == true )
         {
            jsoncons::json* pjson = static_cast<jsoncons::json*>( m_pairRequestData.second );
            arguments_.append( "json", (void*)pjson );                       // add parsed json document to arguments for command execution
         }
      }

      unsigned uCommandIndex = 0;

      if( !m_pdtoresponse )
      {
         // ## double checked locking pattern to ensure that response object is created only once and is thread safe

         std::lock_guard<std::mutex> lock( m_mutexRouter );                  // lock mutex to ensure thread safety when creating response object
         if( !m_pdtoresponse )
         {
            m_pdtoresponse = std::make_unique<CDTOResponse>();
            m_pdtoresponse->Initialize();
         }
      }
      
      // ### Check for echo, echo is used to return information to client and this is sent fron client 
      if( arguments_.exists("echo") == true ) { m_pdtoresponse->AddContext( "echo", arguments_["echo"].as_variant_view() ); }

      while( uCommandIndex < vectorPath.size() )
      {
         std::string_view stringCommand = vectorPath[uCommandIndex];
         if( stringCommand == "db" )                                             // database related commands, select, create, delete, open, close database
         {
            result_ = ExecuteCommand_<CAPIDatabase>( vectorPath, arguments_, uCommandIndex );
         } 
         else if( stringCommand == "sql" )                                       // sql commands are logic related to sql queries, adding, remove or edit sql queries
         {
            result_ = ExecuteCommand_<CAPISql>( vectorPath, arguments_, uCommandIndex );
         }
         else if( stringCommand == "sys" )                                       // system related commands, thing that affects the complete system
         {
            result_ = ExecuteCommand_<CAPISystem>( vectorPath, arguments_, uCommandIndex );
         }

         uCommandIndex++;

         if( result_.first == false ) { return result_; }
      }
   }
   

   return { true, "" };
}

/** --------------------------------------------------------------------------
 *  @brief Prepare for running command, this can include things like encoding values in arguments, validating arguments, etc.
 */
std::pair<bool, std::string> CRouter::Prepare()
{                                                                                                  assert( IsPrepared() == false );
   // ## prepare for running command, this can include things like encoding values in arguments, validating arguments, etc.

   SetFlag( eFlagPrepared );                                                  // set prepared flag to indicate that preparation is done

   /// ### Prepare request data if sent with request
   if( m_stringBody.empty() == false )
   {
      if( IsRequestFormatXml() == true )
      {
         pugi::xml_document* pdocument = new pugi::xml_document;
         pugi::xml_parse_result parseResult = pdocument->load_string( m_stringBody.data() );
         if( parseResult.status != pugi::xml_parse_status::status_ok )
         {
            delete pdocument;
            return { false, "Failed to parse XML body: " + std::string(parseResult.description()) };
         }

         SetResponseData( eRequestFormatXml, pdocument );                     // store parsed xml in response data to be used later in command execution
      }
      else if( IsRequestFormatJson() == true )
      {
         jsoncons::json* pjson = new jsoncons::json;
         try
         {
            (*pjson) = jsoncons::json::parse( m_stringBody );
         }
         catch( const std::exception& e )
         {
            delete pjson;
            return { false, "Failed to parse JSON body: " + std::string(e.what()) };
         }

         SetResponseData( eRequestFormatJson, pjson );                        // store parsed json in response data to be used later in command execution
      }
   }

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Run( const std::vector<std::string_view>& vectorCommand, gd::argument::arguments& argumentsParameter )
{
   // ## run commands from vectorCommand with argumentsParameter

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Encode_s( gd::argument::arguments& arguments_, const std::vector<std::string>& vectorName )
{
   std::string stringValueEncoded; // encoded value

   // ## encode values in arguments for specified names in vectorName
   for( const auto& stringName : vectorName )
   {
      decltype( arguments_.next() ) positionNext = nullptr;
      
      for(auto* position_ = arguments_.next(); position_ != nullptr; position_ = arguments_.next(position_) ) 
      {
         auto name_ = gd::argument::arguments::get_name_s(position_);
         if( name_ != stringName ) { continue; }

         auto stringValue = gd::argument::arguments::get_argument_s( position_ ).as_variant_view().as_string_view(); // get string value as string view to avoid alocate
         stringValueEncoded.clear();
         auto result_ = gd::utf8::uri::convert_uri_to_uf8( stringValue, stringValueEncoded );
         for( char& i_ : stringValueEncoded ) { if( i_ == '+' ) { i_ = ' '; } }
         arguments_.set( position_, stringValueEncoded, &positionNext );
         position_ = positionNext;
      }
   }
   return { true, "" };
}
