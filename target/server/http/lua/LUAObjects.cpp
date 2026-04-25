// @FILE [tag: lua, objects] [summary: Implementation file for LUA objects and utility functions] [type: source] [name: LUAObjects.cpp]

#include <filesystem>
#include <string>
#include <string_view>
#include <variant>

#include "gd/gd_file.h"
#include "gd/gd_parse.h"
#include "gd/gd_table_aggregate.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_arguments.h"
#include "gd/gd_table_table.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_variant.h"

#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_glue_to_gd.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_code.h"


#include "lua/sol.hpp"

#include "../Document.h"
#include "../Application.h"

#include "LUAObjects.h"

LUA_BEGIN

void ConvertTo_g( const sol::table& tableText, std::vector<std::string>& vecorText )
{
   for( auto& it : tableText )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::string ) { vecorText.push_back( value_.as<std::string>() ); }
   }
}


void ConvertTo_g( const sol::table& tableValue, std::vector<gd::variant>& vecorValue )
{
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { vecorValue.push_back( value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { vecorValue.push_back( gd::variant( value_.as<std::string_view>() ) ); }
      else if( type_ == sol::type::boolean ) { vecorValue.push_back( value_.as<bool>() ); }
   }
}

void ConvertTo_g( const sol::table& tableValue, std::vector<gd::variant_view>& vecorValue )
{
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { vecorValue.push_back( value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { vecorValue.push_back( gd::variant_view( value_.as<std::string_view>() ) ); }
      else if( type_ == sol::type::boolean ) { vecorValue.push_back( value_.as<bool>() ); }
   }
}

gd::argument::arguments ConvertToArguments_g( const sol::table& tableValue )
{
   gd::argument::arguments argumentsValue;
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<std::string_view>()); }
   }

   return argumentsValue; 
}

gd::argument::shared::arguments ConvertToSharedArguments_g( const sol::table& tableValue )
{
   gd::argument::shared::arguments argumentsValue;
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<std::string_view>()); }
   }

   return argumentsValue; 
}


void ConvertToArguments_g( const sol::table& tableValue, gd::argument::arguments& argumentsValue )
{
   for( const auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<std::string_view>()); }
      else if( type_ == sol::type::table ) 
      {
         const auto& name_ = it.first.as<std::string_view>();
         const auto& tableInner = value_.as<sol::table>();
         for( const auto& itInner : tableInner )
         {
            const auto& valueInner = itInner.second;
            sol::type iType = valueInner.get_type();
            if( iType == sol::type::number ) { argumentsValue.append( name_, valueInner.as<int64_t>()); }
            else if( iType == sol::type::string ) { argumentsValue.append( name_, valueInner.as<std::string_view>()); }
         }
      }
      else if( type_ == sol::type::boolean ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<bool>()); }
   }
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::variant& value_ )
{
   if( value_.is_integer() == true )      { return value_.as_int64(); }
   else if( value_.is_string() == true )  { return value_.as_string(); }
   else if( value_.is_decimal() == true ) { return value_.as_double(); }
   else if( value_.is_bool() == true )    { return value_.as_bool(); }
   return sol::lua_nil;
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::expression::value& value_ )
{
   if( value_.is_integer() == true )      { return value_.is_integer(); }
   else if( value_.is_string() == true )  { return value_.as_string(); }
   else if( value_.is_double() == true )  { return value_.as_double(); }
   else if( value_.is_bool() == true )    { return value_.as_bool(); }
   return sol::lua_nil;
}

std::variant<int64_t, std::string, double, bool, sol::table, sol::lua_nil_t> ConvertToAnyWithTable_g( gd::variant_view value_ )
{
   if( value_.is_integer() == true )      { return value_.as_int64(); }
   else if( value_.is_string() == true )  { return value_.as_string(); }
   else if( value_.is_decimal() == true ) { return value_.as_double(); }
   else if( value_.is_binary() == true )  { return value_.as_string(); }
   else if( value_.is_bool() == true )    { return value_.as_bool(); }
   return sol::lua_nil;
}

std::variant<int64_t, std::string, double, bool, sol::table, sol::lua_nil_t> ConvertToAnyWithTable_g( const gd::argument::arguments& argumentsValue )
{
   sol::table table_;

   for( const auto& [key_, value_] : argumentsValue.named() )
   {
      if( value_.is_integer() == true ) { table_[key_] = value_.as_int64(); }
      else if( value_.is_string() == true ) { table_[key_] = value_.as_string(); }
      else if( value_.is_decimal() == true ) { table_[key_] = value_.as_double(); }
      else if( value_.is_binary() == true ) { table_[key_] = value_.as_string(); }
   }
   return table_;
}



std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::variant_view& value_ )
{
   if( value_.is_integer() == true )      { return value_.as_int64(); }
   else if( value_.is_string() == true )  { return value_.as_string(); }
   else if( value_.is_decimal() == true ) { return value_.as_double(); }
   return sol::lua_nil;
}

gd::variant ConvertFromAny_g( const std::variant<int64_t, std::string, double, bool, sol::lua_nil_t>& any_ )
{
   auto uIndex = any_.index();
   if( uIndex == 0 ) return std::get<0>( any_ );
   else if( uIndex == 1 ) return std::get<1>( any_ );
   else if( uIndex == 2 ) return std::get<2>( any_ );
   else if( uIndex == 3 ) return std::get<3>( any_ );

   return gd::variant();
}

void ConvertFromAny_g( const std::variant<int64_t, std::string, double, bool, sol::lua_nil_t>& any_, gd::variant_view& variantviewValue )
{
   auto uIndex = any_.index();
   if( uIndex == 0 ) variantviewValue = std::get<0>( any_ );
   else if( uIndex == 1 ) variantviewValue = std::get<1>( any_ );
   else if( uIndex == 2 ) variantviewValue = std::get<2>( any_ );
   else if( uIndex == 3 ) variantviewValue = std::get<3>( any_ );
   else
   {
      variantviewValue.clear();
   }
}

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- Table
// ----------------------------------------------------------------------------

