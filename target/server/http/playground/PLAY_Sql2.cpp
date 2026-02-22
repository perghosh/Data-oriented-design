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
#include "gd/gd_sql_query_builder.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[sql] select", "[sql]" )
{
   using namespace gd::sql;
   query querySelect;

   querySelect.table_add("test_table");
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
   
   std::cout << "----------------------------------" << std::endl;

   std::array<char, 128> buffer_;
   query query01("table-name", tag_table{});
   query01 << field_g("name", buffer_).as("alias") << field_g("name", buffer_).as("alias").orderby();

   std::string stringSQL2 = query01.sql_get( eSqlSelect );
   std::cout << stringSQL2 << "\n";

   std::cout << "----------------------------------" << std::endl;

   std::array<char, 128> buffer2_;
   std::array<char, 128> buffer3_;
   query query02("TPoll", tag_table{});
   query02 << field_g("FName").value("Test1");

   stringSQL2 = query02.sql_get( eSqlInsert );
   std::cout << stringSQL2 << "\n";
}