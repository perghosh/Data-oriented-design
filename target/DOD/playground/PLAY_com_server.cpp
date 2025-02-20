#include <array>

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/com/gd_com_server.h"

#include "gd/gd_arguments_common.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[com_server] add commands", "[com_server]" ) {
   using namespace gd::com::server;
   std::array<std::string_view, 10> arrayUrl = 
   {
      "https://example.com/search?q=blue%20widgets&sort=price%3Aasc",
      "https://anothersite.net/application/products?category=electronics&brand=Samsung&discount=10%25",
      "https://mysite.org/system/profile?user=john.doe%40example.com&lang=en-US",
      "https://api.test.io/data?date=2023-10-27T10%3A30%3A00Z&format=json",
      "https://store.shopping.co.uk/database/items?keyword=red%20shoes&size=UK%208&colour=red",
      "https://images.sample.net/result/view?image=path%2Fto%2Fmy%20image.jpg&width=500&height=300",
      "https://forum.example.net/thread?id=12345&title=How%20to%20encode%20URLs%3F",
      "https://maps.google.com/search?q=1600%20Amphitheatre%20Parkway%2C%20Mountain%20View%2C%20CA",
      "https://website.com/page?param1=value%2Bwith%2Bspaces&param2=another%20value",
      "https://example.com/api/v1/users?filter=%7B%22name%22%3A%22John%20Doe%22%7D"
   };

   auto pserver = gd::com::pointer< gd::com::server::router::server >(new gd::com::server::router::server);
   auto pcommand = gd::com::pointer< gd::com::server::router::command >(new gd::com::server::router::command(pserver));

   for( const auto& stringUrl : arrayUrl ) {
      auto result = pcommand->append(stringUrl, gd::types::tag_uri{});                             REQUIRE(result.first == true);
   }

   std::string stringCommand = pcommand->print();
   std::cout << stringCommand << std::endl;

   pcommand->clear();

   for( const auto& stringUrl : arrayUrl ) {
      std::string_view s_ = gd::utf8::move::find_nth(stringUrl, 2, '/');
      if( s_.empty() == false ) { auto result = pcommand->append(s_, gd::types::tag_uri{});         REQUIRE(result.first == true);}
   }

   stringCommand = pcommand->print();
   std::cout << stringCommand << std::endl;

   auto uCount = pcommand->count(to_command_priority_g("command"));
   std::cout << "count: " << uCount << std::endl;
   uCount = pcommand->count(to_command_priority_g("stack"));
   std::cout << "count: " << uCount << std::endl;
   { auto result = pcommand->append( to_command_priority_g("stack"), gd::argument::arguments({{"one", 1}, {"two", 2}}) );REQUIRE(result.first == true); }

   std::cout << pcommand->print() << std::endl;
   pcommand->sort();
   std::cout << pcommand->print() << std::endl;

   { auto result = pcommand->append( to_command_priority_g("register"), gd::argument::arguments({{"register-one", 1}, {"register-two", 2}}) );REQUIRE(result.first == true); }
   std::cout << pcommand->print() << std::endl;
   pcommand->sort();
   std::cout << pcommand->print() << std::endl;

   // { auto result = pcommand->append( to_command_priority_g("stack"), gd::types::tag_uri{});         REQUIRE(result.first == true); }

}