// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.cpp]

#include <cassert>
#include <sstream>
#include <string_view>

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "gd/gd_arguments_io.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "gd/gd_sql_query.h"
#include "gd/gd_sql_query_builder.h"


#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "gd/expression/gd_expression_glue_to_gd.h"

#include "gd/parse/gd_parse_json.h"

#include "../convert/CONVERTCore.h"

#include "../api/API_Base.h"
#include "../Document.h"

#include "RENDERSql.h"

namespace utility
{
   // @brief Converts a string representation of a SQL part type to its corresponding enum value.
   unsigned get_part_type_from_string( std::string_view stringPartType )
   {
      using namespace gd::types::detail;
      unsigned uPartType = 0;
      if( gd::types::detail::hash_match_g( stringPartType, "select" ) == true ) { uPartType = CRENDERSql::ePartTypeSelect; }
      else if( gd::types::detail::hash_match_g( stringPartType, "value" ) == true ) { uPartType = CRENDERSql::ePartTypeValue; }
      else if( gd::types::detail::hash_match_g( stringPartType, "where" ) == true ) { uPartType = CRENDERSql::ePartTypeWhere; }
      else if( gd::types::detail::hash_match_g( stringPartType, "returning" ) == true ) { uPartType = CRENDERSql::ePartTypeReturning; }
      return uPartType;
   }

   // AddConditionToQuery
   std::pair<bool, std::string> add_condition_to_query( std::vector<gd::argument::arguments>& vectorCondition, gd::sql::query& queryAddTo  )
   {
      for( auto& argumentsCondition : vectorCondition )
      {                                                                                            assert( gd::sql::query::validate_condition_s( argumentsCondition ).first == true );
         // ## check for operator
         if( argumentsCondition.exists( "operator" ) == false ) { argumentsCondition.append( "operator", uint32_t( gd::sql::eOperatorEqual ) ); }
         else
         {
            auto v_ = argumentsCondition["operator"].as_variant_view();
            if( v_.is_string() == true )
            {
               auto eOperator = gd::sql::query::get_where_operator_number_s(v_.as_string_view());
               if( eOperator == gd::sql::eOperatorError ) { return { false, std::format( "Invalid operator: {}", v_.as_string_view() ) }; }

               argumentsCondition.set( "operator", eOperator );
            }
         }

         queryAddTo.condition_add( argumentsCondition, gd::sql::tag_arguments{} );
      }  

      return { true, "" };
   }
}

CRENDERSql::CRENDERSql( const CAPIContext* papicontext ) : m_papicontext( papicontext ), m_tableField( 8, gd::table::tag_full_meta{} ) 
{                                                                                                  assert( papicontext ); assert( papicontext->GetDocument() );
   const auto* pdocument_ = papicontext->GetDocument();
   if( pdocument_->DATABASE_Get() ) { m_eSqlDialect = gd::sql::enumSqlDialect( pdocument_->DATABASE_Get()->GetDialect() ); }
}

CRENDERSql::CRENDERSql( const CAPIContext* papicontext, uint64_t uStatementRow ): 
   m_papicontext( papicontext ), m_iRowStatement( int64_t(uStatementRow) ), m_tableField( 8, gd::table::tag_full_meta{} )
{                                                                                                  assert( papicontext ); assert( papicontext->GetDocument() );
   const auto* pdocument_ = papicontext->GetDocument();
   if( pdocument_->DATABASE_Get() ) { m_eSqlDialect = gd::sql::enumSqlDialect( pdocument_->DATABASE_Get()->GetDialect() ); }
}

void CRENDERSql::common_construct( const CRENDERSql& o ) 
{
   m_papicontext = o.m_papicontext;
   m_eSqlDialect = o.m_eSqlDialect;
   m_tableField = o.m_tableField;
   m_argumentsProperty = o.m_argumentsProperty;
}

void CRENDERSql::common_construct( CRENDERSql&& o ) noexcept 
{
   m_papicontext = o.m_papicontext;
   m_eSqlDialect = o.m_eSqlDialect;
   m_tableField = std::move(o.m_tableField);
   m_argumentsProperty = std::move(o.m_argumentsProperty);
}

inline const CDocument* CRENDERSql::GetDocument() const
{                                                                             assert( m_papicontext ); assert( m_papicontext->GetDocument() );
   return m_papicontext->GetDocument();
}


/** --------------------------------------------------------------------------
 * @brief Initializes the static data table structure used to hold response body parts for SQL field information.
 */
void CRENDERSql::Initialize()
{                                                                                                  assert( m_tableField.empty() == true && "Table field must be empty before initialization" );
   using namespace std::literals::string_view_literals;
   // Initialize datable that will hold response body parts

   // ## Magic Static (Meyers' Singleton)
   // The compiler guarantees thread-safe initialization exactly once.
   // This is typically faster than manual mutex locking.
   static gd::table::detail::columns* pcolumnsField_s = []() -> gd::table::detail::columns* {
      constexpr auto uSize = m_uMaxStringBufferLength_s;
      auto* p = new gd::table::detail::columns{};

      p->add( "uint32", 0, "key" );
      p->add( "uint32", 0, "meta" );                                          // meta information for column, fast access to column information, avoid to find it again
      p->add( "string", uSize, "schema" );                                    // schema for table field belongs to
      p->add( "string", uSize, "table" );                                     // name for table field belongs to
      p->add( "string", uSize, "name" );                                      // name for column in table
      p->add( "string", uSize, "alias" );                                     // alias for column in table
      p->add( "string", uSize * 2, "value" );                                 // value for column in table
      p->add( "uint32", 0, "type" );                                          // type of value
      p->add( "uint32", 0, "type_part" );                                     // part type of value, this same as sql part like select, where    
      p->add( "rstring", 0, "raw" );                                          // raw sql stamement for column
      p->add_reference();

      CRENDERSql::m_pcolumnsField_s = p; // assign to static member for use in other instances of CRENDERSql

                                                                                                   assert( m_pcolumnsField_s->find_index( "key"sv ) == eColumnFieldKey );
                                                                                                   assert( m_pcolumnsField_s->find_index( "meta"sv ) == eColumnFieldMeta );
                                                                                                   assert( m_pcolumnsField_s->find_index( "alias"sv ) == eColumnFieldAlias );
                                                                                                   assert( m_pcolumnsField_s->find_index( "type_part"sv ) == eColumnFieldPartType );
      return p;
   }();

   m_tableField.set_columns( m_pcolumnsField_s, gd::table::tag_static_columns{} );
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
         ePartType = static_cast<enumPartType>(utility::get_part_type_from_string( stringNodeName ));
         if( ePartType == ePartTypeUnknown ) { continue; }                    // if node name is not recognized as part type skip it )
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

      argumentsField.append( "type_part", uint32_t( ePartType ) );            // type of value query part

      // get name attribute
      std::string string_ = node_.attribute( "name" ).as_string();            // Field name 
      if( string_.empty() == true ) { string_ = node_.attribute( "name" ).as_string(); }// try to get column name as fallback
      if( string_.empty() == false ) { argumentsField["name"] = string_; }

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

      AddColumn( argumentsField );
   }

   return { true, "" };
}

