// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <array>
#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"
#include "gd/gd_uuid.h"

#include "gd/gd_sql_query.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[sql] simple select", "[sql]" ) {
   using namespace gd::sql;
   query querySelect;

   querySelect.table_add( "test_table" );
   querySelect.field_add( {{"name", "id"}, {"alias", "key"}}, tag_arguments{} );
   querySelect.field_add( "name" );

   auto stringSQL = querySelect.sql_get( eSqlSelect );
   std::cout << stringSQL << "\n";
   querySelect.clear();

   querySelect.table_add( "test_table1" );
   querySelect.field_add( {{"name", "id"}, {"alias", "key"}}, tag_arguments{} );
   querySelect.field_add( "name" );

   stringSQL = querySelect.sql_get( eSqlSelect );
   std::cout << stringSQL << "\n";

   querySelect.clear();
   querySelect.table_add( "table1" );
   querySelect.field_add( {{"name", "id"}, {"alias", "key"}}, tag_arguments{} );
   querySelect.field_add( "name" );
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorTypeNumberEqual}, {"value", 123} }, tag_arguments{} );

   stringSQL = querySelect.sql_get( eSqlSelect );
   std::cout << stringSQL << "\n";

   querySelect.clear();
   querySelect.table_add( "table1" );
   querySelect.field_add( {{"name", "id"}, {"alias", "key"}}, tag_arguments{} );
   querySelect.field_add( "name" );
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorTypeNumberEqual}, {"value", 123} }, tag_arguments{} );
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorTypeNumberEqual}, {"value", 456} }, tag_arguments{} );
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorTypeNumberEqual}, {"value", 789} }, tag_arguments{} );

   stringSQL = querySelect.sql_get( eSqlSelect );
   std::cout << stringSQL << "\n";


}
