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
   // file:///C:/dev/home/DOD/target/server/http/Router.cpp() <-- this works
   // file:///C:/dev/home/DOD/target/server/http/Router.cpp(80) <-- this DO NOT work :(
}