void CRENDERSql::AddColumn( std::string_view stringName, gd::variant_view variantviewValue )
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsField(buffer_);
   argumentsField.append_argument( "name", stringName ); // column name
   argumentsField.append_argument( "value", variantviewValue);
   AddColumn( argumentsField );
}

/** --------------------------------------------------------------------------
 * @brief Add new field to be used in  SQL query that is generated.
 *
 * @param argumentsField information about the field to add.
 */
void CRENDERSql::AddColumn( const gd::argument::arguments& argumentsField )
{                                                                                                  assert( m_pcolumnsField_s != nullptr ); assert( argumentsField.exists( "name" ) == true ); 
#ifndef NDEBUG
   [[maybe_unused]] std::string stringValues_d = gd::argument::debug::print( argumentsField );
#endif // NDEBUG
   auto uRow = m_tableField.row_add_one();
   m_tableField.cell_set( uRow, 0u, uint32_t(uRow + 1) );                     // set key value

   for( auto [key_, value_] : argumentsField.named() )
   {                                                                                               assert( m_pcolumnsField_s->find_index( key_ ) != -1 );
      char iFirstChar = key_[0];
      if( iFirstChar != 't' )                                                 // not type or type_part (these are numbers)
      {
         if( value_.is_string() == true ) { Add( key_, value_.as_string_view() ); }
         else { Add( key_, value_ ); }
      }
      else if( key_ == "type" )                                                 
      { 
         if( value_.is_string() == true ) 
         { 
            // ## translate type name to type number .....................
            auto stringType = value_.as_string_view();
            if( stringType.length() <= 2 ) { continue; }
            uint32_t uType = gd::types::type_g( value_.as_string_view() );
            m_tableField.cell_set( uRow, eColumnFieldType, uType);
         }
         else { m_tableField.cell_set( uRow, eColumnFieldType, value_); }
      }
      else if( key_ == "type_part" )
      {
         if( value_.is_string() == true )
         {
            // ## translate part type name to part type number .....................
            auto stringPartType = value_.as_string_view();
            if( stringPartType.length() <= 4 ) { continue; }
            uint32_t uPartType = 0;
            if( gd::types::detail::hash_match_g( stringPartType, "select" ) == true ) { uPartType = ePartTypeSelect; }
            else if( gd::types::detail::hash_match_g( stringPartType, "value" ) == true ) { uPartType = ePartTypeValue; }
            else if( gd::types::detail::hash_match_g( stringPartType, "where" ) == true ) { uPartType = ePartTypeWhere; }
            else if( gd::types::detail::hash_match_g( stringPartType, "returning" ) == true ) { uPartType = ePartTypeReturning; }
            else { continue; } // if part type is not select, value, where or returning skip it
            m_tableField.cell_set( uRow, eColumnFieldPartType, uPartType );
         }
         else { m_tableField.cell_set( uRow, eColumnFieldPartType, value_ ); }
      }
      else
      {
         if( value_.is_string() == true ) { Add( key_, value_.as_string_view() ); }
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
std::pair<bool,std::string> CRENDERSql::AddColumn( std::string_view stringJson, gd::types::tag_json )
{
   if( stringJson.empty() == true ) { return { true, "empty json" }; }
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_(buffer_);
   
   auto result_ = gd::parse::json::parse_shallow_object_g( stringJson, arguments_, false );
   if( result_.first == false ) { return result_; }
   
   // check for value
   if( arguments_.exists("value") == false ) { return {false, "missing value"}; }

   AddColumn( arguments_ );

   return {true, ""};
}

std::pair<bool, std::string> CRENDERSql::SetColumn( std::string_view stringColumn, const gd::argument::arguments& argumentsColumn )
{
   auto uRow = FindRowForColumnName( stringColumn );
   if( uRow != -1 ) { SetColumn( uRow, argumentsColumn ); return {true, ""}; }
   return {false, std::format("column '{}' not found", stringColumn) };
}

void CRENDERSql::SetColumn( uint64_t uRow, const gd::argument::arguments& argumentsColumn )
{                                                                                                  assert( uRow < m_tableField.get_row_count() );
   for( auto [key_, value_] : argumentsColumn.named()  )
   {                                                                                               assert( m_pcolumnsField_s->find_index( key_ ) != -1 );
      char iFirstChar = key_[0];
      if( iFirstChar != 't' )                                                 // not type or type_part (these are numbers)
      {
         if( value_.is_string() == true ) { m_tableField.cell_set( uRow, key_, value_.as_string_view(), gd::table::tag_spill{} ); }
         else { m_tableField.cell_set( uRow, key_, value_, gd::table::tag_spill{} ); }
      }
      else if( key_ == "type" )
      {
         if( value_.is_string() == true )
         {
            // ## translate type name to type number .....................
            auto stringType = value_.as_string_view();
            if( stringType.length() <= 2 ) { continue; }
            uint32_t uType = gd::types::type_g( value_.as_string_view() );
            m_tableField.cell_set( uRow, eColumnFieldType, uType, gd::table::tag_spill{} );
         }
         else { m_tableField.cell_set( uRow, eColumnFieldType, value_, gd::table::tag_spill{} ); }
      }
      else if( key_ == "type_part" )
      {
         if( value_.is_string() == true )
         {
            // ## translate part type name to part type number .....................
            auto stringPartType = value_.as_string_view();
            if( stringPartType.length() <= 4 ) { continue; }
            uint32_t uPartType = utility::get_part_type_from_string( stringPartType );
            m_tableField.cell_set( uRow, eColumnFieldPartType, uPartType, gd::table::tag_spill{} );
         }
         else { m_tableField.cell_set( uRow, eColumnFieldPartType, value_, gd::table::tag_spill{} ); }
      }
   }
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
      auto column_ = m_tableField.cell_get_variant_view( uRow, eColumnFieldName, gd::table::tag_not_null{}); // get column name as variant view
      if( column_.as_string_view() == name_.as_string_view() ) return uRow;
   }
   return -1;
}

/** -------------------------------------------------------------------------- Find
 * @brief Find the first row that matches all named column/value filters in `argumentsColumn`.
 *
 * Matching strategy:
 * 1. Uses the first named pair in `argumentsColumn` as an initial pre-filter.
 * 2. For each candidate row, verifies all remaining named pairs.
 * 3. Returns immediately on the first full match.
 *
 * **important**: If the first filter column cannot be resolved in `m_tableField`,
 * the method returns `-1`.
 *
 * @param argumentsColumn Named filters where key = column name and value = expected value.
 * @return int64_t Matching row index, or `-1` when no row satisfies all filters.
 */
int64_t CRENDERSql::Find( const gd::argument::arguments& argumentsColumn ) const
{                                                                                                  assert( argumentsColumn.empty() == false ); 
   // ## Get first value and column to have a match for row
   auto [key_, compare_] = *argumentsColumn.named().begin();
   int32_t iColumnIndex = m_tableField.column_find_index( key_ );                                  assert( iColumnIndex != -1 ); // column must be found to have a match for row)
   if( iColumnIndex == -1 ) { return -1; } // if column is not found return -1

   for( std::size_t uRow = 0; uRow < m_tableField.get_row_count(); ++uRow )
   {
      bool bMatch = true;

      gd::variant_view cell_ = m_tableField.cell_get_variant_view( uRow, iColumnIndex, gd::table::tag_not_null{} ); // get value
      if( cell_.compare_convert( compare_ ) == false ) { continue; } // if first value does not match skip to next row )


      //for( auto [key_, value_] : argumentsColumn.named() + 1 )
      for( auto it = argumentsColumn.named_begin() + 1; it != argumentsColumn.named_end(); ++it )
      {
         auto [key_, value_] = *it;                                                                assert( m_tableField.column_find_index( key_ ) != -1 ); // column must be found to have a match for row)
         auto cellValue_ = m_tableField.cell_get_variant_view( uRow, key_, gd::table::tag_not_null{} ); // get column name as variant view
         if( cellValue_.compare_convert( value_ ) == false ) { bMatch = false; break; }
      }
      
      if( bMatch == true ) { return uRow; }
   }
   return -1; 
}


void CRENDERSql::AddValues( const gd::argument::arguments& argumentsField )
{
   std::string_view stringTable;

   if( m_iRowStatement != -1 )
   {
      const uint64_t uRowStatement = uint64_t( m_iRowStatement );
      const META::CQueries* pqueries = GetDocument()->QUERIES_Get();                                 assert( pqueries != nullptr );
      stringTable = pqueries->GetTable( uRowStatement );
   }


   for( auto [key_, value_] : argumentsField.named() )
   {
      auto uRow = m_tableField.row_add_one();                                 // Add row to store field information
      m_tableField.cell_set( uRow, eColumnFieldName, key_, gd::table::tag_spill{});

      if( value_.is_text() == true ) 
      { 
         m_tableField.cell_set( uRow, eColumnFieldValue, value_.as_string_view(), gd::table::tag_spill{}); 
      }
      else
      {
         m_tableField.cell_set( uRow, eColumnFieldValue, value_.as_string(), gd::table::tag_spill{});
      }

      if( stringTable.empty() == false ) { m_tableField.cell_set( uRow, eColumnFieldTable, stringTable, gd::table::tag_spill{} ); }
   }
}

std::pair<bool, std::string> CRENDERSql::AddColumnValues( std::string_view stringJson, gd::types::tag_json )
{                                                                                                  assert( GetDocument() != nullptr );
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments argumentsField( buffer_ );

   auto result_ = gd::parse::json::parse_shallow_object_g( stringJson, argumentsField, false );
   if( result_.first == false ) { return result_; }
   AddValues( argumentsField );
   return { true, "" };
}

/** -------------------------------------------------------------------------- AddColumns
 * @brief Parses JSON column input and prepares it for column metadata processing.
 * 
 * Attempts to parse `stringJson` with columns and add each column to the internal table.
 * 
 * Valid formats are: single column [table,name,value,{part}] where part is optional.
 * Multiple columns can be provided as an array of such objects: [ [table,name,value,{part}], [table,name,value,{part}] ].
 * Named fiels in array use [{table: "schema.table", name: "column_name", value: "value", type_part: "select|value|where|returning"}]
 * 
 * Uses `gd::argument::arguments` passed to void AddColumn as dto for column and add value with keys "table", "name", "value", "type_part".
 * 
 * - Uses a local `gd::argument::arguments` buffer (`arguments_`) as staging storage.
 * - Catches `jsoncons::json_exception` and returns `{ false, error }` on parse failure.
 * - Returns `{ true, "" }` when JSON parsing succeeds.
 * 
 * **Note:** This implementation currently validates/parses input only; column extraction logic is not yet applied.
 * 
 * @param stringJson JSON payload containing column data.
 * @return std::pair<bool, std::string> Parse status and optional error message.
 */
std::pair<bool, std::string> CRENDERSql::AddColumns( std::string_view stringJson, gd::types::tag_json )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_(buffer_);
   std::pair<bool, std::string> result_;

   // ## Lambda to add column from array format [table,name,value,{part},operator]
   auto add_array_ = [&]( const jsoncons::json& jsonColumn, std::size_t uColumnIndex ) -> std::pair<bool, std::string>
   {                                                                                               assert( jsonColumn.is_array() == true );
      if( ValidateIsAllStrings_s( jsonColumn ) == false ) { return { false, "all elements must be strings" }; }

      ToArgumentsFromArray_s( jsonColumn, arguments_ );                       // convert array to arguments with keys "table", "name", "value", "type_part" and "operator"

      AddColumn( arguments_ );
      return { true, "" };
   };

   // ## Lambda to add column from object format {table: "schema.table", name: "column_name", value: "value", type_part: "select|value|where|returning"}
   auto add_object_ = [&]( const jsoncons::json& jsonColumn, std::size_t uColumnIndex ) -> std::pair<bool, std::string>
   {                                                                                               assert( jsonColumn.is_object() == true );
      if( ValidateIsAllStrings_s( jsonColumn ) == false ) { return { false, "all elements must be strings" }; }

      ToArgumentsFromObject_s( jsonColumn, arguments_ );                       // convert array to arguments with keys "table", "name", "value", "type_part" and "operator"

      AddColumn( arguments_ );
      return { true, "" };
   };


   try 
   {
      // ## parse columns, this has to be an array
      jsoncons::json jsonArray = jsoncons::json::parse(stringJson);          // parse information about columns

      if( jsonArray.is_array() == false ) { return { false, "must be an array" }; }

      // ## Check if it's an array of arrays (multiple columns) or a single array (one column)
      if( jsonArray.size() > 0 && jsonArray[0].is_array() == false )
      {
         return add_array_( jsonArray, 0 );                                   
      }

      // ## iterate rows in array or if only one array then add it as one column
      for( std::size_t u = 0; u < jsonArray.size(); ++u )
      {
         const jsoncons::json& jsonRow = jsonArray[u];
         if( jsonRow.is_array() == false ) { result_ = add_array_( jsonRow, u ); }
         else if( jsonRow.is_object() == true ) { result_ = add_object_( jsonRow, u ); }
         
      }
   }
   catch( jsoncons::json_exception& e )
   {
      std::string stringError = e.what();
      return {false, stringError};
   }
   
   return {true, ""};
}

/// @brief Adds a condition to the internal list of conditions for SQL query generation.
void CRENDERSql::AddCondition( const gd::argument::arguments& argumentsCondition )
{                                                                                                  assert( gd::sql::query::validate_condition_s( argumentsCondition ).first == true );
   m_vectorCondition.push_back( argumentsCondition );
}

/// @brief Adds a condition to the internal list of conditions for SQL query generation using move semantics.
void CRENDERSql::AddCondition( gd::argument::arguments&& argumentsCondition )
{                                                                                                  assert( gd::sql::query::validate_condition_s( argumentsCondition ).first == true );
   m_vectorCondition.push_back( std::move(argumentsCondition) );
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
         arguments_.append( "name", itValue.key() );
         arguments_.append_argument( "value", CONVERT::AsVariant( itValue.value() ) );
         arguments_.append( "type_part", uint32_t( ePartTypeValue ) ); // value part of query (insert and update queries)
         AddColumn( arguments_ );
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
            arguments_.append( "name", itValue.key() );
            arguments_.append_argument( "value", CONVERT::AsVariant( itValue.value() ) );
         }
         AddCondition( arguments_ );
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

std::pair<bool, std::string> EXPRESSION_GetArgument_s( const std::vector<gd::expression::value>& vectorArgument, gd::expression::value* pvalueReturn );
std::pair<bool, std::string> EXPRESSION_Exists_s( const std::vector<gd::expression::value>& vectorArgument, gd::expression::value* pvalueReturn );

// Array of MethodInfo for visual studio operations
const gd::expression::method pmethodSource_g[] = {
   { (void*)&EXPRESSION_Exists_s, "exists", 2, 1 },
   { (void*)&EXPRESSION_GetArgument_s, "get_argument", 2, 1 },
};

const size_t uMethodSourceSize_g = sizeof(pmethodSource_g) / sizeof(gd::expression::method);


std::pair<bool, std::string> CRENDERSql::Preprocess( std::string_view stringSqlTemplate )
{
   // ## Test for preparsing, find "{??" to check if there are tags to process
   auto position_ = stringSqlTemplate.find( "{??" );
   if( position_ != std::string::npos ) 
   {
      gd::argument::shared::arguments argumentsValues; // hold the values for the arguments that can be used in the expression evaluation
      ToArguments( argumentsValues );                                        // convert the table field to arguments for expression evaluation
      std::string stringNew = std::string(stringSqlTemplate);
      
      // ## prepare runtime for expression evaluation, add methods and variables
      using namespace gd::expression;
      gd::expression::runtime runtime_;
      runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""}); // global scope
      runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string( "str" ) } ); // str scope for string operations
      runtime_.add( { (unsigned)uMethodSourceSize_g, pmethodSource_g, std::string("args")});
      runtime_.set_variable( "args", std::pair<const char*, void*>("args", &argumentsValues)); // set source variable to the expression source

      // ## callback for expression evaluation, this will be called for each expression found in the string, and will return the result of the expression to replace in the string
      auto callback_ = [&]( const std::string_view& stringExpression, bool* pbError ) -> std::string
      {
         std::vector<gd::expression::token> vectorToken;
         std::pair<bool, std::string> result_ = gd::expression::token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});
         if( result_.first == false && pbError != nullptr ) { *pbError = true; return result_.second; }

         std::vector<token> vectorPostfix;
         vectorPostfix.reserve( vectorToken.size() );
         result_ = gd::expression::token::compile_s(vectorToken, vectorPostfix, tag_postfix{});
         if( result_.first == false && pbError != nullptr ) { *pbError = true; return result_.second; }

         std::vector<gd::expression::value> vectorReturn;
         result_ = gd::expression::token::calculate_s(vectorPostfix, &vectorReturn, runtime_);
         if( result_.first == false && pbError != nullptr ) { *pbError = true; return result_.second; }

         std::string stringResult;
         if( vectorReturn.size() > 0 ) { stringResult = vectorReturn[0].as_string(); }

         return stringResult; // return empty variant if not found
      };


      bool bError = false;
      auto stringResult = gd::sql::replace_g( stringNew, argumentsValues, callback_, &bError, gd::sql::tag_preprocess{} ); 
      if( bError == true ) { return { false, "Error during SQL preprocessing, sql: " + std::string(stringSqlTemplate) }; }

      stringNew.clear();
      auto result_ = gd::sql::replace_g( stringResult, argumentsValues, stringNew, gd::sql::tag_brace{} ); 
      if( result_.first == false ) { return result_; }
      return { true, std::move(stringNew) };
   }

   return { true, "" };
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
std::pair<bool, std::string> CRENDERSql::Prepare()                                                // @CRITICAL [tag: type, column, sql] [description: update types for each field from metadata about fileds]
{                                                                                                  assert( GetDocument() != nullptr ); 
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments argumentsFind( buffer_ );
   const META::CDatabase* pdatabase_ = GetDocument()->DATABASE_Get();

   if( m_iRowStatement >= 0 )
   {
      const uint64_t uRowStatement = uint64_t( m_iRowStatement );
      // ## Prepare table information for fields where this is not done ......

      const META::CQueries* pqueries = GetDocument()->QUERIES_Get();                                 assert( pqueries != nullptr );
      auto stringTable = pqueries->GetTable( uRowStatement );
      if( stringTable.empty() == false )
      {
         FillColumn( eColumnFieldTable, stringTable );                       // fill table name for fields where this is not set @TODO [summary: need more flexibility, now only one table]
      }
   }

   // ## Loop through rows fields and get type for field to set type.
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      argumentsFind.clear();
      std::string stringTable = itRow.cell_get_variant_view( "table", gd::table::tag_not_null{}).as_string();
      if( stringTable.empty() == true )                                       // No table defaults to type 0, no formating is done, just a value
      { 
         itRow.cell_set( "type", 0, gd::table::tag_convert{} ); 
         continue; 
      } 
      std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{}).as_string();

      
      argumentsFind.append( { {"table", stringTable}, {"column", stringName} });
      int64_t iRow = pdatabase_->Column_FindRow( argumentsFind );

      if( iRow > 0 )
      {
         uint32_t uType = pdatabase_->Column_GetType( iRow );
         itRow.cell_set( eColumnFieldType, uType );                           // set type for column in table field
         itRow.cell_set( eColumnFieldMeta, static_cast<uint32_t>(iRow) );     // set column meta row
      }
   }

   // ## Loop conditions and update type
   for( auto& argumentsCondition : m_vectorCondition )
   {
      argumentsFind.clear();
      std::string stringTable = argumentsCondition["table"].as_string();
      std::string stringColumn = argumentsCondition["name"].as_string();
      argumentsFind.append( { {"table", stringTable}, {"column", stringColumn} } );
      int64_t iRow = pdatabase_->Column_FindRow( argumentsFind );                                  assert( iRow >= 0 && "Developer error because this should not assert" );
      if( iRow > 0 )
      {
         uint32_t uType = pdatabase_->Column_GetType( iRow );
         argumentsCondition.set( "type", uType );                             // set type for column in condition
         argumentsCondition.set( "meta", static_cast<uint32_t>( iRow ) );     // set meta row for column in condition
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

std::pair<bool, std::string> CRENDERSql::Query_AddFields( gd::sql::query* pquery )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_( buffer_ );

   //if( pquery->table_size() == 0 ) { pquery->table_add( "default_table" ); }

   std::vector< std::pair<uint32_t, gd::variant_view> > vectorValue;
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      arguments_.clear();
      std::string_view stringTable = itRow.cell_get_variant_view( eColumnFieldTable, gd::table::tag_not_null{}).as_string_view();
      std::string_view stringName = itRow.cell_get_variant_view( eColumnFieldName, gd::table::tag_not_null{}).as_string_view();
      //queryInsert.field_add( stringName );                                  // add column to query
      uint32_t uType = itRow.cell_get_variant_view(eColumnFieldType).as_uint();
      auto value_ = itRow.cell_get_variant_view(eColumnFieldValue, gd::table::tag_not_null{});
      arguments_.append( { { "name", stringName }, { "value", value_ }, { "type", uType } }, gd::types::tag_view{});
      if( stringTable.empty() == true )
      {
         pquery->field_add( arguments_, gd::sql::tag_arguments{} );           // add column to query
      }
      else
      {
         int32_t iTableKey = pquery->table_get_key( stringTable );            // get the key for the table to use in field and condition parts
         if( iTableKey == -1 )
         {
            auto ptable_ = pquery->table_add( stringTable );                  // add table to query if not added yet
            iTableKey = ptable_->get_key();                                   // get the key for the table to use in field and condition parts
         }
         pquery->field_add( iTableKey, arguments_, gd::sql::tag_arguments{} );  // add column to query with table key
      }
   }

   return { true, "" };
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
         result_ = ToSqlSelect( stringQuery );
         break;
      case eSqlQueryTypeCount:
         break;
      default:
         return {false, "Invalid query type"};
   }
   return result_;
}

