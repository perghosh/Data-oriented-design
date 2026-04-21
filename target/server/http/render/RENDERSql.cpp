// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.cpp]

#include <cassert>
#include <sstream>
#include <string_view>

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

CRENDERSql::CRENDERSql( const CDocument* pdocument ) : m_pdocument( pdocument ), m_tableField( 8, gd::table::tag_full_meta{} ) 
{                                                                                                  assert( pdocument ); 
   if( pdocument->DATABASE_Get() ) { m_eSqlDialect = gd::sql::enumSqlDialect( pdocument->DATABASE_Get()->GetDialect() ); }
}

void CRENDERSql::common_construct( const CRENDERSql& o ) 
{
   m_pdocument = o.m_pdocument;
   m_eSqlDialect = o.m_eSqlDialect;
   m_tableField = o.m_tableField;
   m_argumentsProperty = o.m_argumentsProperty;
}

void CRENDERSql::common_construct( CRENDERSql&& o ) noexcept 
{
   m_pdocument = o.m_pdocument;
   m_eSqlDialect = o.m_eSqlDialect;
   m_tableField = std::move(o.m_tableField);
   m_argumentsProperty = std::move(o.m_argumentsProperty);
}


/** --------------------------------------------------------------------------
 * @brief Initializes the static data table structure used to hold response body parts for SQL field information.
 */
void CRENDERSql::Initialize()
{
   using namespace std::literals::string_view_literals;
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
      m_pcolumnsField_s->add( "uint32", 0, "part_type" );                     // part type of value, this same as sql part like select, where    
      m_pcolumnsField_s->add_reference();
                                                                                                   assert( m_pcolumnsField_s->find_index( "key"sv ) == eColumnFieldId );
                                                                                                   assert( m_pcolumnsField_s->find_index( "alias"sv ) == eColumnFieldAlias );
                                                                                                   assert( m_pcolumnsField_s->find_index( "part_type"sv ) == eColumnFieldPartType );
   }

   m_tableField.set_columns( m_pcolumnsField_s );
   m_tableField.prepare();
}

std::pair<bool, std::string> CRENDERSql::Add( const pugi::xml_node& xmlnodeValues )
{                                                                                                  assert( m_pcolumnsField_s != nullptr );
   using namespace gd::types::detail;
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsField( buffer_ );

   std::string stringTable = xmlnodeValues.attribute( "table" ).as_string();   // Table name

   for(pugi::xml_node node_ : xmlnodeValues.children()) 
   {
      enumPartType ePartType = ePartTypeUnknown;
      std::string_view stringNodeName = node_.name();
      if( stringNodeName.length() > 4 )
      {
         if( hash_match_g( stringNodeName, "value" ) == true ) { ePartType = ePartTypeValue; }
         else if( hash_match_g( stringNodeName, "where" ) == true ) { ePartType = ePartTypeWhere; }
         else if( hash_match_g( stringNodeName, "select" ) == true ) { ePartType = ePartTypeSelect; }
         else if( hash_match_g( stringNodeName, "returning" ) == true ) { ePartType = ePartTypeReturning; }
         else { continue; } // if node name is not value, where or select skip it
      }
      else { continue; }                                                      // if node name is too short to be value, where or select skip it

      if( ePartType >= ePartTypeReturning )
      {
         if( ePartType == ePartTypeReturning )
         {
            std::string string_ = node_.attribute( "name" ).as_string();      // Field name 
            AddProperty( "returning", string_ );
            if( string_.empty() == false ) {}
         }
         continue;
      }


      argumentsField.clear();

      argumentsField.append( "part_type", uint32_t( ePartType ) );            // type of value query part

      // get name attribute
      std::string string_ = node_.attribute( "name" ).as_string();            // Field name 
      if( string_.empty() == true ) { string_ = node_.attribute( "column" ).as_string(); }// try to get column name as fallback
      if( string_.empty() == false ) { argumentsField["column"] = string_; }

      string_ = node_.attribute( "alias" ).as_string();                       // Field alias
      if( string_.empty() == false ) { argumentsField["alias"] = string_; }

      string_ = node_.attribute( "type" ).as_string();                        // type if value should be converted
      if( string_.empty() == false ) { argumentsField["type"] = string_; }

      string_ = node_.attribute( "value" ).as_string(); // get value          // try to get value from attribute first
      if( string_.empty() == false ) { argumentsField["value"] = string_; }
      else
      {                                                                       // if value is not in attribute try to get it from node value
         string_ = node_.child_value();                                       // @TODO: need to have logick for utf8 or ascii
         if( string_.empty() == false ) { argumentsField["value"] = string_; }
      }

      string_ = node_.attribute( "table" ).as_string();                       // get table name
      if( string_.empty() == false ) { argumentsField["table"] = string_; }
      else if( stringTable.empty() == false ) { argumentsField["table"] = stringTable; } // if table name is not in attribute try to get it from parent node attribute

      AddValue( argumentsField );
   }

   return { true, "" };
}

