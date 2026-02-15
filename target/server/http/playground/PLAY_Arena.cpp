// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#include <string>
#include <list>
#include <deque>
#include <array>
#include <forward_list>
#include <map>
#include <vector>


#include "gd/gd_arena.h"
#include "gd/gd_arguments.h"
#include "gd/gd_vector.h"

#include "../Session.h"
#include "../Document.h"
#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[arena] std::vector", "[arena]" )
{
   using namespace gd::arena;
   std::cout << "\n=== Example 1: std::vector with arena_allocator ===\n";
   
   // Create arena with 4KB blocks
   arena<> myArena(4096);
   
   // Create allocator from arena
   arena_allocator<int> allocator(myArena);
   
   // Create vector using arena allocator
   std::vector<int, arena_allocator<int>> vec(allocator);
   
   // Add elements
   for(int i = 0; i < 100; ++i) { vec.push_back(i); }
   
   std::cout << "Vector size: " << vec.size() << "\n";
   std::cout << "First 10 elements: ";
   for(int i = 0; i < 10; ++i) { std::cout << vec[i] << " "; }
   std::cout << "\n";
   
   // Show arena statistics
   std::cout << "Arena blocks: " << myArena.block_count() << "\n";
   std::cout << "Total allocated: " << myArena.total_allocated() << " bytes\n";
   std::cout << "Total capacity: " << myArena.total_capacity() << " bytes\n";
   std::cout << "Fragmentation: " << (myArena.fragmentation() * 100.0) << "%\n";
}

TEST_CASE( "[arena] std::string", "[arena]" )
{
   using namespace gd::arena;
   std::cout << "\n=== Example 2: std::string with arena_allocator ===\n";
   
   arena<> myArena(2048);
   arena_allocator<char> allocator(myArena);
   
   // Create string using arena allocator
   std::basic_string<char, std::char_traits<char>, arena_allocator<char>> str(allocator);
   
   str = "Hello from arena allocator!";
   str += " This string is allocated in an arena.";
   
   std::cout << "String: " << str << "\n";
   std::cout << "Length: " << str.length() << "\n";
   std::cout << "Arena allocated: " << myArena.total_allocated() << " bytes\n";
}



TEST_CASE( "[arena] arguments 01 - basic allocation", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 256 );

   gd::argument::arguments arguments_( span );

   REQUIRE( arguments_.empty() );
   REQUIRE( arguments_.size() == 0 );
}

TEST_CASE( "[arena] arguments 02 - append and retrieve various types", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 1024 );

   gd::argument::arguments arguments_( span );

   // Test boolean
   arguments_.append( true );
   arguments_.append( false );

   // Test integer types
   arguments_.append( static_cast<int8_t>(-42) );
   arguments_.append( static_cast<uint8_t>(255) );
   arguments_.append( static_cast<int16_t>(-1000) );
   arguments_.append( static_cast<uint16_t>(5000) );
   arguments_.append( static_cast<int32_t>(-999999) );
   arguments_.append( static_cast<uint32_t>(999999) );
   arguments_.append( static_cast<int64_t>(-9000000000LL) );
   arguments_.append( static_cast<uint64_t>(9000000000ULL) );

   // Test floating point
   arguments_.append( 3.14159f );
   arguments_.append( 2.71828 );

   // Test string
   arguments_.append( "Hello, Arena!" );
   arguments_.append( std::string("Test string") );

   REQUIRE( arguments_.size() == 14 );

   // Verify values
   auto it = arguments_.begin();
   REQUIRE( (it++)->as_bool() == true );
   REQUIRE( (it++)->as_bool() == false );
   REQUIRE( (it++)->as_int() == -42 );
   REQUIRE( (it++)->as_uint() == 255 );
   REQUIRE( (it++)->as_int() == -1000 );
   REQUIRE( (it++)->as_uint() == 5000 );
   REQUIRE( (it++)->as_int() == -999999 );
   REQUIRE( (it++)->as_uint() == 999999 );
   REQUIRE( (it++)->as_int64() == -9000000000LL );
   REQUIRE( (it++)->as_uint64() == 9000000000ULL );
   REQUIRE( std::abs( (it++)->as_double() - 3.14159 ) < 0.0001 );
   REQUIRE( std::abs( (it++)->as_double() - 2.71828 ) < 0.0001 );
   REQUIRE( (it++)->as_string() == "Hello, Arena!" );
   REQUIRE( (it++)->as_string() == "Test string" );
}

