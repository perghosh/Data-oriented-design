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

TEST_CASE( "[session] vector - default construction", "[session]" )
{
   gd::stack::vector<std::byte, 128> vec;

   gd::argument::arguments arguments_( vec );

   arguments_.append("test", "test");
   arguments_.append("test1", "test1");
   arguments_.append("test2", "test2");

   REQUIRE(arguments_["test1"].as_string() == "test1");
}

TEST_CASE( "[session] vector - size construction", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec(3, 42);
   
   REQUIRE_FALSE(vec.empty());
   REQUIRE(vec.size() == 3);
   REQUIRE(vec.capacity() == 5);
   REQUIRE(vec[0] == 42);
   REQUIRE(vec[1] == 42);
   REQUIRE(vec[2] == 42);
}

TEST_CASE( "[session] vector - size construction exceeds inline capacity", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec(10, 42);
   
   REQUIRE_FALSE(vec.empty());
   REQUIRE(vec.size() == 10);
   REQUIRE(vec.capacity() == 10);
   REQUIRE(vec[0] == 42);
   REQUIRE(vec[9] == 42);
}

TEST_CASE( "[session] vector - initializer list construction", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3};
   
   REQUIRE(vec.size() == 3);
   REQUIRE(vec[0] == 1);
   REQUIRE(vec[1] == 2);
   REQUIRE(vec[2] == 3);
}

TEST_CASE( "[session] vector - initializer list exceeds inline capacity", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5, 6, 7, 8};
   
   REQUIRE(vec.size() == 8);
   REQUIRE(vec.capacity() == 8);
   REQUIRE(vec[0] == 1);
   REQUIRE(vec[7] == 8);
}

TEST_CASE( "[session] vector - iterator range construction", "[session]" )
{
   std::vector<uint32_t> src = {10, 20, 30, 40};
   gd::stack::vector<uint32_t, 5> vec(src.begin(), src.end());
   
   REQUIRE(vec.size() == 4);
   REQUIRE(vec[0] == 10);
   REQUIRE(vec[1] == 20);
   REQUIRE(vec[2] == 30);
   REQUIRE(vec[3] == 40);
}

TEST_CASE( "[session] vector - iterator range exceeds inline capacity", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2(vec1.begin(), vec1.end());
   
   REQUIRE(vec2.size() == 10);
   for(auto i = 0u; i < 10; i++)
   {
      REQUIRE(vec2[i] == i);
   }
}

TEST_CASE( "[session] vector - copy construction", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 3; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2(vec1);
   
   REQUIRE(vec2.size() == vec1.size());
   REQUIRE(vec2[0] == 0);
   REQUIRE(vec2[1] == 1);
   REQUIRE(vec2[2] == 2);
}

TEST_CASE( "[session] vector - copy construction from heap-allocated vector", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2(vec1);
   
   REQUIRE(vec2.size() == vec1.size());
   for(auto i = 0u; i < 10; i++)
   {
      REQUIRE(vec2[i] == i);
   }
}

TEST_CASE( "[session] vector - move construction", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 3; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2(std::move(vec1));
   
   REQUIRE(vec2.size() == 3);
   REQUIRE(vec2[0] == 0);
   REQUIRE(vec2[1] == 1);
   REQUIRE(vec2[2] == 2);
}

TEST_CASE( "[session] vector - move construction from heap-allocated vector", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   
   auto pHeapBuffer = vec1.data();
   
   gd::stack::vector<uint32_t, 5> vec2(std::move(vec1));
   
   REQUIRE(vec2.size() == 10);
   REQUIRE(vec1.size() == 0);
   REQUIRE(vec2.data() == pHeapBuffer); // Should have taken ownership
}

TEST_CASE( "[session] vector - copy assignment", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec2 = {4, 5};
   
   vec2 = vec1;
   
   REQUIRE(vec2.size() == 3);
   REQUIRE(vec2[0] == 1);
   REQUIRE(vec2[1] == 2);
   REQUIRE(vec2[2] == 3);
}

TEST_CASE( "[session] vector - copy assignment to larger", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2};
   gd::stack::vector<uint32_t, 5> vec2 = {3, 4, 5, 6, 7};
   
   vec2 = vec1;
   
   REQUIRE(vec2.size() == 2);
   REQUIRE(vec2[0] == 1);
   REQUIRE(vec2[1] == 2);
}

TEST_CASE( "[session] vector - copy assignment from heap-allocated", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2 = {100, 200};
   vec2 = vec1;
   
   REQUIRE(vec2.size() == 10);
   for(auto i = 0u; i < 10; i++)
   {
      REQUIRE(vec2[i] == i);
   }
}

