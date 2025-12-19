// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#include <psapi.h>
#endif

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


/**
 * @brief [uri] sqlite playground test
 * 
 * Create database, add table and select data from table and print to console.
 */
TEST_CASE( "[database] sqlite select to table", "[database]" )
{
#ifdef _WIN32
   // Enable memory leak detection
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
   _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
   _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
   _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
   
   // Set a checkpoint for memory allocations before database operations
   _CrtMemState memStateBefore;
   _CrtMemCheckpoint(&memStateBefore);
   
   std::cout << "Memory leak detection enabled" << std::endl;
#endif
   {
      std::string stringDatabaseFile = "test_database.sqlite";

      std::string stringRoot = FOLDER_GetRoot_g("test/ignore-files");
      std::cout << "Root folder: " << stringRoot << std::endl;

      std::string stringDatabasePath = gd::file::path( stringRoot ).add( stringDatabaseFile ).string();
      std::cout << "Database: " << stringDatabasePath << std::endl;
      if( std::filesystem::exists( stringDatabaseFile ) ) { std::filesystem::remove( stringDatabaseFile ); }

      gd::database::sqlite::database databaseSqlite;
      auto result_ = databaseSqlite.open( stringDatabaseFile, {"create", "write"});                REQUIRE(result_.first == true);
      result_ = databaseSqlite.execute( "CREATE TABLE TTest (TestK INTEGER PRIMARY KEY, FName TEXT, FAge INTEGER);" );REQUIRE( result_.first == true );
      result_ = databaseSqlite.execute( "INSERT INTO TTest (FName, FAge) VALUES ('Alice', 30), ('Bob', 25), ('Charlie', 35);" ); REQUIRE( result_.first == true );
      {
         gd::database::sqlite::cursor cursor_( &databaseSqlite );

         result_ = cursor_.open( "SELECT TestK, FName, FAge FROM TTest;" );                        REQUIRE( result_.first == true );

         gd::database::sqlite::cursor_i pcursor_i;
         pcursor_i.attach( &cursor_ );

         gd::table::dto::table tableResult;
         gd::database::to_table( &pcursor_i, &tableResult);

         std::string stringTable = gd::table::to_string( tableResult, gd::table::tag_io_cli{});
         std::cout << "Table result:\n" << stringTable << std::endl;

         pcursor_i.detach();
      }

      databaseSqlite.close();
   }

#ifdef _WIN32
   // Check memory state after database operations
   _CrtMemState memStateAfter, memStateDiff;
   _CrtMemCheckpoint(&memStateAfter);
   
   // Compare memory states to detect leaks
   if (_CrtMemDifference(&memStateDiff, &memStateBefore, &memStateAfter)) {
      std::cout << "\nMEMORY LEAKS DETECTED:" << std::endl;
      _CrtMemDumpStatistics(&memStateDiff);
      
      // Output detailed leak information to debug console
      std::cout << "\nDetailed memory leak information:" << std::endl;
      _CrtDumpMemoryLeaks();
   } else {
      std::cout << "\nNo memory leaks detected" << std::endl;
   }
#endif
}

TEST_CASE( "[database] sqlite select to table with callback", "[database]" )
{
   std::string stringDatabaseFile = "test01.sqlite";

   std::string stringRoot = FOLDER_GetRoot_g("test/ignore-files");
   std::cout << "Root folder: " << stringRoot << std::endl;

   std::string stringDatabasePath = gd::file::path( stringRoot ).add( stringDatabaseFile ).string();
   std::cout << "Database: " << stringDatabasePath << std::endl;
   if( std::filesystem::exists( stringDatabaseFile ) ) { std::filesystem::remove( stringDatabaseFile ); }

   gd::database::sqlite::database databaseSqlite;
   auto result_ = databaseSqlite.open( stringDatabaseFile, {"create", "write"});                REQUIRE(result_.first == true);
   result_ = databaseSqlite.execute( "CREATE TABLE TUser (UserK INTEGER PRIMARY KEY, FName TEXT, FAge INTEGER);" );REQUIRE( result_.first == true );

   gd::argument::arguments argumentsInsert;
   result_ = databaseSqlite.execute( "INSERT INTO TUser (FName, FAge) VALUES ('Alice', 30), ('Bob', 25), ('Charlie', 35) RETURNING UserK;", []( const auto* parguments_ ) {
      std::string string_ = gd::argument::debug::print( *parguments_ );
      std::cout << "Insert callback arguments: " << string_ << std::endl;
      return true;
   }); REQUIRE(result_.first == true);

   result_ = databaseSqlite.execute( "CREATE TABLE TUserNew ( UserK BLOB PRIMARY KEY DEFAULT (randomblob(16)), FName TEXT NOT NULL, FAlias TEXT NOT NULL);" );REQUIRE( result_.first == true );
   result_ = databaseSqlite.execute( "INSERT INTO TUserNew (FName, FAlias) VALUES ('Alice', '1'), ('Bob', '25'), ('Charlie', '35') RETURNING UserK;", []( const auto* parguments_ ) {
      std::string string_ = gd::argument::debug::print( *parguments_ );
      std::cout << "Insert callback arguments: " << string_ << std::endl;
      return true;
   }); REQUIRE(result_.first == true);


   databaseSqlite.close();
}
