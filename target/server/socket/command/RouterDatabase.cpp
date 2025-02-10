#include "gd/gd_database_sqlite.h"
#include "gd/gd_file.h"

#include "RouterDatabase.h"


std::pair<bool, std::string> CRouterDatabase::Execute(const std::string_view& stringCommand, gd::com::server::command_i* pCommand, gd::com::server::response_i* presponse) 
{
    // Implementation of the Execute method
    // For now, just return a dummy response
    return {true, ""};
}

std::pair<bool, std::string> CRouterDatabase::CreateDatabase(const gd::argument::arguments& arguments_)
{
   // ## Test if database is a sqlite database
   auto stringFile = arguments_["file"].as_string();
   if ( stringFile.empty() == false )
   {
      gd::file::path pathDatabaseFile(stringFile);
      std::string stringName = pathDatabaseFile.filename();
      if ( stringName.empty() == true ) return { false, "No database name" };

      // ## Create database
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i(stringName);
      auto result_ = pdatabase->open({ {"file", pathDatabaseFile.string()}, {"create", true}});
      
      
      //result_ = pdatabase->execute(stringSql2);
   }

   return std::pair<bool, std::string>();
}