std::pair<bool, std::string> CRENDERSql::ToSqlSelect( std::string& stringQuery )
{
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments arguments_( buffer_ );

   std::string stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();
   gd::sql::query querySelect( m_eSqlDialect );

   querySelect.table_add( stringTable );

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   { 
      arguments_.clear();
      uint32_t uTableKey = querySelect.table_get_key(); // get the key for the table field or conditions is added for

      gd::variant_view variantviewTable = itRow.cell_get_variant_view( eColumnFieldTable, gd::table::tag_not_null{});
      if( variantviewTable.is_char_string() == true ) 
      { 
         // ## check if table is added to query, if not add it with alias if alias is present
         int32_t iTableKey = querySelect.table_get_key( variantviewTable.as_string_view() ); 
         if( iTableKey == -1 )
         {
            auto ptable_ = querySelect.table_add( variantviewTable.as_string_view() );
            uTableKey = ptable_->get_key();                                   // get the key for the table to use in field and condition parts
         }
         else { uTableKey = static_cast<uint32_t>( iTableKey ); }               // set the key for the table to use in field and condition parts
      }
         
         

      std::string_view stringColumn = itRow.cell_get_variant_view( eColumnFieldName, gd::table::tag_not_null{}).as_string_view();
      arguments_.append( "name", stringColumn );

      uint32_t uPartType = itRow.cell_get_variant_view( eColumnFieldPartType ).as_uint();
      if( uPartType == uint32_t( ePartTypeWhere ) )
      {
         auto v_ = itRow.cell_get_variant_view( eColumnFieldRaw );
         if( v_.is_char_string() == true ) arguments_.append( "raw", v_.as_string_view() );
         v_ = itRow.cell_get_variant_view( eColumnFieldValue, gd::table::tag_not_null{} );
         if( v_.is_null() == false ) arguments_.append_argument( std::string_view("value"), v_, gd::types::tag_view{} );

         querySelect.condition_add( uTableKey, arguments_, gd::sql::tag_arguments{} );   // add condition to query
         continue;
      }
      else
      {
         auto v_ = itRow.cell_get_variant_view( eColumnFieldAlias, gd::table::tag_not_null{});
         if( v_.is_char_string() == true ) arguments_.append( "alias", v_.as_string_view() );
         v_ = itRow.cell_get_variant_view( eColumnFieldRaw );
         if( v_.is_char_string() == true ) arguments_.append( "raw", v_.as_string_view() );

         querySelect.field_add( uTableKey, arguments_, gd::sql::tag_arguments{} );       // add column to query
      }
   }

   // ## Add conditions to query if there are any ............................
   if( m_vectorCondition.empty() == false )
   {
      auto [success, error] = utility::add_condition_to_query( m_vectorCondition, querySelect );
      if( success == false ) return { false, error };
   }

   std::string stringSelectSql;
   stringSelectSql += querySelect.sql_get( gd::sql::eSqlSelect );

   if( stringQuery.empty() == true ) stringQuery = std::move( stringSelectSql );
   else stringQuery += "\n\n" + stringSelectSql;

   return { true, "" };
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

      std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{}).as_string();
      //queryInsert.field_add( stringName );                                  // add column to query
      uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
      auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
      arguments_.append( { { "name", stringName }, { "value", value_ }, { "type", uType } }, gd::types::tag_view{});
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

      uint32_t uType = itRow.cell_get_variant_view( "type_part" );
      if( uType == uint32_t( ePartTypeValue ) )
      {
         std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{}).as_string();
         uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
         auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
         arguments_.append( { { "name", stringName }, { "value", value_ }, { "type", uType } }, gd::types::tag_view{});
         queryUpdate.field_add( arguments_, gd::sql::tag_arguments{} );       // add column to query
      }
      else if( uType == uint32_t( ePartTypeWhere ) )
      {
         std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{}).as_string();
         uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
         auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
         uint32_t uOperator = itRow.cell_get_variant_view("operator").as_uint();
         // ## Generate condition, name, value, type and operator are needed to generate condition for where part of query
         arguments_.append( { { "name", stringName }, { "value", value_ }, { "type", uType }, { "operator", uOperator } }, gd::types::tag_view{});
         queryUpdate.condition_add( arguments_, gd::sql::tag_arguments{} );   // add condition to query
      }
   }

   // ## Add conditions to query if there are any ............................
   if( m_vectorCondition.empty() == false )
   {
      auto [bOk, stringError] = utility::add_condition_to_query( m_vectorCondition, queryUpdate );
      if( bOk == false ) return { false, stringError };
   }



   // ## Generate update query ..............................................

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

   std::string stringTable;

   // ## Extract table name ..................................................   
   if( m_tableField.get_row_count() > 0 ) stringTable = m_tableField.cell_get_variant_view(0u, "table", gd::table::tag_not_null{}).as_string();
   if( stringTable.empty() == true && m_vectorCondition.empty() == false ) { stringTable = m_vectorCondition[0]["table"].as_string(); }

   if( stringTable.empty() == true ) { return { false, "Table name not found" }; }

   gd::sql::query queryDelete( m_eSqlDialect, gd::sql::eSqlDelete, stringTable, gd::sql::tag_table{});

   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      arguments_.clear();

      uint32_t uType = itRow.cell_get_variant_view( "type_part" );
      if( uType == uint32_t( ePartTypeWhere ) )
      {
         std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{}).as_string();
         uint32_t uType = itRow.cell_get_variant_view("type").as_uint();
         auto value_ = itRow.cell_get_variant_view("value", gd::table::tag_not_null{});
         uint32_t uOperator = itRow.cell_get_variant_view("operator").as_uint();
         // ## Generate condition, name, value, type and operator are needed to generate condition for where part of query
         arguments_.append( { { "name", stringName }, { "value", value_ }, { "type", uType }, { "operator", uOperator } }, gd::types::tag_view{});
         queryDelete.condition_add( arguments_, gd::sql::tag_arguments{} );   // add condition to query
      }
   }

   // ## Add conditions to query if there are any ............................
   if( m_vectorCondition.empty() == false )
   {
      auto [success, error] = utility::add_condition_to_query( m_vectorCondition, queryDelete );
      if( success == false ) return { false, error };
   }

   // ## Generate delete query ..............................................

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

