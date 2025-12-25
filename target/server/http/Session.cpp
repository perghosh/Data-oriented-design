// @FILE [tag: session] [description: http session] [type: source] [name: Session.cpp]

#include <chrono>

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

/// Get the UUID at the specified index
gd::uuid CSessions::At( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   gd::uuid uuid_;
   const auto* pbValue = m_tableSession.cell_get( (uint64_t)uIndex, 0 );
   uuid_ = pbValue;
   return uuid_;
}

/** --------------------------------------------------------------------------
 * @brief Updates the timestamp of a session to the current time.
 * @param uIndex The index of the session to update.
 * 
 * This method refreshes the last active time for a session by setting the time
 * column to the current timestamp. This is typically called when the client
 * associated with the session makes a new request, keeping the session alive.
 * 
 * Note: This method is not thread-safe. If thread safety is required, the caller
 * should acquire a lock on m_mutexTable before calling this method.
 * 
 * @see GetTime_s() for the timestamp format used
 */
void CSessions::Update( size_t uIndex )
{                                                                                                  assert( uIndex < m_tableSession.size() );
   uint64_t uTime = GetTime_s();                                               // Get the current time in milliseconds since epoch
   
   m_tableSession.cell_set( uIndex, CSessions::eColumnTime, uTime );           // Update the session's timestamp in the time column (eColumnTime = 1)
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


/** ---------------------------------------------------------------------------
 * @brief Purges expired sessions from the table.
 * @param uCurrentTimeMs The current time in milliseconds.
 * @param uExpireLimitMs The maximum age of a session in milliseconds.
 */
void CSessions::Purge(uint64_t uCurrentTimeMs, uint64_t uExpireLimitMs)
{                                                                                                  assert(uExpireLimitMs < (10LL * 365 * 24 * 60 * 60 * 1000) && "realistic? should not be more than 10 years...");
   // Calculate the expiration threshold in milliseconds
   // Sessions older than this timestamp will be purged
   uint64_t uExpireThresholdMs = uCurrentTimeMs - uExpireLimitMs;             // `uExpireThresholdMs` now holds the cutoff timestamp
   
   // ## Iterate through all sessions and clear expired ones
   for(uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow)
   {
      if( m_tableSession.cell_is_null(uRow, eColumnTime) == false )
      {
         uint64_t uSessionTimeMs = m_tableSession.cell_get<uint64_t>(uRow, eColumnTime);
         
         if(uSessionTimeMs < uExpireThresholdMs)                               // If session timestamp is older than the expire threshold, clear it
         {
            Clear(uRow);
         }
      }
   }
}

/// Count active sessions
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

/** ---------------------------------------------------------------------------
 * Count active sessions within a specified time range.
 *
 * @param uCurrentTimeMs The current time in milliseconds.
 * @param uExpireLimitMs The maximum allowed age of a session in milliseconds.
 * @return The number of active sessions within the specified time range.
 */
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

/** --------------------------------------------------------------------------
 * @brief Convert days to milliseconds for session expiration calculations
 * @param uDays Number of days to convert
 * @return Equivalent time in milliseconds
 */
uint64_t CSessions::DaysToMs_s(uint64_t uDays)
{
   // 1 day = 24 hours = 24 * 60 minutes = 24 * 60 * 60 seconds = 24 * 60 * 60 * 1000 milliseconds
   return uDays * 24ULL * 60ULL * 60ULL * 1000ULL;
}

/** --------------------------------------------------------------------------
 * @brief Convert hours to milliseconds for session expiration calculations
 * @param uHours Number of hours to convert
 * @return Equivalent time in milliseconds
 */
uint64_t CSessions::HoursToMs_s(uint64_t uHours)
{
   // 1 hour = 60 minutes = 60 * 60 seconds = 60 * 60 * 1000 milliseconds
   return uHours * 60ULL * 60ULL * 1000ULL;
}
