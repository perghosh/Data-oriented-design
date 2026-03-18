#include "gd__statement.h"

_GD_MODULES_DBMETA_BEGIN

std::pair<bool, std::string> statement::add_statement( std::string_view stringName, std::string_view stringStatement, enumFormat eFormat, uint32_t uType, uint32_t uRule )
{                                                                                                  assert( m_ptableStatement != nullptr );
   auto uRow = m_ptableStatement->row_add_one();
   uint32_t uKey = (uint32_t)uRow;
   m_ptableStatement->cell_set( uRow, "key", (uint32_t)uKey );
   m_ptableStatement->cell_set( uRow, "uuid", gd::types::uuid_generate_g() );
   m_ptableStatement->cell_set( uRow, "name", stringName );
   m_ptableStatement->cell_set( uRow, "type", (uint32_t)uType );
   m_ptableStatement->cell_set( uRow, "format", (uint32_t)eFormat );
   m_ptableStatement->cell_set( uRow, "rule", uRule );
   m_ptableStatement->cell_set( uRow, "statement", stringStatement );
   return { true, "" }; 
}



void statement::create_statement_s( gd::table::arguments::table& tableStatement )
{
   // ## Create table with meta information about statements

   tableStatement.set_flags( gd::table::tag_meta{} );   
   tableStatement.column_prepare();
   tableStatement.column_add( {
      { "uint32",   0, "key"         }, // key (these keys do also represent row number to be fast)
      { "uuid",     0, "uuid"        }, // unique identifier for statement
      { "string",  32, "name"        }, // statement name, this is used to identify statement and also used in query templates to refer to statement
      { "uint32",   0, "type"        }, // type of statement, describes what kind of statement this is, like select, insert, update, delete, or other types of statements if needed
      { "uint32",   0, "format"      }, // statement format, this is used to determine how to generate sql statement, and how to parse arguments for statement
      { "uint32",   0, "rule"        }, // statement rule, this is used to determine how to use statement, and what is allowed for statement
      { "rstring",  0, "statement"   }, // raw data for statement, this is used to store the actual sql statement template, this can also be used to store other types of statements if needed
   }, gd::table::tag_type_name{});
   tableStatement.prepare();
}


_GD_MODULES_DBMETA_END