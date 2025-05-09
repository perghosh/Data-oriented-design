#include <algorithm>
#include <chrono>
#include <span>
#include <random>
#include <ranges>
#include <list>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_variant_common.h"

#include "gd/gd_strings.h"



#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[gd] strings", "[gd]" ) {
   static_assert(std::ranges::forward_range<gd::strings32>);

   static_assert(std::ranges::forward_range<std::list<double>>);

   gd::strings32 strings_;

   strings_.append("one");
   strings_ << 1 << 2 << 33;
   strings_.add("two", "ten", "six", "five");

   for ( auto it : strings_ ) {
      std::cout << it << " - ";
   }
   std::cout << std::endl;


   std::vector<gd::variant_view> vectorValue = { {10}, {20}, {30}, {40}, {50} };
   gd::strings32 strings2_;
   strings2_.append_any(1);
   strings2_.append_any(gd::variant_view(1));
   strings2_.append_any(true);
   strings2_.append_any(1.1);
   strings2_.append_any(vectorValue);
   strings2_.append_any({ {10}, {20}, {30}, {35}, {40}, {50}, {60}, {70} });

   // strings2_.find("10");
   auto bFound = strings2_.exists("10");                                                          REQUIRE(bFound == true);
   bFound = strings2_.exists("101");                                                              REQUIRE(bFound == false);

   auto itFound = strings2_.find("10");                                                           REQUIRE(itFound != strings2_.end());
   auto itMissing = strings2_.find("101");                                                        REQUIRE(itMissing == strings2_.end());
   auto itEnd = strings2_.find("60", itFound);                                                    REQUIRE(itEnd != strings2_.end());
   auto it35 = strings2_.find("35", itFound, itEnd);                                              REQUIRE(it35 != strings2_.end());

   { auto find_ = std::ranges::find( strings2_, "35" );                                           REQUIRE(find_ != strings2_.end()); }
   { auto find_ = std::find( strings2_.begin(), strings2_.end(), "35");                           REQUIRE(find_ != strings2_.end()); }

   std::string stringResult = strings2_.join();
   std::cout << stringResult << std::endl;
   stringResult = strings2_.join(", ");
   std::cout << stringResult << std::endl;

   auto it = strings2_.begin();
   std::advance(it, 2);

   auto range_ = std::ranges::subrange(strings2_);

   auto x = range_.front();

   std::cout << "Five first: ";
   for ( auto it : range_ | std::views::take(5) ) {
      std::cout << it << " ";
   }
   std::cout << std::endl;


   auto above1_ = std::views::all(strings2_) | std::views::filter([](const auto& s_) { return s_[0] > '1'; });
   std::cout << "Range over 1: ";
   for ( auto it : above1_ ) {
      std::cout << it << " ";
   }
   std::cout << std::endl;

   { auto vector_ = gd::get<std::vector<std::string_view>>( strings2_ ); }
   { auto list_ = gd::get<std::list<std::string_view>>( strings2_ ); }

   {
      gd::strings32 strings3_;
      auto list_ = gd::get<std::list<std::string_view>>( strings2_ );
      strings3_.append(list_);
      auto string_ = strings3_.join( " * " );
      std::cout << string_ << std::endl;
   }
}

TEST_CASE( "[gd] using get on variant and variant_view", "[gd]" ) {
   gd::variant v_ = 1.01;
   gd::variant_view vv_ = 10.01;

   { auto x = gd::get<double>(v_); }
   { auto x = gd::get<int>(v_); }

   { auto x = gd::get<int32_t>(v_); }
   { auto x = gd::get<int32_t>(vv_); }
}


TEST_CASE( "[gd] arguments using index", "[gd]" ) {
   gd::argument::arguments arguments_;
   arguments_.append("1", 1);
   arguments_.append("2", "2");
   arguments_.append("3", 3);
   arguments_.append("4", 4);
   arguments_.append("5", 5);
   arguments_.append_many( 100, 200, 300, 400, 500 );

   using namespace gd::argument;
   std::string_view stringName01 = "1";
   gd::argument::index_edit index_edit( stringName01 );
   auto edit_ = arguments_[index_edit];
   auto edit1_ = arguments_["1"_edit];
   assert( (int)edit_ == (int)edit1_ );
   arguments_[index_edit] = 100;
   int iNumber1 = arguments_["1"];
   iNumber1 *= 2;
   arguments_[index_edit] = iNumber1;
   int iNumber7a = arguments_[7];
   int iNumber7b = arguments_[7_edit];
   assert( iNumber7a == iNumber7b );
}

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

   {
      gd::argument::arguments arguments_;
      arguments_.append_many( 100, 200, 300, 400, 500 );
      //arguments_.insert( 2, "test", 250, gd::argument::shared::arguments::tag_view{});
      std::cout << arguments_.print() << "\n";
   }
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
      arguments_.append_many( 100, 200, 300, 400, 500 );                       // append 5 values
      arguments_.insert( 2, "test", 250, gd::argument::shared::arguments::tag_view{});// insert named value before value with index 2
      std::cout << arguments_.print() << "\n";
   }

   {
      gd::argument::arguments arguments_;
      arguments_.append_many( 100, 200, 300, 400, 500 );
      arguments_.insert( 2, "test1", 250, gd::argument::shared::arguments::tag_view{});
      arguments_.insert( 2, "test2", "1234567890", gd::argument::shared::arguments::tag_view{});
      arguments_.insert( 2, "test3", 250, gd::argument::shared::arguments::tag_view{});
      arguments_.insert( 2, "test4", 250, gd::argument::shared::arguments::tag_view{});
      arguments_.insert( 2, "test5", 250, gd::argument::shared::arguments::tag_view{});
      std::cout << arguments_.print() << "\n";
   }


   {

      {
         gd::argument::shared::arguments arguments_;
         arguments_.append("000", 10);
         arguments_.append_many(100, 200, 300, 400, 500);
         arguments_.append("111", 10);
         arguments_.append_many(100, 200, 300, 400, 500);
         std::cout << arguments_.print() << "\n";

         auto value_ = arguments_.get_argument("111", 3, gd::argument::tag_section{});
         std::cout << value_.as_int64() << "\n";
         arguments_[gd::argument::index_edit{"111", 3}] = 33333;
         value_ = arguments_.get_argument("111", 3, gd::argument::tag_section{});
         std::cout << value_.as_int64() << "\n";
      }

      {
         gd::argument::arguments arguments_;
         arguments_.append("000", 10);
         arguments_.append_many(100, 200, 300, 400, 500);
         arguments_.append("111", 10);
         arguments_.append_many(100, 200, 300, 400, 500);
         std::cout << arguments_.print() << "\n";

         auto value_ = arguments_.get_argument("111", 3, gd::argument::tag_section{});
         std::cout << value_.as_int64() << "\n";
         arguments_[gd::argument::index_edit{"111", 3}] = 33333;
         value_ = arguments_.get_argument("111", 3, gd::argument::tag_section{});
         std::cout << value_.as_int64() << "\n";
      }
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