void CRENDERSql::AddValue( std::string_view stringName, gd::variant_view variantviewValue )
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsField(buffer_);
   argumentsField.append_argument( "column", stringName ); // column name
   argumentsField.append_argument( "value", variantviewValue);
   AddValue( argumentsField );
}

/** --------------------------------------------------------------------------
 * @brief Add new field to be used in  SQL query that is generated.
 *
 * @param argumentsField information about the field to add.
 */
void CRENDERSql::AddValue( const gd::argument::arguments& argumentsField )
{                                                                                                  assert( m_pcolumnsField_s != nullptr );
   auto uRow = m_tableField.row_add_one();
   m_tableField.cell_set( uRow, 0u, uint32_t(uRow + 1) );                     // set key value

   for( auto [key_, value_] : argumentsField.named() )
   {                                                                                               assert( m_pcolumnsField_s->find_index( key_ ) != -1 );
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

/** -------------------------------------------------------------------------- FindColumnName
 * @brief Finds the row index in table if information for column name is present.
 *
 * @param stringName The name of the column to find.
 * @return int64_t The index of the column if found, or -1 if not found.
 */
int64_t CRENDERSql::FindRowForColumnName( std::string_view stringName ) const
{
   gd::variant_view name_( stringName );
   for( std::size_t uRow = 0; uRow < m_tableField.get_row_count(); ++uRow )
   {
      auto column_ = m_tableField.cell_get_variant_view( uRow, eColumnFieldColumn, gd::table::tag_not_null{}); // get column name as variant view
      if( column_.as_string_view() == name_.as_string_view() ) return uRow;
   }
   return -1;
}

void CRENDERSql::AddValues( const gd::argument::arguments& argumentsField )
{
   for( auto [key_, value_] : argumentsField.named() )
   {
      auto uRow = m_tableField.row_add_one();                                 // Add row to store field information
      m_tableField.cell_set( uRow, eColumnFieldColumn, key_, gd::table::tag_spill{});

      if( value_.is_text() == true ) 
      { 
         m_tableField.cell_set( uRow, eColumnFieldValue, value_.as_string_view(), gd::table::tag_spill{}); 
      }
      else
      {
         m_tableField.cell_set( uRow, eColumnFieldValue, value_.as_string_view(), gd::table::tag_spill{});
      }
   }
}

std::pair<bool, std::string> CRENDERSql::AddValues( std::string_view stringJson, gd::types::tag_json )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments argumentsField( buffer_ );

   auto result_ = gd::parse::json::parse_shallow_object_g( stringJson, argumentsField, false );
   if( result_.first == false ) { return result_; }
   AddValues( argumentsField );
   return { true, "" };
}

/** -------------------------------------------------------------------------- AddRecord
 * @brief Parses a JSON record and adds its fields to the internal table for SQL query generation.
 * 
 * This method processes a JSON object containing database record information with three main sections:
 * - **table**: The target table name
 * - **values**: Column-value pairs for INSERT/UPDATE operations (marked as `ePartTypeValue`)
 * - **where**: Column-value pairs for WHERE conditions (marked as `ePartTypeWhere`)
 * - **returning**: Optional RETURNING clause for PostgreSQL-style queries
 * 
 * The method extracts each field and converts it to the internal `arguments` format before
 * adding it to `m_tableField` via `AddValue()`. This allows building complex SQL statements
 * from structured JSON input.
 * 
 * **Expected JSON structure:**
 * ```json
 * {
 *   "table": "users",
 *   "values": { "name": "John", "age": 30 },
 *   "where": { "id": 1 },
 *   "returning": "*"
 * }
 * ```
 * 
 * @param stringJson JSON string containing the record data (table, values, where, returning)
 * @param tag_json Tag parameter indicating JSON parsing context (tag dispatch pattern)
 * @return std::pair<bool,std::string> Success status and error message. Returns `{true, ""}` on success,
 *         or `{false, error_description}` if JSON parsing fails or structure is invalid
 * 
 * @code
 * CRENDERSql renderSql;
 * auto result_ = renderSql.AddRecord( R"({"table":"users","values":{"name":"John"},"where":{"id":1}})", gd::types::tag_json{} );
 * if( result_.first == false ) {  handle error: result_.second  }
 * @endcode
 */
std::pair<bool,std::string> CRENDERSql::AddRecord( std::string_view stringJson, gd::types::tag_json )
{                                                                                                  assert( m_pcolumnsField_s != nullptr );
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
         arguments_.append( "part_type", uint32_t( ePartTypeValue ) ); // value part of query (insert and update queries)
         AddValue( arguments_ );
      }

      auto jsonWhere = jsonRecord["where"];
      for( const auto& itValue : jsonWhere.object_range() )
      {
         arguments_.clear();
         if( itValue.value().is_object() == true ) 
         { continue; 
         // @TODO: Manage objects in where part, for example for operators like greater than, less than, etc.
         } 
         else
         {
            arguments_.append( "table", stringTable );
            arguments_.append( "column", itValue.key() );
            arguments_.append_argument( "value", CONVERT::AsVariant( itValue.value() ) );
            arguments_.append( "part_type", uint32_t( ePartTypeWhere ) ); // where part of query (select, update and delete queries)
         }
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

/** --------------------------------------------------------------------------
 * @brief Prepares the SQL query by validating and transforming the data in the table field.
 * 
 * This method iterates through the rows in `m_tableField`, retrieves the table and column information,
 * and uses the database metadata to determine the data type of each column. It then updates the
 * "type" field in `m_tableField` for each column accordingly. This preparation step ensures that
 * the data is correctly typed before generating the final SQL query.
 * 
 * @return std::pair<bool, std::string> A pair containing a boolean indicating success and a string containing an error message if preparation fails.
 */
std::pair<bool, std::string> CRENDERSql::Prepare()
{                                                                                                  assert( m_pdocument != nullptr ); assert( m_tableField.size() > 0 ); // at least one field should be added before preparing query
   std::array<std::byte, 256> buffer_;
   const META::CDatabase* pdatabase_ = m_pdocument->DATABASE_Get();

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      //uint32_t uType = itRow.cell_get_variant_view( "part_type" );

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
   m_tableField.cell_set( uRow, stringName, stringValue, gd::table::tag_spill{}, gd::table::tag_convert{} );
}

void CRENDERSql::Add( std::string_view stringName, gd::variant_view variantviewValue )
{
   std::string stringValue = variantviewValue.as_string();
   Add( stringName, std::string_view( stringValue ) );
}

/** --------------------------------------------------------------------------
 * @brief Counts the number of rows in the field table that match the specified part type.
 * @param ePartType The part type to search for and count in the table.
 * @return The number of rows with a matching part type.
 */
std::size_t CRENDERSql::CountPartType( enumPartType ePartType ) const
{
#ifndef NDEBUG
   std::string stringTable_d = gd::table::arguments::debug::print( m_tableField );
#endif // NDEBUG
   std::size_t uCount = 0;
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      uint32_t uType = itRow.cell_get_variant_view(eColumnFieldPartType).as_uint();
      if( uType == static_cast<uint32_t>( ePartType ) ) { ++uCount; }
   }
   return uCount;
}

void CRENDERSql::SetColumnValue( std::string_view stringName, gd::variant_view variantviewValue )
{
   auto iColumn = m_tableField.column_find_index( stringName );
   if( iColumn < 0 ) return;
   m_tableField.column_fill( (unsigned)iColumn, variantviewValue, gd::table::tag_convert{} );
}

/** ------------------------------------------------------------------------- FillColumn
 * Fill cells in table where there no value set
 */
void CRENDERSql::FillColumn( enumColumnField eColumnField, gd::variant_view variantviewValue )
{
   // ## Loop rows in table and fill cells in column with value if cell is empty
   for( uint64_t uRow = 0u, uMax = m_tableField.size(); uRow < uMax; ++uRow )
   {
      auto v_ = m_tableField.cell_get_variant_view( uRow, (unsigned)eColumnField, gd::table::tag_not_null{} );
      if( v_.is_null() == false ) continue;                                   // if cell is not empty skip it

      m_tableField.cell_set( uRow, eColumnField, variantviewValue, gd::table::tag_spill{});
   }
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
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_( buffer_ );

   std::string stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();

   gd::sql::query queryInsert( m_eSqlDialect, gd::sql::eSqlInsert, stringTable, gd::sql::tag_table{});

   // ## Extract column names and values from table field and add to query ...
   
   std::vector< std::pair<uint32_t, gd::variant_view> > vectorValue;
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      arguments_.clear();

      std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
      //queryInsert.field_add( stringColumn );                                  // add column to query
      uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
      auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
      arguments_.append( { { "name", stringColumn }, { "value", value_ }, { "type", uType } }, gd::types::tag_view{});
      queryInsert.field_add( arguments_, gd::sql::tag_arguments{} );       // add column to query
   }

   // ## Generate insert query ..............................................

   std::string stringInsertSql;
	stringInsertSql += queryInsert.sql_get( gd::sql::eSqlInsert );

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

   gd::sql::query queryUpdate( m_eSqlDialect, gd::sql::eSqlUpdate, stringTable, gd::sql::tag_table{});

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      arguments_.clear();

      uint32_t uType = itRow.cell_get_variant_view( "part_type" );
      if( uType == uint32_t( ePartTypeValue ) )
      {
         std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
         uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
         auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
         arguments_.append( { { "name", stringColumn }, { "value", value_ }, { "type", uType } }, gd::types::tag_view{});
         queryUpdate.field_add( arguments_, gd::sql::tag_arguments{} );       // add column to query
      }
      else if( uType == uint32_t( ePartTypeWhere ) )
      {
         std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
         uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
         auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
         uint32_t uOperator = itRow.cell_get_variant_view("operator").as_uint();
         // ## Generate condition, name, value, type and operator are needed to generate condition for where part of query
         arguments_.append( { { "name", stringColumn }, { "value", value_ }, { "type", uType }, { "operator", uOperator } }, gd::types::tag_view{});
         queryUpdate.condition_add( arguments_, gd::sql::tag_arguments{} );   // add condition to query
      }
   }


   // ## Generate insert query ..............................................

   std::string stringUpdateSql;
	stringUpdateSql += queryUpdate.sql_get( gd::sql::eSqlUpdate );

   if( stringQuery.empty() == true ) stringQuery = std::move( stringUpdateSql );
   else stringQuery += "\n\n" + stringUpdateSql;

   return { true, "" };
}