namespace {
   /// add columns by parsing string to extract information about columns to adds
   void add_column_s(gd::table::dto::table*& ptable, const std::string_view& stringColumn)
   {
      auto result_ =  ptable->column_add( stringColumn, gd::table::tag_parse{});
      if(result_.first == false)
      {
         delete ptable;
         ptable = nullptr;
         std::string stringError("Invalid format: ");
         stringError += result_.second;
         throw sol::error( stringError );
      }

      result_ = ptable->prepare();
      if(result_.first == false)
      {
         delete ptable;
         ptable = nullptr;
         std::string stringError("Invalid format: ");
         stringError += result_.second;
         throw sol::error( stringError );
      }
   }
}

/// Construct table with full meta support and start with 10 number of allocated rows (grows dynamically)
/// Use parse logic in table to generate columns, read internal dto table docs to find how that works
Table::Table(const std::string_view& stringColumn )
{
   m_ptable = new gd::table::dto::table( 10, gd::table::tag_full_meta{});
   add_column_s( m_ptable, stringColumn );
}

/// Construct table with full meta support and start with specified number of allocated rows (grows dynamically)
/// Use parse logic in table to generate columns, read internal dto table docs to find how that works
Table::Table( uint64_t uRowCount, const std::string_view& stringColumn )
{
   m_ptable = new gd::table::dto::table( static_cast<unsigned>(uRowCount), gd::table::tag_full_meta{});
   add_column_s( m_ptable, stringColumn );
}


Table::~Table() 
{ 
   delete m_ptable; 
}

/** ---------------------------------------------------------------------------
 * @brief Set column attribute for attribute name
 * @param column_ {int64_t|string} column attribute is set to
 * @param stringAttribute name for attribute, could be "name|alias"
 * @param value_ value set for attribute
 */
void Table::SetColumnAttribute(std::variant< int64_t, std::string_view> column_, const std::string_view& stringAttribute, std::variant<int64_t, std::string_view> value_ )
{
   bool bError = false;
   int iColumn;

   if( column_.index() == 1 ) 
   { 
      iColumn = m_ptable->column_find_index( std::get<1>( column_ ) ); 
      if( iColumn < 0 ) { bError = true; } 
   }
   else { iColumn = (int)std::get<0>( column_ ); }

   if( bError == false && stringAttribute == "name" )
   {
      if( value_.index() == 1 ) m_ptable->column_rename( iColumn, std::get<1>( value_ ) );
      else bError = true;
   }

   if( bError == true )
   {
      std::string stringError = std::format( "invalid attribute value for column {}, attribute: {}, value: ", iColumn, stringAttribute );
      if( value_.index() == 0 ) stringError += std::to_string( std::get<0>( value_ ) );
      else if( value_.index() == 1 ) stringError += std::get<1>( value_ );
      throw sol::error( stringError );
   }
}


/** ---------------------------------------------------------------------------
 * @brief Returns list of selected column attribute
 * Column attribute are returned as lua table for all columns
 * @param stringAttribute column attribugte name to return as table, "name", "alias", 
 * @return sol::table table with selected column attribute value
 */
sol::table Table::GetColumns( const sol::variadic_args& variadicargs )
{
   auto uArgumentCount = variadicargs.size();
   sol::state_view stateview_ = variadicargs.lua_state();

   sol::table tableAttributeValue = stateview_.create_table();

   if(uArgumentCount == 1)
   {
      sol::type eType = variadicargs[0].get_type();
      if( eType != sol::type::string ) { throw sol::error( "invalid argument" ); }

      std::string stringAttribute = variadicargs[0];

      uint32_t uAttribute = gd::types::detail::hash_type( stringAttribute );      // to optimize, no need to compare strings to check what type to return

      for(unsigned uIndex = 0, uCount = m_ptable->get_column_count(); uIndex < uCount; uIndex++)
      {
         switch(uAttribute)
         {
         case gd::types::detail::hash_type("name"):
            tableAttributeValue.add( m_ptable->column_get_name( uIndex ) );
            break;
         case gd::types::detail::hash_type("alias"):
            tableAttributeValue.add( m_ptable->column_get_alias( uIndex ) );
            break;
         case gd::types::detail::hash_type("type"):
            tableAttributeValue.add( m_ptable->column_get_type( uIndex ) );
            break;
         case gd::types::detail::hash_type("ctype"):
            tableAttributeValue.add( m_ptable->column_get_ctype( uIndex ) );
            break;
         }
      }
   }
   else
   {
      throw sol::error( "invalid argument" );
   }

   return tableAttributeValue;
}

/// Get cell value in table
std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Table::GetCellValue( uint64_t uRow, std::variant<int64_t, std::string_view> column_ ) const
{                                                                                                  assert( m_ptable != nullptr );
   int iColumn;
   gd::variant_view value_;

   if( column_.index() == 1 ) 
   { 
      auto name_ = std::get<1>( column_ );
      iColumn = m_ptable->column_find_index( name_ ); 
      if( iColumn < 0 ) 
      { 
         throw sol::error( std::format("invalid column name {}", name_) ); 
      } 
   }
   else
   { 
      iColumn = (int)std::get<0>( column_ ); 
   }

   if( uRow < m_ptable->get_row_count() && ( unsigned )iColumn < m_ptable->get_column_count() )
   {
      value_ = m_ptable->cell_get_variant_view( uRow, iColumn );
   }
   else
   {
      std::string stringError = std::format("row or column is invalid or out of range, max row: {}, specified row: {}", (m_ptable->get_row_count() - 1), uRow );

      if( column_.index() == 1 ) stringError += std::string(", column name: ") + std::get<1>( column_ ).data();

      throw sol::error( stringError );
   }

   return ConvertToAny_g( value_ );
}

