#include <random>

#include "gd/gd_utf8.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[fake_data] 01", "[fake_data]" ) {
   using namespace gd::table;

   std::string stringAlphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
   std::random_device randomdevice;
   std::mt19937 mt19937Alphabet(randomdevice());
   std::uniform_int_distribution<> UIDAlphabet(0, (unsigned)stringAlphabet.size() - 1);
   std::uniform_int_distribution<> UIDCount(5, 40);

   dto::table tableText( "int64,key;double,currency;string,50,namey;string,20,city", tag_parse{}, tag_prepare{});
   for(auto it = 0; it < 1000; it++)
   {
      std::string stringRandom;
      for( unsigned u = 0, uMax = UIDCount(mt19937Alphabet); u < uMax; u++ ) stringRandom += stringAlphabet[UIDAlphabet(mt19937Alphabet)];
      tableText.row_add({{it},{it * 100}, stringRandom, stringRandom.substr(0, stringRandom.length() / 2)}, gd::table::tag_convert{});
   }

   std::string stringPrint = gd::table::to_string( tableText, gd::table::tag_io_cli{});
   std::cout << stringPrint;

   auto vectorValue = tableText.row_get_variant_view( 500 );
   int64_t iRow = tableText.find( vectorValue );                                                   REQUIRE( iRow == 500 );

   vectorValue = tableText.row_get_variant_view( 700 );
   iRow = tableText.find({ { "key", vectorValue[0] }, { "currency", vectorValue[1]} });            REQUIRE(iRow == 700);
   
}