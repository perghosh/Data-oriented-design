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
/*
   enum enumColumnFlag 
   {
      eColumnFlagKey = 1,
      eColumnFlagFKey = 2,
      eColumnFlagNotNull = 4,
      eColumnFlagComputed = 8,
   };
   */

   enum enumColumnTable
   {
      eColumnTableKey = 0,
      eColumnTableFirstKey = 1,
      eColumnTableSchema = 2,
      eColumnTableTable = 3,
      eColumnTableAlias = 4,
      eColumnTableDescription = 5,
   };

   enum enumColumnField
   {
      eColumnFieldKey,           ///< column id (key), used for internal purposes
      eColumnFieldMeta,          ///< column meta information, used for internal purposes
      eColumnFieldSchema,        ///< schema for table field belongs to
      eColumnFieldTable,         ///< name for table field belongs to
      eColumnFieldName,          ///< name for column in table
      eColumnFieldAlias,         ///< alias for column in table
      eColumnFieldValue,         ///< value for column in table
      eColumnFieldType,          ///< gd type value for column value
      eColumnFieldPartType,      ///< sql part type for column, used to separate columns for select, value and where parts of query
      ///< This is used to be able to filter out columns for different parts of query, for example when creating insert query we only need value part, when creating select query we only need select part, etc.
      eColumnFieldRaw,           ///< raw sql statement for column, used when column value is not enough to specify what we want to do with column, for example when we want to specify that column should be used in group by or order by part of query, etc.
      eColumnField_Max
   };

   enum enumColumnFlag
   {
      eColumnFlagKey = 0x01,     ///< column is a key column, used for where part of query
      eColumnFlagSchema = 0x02,  ///< column has schema specified
      eColumnFlagTable = 0x04,   ///< column has table specified
      eColumnFlagName = 0x08,    ///< column has name specified
      eColumnFlagAlias = 0x10,   ///< column has alias specified
      eColumnFlagValue = 0x20,   ///< column has value specified
      eColumnFlagType = 0x40,    ///< column has type specified
      eColumnFlagPartType = 0x80,///< column has part type specified
      eColumnFlagRaw = 0x100,    ///< column has raw sql statement specified
   };

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

   uint32_t table_key(uint64_t uRow) const noexcept { return m_ptableTable->cell_get_variant_view(uRow, eColumnTableKey); }

// @API [tag: operation]
   std::pair<bool, std::string> initialize();

// @API [tag: add]
   std::pair<bool, std::string> add(const gd::argument::arguments& argumentsTable, gd::types::tag_table);
   std::pair<bool, std::string> add( gd::table::dto::table& tableTable, gd::types::tag_table );
   std::pair<bool, std::string> add( gd::table::dto::table& tableColumn, gd::types::tag_column );

// @API [tag: find]
   int64_t find(const std::string_view& stringSchema, const std::string_view& stringTable, gd::types::tag_table) const noexcept;

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
