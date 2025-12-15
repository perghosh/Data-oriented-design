#include <filesystem>

#include "gd/gd_database_sqlite.h"

#include "APIDatabase.h"


void CAPIDatabase::common_construct( const CAPIDatabase& o ) 
{ 
    m_vectorCommand = o.m_vectorCommand;
    m_argumentsParameter = o.m_argumentsParameter;
}

void CAPIDatabase::common_construct( CAPIDatabase&& o ) noexcept 
{ 
    m_vectorCommand = std::move( o.m_vectorCommand );
    m_argumentsParameter = std::move( o.m_argumentsParameter );
}

std::pair<bool, std::string> CAPIDatabase::Execute()
{                                                                                                  assert( m_vectorCommand.empty() == false );
   // ## execute database command based on m_vectorCommand and m_argumentsParameter

   std::string_view stringCommand = m_vectorCommand[0];
   if( stringCommand == "db" )
   {
      stringCommand = m_vectorCommand.size() > 1 ? m_vectorCommand[1] : "";
   }

   if( stringCommand == "create" )
   {
      return Execute_Create();
   }
   else if( stringCommand == "delete" )
   {
   }
    
   return { true, "" };
}

std::pair<bool, std::string> CAPIDatabase::Execute_Create()
{
   std::string stringType = m_argumentsParameter["type"].as_string();
   std::string stringName = m_argumentsParameter["name"].as_string();

   if( stringType.empty() == true || stringType == "sqlite" )
   {
      // ## create sqlite database with name

      // ### Check the file name for sqlite database so it do not exists
      std::filesystem::path pathFile( stringName );
      if( pathFile.has_extension() == false ) { pathFile += ".sqlite"; }

      if( pathFile.is_absolute() == false )
      {
         std::filesystem::path pathAbsolute = std::filesystem::absolute( pathFile );
         if( std::filesystem::exists( pathAbsolute ) == true ) { return { false, "database file already exists: " + pathAbsolute.string() }; }
         pathFile = pathAbsolute;
         stringName = pathFile.string();
      }

      gd::database::sqlite::database databaseNew;
   }


   return { true, "" };
}