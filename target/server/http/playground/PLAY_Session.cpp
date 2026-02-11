// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#include <string>
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

TEST_CASE( "[session] borrow vector 2 - default construct", "[session]" )
{
   gd::borrow::vector<int> vector_;                                             REQUIRE( vector_.empty() == true ); REQUIRE( vector_.size() == 0 ); REQUIRE( vector_.owner() == true ); REQUIRE( vector_.is_borrowed() == false );
   vector_.push_back( 1 );                                                      REQUIRE( vector_.size() == 1 ); REQUIRE( vector_[0] == 1 );
   vector_.push_back( 2 );                                                      REQUIRE( vector_.size() == 2 ); REQUIRE( vector_[1] == 2 );
}

TEST_CASE( "[session] borrow vector 3 - initializer list", "[session]" )
{
   gd::borrow::vector<int> vector_ = { 1, 2, 3, 4, 5 };                        REQUIRE( vector_.size() == 5 ); REQUIRE( vector_[0] == 1 ); REQUIRE( vector_[4] == 5 ); REQUIRE( vector_.owner() == true );
   gd::borrow::vector<int> vector2_ = { 10, 20, 30 };                          REQUIRE( vector2_.size() == 3 ); REQUIRE( vector2_[0] == 10 ); REQUIRE( vector2_[2] == 30 );
}

TEST_CASE( "[session] borrow vector 4 - element access", "[session]" )
{
   std::array<int,10> buffer_;
   gd::borrow::vector<int> vector_( buffer_ );
   vector_.push_back( 1 ); vector_.push_back( 2 ); vector_.push_back( 3 );     REQUIRE( vector_.front() == 1 ); REQUIRE( vector_.back() == 3 );
   REQUIRE( vector_.at( 0 ) == 1 ); REQUIRE( vector_.at( 2 ) == 3 );           REQUIRE( vector_[1] == 2 );
   int* pData = vector_.data();                                                 REQUIRE( pData != nullptr ); REQUIRE( pData[0] == 1 );
}

TEST_CASE( "[session] borrow vector 5 - iterators", "[session]" )
{
   gd::borrow::vector<int> vector_ = { 10, 20, 30, 40, 50 };
   auto it = vector_.begin();                                                   REQUIRE( *it == 10 );
   ++it;                                                                        REQUIRE( *it == 20 );
   auto itEnd = vector_.end();                                                 REQUIRE( *(itEnd - 1) == 50 );
   REQUIRE( vector_.cbegin() != vector_.cend() );
   auto rit = vector_.rbegin();                                                REQUIRE( *rit == 50 );
   REQUIRE( vector_.rend() != vector_.rbegin() );
   int sum = 0; for( int& i : vector_ ) sum += i;                              REQUIRE( sum == 150 );
}

TEST_CASE( "[session] borrow vector 6 - capacity", "[session]" )
{
   std::array<int,5> buffer_;
   gd::borrow::vector<int> vector_( buffer_ );
   REQUIRE( vector_.is_borrowed() == true );                                   REQUIRE( vector_.owner() == false ); REQUIRE( vector_.empty() == true );
   vector_.push_back( 1 ); vector_.push_back( 2 );                              REQUIRE( vector_.size() == 2 ); REQUIRE( vector_.empty() == false );
   vector_.push_back( 3 ); vector_.push_back( 4 ); vector_.push_back( 5 );       REQUIRE( vector_.size() == 5 ); REQUIRE( vector_.owner() == false );
   vector_.push_back( 6 );                                                      REQUIRE( vector_.owner() == true ); REQUIRE( vector_.size() == 6 );
}

TEST_CASE( "[session] borrow vector 7 - modifiers", "[session]" )
{
   gd::borrow::vector<int> vector_ = { 1, 2, 3, 4, 5 };
   vector_.clear();                                                             REQUIRE( vector_.empty() == true ); REQUIRE( vector_.size() == 0 );
   vector_.push_back( 10 ); vector_.push_back( 20 );                            REQUIRE( vector_.size() == 2 );
   vector_.pop_back();                                                          REQUIRE( vector_.size() == 1 ); REQUIRE( vector_.back() == 10 );
   vector_.resize( 5 );                                                         REQUIRE( vector_.size() == 5 );
   vector_.resize( 3 );                                                         REQUIRE( vector_.size() == 3 );
   vector_.resize( 5, 99 );                                                     REQUIRE( vector_[4] == 99 );
}

