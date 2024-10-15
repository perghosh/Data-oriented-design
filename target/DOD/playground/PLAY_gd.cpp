#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"


#include "main.h"

#include "catch2/catch_amalgamated.hpp"

// Run logic on arguments to test new features --------------------------------
TEST_CASE( "[gd] arguments", "[gd]" ) {
   std::cout << "check `arguments` methods" << std::endl;

   std::string stringTemplate = "one=1&two=2&three=3&four=4";
   auto vectorPair = gd::utf8::split_pair( stringTemplate, '=', '&', gd::utf8::tag_string_view{});
   gd::argument::arguments arguments_;
   arguments_.append( vectorPair, gd::argument::tag_parse_type{});
   auto uTypeNumber = arguments_["four"].type_number();                                            REQUIRE( uTypeNumber == gd::types::eTypeNumberInt64);

   arguments_.clear();
   stringTemplate = "one=1&one=1&one=1&one=1&one=1&two=2&one=1";
   vectorPair = gd::utf8::split_pair( stringTemplate, '=', '&', gd::utf8::tag_string_view{});
   arguments_.append( vectorPair, gd::argument::tag_parse_type{});
   auto vectorOne = arguments_.get_argument_all("one");                                            REQUIRE( vectorOne.size() == 6 );
}


TEST_CASE( "[gd] arguments shared", "[gd]" ) {
   gd::argument::shared::arguments arguments_( "one", 1, gd::argument::shared::arguments::tag_no_initializer_list{});
   arguments_.append( "two", 222 );

   uint32_t uOne = arguments_["one"];
   uint32_t uTwo = uOne + uOne;
   uTwo = arguments_["two"];
   uTwo = uOne + uOne;

   {
      gd::argument::shared::arguments arguments_;
      arguments_.append("ten", "1");
      //std::string_view stringTen = arguments_["ten"].as_string_view();
      arguments_.append("ten2", "1");
      arguments_.append("ten3", "1");
      arguments_.append("ten4", "1");
      auto uCount = arguments_.size();
      std::string_view stringTen = arguments_["ten3"].as_string_view();
   }

   auto uCount = arguments_.size();
}