/// Set value to cell in table
void Table::SetCellValue( uint64_t uRow, std::variant<int64_t, std::string_view> column_, std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> value_ )
{                                                                                                  assert( m_ptable != nullptr );
   int iColumn;

   if( column_.index() == 1 ) { iColumn = m_ptable->column_find_index( std::get<1>( column_ ) ); if( iColumn < 0 ) { throw sol::error( std::format("invalid column name {}", std::get<1>( column_ )) ); } }
   else                       { iColumn = (int)std::get<0>( column_ ); }

   if( uRow < m_ptable->get_row_count() && ( unsigned )iColumn < m_ptable->get_column_count() )
   { 
      gd::variant_view variantviewValue;
      ConvertFromAny_g( value_, variantviewValue );
      m_ptable->cell_set( uRow, iColumn, variantviewValue, gd::table::tag_convert{});
   }
   else
   {
      std::string stringError("row or column is invalid or out of range");
      throw sol::error( stringError );
   }
}

void Table::SetCellValues( const Table& table, const sol::table& tableArguments )
{
   const auto* ptableRight = table.GetTable();

   gd::argument::arguments argumentsValue;
   ConvertToArguments_g( tableArguments, argumentsValue );
   
   if(argumentsValue.exists("lookup") == true)
   {
      unsigned uColumnLeft;
      unsigned uColumnRight;
      auto pairJoin = argumentsValue.find_pair( "lookup" );
      if( pairJoin.first.is_integer() == true )
      {
         uColumnLeft = pairJoin.first.as_uint();
         uColumnRight = pairJoin.second.as_uint();
      }

      std::vector< std::pair<uint64_t,uint64_t> > vectorJoin;
      gd::table::dto::table::join_s( m_ptable, uColumnLeft, ptableRight, uColumnRight, vectorJoin );


      auto pairCopy = argumentsValue.find_pair( "copy" );
      if(pairCopy.first.is_integer() == true)
      {
         uColumnLeft = pairCopy.first.as_uint();
         uColumnRight = pairCopy.second.as_uint();
      }

      for( auto itCopyRow : vectorJoin )
      {
         const auto value_ = ptableRight->cell_get_variant_view( itCopyRow.second, uColumnRight );
         m_ptable->cell_set( itCopyRow.first, uColumnLeft, value_ );
      }
   }

}

/** ---------------------------------------------------------------------------
 * @brief fill column or area in table with value
 * @param column_ column or area to fill with value
 * @param value_ value to set to specified cells
 */
void Table::Fill(std::variant<int64_t, std::string_view, sol::table> column_, std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> value_)
{
   int iColumn = -1;

   if( column_.index() == 1 )      { iColumn = m_ptable->column_find_index( std::get<1>( column_ ) ); if( iColumn < 0 ) { throw sol::error( std::format("invalid column name {}", std::get<1>( column_ )) ); } }
   else if( column_.index() == 1 ) { iColumn = (int)std::get<0>( column_ ); }
   else 
   {                                                                                               assert( false ); // TODO: fix this

   }

   gd::variant_view variantviewValue;
   ConvertFromAny_g( value_, variantviewValue );                               // convert value to search for to variant view that is used to find value in table

   if(iColumn != -1)
   {
      m_ptable->column_fill( (unsigned)iColumn, variantviewValue );
   }
}

/** ---------------------------------------------------------------------------
 * @brief Find value in table
 * @param column_ column where value are searched for
 * @param value_ value to search for
 * @return int64_t row index to row where value was found
 */
int64_t Table::Find(std::variant<int64_t, std::string_view> column_, std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> value_)
{
   int iColumn;

   if( column_.index() == 1 ) { iColumn = m_ptable->column_find_index( std::get<1>( column_ ) ); if( iColumn < 0 ) { throw sol::error( std::format("invalid column name {}", std::get<1>( column_ )) ); } }
   else                       { iColumn = (int)std::get<0>( column_ ); }


   gd::variant_view variantviewValue;
   ConvertFromAny_g( value_, variantviewValue );                               // convert value to search for to variant view that is used to find value in table

   int64_t iRow = m_ptable->find_variant_view( (unsigned)iColumn, variantviewValue );

   return iRow;
}

/** ---------------------------------------------------------------------------
 * @brief Add row to table
 * @param row_ optinal value that holds number of rows to add or table with row values
 */
void Table::AddRow(const sol::optional<std::variant<uint64_t, sol::table>> row_)
{                                                                                                  assert( m_ptable != nullptr );
   if(row_.has_value() == true)
   {
      const std::variant<uint64_t, sol::table>& temp_ = row_.value();
      if(temp_.index() == 0)
      {
         uint64_t uRowCount = std::get<0>( temp_ );
         m_ptable->row_add( uRowCount );
      }
      else
      {
         const auto& table_ = std::get<1>( temp_ );
         std::vector<gd::variant_view> vectorValue;
         ConvertTo_g( table_, vectorValue );
         m_ptable->row_add( vectorValue, gd::table::tag_convert{});
      }
   }
   else
   {
      m_ptable->row_add();
   }
}

/** ----------------------------------------------------------------------------
 * @brief Read data from file or from sent data.
 * 
 * Read information into table. It could be a csv file that is read. Make sure that
 * the csv file format matches.
 * 
 * @param stringFileOrData File name or raw data that is read into table
 * @param format_ options how to read data into table
 * @param format_.begin first row to read
 * @param format_.columns comma separated string with columns read values are inserted to
 * @param format_.file if true then load text from file
 * @param format_.format format of string read into table, `csv` = CSV formated text
 */