TEST_CASE( "[session] borrow vector 8 - insert and erase", "[session]" )
{
   gd::borrow::vector<int> vector_ = { 10, 30, 50 };
   auto it = vector_.insert( vector_.begin() + 1, 20 );                        REQUIRE( *it == 20 ); REQUIRE( vector_[1] == 20 ); REQUIRE( vector_.size() == 4 );
   vector_.insert( vector_.end(), 60 );                                          REQUIRE( vector_.back() == 60 );
   it = vector_.erase( vector_.begin() );                                       REQUIRE( vector_.front() == 20 ); REQUIRE( vector_.size() == 4 );
   it = vector_.erase( vector_.begin(), vector_.end() );                        REQUIRE( vector_.empty() == true );
}

TEST_CASE( "[session] borrow vector 9 - move semantics", "[session]" )
{
   gd::borrow::vector<int> vector1_ = { 1, 2, 3 };
   gd::borrow::vector<int> vector2_( std::move( vector1_ ) );
                                                                               REQUIRE( vector2_.size() == 3 ); REQUIRE( vector2_[0] == 1 );
   gd::borrow::vector<int> vector3_; vector3_ = std::move( vector2_ );         REQUIRE( vector3_.size() == 3 );
}

TEST_CASE( "[session] borrow vector 10 - copy semantics", "[session]" )
{
   gd::borrow::vector<int> vector1_ = { 1, 2, 3 };
   gd::borrow::vector<int> vector2_( vector1_ );
                                                                               REQUIRE( vector2_.size() == 3 ); REQUIRE( vector2_[0] == 1 ); REQUIRE( vector1_.size() == 3 );
   gd::borrow::vector<int> vector3_; vector3_ = vector1_;                     REQUIRE( vector3_.size() == 3 );
}

TEST_CASE( "[session] borrow vector 11 - comparison", "[session]" )
{
   gd::borrow::vector<int> vector1_ = { 1, 2, 3 };
   gd::borrow::vector<int> vector2_ = { 1, 2, 3 };
   gd::borrow::vector<int> vector3_ = { 1, 2, 4 };
   REQUIRE( vector1_ == vector2_ );                                             REQUIRE( vector1_ != vector3_ );
   REQUIRE( vector3_ > vector2_ );                                              REQUIRE( vector2_ < vector3_ );
}

TEST_CASE( "[session] borrow vector 12 - swap", "[session]" )
{
   std::array<int,5> buffer1_; std::array<int,5> buffer2_;
   gd::borrow::vector<int> vector1_( buffer1_ ); vector1_.push_back( 1 );
   gd::borrow::vector<int> vector2_( buffer2_ ); vector2_.push_back( 2 );
   vector1_.swap( vector2_ );                                                  REQUIRE( vector1_[0] == 2 ); REQUIRE( vector2_[0] == 1 );
   REQUIRE( vector1_.is_borrowed() == true );                                  REQUIRE( vector2_.is_borrowed() == true );
}

TEST_CASE( "[session] borrow vector 13 - reserve", "[session]" )
{
   gd::borrow::vector<int> vector_; vector_.reserve( 100 );                    REQUIRE( vector_.capacity() >= 100 );
   for( int i = 0; i < 50; ++i ) vector_.push_back( i );                      REQUIRE( vector_.size() == 50 );
}

TEST_CASE( "[session] borrow vector 14 - rvalue push", "[session]" )
{
   gd::borrow::vector<std::string> vector_;
   vector_.push_back( std::string( "hello" ) );                                REQUIRE( vector_[0] == "hello" );
   vector_.emplace_back( "world" );                                             REQUIRE( vector_[1] == "world" );
}

TEST_CASE( "[session] borrow vector 15 - multi emplace types", "[session]" )
{
   gd::borrow::vector<double> vector_;
   vector_.emplace_back( 1.1, 2.2, 3.3, 4.4, 5.5 );                            REQUIRE( vector_[0] == 1.1 ); REQUIRE( vector_[4] == 5.5 );
   gd::borrow::vector<long long> vector2_; vector2_.emplace_back( 1LL, 2LL, 3LL ); REQUIRE( vector2_[2] == 3LL );
}