TEST_CASE( "[session] vector - move assignment", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec2 = {4, 5};
   
   vec2 = std::move(vec1);
   
   REQUIRE(vec2.size() == 3);
   REQUIRE(vec2[0] == 1);
   REQUIRE(vec2[1] == 2);
   REQUIRE(vec2[2] == 3);
   REQUIRE(vec1.size() == 0);
}

TEST_CASE( "[session] vector - move assignment from heap-allocated", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   auto pHeapBuffer = vec1.data();
   
   gd::stack::vector<uint32_t, 5> vec2 = {100, 200};
   vec2 = std::move(vec1);
   
   REQUIRE(vec2.size() == 10);
   REQUIRE(vec1.size() == 0);
   REQUIRE(vec2.data() == pHeapBuffer);
}

TEST_CASE( "[session] vector - push_back const ref", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   uint32_t val = 42;
   
   vec.push_back(val);
   vec.push_back(val);
   
   REQUIRE(vec.size() == 2);
   REQUIRE(vec[0] == 42);
   REQUIRE(vec[1] == 42);
}

TEST_CASE( "[session] vector - push_back rvalue", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   vec.push_back(10);
   vec.push_back(20);
   vec.push_back(30);
   
   REQUIRE(vec.size() == 3);
   REQUIRE(vec[0] == 10);
   REQUIRE(vec[1] == 20);
   REQUIRE(vec[2] == 30);
}

TEST_CASE( "[session] vector - push_back exceeds inline capacity", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   for(auto i = 0u; i < 10; i++)
   {
      vec.push_back(i);
   }
   
   REQUIRE(vec.size() == 10);
   REQUIRE(vec.capacity() >= 10);
   for(auto i = 0u; i < 10; i++)
   {
      REQUIRE(vec[i] == i);
   }
}

TEST_CASE( "[session] vector - emplace_back", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   auto& ref = vec.emplace_back(42);
   
   REQUIRE(vec.size() == 1);
   REQUIRE(vec[0] == 42);
   REQUIRE(ref == 42);
}

TEST_CASE( "[session] vector - emplace_back multiple arguments", "[session]" )
{
   gd::stack::vector<std::pair<uint32_t, uint32_t>, 5> vec;
   
   vec.emplace_back(1, 2);
   vec.emplace_back(3, 4);
   
   REQUIRE(vec.size() == 2);
   REQUIRE(vec[0].first == 1);
   REQUIRE(vec[0].second == 2);
   REQUIRE(vec[1].first == 3);
   REQUIRE(vec[1].second == 4);
}

TEST_CASE( "[session] vector - pop_back", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   vec.pop_back();
   REQUIRE(vec.size() == 4);
   REQUIRE(vec.back() == 4);
   
   vec.pop_back();
   REQUIRE(vec.size() == 3);
   REQUIRE(vec.back() == 3);
}

TEST_CASE( "[session] vector - operator[]", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {10, 20, 30};
   
   REQUIRE(vec[0] == 10);
   REQUIRE(vec[1] == 20);
   REQUIRE(vec[2] == 30);
}

TEST_CASE( "[session] vector - const operator[]", "[session]" )
{
   const gd::stack::vector<uint32_t, 5> vec = {10, 20, 30};
   
   REQUIRE(vec[0] == 10);
   REQUIRE(vec[1] == 20);
   REQUIRE(vec[2] == 30);
}

TEST_CASE( "[session] vector - at()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {10, 20, 30};
   
   REQUIRE(vec.at(0) == 10);
   REQUIRE(vec.at(1) == 20);
   REQUIRE(vec.at(2) == 30);
}

TEST_CASE( "[session] vector - at() out of range throws", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {10, 20, 30};
   
   REQUIRE_THROWS_AS(vec.at(3), std::out_of_range);
   REQUIRE_THROWS_AS(vec.at(10), std::out_of_range);
}

TEST_CASE( "[session] vector - front() and back()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   REQUIRE(vec.front() == 1);
   REQUIRE(vec.back() == 5);
   
   vec.front() = 10;
   vec.back() = 50;
   
   REQUIRE(vec[0] == 10);
   REQUIRE(vec[4] == 50);
}

TEST_CASE( "[session] vector - const front() and back()", "[session]" )
{
   const gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   REQUIRE(vec.front() == 1);
   REQUIRE(vec.back() == 5);
}

TEST_CASE( "[session] vector - data()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3};
   
   uint32_t* pData = vec.data();
   REQUIRE(pData[0] == 1);
   REQUIRE(pData[1] == 2);
   REQUIRE(pData[2] == 3);
}

TEST_CASE( "[session] vector - const data()", "[session]" )
{
   const gd::stack::vector<uint32_t, 5> vec = {1, 2, 3};
   
   const uint32_t* pData = vec.data();
   REQUIRE(pData[0] == 1);
   REQUIRE(pData[1] == 2);
   REQUIRE(pData[2] == 3);
}