void Table::Read( const std::string_view& stringFileOrData, const std::variant<std::string_view,sol::table>& format_ )
{
   using namespace gd::types;

   std::string stringData;
   std::string_view stringTable;
   gd::argument::arguments argumentsOption;

   if( format_.index() == 0 )
   {
      argumentsOption.append( "format", std::get<0>( format_ ) );
   }
   else
   {
      ConvertToArguments_g( std::get<1>( format_ ), argumentsOption );
   }


   if( argumentsOption["file"].is_true() == true && std::filesystem::exists(stringFileOrData) == true)
   {
      auto result_ = gd::file::read_file_g( stringFileOrData, stringData );
      if( result_.first == false ) { throw sol::error( result_.second ); }

      stringTable = stringData;
   }
   else
   {
      stringTable = stringFileOrData;
   }

   std::string stringFormat = argumentsOption["format"].as_string();

   // ## CSV reading ----------------------------------------------------------
   if( stringFormat.empty() == true || stringFormat == "csv" )                  // ## if csv data then read csv values into table
   {
      uint64_t uFirstRow = 0;
      std::string_view stringCsv( stringTable );
      if( argumentsOption["begin"].is_true() ) uFirstRow = argumentsOption["begin"].as_uint64();

      // ## Move to row where to start reading if not 0
      if( uFirstRow != 0 )
      {
         const char* pbszBegin = stringTable.data();
         for( uint64_t u = 0; u < uFirstRow && pbszBegin != nullptr; u++ )
         {
            pbszBegin = gd::parse::strchr( pbszBegin, '\n', gd::parse::csv() );
            if( pbszBegin != nullptr ) pbszBegin = gd::parse::next_non_space_g( pbszBegin );
         }

         if( pbszBegin != nullptr )
         {  // set 
            stringCsv = std::string_view( pbszBegin, (&stringTable.back() + 1) - pbszBegin );
         }
         else return;                                                          // no more rows
      }
      else { stringCsv = stringTable; }

      if( argumentsOption.exists( "length" ) == true )                          // ## do we have max number of rows (length)
      {
         uint64_t uLength = argumentsOption["length"].as_uint64();
         const char* pbszPosition = stringCsv.data();
         for( uint64_t u = 0; u < uLength && pbszPosition != nullptr; u++ )
         {
            pbszPosition = gd::parse::strchr( pbszPosition, '\n', gd::parse::csv() );
         }

         const char* pbszBegin = stringCsv.data();
         if( pbszPosition != nullptr && pbszPosition > pbszBegin )
         {
            pbszPosition--;
            while( pbszPosition > pbszBegin && is_ctype_g( *pbszPosition, "space"_ctype ) == true ) pbszPosition--;
            pbszPosition++;                                                     // add one because it should then point to valid character, it should point to character after last valid character
         }

         stringCsv = std::string_view( pbszBegin, pbszPosition - pbszBegin );
      }


      // ## read csv data into table

      std::string stringColumns = argumentsOption["columns"].as_string();
      if(stringColumns.empty() == false)                                       // if columns is set the convert to column index and pass that to know what column value is placed in
      {
         std::vector< unsigned > vectorColumnIndex;
         std::vector< std::variant< unsigned, std::string > > vectorColumn;
         gd::utf8::split( stringColumns, ',', vectorColumn );
         for(const auto& it : vectorColumn)
         {
            unsigned uColumn;
            if( it.index() == 0 ) uColumn = std::get<0>( it ); 
            else
            {
               uColumn = (unsigned)m_ptable->column_find_index( std::get<1>( it ) );
               if( uColumn >= m_ptable->get_column_count() ) throw sol::error( std::string("column out of range: ") + std::get<1>( it ) );
            }
            vectorColumnIndex.push_back( uColumn );
         }

         auto result_ = gd::table::read_g( *m_ptable, stringCsv, vectorColumnIndex, ',', '\n', gd::table::tag_io_csv{});
         if( result_.first == false ) throw sol::error( result_.second );
      }
      else
      {                                                                        // values in cvs should match columns in table
         // ## Default csv reading, not using any special settings
         auto result_ = gd::table::read_g( *m_ptable, stringCsv, ',', '\n', gd::table::tag_io_csv{});
         if( result_.first == false ) throw sol::error( result_.second );
      }
   }
}

/**
 * @brief 
~~~(.lua)
-- create table
local tableMainResult = Table.new( "rstring,name;string,20,unit;double,value;double,standard_error;int64,code" )
-- read csv file
tableMainResult:Read( sCsvFile, { format="csv", begin=1, file=1} ) 
-- write sql insert into queries
local sSqlResult = tableMainResult:Write( { table="TName", format="sql"} )
~~~
 * @param tableOption 
 * @return 
 */
