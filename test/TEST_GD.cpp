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

   for ( unsigned u = 10; u < 20; u++ ) { strings32_test(u, 10); }
}

TEST_CASE("Strings Constructors and Assignment", "[strings]") {
   using namespace gd::pointer;
   { strings str; REQUIRE(str.m_vectorText.empty()); }
   { strings str(gd::types::tag_owner{}); str.append("test"); REQUIRE(str.m_vectorText.size() == 1); REQUIRE(std::strcmp(str.m_vectorText[0], "test") == 0); }
   { strings str; const char* p = "test"; str.append(p); REQUIRE(str.m_vectorText.size() == 1); REQUIRE(std::strcmp(str.m_vectorText[0], "test") == 0); }
   { strings str1(gd::types::tag_owner{}); str1.append("test"); strings str2(str1); REQUIRE(str2.m_vectorText.size() == 1); REQUIRE(std::strcmp(str2.m_vectorText[0], "test") == 0); }
   { strings str1(gd::types::tag_owner{}); str1.append("test"); strings str2(std::move(str1)); REQUIRE(str2.m_vectorText.size() == 1); REQUIRE(std::strcmp(str2.m_vectorText[0], "test") == 0); }
   { strings str1(gd::types::tag_owner{}); str1.append("test"); strings str2; str2 = str1; REQUIRE(str2.m_vectorText.size() == 1); REQUIRE(std::strcmp(str2.m_vectorText[0], "test") == 0); }
   { strings str1(gd::types::tag_owner{}); str1.append("test"); strings str2; str2 = std::move(str1); REQUIRE(str2.m_vectorText.size() == 1); REQUIRE(std::strcmp(str2.m_vectorText[0], "test") == 0); }
}

TEST_CASE("Strings Methods", "[strings]") {
   using namespace gd::pointer;
   { strings str; const char* p = "one"; str.append(p); REQUIRE(str.m_vectorText.size() == 1); REQUIRE(std::strcmp(str.m_vectorText[0], "one") == 0); }
   { strings str(gd::types::tag_owner{}); str.append("one"); str.append("two"); REQUIRE(str.m_vectorText.size() == 2); REQUIRE(std::strcmp(str.m_vectorText[0], "one") == 0); REQUIRE(std::strcmp(str.m_vectorText[1], "two") == 0); }
   { strings str; str.append("one"); REQUIRE(str.exists("one")); REQUIRE_FALSE(str.exists("two")); }
   { strings str(gd::types::tag_owner{}); str.append("one"); str.append("two"); std::vector<const char*> vec; str.clone_s(str.m_vectorText, vec); REQUIRE(vec.size() == 2); REQUIRE(std::strcmp(vec[0], "one") == 0); REQUIRE(std::strcmp(vec[1], "two") == 0); }
   { strings str; const char* list[] = { "one", "two" }; std::vector<const char*> vec; str.clone_s(list, 2, vec); REQUIRE(vec.size() == 2); REQUIRE(std::strcmp(vec[0], "one") == 0); REQUIRE(std::strcmp(vec[1], "two") == 0); }
}

