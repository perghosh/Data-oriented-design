// @FILE [tag: query] [description: http session] [type: source] [name: METAQueries.cpp]

#include <format>

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

/** ---------------------------------------------------------------------------
 * @brief Adds a new query to the table with the specified ID, type, format, and query.
 *
 * @param stringId The ID of the query.
 * @param stringType The type of the query.
 * @param stringFormat The format of the query.
 * @param stringQuery The query itself.
 * @return std::pair<bool, std::string> A pair containing a boolean indicating success and the ID of the added query.
 */
std::pair<bool, std::string> CQueries::Add( std::string_view stringId, std::string_view stringType, std::string_view stringFormat, std::string_view stringQuery )
{
   if( stringId.empty() || stringType.empty() || stringQuery.empty() ) { return { false, "Invalid input" }; }

   auto uType = ToType_s( stringType );
   auto uFormat = ToFormat_s( stringFormat );

   if( uType == eTypeUnknown ) { return { false, "Invalid type" }; }

   auto uRow = m_tableQuery.row_add_one();

   gd::uuid uuid_( gd::types::tag_command_random{} );
   gd::types::uuid uuidQuery( uuid_.data() );


   m_tableQuery.cell_set( uRow, "id", uuidQuery );
   m_tableQuery.cell_set( uRow, "type", uType );
   m_tableQuery.cell_set( uRow, "format", uFormat );
   m_tableQuery.cell_set( uRow, "query", stringQuery );

   std::string stringUuid = gd::binary_to_hex_g( uuidQuery.data(), 16, false );

   return { true, stringUuid };
}

std::pair<bool, std::string> CQueries::Delete( const std::pair<std::string_view, std::string_view>& pair_ )
{
   std::string_view stringName = pair_.first;
   std::string_view stringUuid = pair_.second;
   int64_t iRow = -1;
   if( stringName.empty() == false) { iRow = m_tableQuery.find( eColumnName, stringName ); }
   if(iRow == -1)
   {
      gd::uuid uuid_( stringUuid );
      gd::types::uuid uuidFind( uuid_.data() );
      iRow = m_tableQuery.find( eColumnId, uuidFind );
   }

   if( iRow == -1 ) return { false, std::format( "No row for {} or {}", stringName, stringUuid)};

   return { true, "" };
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

uint16_t CQueries::ToType_s( std::string_view stringType )
{
   if( stringType == "select" ) return eTypeSelect;
   if( stringType == "insert" ) return eTypeInsert;
   if( stringType == "update" ) return eTypeUpdate;
   if( stringType == "delete" ) return eTypeDelete;
   if( stringType == "ask" ) return eTypeAsk;
   if( stringType == "batch" ) return eTypeBatch;
   return eTypeUnknown;
}

uint16_t CQueries::ToFormat_s( std::string_view stringFormat )
{
   if( stringFormat == "text" ) return eFormatText;
   if( stringFormat == "jinja" ) return eFormatJinja;
   if( stringFormat == "json" ) return eFormatJson;
   if( stringFormat == "xml" ) return eFormatXml;
   return eFormatText;
}


NAMESPACE_META_END
