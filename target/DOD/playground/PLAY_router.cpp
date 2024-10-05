#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_com_server.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

// Run logic on arguments to test new features --------------------------------
TEST_CASE( "[router] add variables", "[router]" ) {
   using namespace gd::com::server::router;

   auto pserver = gd::com::pointer< server >( new server );
   auto pcommand = gd::com::pointer< command >( new command( pserver) );
   auto presponse = gd::com::pointer< response >( new response() );   

   std::string stringTemplate = "command?one=1&one=1&one=1&one=1&one=1&two=2&one=1";

   //gd::com::pointer
}