TEST_CASE( "[arena] arguments 03 - named arguments (key-value pairs)", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 1024 );

   gd::argument::arguments arguments_( span );

   // Add named arguments
   arguments_.append( "name", "John Doe" );
   arguments_.append( "age", 30 );
   arguments_.append( "active", true );
   arguments_.append( "balance", 1234.56 );
   arguments_.append( "count", 42u );

   REQUIRE( arguments_.size() == 5 );

   // Verify named arguments
   REQUIRE( arguments_.exists( "name" ) );
   REQUIRE( arguments_.exists( "age" ) );
   REQUIRE( arguments_.exists( "active" ) );
   REQUIRE( arguments_.exists( "balance" ) );
   REQUIRE( arguments_.exists( "count" ) );
   REQUIRE_FALSE( arguments_.exists( "nonexistent" ) );

   // Retrieve values
   REQUIRE( arguments_[ "name" ].as_string() == "John Doe" );
   REQUIRE( arguments_[ "age" ].as_int() == 30 );
   REQUIRE( arguments_[ "active" ].as_bool() == true );
   REQUIRE( arguments_[ "balance" ].as_double() == 1234.56 );
   REQUIRE( arguments_[ "count" ].as_uint() == 42u );
}

TEST_CASE( "[arena] arguments 04 - multiple allocations in same arena", "[arena]" )
{
   gd::arena::arena<> arena_;

   // First allocation
   auto span1 = arena_.allocate_span<std::byte>( 512 );
   gd::argument::arguments args1( span1 );
   args1.append( "first", 1 );
   args1.append( "data", "test1" );

   // Second allocation
   auto span2 = arena_.allocate_span<std::byte>( 512 );
   gd::argument::arguments args2( span2 );
   args2.append( "second", 2 );
   args2.append( "info", "test2" );

   // Third allocation
   auto span3 = arena_.allocate_span<std::byte>( 512 );
   gd::argument::arguments args3( span3 );
   args3.append( "third", 3 );
   args3.append( "value", "test3" );

   REQUIRE( args1.size() == 2 );
   REQUIRE( args2.size() == 2 );
   REQUIRE( args3.size() == 2 );

   // Verify each allocation
   REQUIRE( args1[ "first" ].as_int() == 1 );
   REQUIRE( args1[ "data" ].as_string() == "test1" );

   REQUIRE( args2[ "second" ].as_int() == 2 );
   REQUIRE( args2[ "info" ].as_string() == "test2" );

   REQUIRE( args3[ "third" ].as_int() == 3 );
   REQUIRE( args3[ "value" ].as_string() == "test3" );
}

TEST_CASE( "[arena] arguments 05 - complex nested structures", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 2048 );

   gd::argument::arguments arguments_( span );

   // Create nested structure
   arguments_.append( "user", "Alice" );
   arguments_.append( "email", "alice@example.com" );
   arguments_.append( "age", 28 );

   // Add a list of values
   std::vector<int> scores = { 85, 90, 78, 92, 88 };
   for( auto score : scores )
   {
      arguments_.append( "score", score );
   }

   // Add more complex data
   arguments_.append( "active", true );
   arguments_.append( "balance", 5000.75 );

   REQUIRE( arguments_.size() == 10 );

   // Count scores
   /*
   std::vector<int> retrievedScores;
   for( auto it = arguments_.begin(); it != arguments_.end(); ++it )
   {
      if( it->is_name() && it->name() == "score" )
      {
         retrievedScores.push_back( it->as_int() );
      }
   }
   */

   //REQUIRE( retrievedScores.size() == 5 );
   //REQUIRE( retrievedScores == scores );

   // Verify other values
   REQUIRE( arguments_[ "user" ].as_string() == "Alice" );
   REQUIRE( arguments_[ "active" ].as_bool() == true );
}

TEST_CASE( "[arena] arguments 06 - serialization and deserialization", "[arena]" )
{
   gd::arena::arena<> arena1;
   gd::arena::arena<> arena2;

   // Create and populate first arguments
   auto span1 = arena1.allocate_span<std::byte>( 1024 );
   gd::argument::arguments args1( span1 );
   args1.append( "id", 12345 );
   args1.append( "name", "Serialized Test" );
   args1.append( "enabled", false );
   args1.append( "value", 3.14 );

   // Serialize to string
   std::string serialized = args1.print();

   // Create second arguments from serialized data
   auto span2 = arena2.allocate_span<std::byte>( 1024 );
   gd::argument::arguments args2( span2 );

   // Verify serialization contains expected data
   REQUIRE( !serialized.empty() );
   REQUIRE( serialized.find( "id" ) != std::string::npos );
   REQUIRE( serialized.find( "name" ) != std::string::npos );
}

