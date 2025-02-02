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
#include "gd/gd_sql_query.h"

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


TEST_CASE(" [sqlite] generate sql 01", "[sqlite]") {
   {
      gd::sql::query query;
   }
}

TEST_CASE(" [sqlite] create3", "[sqlite]")
{
   std::string stringSql = R"SQL(CREATE TABLE TProduct (
      ProductK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FProductType VARCHAR(20),
      FName VARCHAR(50)
   );)SQL";

   std::string stringSql2 = R"SQL(CREATE TABLE TProduct_Sales (
      Product_SalesK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      ProductK INTEGER,
      FSales INTEGER
   );)SQL";

   std::string stringDbName = GetApplicationFolder();
   stringDbName += "db03.sqlite";
   if( std::filesystem::exists(stringDbName) == true ) { std::filesystem::remove(stringDbName); }

   gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i("db03");
   auto result_ = pdatabase->open({ {"file", stringDbName}, {"create", true} });                   REQUIRE(result_.first == true);
   result_ = pdatabase->execute(stringSql);                                                        REQUIRE(result_.first == true);
   result_ = pdatabase->execute(stringSql2);                                                        REQUIRE(result_.first == true);


   pdatabase = new gd::database::sqlite::database_i("db03");
   result_ = pdatabase->open({ {"file", stringDbName} });                                          REQUIRE(result_.first == true);
   std::string stringSqlInsert = R"SQL(INSERT INTO TProduct(FProductType, FName) VALUES('Business', 'Visual');)SQL";

   std::string stringSqlInsert2 = R"SQL(INSERT INTO TProduct_Sales(ProductK, FSales) VALUES(1, 100);)SQL";

   result_ = pdatabase->execute(stringSqlInsert);                                                  REQUIRE(result_.first == true);

   result_ = pdatabase->execute(stringSqlInsert2);                                                  REQUIRE(result_.first == true);

   gd::database::cursor_i* pcursor = nullptr;
   pdatabase->get_cursor(&pcursor);

   result_ = pcursor->open("SELECT * FROM TProduct;");                                             REQUIRE(result_.first == true);
   gd::table::dto::table tableProduct;
   gd::database::to_table(pcursor, &tableProduct);
   std::string stringResult;
   gd::table::to_string(tableProduct, stringResult, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult << "\n";

   std::string stringSqlJoin = R"SQL(
   SELECT TProduct.ProductK, TProduct.FProductType, TProduct.FName, TProduct_Sales.Product_SalesK, TProduct_Sales.FSales
   FROM TProduct
   INNER JOIN TProduct_Sales ON TProduct.ProductK = TProduct_Sales.ProductK)SQL";

   result_ = pcursor->open(stringSqlJoin);                                                        REQUIRE(result_.first == true);
   gd::table::dto::table tableJoin;
   gd::database::to_table(pcursor, &tableJoin);
   std::string stringResult2;
   gd::table::to_string(tableJoin, stringResult2, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult2 << "\n";

   pcursor->close();



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

   std::string stringSql3 = R"SQL(CREATE TABLE TAddress (
      AddressK INTEGER PRIMARY KEY AUTOINCREMENT,
      CustomerK INTEGER,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FCity VARCHAR(50),
      FAddress VARCHAR(50),
      FRegion VARCHAR(100)
   );)SQL";

   std::string stringSql4 = R"SQL(CREATE TABLE TPopulation (
      PopulationK INTEGER PRIMARY KEY AUTOINCREMENT,
      CreateD TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      FCity VARCHAR(50),
      FPopulation INTEGER
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
   std::string stringSqlInsert2 = R"SQL(INSERT INTO TAddress(FCity, FAddress, FRegion) VALUES('kungälv', 'gata 2', 'västragötaland');)SQL";
   std::string stringSqlInsert3 = R"SQL(INSERT INTO TPopulation(FCity, FPopulation) VALUES('göteborg', 350000);)SQL";
   result_ = pdatabase->execute(stringSqlInsert);                                                  REQUIRE(result_.first == true);

   gd::database::cursor_i* pcursor = nullptr;
   pdatabase->get_cursor(&pcursor);

   result_ = pcursor->open("SELECT * FROM TCustomer;");                                             REQUIRE(result_.first == true);
   gd::table::dto::table tableCustomer;
   gd::database::to_table(pcursor, &tableCustomer);
   std::string stringResult;
   gd::table::to_string(tableCustomer, stringResult, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult << "\n";


   result_ = pdatabase->execute(stringSql3);
   result_ = pdatabase->execute(stringSqlInsert2);                                                  REQUIRE(result_.first == true);
   result_ = pdatabase->execute(stringSqlInsert2);                                                  REQUIRE(result_.first == true);
   stringSqlInsert2 = R"SQL(INSERT INTO TAddress(FCity, FAddress, FRegion) VALUES('stenungsund', 'gata 5', 'västragötaland');)SQL";
   result_ = pdatabase->execute(stringSqlInsert2);                                                  REQUIRE(result_.first == true);

   gd::variant variantKey;
   result_ = pdatabase->ask("SELECT CustomerK FROM TCustomer WHERE FName = 'Visual';", &variantKey);REQUIRE(result_.first == true);
   std::cout << "Customer key: " << variantKey.as<int64_t>() << "\n";

   std::string stringUpdate = "UPDATE TAddress SET CustomerK = ";  
   stringUpdate += variantKey.as<std::string>();
   stringUpdate += " WHERE FCity = 'kungälv' ";

   result_ = pdatabase->execute(stringUpdate);                                                     REQUIRE(result_.first == true);

   result_ = pdatabase->execute("UPDATE TAddress SET CustomerK = " + stringUpdate);

   result_ = pcursor->open("SELECT * FROM TAddress;");                                             REQUIRE(result_.first == true);
   gd::table::dto::table tableAddress;
   gd::database::to_table(pcursor, &tableAddress);
   std::string stringResult2;
   gd::table::to_string(tableAddress, stringResult2, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult2 << "\n";

   result_ = pdatabase->execute(stringSql4);
   result_ = pdatabase->execute(stringSqlInsert3);                                                 REQUIRE(result_.first == true);

   result_ = pcursor->open("SELECT FPopulation FROM TPopulation;");                                REQUIRE(result_.first == true);
   gd::table::dto::table tablePopulation;
   gd::database::to_table(pcursor, &tablePopulation);
   std::string stringResult3;
   gd::table::to_string(tablePopulation, stringResult3, gd::table::tag_io_header{}, gd::table::tag_io_csv{});
   std::cout << stringResult3 << "\n";

   pcursor->close();
   result_ = pcursor->open(R"SQL(
SELECT Customer.CustomerK, Customer.FName AS CustomerName, Address.FCity AS City, Address.FRegion AS Region 
FROM TCustomer AS Customer JOIN TAddress AS Address ON Customer.CustomerK=Address.CustomerK)SQL"); REQUIRE(result_.first == true);
   gd::table::dto::table tableJoin;
   gd::database::to_table( pcursor, &tableJoin );
   stringResult = gd::table::to_string(tableJoin, gd::table::tag_io_cli{});
   std::cout << stringResult << "\n";

   pcursor->release();
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
