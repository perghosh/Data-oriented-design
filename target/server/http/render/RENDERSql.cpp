// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.cpp]

#include <cassert>
#include <sstream>

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "gd/gd_arguments_io.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_query.h"

#include "gd/parse/gd_parse_json.h"

#include "../convert/CONVERTCore.h"

#include "../Document.h"

#include "RENDERSql.h"



/** --------------------------------------------------------------------------
 * @brief Initializes the static data table structure used to hold response body parts for SQL field information.
 */
void CRENDERSql::Initialize()
{
   // Initialize datable that will hold response body parts

   if( m_pcolumnsField_s == nullptr )
   {
      constexpr auto uSize = m_uMaxStringBufferLength_s;
      m_pcolumnsField_s = new gd::table::detail::columns{};                    /// static columns for body, remember to delete on shutdown (release)
      m_pcolumnsField_s->add( "uint32", 0, "key" );
      m_pcolumnsField_s->add( "string", uSize, "schema" );                    // schema for table field belongs to
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

/** --------------------------------------------------------------------------
 * @brief Add new field to be used in  SQL query that is generated.
 *
 * @param argumentsField information about the field to add.
 */
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

/** ---------------------------------------------------------------------------
 * @brief Add a value to the table field.
 *
 * @param stringJson The JSON string to parse.
 * @param tag_json The tag_json type.
 * @return std::pair<bool,std::string> A pair containing a boolean indicating success and a string containing an error message.
 */
std::pair<bool,std::string> CRENDERSql::AddValue( std::string_view stringJson, gd::types::tag_json )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_(buffer_);
   
   auto result_ = gd::parse::json::parse_shallow_object_g( stringJson, arguments_, false );
   if( result_.first == false ) { return result_; }
   
   // check for value
   if( arguments_.exists("value") == false ) { return {false, "missing value"}; }

   AddValue( arguments_ );

   return {true, ""};
}

std::pair<bool,std::string> CRENDERSql::AddRecord( std::string_view stringJson, gd::types::tag_json )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_(buffer_);

   try 
   {
      jsoncons::json jsonRecord = jsoncons::json::parse(stringJson);

      // ## Extract table and fields from json
      auto jsonTable = jsonRecord["table"];
      std::string stringTable = jsonTable.as_string();

      auto jsonValues = jsonRecord["values"];

      for( const auto& itValue : jsonValues.object_range() )
      {
         arguments_.clear();
         arguments_.append( "table", stringTable );
         arguments_.append( "column", itValue.key() );
         arguments_.append_argument( "value", CONVERT::AsVariant( itValue.value() ) );
         AddValue( arguments_ );
      }

      auto jsonReturning = jsonRecord["returning"];
      if( jsonReturning.is_string() )
      {
         AddProperty( "returning", jsonReturning.as_string_view() );
      }


   }
   catch( jsoncons::json_exception& e )
   {
      std::string stringError = e.what();
      return {false, stringError};
   }
   
   
   return {true, ""};
}

std::pair<bool, std::string> CRENDERSql::Prepare()
{                                                                                                  assert( m_pdocument != nullptr ); assert( m_tableField.size() > 1 ); // at least one field should be added before preparing query
   std::array<std::byte, 256> buffer_;
   const META::CDatabase* pdatabase_ = m_pdocument->DATABASE_Get();

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      std::string stringTable = itRow.cell_get_variant_view( "table", gd::table::tag_not_null{}).as_string();
      std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();

      gd::argument::arguments argumentsFind( buffer_ );
      argumentsFind.append( { {"table", stringTable}, {"column", stringColumn} });
      int64_t iRow = pdatabase_->Column_FindRow( argumentsFind );                                  assert( iRow >= 0 && "Developer error because this should not assert");

      if( iRow > 0 )
      {
         uint32_t uType = pdatabase_->Column_GetType( iRow );
         itRow.cell_set( "type", uType ); // set type for column in table field
      }
   }


   // ## Prepare the query, this can include validation and transformation of the data in the table field before generating the final SQL query.
   // For example, you can check if all required fields are present, if the data types are correct, or if there are any constraints that need to be applied.




   return {true, ""};
}



/** ---------------------------------------------------------------------------
 * @brief Add a value to last row witch are the latest added field..
 *
 * @param stringName The name of the column.
 * @param stringValue The value to add.
 */
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

