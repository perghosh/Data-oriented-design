// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <array>
#include <filesystem>

#include "gd/gd_variant.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_file.h"
#include "gd_tools/html/gd_tools_html_document.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[html] variant", "[html]" ) {
   {
      gd::argument::arguments argumentsValue( { { "test", std::string( "Hello, world!" ) } } );
      argumentsValue.append( "test2", std::string( "Hello, world 2!" ) );
      auto string_d = gd::argument::debug::print( argumentsValue );
   }

   {
      gd::variant variantValue;

      variantValue = std::string("");
      std::string stringValue = variantValue.as_string();

      stringValue = "Hello, world!";
      variantValue = stringValue;
   }

   {
      gd::variant variantValue( std::string("") );

      std::string stringValue = variantValue.get_string();

      stringValue = "Hello, world!";
      variantValue = stringValue;
   }


   {
      gd::variant variantValue( std::string("") );
      gd::argument::shared::arguments argumentValue( { { "test", variantValue } } );
      std::string stringValue = argumentValue["test"].get_string();
      std::cout << "Argument value: " << stringValue << "\n";
   }

   {
      gd::variant variantValue( std::string("1") );
      gd::argument::shared::arguments argumentValue( { { "test", variantValue } } );
      std::string stringValue = argumentValue["test"].get_string();
      std::cout << "Argument value: " << stringValue << "\n";
   }

   {
      gd::variant variantValue( std::string("12") );
      gd::argument::shared::arguments argumentValue( { { "test", variantValue } } );
      std::string stringValue = argumentValue["test"].get_string();
      std::cout << "Argument value: " << stringValue << "\n";
   }

   {
      // multiple tests using gd::argument::shared::arguments in block, they are oneliners and enclosed in {} with long rows

      // ## string values
      {
         gd::argument::shared::arguments argumentValue;

         argumentValue.append( "key", std::string( "123" ) );
         argumentValue.append( std::string( "123" ) );

         auto v_ = argumentValue["key"].get_string();
         v_ = argumentValue[1].get_string();
         
         REQUIRE( argumentValue["key"].get_string() == "123" );
         REQUIRE( argumentValue[1].get_string() == "123" );

         gd::argument::shared::arguments argumentValue02( { { "key", std::string("value") } } );
         v_ = argumentValue02["key"].get_string();
         REQUIRE( argumentValue02["key"].get_string() == "value" );
      }


      // ## string values - append and retrieve
      { gd::argument::shared::arguments argumentValue( { { "key", std::string("value") } } ); REQUIRE( argumentValue["key"].get_string() == "value" ); }
      { gd::argument::shared::arguments argumentValue( { { "empty", std::string("") } } ); REQUIRE( argumentValue["empty"].get_string() == "" ); }
      { gd::argument::shared::arguments argumentValue( { { "spaces", std::string("  hello  ") } } ); REQUIRE( argumentValue["spaces"].get_string() == "  hello  " ); }

      // ## integer values - append and retrieve
      { gd::argument::shared::arguments argumentValue( { { "integer", gd::variant(42) } } ); REQUIRE( argumentValue["integer"].as_int() == 42 ); }
      { gd::argument::shared::arguments argumentValue( { { "negative", gd::variant(-100) } } ); REQUIRE( argumentValue["negative"].as_int() == -100 ); }
      { gd::argument::shared::arguments argumentValue( { { "zero", gd::variant(0) } } ); REQUIRE( argumentValue["zero"].as_int() == 0 ); }

      // ## double values - append and retrieve
      { gd::argument::shared::arguments argumentValue( { { "decimal", gd::variant(3.14) } } ); REQUIRE( argumentValue["decimal"].as_double() == Catch::Approx(3.14) ); }

      // ## bool values - append and retrieve
      { gd::argument::shared::arguments argumentValue( { { "flag", gd::variant(true) } } ); REQUIRE( argumentValue["flag"].as_bool() == true ); }
      { gd::argument::shared::arguments argumentValue( { { "flag", gd::variant(false) } } ); REQUIRE( argumentValue["flag"].as_bool() == false ); }

      // ## multiple named values - size and access
      { gd::argument::shared::arguments argumentValue( { { "a", std::string("1") }, { "b", std::string("2") }, { "c", std::string("3") } } ); REQUIRE( argumentValue.size() == 3 ); }
      { gd::argument::shared::arguments argumentValue( { { "first", std::string("A") }, { "second", std::string("B") } } ); REQUIRE( argumentValue["first"].get_string() == "A" ); REQUIRE( argumentValue["second"].get_string() == "B" ); }

      // ## as_string_view - lightweight access without allocation
      { gd::argument::arguments argumentValue;argumentValue.append( "view", std::string("hello") ); REQUIRE( argumentValue["view"].as_string_view() == "hello" ); }
      { gd::argument::shared::arguments argumentValue;argumentValue.append( "view", std::string("hello") ); REQUIRE( argumentValue["view"].as_string_view() == "hello" ); }
      { gd::argument::shared::arguments argumentValue( { { "view", std::string("hello") } } ); REQUIRE( argumentValue["view"].as_string_view() == "hello" ); }

      // ## missing key returns empty/null argument
      { gd::argument::shared::arguments argumentValue( { { "exists", std::string("yes") } } ); REQUIRE( argumentValue["missing"].empty() == true ); }

      // ## copy semantics - shared reference counting
      { gd::argument::shared::arguments argumentValue( { { "shared", std::string("data") } } ); auto argumentCopy = argumentValue; REQUIRE( argumentCopy["shared"].get_string() == "data" ); }

      // ## append after construction
      { gd::argument::shared::arguments argumentValue; argumentValue.append("name", std::string("appended")); REQUIRE( argumentValue["name"].get_string() == "appended" ); }
      { gd::argument::shared::arguments argumentValue; argumentValue.append("i1", 10); argumentValue.append("i2", 20); REQUIRE( argumentValue["i1"].as_int() + argumentValue["i2"].as_int() == 30 ); }

      // ## integer conversion via as_string
      { gd::argument::shared::arguments argumentValue( { { "number", gd::variant(999) } } ); REQUIRE( argumentValue["number"].as_string() == "999" ); }

      // ## index-based access (zero based)
      { gd::argument::shared::arguments argumentValue( { { "first", std::string("X") }, { "second", std::string("Y") } } ); REQUIRE( argumentValue[0u].get_string() == "X" ); REQUIRE( argumentValue[1u].get_string() == "Y" ); }
   }

}

