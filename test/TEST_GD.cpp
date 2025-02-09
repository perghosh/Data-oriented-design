#include <chrono>
#include <span>
#include <random>
#include <ranges>
#include <list>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_file.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_variant_common.h"
#include "gd/gd_strings.h"



#include "main.h"

#include "catch2/catch_amalgamated.hpp"

// ----------------------------------------------------------------------------
// // ## Tests for gd::strings32

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

/// Test the `strings32` class with random strings of varying lengths and operations on them.
void strings32_test(size_t uStringCount, size_t uMaxStringLength)
{
   std::random_device rd;
   std::mt19937 mt19937Generate(rd());
   std::uniform_int_distribution<> UIDPrintChar(32, 126); // Printable ASCII characters
   std::uniform_int_distribution<> UIDStringLength(1, int(uMaxStringLength));
   std::uniform_int_distribution<> UIDIndex(0, int(uStringCount) - 1);

   gd::strings32 strings32Container;

   // Generate and append random strings
   for (size_t i = 0; i < uStringCount; ++i)
   {
      size_t uLength = UIDStringLength(mt19937Generate);
      std::string stringRandom;
      stringRandom.reserve(uLength);

      for (size_t j = 0; j < uLength; ++j) { stringRandom += static_cast<char>(UIDPrintChar(mt19937Generate)); }
      strings32Container.append(stringRandom);
   }
                                                                                                   REQUIRE(strings32Container.size() == uStringCount);

   // Replace a random string
   size_t uReplaceIndex = UIDIndex(mt19937Generate);
   std::string stringReplacement = "REPLACED_STRING";
   auto itReplace = strings32Container.begin() + uReplaceIndex;
   strings32Container.replace(itReplace, stringReplacement);                                       REQUIRE(strings32Container.size() == uStringCount);
                                                                                                   REQUIRE(itReplace.as_string_view() == stringReplacement);
   // Remove a random string
   size_t uRemoveIndex = UIDIndex(mt19937Generate);
   auto itRemove = strings32Container.begin() + uRemoveIndex;
   strings32Container.erase(itRemove);                                                             REQUIRE(strings32Container.size() == uStringCount - 1);
}

// ----------------------------------------------------------------------------
// ## Tests for gd::file::path

TEST_CASE("Path Constructors and Assignment", "[path]") {
   { gd::file::path p; REQUIRE(p.empty()); }
   { gd::file::path p("test/path"); REQUIRE(p == "test/path"); }
   { std::string_view sv = "test/path"; gd::file::path p(sv); REQUIRE(p == "test/path"); }
   { std::string s = "test/path"; gd::file::path p(s); REQUIRE(p == "test/path"); }
   { std::string s = "test/path"; gd::file::path p(std::move(s)); REQUIRE(p == "test/path"); }
   { gd::file::path p1("test/path"); gd::file::path p2(p1); REQUIRE(p2 == "test/path"); }
   { gd::file::path p1("test/path"); gd::file::path p2(std::move(p1)); REQUIRE(p2 == "test/path"); }
   { gd::file::path p1("test/path"); gd::file::path p2; p2 = p1; REQUIRE(p2 == "test/path"); }
   { gd::file::path p1("test/path"); gd::file::path p2; p2 = std::move(p1); REQUIRE(p2 == "test/path"); }
}

TEST_CASE("Path Methods", "[path]") {
   { gd::file::path p("test/path/file.txt"); REQUIRE(p.has_filename()); }
   { gd::file::path p("test/path/"); REQUIRE(p.has_separator()); }
   { gd::file::path p("/test/path"); REQUIRE(p.has_begin_separator()); }
   { gd::file::path p("test/path/file.txt"); REQUIRE(p.filename().string() == "file.txt"); }
   { gd::file::path p("test/path/file.txt"); REQUIRE(p.extension().string() == ".txt"); }
   { gd::file::path p("test"); p.add("path"); REQUIRE(p == "test/path"); }
   { gd::file::path p("test"); p.add({"path", "to", "file"}); REQUIRE(p == "test/path/to/file"); }
   { gd::file::path p("test"); std::vector<std::string_view> vec = {"path", "to", "file"}; p.add(vec); REQUIRE(p == "test/path/to/file"); }
   { gd::file::path p1("test"); gd::file::path p2("path"); gd::file::path p3 = p1 / p2; REQUIRE(p3 == "test/path"); }
   { gd::file::path p1("test"); gd::file::path p2 = p1 / "path"; REQUIRE(p2 == "test/path"); }
   { gd::file::path p("test/path"); p.erase_end(); REQUIRE(p == "test"); }
   { gd::file::path p("test/path/file.txt"); p.remove_filename(); REQUIRE(p == "test/path/"); }
   { gd::file::path p("test/path/file.txt"); p.replace_filename("newfile.txt"); REQUIRE(p == "test/path/newfile.txt"); }
   { gd::file::path p("test/path/file.txt"); p.replace_extension(".md"); REQUIRE(p == "test/path/file.md"); }
   { gd::file::path p("test/path"); p.clear(); REQUIRE(p.empty()); }
   { gd::file::path p("test/path"); std::string result; for (auto it = p.begin(); it != p.end(); ++it) { result += *it; } REQUIRE(gd::file::path(result) == "test/path"); }

   for ( unsigned u = 10; u < 20; u++ ) { strings32_test(u, 10); }
}
