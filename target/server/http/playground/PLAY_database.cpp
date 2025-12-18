// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_database_sqlite.h"
#include "gd/database/gd_database_io.h"

#include "gd/parse/gd_parse_uri.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[uri] sqlite", "[uri]" )
{
      gd::database::sqlite::database databaseSqlite;
   
      std::string stringDatabaseFile = "test_database.sqlite";
      if( std::filesystem::exists( stringDatabaseFile ) ) { std::filesystem::remove( stringDatabaseFile ); }

      std::string stringRoot = FOLDER_GetRoot_g("test/ignore-files");
      std::cout << "Root folder: " << stringRoot << std::endl;

      std::string stringDatabasePath = gd::file::path( stringRoot ).add( stringDatabaseFile ).string();
      std::cout << "Database: " << stringDatabasePath << std::endl;
      if( std::filesystem::exists( stringDatabaseFile ) ) { std::filesystem::remove( stringDatabaseFile ); }

      auto result_ = databaseSqlite.open( stringDatabaseFile, {"create", "write"});                REQUIRE(result_.first == true);

      result_ = databaseSqlite.execute( "CREATE TABLE TTest (TestK INTEGER PRIMARY KEY, FName TEXT, FAge INTEGER);" );REQUIRE( result_.first == true );

      databaseSqlite.execute( "INSERT INTO TTest (FName, FAge) VALUES ('Alice', 30), ('Bob', 25), ('Charlie', 35);" ); REQUIRE( result_.first == true );

      gd::database::sqlite::cursor cursor_( &databaseSqlite );
      result_ = cursor_.open( "SELECT TestK, FName, FAge FROM TTest;" );                           REQUIRE( result_.first == true );

      gd::database::sqlite::cursor_i pcursor_i;

      pcursor_i.attach( &cursor_ );

      gd::table::dto::table tableResult;
      gd::database::to_table( &pcursor_i, &tableResult);

      std::string stringTable = gd::table::to_string( tableResult, gd::table::tag_io_cli{});
      std::cout << "Table result:\n" << stringTable << std::endl;

      pcursor_i.detach();

      databaseSqlite.close();


   /*
      auto [ successOpen, errorOpen ] = databaseSqlite.open( stringDatabaseFile );
      REQUIRE( successOpen == true );
   
      auto [ successExec, errorExec ] = databaseSqlite.execute( "CREATE TABLE test_table (id INTEGER PRIMARY KEY, name TEXT, age INTEGER);" );
      REQUIRE( successExec == true );
   
      auto [ successInsert, errorInsert ] = databaseSqlite.execute( "INSERT INTO test_table (name, age) VALUES ('Alice', 30), ('Bob', 25), ('Charlie', 35);" );
      REQUIRE( successInsert == true );
   
      auto [ successQuery, errorQuery, results ] = databaseSqlite.query( "SELECT id, name, age FROM test_table;" );
      REQUIRE( successQuery == true );
   
      for( const auto& row : results )
      {
         int id = row.at( "id" ).as_int();
         std::string name = row.at( "name" ).as_string();
         int age = row.at( "age" ).as_int();
   
         std::cout << "ID: " << id << ", Name: " << name << ", Age: " << age << std::endl;
      }
   
      databaseSqlite.close();
      */
}