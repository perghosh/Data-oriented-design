// @FILE [tag: database, meta] [description: Database description and meta information] [type: header] [name: gd__database.h]

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_types.h"
#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

#ifndef _GD_MODULES_DBMETA_BEGIN

#  define _GD_MODULES_DBMETA_BEGIN namespace gd { namespace modules { namespace dbmeta {
#  define _GD_MODULES_DBMETA_END } } }

#endif

_GD_MODULES_DBMETA_BEGIN

/** @CLASS [name: database] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class database
{
// @API [tag: construction]
public:
   database() {}
   // copy
   database( const database& o ) { common_construct( o ); }
   database( database&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   database& operator=( const database& o ) { common_construct( o ); return *this; }
   database& operator=( database&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~database() {}
private:
   // common copy
   void common_construct( const database& o ) {}
   void common_construct( database&& o ) noexcept {}

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
   uint32_t m_uDatabaseDialect{ 0 };  ///< database dialect, used to determine how the syntax of sql statements should be, and how to connect to database
   std::unique_ptr<gd::table::arguments::table> m_ptableTable;  ///< table holding list of tables
   std::unique_ptr<gd::table::arguments::table> m_ptableColumn;  ///< table holding column information


// @API [tag: free-functions]
public:
   static void create_table_s( gd::table::arguments::table& tableTable );    ///< create table structure
   static void create_column_s( gd::table::arguments::table& tableColumn );  ///< create column structure

};

_GD_MODULES_DBMETA_END