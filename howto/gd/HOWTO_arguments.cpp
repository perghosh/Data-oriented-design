#include <chrono>
#include <span>
#include <random>
#include <ranges>
#include <list>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"



#include "../main.h"

#include "catch2/catch_amalgamated.hpp"

void pass_arguments(const gd::argument::arguments& arguments_) {
   std::cout << arguments_.print() << std::endl;
}

TEST_CASE( "pass", "[howto_gd_arguments]" ) {
   pass_arguments({ { "first", 1000 }, {"second", 2000.02} } );               // prints - "first": 1000, "second": 2000.02"
   gd::argument::arguments arguments_;
   arguments_ += { "first", 1000 };
   pass_arguments( arguments_ );                                              // prints - "first": 1000
   arguments_ += {"second", 2000.0201};
   pass_arguments( arguments_ );                                              // prints - "first": 1000, "second": 2000.02"
   arguments_ += { "third", "3000" };
   pass_arguments( arguments_ );                                              // prints - "first": 1000, "second": 2000.02", "third": 3000"
   arguments_.set("third",3333);
   arguments_.remove("second");
   pass_arguments( arguments_ );                                              // prints - "first": 1000, "third": 3000"
}

TEST_CASE( "work with types", "[howto_gd_arguments]" ) {
   gd::argument::arguments arguments_;

   int32_t iValue = 10;
   arguments_.append( iValue );
   iValue = 0;
   arguments_[int(0)] >> iValue;
   std::cout << iValue << std::endl;
}
