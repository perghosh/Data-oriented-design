// @FILE [tag: sql, build] [description: SqlBuilder service is used to generate SQL queries] [name: SERVICE_SqlBuilder.cpp] [type: source]


#include "gd/gd_sql_value.h"

#include "SERVICE_SqlBuilder.h"

NAMESPACE_SERVICE_BEGIN

std::pair<bool, std::string> CSqlBuilder::Initialize( gd::argument::shared::arguments arguments_ )
{
   m_argumentsValues = arguments_;
   return { true, "" };
}

std::pair<bool, std::string> CSqlBuilder::Initialize( gd::argument::shared::arguments arguments_, std::string_view stringSql )
{
   m_argumentsValues = arguments_;
   m_stringSql = stringSql;

   return { true, "" };
}

/**
 * Build SQL query from template string and arguments values.
 */
std::pair<bool, std::string> CSqlBuilder::Build( std::string& stringSqlReady )
{
   std::string stringNew;
   stringNew.reserve( m_stringSql.size() );
   auto result_ = gd::sql::replace_g( m_stringSql, m_argumentsValues, stringNew, gd::sql::tag_brace{} ); 
   if( result_.first == false ) { return result_; }
   stringSqlReady = std::move(stringNew);
   return { true, "" };
}

NAMESPACE_SERVICE_END