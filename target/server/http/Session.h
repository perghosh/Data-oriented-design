// @FILE [tag: session] [description: http session] [type: header] [name: Session.h]

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

class CApplication;

/** @CLASS [name: CSessions] [description:  ]
 * \brief Sessions is a flexible data manager used to connect data for session values
 *
 * In web server session is used to connect data to clients and clients are identified by session id
 * Simple functionality but need to be very fast and thread safe
 * 
 * Table that us used to handle sessions are pre allocated and the reason for this is to avoid locking table adding users
 * 
 * In table there are five fixed columns and the names are: id, time, ip4, ip6, data.
 * - id: session id, uuid
 * - time: last active time, unix time stamp
 * - ip4: ipv4 address of client
 * - ip6: ipv6 address of client (if this is set it will be a combination of ipv4 and ipv6)
 * - data: reference to data object holding session specific data this can be anything the user want to store
 * 
 * If the time value is 0 or null it means that session is not active, the row is free to use.
 * 
 * @note Most methods are not thread safe because of performance reasons and it is designed to be used in high performance environments.
 * But for some operation there are thread safe logic and this are designed in a way that they might fail.
 * Failing can be that you check if something can be done and then do it but then it can't because another thread already did it.
 * So operations that might fail need to have logic to retry or handle failure in some way.
 *
 \code
 \endcode
 */
class CSessions
{
public:
   enum enumColumn { eColumnId, eColumnTime, eColumnIp4, eColumnIp6 };
// @API [tag: construction]
public:
   CSessions() {}
   // copy
   CSessions( const CSessions& o ) { common_construct( o ); }
   CSessions( CSessions&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CSessions& operator=( const CSessions& o ) { common_construct( o ); return *this; }
   CSessions& operator=( CSessions&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CSessions() {}
private:
   // common copy
   void common_construct( const CSessions& o ) {}
   void common_construct( CSessions&& o ) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]
   gd::argument::shared::arguments& arguments( size_t uIndex ); 

// @API [summary: general methods]
   void Initialize( size_t uMaxCount ); ///< initialize session manager, pre allocate max number of possible sessions

// @API [tag: session] [summary: operations for specific session values]

   /// Add new session, return uuid of session, it generates new uuid for session
   gd::uuid Add( uint64_t* puIndex = nullptr ); // thread safe
   /// thread safe add session value. It might fail and if so false is returned with error message
   std::pair<bool, std::string> Add( const gd::uuid& uuid_, uint64_t* puIndex ); // thread safe
   /// Find session by id, if not found return null uuid
   gd::uuid At( size_t uIndex );

   /// Update time for session, set to current time
   void Update( size_t uIndex );
   /// Clear session at index, make it free to use
   void Clear( size_t uIndex );

// @API [summary: methods that work on all sessions]

   /// purge table to remove (clear) expired sessions, if time is before expire time remove session
   void Purge( uint64_t uCurrentTime, uint64_t uExpireLimitMs ); 
   /// count number of active sessions in table
   size_t CountActive() const;
   /// count number of active sessions in table that havent expired
   size_t CountActive( uint64_t uCurrentTime, uint64_t uExpireLimitMs ) const;
   /// count number of active sessions in table that havent expired based on expire time, use current time to check
   size_t CountActive( uint64_t uExpireLimitMs ) const; 
   /// Count number of sessioins in table that have expired
   size_t CountExpired( uint64_t uCurrentTime, uint64_t uExpireLimitMs ) const;
   /// Count number of sessions in table that have expired based on expire time, use current time to check
   size_t CountExpired( uint64_t uExpireLimitMs ) const;
   /// Find first unused session index, return false if no free session found
   std::pair<bool, uint64_t> FindFirstFree( uint64_t uOffset = 0 );



protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   gd::argument::shared::arguments m_argumentProperty; ///< properties for session management

   std::mutex m_mutexTable;                    ///< mutex to protect table access
   gd::table::arguments::table m_tableSession; ///< table holding active sessions


// @API [tag: free-functions]
public:
   static void CreateTable_s( gd::table::arguments::table& tableSession );    ///< create session table structure
   static uint64_t GetTime_s();                                               ///< get current unix time in milliseconds
   /// Convert days to milliseconds for session expiration calculations
   static uint64_t DaysToMs_s( uint64_t uDays );
   /// Convert hours to milliseconds for session expiration calculations
   static uint64_t HoursToMs_s( uint64_t uHours );


};

/** --------------------------------------------------------------------------
 * @brief Get arguments object for session at index, if not created it will be created
 * @param uIndex The index of the session.
 * @return Reference to arguments object for session.
 */
inline gd::argument::shared::arguments& CSessions::arguments( size_t uIndex ) 
{                                                                                                  assert( uIndex < m_tableSession.size() );
   gd::argument::shared::arguments* parguments_ = m_tableSession.row_get_arguments_pointer( uIndex );
   return *parguments_;
}


/*
@TASK [project: users] [summary: find session for user]
[description: "Implement method to find session, this do not need to lock table. also check user 
based on index. If user holds information for 20 bytes than the index is used to check position."]

@TASK [project: users] [summary: scan table to remove users based on time]
[description: Need to free users that havent been active for a while]


*/