TEST_CASE( "[html] element creation", "[html]" ) {
   using namespace gd::tools::html;

   element elementRoot( "html" );
   elementRoot.add_attribute( "lang", "en" );
   elementRoot.add_attribute( "lang", "SWE" );

}


TEST_CASE( "[html] load", "[html]" ) {
   using namespace gd::tools::html;

   std::filesystem::path pathCurrentDirecotry = std::filesystem::current_path();
   auto [bFound, stringRootFolder] = gd::file::closest_having_file_g( pathCurrentDirecotry.string(), ROOT_MARKER ); REQUIRE( bFound == true );

  
   std::string stringFileName( stringRootFolder );

   stringFileName += "resource/page_user_edit.html";
   gd::file::path pathFile( stringFileName );

   // Load the HTML file into a string
   std::string stringHtml;
   std::string stringError;
   std::tie( bFound, stringError ) = gd::file::read_file_g( pathFile, stringHtml ); REQUIRE( bFound == true );

   // ## Parse the HTML string into a document object
   parser parser_;
   document documentHtml = parser_.parse( stringHtml );

   auto uCount =  documentHtml.root()->size_all();

   // ## walk elments
   /*
   documentHtml.root()->walk( []( const element& elementCurrent ) -> bool
   {
      std::cout << "Element: " << elementCurrent.name() << ", content: " << elementCurrent.content() << "\n";
      for( const auto& [name_, value_] : elementCurrent.attributes() )
      {
         std::cout << "  Attribute: " << name_ << " = " << value_.as_string() << "\n";
      }
      return true;
   } );
   */

   for( auto it = documentHtml.root()->tree_begin(); it != documentHtml.root()->tree_end(); ++it )
   {
      const element& elementCurrent = *it;
      //std::cout << "Element: " << elementCurrent.name() << ", content: " << elementCurrent.content() << "\n";
      for( const auto& [name_, value_] : elementCurrent.attributes() )
      {
         //std::cout << "  Attribute: " << name_ << " = " << value_.as_string() << "\n";
      }

      if( elementCurrent.size_parents() > 3 )
      {
         // print path
         auto parents_ = elementCurrent.parents();
         std::string stringPath = element::to_string_s( parents_ );
         std::cout << "  Path: " << stringPath << "\n";
      }
   }

}
