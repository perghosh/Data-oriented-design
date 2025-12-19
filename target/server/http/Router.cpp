// @FILE [tag: router, http] [summary: Router class for http server] [type: source]

#include "gd/parse/gd_parse_uri.h"
#include "gd/gd_utf8.h"

#include "api/APIDatabase.h"
#include "api/APISql.h"


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


      /*
      std::string_view stringPath;
      std::string_view stringQuery;

      auto uPosition = stringQueryStringView.find( '?' );
      if( uPosition != std::string::npos ) 
      { 
         stringPath = stringQueryStringView.substr( 0, uPosition ); 
         stringQuery = stringQueryStringView.substr( uPosition + 1 );
      }
      else { stringPath = stringQueryStringView; }

      auto result_ = gd::parse::uri::parse_path( stringQueryStringView, m_vectorCommand );
      if( result_.first == false ) { return result_; }

      if( stringQuery.empty() == false )
      {
         // parse query string into arguments

         auto result_ = gd::parse::uri::parse_query( stringQuery, argumentsQuery );
         if( result_.first == false ) { return result_; }
      }
      */
   }
   else
   {
      m_uFlags &= ~eFlagCommand;
   }

   //auto result_ = boost::urls::parse_uri( stringQueryStringView );


   return { true, "" };
}

// @TODO [tag: router, command] [description: Convert uri encoded text values to normal utf8 strings] [status: open] [assigned: per]
std::pair<bool, std::string> CRouter::Run()
{
   if( IsCommand() == true )
   {
      if( !m_pdtoresponse )
      {
         m_pdtoresponse = std::make_unique<CDTOResponse>();
         m_pdtoresponse->Initialize();
      }

      auto [vectorPath, arguments_] = gd::parse::uri::parse_path_and_query( m_stringQueryString );
      if( vectorPath.empty() == true ) { return { false, "No command found in query string: " + m_stringQueryString }; }

      std::string_view stringCommand = vectorPath[0];
      if( stringCommand == "db" )
      {
         CAPIDatabase database_( m_pApplication, vectorPath, arguments_ );
         return database_.Execute();
      }
      else if( stringCommand == "sql" )
      {

      }
   }
   

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Run( const std::vector<std::string_view>& vectorCommand, gd::argument::arguments& argumentsParameter )
{
   // ## run commands from vectorCommand with argumentsParameter

   std::string_view stringArea = vectorCommand[0];

   if( stringArea == "db" )
   {
      // database commands
      std::string_view stringAction = vectorCommand[1];
      if( stringAction == "create" )
      {
         std::string stringDbName = argumentsParameter["name"].as_string();
         // create database with name
      }
      else if( stringAction == "delete" )
      {
         std::string stringDbName = argumentsParameter["name"].as_string();
         // delete database with name
      }
   }


   return { true, "" };
}





/** ---------------------------------------------------------------------------
 * @brief Executes a server command with the given string command and arguments, returning the result.
 *
 * This method creates a server object, constructs a command with the provided string and arguments,
 * executes it, and optionally stores the result in the provided variant pointer. It returns a pair
 * indicating success or failure along with an error message if applicable.
 *
 * @param stringCommand The command to execute, passed as a string view.
 * @param argumentsVariable The arguments associated with the command, provided as a gd::argument::arguments object.
 * @param[out] pvariantResult Pointer to a gd::variant object where the command result will be stored, if available. Can be nullptr if no result is needed.
 *
 * @return A std::pair<bool, std::string> where:
 *         - The first element (bool) indicates success (true) or failure (false).
 *         - The second element (std::string) contains an error message if the execution fails, or an empty string if successful.
 *
 * @note If the response contains a result and pvariantResult is not nullptr, the result is stored in pvariantResult.
 */
 /*
std::pair<bool, std::string> CRouter::Execute(const std::string_view& stringCommand, const gd::argument::arguments& argumentsVariable, gd::variant* pvariantResult)
{
   // ## create server object and add callback for command execution 

   auto pserver = gd::com::pointer< CServer >( new CServer( papplication_g ) );
   auto [pcommand, presponse] = make_command_s(pserver.get());

   // ## prepare command object with command and arguments
 
   // ### prepare local (stack) variables for command

   pcommand->append(stringCommand, argumentsVariable, gd::types::tag_uri{});
#ifndef NDEBUG
   auto stringCommandDump_d = pcommand->print();
#endif

   auto result_ = Execute( pcommand, presponse );
   if( result_.first == false ) { return result_; }  

   if( presponse->return_size() > 0 && pvariantResult != nullptr )
   {
      *pvariantResult = presponse->return_at(0).as_variant();
   }
   else if( pvariantResult != nullptr )                                        // No result from command but variant is set
   {
      return { false, "No result from: " + std::string(stringCommand) };
   }

   return { true, "" };
}
*/


/*
std::tuple< gd::com::pointer< gd::com::server::router::command >, gd::com::pointer< gd::com::server::router::response > > make_command_s(gd::com::server::server_i* pserver )
{
   gd::com::pointer< gd::com::server::router::response > presponse = gd::com::pointer< gd::com::server::router::response >( new gd::com::server::router::response() );
   gd::com::pointer< gd::com::server::router::command > pcommand = gd::com::pointer< gd::com::server::router::command >( new gd::com::server::router::command( pserver ) );

   return { pcommand, presponse };
}
*/
/*
boost::beast::http::message_generator RouteCommand_s( std::string_view stringTarget )
{
   // ## create router for command
	CRouter router_(papplication_g, stringTarget);                             // create router for the target, router is a simple command router to handle commands
	auto result_ = router_.Parse();                                            // parse the target to get command and parameters
   if( result_.first == false ) { return server_error_( result_.second ); }

   return router_.RouteCommand();
}
*/


std::pair<bool, std::string> CRouter::Encode_s( gd::argument::arguments& arguments_, const std::vector<std::string>& vectorName )
{
   // ## encode values in arguments for specified names in vectorName
   for( const auto& stringName : vectorName )
   {
      std::string stringValue = arguments_[stringName].as_string();
      std::string stringValueEncoded;

      // ## replace all + with space
      for( char& c : stringValue ) { if( c == '+' ) { c = ' '; } }

      auto result_ = gd::utf8::uri::convert_uri_to_uf8( stringValue, stringValueEncoded );
      if( result_.first == false ) { return { false, "failed to encode uri value for parameter: " + stringName }; }
      arguments_.set( stringName, stringValueEncoded );
   }
   return { true, "" };
}
