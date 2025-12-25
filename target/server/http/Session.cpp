// @FILE [tag: session] [description: http session] [type: source] [name: Session.cpp]

#include <chrono>

#include "Application.h"

#include "Session.h"

void CSessions::Initialize( size_t uMaxCount )
{
   CreateTable_s( m_tableSession );
   m_tableSession.row_reserve_add( uMaxCount );
}

gd::uuid CSessions::Add( uint64_t* puIndex )
{
   // ## create new uuid
   gd::uuid uuidNew = gd::uuid::new_uuid_s();

   auto result_ = Add( uuidNew, puIndex );
   if( result_.first == false ) { throw std::runtime_error( result_.second ); }

   return uuidNew;
}

std::pair<bool, std::string> CSessions::Add( const gd::uuid& uuid_, uint64_t* puIndex )
{
   {
      std::scoped_lock lock_( m_mutexTable );
      
      // Find first free row (time == 0)
      for( uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow )
      {
         if( m_tableSession.cell_is_null( uRow, eColumnTime ) == true )
         {
            // Claim it
            uint64_t uNow = GetTime_s();
            m_tableSession.cell_set( uRow, eColumnId, gd::types::binary( uuid_.data(), 16 ) );
            m_tableSession.cell_set( uRow, eColumnTime, uNow );
            
            if( puIndex ) *puIndex = uRow;
            return { true, "" };
         }
      }
   }
   
   return { false, "CSessions::Add: no free sessions available" };            // no free session found, you shouldn't be here....
}

gd::uuid CSessions::At( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   gd::uuid uuid_;
   const auto* pbValue = m_tableSession.cell_get( (uint64_t)uIndex, 0 );
   uuid_ = pbValue;
   return uuid_;
}

void CSessions::Update( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   uint64_t uTime = GetTime_s();
   m_tableSession.cell_set( uIndex, CSessions::eColumnTime, uTime );
}

/** --------------------------------------------------------------------------
 * @brief Clears the session data at the specified index.
 * @param uIndex The index of the session to clear.
 */
void CSessions::Clear( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   m_tableSession.row_arguments_delete( (uint64_t)uIndex );                   // clear arguments for row if set
   m_tableSession.row_set_null( (uint64_t)uIndex );                           // set row to null
}


void CSessions::Purge(uint64_t uCurrentTimeMs, uint64_t uExpireLimitMs)
{                                                                                                  assert(uExpireLimitMs < (10LL * 365 * 24 * 60 * 60 * 1000) && "realistic? should not be more than 10 years...");
   // Calculate the expiration threshold in milliseconds
   // Sessions older than this timestamp will be purged
   uint64_t uExpireThresholdMs = uCurrentTimeMs - uExpireLimitMs;             // `uExpireThresholdMs` now holds the cutoff timestamp
   
   // Iterate through all sessions and clear expired ones
   for(uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow)
   {
      if( m_tableSession.cell_is_null(uRow, eColumnTime) == false )
      {
         uint64_t uSessionTimeMs = m_tableSession.cell_get<uint64_t>(uRow, eColumnTime);
         
         // If session timestamp is older than the expire threshold, clear it
         if(uSessionTimeMs < uExpireThresholdMs)
         {
            Clear(uRow);
         }
      }
   }
}

size_t CSessions::CountActive() const
{
   size_t uCount = 0;
   
   // Count non-null (active) sessions
   for(uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow)
   {
      if(!m_tableSession.cell_is_null(uRow, eColumnTime))
      {
         uCount++;
      }
   }
   
   return uCount;
}

size_t CSessions::CountActive(uint64_t uCurrentTimeMs, uint64_t uExpireLimitMs) const
{                                                                                                  assert(uExpireLimitMs < (10LL * 365 * 24 * 60 * 60 * 1000) && "realistic? should not be more than 10 years...");
   size_t uCount = 0;
   uint64_t uExpireThresholdMs = uCurrentTimeMs - uExpireLimitMs;             // `uExpireThresholdMs` now holds the cutoff timestamp
   
   // Count sessions that are active and not expired
   for(uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow)
   {
      if(m_tableSession.cell_is_null(uRow, eColumnTime) == false)
      {
         uint64_t uSessionTimeMs = m_tableSession.cell_get<uint64_t>(uRow, eColumnTime);
         
         // Session is active if its time is after the expire threshold
         if(uSessionTimeMs >= uExpireThresholdMs) { uCount++; }
      }
   }
   
   return uCount;
}

size_t CSessions::CountActive(uint64_t uExpireLimitMs) const
{
   auto uTime = GetTime_s();

   return CountActive(uTime, uExpireLimitMs);
}

size_t CSessions::CountExpired(uint64_t uCurrentTimeMs, uint64_t uExpireLimitMs) const
{                                                                                                  assert(uExpireLimitMs < (10LL * 365 * 24 * 60 * 60 * 1000) && "realistic? should not be more than 10 years...");
   size_t uCount = 0;
   uint64_t uExpireThresholdMs = uCurrentTimeMs - uExpireLimitMs;             // `uExpireThresholdMs` now holds the cutoff timestamp
   
   // Count sessions that are expired
   for(uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow)
   {
      if(m_tableSession.cell_is_null(uRow, eColumnTime) == false)
      {
         uint64_t uSessionTimeMs = m_tableSession.cell_get<uint64_t>(uRow, eColumnTime);
         
         // Session is expired if its time is before the expire threshold
         if(uSessionTimeMs < uExpireThresholdMs) { uCount++; }
      }
   }
   
   return uCount;
}

size_t CSessions::CountExpired(uint64_t uExpireLimitMs) const
{
   auto uTime = GetTime_s();
   return CountExpired(uTime, uExpireLimitMs);
}

std::pair<bool, uint64_t> CSessions::FindFirstFree(uint64_t uOffset)
{
   std::scoped_lock lock_(m_mutexTable);
   
   // Find first free row (time == 0/null) starting from uOffset
   for(uint64_t uRow = uOffset; uRow < m_tableSession.size(); ++uRow)
   {
      if(m_tableSession.cell_is_null(uRow, eColumnTime))
      {
         return {true, uRow};
      }
   }
   
   return {false, 0};
}



/** --------------------------------------------------------------------------
 * @brief Initializes and prepares a session table with predefined columns and metadata flags.
 * @param tableSession A reference to a table object that will be configured for session data.
 */
void CSessions::CreateTable_s( gd::table::arguments::table& tableSession )
{                                                                                                  assert( tableSession.empty() == true );
   tableSession.set_flags( gd::table::tag_meta{} );
   tableSession.column_add( {{ "uuid", 0, "id"}, { "uint64", 0, "time" }, { "uint64", 0, "ip4" }, { "uint64", 0, "ip6" }, { "uint64", 0, "data" } }, gd::table::tag_type_name{});
   tableSession.prepare();
}

uint64_t CSessions::GetTime_s()
{
   uint64_t uMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(
       std::chrono::system_clock::now().time_since_epoch()
   ).count();

   return uMilliSeconds;
}
