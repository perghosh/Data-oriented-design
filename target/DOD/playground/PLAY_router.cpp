#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/com/gd_com_server.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

// forward declare run method
std::pair<bool, std::string> Run(const std::string_view& stringCommand, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse );

TEST_CASE( "[router] add variables", "[router]" ) {
   using namespace gd::com::server::router;
   auto pserver = gd::com::pointer< server >(new server);
   auto pcommand = gd::com::pointer< command >(new command(pserver));

   // appends three stack variables (like lokal variables)
   pcommand->append( {{"iso", "2025-03-15"}, {"european", "15/03/2025"}, {"long", "March 15, 2025"}}, gd::types::tag_variable{});
   pcommand->append( gd::com::server::ePriorityGlobal, {{"iso", "2000-03-15"}, {"european", "15/03/2000"}, {"long", "March 15, 2000"}}, gd::types::tag_variable{});
   // appends a command with argument called query
   pcommand->append("database/select?query=test-name", gd::types::tag_uri{});

   gd::argument::arguments arguments_;
   pcommand->get_command_variable(0u, 0u, &arguments_ );
   std::cout << "arguments: " << arguments_.print() << "\n";
   arguments_.clear();

   pcommand->get_command_variable(0u, "all", &arguments_);
   std::cout << "arguments: " << arguments_.print() << "\n";
   arguments_.clear();

   pcommand->get_command_variable(0u, 0u, &arguments_ );
   std::cout << "arguments: " << arguments_.print() << "\n";

   pcommand->get_variables(&arguments_, 1u);
   std::cout << "arguments: " << arguments_.print() << "\n";
   pcommand->get_variables( &arguments_, 2u);
   std::cout << "arguments: " << arguments_.print() << "\n";
   pcommand->get_variables( &arguments_, "global");
   std::cout << "arguments: " << arguments_.print() << "\n";
   pcommand->clear( "stack" );
   pcommand->get_variables(&arguments_, "stack");

}

// Run logic on arguments to test new features --------------------------------
/*
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
*/


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