TEST_CASE( "[session] vector - iterators", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   auto it = vec.begin();
   REQUIRE(*it == 1);
   ++it;
   REQUIRE(*it == 2);
   
   auto itEnd = vec.end();
   REQUIRE(*(itEnd - 1) == 5);
}

TEST_CASE( "[session] vector - const iterators", "[session]" )
{
   const gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   auto it = vec.begin();
   REQUIRE(*it == 1);
   ++it;
   REQUIRE(*it == 2);
   
   auto itEnd = vec.end();
   REQUIRE(*(itEnd - 1) == 5);
}

TEST_CASE( "[session] vector - cbegin/cend", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3};
   
   int sum = 0;
   for(auto it = vec.cbegin(); it != vec.cend(); ++it)
   {
      sum += *it;
   }
   REQUIRE(sum == 6);
}

TEST_CASE( "[session] vector - reverse iterators", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   auto rit = vec.rbegin();
   REQUIRE(*rit == 5);
   ++rit;
   REQUIRE(*rit == 4);
   
   auto ritEnd = vec.rend();
   REQUIRE(*(ritEnd - 1) == 1);
}

TEST_CASE( "[session] vector - const reverse iterators", "[session]" )
{
   const gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   auto rit = vec.rbegin();
   REQUIRE(*rit == 5);
   ++rit;
   REQUIRE(*rit == 4);
}

TEST_CASE( "[session] vector - crbegin/crend", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3};
   
   auto rit = vec.crbegin();
   REQUIRE(*rit == 3);
   ++rit;
   REQUIRE(*rit == 2);
   ++rit;
   REQUIRE(*rit == 1);
   REQUIRE(rit + 1 == vec.crend());
}

TEST_CASE( "[session] vector - empty()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   REQUIRE(vec.empty());
   
   vec.push_back(1);
   REQUIRE_FALSE(vec.empty());
}

TEST_CASE( "[session] vector - size()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   REQUIRE(vec.size() == 0);
   
   vec.push_back(1);
   REQUIRE(vec.size() == 1);
   
   vec.push_back(2);
   REQUIRE(vec.size() == 2);
}

TEST_CASE( "[session] vector - capacity()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   REQUIRE(vec.capacity() == 5);
   
   for(auto i = 0u; i < 10; i++)
   {
      vec.push_back(i);
   }
   REQUIRE(vec.capacity() >= 10);
}

TEST_CASE( "[session] vector - inline_capacity()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   REQUIRE(vec.inline_capacity() == 5);
   
   gd::stack::vector<uint32_t, 10> vec2;
   REQUIRE(vec2.inline_capacity() == 10);
   
   REQUIRE(gd::stack::vector<uint32_t, 5>::inline_capacity() == 5);
}

TEST_CASE( "[session] vector - reserve()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   vec.reserve(10);
   REQUIRE(vec.capacity() >= 10);
   REQUIRE(vec.size() == 0);
   
   vec.reserve(5); // Should not shrink
   REQUIRE(vec.capacity() >= 10);
}

TEST_CASE( "[session] vector - resize() default", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3};
   
   vec.resize(5);
   REQUIRE(vec.size() == 5);
   REQUIRE(vec[0] == 1);
   REQUIRE(vec[1] == 2);
   REQUIRE(vec[2] == 3);
   REQUIRE(vec[3] == 0);
   REQUIRE(vec[4] == 0);
}

TEST_CASE( "[session] vector - resize() shrink", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   vec.resize(3);
   REQUIRE(vec.size() == 3);
   REQUIRE(vec[0] == 1);
   REQUIRE(vec[1] == 2);
   REQUIRE(vec[2] == 3);
}

TEST_CASE( "[session] vector - resize() with value", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2};
   
   vec.resize(5, 99);
   REQUIRE(vec.size() == 5);
   REQUIRE(vec[0] == 1);
   REQUIRE(vec[1] == 2);
   REQUIRE(vec[2] == 99);
   REQUIRE(vec[3] == 99);
   REQUIRE(vec[4] == 99);
}

TEST_CASE( "[session] vector - resize() exceeds inline capacity", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2};
   
   vec.resize(10, 42);
   REQUIRE(vec.size() == 10);
   REQUIRE(vec[0] == 1);
   REQUIRE(vec[1] == 2);
   for(auto i = 2u; i < 10; i++)
   {
      REQUIRE(vec[i] == 42);
   }
}

TEST_CASE( "[session] vector - clear()", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec = {1, 2, 3, 4, 5};
   
   vec.clear();
   REQUIRE(vec.empty());
   REQUIRE(vec.size() == 0);
   REQUIRE(vec.capacity() == 5); // Capacity should remain
}

TEST_CASE( "[session] vector - clear() heap-allocated", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   for(auto i = 0u; i < 10; i++) { vec.push_back(i); }
   
   vec.clear();
   REQUIRE(vec.empty());
   REQUIRE(vec.size() == 0);
}