std::pair<bool, std::string> CRENDERSql::ToSqlFromTemplate( std::string_view stringTemplate, std::string& stringQuery )
{
   gd::sql::query query_(m_eSqlDialect);
   Query_AddFields(&query_);

   auto [bSuccess, stringError] = query_.sql_format( stringTemplate, stringQuery );
   if( bSuccess == false ) { return { false, "Failed to generate SQL from template: " + stringError }; }

   return { true, "" };
}

std::pair<bool, std::string> CRENDERSql::ToBulkInsert( const gd::argument::arguments& argumentsOptions, pugi::xml_document* pxmldocument, std::function<bool( std::string_view )> execute_ )
{                                                                                                  assert( GetDocument() != nullptr ); 
   using namespace gd::sql;
   std::array<char, 128> buffer_; // buffer to avoid allocate memory
   uint64_t uInsertCount = 0;

   const META::CDatabase* pdatabase_ = GetDocument()->DATABASE_Get();                                assert( pdatabase_ != nullptr );

   std::string stringInsertTemplate; // If insert template is set for query
   std::string stringForm = argumentsOptions["form"].as_string(); // layout is required and should be string
   std::string stringContainer = argumentsOptions["container"].as_string(); // container is required and should be string
   if( stringContainer.empty() == true ) { stringContainer = "//values"; }

   // ## xml form is like <values column1="value1" column2="value2" />
   if( stringForm == "attribute" )                                            // each value is sent as attribute in xml element, for example <values column1="value1" column2="value2" />
   {
      std::string stringTable = MetaGetTable();                               // get table for values

      // ### Loop elements in container
      pugi::xpath_node_set xpathnodesetValues = pxmldocument->select_nodes(stringContainer.c_str());
      for( auto& xpathnode_ : xpathnodesetValues )
      {
         query queryInsert{ m_eSqlDialect };

         queryInsert << table_g( stringTable, buffer_ );                     // set table for insert query

         pugi::xml_node xmlnodeValue = xpathnode_.node();
         for( auto& xmlattribute_ : xmlnodeValue.attributes() )              // loop attributes in element and add attribute name and values to query
         {
            std::string_view stringName = xmlattribute_.name();
            std::string_view stringValue = xmlattribute_.value();

            gd::argument::arguments argumentsFind( buffer_ );
            argumentsFind.append( { {std::string_view("table"), gd::variant_view(stringTable)}, {std::string_view("column"), gd::variant_view(stringName)} }, gd::types::tag_view{});
            int64_t iRow = pdatabase_->Column_FindRow( argumentsFind ); 
            if( iRow == -1 ) { return { false, "column not found in database: " + std::string(stringName) }; }

            auto uType = pdatabase_->Column_GetType( iRow );
            queryInsert << field_g( stringName, buffer_ ).value( stringValue ).type( uType );
         }

         std::string stringInsertSql;

         if( stringInsertTemplate.empty() == false )
         {
            const gd::argument::arguments* pargumentsGlobal = m_papicontext->GetGlobalArguments();
         }
         else 
         {
            stringInsertSql = queryInsert.sql_get( eSqlInsert );
         }
      }
   }
   else if( stringForm == "container" )                                       // each value is sent as child element in container element, for example <values><value column="column1">value1</value><value column="column2">value2</value></values>
   {

   }



   return { true, "" };
}

