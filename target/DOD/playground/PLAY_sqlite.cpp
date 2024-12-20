#include <random>
#include <chrono>
#include <memory>
#include <ranges>
#include <filesystem>


#include "gd/gd_cli_options.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_file.h"
#include "gd/gd_file_rotate.h"
#include "gd/gd_utf8.h"
#include "gd/gd_database_sqlite.h"

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


   std::cout << GetApplicationFolder() << "\n";

}