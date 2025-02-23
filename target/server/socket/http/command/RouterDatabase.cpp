#include "gd/gd_database_sqlite.h"
#include "gd/gd_database_odbc.h"
#include "gd/gd_file.h"

#include "RouterDatabase.h"



std::pair<bool, std::string> CRouterDatabase::get(gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse)
{
   gd::com::server::router::command* pcommand_ = (gd::com::server::router::command*)pcommand;

   if( pcommand_->empty() == false )
   {
      auto* parguments = pcommand_->get_command(0);
      auto stringCommand = (*parguments)[0];
      if( stringCommand == std::string_view{"database"} ) { stringCommand = (*parguments)[1]; }
      auto result_ = Execute( stringCommand, pcommand_, presponse );
   }

   return {true, ""};
}


std::pair<bool, std::string> CRouterDatabase::Execute(const std::string_view& stringCommand, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse)
{
   gd::com::server::router::command* pcommand_ = (gd::com::server::router::command*)pcommand;

   if( stringCommand == "create" )
   {
      gd::argument::arguments arguments_;
      if( pcommand_->get_active() != -1 ) arguments_ = pcommand_->query_select( { "filename", "dsn" }, (size_t)pcommand_->get_active() );
      else                                arguments_ = pcommand_->query_select( { "filename", "dsn" } );
      return CreateDatabase(arguments_);
   }


   return {true, ""};
}

std::pair<bool, std::string> CRouterDatabase::CreateDatabase(const gd::argument::arguments& arguments_)
{
   // ## Test if database is a sqlite database
   
   if( arguments_.exists("filename") == true )
   {
      auto stringFile = arguments_["filename"].as_string();

      gd::file::path pathDatabaseFile(stringFile);
      std::string stringName = pathDatabaseFile.stem().string();               // get filename without extension
      if ( stringName.empty() == true ) return { false, "No database name" };

      // ## Create database
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i(stringFile);
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

   return {true, ""};
}

std::pair<bool, std::string> CRouterDatabase::RemoveDatabase(const gd::argument::arguments& arguments_)
{
   return {true, ""};
}