TEST_CASE( "[session] vector - swap() inline", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec2 = {4, 5};
   
   vec1.swap(vec2);
   
   REQUIRE(vec1.size() == 2);
   REQUIRE(vec1[0] == 4);
   REQUIRE(vec1[1] == 5);
   
   REQUIRE(vec2.size() == 3);
   REQUIRE(vec2[0] == 1);
   REQUIRE(vec2[1] == 2);
   REQUIRE(vec2[2] == 3);
}

TEST_CASE( "[session] vector - swap() with heap-allocated", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2 = {100, 200};
   
   vec1.swap(vec2);
   
   REQUIRE(vec1.size() == 2);
   REQUIRE(vec1[0] == 100);
   REQUIRE(vec1[1] == 200);
   
   REQUIRE(vec2.size() == 10);
   for(auto i = 0u; i < 10; i++)
   {
      REQUIRE(vec2[i] == i);
   }
}

TEST_CASE( "[session] vector - swap() both heap-allocated", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1;
   for(auto i = 0u; i < 10; i++) { vec1.push_back(i); }
   
   gd::stack::vector<uint32_t, 5> vec2;
   for(auto i = 0u; i < 8; i++) { vec2.push_back(100 + i); }
   
   vec1.swap(vec2);
   
   REQUIRE(vec1.size() == 8);
   for(auto i = 0u; i < 8; i++)
   {
      REQUIRE(vec1[i] == 100 + i);
   }
   
   REQUIRE(vec2.size() == 10);
   for(auto i = 0u; i < 10; i++)
   {
      REQUIRE(vec2[i] == i);
   }
}

TEST_CASE( "[session] vector - equality operator", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec2 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec3 = {1, 2, 4};
   gd::stack::vector<uint32_t, 5> vec4 = {1, 2};
   
   REQUIRE(vec1 == vec2);
   REQUIRE_FALSE(vec1 == vec3);
   REQUIRE_FALSE(vec1 == vec4);
}

TEST_CASE( "[session] vector - inequality operator", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec2 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec3 = {1, 2, 4};
   
   REQUIRE_FALSE(vec1 != vec2);
   REQUIRE(vec1 != vec3);
}

TEST_CASE( "[session] vector - comparison operators", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec1 = {1, 2, 3};
   gd::stack::vector<uint32_t, 5> vec2 = {1, 2, 4};
   gd::stack::vector<uint32_t, 5> vec3 = {1, 2, 3, 4};
   
   REQUIRE(vec1 < vec2);
   REQUIRE(vec1 <= vec2);
   REQUIRE_FALSE(vec1 > vec2);
   REQUIRE_FALSE(vec1 >= vec2);
   
   REQUIRE(vec1 < vec3); // Shorter is less when prefixes match
   REQUIRE_FALSE(vec3 < vec1);
}

TEST_CASE( "[session] vector - with string type", "[session]" )
{
   gd::stack::vector<std::string, 3> vec;
   
   vec.push_back("hello");
   vec.push_back("world");
   vec.push_back("!");
   
   REQUIRE(vec.size() == 3);
   REQUIRE(vec[0] == "hello");
   REQUIRE(vec[1] == "world");
   REQUIRE(vec[2] == "!");
}

TEST_CASE( "[session] vector - with string type move", "[session]" )
{
   gd::stack::vector<std::string, 3> vec;
   
   std::string str1 = "hello";
   std::string str2 = "world";
   
   vec.push_back(std::move(str1));
   vec.emplace_back(std::move(str2));
   
   REQUIRE(vec.size() == 2);
   REQUIRE(vec[0] == "hello");
   REQUIRE(vec[1] == "world");
   REQUIRE(str1.empty()); // Moved from
   REQUIRE(str2.empty()); // Moved from
}

TEST_CASE( "[session] vector - inline to heap transition", "[session]" )
{
   gd::stack::vector<uint32_t, 5> vec;
   
   // Initially using inline storage
   vec.push_back(1);
   vec.push_back(2);
   REQUIRE(vec.capacity() == 5);
   
   // Add elements to trigger heap allocation
   vec.push_back(3);
   vec.push_back(4);
   vec.push_back(5);
   REQUIRE(vec.capacity() == 5);
   
   // This should trigger allocation
   vec.push_back(6);
   REQUIRE(vec.capacity() >= 6);
   REQUIRE(vec.size() == 6);
   
   // Verify all values
   for(auto i = 0u; i < 6; i++)
   {
      REQUIRE(vec[i] == i + 1);
   }
}

TEST_CASE("[session] test uri logic", "[session]") 
{
   CSessions sessions;
   sessions.Initialize(20000);

   for(auto u = 0u; u <= 1000; u++)
   {
      sessions.Add();
   }
}