/*
std::pair<bool, std::string> CRENDERSql::AddConditionToQuery( gd::sql::query& queryAddTo  )
{
   for( auto& argumentsCondition : m_argumentsCondition )
   {                                                                                               assert( gd::sql::query::validate_condition_s( argumentsCondition ).first == true );
      // ## check for operator
      if( argumentsCondition.exists( "operator" ) == false ) { argumentsCondition.append( "operator", uint32_t( gd::sql::eOperatorEqual ) ); }
      else
      {
         auto v_ = argumentsCondition["operator"].as_variant_view();
         if( v_.is_string() == true )
         {
            auto eOperator = gd::sql::query::get_where_operator_number_s(v_.as_string_view());
            if( eOperator == gd::sql::eOperatorError ) { return { false, std::format( "Invalid operator: {}", v_.as_string_view() ) }; }

            argumentsCondition.set( "operator", eOperator );
         }
      }

      queryAddTo.condition_add( argumentsCondition, gd::sql::tag_arguments{} );
   }  

   return { true, "" };
}
*/

/// Convert the internal table field data into a structured arguments collection
void CRENDERSql::ToArguments( gd::argument::arguments& arguments ) const
{
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{} ).as_string();
      auto value_ = itRow.cell_get_variant_view( "value", gd::table::tag_not_null{} );
      arguments.append_argument( stringName, value_ );
   }
}

