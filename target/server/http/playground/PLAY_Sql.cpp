// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <array>
#include <filesystem>

#include "gd/gd_arena.h"
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

TEST_CASE( "[sql] builder 1", "[sql]" ) {
   using namespace gd::sql;

   {
      std::array<char, 128> buffer_;
      query query01;
      query01 << table_g( "table_name" )
              << table_g( "table_name_02" ).parent( "table_name" ).join( "table_name.id = table_name_02.table_name_id" );
      query01 << field_g("name", buffer_).as("alias");
      query01 << field_g("table_name_02", "name", buffer_).as("alias02");
      query01 << field_g("name", buffer_).as("alias").orderby();

      std::string stringSQL = query01.sql_get( eSqlSelect );
      std::cout << stringSQL << "\n";
   }


   {
      gd::memory::arena<> arena_( 1024 );
      
      query query01;
      auto buffer1 = arena_.allocate_span<std::byte>(128u);
      auto buffer2 = arena_.allocate_span<std::byte>(128u);
      auto buffer3 = arena_.allocate_span<std::byte>(128u);
      
      query01 << table_g( "table_name" )
              << table_g( "table_name_02" ).parent( "table_name" ).join( "table_name.id = table_name_02.table_name_id" )
              << field_g("name", buffer1).as("alias")
              << field_g("table_name_02", "name", buffer2).as("alias02")
              << field_g("name", buffer3).as("alias").orderby();

      std::string stringSQL = query01.sql_get( eSqlSelect );
      std::cout << stringSQL << "\n";
   }

   {
      gd::memory::arena<> arena_( 1024 );
      
      query query01;
      
      query01 << table_g( "table_name" )
              << table_g( "table_name_02" ).parent( "table_name" ).join( "table_name.id = table_name_02.table_name_id" )
              << field_g("name", arena_.allocate( 128 ), 128 ).as("alias")
              << field_g("table_name_02", "name", arena_.allocate( 128 ), 128 ).as("alias02")
              << field_g("name", arena_.allocate( 128 ), 128 ).as("alias").orderby();

      std::string stringSQL = query01.sql_get( eSqlSelect );
      std::cout << stringSQL << "\n";
   }


   {
      query query01;
      query01 << table_g( "table_name" )
              << table_g( "table_name_02" ).parent( "table_name" ).join( "table_name.id = table_name_02.table_name_id" )
              << field_g("name").as("alias")
              << field_g("table_name_02", "name").as("alias02")
              << field_g("name").as("alias").orderby();

      std::string stringSQL = query01.sql_get( eSqlSelect );
      std::cout << stringSQL << "\n";
   }

}

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
   querySelect.field_add_parttype( "orderby", {{"name", "name"}}, tag_arguments{});

   stringSQL = querySelect.sql_get( eSqlSelect );
   std::cout << stringSQL << "\n";

   querySelect.clear();
   querySelect.table_add( "table1" );
   querySelect.field_add( {{"name", "id"}, {"alias", "key"}}, tag_arguments{} );
   querySelect.field_add_parttype( "orderby", {{"name", "name"}}, tag_arguments{});
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorEqual}, {"value", 123} }, tag_arguments{} );

   stringSQL = querySelect.sql_get( eSqlSelect );
   std::cout << stringSQL << "\n";

   querySelect.clear();
   querySelect.table_add( "table1" );
   querySelect.field_add( {{"name", "id"}, {"alias", "key"}}, tag_arguments{} );
   querySelect.field_add( "name" );
   querySelect.field_add_parttype( eSqlPartSelect|eSqlPartOrderBy, {{"name", "city"}, {"alias", "stad"}}, tag_arguments{} );
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorEqual}, {"value", 123} }, tag_arguments{} );
   querySelect.condition_add( { {"name", "id"}, {"operator", eOperatorEqual}, {"value", 456} }, tag_arguments{} );
   querySelect.condition_add( { {"name", "id"}, {"operator", "="}, {"value", 789}}, tag_arguments{});

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
   queryUpdate.condition_add( { {"name", "uuid"}, {"operator", "="}, {"value", "1a2b3c4d5e6f7890abcdef1234567890"}, {"type", "uuid"} }, tag_arguments{} );

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

