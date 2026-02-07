// @FILE [tag: session] [description: http session] [type: source] [name: Session.cpp]

#include <chrono>

#include "gd/gd_binary.h"

#include "Session.h"

CSessions::~CSessions()
{

}

void CSessions::Initialize( size_t uMaxCount )
{
   CreateTable_s( m_tableSession );
   m_tableSession.row_reserve_add( uMaxCount );
   m_tableSession.row_add( uMaxCount, gd::table::tag_null{});
}

gd::uuid CSessions::Add( uint64_t* puIndex )
{
   // ## create new uuid
   gd::uuid uuidNew = gd::uuid::new_uuid_s();

   auto result_ = Add( uuidNew, puIndex );
   if( result_.first == false ) { throw std::runtime_error( result_.second ); }

   return uuidNew;
}

int64_t CSessions::Add( const gd::types::uuid& uuid_ )
{
   std::scoped_lock lock_( m_mutexTable );
   
   auto row_ = FindFirstFree();
   if( row_.first == true )
   {
      uint64_t uRow = (uint64_t)row_.second;
      uint64_t uNow = GetTime_s();
      m_tableSession.cell_set( uRow, eColumnId, uuid_ );
      m_tableSession.cell_set( uRow, eColumnTime, uNow );
      return (int64_t)uRow;
   }
   
   return -1;
}

int64_t CSessions::AddLast( const gd::types::uuid& uuid_ )
{
   std::scoped_lock lock_( m_mutexTable );

   int64_t iRow = m_tableSession.size() - 1;

   // ## Find first free row from the end
   while( iRow >= 0 )
   {
      if( m_tableSession.cell_is_null( (uint64_t)iRow, eColumnTime ) == true )
      {
         break;
      }
      --iRow;
   }

   if( iRow < 0 ) { return -1; } // no free row found

   uint64_t uNow = GetTime_s();
   m_tableSession.cell_set( (uint64_t)iRow, eColumnId, uuid_ );
   m_tableSession.cell_set( (uint64_t)iRow, eColumnTime, uNow );

   return iRow;
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
            m_tableSession.cell_set( uRow, eColumnId, gd::types::uuid( uuid_.data() ) );
            m_tableSession.cell_set( uRow, eColumnTime, uNow );
            
            if( puIndex ) *puIndex = uRow;
            return { true, "" };
         }
      }
   }
   
   return { false, "CSessions::Add: no free sessions available" };            // no free session found, you shouldn't be here....
}

std::pair<bool, std::string> CSessions::Add( std::string_view stringUuid, uint64_t* puIndex )
{
   gd::uuid uuid_;

   try
   {
      uuid_ = gd::uuid( stringUuid.data(), stringUuid.data() + stringUuid.length() );
      return Add( uuid_, puIndex );
   }
   catch( const std::exception& e )
   {
      return { false, std::string( "CSessions::Add: invalid uuid format: " ) + e.what() };
   }

   return { false, "CSessions::Add: unknown error" };
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
   if( m_tableSession.row_is_arguments( (uint64_t)uIndex ) == true )
   {
      m_tableSession.row_arguments_delete( (uint64_t)uIndex );                // clear arguments for row if set
   }
   m_tableSession.row_set_null( (uint64_t)uIndex );                           // set row to null
}

/// Delete session if found for uuid
bool CSessions::Delete( const gd::types::uuid& uuid_ )
{
   int64_t iRow = Find(uuid_);
   if(iRow != -1)
   {
      Clear(iRow);
      return true;
   }
   
   return false;
}

/// Delete session at index
bool CSessions::Delete( uint64_t uRow )
{
   if( uRow < SizeMax() )
   {
      Clear(uRow);
      return true;
   }
   
   return false;
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

/// Find the row index of a session with the given UUID and return its row index, if not found return -1
int64_t CSessions::Find( const gd::types::uuid& uuid_ )
{
   for(uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow)
   {
      if(m_tableSession.cell_is_null(uRow, eColumnId) == false)
      {
         gd::types::uuid uuid = m_tableSession.cell_get<gd::types::uuid>(uRow, eColumnId);
         
         if(uuid == uuid_) { return uRow; }
      }
   }
   
   return -1;
}

/// Find first
std::pair<bool, uint64_t> CSessions::FindFirstFree(uint64_t uOffset)
{
   // Find first free row (time == 0/null) starting from uOffset
   for(uint64_t uRow = uOffset; uRow < m_tableSession.size(); ++uRow)
   {
      if(m_tableSession.cell_is_null(uRow, eColumnTime) == true)
      {
         return {true, uRow};
      }
   }
   
   return {false, 0};
}

void CSessions::Copy( gd::table::dto::table& table_ )
{
   // ## If table is empty then create new table

   if( table_.empty() == true )
   {
      table_ = std::move( gd::table::dto::table( m_tableSession.get_columns() ) );
      table_.set_flags( m_tableSession.get_flags() & gd::table::dto::table::eTableFlagMASK );
      table_.prepare();
   }

   // ## Copy all session rows with active sessions to target table
   auto uColumCount = m_tableSession.get_column_count();
   for( uint64_t uRow = 0; uRow < m_tableSession.size(); ++uRow )
   {
      if( m_tableSession.cell_is_null( uRow, eColumnTime ) == false )
      {
         uint64_t uNewRow = table_.row_add_one();
         for( unsigned uColumn = 0; uColumn < uColumCount; ++uColumn )
         {
            const auto* pbValue = m_tableSession.cell_get( uRow, uColumn );
            table_.cell_set( uNewRow, uColumn, pbValue );
         }
      }
   }
}



/** --------------------------------------------------------------------------
 * @brief Initializes and prepares a session table with predefined columns and metadata flags.
 * @param tableSession A reference to a table object that will be configured for session data.
 */
void CSessions::CreateTable_s( gd::table::arguments::table& tableSession )
{                                                                                                  assert( tableSession.empty() == true );
   tableSession.set_flags( gd::table::tag_meta{} );
   tableSession.column_prepare();
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
