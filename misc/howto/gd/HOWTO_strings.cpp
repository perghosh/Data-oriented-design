#include <algorithm>
#include <chrono>
#include <span>
#include <random>
#include <ranges>
#include <list>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_strings.h"
#include "gd/gd_arguments_shared.h"



#include "../main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "append", "[howto_gd_strings]" ) {
   gd::strings32 strings_;
   strings_.append("one");
   strings_.append("two");
   strings_ << "three";
   strings_.append_any(100).append_any(200).append_any(300);
   strings_.add( 1, 2.0, true, "test" );

   auto uCount = strings_.count();

   REQUIRE(strings_[0] == "one");
   REQUIRE(strings_[uCount-1] == "test");

   for( const auto& it : strings_ ) {
      std::cout << it << ", ";
   }
   std::cout << "\n";

   gd::strings32 strings2_({ "a", "b", "c", "d", "e" });
   REQUIRE(strings2_.join() == "abcde" );
   std::vector<std::string> vector_ = { "f", "g", "h", "i", "j" };
   strings2_.append( vector_ );
   REQUIRE(strings2_.join() == "abcdefghij" );
   strings_.append_any( strings2_ );
   std::cout << strings_.join(" - ") << "\n";

   REQUIRE(strings_.find( "100" ) != strings_.end() );
   REQUIRE(strings_.find( "101" ) == strings_.end() );

   strings_ += "1";
   strings_ += 1.00001;
   strings_ += 1ull;
   std::cout << strings_.join(" - ") << "\n";

   for( auto it = strings_.begin(); it != strings_.end(); ) {
      if( std::atoi((*it).data()) != 1 ) { it = strings_.erase(it); }
      else { ++it; }
   }
   std::cout << strings_.join(" - ") << "\n";
   REQUIRE(strings_.find( "1.00001" ) != strings_.end() );
}