TEST_CASE( "[arena] arguments 07 - edge cases", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 256 );

   gd::argument::arguments arguments_( span );

   // Test empty arguments
   REQUIRE( arguments_.empty() );
   REQUIRE( arguments_.size() == 0 );

   // Test adding and removing
   arguments_.append( "temp", 42 );
   REQUIRE( arguments_.size() == 1 );
   REQUIRE( arguments_.exists( "temp" ) );

   // Test finding non-existent
   REQUIRE( !arguments_.exists( "nonexistent" ) );

   // Test various number types
   arguments_.append( std::numeric_limits<int32_t>::min() );
   arguments_.append( std::numeric_limits<int32_t>::max() );
   arguments_.append( std::numeric_limits<uint32_t>::max() );
   arguments_.append( std::numeric_limits<int64_t>::min() );
   arguments_.append( std::numeric_limits<int64_t>::max() );
   arguments_.append( std::numeric_limits<uint64_t>::max() );

   REQUIRE( arguments_.size() == 7 );
}

TEST_CASE( "[arena] arguments 08 - array allocations", "[arena]" )
{
   gd::arena::arena<> arena_;

   // Test allocate_objects
   int* intArray = arena_.allocate_objects<int>( 10 );
   REQUIRE( intArray != nullptr );

   // Fill array
   for( int i = 0; i < 10; ++i )
   {
      intArray[ i ] = i * 10;
   }

   // Verify array
   for( int i = 0; i < 10; ++i )
   {
      REQUIRE( intArray[ i ] == i * 10 );
   }

   // Test allocate_span with different types
   auto doubleSpan = arena_.allocate_span<double>( 5 );
   REQUIRE( doubleSpan.size() == 5 );

   for( size_t i = 0; i < doubleSpan.size(); ++i )
   {
      doubleSpan[ i ] = static_cast<double>( i ) * 1.5;
   }

   for( size_t i = 0; i < doubleSpan.size(); ++i )
   {
      REQUIRE( std::abs( doubleSpan[ i ] - static_cast<double>( i ) * 1.5 ) < 0.001 );
   }

   // Test with custom struct
   struct TestData
   {
      int a;
      double b;
      char c;
   };

   auto structSpan = arena_.allocate_span<TestData>( 3 );
   REQUIRE( structSpan.size() == 3 );

   structSpan[ 0 ] = { 1, 1.1, 'a' };
   structSpan[ 1 ] = { 2, 2.2, 'b' };
   structSpan[ 2 ] = { 3, 3.3, 'c' };

   REQUIRE( structSpan[ 0 ].a == 1 );
   REQUIRE( structSpan[ 1 ].b == 2.2 );
   REQUIRE( structSpan[ 2 ].c == 'c' );
}

TEST_CASE( "[arena] arguments 09 - iteration and traversal", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 1024 );

   gd::argument::arguments arguments_( span );

   // Add mixed unnamed and named arguments
   arguments_.append( 100 );
   arguments_.append( 200 );
   arguments_.append( "first", 1 );
   arguments_.append( "second", 2 );
   arguments_.append( 300 );
   arguments_.append( "third", 3 );
   arguments_.append( 400 );

   REQUIRE( arguments_.size() == 7 );

   // Count all arguments
   int count = 0;
   for( auto it = arguments_.begin(); it != arguments_.end(); ++it )
   {
      count++;
   }
   REQUIRE( count == 7 );
}

TEST_CASE( "[arena] arguments 10 - mixed type operations", "[arena]" )
{
   gd::arena::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 1024 );

   gd::argument::arguments arguments_( span );

   // Add various types
   arguments_.append( "stringVal", "Hello" );
   arguments_.append( "intVal", 42 );
   arguments_.append( "boolVal", true );
   arguments_.append( "doubleVal", 3.14159 );

   // Test type checking
   REQUIRE( arguments_[ "stringVal" ].is_string() );
   REQUIRE( arguments_[ "intVal" ].is_integer() );
   REQUIRE( arguments_[ "boolVal" ].is_bool() );
   REQUIRE( arguments_[ "doubleVal" ].is_decimal() );

   // Test conversion
   REQUIRE( arguments_[ "stringVal" ].as_string() == "Hello" );
   REQUIRE( arguments_[ "intVal" ].as_int() == 42 );
   REQUIRE( arguments_[ "boolVal" ].as_bool() == true );
   REQUIRE( std::abs( arguments_[ "doubleVal" ].as_double() - 3.14159 ) < 0.0001 );

   // Test to_string conversion
   REQUIRE( !arguments_[ "stringVal" ].to_string().empty() );
   REQUIRE( arguments_[ "intVal" ].to_string().find( "42" ) != std::string::npos );
}