std::string Table::Write(const sol::table& tableOption)
{
   using namespace gd::table;

   std::string stringResult;                 // written result returned
   gd::argument::arguments argumentsOption;  // options harvested from table argument
   std::vector<unsigned> vectorColumn;

   ConvertToArguments_g( tableOption, argumentsOption );

   // ## check for columns to 
   if( argumentsOption.exists( "columns" ) == true )
   {
      std::string_view stringColumns = argumentsOption["columns"].as_string_view();
      std::vector< std::variant< unsigned, std::string > > vector_;
      gd::utf8::split( stringColumns, ',', vector_ );
      for( auto it : vector_ )
      {
         if( it.index() == 1 )
         {
            int iColumn = m_ptable->column_find_index( std::get<1>( it ) );
            if( iColumn != -1 ) { vectorColumn.push_back( unsigned( iColumn ) ); }
            else                { throw sol::error( std::string( "Unknown column: ") + std::get<1>( it ) ); }
         }
         else { vectorColumn.push_back( std::get<0>( it ) ); }
      }
   }

   int32_t iMax = -1;
   if( argumentsOption.exists("max_column_width") == true ) iMax = argumentsOption["max_column_width"].as_int();
   int64_t iCount = -1;
   if( argumentsOption.exists("count") == true ) iCount = argumentsOption["count"].as_int64();
   bool bHeader = false;
   if( argumentsOption.exists("header") == true ) bHeader = argumentsOption["header"].is_true();
   bool bBody = true;
   if( argumentsOption.exists("body") == true ) bBody = argumentsOption["body"].is_true();

   std::string stringFormat = argumentsOption["format"].as_string();
   if( stringFormat == "cli" ) 
   {
      if(bBody == false)
      {
         if(bHeader == true)
         {
            stringResult = to_string( *m_ptable, tag_io_header{},tag_io_cli{} );
         }
      }
      else
      {
         stringResult = to_string( *m_ptable, argumentsOption, tag_io_cli{} );
      }
   }
   else if(stringFormat == "csv")
   {
      //std::string stringTable = argumentsOption["table"].as_string();
      //gd::table::write_insert_g( stringTable, *m_ptable, stringResult, tag_io_sql{} );
   }
   else if(stringFormat == "sql")
   {
      gd::argument::arguments argumentsWriteOption;
      std::string stringTable = argumentsOption["table"].as_string();
      if( argumentsOption.exists("names") == true ) { argumentsWriteOption.append_argument( "names", argumentsOption["names"].as_variant_view() ); }
      
      if(argumentsOption.exists("template") == true)                            // if template then generate strings from that template
      {
         // ## 
         uint64_t uRow = argumentsOption["row"].as_uint64();
         if( uRow >= m_ptable->get_reserved_row_count() ) throw sol::error( std::format("Max row is {}, row {} is out of range", m_ptable->get_reserved_row_count() - 1, uRow ));
         gd::argument::arguments argumentsValue = m_ptable->row_get_arguments( uRow );
         std::string stringTemplate = argumentsOption["template"].as_string();
      }
      else
      {
         // ## write insert queries
         if( vectorColumn.empty() == true ) { gd::table::write_insert_g( stringTable, *m_ptable, stringResult, argumentsWriteOption, tag_io_sql{} ); }
         else { gd::table::write_insert_g( stringTable, *m_ptable, vectorColumn, stringResult, argumentsWriteOption, tag_io_sql{} ); }
      }
   }



   return stringResult;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------- Expression
// ----------------------------------------------------------------------------


std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Expression::Calculate( std::string_view stringExpression, std::optional<sol::table> table_ )
{
   using namespace gd::expression;
   gd::expression::value valueReturn;
   gd::expression::runtime runtime_;
   std::vector<token> vectorToken;

   if( table_.has_value() == true )
   {
      gd::argument::arguments argumentsValue;
      ConvertToArguments_g( table_.value(), argumentsValue );
      auto values_ = to_named_values_g( argumentsValue );
      runtime_.set_all_variables( std::move(values_) );
   }
   else if( m_arguments.empty() == false )
   {
      auto values_ = to_named_values_g( m_arguments );
      runtime_.set_all_variables( std::move(values_) );
   }

   runtime_.add( { (unsigned)uMethodDefaultSize_g, pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, pmethodString_g, std::string("str")});


   vectorToken.reserve( 12 );
   std::pair<bool, std::string> result = token::parse_s(stringExpression, vectorToken, tag_formula{});
   if( result.first == false ) { throw sol::error(result.second); }

   std::vector<token> vectorPostfix;
   vectorPostfix.reserve( vectorToken.size() );
   result = token::compile_s(vectorToken, vectorPostfix, tag_postfix{});
   if( result.first == false ) { throw sol::error(result.second); }



   auto [bOk, stringError] = token::calculate_s( vectorPostfix, &valueReturn, runtime_ );
   if( bOk == false ) throw sol::error( stringError );

   return ConvertToAny_g( valueReturn );
}

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------------ Sql
// ----------------------------------------------------------------------------

/// Get value for name ------------------------------------------------------
std::string Sql::GetValue( const std::string_view& stringName ) const
{
   auto iRow = m_psql->FindRowForColumnName( stringName ); // check if column name exists, if not exception is thrown
   if( iRow < 0 ) { throw sol::error( std::format( "column name not found: {}", stringName ) ); }

   std::string_view stringValue = m_psql->GetValue( (uint64_t)iRow );
   return std::string( stringValue );
}

/// Set all key value pairs. -------------------------------------------------
void Sql::AddValues( const sol::table& tableValues )
{
   auto argumentsValue = ConvertToArguments_g( tableValues );
   m_psql->AddColumn( argumentsValue );
}

void Sql::AddColumn( const sol::table& tableField )
{
   auto argumentsValue = ConvertToArguments_g( tableField );

   // ## Verify column information and that name for column is found, this is needed because column name is used to know what column to set value for
   unsigned uFlags = 0;
   auto result_ = m_psql->Validate( argumentsValue, &uFlags );  // validate that column names (keys)
   if( result_.first == false ) { throw sol::error( result_.second ); }
   if( (uFlags & CRENDERSql::eColumnFlagName) != CRENDERSql::eColumnFlagName ) { throw sol::error( "column name not found" ); }

   m_psql->AddColumn( argumentsValue );
}

void Sql::SetColumn( std::variant<uint64_t, std::string_view> column_, const sol::table& tableField )
{
   auto argumentsValue = ConvertToArguments_g( tableField );

   uint64_t uRow = 0;

   // ## Verify column information and that name for column is found, this is needed because column name is used to know what column to set value for
   auto result_ = m_psql->Validate( argumentsValue );                         // validate that column names (keys)
   if( result_.first == false ) { throw sol::error( result_.second ); }

   if( column_.index() == 1 )
   {
      auto iRow = m_psql->FindRowForColumnName( std::get<1>( column_ ) ); // check if column name exists, if not exception is thrown
      if( iRow == (unsigned)-1 ) { throw sol::error( std::format( "not found: {}", std::get<1>( column_ ) ) ); }
      uRow = (uint64_t)iRow;
   }
   else
   {
      uRow = std::get<0>( column_ );
      if( uRow >= m_psql->Size() ) { throw sol::error( std::format( "index out of range: {}, max: {}", uRow, m_psql->Size() - 1 ) ); }
   }

   m_psql->SetColumn( uRow, argumentsValue );
}

/// Remove column by index or name, return index of removed column, if column is not found then return -1
int64_t Sql::RemoveColumn( std::variant<uint64_t, std::string_view, sol::table> column_ )
{
   int64_t iRow = -1;
   if( column_.index() == 2 )
   {
      auto argumentsValue = ConvertToArguments_g( std::get<2>( column_ ) );
      iRow = m_psql->Find( argumentsValue ); 
   }
   else if( column_.index() == 1 )
   {
      iRow = m_psql->FindRowForColumnName( std::get<1>( column_ ) ); // check if column name exists, if not exception is thrown
   }
   else
   {
      uint64_t uRow = std::get<0>( column_ );
      if( uRow >= m_psql->Size() ) { throw sol::error( std::format( "index out of range: {}, max: {}", uRow, m_psql->Size() - 1 ) ); }
      iRow = (int64_t)uRow;
   }

   if( iRow >= 0 ) { m_psql->Remove( (uint64_t)iRow ); }
   return iRow;
}

/// Add condition to sql, this is used to add where condition to sql, for example "id = 1", "name = 'John'", etc.
void Sql::AddCondition( const sol::table& tableField )
{
   auto argumentsCondition = ConvertToArguments_g( tableField );

   // ## Verify column information and that name for column is found, this is needed because column name is used to know what column to set value for
   auto result_ = m_psql->ValidateCondition( argumentsCondition );  // validate that column names (keys)
   if( result_.first == false ) { throw sol::error( result_.second ); }

   m_psql->AddCondition( argumentsCondition );
}


/// @brief Build sql insert string -------------------------------------------
std::string Sql::AsInsert( std::optional<sol::table> table_ ) const
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsValue( buffer_ );

   if( table_.has_value() == true )
   {
      ConvertToArguments_g( table_.value(), argumentsValue );
      if( argumentsValue.exists("returning") == true ) { m_psql->AddProperty( "returning", argumentsValue["returning"].as_string() ); }

      if( argumentsValue.exists( "table" ) == true )
      {
         std::string stringTable = argumentsValue["table"].as_string();
         m_psql->FillColumn( CRENDERSql::eColumnFieldTable, stringTable );
      }
   }

   std::string stringInsert;
   auto result_ = m_psql->ToSqlInsert( stringInsert );
   if( result_.first == false ) { throw sol::error( result_.second ); }
   return stringInsert;
}

