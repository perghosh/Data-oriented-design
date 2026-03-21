// @FILE [tag: query] [description: http session] [type: source] [name: METAQueries.cpp]

#include <format>

#include "gd/gd_binary.h"
#include "gd/gd_uuid.h"

#include "METAQueries.h"

NAMESPACE_META_BEGIN

void CQueries::common_construct(const CQueries &o)
{
   m_argumentProperty = o.m_argumentProperty;
   //m_tableQuery = o.m_tableQuery;

   // @TODO: Complete this, with copying statement object, this is not implemented yet
}



std::pair<bool, std::string> CQueries::Initialize( const gd::argument::arguments& arguments_ )
{
   std::lock_guard<std::mutex> lock_(m_mutexStatement);

   if( m_statement.empty() == true ) 
   { 
      m_statement.initialize();
   }

   //CQueries::CreateTable_s( m_tableQuery );
   return { true, "" };
}

std::pair<bool, std::string> CQueries::Add( std::string_view stringQuery, enumFormat eFormat, const gd::argument::arguments* parguments_ )
{
/*
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
   */

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new query to the table with the specified ID, type, format, and query.
 *
 * @param stringName Name for query.
 * @param stringType The type of the query.
 * @param stringFormat The format of the query.
 * @param stringQuery The query itself.
 * @return std::pair<bool, std::string> A pair containing a boolean indicating success and the ID of the added query.
 */
std::pair<bool, std::string> CQueries::Add( std::string_view stringName, std::string_view stringType, std::string_view stringFormat, std::string_view stringQuery )
{
   if( stringType.empty() == true ) stringType = "select";
   if( stringQuery.empty() ) { return { false, "Invalid input" }; }

   std::lock_guard<std::mutex> lock_(m_mutexStatement);

   auto result_ = m_statement.add( stringName, stringQuery, stringFormat, stringType ); // TODO: Compmlete this, new logic
   if( result_.first == false ) { return { false, result_.second }; }

   return result_;
}

std::pair<bool, std::string> CQueries::Delete( const std::pair<std::string_view, std::string_view>& pair_ )
{
   std::lock_guard<std::mutex> lock_(m_mutexStatement);

   std::string_view stringName = pair_.first;
   std::string_view stringUuid = pair_.second;
   int64_t iRow = -1;
   if( stringName.empty() == false) { iRow = m_statement.find( stringName ); }
   if(iRow == -1)
   {
      gd::uuid uuid_( stringUuid );
      gd::types::uuid uuidFind( uuid_.data() );
      iRow = m_statement.find( uuidFind );
   }

   if( iRow == -1 ) return { false, std::format( "No row for {} or {}", stringName, stringUuid)};

   m_statement.erase( iRow );

   return { true, "" };
}

int64_t CQueries::Find( const gd::argument::arguments& arguments_ ) const
{
   if( arguments_.exists( "name" ) == true )
   {
      std::string_view stringName = arguments_["name"].get<std::string_view>();
      if( stringName.empty() == false )
      {
         int64_t iRow = m_statement.find( stringName );
         if( iRow != -1 ) return iRow;
      }
   }
   /*
   gd::uuid uuidFind;
   std::string_view stringUuid = arguments_.get<std::string_view>( "id" );
   if( stringUuid.empty() == false )
   {
      uuidFind = gd::uuid( stringUuid );
      gd::types::uuid uuidFindType( uuidFind.data() );
      int64_t iRow = m_tableQuery.find( eColumnId, uuidFindType );
      if( iRow != -1 ) return iRow;
   }
   */
   return -1;
}

/// @brief Retrieves the UUID of a query based on its row index.
gd::types::uuid CQueries::GetQueryId( uint64_t uRow )
{
   gd::types::uuid uuidId = m_statement.get_id( uRow );

   return uuidId;
}

std::pair<bool, std::string> CQueries::GetQuery( std::string_view stringName, std::string& stringQuery )
{
   auto iIndex = m_statement.find( stringName );
   if( iIndex != -1 )
   {
      stringQuery = m_statement.get_statement( iIndex );
      return { true, "" };
   }

   return { false, std::format( "No query found for name '{}'", stringName ) };
}

std::pair<bool, std::string> CQueries::Load( std::string_view stringPath )
{
    
   return { true, "" };
}


NAMESPACE_META_END
