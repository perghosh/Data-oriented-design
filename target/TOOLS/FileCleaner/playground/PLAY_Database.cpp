#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_sql_value.h"

#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE( "[database] test database in cleaner", "[database]" ) {
   CApplication application_;

   std::string stringDatabaseFile = FOLDER_GetRoot_g( "temp__/test.db" );
   gd::file::path path_(stringDatabaseFile);
   if( std::filesystem::exists(path_) == true ) { std::filesystem::remove(path_); }

   auto result_ = application_.DATABASE_Open({ {"file", stringDatabaseFile} });                     REQUIRE(result_.first == true);
   result_ = application_.DATABASE_Update();                                                       REQUIRE(result_.first == true);

   std::string stringStatementsFile = FOLDER_GetRoot_g( "resource/cleaner-statements.xml" );
   result_ = application_.STATEMENTS_Load( stringStatementsFile );                                 REQUIRE(result_.first == true);

   application::database::metadata::CStatements* pstatements_ = application_;

   gd::database::database_i* pdatabase_ = application_;                        // pointer to application database
   const auto* pstatement = pstatements_->Find( "insert", "project" );                             REQUIRE(pstatement != nullptr);
   auto stringInsert = pstatement->GetSql().as_string();

   for( auto u = 0u; u < 100; ++u )
   {
      auto string_ = gd::sql::replace_g(stringInsert, { { "name", std::string("project-name: ") + std::to_string( u ) }, { "description", "project description" }, {"version", 1} }, gd::sql::tag_brace{});
      auto result_ = pdatabase_->execute(string_);                                                 REQUIRE(result_.first == true);
   }

   pstatement = pstatements_->Find("select", "project");                                           REQUIRE(pstatement != nullptr);
   gd::database::cursor_i* pcursor_;
   pdatabase_->get_cursor( &pcursor_ );
   result_ = pcursor_->open(pstatement->GetSql());                                                 REQUIRE(result_.first == true);


/*
   gd::argument::arguments arguments_;
   arguments_.append("test-folder", stringFolder);
   arguments_.append("database-file", "test.db");
   std::cout << arguments_.print( "Folder where database are places is {} and database file is: {}\n" );
   */
}