std::pair<bool, std::string> CRENDERSql::ToSqlDelete( std::string& stringQuery )
{
   // ## Generate delete query ..............................................
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_( buffer_ );

   std::string stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();

   gd::sql::query queryDelete( m_eSqlDialect, gd::sql::eSqlDelete, stringTable, gd::sql::tag_table{});

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      arguments_.clear();

      uint32_t uType = itRow.cell_get_variant_view( "part_type" );
      if( uType == uint32_t( ePartTypeWhere ) )
      {
         std::string stringColumn = itRow.cell_get_variant_view( "column", gd::table::tag_not_null{}).as_string();
         uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
         auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
         uint32_t uOperator = itRow.cell_get_variant_view("operator").as_uint();
         // ## Generate condition, name, value, type and operator are needed to generate condition for where part of query
         arguments_.append( { { "name", stringColumn }, { "value", value_ }, { "type", uType }, { "operator", uOperator } }, gd::types::tag_view{});
         queryDelete.condition_add( arguments_, gd::sql::tag_arguments{} );   // add condition to query
      }
   }

   // ## Generate insert query ..............................................

   std::string stringDeleteSql;
	stringDeleteSql += queryDelete.sql_get( gd::sql::eSqlDelete );

   if( stringQuery.empty() == true ) stringQuery = std::move( stringDeleteSql );
   else stringQuery += "\n\n" + stringDeleteSql;

   return { true, "" };
}

