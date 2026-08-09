// @FILE [tag: arguments, playground] [description: code used to manage arguments and arg] [type: playground]

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_variant_arg.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[arguments] test arg to add values", "[arguments]") 
{
   gd::argument::arguments argumentsTest;
   argumentsTest.append( "key1", "value1" );
   argumentsTest.append( "key2", 42 ); 

   gd::arg_view test( "key", 3.14 );

   argumentsTest += gd::arg_view("key2","tttt");
   //argumentsTest = gd::arg_view("key2","tttt");

   argumentsTest << std::make_pair<const char*, gd::variant_view>( "key3", true ) << std::make_pair<const char*, gd::variant_view>( "key4", 12345u ) 
      << gd::arg_view( "key5", "value5" );

   gd::arg_view test4( "key4" );
   argumentsTest >> test4;

   //argumentsTest = { "key5", "value5" };

}

// test to check for named error
TEST_CASE("[arguments] key value iterate", "[arguments]")
{
   // simulate JSON.stringify({FLoginName: sName, FPassword: sPassword})
   std::string stringJson;
   stringJson += "{";
   stringJson += "\"FLoginName\": \"testuser\",";
   stringJson += "\"FPassword\": \"testpassword\"";
   stringJson += "}";

   gd::argument::arguments argumentsTest;

   argumentsTest.append("FLoginName", "testuser");
   argumentsTest.append("FPassword", "testpassword");
   argumentsTest.append("active", true);

   for(const auto& [key, value] : argumentsTest.named() )  
   {
      std::cout << "Key: " << key << ", Value:" << value.as_string() << std::endl;
   }

   //argumentsTest.append(stringJson, gd::types::tag_json{});
}
