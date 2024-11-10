#include <random>
#include <chrono>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_sql_value.h"


#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[gd] replace", "[gd]" ) {
   std::string stringSQL = "1111{?name1;{=found};not_found}2222";
   auto stringReplace1 = gd::sql::replace_g( stringSQL, gd::argument::arguments(), gd::sql::tag_preprocess{});

   std::vector< std::pair<std::string_view, gd::variant> > vectorValue;

   bool bError = false;
   auto stringReplace2 = gd::sql::replace_g(stringSQL, [&vectorValue]( const auto& name_ ) -> gd::variant_view {
      for( auto it : vectorValue ) { if( it.first == name_ ) return gd::variant_view( it.second ); }
      return gd::variant_view();
   }, &bError, gd::sql::tag_preprocess{});

   vectorValue.push_back( { "name1", 1 } );
   auto stringReplace3 = gd::sql::replace_g(stringSQL, [&vectorValue]( const auto& name_ ) -> gd::variant_view {
      for( auto it : vectorValue ) { if( it.first == name_ ) return gd::variant_view( it.second ); }
      return gd::variant_view();
   }, &bError, gd::sql::tag_preprocess{});
                                                                                                   REQUIRE( stringReplace1 == stringReplace2 );
                                                                                                   REQUIRE( stringReplace1 != stringReplace3 );


   stringSQL = R"SQL(SELECT FHomoMean as homo_mean,
                   FHomoMax as homo_max,
                   FLumoMean as lumo_mean,
                   FLumoMin as lumo_min
            FROM TBodyTypePDOS
            WHERE BodyTypeK = (SELECT BodyTypeK FROM TBodyType WHERE FLevel = {level} AND FId = {id})
            {?trajectory;AND TrajectoryK = (SELECT TrajectoryK FROM TTrajectory WHERE FLevel = {level} AND FId = {id};AND TrajectoryK IS NULL} )SQL";
   gd::argument::arguments arguments_;
   arguments_.append( "id", 1 );
   arguments_.append( "level", 0 );
   stringSQL = gd::sql::replace_g( stringSQL, arguments_, gd::sql::tag_preprocess{});
   stringSQL = gd::sql::replace_g( stringSQL, arguments_, gd::sql::tag_brace{} );
                                                                                                   REQUIRE( stringSQL.find( '{' ) == std::string::npos );
   /*

   std::vector<gd::variant_view> vectorValue;

   // Trajectory or null trajectory?
   if (trajectory_id == s_max) {
      sql.append(std::string("AND TrajectoryK IS NULL"));
      vectorValue = {level, body_type_id};
   } else {
      sql.append(std::string("AND TrajectoryK = (SELECT TrajectoryK FROM TTrajectory WHERE FLevel = ? AND FId = ?)"));
      vectorValue = {level, body_type_id, level, trajectory_id};
   }

   */

}


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

   {
      using namespace gd::argument::shared;
      std::unique_ptr<const char, decltype([](auto p_){ std::cout << p_ << std::endl; } )> quit_("\n## End section - resize values executed");
      gd::argument::shared::arguments arguments_;
      arguments_.append("1", 1);
      arguments_.append("2", "2");
      arguments_.append("3", 3);
      arguments_.append("4", 4);
      arguments_.append("5", 5);

      auto value_ = arguments_["2"].as_string();

      std::cout << value_ << "\n";
      value_ = arguments_.print();
      std::cout << value_ << "\n";

      arguments_.set( "2", "222222" );
      value_ = arguments_.print();
      std::cout << value_ << "\n";
      arguments_.remove( "4" );
      value_ = arguments_.print();
      std::cout << value_ << "\n";
   }

   {
      using namespace gd::argument::shared;
      std::random_device randomdevice;
      std::mt19937 mt19937RandomNumber(randomdevice()); 
      std::string stringSelectCharFrom = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
      std::uniform_int_distribution<> pick_character_(0, (int)stringSelectCharFrom.size() - 1);

      gd::argument::shared::arguments arguments_;

      for(unsigned u = 0; u < 100; u++)
      {
         unsigned uLength = u % 20 + 5;
         std::string stringKey;
         for(unsigned uNextChar = 0; uNextChar < uLength; uNextChar++)
         {
            auto index_ = pick_character_( mt19937RandomNumber );
            stringKey += stringSelectCharFrom[index_];
         }

         arguments_.append( stringKey, uLength );
      }

      auto value_ = arguments_.print();
      std::cout << value_ << "\n";
   }

   {
      gd::argument::shared::arguments arguments_;
      arguments_.append_many( 100, 200, 300, 400, 500 );
      arguments_.insert( 2, "test", 250, gd::argument::shared::arguments::tag_view{});
      std::cout << arguments_.print() << "\n";
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