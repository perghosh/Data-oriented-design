// @FILE [tag: query] [description: http session] [type: source] [name: METAQueries.cpp]

#include "gd/gd_binary.h"
#include "gd/gd_uuid.h"

#include "METAQueries.h"

NAMESPACE_META_BEGIN

void CQueries::common_construct(const CQueries &o)
{
   m_argumentProperty = o.m_argumentProperty;
   m_tableQuery = o.m_tableQuery;
}



std::pair<bool, std::string> CQueries::Initialize( const gd::argument::arguments& arguments_ )
{
   CQueries::CreateTable_s( m_tableQuery );
   return { true, "" };
}

std::pair<bool, std::string> CQueries::Add( std::string_view stringQuery, enumFormat eFormat, const gd::argument::arguments* parguments_ )
{
   auto uRow = m_tableQuery.row_add_one();
   gd::uuid uuidQuery( gd::types::tag_command_random{});
   gd::types::uuid uuidQueryAdd( uuidQuery.data() );
   m_tableQuery.cell_set( uRow, "type", (uint16_t)eFormat );
   m_tableQuery.cell_set( uRow, "uuid", uuidQueryAdd );
   uint16_t uFlags = 0;
   m_tableQuery.cell_set( uRow, "flags", gd::variant_view( uFlags ) );
   m_tableQuery.cell_set( uRow, "query", gd::variant_view( stringQuery ) );

   if( parguments_ != nullptr )
   {
   }

   std::string stringId = gd::binary_to_hex_g( uuidQuery.data(), 16, false );

   return { true, stringId };
}


/** --------------------------------------------------------------------------
 * @brief Initializes and prepares a query table with predefined columns and metadata flags.
 * 
 * Queries can be stored in raw text format with jinja templates to prepare query.
 * json and xml format is also supported. There queries are stored with each column in elements or similar in json.
 * 
 * @param tableQuery A reference to a table object that will be configured for query data.
 */
void CQueries::CreateTable_s( gd::table::arguments::table& tableQuery )
{                                                                                                  assert( tableQuery.empty() == true );
   tableQuery.set_flags( gd::table::tag_meta{} );
   tableQuery.column_prepare();
   tableQuery.column_add( {{ "uuid", 0, "id"}, { "uint16", 0, "flags" }, { "uint16", 0, "type" }, { "rstring", 0, "name" }, { "rstring", 0, "query" }, { "rstring", 0, "meta" } }, gd::table::tag_type_name{});
   tableQuery.prepare();
}

NAMESPACE_META_END
