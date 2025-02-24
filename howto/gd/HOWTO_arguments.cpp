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

void print_arguments(const gd::argument::arguments& arguments_) {
   std::cout << arguments_.print() << std::endl;
}

TEST_CASE( "print_arguments", "[howto_gd_arguments]" ) {
   print_arguments({ { "first", 1000 }, {"second", 2000.02} } );               // prints - "first": 1000, "second": 2000.02"
   gd::argument::arguments arguments_;
   arguments_ += { "first", 1000 }; print_arguments( arguments_ );             // prints - "first": 1000
   arguments_ += {"second", 2000.0201}; print_arguments(arguments_);           // prints - "first": 1000, "second": 2000.02"
   arguments_ += { "third", "3000" }; print_arguments( arguments_ );           // prints - "first": 1000, "second": 2000.02", "third": 3000"
   arguments_.set("third",3333); arguments_.remove("second");                  // prints - "first": 1000, "third": 3333"
   arguments_.remove(1); print_arguments( arguments_ );                        // prints - "first": 1000
   arguments_ += { "", 2000 }; print_arguments(arguments_);                    // prints - "first": 1000, 2000
   arguments_.append( { 3000, 4000, 5000 } ); print_arguments(arguments_);     // prints - "first": 1000, 2000, 3000, 4000, 5000
   arguments_ += {{ "DC", 600 }, { "DCC", 700 }, { "DCCC", 800 } }; print_arguments(arguments_); // prints - "first": 1000, 2000, 3000, 4000, 5000, "DC": 600, "DCC": 700, "DCCC": 800

   auto it = arguments_.begin();
   std::advance(it, 5);
   std::cout << it.get_argument().as_string() << std::endl;
   arguments_.erase(it); print_arguments(arguments_);                         // prints - "first": 1000, 2000, 3000, 4000, 5000, "DCC": 700, "DCCC": 800 arguments_.erase(it); print_arguments(arguments_);                        // prints - "first": 1000, 2000, 3000, 4000, 5000, "DCC": 700, "DCCC": 800
}

struct test_struct {
   template <typename VALUE>
   test_struct& operator+=(const VALUE& v) { m_arguments.append(v); return *this; }

   gd::argument::arguments m_arguments;
};

TEST_CASE( "work with types", "[howto_gd_arguments]" ) {
   gd::argument::arguments arguments_;

   int32_t iValue = 10;
   arguments_.append( iValue );
   iValue = 0;
   arguments_[int(0)] >> iValue;
   std::cout << iValue << std::endl;
}
