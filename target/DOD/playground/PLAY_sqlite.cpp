#include <random>
#include <chrono>
#include <memory>
#include <ranges>
#include <filesystem>


#include "gd/gd_com.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_io.h"

#include "gd/gd_database_sqlite.h"
#include "gd/database/gd_database_io.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_define.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_printer2.h"

/// Return folder for application as string.
std::string GetApplicationFolder()
{
   std::string stringFilePath = mainarguments_g.m_ppbszArgumentValue[0];
   auto position_ = stringFilePath.find_last_of("\\/");
   if( position_ != std::string::npos ) { stringFilePath = stringFilePath.substr( 0, position_ + 1 ); }

   return stringFilePath;
}


TEST_CASE(" [sqlite] create2", "[sqlite]")
{
   std::string stringSql2 = R"SQL(CREATE TABLE TCustomer (
      CustomerK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FCustomerType VARCHAR(20),
      FName VARCHAR(50),
      FAddress VARCHAR(50),
      FEmail VARCHAR(100)
   );)SQL";

   std::string stringDbName = GetApplicationFolder();
   stringDbName += "db02.sqlite";
   if( std::filesystem::exists(stringDbName) == true) {std::filesystem::remove(stringDbName);}

   gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i("db02");
   auto result_ = pdatabase->open({ {"file", stringDbName}, {"create", true} });                   REQUIRE(result_.first == true);
   result_ = pdatabase->execute(stringSql2);
   

   pdatabase = new gd::database::sqlite::database_i("db02");
   result_ = pdatabase->open({ {"file", stringDbName} });                                          REQUIRE(result_.first == true);
   std::string stringSqlInsert = R"SQL(INSERT INTO TCustomer(FCustomerType, FName, FAddress, FEmail) VALUES('Business', 'Visual', 'Street 4', 'visual@gmail.com');)SQL";
   result_ = pdatabase->execute(stringSqlInsert);                                                  REQUIRE(result_.first == true);

   gd::database::cursor_i* pcursor = nullptr;
   pdatabase->get_cursor(&pcursor);

   result_ = pcursor->open("SELECT * FROM TCustomer;");                                             REQUIRE(result_.first == true);
   gd::table::dto::table tableCustomer;
   gd::database::to_table(pcursor, &tableCustomer);
   std::string stringResult;
   gd::table::to_string(tableCustomer, stringResult, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult << "\n";
   pdatabase->release();
}

TEST_CASE( "[sqlite] create", "[sqlite]" ) {
   gd::log::logger<0>* plogger = gd::log::get_s();
   plogger->clear();

   std::string stringSql = R"SQL(CREATE TABLE TUser (
      UserK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FName VARCHAR(50),
      FSurname VARCHAR(50),
      FAge INTEGER,
      FGender INTEGER
   );)SQL";

   std::string stringDbName = GetApplicationFolder();
   stringDbName += "db01.sqlite";
   if( std::filesystem::exists( stringDbName ) == true ) { std::filesystem::remove( stringDbName ); }

   gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i("db01");
   auto result_ = pdatabase->open( { {"file", stringDbName }, {"create", true } } );               REQUIRE( result_.first == true );
   pdatabase->execute( stringSql );

   pdatabase->close();
   pdatabase->release();

   // ## add record to TUser
   pdatabase = new gd::database::sqlite::database_i("db01");
   result_ = pdatabase->open({ {"file", stringDbName } });                                         REQUIRE(result_.first == true);
   std::string stringSqlInsert = R"SQL(INSERT INTO TUser(FName, FSurname, FAge, FGender) VALUES('John', 'Doe', 25, 1);)SQL";
   result_ = pdatabase->execute(stringSqlInsert);                                                  REQUIRE(result_.first == true);

   gd::database::cursor_i* pcursor = nullptr;
   pdatabase->get_cursor( &pcursor );

   result_ = pcursor->open("SELECT * FROM TUser;");                                                REQUIRE(result_.first == true);
   gd::table::dto::table tableUser;
   gd::database::to_table( pcursor, &tableUser );
   std::string stringResult;
   gd::table::to_string(tableUser, stringResult, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult << "\n";
   stringResult = gd::table::to_string(tableUser, gd::table::tag_io_cli{});
   std::cout << stringResult << "\n";
   pcursor->close();
   pcursor->release();

   pdatabase->close();
   pdatabase->release();
}

