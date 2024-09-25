#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[gd] arguments", "[gd]" ) {
   std::cout << "check `arguments` methods" << std::endl;

   std::string stringTemplate = "one=1&two=2&three=3&four=4";
   auto vectorPair = gd::utf8::split_pair( stringTemplate, '=', '&', gd::utf8::tag_string_view{});
   gd::argument::arguments arguments_;
   arguments_.append( vectorPair, gd::argument::tag_parse_type{});
}


