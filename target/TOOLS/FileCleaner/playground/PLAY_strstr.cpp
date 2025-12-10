// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

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

   std::string stringText = R"(This is a test string with key3 some patterns to find: [key1: `value1 key3`] [key2: \"value2 key33\"] [key3: value3] [key4: `````1234567890`````] )";
   const char* begin = stringText.c_str();
   const char* piEnd = begin + stringText.size();
   const char* piFindKey3 = "key3";
   unsigned uLength = (unsigned)strlen(piFindKey3);
   
   const code codeParser('[', ']');
   const char* piResult = strstr(begin, piEnd, piFindKey3, uLength, codeParser, true);
   
   REQUIRE(piResult != nullptr);
   REQUIRE(std::string(piResult, uLength) == piFindKey3);

   if( piResult != nullptr )
   {
      piResult += uLength; // Move past the found key
      auto result_ = codeParser.read_value(piResult, piEnd);
      std::string_view stringValue(result_.first, result_.second);
      std::cout << "Found value: " << stringValue << std::endl;

      auto result1_ = codeParser.read_value( std::string_view( piResult, piEnd ) );
      std::cout << "Read value: " << result1_ << std::endl;
   }

   const char* piFindKey2 = "key2";
   unsigned uLength2 = (unsigned)strlen(piFindKey2);
   const char* piResult2 = strstr(begin, piEnd, piFindKey2, uLength2, codeParser, true);

   REQUIRE(piResult2 != nullptr);
   REQUIRE(std::string(piResult2, uLength2) == piFindKey2);

   if( piResult2 != nullptr )
   {
      piResult2 += uLength2;
      auto result2_ = codeParser.read_value(piResult2, piEnd);
      std::string_view stringValue2(result2_.first, result2_.second);
      std::cout << "Found value for key2: " << stringValue2 << std::endl;
   }

   const char* piFindKey4 = "key4";
   unsigned uLength4 = (unsigned)strlen(piFindKey4);
   const char* piResult4 = strstr(begin, piEnd, piFindKey4, uLength4, codeParser, true);
   REQUIRE(piResult4 != nullptr);
   REQUIRE(std::string(piResult4, uLength4) == piFindKey4);

   if( piResult4 != nullptr )
   {
      auto result4_ = codeParser.read_value(piResult4, piEnd);
      std::string_view stringValue4(result4_.first, result4_.second);
      std::cout << "Found value for key4: " << stringValue4 << std::endl;
   }
}