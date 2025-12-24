// @FILE [tag: session] [description: http session] [type: source] [name: Session.cpp]

#include <chrono>

#include "Application.h"

#include "Session.h"

uint64_t GetTime()
{
   uint64_t uMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(
       std::chrono::system_clock::now().time_since_epoch()
   ).count();

   return uMilliSeconds;
}

void CSessions::Initialize( size_t uMaxCount )
{
   CreateTable_s( m_tableSession );
   m_tableSession.row_reserve_add( uMaxCount );
}

gd::uuid CSessions::Add()
{
   // ## create new uuid
   gd::uuid uuidNew = gd::uuid::new_uuid_s();

   // ## add session to table but remember to lock table during operation
   {
      std::scoped_lock lock( m_mutexTable );
      uint64_t uRow = m_tableSession.row_add_one();
      m_tableSession.cell_set( uRow, "uuid", gd::types::binary( uuidNew.data(), 16 ) );
   }

   return uuidNew;
}

std::pair<bool, std::string> CSessions::Add( const gd::uuid& uuid_ )
{
   return { true, "" };
}

gd::uuid CSessions::At( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   gd::uuid uuid_;
   const auto* pbValue = m_tableSession.cell_get( 0, uIndex );
   uuid_ = pbValue;
   return uuid_;
}

void CSessions::Update( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   uint64_t uTime = GetTime();
   m_tableSession.cell_set( uIndex, CSessions::eColumnTime, uTime );
}




/** --------------------------------------------------------------------------
 * @brief Initializes and prepares a session table with predefined columns and metadata flags.
 * @param tableSession A reference to a table object that will be configured for session data.
 */
void CSessions::CreateTable_s( gd::table::arguments::table& tableSession )
{                                                                                                  assert( tableSession.empty() == true );
   tableSession.set_flags( gd::table::tag_meta{} );
   tableSession.column_add( {{ "uuid", 0, "id"}, { "uint64", 0, "time" }, { "uint64", 0, "ip4" }, { "uint64", 0, "ip6" } }, gd::table::tag_type_name{});
   tableSession.prepare();
}