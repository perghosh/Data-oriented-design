/** @CLASS [name: CDatabase] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */

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

#ifndef NAMESPACE_META_BEGIN

#  define NAMESPACE_META_BEGIN namespace META {
#  define NAMESPACE_META_END  }

#endif

NAMESPACE_META_BEGIN

class CDatabase
{
   // @API [tag: construction]
public:
   CDatabase() {}
   // copy
   CDatabase( const CDatabase& o ) { common_construct( o ); }
   CDatabase( CDatabase&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CDatabase& operator=( const CDatabase& o ) { common_construct( o ); return *this; }
   CDatabase& operator=( CDatabase&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CDatabase() {}
private:
   // common copy
   void common_construct( const CDatabase& o ) {}
   void common_construct( CDatabase&& o ) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]


protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:


// @API [tag: free-functions]
public:
   static void CreateTable_s( gd::table::arguments::table& tableDatabase );    ///< create session table structure
   static void CreateConnections_s( gd::table::arguments::table& tableConnections );

};

NAMESPACE_META_END