/// Convert the internal table field data into a structured shared arguments collection
void CRENDERSql::ToArguments( gd::argument::shared::arguments& arguments ) const
{
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); ++itRow )
   {
      std::string stringName = itRow.cell_get_variant_view( "name", gd::table::tag_not_null{} ).as_string();
      auto value_ = itRow.cell_get_variant_view( "value", gd::table::tag_not_null{} );
      arguments.append_argument( stringName, value_ );
   }
}


/**  -------------------------------------------------------------------------- Validate
 * @brief Validate named SQL render attributes against known field columns.
 * 
 * Verifies that every key in `argumentsValue` exists in `m_tableField`.
 * When a key is valid, this method accumulates corresponding `eColumnFlag*`
 * values into `uRequired` so callers can check which required parts were found.
 * 
 * **Important:** validation fails immediately on the first unknown column name.
 * 
 * @param argumentsValue Named argument collection to validate.
 * @param puFound Optional output pointer that receives the accumulated found flags.
 * @return std::pair<bool, std::string> `{ true, "" }` if all keys are valid;
 *         otherwise `{ false, "Invalid column name: <column>" }`.
 */
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
         case 'n': uRequired |= eColumnFlagName; break;
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

/// Validate condition arguments against known field columns, ensuring all keys are valid for condition construction.
std::pair<bool, std::string> CRENDERSql::ValidateCondition( gd::argument::arguments argumentsValue ) const
{
   auto [bOk, stringField] = gd::sql::query::validate_condition_s(argumentsValue);

   if( bOk == false ) { return { false, std::format( "Invalid condition field: {}", stringField ) }; }

   return { true, "" };
}