void CRENDERSql::SetColumnValue( std::string_view stringName, gd::variant_view variantviewValue )
{
   auto iColumn = m_tableField.column_find_index( stringName );
   if( iColumn < 0 ) return
   m_tableField.column_fill( (unsigned)iColumn, variantviewValue, gd::table::tag_convert{} );
}

std::pair<bool,std::string> CRENDERSql::GetQuery( enumSqlQueryType eSqlQueryType, std::string& stringQuery )
{
   std::pair<bool, std::string> result_( true, "" );
   switch( eSqlQueryType )
   {
      case eSqlQueryTypeInsert:
      result_ = ToSqlInsert( stringQuery );
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
   return result_;
}

/** --------------------------------------------------------------------------
 * @brief Generates a SQL INSERT statement and appends it to the provided query string.
 * @param stringQuery A reference to the string where the generated INSERT statement will be appended. 
 * @return A pair containing a boolean success status (true if successful) and an error message string (empty on success).
 */
std::pair<bool,std::string> CRENDERSql::ToSqlInsert( std::string& stringQuery )
{
   std::string stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();

   gd::sql::query queryInsert( gd::sql::eSqlInsert, stringTable, gd::sql::tag_table{});
   queryInsert.sql_set_dialect( gd::sql::sql_get_dialect_g("sqlite"));

   // ## Extract column names and values from table field and add to query ...
   
   std::vector< std::pair<uint32_t, gd::variant_view> > vectorValue;
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
      queryInsert.field_add( stringColumn );                                  // add column to query
      uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
      auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
      vectorValue.push_back( { uType, value_ } );                             // add value to vector for later use in query
   }

   // ## Generate insert query ..............................................

   std::string stringInsertSql;
	stringInsertSql += "INSERT INTO ";
	stringInsertSql += queryInsert.sql_get_insert();
	stringInsertSql += "\nVALUES(";
	stringInsertSql += gd::sql::query::values_get_s( vectorValue, m_eSqlDialect ).second;// append values from name value pairs
	stringInsertSql += ")";

   std::string_view stringReturning = GetProperty( "returning" ).as_string_view();
   if( stringReturning.empty() == false )
   {
      stringInsertSql += "\n";                                                // append newline before returning clause
      gd::sql::query::returning_get_s( stringReturning, stringInsertSql, m_eSqlDialect ); // generate returning clause and append to insert query
   }

   // ## Set the out string to query .........................................

   if( stringQuery.empty() == true ) stringQuery = std::move( stringInsertSql );
   else stringQuery += "\n\n" + stringInsertSql;

   return { true, "" };
}

std::pair<bool, std::string> CRENDERSql::ToSqlUpdate( std::string& stringQuery )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_( buffer_ );

   std::string stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();

   gd::sql::query queryUpdate( gd::sql::eSqlUpdate, stringTable, gd::sql::tag_table{});

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      arguments_.clear();
      std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
      uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
      auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
      arguments_.append( { { "name", stringColumn }, { "value", value_ }, { "type", uType } }, gd::types::tag_view{});
   }


   // ## Generate insert query ..............................................

   std::string stringUpdateSql;
	stringUpdateSql += queryUpdate.sql_get_update();
	stringUpdateSql += "\nWHERE ";
	//stringUpdateSql += gd::sql::query::where_get_s( vectorValue, m_eSqlDialect ).second;// append where clause from name value pairs


   if( stringQuery.empty() == true ) stringQuery = std::move( stringUpdateSql );
   else stringQuery += "\n\n" + stringUpdateSql;

   return { true, "" };
}

std::pair<bool, std::string> CRENDERSql::ToSqlDelete( std::string& stringQuery )
{
   // ## Generate insert query ..............................................
   /*
   std::string stringDeleteSql;
	stringDeleteSql += queryUpdate.sql_get_delete( vectorValue );
	stringDeleteSql += "\nWHERE ";
	//stringUpdateSql += gd::sql::query::where_get_s( vectorValue, m_eSqlDialect ).second;// append where clause from name value pairs

   if( stringQuery.empty() == true ) stringQuery = std::move( stringDeleteSql );
   else stringQuery += "\n\n" + stringDeleteSql;
   */

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