std::string Sql::AsSelect( std::optional<sol::table> table_ ) const
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsValue( buffer_ );
   if( table_.has_value() == true )
   {
      ConvertToArguments_g( table_.value(), argumentsValue );
      if( argumentsValue.exists( "table" ) == true )
      {
         std::string stringTable = argumentsValue["table"].as_string();
         m_psql->FillColumn( CRENDERSql::eColumnFieldTable, stringTable );
      }
   }
   std::string stringSelect;
   auto result_ = m_psql->ToSqlSelect( stringSelect );
   if( result_.first == false ) { throw sol::error( result_.second ); }
   return stringSelect;
}

/// Build sql string, this will return built sql string, if there is error then exception is thrown with error message.
std::string Sql::Build() const
{
   std::string stringResult;
   /*
   auto result_ = m_psqlbuilder->Build( stringResult );
   if( result_.first == false ) { throw sol::error( result_.second ); }
   */
   return stringResult;
}



// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- Database
// ----------------------------------------------------------------------------

Database::Database( sol::table tableOpen )
{
   m_pdatabase = nullptr;
   Open( tableOpen );
}

Database::~Database()
{
   if( m_pdatabase != nullptr && m_bOwner == true )
   {
      m_pdatabase->release();
   }
}


/** ----------------------------------------------------------------------------
 * @brief Open database (creates a database connection)
 * @param connect_ information needed to connect database
 */
void Database::Open( const std::variant<std::string_view, sol::table>& connect_ )
{
   Close();

   auto open_sqlite_( [] ( const std::string_view& stringOpen ) -> gd::database::database_i* {
      if( std::filesystem::exists( stringOpen ) == false ) throw sol::error( std::string( "Database file not found: ") + stringOpen.data() );

      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();  // create database interface
      auto [bOk, stringError] = pdatabase->open( stringOpen );                  // open database, for now this is hardcoded to open sqlite
      if( bOk == false )
      {
         pdatabase->release();
         throw sol::error( stringError );
      }
      return pdatabase;
   });

   if( connect_.index() == 0 )
   {
      std::string_view stringOpen = std::get<0>( connect_ );

      m_pdatabase = open_sqlite_( stringOpen );
      if( m_pdatabase != nullptr ) { m_bOwner = true; }

   }
   else
   {
      gd::argument::arguments argumentsOpen;
      ConvertToArguments_g( std::get<1>( connect_ ), argumentsOpen );

      if( argumentsOpen["file"].is_true() )
      {
         std::string stringOpen = argumentsOpen["file"].as_string();
         m_pdatabase = open_sqlite_( stringOpen );
         if( m_pdatabase != nullptr ) { m_bOwner = true; }
      }
   }
}

/// Execute sql statement, this is for sql statements that doesn't return value, for example insert, update, delete, create table, etc.
void Database::Execute( const std::string_view& stringSql )
{
   auto [bOk, stringError] = m_pdatabase->execute( stringSql );
   if( bOk == false ) { throw sol::error( stringError ); }
}

/// Execute sql statement and return value, this is for sql statements that return value or values, using `returns`
std::variant<int64_t, std::string, double, bool, sol::table, sol::lua_nil_t> Database::ExecuteReturn( const std::string_view& stringSql )
{                                                                                                  assert( m_pdatabase != nullptr );
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsKey( buffer_ );

   auto [bOk, stringError] = m_pdatabase->execute( stringSql, [&argumentsKey]( const auto* parguments_ ){ argumentsKey = *parguments_; return true; });
   if( bOk == false ) { throw sol::error( stringError ); }

   auto uCount = argumentsKey.count();
   if( uCount == 1 )
   {
      return ConvertToAnyWithTable_g( argumentsKey[0u].as_variant_view() );
   }
   else if( argumentsKey.size() > 0 )
   {
      return ConvertToAnyWithTable_g( argumentsKey );
   }
   else
   {
      return sol::lua_nil;
   }
}


