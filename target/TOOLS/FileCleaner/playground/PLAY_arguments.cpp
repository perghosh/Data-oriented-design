// @FILE [tag: arguments, playground] [description: code used to manage arguments and arg] [type: playground]

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_variant_arg.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[arguments] test arg to add values", "[arguments]") 
{
   gd::argument::arguments argumentsTest;
   argumentsTest.append( "key1", "value1" );
   argumentsTest.append( "key2", 42 ); 

   gd::arg_view test( "key", 3.14 );

   argumentsTest += gd::arg_view("key2","tttt");
   argumentsTest = gd::arg_view("key2","tttt");

   argumentsTest << gd::arg_view( "key3", true ) << gd::arg_view( "key4", 12345u );

   gd::arg_view test4( "key4" );
   argumentsTest >> test4;

}

