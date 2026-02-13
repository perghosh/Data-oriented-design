// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#include <string>
#include <list>
#include <deque>
#include <array>
#include <forward_list>
#include <map>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#include <psapi.h>
#endif

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_io.h"
#include "gd/gd_vector.h"

#include "../Session.h"
#include "../Document.h"
#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[session] borrow vector 1", "[session]" )
{
   gd::borrow::vector< std::pair<std::string_view, gd::variant_view> > vector_;
   vector_.push_back( {"key1", "value1"} );
   vector_.push_back( {"key2", "value2"} );
   vector_.push_back( {"key3", "value3"} );
   vector_.push_back( {"key5", "value4"} );

   gd::argument::arguments arguments_( vector_, gd::types::tag_view{});

}

TEST_CASE( "[session] borrow vector 2", "[session]" )
{
   gd::borrow::vector< std::pair<std::string_view, gd::variant> > vector_;
   vector_.push_back( {"key1", "value1"} );
   vector_.push_back( {"key2", "value2"} );
   vector_.push_back( {"key3", "value3"} );
   vector_.push_back( {"key5", "value4"} );

   gd::argument::arguments arguments_( vector_);                              REQUIRE( arguments_["key3"] == "value3" );
}

TEST_CASE( "[session] std::list", "[session]" )
{
   std::list< std::pair<std::string_view, gd::variant> > list_;
   list_.push_back( {"key1", "value1"} );
   list_.push_back( {"key2", "value2"} );
   list_.push_back( {"key3", "value3"} );
   list_.push_back( {"key5", "value4"} );

   gd::argument::arguments arguments_( list_);
   REQUIRE( arguments_["key3"] == "value3" );
}

TEST_CASE( "[session] borrow vector test", "[session]" )
{
   std::array<int,20> buffer_;
   gd::borrow::vector<int> vector_( buffer_ );

   vector_.push_back( 1 );                                                    REQUIRE( vector_[0] == 1 ); REQUIRE( vector_.owner() == false);
   for( int i = 0; i < 10; ++i ) { vector_.push_back( i ); }                  REQUIRE( vector_[0] == 1 ); REQUIRE( vector_[1] == 0 ); REQUIRE( vector_[2] == 1 ); REQUIRE( vector_[3] == 2 ); REQUIRE( vector_[4] == 3 ); REQUIRE( vector_[5] == 4 ); REQUIRE( vector_[6] == 5 ); REQUIRE( vector_[7] == 6 ); REQUIRE( vector_[8] == 7 ); REQUIRE( vector_[9] == 8 );
   vector_.emplace_back( 10, 20, 30, 40, 50, 60 );                            REQUIRE( vector_[11] == 10 );  REQUIRE( vector_[16] == 60 );
   for( int i = 100; i < 110; ++i ) { vector_.push_back( i ); }               REQUIRE( vector_.owner() == true);
}

TEST_CASE( "[session] vector - default construction", "[session]" )
{
   gd::stack::vector<std::byte, 128> vec;

   gd::argument::arguments arguments_( vec );

   arguments_.append("test", "test");
   arguments_.append("test1", "test1");
   arguments_.append("test2", "test2");

   REQUIRE(arguments_["test1"].as_string() == "test1");
   // file:///C:/dev/home/DOD/target/server/http/Router.cpp() <-- this works
   // file:///C:/dev/home/DOD/target/server/http/Router.cpp(80) <-- this DO NOT work :(
}

