// $TAG #play #ini #xml #history

#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"
#include "gd/parse/gd_parse_formats.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[table] custom columns", "[strstr]") 
{
   using namespace gd::parse;

   std::string stringText = "This is a test string with key3 some patterns to find: [key1: `value1 key3`] [key2: \"value2 key3\"] [key3: value3]";
   const char* begin = stringText.c_str();
   const char* piEnd = begin + stringText.size();
   const char* find = "key3";
   unsigned uLength = (unsigned)strlen(find);
   
   const code codeParser('[', ']');
   const char* piResult = strstr(begin, piEnd, find, uLength, codeParser, true);
   
   REQUIRE(piResult != nullptr);
   REQUIRE(std::string(piResult, uLength) == find);

   if( piResult != nullptr )
   {
      piResult += uLength; // Move past the found key
      auto result_ = codeParser.read_value(piResult, piEnd);
      std::string_view stringValue(result_.first, result_.second);
      std::cout << "Found value: " << stringValue << std::endl;

      auto result1_ = codeParser.read_value( std::string_view( piResult, piEnd ) );
      std::cout << "Read value: " << result1_ << std::endl;
   }
}