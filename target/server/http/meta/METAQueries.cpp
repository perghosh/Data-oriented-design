// @FILE [tag: query] [description: http session] [type: source] [name: METAQueries.cpp]

#include "METAQueries.h"

NAMESPACE_META_BEGIN

void CQueries::common_construct(const CQueries &o)
{
   m_argumentProperty = o.m_argumentProperty;
   m_tableQuery = o.m_tableQuery;
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
   tableQuery.column_add( {{ "uuid", 0, "id"}, { "uint16", 0, "flags" }, { "uint16", 0, "type" }, { "rstring", 0, "query" }, { "rstring", 0, "meta" } }, gd::table::tag_type_name{});
   tableQuery.prepare();
}

NAMESPACE_META_END
