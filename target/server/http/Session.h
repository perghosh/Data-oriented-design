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
 * @TODO [description: Sessions in two modes, one that is thread save and session values are]
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

// @API [tag: operation]
   void Initialize( size_t uMaxCount ); ///< initialize session manager, pre allocate max number of possible sessions

   gd::uuid Add();
   std::pair<bool, std::string> Add( const gd::uuid& uuid_ );

   gd::uuid At( size_t uIndex );

   void Update( size_t uIndex );


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
   static void CreateTable_s( gd::table::arguments::table& tableSession ); ///< create session table structure


};


/*
@TASK [project: users] [summary: find session for user]
[description: "Implement method to find session, this do not need to lock table. also check user 
based on index. If user holds information for 20 bytes than the index is used to check position."]

@TASK [project: users] [summary: scan table to remove users based on time]
[description: Need to free users that havent been active for a while]


*/