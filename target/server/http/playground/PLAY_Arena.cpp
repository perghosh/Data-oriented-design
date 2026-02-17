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
#include "gd/gd_arena_borrow.h"
#include "gd/gd_arguments.h"
#include "gd/gd_vector.h"

#include "../Session.h"
#include "../Document.h"
#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[arena] std::vector", "[arena]" )
{
   using namespace gd::memory;
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
   using namespace gd::memory;
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
   gd::memory::arena<> arena_;

   auto span = arena_.allocate_span<std::byte>( 256 );

   gd::argument::arguments arguments_( span );

   REQUIRE( arguments_.empty() );
   REQUIRE( arguments_.size() == 0 );
}

TEST_CASE( "[arena] arguments 02 - append and retrieve various types", "[arena]" )
{
   gd::memory::arena<> arena_;

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
   gd::memory::arena<> arena_;

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
   gd::memory::arena<> arena_;

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
   gd::memory::arena<> arena_;

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

   // Verify other values
   REQUIRE( arguments_[ "user" ].as_string() == "Alice" );
   REQUIRE( arguments_[ "active" ].as_bool() == true );
}

TEST_CASE( "[arena] arguments 06 - serialization and deserialization", "[arena]" )
{
   gd::memory::arena<> arena1;
   gd::memory::arena<> arena2;

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
   gd::memory::arena<> arena_;

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
   gd::memory::arena<> arena_;

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
   gd::memory::arena<> arena_;

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
   gd::memory::arena<> arena_;

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

TEST_CASE( "[arena] iterator_block - multiple blocks iteration", "[arena]" )
{
   using namespace gd::memory;
   
   // Create arena with small block size (256 bytes) to force multiple blocks
   arena<> myArena(256);
   
   std::cout << "\n=== Test: iterator_block - multiple blocks ===\n";
   
   // Make many small allocations (each about 16-24 bytes with alignment)
   std::vector<int*> allocations;
   const int numAllocations = 100;
   
   for( int i = 0; i < numAllocations; ++i )
   {
      int* p = myArena.allocate_objects<int>( 1 );
      REQUIRE( p != nullptr );
      *p = i;
      allocations.push_back( p );
   }
   
   // Verify we created multiple blocks
   REQUIRE( myArena.block_count() > 1 );
   
   std::cout << "Total blocks created: " << myArena.block_count() << "\n";
   std::cout << "Total allocated: " << myArena.total_allocated() << " bytes\n";
   
   // Test block iteration
   int blockCount = 0;
   size_t totalCapacity = 0;
   size_t totalUsed = 0;
   size_t totalAllocCount = 0;
   
   for( auto it = myArena.begin_block(); it != myArena.end_block(); ++it )
   {
      blockCount++;
      REQUIRE( it->is_valid() );
      REQUIRE( it->m_uBlockSize > 0 );
      
      totalCapacity += it->m_uBlockSize;
      totalUsed += it->m_uUsedSize;
      totalAllocCount += it->m_uAllocCount;
      
      std::cout << "Block " << blockCount << ": size=" << it->m_uBlockSize 
                << ", used=" << it->m_uUsedSize 
                << ", allocs=" << it->m_uAllocCount << "\n";
   }
   
   REQUIRE( blockCount == myArena.block_count() );
   REQUIRE( totalCapacity == myArena.total_capacity() );
   REQUIRE( totalUsed == myArena.total_allocated() );
   
   std::cout << "Block iteration successful - visited " << blockCount << " blocks\n";
   
   // Verify all allocations are still valid
   for( int i = 0; i < numAllocations; ++i )
   {
      REQUIRE( *allocations[i] == i );
   }
}

TEST_CASE( "[arena] iterator_allocation - multiple allocations iteration", "[arena]" )
{
   using namespace gd::memory;
   
   // Create arena with small block size (512 bytes) to force multiple blocks
   arena<> myArena(512);
   
   std::cout << "\n=== Test: iterator_allocation - multiple allocations ===\n";
   
   // Make many small allocations of different sizes
   const int numInts = 50;
   const int numDoubles = 30;
   const int numShorts = 40;
   
   std::vector<int*> intAllocs;
   std::vector<double*> doubleAllocs;
   std::vector<short*> shortAllocs;
   
   // Allocate integers
   for( int i = 0; i < numInts; ++i )
   {
      int* p = myArena.allocate_objects<int>( 1 );
      REQUIRE( p != nullptr );
      *p = i;
      intAllocs.push_back( p );
   }
   
   // Allocate doubles
   for( int i = 0; i < numDoubles; ++i )
   {
      double* p = myArena.allocate_objects<double>( 1 );
      REQUIRE( p != nullptr );
      *p = i * 1.5;
      doubleAllocs.push_back( p );
   }
   
   // Allocate shorts
   for( int i = 0; i < numShorts; ++i )
   {
      short* p = myArena.allocate_objects<short>( 1 );
      REQUIRE( p != nullptr );
      *p = static_cast<short>(i);
      shortAllocs.push_back( p );
   }
   
   const int expectedAllocations = int(numInts + numDoubles + numShorts);
   const int expectedBlocks = (int)myArena.block_count();
   
   std::cout << "Total allocations made: " << expectedAllocations << "\n";
   std::cout << "Total blocks created: " << expectedBlocks << "\n";
   
   REQUIRE( expectedBlocks > 1 );
   
   // Test allocation iteration
   int allocationCount = 0;
   int blocksVisited = 0;
}

TEST_CASE( "[arena::borrow] std::string allocation count", "[arena][borrow]" ) {
   std::array<std::byte, 2048> buffer;
   gd::arena::borrow::arena arena_( buffer );
   gd::arena::borrow::arena_allocator<char> allocator(arena_);
   std::basic_string<char, std::char_traits<char>, gd::arena::borrow::arena_allocator<char>> string_(allocator);

   //string_.reserve( 100 );

   // add 500 characters to force multiple blocks
   for( int i = 0; i < 600; ++i )
   {
      string_ += "x";
   }

   std::cout << "String length: " << string_.length() << "\n";
   std::cout << "Used: " << arena_.used() << " and capacity: " << arena_.capacity() << "\n";
}

TEST_CASE( "[arena::borrow] string and vector", "[arena][borrow]" ) {
   // Initialize arena requesting 2048 bytes from heap
   std::array<std::byte, 2048> buffer;
   gd::arena::borrow::arena arena_( buffer );
 
   for( int i = 0; i < 10; ++i )
   {
      arena_.reset();
      gd::arena::borrow::arena_allocator<char> allocator(arena_);
      std::basic_string<char, std::char_traits<char>, gd::arena::borrow::arena_allocator<char>> string_(allocator);

      string_ += "Hello from arena allocator!";
      string_ += " This string is allocated in an arena.";
      string_ += " Additional text.";

      std::vector<int, gd::arena::borrow::arena_allocator<int>> vec{ gd::arena::borrow::arena_allocator<int>( arena_ ) };
      vec.reserve( 20 );
      for( int j = 0; j < 20; ++j )
      {
         vec.push_back( j );
      }

      for( auto& val : vec )
      {
         string_ += std::to_string( val ) + " ";
      }

      std::cout << "String: " << string_ << "\n";
      std::cout << "Used: " << arena_.used() << " and capacity: " << arena_.capacity() << "\n";
   }

   arena_.reset();
   int* piBuffer = arena_.allocate_objects<int>( 100 ); // Allocate some more to test reuse after reset
   for( int i = 0; i < 100; ++i )
   {
      piBuffer[ i ] = i * 10;
   }

   // sum numbers to verify allocation is working
   int sum = 0;
   for( int i = 0; i < 100; ++i ) 
   {
      sum += piBuffer[ i ];
   }
   std::cout << "Used: " << arena_.used() << " and capacity: " << arena_.capacity() << "\n";
}


// A type with very strict alignment requirements
struct alignas(64) HighAlign {
    float data[16];
};


TEST_CASE( "[arena::borrow] simd", "[arena][borrow]" ) {
    alignas(64) std::array<std::byte, 1024> buffer;
    gd::arena::borrow::arena my_arena(buffer);
    gd::arena::borrow::arena_allocator<HighAlign> alloc(my_arena);

    std::cout << "--- Starting Alignment Test ---\n";

    // 1. Test Arena Allocation Alignment
    HighAlign* p1 = alloc.allocate(1);
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(p1);
    std::cout << "Arena Addr: " << addr1 << " | Alignment: " << (addr1 % 64 == 0 ? "PASS" : "FAIL") << "\n";
    assert(addr1 % 64 == 0);
    assert(my_arena.contains(p1));

    // 2. Test Heap Fallback Alignment
    // Force fallback by allocating something huge
    HighAlign* p2 = alloc.allocate(100); 
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(p2);
    
    std::cout << "Heap Addr:  " << addr2 << " | Alignment: " << (addr2 % 64 == 0 ? "PASS" : "FAIL") << "\n";
    
    // If this fails, the 'paddedHeaderSize' logic in the previous step is missing!
    assert(addr2 % 64 == 0); 
    assert(!my_arena.contains(p2)); // Should be on heap

    // 3. Test Deallocation of Fallback
    // This should not crash and should correctly handle the padded header
    alloc.deallocate(p2, 100);
    
    std::cout << "Cleanup successful. All assertions passed.\n";
}




// Helper to check alignment
bool is_aligned(void* ptr, std::size_t alignment) {
    return (reinterpret_cast<std::uintptr_t>(ptr) & (alignment - 1)) == 0;
}

TEST_CASE( "[arena::borrow] lifecycle 01 - borrowed storage (std::array)", "[arena][borrow]" ) {
    // Setup a buffer on the stack
    std::array<std::byte, 1024> buffer;
    
    // Initialize arena borrowing the buffer
    gd::arena::borrow::arena arena(buffer);

    SECTION("Check initial state properties") {
        REQUIRE( arena.capacity() == 1024 );
        REQUIRE( arena.used() == 0 );
        REQUIRE( arena.available() == 1024 );
        
        // Logic checks for ownership flags
        REQUIRE( arena.is_borrowed() == true );
        REQUIRE( arena.owner() == false );
    }

    SECTION("Basic allocation works") {
        void* p1 = arena.allocate(128);
        REQUIRE( p1 != nullptr );
        REQUIRE( arena.contains(p1) );
        REQUIRE( arena.used() == 128 );
        REQUIRE( arena.available() == (1024 - 128) );

        // Ensure we are actually inside the source buffer
        REQUIRE( p1 >= buffer.data() );
        REQUIRE( p1 < buffer.data() + buffer.size() );
    }

    SECTION("Reset clears usage but keeps storage") {
        void* p1 = arena.allocate(500);
        REQUIRE( arena.used() == 500 );
        
        arena.reset();
        
        REQUIRE( arena.used() == 0 );
        REQUIRE( arena.available() == 1024 );
        
        // Allocation after reset should succeed (reuse memory)
        void* p2 = arena.allocate(500);
        REQUIRE( p2 != nullptr );
        // In a bump allocator, p2 usually equals p1 if alignment is identical
        REQUIRE( p2 == p1 ); 
    }
}

TEST_CASE( "[arena::borrow] lifecycle 02 - owned storage (heap)", "[arena][borrow]" ) {
    // Initialize arena requesting 2048 bytes from heap
    gd::arena::borrow::arena arena(nullptr, 2048);

    REQUIRE( arena.capacity() == 2048 );
    REQUIRE( arena.is_borrowed() == false );
    REQUIRE( arena.owner() == true );

    void* p1 = arena.allocate(100);
    REQUIRE( p1 != nullptr );
    REQUIRE( arena.contains(p1) );
}

TEST_CASE( "[arena::borrow] allocation 03 - alignment and exhaustion", "[arena][borrow]" ) {
    std::array<std::byte, 100> buffer;
    gd::arena::borrow::arena arena(buffer);

    SECTION("Alignment padding is added correctly") {
        // Allocate 1 byte (misaligns the bump pointer)
        void* p1 = arena.allocate(1); 
        REQUIRE( p1 != nullptr );
        REQUIRE( arena.used() == 1 );

        // Allocate double (needs 8-byte alignment)
        // If p1 was at 0, next is at 1. Next aligned 8 is 8. Padding = 7 bytes.
        // Used should become 8 + sizeof(double) = 16.
        void* p2 = arena.allocate(sizeof(double), alignof(double));
        
        REQUIRE( p2 != nullptr );
        REQUIRE( is_aligned(p2, alignof(double)) );
        REQUIRE( arena.used() >= (1 + sizeof(double)) ); 
    }

    SECTION("Exhaustion returns nullptr") {
        // Consume almost all
        auto temp_ =arena.allocate(80);
        REQUIRE( arena.available() == 20 );

        // Try to allocate more than available
        void* pFail = arena.allocate(21);
        REQUIRE( pFail == nullptr );
        
        // State remains unchanged
        REQUIRE( arena.used() == 80 );
    }
}

TEST_CASE( "[arena::borrow] adapter 04 - STL allocator fallback logic", "[arena][borrow]" ) {
    // Create a small arena to force fallback
    std::array<std::byte, 128> small_buffer;
    gd::arena::borrow::arena arena(small_buffer);

    using Allocator = gd::arena::borrow::arena_allocator<int>;
    std::vector<int, Allocator> vec{ Allocator(&arena) };

    SECTION("Allocations fit in arena") {
        vec.reserve(10); // 10 * 4 bytes = 40 bytes (fits in 128)
        
        REQUIRE( vec.capacity() >= 10 );
        REQUIRE( arena.used() > 0 );
        REQUIRE( arena.contains(vec.data()) );
        
        vec.push_back(42);
        REQUIRE( vec[0] == 42 );
    }

    SECTION("Fallback to heap when arena is full") {
        // 1. Fill the arena manually so the vector can't fit there
        void* dummy = arena.allocate(100); 
        REQUIRE( dummy != nullptr );
        REQUIRE( arena.available() < 40 ); // Less than needed for vector below

        // 2. Trigger vector allocation
        // This should trigger the `::operator new` fallback in the allocator header logic
        vec.reserve(20); // 80 bytes needed, won't fit
        
        REQUIRE( vec.capacity() >= 20 );
        
        // 3. Verify logic: Data is valid, but NOT in arena
        vec.push_back(999);
        REQUIRE( vec[0] == 999 );
        REQUIRE_FALSE( arena.contains(vec.data()) ); 
    }
    
    SECTION("Fallback on null arena") {
        // Construct allocator with nullptr (pure heap mode)
        Allocator heapAlloc(nullptr);
        std::vector<int, Allocator> heapVec(heapAlloc);
        
        heapVec.push_back(100);
        REQUIRE( heapVec[0] == 100 );
        
        // No arena involved, simply ensure no crash
    }
}

#ifdef _WIN32

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

/** * Helper to check for leaks within a specific scope.
 */
struct LeakCheck {
    _CrtMemState s1, s2, s3;
    LeakCheck() { _CrtMemCheckpoint(&s1); }
    ~LeakCheck() {
        _CrtMemCheckpoint(&s2);
        if (_CrtMemDifference(&s3, &s1, &s2)) {
            _CrtMemDumpStatistics(&s3);
            // This will trigger a failure in Catch2 if leaks are detected
            FAIL("Memory leak detected! Check output window for details.");
        }
    }
};

TEST_CASE( "[arena::borrow] memory 05 - owned arena heap cleanup", "[arena][borrow][windows]" ) {
    SECTION("Arena-owned buffer is released on destruction") {
        LeakCheck check; // Check for leaks in this scope
        {
            // Arena allocates 4KB on heap
            gd::arena::borrow::arena arena(nullptr, 4096);
            REQUIRE(arena.owner() == true);
            void* p = arena.allocate(100);
            REQUIRE(p != nullptr);
        }
        // arena goes out of scope, calls destroy() which calls ::operator delete
    }
}

TEST_CASE( "[arena::borrow] memory 06 - allocator fallback cleanup", "[arena][borrow][windows]" ) {
    SECTION("STL allocator cleans up heap fallbacks") {
        LeakCheck check;
        {
            // Small arena to force fallback
            gd::arena::borrow::arena arena(nullptr, 64); 
            using Allocator = gd::arena::borrow::arena_allocator<int>;
            std::vector<int, Allocator> vec((Allocator(&arena)));

            // 1. This fits in the 64-byte arena
            vec.push_back(1); 
            
            // 2. This resize will exceed 64 bytes, forcing a heap fallback allocation
            // The allocator writes an allocation_header to the heap
            vec.resize(100); 
            
            REQUIRE_FALSE(arena.contains(vec.data()));
        }
        // 1. vector goes out of scope, calling deallocate()
        // 2. deallocate() detects the pointer is not in the arena
        // 3. deallocate() adjusts the pointer to the header and deletes it
    }
}



TEST_CASE( "[arena::borrow] stress 08 - heavy allocations with leak check", "[arena][borrow][stress][windows]" ) {
    // 1. Setup Windows leak detection for this scope
    _CrtMemState sOld, sNew, sDiff;
    _CrtMemCheckpoint(&sOld);

    {
        const std::size_t ARENA_SIZE = 1024 * 512; // 512KB Arena
        gd::arena::borrow::arena arena(nullptr, ARENA_SIZE);
        
        using Allocator = gd::arena::borrow::arena_allocator<std::byte>;
        Allocator alloc(&arena);

        struct Rec { std::byte* p; std::size_t s; };
        std::vector<Rec> allocations;
        
        std::mt19937 rng(123);
        std::uniform_int_distribution<std::size_t> dist(64, 10000);

        // 2. Perform enough allocations to force many heap fallbacks
        // Total potential memory: 1000 * ~5KB = ~5MB (10x the arena size)
        for(int i = 0; i < 1000; ++i) {
            std::size_t size = dist(rng);
            std::byte* p = alloc.allocate(size);
            
            if (p) {
                allocations.push_back({p, size});
                std::memset(p, 0xAA, size); // Touch memory to ensure it's mapped
            }
        }

        // 3. Cleanup: Deallocate everything
        // This tests that the allocator correctly identifies arena vs heap pointers
        for(auto& a : allocations) {
            alloc.deallocate(a.p, a.s);
        }
    } 
    // Arena is destroyed here. If it owned memory, it calls destroy().

    // 4. Final Verification
    _CrtMemCheckpoint(&sNew);
    if (_CrtMemDifference(&sDiff, &sOld, &sNew)) {
        _CrtMemDumpStatistics(&sDiff);
        FAIL("Stress test leaked memory! Check the Debug Output window.");
    }
}

#endif