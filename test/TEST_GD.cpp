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

// ## Tests for gd::strings32

TEST_CASE("strings32 Constructors and Assignment", "[GD]") {
   std::cout << "check `gd::strings32` constructors and assignment" << std::endl;

   { gd::strings32 strings; REQUIRE(strings.size() == 0); }
   { gd::strings32 strings("test"); REQUIRE(strings.size() == 1); REQUIRE(strings[0] == "test"); }
   { std::string_view sv = "test"; gd::strings32 strings(sv); REQUIRE(strings.size() == 1); REQUIRE(strings[0] == "test"); }
   { std::string s = "test"; gd::strings32 strings(s); REQUIRE(strings.size() == 1); REQUIRE(strings[0] == "test"); }
   { std::string s = "test"; gd::strings32 strings(std::move(s)); REQUIRE(strings.size() == 1); REQUIRE(strings[0] == "test"); }
   { gd::strings32 strings1("test"); gd::strings32 strings2(strings1); REQUIRE(strings2.size() == 1); REQUIRE(strings2[0] == "test"); }
   { gd::strings32 strings1("test"); gd::strings32 strings2(std::move(strings1)); REQUIRE(strings2.size() == 1); REQUIRE(strings2[0] == "test"); }
   { gd::strings32 strings1("test"); gd::strings32 strings2; strings2 = strings1; REQUIRE(strings2.size() == 1); REQUIRE(strings2[0] == "test"); }
   { gd::strings32 strings1("test"); gd::strings32 strings2; strings2 = std::move(strings1); REQUIRE(strings2.size() == 1); REQUIRE(strings2[0] == "test"); }
}

TEST_CASE("strings32 Methods", "[GD]") {
   std::cout << "check `gd::strings32` methods" << std::endl;

   { gd::strings32 strings; strings.append("one"); REQUIRE(strings.size() == 1); REQUIRE(strings[0] == "one"); }
   { gd::strings32 strings; strings.append({"one", "two"}); REQUIRE(strings.size() == 2); REQUIRE(strings[0] == "one"); REQUIRE(strings[1] == "two"); }
   { gd::strings32 strings; strings.append(std::vector<std::string>{"one", "two"}); REQUIRE(strings.size() == 2); REQUIRE(strings[0] == "one"); REQUIRE(strings[1] == "two"); }
   { gd::strings32 strings; strings.append(std::vector<std::string_view>{"one", "two"}); REQUIRE(strings.size() == 2); REQUIRE(strings[0] == "one"); REQUIRE(strings[1] == "two"); }
   { gd::strings32 strings; strings << "one" << "two"; REQUIRE(strings.size() == 2); REQUIRE(strings[0] == "one"); REQUIRE(strings[1] == "two"); }
   { gd::strings32 strings; strings.add("one", "two"); REQUIRE(strings.size() == 2); REQUIRE(strings[0] == "one"); REQUIRE(strings[1] == "two"); }
   { gd::strings32 strings; strings.append_any(gd::variant_view("one")); REQUIRE(strings.size() == 1); REQUIRE(strings[0] == "one"); }
   { gd::strings32 strings; strings.append_any(std::vector<gd::variant_view>{{"one"}, {"two"}}); REQUIRE(strings.size() == 2); REQUIRE(strings[0] == "one"); REQUIRE(strings[1] == "two"); }
   { gd::strings32 strings; strings.append("one"); strings.erase(strings.begin()); REQUIRE(strings.size() == 0); }
   { gd::strings32 strings; strings.append("one"); strings.replace(strings.begin(), "two"); REQUIRE(strings[0] == "two"); }
   { gd::strings32 strings; strings.append({"one", "two"}); std::string result = strings.join(", "); REQUIRE(result == "one, two"); }
   { gd::strings32 strings; strings.append({"one", "two"}); auto it = strings.begin(); REQUIRE(*it == "one"); ++it; REQUIRE(*it == "two"); }
}


// ## Tests for gd::file::path