// Validate values in table byt match against metadata for columns and also check values
std::pair<bool, std::string> CRENDERSql::ValidateColumnValues() const
{                                                                                                  assert( GetDocument() != nullptr ); 
   std::array<std::byte, 256> buffer_;
   gd::argument::arguments argumentsFind( buffer_ );
   const META::CDatabase* pdatabase_ = GetDocument()->DATABASE_Get();                              assert( pdatabase_ != nullptr );

   bool bIsValueValid = true; // 
   for( auto itRow = m_tableField.row_begin(); itRow != m_tableField.row_end(); itRow++ )
   {
      const auto bIsNull = itRow.cell_is_null( eColumnFieldMeta );
      if( bIsNull == false )
      {
         
         uint32_t uRowMeta = itRow.cell_get_variant_view( eColumnFieldMeta ).as_uint(); // index to meta row for columns
         uint32_t uType = pdatabase_->Column_GetType( uRowMeta );             // get type for column from meta row

         auto value_ = itRow.cell_get_variant_view( eColumnFieldValue, gd::table::tag_not_null{} );
         if( value_.is_string() == true )
         {
            bIsValueValid = gd::sql::validate_value_g( value_.as_string_view(), uType );
         }
      }

      if( bIsValueValid == false ) { return { false, std::format( "Invalid value for column: {}", itRow.cell_get_variant_view( eColumnFieldName ).as_string() ) }; }
   }
   return { true, "" };
}

/// @brief Gets the table name from the metadata based on the current row statement
std::string CRENDERSql::MetaGetTable() const 
{                                                                                                  assert( GetDocument() != nullptr );
   std::string stringTable;
   if( m_iRowStatement != -1 ) 
   {
      const uint64_t uRowStatement = uint64_t( m_iRowStatement );
      const META::CQueries* pqueries = GetDocument()->QUERIES_Get();                                 assert( pqueries != nullptr );
      stringTable = pqueries->GetTable( uRowStatement );
   }
   return stringTable;
}