TEST_CASE("ViewStrings Tests", "[view::strings]") {
   using namespace gd::view;
   { strings s; REQUIRE(s.empty()); REQUIRE(s.size() == 0); }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s(vec); REQUIRE_FALSE(s.empty()); REQUIRE(s.size() == 3); REQUIRE(s[0] == "one"); REQUIRE(s[1] == "two"); REQUIRE(s[2] == "three"); }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s(std::move(vec)); REQUIRE_FALSE(s.empty()); REQUIRE(s.size() == 3); REQUIRE(s[0] == "one"); REQUIRE(s[1] == "two"); REQUIRE(s[2] == "three"); }
   { const char* arr[] = {"one", "two", "three"}; strings s(arr, 3); REQUIRE_FALSE(s.empty()); REQUIRE(s.size() == 3); REQUIRE(s[0] == "one"); REQUIRE(s[1] == "two"); REQUIRE(s[2] == "three"); }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s1(vec); strings s2(s1); REQUIRE(s1.size() == s2.size()); for (size_t i = 0; i < s1.size(); ++i) { REQUIRE(s1[i] == s2[i]); } }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s1(vec); strings s2 = std::move(s1); REQUIRE_FALSE(s2.empty()); REQUIRE(s2.size() == 3); REQUIRE(s2[0] == "one"); REQUIRE(s2[1] == "two"); REQUIRE(s2[2] == "three"); }
   { strings s; s.append("one"); REQUIRE(s.size() == 1); REQUIRE(s[0] == "one"); }
   { strings s1; s1.append("one"); strings s2; s2.append("two"); s2.append(s1); REQUIRE(s2.size() == 2); REQUIRE(s2[0] == "two"); REQUIRE(s2[1] == "one"); }
   { std::vector<std::string_view> vec = {"one", "two"}; strings s; s.append(vec); REQUIRE(s.size() == 2); REQUIRE(s[0] == "one"); REQUIRE(s[1] == "two"); }
   { std::vector<std::string> vec = {"one", "two"}; strings s; s.append(vec); REQUIRE(s.size() == 2); REQUIRE(s[0] == "one"); REQUIRE(s[1] == "two"); }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s(vec); REQUIRE(s.exists("one")); REQUIRE_FALSE(s.exists("four")); }
   { strings s; s += "one"; REQUIRE(s.size() == 1); REQUIRE(s[0] == "one"); }
   { strings s1; s1 += "one"; strings s2; s2 += "two"; s2 += s1; REQUIRE(s2.size() == 2); REQUIRE(s2[0] == "two"); REQUIRE(s2[1] == "one"); }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s(vec); REQUIRE(s.get_string_view(0) == "one"); REQUIRE(s.get_string_view(1) == "two"); REQUIRE(s.get_string_view(2) == "three"); }
   { std::vector<std::string_view> vec = {"one", "two", "three"}; strings s(vec); REQUIRE(s.get_string(0) == "one"); REQUIRE(s.get_string(1) == "two"); REQUIRE(s.get_string(2) == "three"); }
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
   { gd::file::path p("test/path/file.txt"); REQUIRE(p.stem().string() == "file"); }
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
}


TEST_CASE("Move namespace UTF-8 operations") {
   using namespace gd::utf8;
   const uint8_t* puszTextString = reinterpret_cast<const uint8_t*>("Hello\tWorld\nTest  End");

   REQUIRE(move::next(puszTextString) == puszTextString + 1);  // Basic next char
   REQUIRE(move::next(puszTextString, {2}) == puszTextString + 2);  // Next with count
   REQUIRE(move::previous(move::next(puszTextString)) == puszTextString);  // Previous after next
   REQUIRE(move::previous(puszTextString + 1, {1}) == puszTextString);  // Previous with count from start doesn't move
   REQUIRE(move::advance(puszTextString, {1}) == puszTextString + 1);  // Positive advance
   REQUIRE(move::advance(puszTextString + 1, {-1}) == puszTextString);  // Negative advance from start
   REQUIRE(move::end(puszTextString) == puszTextString + 21);  // End of string (assuming null-terminated)
   REQUIRE(move::next_space(puszTextString + 4) == puszTextString + 5);  // Find tab after "Hello"
   REQUIRE(move::next_non_space(puszTextString + 5) == puszTextString + 6);  // Find 'W' after tab
   REQUIRE(move::find(puszTextString, 'W') == puszTextString + 6);  // Find 'W'
   REQUIRE(move::find(puszTextString, puszTextString + 5, 'W') == nullptr);  // 'W' not in first 5 chars
   REQUIRE(move::find_character(puszTextString, reinterpret_cast<const uint8_t*>("World"), {5}) == puszTextString + 6);  // Find "World"
   REQUIRE(std::string(move::find("Hello World", 'W')) == "World");  // string_view find
   REQUIRE(move::find_nth(puszTextString, {1}, 'l') == puszTextString + 3);  // Second 'l'
   REQUIRE(move::find_nth(puszTextString, puszTextString + 5, {0}, 'l') == puszTextString + 2);  // First 'l' in range
   REQUIRE(std::string(move::find_nth("Hello World", {1}, 'l')) == "lo World");  // Second 'l' in string_view
}