TEST_CASE( "[sql] table_builder", "[sql]" ) {
   using namespace gd::sql;

   // Basic table
   { query q; q << table_g("users"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Table with alias
   { query q; q << table_g("users").as("u"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Table with schema
   { query q; q << table_g("users").schema("public"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Table with parent
   { query q; q << table_g("orders").parent("users"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Table with join
   { query q; q << table_g("orders").join("LEFT JOIN users ON orders.user_id = users.id"); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Table with key/fk
   { query q; q << table_g("orders").key("id").fk("user_id"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Table with owner
   { query q; q << table_g("users").owner("admin"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Multiple tables
   { query q; q << table_g("users") << table_g("orders").join("JOIN users ON orders.user_id = users.id");
     std::cout << q.sql_get(eSqlSelect) << "\n"; }
}

TEST_CASE( "[sql] field_builder", "[sql]" ) {
   using namespace gd::sql;

   // Basic field
   { query q; q << table_g("users") << field_g("name").select(); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Field with alias
   { query q; q << table_g("users") << field_g("name").as("user_name").select(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Field with value
   { query q; q << table_g("users") << field_g("id").value(123).insert(); 
     std::cout << q.sql_get(eSqlInsert) << "\n"; }

   // Field with type
   { query q; q << table_g("users") << field_g("id").type("int32").select(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Field with raw
   { query q; q << table_g("users") << field_g("created_at").raw("NOW()").select(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Add multiple fields 
   { query q; q << table_g("users") << fields_g("users", "name", "age", "email", "created_at", "updated_at").select();
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Add multiple fields with aliases
   { query q; q << table_g("users") << fields_g("users", {{"name","alias_name"}, {"age","alias_age"}}).select();
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Add multiple fields with aliases using vector
   { std::vector<std::pair<std::string_view, std::string_view>> fields_ = { {"name","alias_name"}, {"age","alias_age"} };
     query q; q << table_g("users") << fields_g("users", fields_).select();
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Add multiple fields with values
   { std::vector<std::pair<std::string_view, gd::variant_view>> fields_ = { {"name","alias_name"}, {"age","alias_age"} };
     query q; q << table_g("users") << fields_g("users", fields_).insert();
     std::cout << q.get_insert() << "\n"; }

   // Add multiple fields with values
   { std::vector<std::pair<std::string_view, gd::variant_view>> fields_ = { {"id","f47ac10b58cc4372a5670e02b2c3d479"}, {"id2","1e3f7b8a9d2c4f6e8b1a5c7d9e3f2a1b"} };
     query q; q << table_g("users") << fields_g("users", fields_).insert().type("string"); // hex values are treated as string
     std::cout << q.get_insert() << "\n"; 
     query q2; q2 << table_g("users") << fields_g("users", fields_).insert().type("binary"); // hex values are treated as binary
     std::cout << q2.get_insert() << "\n"; 
     query q3; q3 << table_g("users") << fields_g("users", fields_).insert().type("int32"); // hex values are treated as number (no quoting or hex)
     std::cout << q3.get_insert() << "\n"; 
   }

   // Field with orderby
   { query q; q << table_g("users") << field_g("name").select() << field_g("age").orderby(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Field with groupby
   { query q; q << table_g("users") << field_g("age").groupby() << field_g("COUNT(*)").select(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Field with table qualification
   { query q; q << table_g("u", "users") << field_g("u", "name").select(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Multiple fields
   { query q; q << table_g("users") << field_g("id").select() << field_g("name").select() 
     << field_g("email").select(); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Update with multiple fields
   { query q; q << table_g("users") << field_g("name").value("John").update() 
     << field_g("email").value("john@test.com").update() 
     << condition_g("id").value(1).eq(); std::cout << q.sql_get(eSqlUpdate) << "\n"; }
}

TEST_CASE( "[sql] condition_builder", "[sql]" ) {
   using namespace gd::sql;

   // Basic equality
   { query q; q << table_g("users") << field_g("*").select() << condition_g("id").value(1).eq(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Not equal
   { query q; q << table_g("users") << field_g("*").select() << condition_g("id").value(1).ne(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Less than
   { query q; q << table_g("users") << field_g("*").select() << condition_g("age").value(18).lt(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Less than or equal
   { query q; q << table_g("users") << field_g("*").select() << condition_g("age").value(18).le(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Greater than
   { query q; q << table_g("users") << field_g("*").select() << condition_g("age").value(18).gt(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Greater than or equal
   { query q; q << table_g("users") << field_g("*").select() << condition_g("age").value(18).ge(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // LIKE
   { query q; q << table_g("users") << field_g("*").select() 
     << condition_g("email").value("%@test.com").like(); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // IN
   { query q; q << table_g("users") << field_g("*").select() 
     << condition_g("status").value("'active','pending'").in(); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // IS NULL
   { query q; q << table_g("users") << field_g("*").select() << condition_g("deleted_at").is_null(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // IS NOT NULL
   { query q; q << table_g("users") << field_g("*").select() << condition_g("deleted_at").is_not_null(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // AND grouping
   { query q; q << table_g("users") << field_g("*").select() 
     << condition_g("name").value("John").eq().and_() << condition_g("age").value(25).gt(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // OR grouping
   { query q; q << table_g("users") << field_g("*").select() 
     << condition_g("name").value("John").eq().or_() << condition_g("name").value("Jane").eq(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // NOT grouping
   { query q; q << table_g("users") << field_g("*").select() << condition_g("active").value(0).eq().not_(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Condition with table qualification
   { query q; q << table_g("u", "users") << field_g("*").select() 
     << condition_g("u", "id").value(1).eq(); std::cout << q.sql_get(eSqlSelect) << "\n"; }
}

TEST_CASE( "[sql] builder combined", "[sql]" ) {
   using namespace gd::sql;

   // Complex SELECT with joins
   { query q; q << table_g("users").as("u") << table_g("orders").as("o")
     .join("LEFT JOIN orders ON u.id = o.user_id") << field_g("u", "name").select() 
     << field_g("o", "amount").select() << field_g("o", "created_at").orderby() 
     << condition_g("u", "active").value(1).eq(); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // INSERT with multiple fields
   { query q; q << table_g("users") << field_g("name").value("John").insert() 
     << field_g("email").value("john@test.com").insert() << field_g("age").value(25).insert(); 
     std::cout << q.sql_get(eSqlInsert) << "\n"; }

   // UPDATE with conditions
   { query q; q << table_g("users") << field_g("name").value("John").update() 
     << field_g("email").value("new@test.com").update() << condition_g("id").value(1).eq(); 
     std::cout << q.sql_get(eSqlUpdate) << "\n"; }

   // DELETE with conditions
   { query q; q << table_g("users") << condition_g("id").value(1).eq() 
     << condition_g("deleted_at").is_null().and_(); std::cout << q.sql_get(eSqlDelete) << "\n"; }

   // GROUP BY with HAVING (using AND for group)
   { query q; q << table_g("users") << field_g("age").groupby() 
     << field_g("COUNT(*)").select() << condition_g("age").value(18).ge().and_(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Multiple tables with different schemas
   { query q; q << table_g("users").schema("public") << table_g("logs").schema("audit")
     .join("LEFT JOIN audit.logs ON users.id = logs.user_id") << field_g("users", "name").select()
     << field_g("logs", "action").select(); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // Complex conditions with OR
   { query q; q << table_g("users") << field_g("*").select() 
     << condition_g("name").value("John").eq().or_() << condition_g("name").value("Jane").eq(); 
     std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // With raw SQL in conditions
   { query q; q << table_g("users") << field_g("*").select() 
     << condition_g("created_at").raw("> '2024-01-01'"); std::cout << q.sql_get(eSqlSelect) << "\n"; }

   // With table key/fk relationships
   { query q; q << table_g("users").key("id") << table_g("orders").key("id").fk("user_id")
     .join("JOIN orders ON users.id = orders.user_id") << field_g("users", "name").select()
     << field_g("orders", "amount").select(); std::cout << q.sql_get(eSqlSelect) << "\n"; }
}

TEST_CASE( "[sql] builder dialects", "[sql]" ) {
   using namespace gd::sql;

   // SQLite
   { query q(eSqlDialectSqlite); q << table_g("users") << field_g("id").value(1).insert();
     std::cout << "SQLite: " << q.sql_get(eSqlInsert) << "\n"; }

   // SQL Server
   { query q(eSqlDialectSqlServer); q << table_g("users") << field_g("id").value(1).insert();
     std::cout << "SQLServer: " << q.sql_get(eSqlInsert) << "\n"; }

   // Oracle
   { query q(eSqlDialectOracle); q << table_g("users") << field_g("id").value(1).insert();
     std::cout << "Oracle: " << q.sql_get(eSqlInsert) << "\n"; }

   // PostgreSQL
   { query q(eSqlDialectPostgreSql); q << table_g("users") << field_g("id").value(1).insert();
     std::cout << "Postgres: " << q.sql_get(eSqlInsert) << "\n"; }

   // MySQL
   { query q(eSqlDialectMySql); q << table_g("users") << field_g("id").value(1).insert();
     std::cout << "MySQL: " << q.sql_get(eSqlInsert) << "\n"; }
}