/** --------------------------------------------------------------------------- ToArgumentsFromArray_s
 * @brief Converts a JSON array into a `gd::argument::arguments` key-value structure.
 * 
 * Parses a JSON array with positional elements representing column metadata:
 * - `[0]` → `"table"`
 * - `[1]` → `"name"`
 * - `[2]` → `"value"` *(optional)*
 * - `[3]` → `"type_part"` *(optional, must be a known part type string)*
 * - `[4]` → `"operator"` *(optional, triggers `where` part type when present)*
 * 
 * At minimum, the array must contain `[table, name]`. If a `type_part` string
 * at index `[3]` is unrecognized, the method returns an error.
 * 
 * @param jsonColumn  JSON array with positional column descriptor values.
 * @param arguments_  Output `arguments` object that will be populated; cleared before writing.
 * @return std::pair<bool, std::string> `{true, ""}` on success, or `{false, errorMessage}` on failure.
 * 
 * @code
 * // Minimal: table + name
 * jsoncons::json jsonArray = jsoncons::json::parse(R"(["users","id"])");
 * gd::argument::arguments argumentsResult;
 * auto [bOk, stringError] = CRENDERSql::ToArgumentsFromArray_s(jsonArray, argumentsResult);
 * 
 * // Full: table, name, value, type_part, operator
 * jsoncons::json jsonFull = jsoncons::json::parse(R"(["users","id","42","where","="])");
 * @endcode
 */
std::pair<bool, std::string> CRENDERSql::ToArgumentsFromArray_s( const jsoncons::json& jsonColumn, gd::argument::arguments& arguments_ )
{
   auto uCount = jsonColumn.size();
   if( uCount < 2 ) { return { false, std::format( "requires at least [table,name]" ) }; }

   arguments_.clear();

   arguments_.append( "table", jsonColumn[0].as_string_view() );
   arguments_.append( "name", jsonColumn[1].as_string_view() );

   if( uCount >= 3 ) { arguments_.append( "value", jsonColumn[2].as_string_view() ); }

   // ## check for where part and this is before other parts because where parts need to have operator
   if( uCount >= 5 ) 
   { 
      arguments_.append( "operator", jsonColumn[4].as_string_view() ); 
      arguments_.append( "type_part", ePartTypeWhere );
   }

   if( uCount >= 4  )
   {
      auto ePart = utility::get_part_type_from_string( jsonColumn[3].as_string_view() );
      if( ePart == ePartTypeUnknown ) { return { false, std::format( "unknown type_part: {}", jsonColumn[3].as_string_view() ) }; }
      arguments_.append( "type_part", ePart );
   }

   return { true, "" };
}

/// @brief Similar to `ToArgumentsFromArray_s`, but parses a JSON object with named keys instead of a positional array.
std::pair<bool, std::string> CRENDERSql::ToArgumentsFromObject_s( const jsoncons::json& jsonColumn, gd::argument::arguments& arguments_ )
{
   if( jsonColumn.is_object() == false ) { return { false, "Expected JSON object" }; }
   arguments_.clear();

   for( const auto& item_ : jsonColumn.object_range() )
   {                                                                                               assert( item_.value().is_string() == true );
      std::string_view stringKey = item_.key();

      if( stringKey == "operator" )
      {
         arguments_.append( "operator", item_.value().as_string_view() );
         arguments_.append( "type_part", ePartTypeWhere );
      }
      else if( stringKey == "type_part" )
      {
         auto ePart = utility::get_part_type_from_string( item_.value().as_string_view() );
         if( ePart == ePartTypeUnknown ) { return { false, std::format( "unknown type_part: {}", item_.value().as_string_view() ) }; }
         arguments_.append( "type_part", ePart );
      }
      else 
      { 
         arguments_.append( item_.key(), item_.value().as_string_view() );
      }
   }
   return { true, "" };    
}



/** ------------------------------------------------------------------------- ValidateIsAllStrings_s
 * @brief Validates that a JSON structure contains only string values.  
 * @param json_ The JSON object, array, or value to validate.
 * @return True if all values are strings; false otherwise.
 */
bool CRENDERSql::ValidateIsAllStrings_s( const jsoncons::json& json_ )
{
   if( json_.is_array() == true )                                             // array
   {
      for( const auto& item : json_.array_range() )
      {
         if( item.is_string() == false ) { return false; }
      }
   }
   else if( json_.is_object() == true )                                       // object
   {
      for( const auto& item : json_.object_range() )
      {
         if( item.value().is_string() == false ) { return false; }
      }
   }
   else if( json_.is_string() == false ) { return false; }

   return true;
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





static std::pair<bool, std::string> EXPRESSION_GetArgument_s( const std::vector<gd::expression::value>& vectorArgument, gd::expression::value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 1);
   auto object_ = vectorArgument[1];                                                               assert(object_.is_pointer() == true);
   gd::argument::shared::arguments* parguments_ = (gd::argument::shared::arguments*)object_.get_pointer();

   auto& name_ = vectorArgument[0];
   if( name_.is_string() == true )
   {
      std::string stringName( name_.as_string() );
      if( stringName.empty() == true ) { return { false, "Argument name cannot be empty." }; }
      auto variantview_ = ( *parguments_ )[stringName].as_variant_view();
      *pvalueReturn = gd::expression::to_value_g( variantview_ );

      return { true, "" };
   }

   return { false, "Invalid argument name type, expected string." };
}

static std::pair<bool, std::string> EXPRESSION_Exists_s( const std::vector<gd::expression::value>& vectorArgument, gd::expression::value* pvalueReturn )
{                                                                                                  assert(vectorArgument.size() > 1);
   auto object_ = vectorArgument[1];                                                               assert(object_.is_pointer() == true);
   gd::argument::shared::arguments* parguments_ = (gd::argument::shared::arguments*)object_.get_pointer();

   auto& name_ = vectorArgument[0];
   if( name_.is_string() == true )
   {
      std::string stringName( name_.as_string() );
      if( stringName.empty() == true ) { return { false, "Argument name cannot be empty." }; }
      auto variantview_ = ( *parguments_ )[stringName].as_variant_view();

      if( variantview_.is_null() == true ) { *pvalueReturn = false; }         // if argument not found, return false
      else { *pvalueReturn = true; }

      return { true, "" };
   }

   return { false, "Invalid argument name type, expected string." };
}
