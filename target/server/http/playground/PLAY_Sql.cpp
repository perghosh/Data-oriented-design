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
   querySelect.field_add( {{"name", "test"}, {"alias", "alias_for_test"}, {"order", 1}}, tag_arguments{} );
   querySelect.field_add( "name" );
   querySelect.set_limit( 10, 10 );

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

TEST_CASE( "[sql] update with types", "[sql]" ) {

/*
1. `3f7c9b1a8d4e6f2a5c8b7d1e9f3a4c6d`
2. `a1b2c3d4e5f67890123456789abcdef0`
3. `7d8e9f0a1b2c3d4e5f67890123456789`
4. `f0e1d2c3b4a5968778695a4b3c2d1e0f`
5. `1a2b3c4d5e6f7890abcdef1234567890`
*/

   using namespace gd::sql;
   query queryUpdate( eSqlDialectSqlite );

   queryUpdate.table_add( "table1" );
   queryUpdate.field_add( {{"name", "id"}, {"value", "id-value"}, {"type", "utf8"}}, tag_arguments{} );
   queryUpdate.field_add( {{"name", "uuid"}, {"value", "3f7c9b1a8d4e6f2a5c8b7d1e9f3a4c6d"}, {"type", "uuid"}}, tag_arguments{} );
   queryUpdate.condition_add( { {"name", "uuid"}, {"operator", eOperatorTypeNumberEqual}, {"value", "1a2b3c4d5e6f7890abcdef1234567890"}, {"type", "uuid"} }, tag_arguments{} );

   auto stringSQL = queryUpdate.sql_get( eSqlUpdate );
   std::cout << stringSQL << "\n";
   queryUpdate.sql_set_dialect( eSqlDialectSqlServer );
   stringSQL = queryUpdate.sql_get( eSqlUpdate );
   std::cout << stringSQL << "\n";
   queryUpdate.sql_set_dialect( eSqlDialectOracle );
   stringSQL = queryUpdate.sql_get( eSqlUpdate );
   std::cout << stringSQL << "\n";

   stringSQL = queryUpdate.sql_get( eSqlInsert );
   std::cout << stringSQL << "\n";

}


TEST_CASE( "[sql] update", "[sql]" ) {
   using namespace gd::sql;
   query queryUpdate;

   queryUpdate.table_add( "table1" );
   queryUpdate.field_add( {{"name", "id"}, {"value", "id-value"}}, tag_arguments{} );
   queryUpdate.field_add( {{"name", "name"}, {"value", "name-value"}}, tag_arguments{} );
   queryUpdate.condition_add( { {"name", "id"}, {"operator", eOperatorTypeNumberEqual}, {"value", 123} }, tag_arguments{} );

   auto stringSQL = queryUpdate.sql_get( eSqlUpdate );
   std::cout << stringSQL << "\n";

   query queryDelete;
   queryDelete.table_add( "table1" );
   queryDelete.condition_add( { {"name", "id"}, {"operator", eOperatorTypeNumberEqual}, {"value", 123} }, tag_arguments{} );

   stringSQL = queryDelete.sql_get( eSqlDelete );
   std::cout << stringSQL << "\n";

}