/** ---------------------------------------------------------------------------
 * @brief Ask for value in database, this method will only take first value found from query
 * @param stringSql sql string used to ask for value
 * @param type_ optional type, if return type is to converted to some specific type
 * @return std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> value from query
*/
std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Database::Ask( const std::string_view& stringSql, std::optional<std::string> type_ )
{
   gd::com::pointer<gd::database::cursor_i> pcursor;
   m_pdatabase->get_cursor( &pcursor );

   auto [bOk, stringError] = pcursor->open( stringSql );
   if( bOk == false ) { throw sol::error( stringError ); }

   gd::variant variantValue;

   if( pcursor->is_valid_row() )
   {
      variantValue = pcursor->get_record()->get_variant( 0 );

      if( type_.has_value() )
      {
         variantValue.convert( type_.value() );
      }
   }
   
   return ConvertToAny_g( variantValue );
}

/// Close connection if open
void Database::Close()
{
   if( m_pdatabase != nullptr && m_bOwner == true )
   {
      m_bOwner = false;
      m_pdatabase->release();
   }
   m_pdatabase = nullptr;
}


// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- Cursor
// ----------------------------------------------------------------------------

void Cursor::Open( const std::string_view& stringSql, const sol::optional<sol::table>& params_ )
{
   if( params_.has_value() == false )
   {
      auto [bOk, stringError] = m_pcursor->open( stringSql );
      if( bOk == false ) { throw sol::error( stringError ); }
   }
   else
   {
      std::vector<gd::variant_view> vectorValue;
      ConvertTo_g( params_.value(), vectorValue );

      auto [ bOk, stringError ] = m_pcursor->prepare( stringSql, vectorValue );                    assert( bOk == true );
      if( bOk == false ) { throw sol::error( stringError ); }
      std::tie( bOk, stringError ) = m_pcursor->open();                                            assert( bOk == true );
      if( bOk == false ) { throw sol::error( stringError ); }
   }
}

void Cursor::Next()
{
   if( m_pcursor->is_open() == true )
   {
      m_pcursor->next();
   }
}

bool Cursor::IsValidRow() const
{
   if( m_pcursor->is_open() ) return m_pcursor->is_valid_row();
   return false;
}


