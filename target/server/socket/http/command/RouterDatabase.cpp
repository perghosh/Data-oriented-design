#include "gd/gd_database_sqlite.h"
#include "gd/gd_database_odbc.h"
#include "gd/gd_file.h"

#include "RouterDatabase.h"


std::pair<bool, std::string> CRouterDatabase::Execute(const std::string_view& stringCommand, gd::com::server::command_i* pCommand, gd::com::server::response_i* presponse) 
{


   return {true, ""};
}

std::pair<bool, std::string> CRouterDatabase::CreateDatabase(const gd::argument::arguments& arguments_)
{
   // ## Test if database is a sqlite database
   
   if( arguments_.exists("file") == true )
   {
      auto stringFile = arguments_["file"].as_string();

      gd::file::path pathDatabaseFile(stringFile);
      std::string stringName = pathDatabaseFile.stem().string();               // get filename without extension
      if ( stringName.empty() == true ) return { false, "No database name" };

      // ## Create database
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i(stringName);
      auto result_ = pdatabase->open({ {"file", pathDatabaseFile.string()}, {"create", true}});
      
      
      //result_ = pdatabase->execute(stringSql2);
   }
   else if( arguments_.exists("dsn") == true )
   {
      // ## Create ODBC database
      auto stringDSN = arguments_["dsn"].as_string();
      //gd::database::odbc::database_i* pdatabase = new gd::database::odbc::database_i("db02");
   }
   else
   {
      return { false, "No database file" };
   }

   return std::pair<bool, std::string>();
}