TEST_CASE( "[sqlite] create with smart pointer", "[sqlite]" ) {
   std::string stringSql = R"SQL(CREATE TABLE TUser (
      UserK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FName VARCHAR(50),
      FSurname VARCHAR(50),
      FAge INTEGER,
      FGender INTEGER
   );)SQL";

   std::string stringDbName = GetApplicationFolder();
   stringDbName += "db01.sqlite";
   if( std::filesystem::exists( stringDbName ) == true ) { std::filesystem::remove( stringDbName ); }

   {
      gd::com::pointer<gd::database::sqlite::database_i> pdatabase = new gd::database::sqlite::database_i("db01");
      auto result_ = pdatabase->open( { {"file", stringDbName }, {"create", true } } );               REQUIRE( result_.first == true );
      pdatabase->execute( stringSql );
   }

   {
      // ## add record to TUser
      gd::com::pointer<gd::database::sqlite::database_i> pdatabase = new gd::database::sqlite::database_i("db01");
      auto result_ = pdatabase->open({ {"file", stringDbName } });                                         REQUIRE(result_.first == true);
      std::string stringSqlInsert = R"SQL(INSERT INTO TUser(FName, FSurname, FAge, FGender) VALUES('John', 'Doe', 25, 1);)SQL";
      result_ = pdatabase->execute(stringSqlInsert);                                                  REQUIRE(result_.first == true);

      {
         gd::com::pointer<gd::database::cursor_i> pcursor;
         pdatabase->get_cursor( &pcursor );

         result_ = pcursor->open("SELECT * FROM TUser;");                                                REQUIRE(result_.first == true);
         gd::table::dto::table tableUser;
         gd::database::to_table( pcursor, &tableUser );
         std::string stringResult;
         gd::table::to_string(tableUser, stringResult, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
         std::cout << stringResult << "\n";
         stringResult = gd::table::to_string(tableUser, gd::table::tag_io_cli{});
         std::cout << stringResult << "\n";
      }
   }
}

TEST_CASE( "[sqlite] arguments table", "[sqlite]" ) {
   //gd::table::arguments::table table(10);
   {
      gd::table::arguments::table table_(  (unsigned)gd::table::arguments::table::eTableFlagAll,{ { "int64", 0, "FInteger"} }, gd::table::tag_prepare{} );

      auto uArgumentsSize = sizeof( gd::argument::shared::arguments );

      auto uRow = table_.get_row_count();
      table_.row_add();
      table_.cell_set( uRow, "FInteger", int64_t(10) );
      auto* parguments_ = table_.row_create_arguments(uRow);
      parguments_->set("ten", uint32_t(10));

      parguments_ = table_.row_get_arguments_pointer(uRow);                                        REQUIRE(parguments_->size() == 1);
      parguments_->set("eleven", uint32_t(11));                                                    REQUIRE(parguments_->size() == 2);

      //char* pbszData = new char[100];

      table_.cell_set( uRow, "new", uint32_t(10) );
      auto arguments_ = table_.row_get_arguments( 0 );

      auto string_d = gd::argument::debug::print(arguments_);
      std::cout << string_d << "\n";

      // int i = table_.cell_get_variant_view(0, "FInteger").get<int>();
   }
}

// Struct with copy and assignment operator
struct test_struct
{
   test_struct() = default;
   test_struct(const test_struct&) = default;
   test_struct& operator=(const test_struct&) = default;
   test_struct(test_struct&&) = default;
   test_struct& operator=(test_struct&&) = default;
   ~test_struct() = default;
   int m_iValue = 0;
};