std::pair<bool, std::string> CRENDERSql::ToSql( std::string_view stringType, std::string& stringQuery )
{                                                                                                  assert( stringType.empty() == false );
   char iType = stringType[0];
   switch( iType )
   {
   case 'i': case 'I':                                                        // insert query
      return ToSqlInsert( stringQuery );
   case 'u': case 'U':                                                        // update query
      return ToSqlUpdate( stringQuery );
   case 'd': case 'D':                                                        // delete query
      return ToSqlDelete( stringQuery );
   default:
      return {false, "Invalid query type"};
   }

   return { false, "" };
}

std::pair<bool, std::string> CRENDERSql::Validate( gd::argument::arguments argumentsValue, unsigned* puFound ) const
{  
   unsigned uRequired = 0;
      
   // ## match values against columns in table ...............................
   for( auto [key_, value_] : argumentsValue.named() )
   {
      bool bFound = m_tableField.column_exists( key_ );  // if column is found
      if( bFound == true )
      {
         char iChar = key_[0];
         switch( iChar )
         {
         case 'k': uRequired |= eColumnFlagKey; break;
         case 's': uRequired |= eColumnFlagSchema; break;
         case 't': { if( key_ == "table" ) uRequired |= eColumnFlagTable; else uRequired |= eColumnFlagType; } break;
         case 'c': uRequired |= eColumnFlagColumn; break;
         case 'a': uRequired |= eColumnFlagAlias; break;
         case 'v': uRequired |= eColumnFlagValue; break;
         case 'p': uRequired |= eColumnFlagPartType; break;
         }
      }
      else { return { false, "Invalid column name: " + std::string( key_ ) }; }
   };

   if( puFound != nullptr ) { *puFound = uRequired; }

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