void Cursor::Close()
{
   m_pcursor->close();
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Cursor::GetValue( const std::variant<int64_t,std::string_view>& column_ )
{
   int iColumn;
   gd::variant_view value_;

   if( column_.index() == 1 ) { iColumn = m_pcursor->get_record()->get_column_index_for_name( std::get<1>( column_ ) ); }
   else                       { iColumn = (int)std::get<0>( column_ ); }

   value_ = m_pcursor->get_record()->get_variant_view( unsigned(iColumn) );

   return ConvertToAny_g( value_ );
}

/** ----------------------------------------------------------------------------
 * @brief Fill table with result from cursor and return it

*sample lua code*
~~~.c
function ReadCodes( iGroup )
   local database = Database.new( {file=app:GetProperty("database")} )
   local sSelectCode = "SELECT CodeK, FName FROM TCode WHERE GroupK = " .. iGroup

   local cursor = Cursor.new( database )
   cursor:Open( sSelectCode )

   local tableCode = cursor:GetTable()
   return tableCode
end

function main()
   local tableCode = ReadCodes( 4 );
   print( tableCode:Write( { format="cli" } ) )
end

main()
~~~
 * @return Table table with result from cursor
 */
Table Cursor::GetTable()
{
   std::unique_ptr< gd::table::table_column_buffer > ptable = std::unique_ptr< gd::table::table_column_buffer >( new gd::table::table_column_buffer( 10 ) );

   if( m_pcursor->is_open() == true )
   {
      assert(false);
      //application::database::TABLE_Append_g( ptable.get(), m_pcursor );
   }
   else
   {
      throw sol::error( "cursor isn't open, no statement to process" );
   }
   return Table( ptable.release() );
}

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------ Document
// ----------------------------------------------------------------------------


Database Document::GetDatabase()
{                                                                                                  assert( m_pdatabase != nullptr );
   return Database( (void*)m_pdatabase );
}

Sql Document::CreateSql()
{
   return Sql( m_pdocument );
}



// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- Application
// ----------------------------------------------------------------------------

Application::Application() 
{ 
   m_papplication = papplication_g; 
}

Application::~Application()
{
   if( m_bOwner == true ) delete m_papplication;
}

Document Application::GetDocument()
{                                                                                                  assert( m_papplication != nullptr );
   if( m_pdatabase == nullptr ) return Document( (void*)m_papplication->GetDocument() );
   return Document( (void*)m_papplication->GetDocument(), m_pdatabase );
}

Database Application::GetDatabase()
{                                                                                                  assert( m_pdatabase != nullptr );
   return Database( (void*)m_pdatabase );
}

/// return number of properties in application
uint64_t Application::GetPropertyCount()
{                                                                                                  assert( m_papplication != nullptr );
   return (uint64_t)m_papplication->PROPERTY_Size();
}

/// Read application property
std::variant<int64_t,std::string, double, bool> Application::GetProperty( const std::variant<int64_t,std::string>& index_ )
{                                                                                                  assert( m_papplication != nullptr );
   decltype( m_papplication->PROPERTY_Get("")) value_;

   if( index_.index() == 1 ) value_ = m_papplication->PROPERTY_Get( std::get<1>( index_ ) );
   else                      value_ = m_papplication->PROPERTY_Get( std::get<0>( index_ ) );

   if( value_.is_string() == true ) return value_.as_string();
   else if( value_.is_integer() == true ) return value_.as_int64();
   else if( value_.is_decimal() == true ) return value_.as_double();
   else { return value_.as_bool(); }
                                                                                                   assert( false );
   return false;
}

/// Return property name for property index
std::string Application::GetPropertyName( uint64_t uIndex ) const
{                                                                                                  assert( uIndex < m_papplication->PROPERTY_Size() );
   return m_papplication->PROPERTY_GetName( uIndex );
}

/// Set application property
void Application::SetProperty( const std::string_view& stringName, std::variant<int64_t,std::string, double> value_ )
{
   m_papplication->PROPERTY_Set( stringName, gd::to_variant_view_g( value_, gd::variant_type::tag_std_variant{}));
}

/// Set log level
void Application::SetLogLevel( const std::string_view& stringLevel )
{
   /*
   char chLevel = stringLevel[0];
   if( chLevel >= 'a' ) chLevel = chLevel - ( 'a' - 'A' );
   m_papplication->LOG_SetLevel_s( chLevel );
   */
}

/// Initialize application object.
/// May be used if you do not have any prepared application object 
void Application::Initialize( std::optional<std::string> connect_database_ )
{
   if( m_papplication == nullptr )
   {
      m_papplication = new CApplication();
      m_bOwner = true;
   }
}

/** ---------------------------------------------------------------------------
 * @brief prints message using application message logic
 * 
 * Sample lua code using the `Message` method
 @code
 function main()
 app:Message("=== Execute main")
 app:Message("ERROR", "--- This is not an error")
 app:Message("DEBUG", "--- Debug message")
 end

 main()
 @endcode
 * @param stringTypeOrMessage If only one parameter this is the message, more than one this is the type
 * @param message_ If type for message is set then this has the printed message
 */
void Application::Message(const std::string_view & stringTypeOrMessage, std::optional<std::string> message_)
{
   std::string stringType;
   std::string_view stringMessage;
   if( message_.has_value() )
   {
      stringMessage = message_.value();
      stringType = stringTypeOrMessage;
      //m_papplication->MESSAGE_Print( stringMessage, stringType );
   }
   else
   {
      //m_papplication->MESSAGE_Print( stringTypeOrMessage );
   }
}

// ----------------------------------------------------------------------------
// -------------------------------------------------------------------- Request
// ----------------------------------------------------------------------------

Request::Request( CAPIContext* pcontext ) 
{ 
   m_pcontext = pcontext; 
   auto* pdocument = m_pcontext->GetDocument();
   if( pdocument != nullptr )
   {
      m_psql = std::move( std::make_unique<CRENDERSql>( pdocument ) );
      m_psql->Initialize();
   }
}

Application Request::GetApplication()
{
   return Application( m_pcontext->GetApplication(), m_pcontext->GetDatabase() );
}
   
Document Request::GetDocument()
{
   return Document( m_pcontext->GetDocument(), m_pcontext->GetDatabase() );
}

/// Get database from request
Database Request::GetDatabase()
{
   return Database( (void*)m_pcontext->GetDatabase() );
}

/// Get IP address from request, if session is available
std::string Request::GetIpAddress()
{
   if( m_pcontext->HasSession() == true ) { return m_pcontext->GetIpAddress(); }
   return {};
}

/// Get session ID from request, if session is available
std::string Request::GetSessionId()
{
   if( m_pcontext->HasSession() == true ) { return m_pcontext->GetSessionId(); }
   return {};
}


/// @brief Get script value from Request, these values can be used to fill in missing values
std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Request::GetScriptValue( std::string_view stringName, std::optional<std::string> type_ )
{
   gd::variant_view variantValue = m_pcontext->GetGlobal( stringName );
   if( type_.has_value() == true )
   {
      gd::variant variantValueTemp = variantValue.as_variant();
      bool bOk = variantValueTemp.convert( type_.value() );
      if( bOk == false ) { throw sol::error( std::format( "Failed to convert value to type: {}", type_.value() ) ); }
      return ConvertToAny_g( variantValueTemp );
   }
   return ConvertToAny_g( variantValue );
}

/// @brief Set global variable to Request, these values can be used to fill in missing values
void Request::SetScriptValue( std::string_view stringName, std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> value_, std::optional<std::string> type_ )
{
   gd::variant variantValue = ConvertFromAny_g( value_ );
   if( type_.has_value() == true )
   {
      bool bOk = variantValue.convert( type_.value() );
      if( bOk == false ) { throw sol::error( std::format( "Failed to convert value to type: {}", type_.value() ) ); }
   }

   m_pcontext->SetGlobal( stringName, variantValue.as_variant_view() );
}

/// @brief Create SQL object that can be used to build sql string
Sql Request::CreateSql()
{
   return Sql( m_psql.get() );
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Request::GetClientValue( std::string_view stringName, std::optional<std::string> type_)
{
   if( m_psql == nullptr ) { throw sol::error( "SQL object not initialized" ); }
   auto iRow = m_psql->FindRowForColumnName( stringName );                    // check if column name exists, if not exception is thrown
   if( iRow < 0 ) { throw sol::error( std::format( "column name not found: {}", stringName ) ); }

   std::string_view stringValue = m_psql->GetValue( iRow );
   if( type_.has_value() == true )
   {
      gd::variant variantValue( stringValue );
      bool bOk = variantValue.convert( type_.value() );
      if( bOk == false ) { throw sol::error( std::format( "Failed to convert value to type: {}", type_.value() ) ); }
      return ConvertToAny_g( variantValue );
   }
  
   return ConvertToAny_g( gd::variant( stringValue ) );
}

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- Response
// ----------------------------------------------------------------------------


void Response::Message( const std::string_view& stringTypeOrMessage, std::optional<std::string> message_ )
{
   std::string_view stringType = message_.has_value() ? "" : stringTypeOrMessage;
   std::string_view stringMessage = message_.has_value() ? message_.value() : stringTypeOrMessage;
   
   m_papicontext->GetObjects()->Add( stringMessage );
}

LUA_END