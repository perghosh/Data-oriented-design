#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_com_server.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

// forward declare run method
std::pair<bool, std::string> Run(const std::string_view& stringCommand, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse );

// Run logic on arguments to test new features --------------------------------
TEST_CASE( "[router] add variables", "[router]" ) {
   using namespace gd::com::server::router;

   auto pserver = gd::com::pointer< server >( new server );
   auto pcommand = gd::com::pointer< command >( new command( pserver) );
   auto presponse = gd::com::pointer< response >( new response() );   

   std::string stringTemplate = "command?one=1&one=1&one=1&one=1&one=1&two=2&one=1;command?one=1;command?one=1&one=1&one=1&one=1";
   pserver->callback_add( Run );
   auto result_ = pserver->get( stringTemplate, pcommand, presponse );                             REQUIRE( result_.first );


   //gd::com::pointer
}


std::pair<bool, std::string> Run(const std::string_view& stringCommand, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse )
{
   gd::com::server::router::command* pcommand_ = (gd::com::server::router::command*)pcommand;
   std::vector< std::string_view > vectorCommand = pcommand_->add_querystring( stringCommand );


   std::string_view stringCommandName;
   stringCommandName = vectorCommand.at( 0 );
   if( stringCommandName == "command" )
   {
      std::cout << "running command\n";
   }

   return { true, "" };
}



