// @FILE [tag: database, sql, meta, condition] [description: Store information about conditions and rules how to use these] [type: header] [name: gd__condition.h]

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_types.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

#ifndef _GD_MODULES_DBMETA_BEGIN

#  define _GD_MODULES_DBMETA_BEGIN namespace gd { namespace modules { namespace dbmeta {
#  define _GD_MODULES_DBMETA_END } } }

#endif

_GD_MODULES_DBMETA_BEGIN

/** @CLASS [name: condition] [description: conditions container for condition parts for use in query statements]
 * \brief Stores all condition-sets and their individual conditions in a single flat table.
 *
 * Each row represents one named condition belonging to a condition-set.
 * The condition-set is identified by `set_name` and the optional `table` column
 * tells which database table the set applies to.
 *
 * Columns: key, uuid, set_name, table, name, field, expression, description
 *
 * Sample xml:
   <condition-sets>
      <condition-set name="vote" table="TPoll" description="Filter for vote statements">
         <condition name="last_7_days" field="CreateD">CreateD &gt;= DATE('now', '-7 days') AND CreateD &lt; DATE('now')</condition>
         <condition name="last_week"   field="CreateD">CreateD &gt;= DATE('now', 'weekday 0', '-7 days') AND CreateD &lt; DATE('now', 'weekday 0')</condition>
         <condition name="last_month"  field="CreateD">CreateD &gt;= DATE('now', '-1 month') AND CreateD &lt; DATE('now')</condition>
      </condition-set>
   </condition-sets>
 *
 \code
   gd::modules::dbmeta::condition conditionFilter;
   conditionFilter.initialize();
   conditionFilter.add( "vote", "TPoll", "last_7_days", "CreateD",
                        "CreateD >= DATE('now', '-7 days') AND CreateD < DATE('now')",
                        "Rows created in the last 7 days" );
   auto iRow = conditionFilter.find_set( "vote" );
 \endcode
 */
class condition
{
public:
// @API [tag: constants]

   /// Column indexes for fixed columns in condition table
   enum enumColumn
   {
      eColumnKey,         ///< uint32  — row key (same value as row index for fast lookup)
      eColumnUuid,        ///< uuid    — unique identifier
      eColumnSet,         ///< rstring — name of the owning condition-set
      eColumnTable,       ///< rstring — database table the condition-set applies to
      eColumnName,        ///< rstring — name of this individual condition
      eColumnField,       ///< rstring — field / column the expression filters on
      eColumnExpression,  ///< rstring — SQL expression fragment
      eColumnDescription, ///< rstring — optional human-readable description
   };

// @API [tag: construction]
public:
   condition() {}
   condition( const condition& o ) { common_construct( o ); }
   condition( condition&& o ) noexcept { common_construct( std::move( o ) ); }
   condition& operator=( const condition& o ) { common_construct( o ); return *this; }
   condition& operator=( condition&& o ) noexcept { common_construct( std::move( o ) ); return *this; }
   ~condition() {}

private:
   void common_construct( const condition& o ) {}
   void common_construct( condition&& o ) noexcept {}

// ## methods ------------------------------------------------------------------
public:
// @API [tag: operation]

   void initialize(); ///< initialize condition object, prepares the internal table

   /// add one condition row ------------------------------------------------- add
   std::pair<bool, std::string> add( std::string_view stringSet,
                                     std::string_view stringTable,
                                     std::string_view stringName,
                                     std::string_view stringField,
                                     std::string_view stringExpression,
                                     std::string_view stringDescription = {} );

   /// add one condition row from a pre-filled arguments object -------------- add
   std::pair<bool, std::string> add( const gd::argument::arguments& argumentsCondition );

// @API [tag: get]

   gd::types::uuid     get_id( uint64_t uRow ) const;           ///< get uuid for row
   std::string_view    get_set( uint64_t uRow ) const;          ///< get condition-set name for row
   std::string_view    get_name( uint64_t uRow ) const;         ///< get condition name for row
   std::string_view    get_table( uint64_t uRow ) const;        ///< get table name for row
   std::string_view    get_field( uint64_t uRow ) const;        ///< get field name for row
   std::string_view    get_expression( uint64_t uRow ) const;   ///< get SQL expression for row

// @API [tag: search]

   int64_t find( std::string_view stringName ) const;           ///< find first row with matching condition name, returns -1 if not found
   int64_t find( const gd::types::uuid* puuid ) const;         ///< find row by uuid, returns -1 if not found
   int64_t find( const gd::types::uuid& uuid ) const { return find( &uuid ); } ///< find row by uuid reference
   int64_t find_set( std::string_view stringSetName ) const;    ///< find first row belonging to a condition-set, returns -1 if not found

// @API [tag: size]

   size_t size() const noexcept { return m_ptableCondition ? m_ptableCondition->size() : 0; } ///< number of condition rows
   bool   empty() const noexcept { return size() == 0; }                                       ///< true when no conditions are stored

   void erase( uint64_t uRow ); ///< mark row as deleted

// ## attributes ---------------------------------------------------------------
public:
   std::unique_ptr<gd::table::arguments::table> m_ptableCondition; ///< owning table holding all condition rows

// @API [tag: free-functions]
public:
   static void create_condition_s( gd::table::arguments::table& tableCondition ); ///< build the column schema for the condition table
};

_GD_MODULES_DBMETA_END