#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
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
      arguments_.append("ten2", "2");
      arguments_.append("ten3", "3");
      arguments_.append("ten4", "4");
      auto s_ = arguments_["ten"].as_string_view();
      s_ = arguments_["ten2"].as_string_view();
      auto uCount = arguments_.size();
      std::string_view stringTen = arguments_["ten3"].as_string_view();

      auto argumentsCopy = arguments_;
   }

   {
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - adding three numbers ");

      gd::argument::shared::arguments arguments_;
      arguments_.append( 100 );
      arguments_.append( 200 );
      arguments_.append( 300 );

      uint32_t u_ = arguments_[1];
      auto uCount = arguments_.size();
   }

   {
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - adding three numbers in one method");

      gd::argument::shared::arguments arguments_;
      arguments_.append_many( 100, 200, 300);

      uint32_t u_ = arguments_[0u];
      auto uCount = arguments_.size();

      for(auto it = std::begin( arguments_ ), itEnd = std::end( arguments_ ); it != itEnd; ++it )
      {
         uint32_t u_ = *it;
         std::cout << "number: " << u_ << "\n";
      }

      for(auto it : arguments_)
      {
         uint32_t u_ = it;
         std::cout << "number: " << u_ << "\n";
      }
   }

   {
      using namespace gd::argument::shared;
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - get vector for name values");
      gd::argument::shared::arguments arguments_;
      arguments_.append_argument( "values", 0, arguments::tag_view{});
      arguments_.append_many( 100, 200, 300, 400, 500 );
      arguments_.append_argument( "sum", 0u, arguments::tag_view{} );

      arguments_.append_argument("names", "name value", arguments::tag_view{});
      arguments_.append_many("100 as text", "200 as text", "300 as text");

      auto vector_ = arguments_.get_argument_section( "values", arguments::tag_view{} );
      std::cout << gd::debug::print( vector_ ) << "\n";

      vector_ = arguments_.get_argument_section( "names", arguments::tag_view{} );
      std::cout << gd::debug::print( vector_ ) << "\n";
   }


   auto uCount = arguments_.size();
}


TEST_CASE( "[gd] cli options test", "[gd]" ) {
   gd::cli::options optionsApplication("application");

   {
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## gd_cli_options section ending - tested simple ");
      gd::cli::options options_( "merge", "description text" );
      options_.add({"source_database_path", 's', "Path to database to merge from"});
      options_.add({"destination", 'd', "Path to database to merge into (shorter version)"});
      options_.add({"source", "Path to database to merge from (shorter version)"});
      options_.add({"destination_database_path", "Path to database to merge into"});
      options_.add({"merge_to", "if a third database is set where database is merge to"});
      options_.add({"folder", "set root folder that is relative to other files set when to merge"});
      optionsApplication.sub_add( std::move( options_ ) );

      std::vector<std::string> vectorArgument = {"executable, this is skipped", "merge", "--destination", "C:\\" };
      auto [bOk, stringError] = optionsApplication.parse( vectorArgument );                        REQUIRE( bOk == true );
      REQUIRE( optionsApplication.sub_get("merge")["destination"].as_string() == "C:\\" );
      optionsApplication.clear_all();

      vectorArgument = {"executable, this is skipped", "merge", "--destination", "D:\\" };
      std::tie(bOk, stringError) = optionsApplication.parse( vectorArgument );                     REQUIRE( bOk == true );
      REQUIRE( optionsApplication.sub_get("merge")["destination"].as_string() == "D:\\" );
      optionsApplication.clear_all();

      vectorArgument = {"executable, this is skipped", "merge", "-destination", "D:\\" };
      std::tie(bOk, stringError) = optionsApplication.parse( vectorArgument );                     REQUIRE( bOk == false );
      optionsApplication.clear_all();

      optionsApplication.sub_find("merge")->set_flag( gd::cli::options::eFlagSingleDash, 0 ); 
      vectorArgument = {"executable, this is skipped", "merge", "-destination", "D:\\" };
      std::tie(bOk, stringError) = optionsApplication.parse( vectorArgument );                     REQUIRE( bOk == true );
      REQUIRE( optionsApplication.sub_get("merge")["destination"].as_string() == "D:\\" );
      optionsApplication.clear_all();
      optionsApplication.sub_find("merge")->set_flag( 0, gd::cli::options::eFlagSingleDash ); 
   }
}

TEST_CASE( "[gd] arguments equal to shared", "[gd]" ) {
   gd::argument::arguments arguments_( "one", 1, gd::argument::arguments::tag_no_initializer_list{});
   arguments_.append( "two", 222 );

   uint32_t uOne = arguments_["one"];
   uint32_t uTwo = uOne + uOne;
   uTwo = arguments_["two"];
   uTwo = uOne + uOne;

   {
      gd::argument::shared::arguments arguments_;
      arguments_.append("ten", "1");
      arguments_.append("ten2", "2");
      arguments_.append("ten3", "3");
      arguments_.append("ten4", "4");
      auto s_ = arguments_["ten"].as_string_view();
      s_ = arguments_["ten2"].as_string_view();
      auto uCount = arguments_.size();
      std::string_view stringTen = arguments_["ten3"].as_string_view();

      auto argumentsCopy = arguments_;
   }

   {
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - adding three numbers ");

      gd::argument::arguments arguments_;
      arguments_.append( 100 );
      arguments_.append( 200 );
      arguments_.append( 300 );

      uint32_t u_ = arguments_[1];
      auto uCount = arguments_.size();
   }

   {
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - adding three numbers in one method");

      gd::argument::arguments arguments_;
      arguments_.append_many( 100, 200, 300);

      uint32_t u_ = arguments_[0u];
      auto uCount = arguments_.size();

      for(auto it = std::begin( arguments_ ), itEnd = std::end( arguments_ ); it != itEnd; ++it )
      {
         uint32_t u_ = *it;
         std::cout << "number: " << u_ << "\n";
      }

      for(auto it : arguments_)
      {
         uint32_t u_ = it;
         std::cout << "number: " << u_ << "\n";
      }
   }

   {
      using namespace gd::argument;
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - get vector for name values");
      gd::argument::arguments arguments_;
      arguments_.append_argument( "values", 0, arguments::tag_view{});
      arguments_.append_many( 100, 200, 300, 400, 500 );
      arguments_.append_argument( "sum", 0u, arguments::tag_view{} );

      arguments_.append_argument("names", "name value", arguments::tag_view{});
      arguments_.append_many("100 as text", "200 as text", "300 as text");

      auto vector_ = arguments_.get_argument_section( "values", arguments::tag_view{} );
      std::cout << gd::debug::print( vector_ ) << "\n";

      vector_ = arguments_.get_argument_section( "names", arguments::tag_view{} );
      std::cout << gd::debug::print( vector_ ) << "\n";
   }


   auto uCount = arguments_.size();
}