// @FILE [tag: router, http] [summary: Router class for http server] [type: source]


#include <boost/url.hpp>
#include <boost/url/parse.hpp>

#include "Router.h"

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
{
   assert( m_stringQueryString.empty() == false );
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

   auto result_ = boost::urls::parse_uri( stringQueryStringView );


   return { true, "" };
}

std::pair<bool, std::string> CRouter::Run()
{
   // ## run commands
   /*
   for( const auto& command : m_vectorCommand )
   {
      // process each command
   }
   */

   return { true, "" };
}
