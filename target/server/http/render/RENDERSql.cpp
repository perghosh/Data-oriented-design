// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.cpp]

#include <cassert>
#include <sstream>

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "gd/gd_arguments_io.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_query.h"

#include "RENDERSql.h"


void CRENDERSql::Initialize()
{
   // Initialize datable that will hold response body parts

   if( m_pcolumnsField_s == nullptr )
   {
      constexpr auto uSize = m_uMaxStringBufferLength_s;
      m_pcolumnsField_s = new gd::table::detail::columns{};                    /// static columns for body, remember to delete on shutdown (release)
      m_pcolumnsField_s->add( "uint32", 0, "key" );
      m_pcolumnsField_s->add( "string", uSize, "table" );                     // name for table field belongs to
      m_pcolumnsField_s->add( "string", uSize, "column" );                    // name for column in table
      m_pcolumnsField_s->add( "string", uSize, "alias" );                     // alias for column in table
      m_pcolumnsField_s->add( "string", uSize * 2, "value" );                 // value for column in table
      m_pcolumnsField_s->add( "uint32", 0, "type" );                          // type of value
      m_pcolumnsField_s->add_reference();
   }

   m_tableField.set_columns( m_pcolumnsField_s );
   m_tableField.prepare();
}

void CRENDERSql::AddValue( const gd::argument::arguments argumentsField )
{
   auto uRow = m_tableField.row_add_one();
   m_tableField.cell_set( uRow, 0u, uint32_t(uRow + 1) );                     // set key value

   for( auto [key_, value_] : argumentsField.named() )
   {
      if( value_.is_string() == true ) { Add( key_, value_.as_string_view() ); }
      else
      { 
         if( key_ == "type" ) { m_tableField.cell_set( uRow, eColumnFieldType, value_); }
         else { Add( key_, value_ ); }
      }
   }
}

void CRENDERSql::Add( std::string_view stringName, std::string_view stringValue )
{
   uint64_t uRow = m_tableField.get_row_count() - 1;
   m_tableField.cell_set( uRow, stringName, stringValue, gd::table::tag_spill{});
}

void CRENDERSql::Add( std::string_view stringName, gd::variant_view variantviewValue )
{
   std::string stringValue = variantviewValue.as_string();
   Add( stringName, std::string_view( stringValue ) );
}

std::pair<bool,std::string> CRENDERSql::GetQuery( enumSqlQueryType eSqlQueryType, std::string& stringQuery )
{
   switch( eSqlQueryType )
   {
      case eSqlQueryTypeInsert:
         break;
      case eSqlQueryTypeUpdate:
         break;
      case eSqlQueryTypeDelete:
         break;
      case eSqlQueryTypeSelect:
         break;
      case eSqlQueryTypeCount:
         break;
      default:
         return {false, "Invalid query type"};
   }
   return {true, stringQuery};
}

std::pair<bool,std::string> CRENDERSql::ToSqlInsert( std::string& stringQuery )
{
   std::string stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();

   gd::sql::query queryInsert( gd::sql::eSqlInsert, stringTable, gd::sql::tag_table{});
   queryInsert.sql_set_dialect( gd::sql::sql_get_dialect_g("sqlite"));

   std::vector<gd::variant_view> vectorValue;
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
      queryInsert.field_add( stringColumn );
      auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
      vectorValue.push_back( value_ );
   }

   // ## Generate insert query ..............................................

   std::string stringInsertSql;
	stringInsertSql += "INSERT INTO ";
	stringInsertSql += queryInsert.sql_get_insert();
	stringInsertSql += "\nVALUES(";
	stringInsertSql += gd::sql::query::values_get_s( vectorValue ).second;// append values from name value pairs
	stringInsertSql += ")";

   // ## Set the out string to query .........................................

   if( stringQuery.empty() == true ) stringQuery = std::move( stringInsertSql );
   else stringQuery += "\n\n" + stringInsertSql;

   return { true, "" };
}


/// Destroy static members
void CRENDERSql::Destroy_s()
{
   if( m_pcolumnsField_s != nullptr )
   {                                                                          assert( m_pcolumnsField_s->get_reference() == 1 );
      m_pcolumnsField_s->release();
      m_pcolumnsField_s = nullptr;
   }
}
