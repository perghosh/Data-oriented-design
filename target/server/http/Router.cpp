// @FILE [tag: router, http] [summary: Router class for http server] [type: source]

#include "gd/parse/gd_parse_uri.h"
#include "gd/gd_utf8.h"

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


/** @CRITICAL [tag: router, command] [description: Execute command from parsed query string]
 * 
 * @brief 
 * @return 
 */
std::pair<bool, std::string> CRouter::Run()
{
   std::pair<bool, std::string> result_;

   if( IsCommand() == true )
   {
      unsigned uCommandIndex = 0;

      if( !m_pdtoresponse )
      {
         m_pdtoresponse = std::make_unique<CDTOResponse>();
         m_pdtoresponse->Initialize();
      }

      // ## parse command path and query arguments and prepare important variables
      auto [vectorPath, arguments_] = gd::parse::uri::parse_path_and_query( m_stringQueryString ); 
      if( vectorPath.empty() == true ) { return { false, "No command found in query string: " + m_stringQueryString }; }
#ifndef NDEBUG
      std::string stringArguments_d = gd::argument::debug::print( arguments_ );
#endif // NDEBUG
      
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
