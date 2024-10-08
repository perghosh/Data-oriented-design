#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

struct Test
{
public:

   Test(gd::variant variantName) 
   {
      m_variantName = variantName;
   }

   void Return()
   {
      std::cout << "Variant as int: " << m_variantName.as_int() << " Variant as double: " << m_variantName.as_double() << std::endl;
   }

   gd::variant m_variantName;

};

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


TEST_CASE( "[gd] kevin 01", "[gd]" ) {
   Test test("3.99");

   test.Return();
   
   gd::variant variantName = "102 3 4 5 6 7";
   std::cout << variantName.as_string() << " : " << variantName.as_double() << " : " << variantName.as_int() << "\n";

   variantName = 100.99;

   std::cout << variantName.as_string() << " : " << variantName.as_double() << " : " << variantName.as_int() << "\n";

   gd::argument::arguments arguments_;
   arguments_.append("Kevin", 17);
   arguments_.append("Nathalie", "Gustafsson");
   std::cout << arguments_["Nathalie"].as_string() << std::endl;
   std::cout << arguments_["Kevin"].as_string() << std::endl;

   std::cout << "end\n";
}
