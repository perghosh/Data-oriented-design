// @FILE [tag: database, sql, expression, meta] [description: Store information about expressions and rules how to use these] [type: header] [name: gd__expression.h]

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
//#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_variant_view.h"

#ifndef _GD_MODULES_DBMETA_BEGIN

#  define _GD_MODULES_DBMETA_BEGIN namespace gd { namespace modules { namespace dbmeta {
#  define _GD_MODULES_DBMETA_END } } }

#endif

_GD_MODULES_DBMETA_BEGIN

class expression
{
   enum enumColumn
   {
      eColumnKey,           ///< column id (key), used for internal purposes
      eColumnUuid,          ///< unique identifier for expression
      eColumnTableKey,      ///< key to the parent table in `m_ptableTable` (defines which table this expression belongs to) and try som match this with index to be fast to lookup
      eColumnGroup,         ///< group identifier for expression
      eColumnId,            ///< expression id, this is used to identify expression and also used in query templates to refer to expression
      eColumnColumn,        ///< expression name, this is used to identify expression and also used in query templates to refer to expression
      eColumnType,          ///< expression type
      eColumnExpression,    ///< raw data for expression, this is used to store the actual sql expression template, this can also be used to store other types of expressions if needed{}
      eColumnDescription,   ///< optional description for statement, this can be used to provide additional information about statement, like what it does, or how to use it
      eColumn_Max
   };

   struct group
   {
      group(uint32_t uKey, int32_t iParent = -1) : m_uKey(uKey), m_iParent(iParent) {}
      void add( std::string_view stringName, gd::variant_view v_ ) { m_argumentsProperty.append_argument( stringName, v_ ); }
      void set(std::string_view stringName, gd::variant_view v_) { m_argumentsProperty.set(stringName, v_); }
      gd::variant_view get(std::string_view stringName) const { return m_argumentsProperty[stringName].as_variant_view(); }
      bool exists(std::string_view stringName) const { return m_argumentsProperty.exists(stringName); }
      bool remove( std::string_view stringName )
      {
         if( m_argumentsProperty.exists( stringName ) == false ) return false;
         m_argumentsProperty.remove( stringName );
         return true;
      }

      uint32_t get_key() const { return m_uKey; }
      int32_t get_parent() const { return m_iParent; }
      std::string_view get_name() const { return m_argumentsProperty["name"].as_string_view(); }
      void set_name(std::string_view stringName) { m_argumentsProperty.set("name", stringName); }

      uint32_t m_uKey{}; ///< key group identifier
      int32_t m_iParent{ -1 }; ///< parent group identifier, this can be used to create group hierarchy,
      gd::argument::arguments m_argumentsProperty; ///< arguments for group, this can be used to store additional information about group
   };



public:
// @API [tag: construction]

// @API [tag: operator]

// @API [tag: get, set]
   std::string_view get_expression(uint64_t uRow); ///< get expression template
   uint32_t get_type(uint64_t uRow) const; ///< get expression type

   std::pair<bool, std::string> initialize();

// @API [tag: add]
   std::pair<bool, std::string> add(const gd::argument::arguments& argumentsTable);
   std::pair<bool, std::string> add(gd::table::dto::table& tableExpression);

// @API [tag: find]
   int64_t find(gd::argument::arguments& argumentsFind) const;


   size_t size() const { return m_ptableExpression ? m_ptableExpression->size() : 0; }
   bool empty() const { return size() == 0; }


   // @API [tag: group] [summary: group specific operations]
   uint32_t next_key() { return m_uNextKey++; }
   std::pair<bool, std::string> add_group( uint32_t uKey, int32_t iParentKey = -1 );
   uint32_t add_group(std::string_view stringName, std::string_view stringParent = "");
   void set_expression_group( uint64_t uRow, uint32_t uKey );

   size_t  find_group(uint32_t uKey) const;
   size_t  find_group(std::string_view stringName, uint32_t* puKey = nullptr) const;

// ## attributes ----------------------------------------------------------------
public:
   uint32_t m_uNextKey{ 0 }; ///< next key identifier, if you do not delete groups this match index in vector
   std::vector<group> m_vectorGroup; ///< vector of groups that for grouping expressions.
   std::unique_ptr<gd::table::arguments::table> m_ptableExpression;  ///< table holding list of expressions

   // @API [tag: free-functions]
public:
   static void create_expression_s(gd::table::arguments::table& tableExpression);///< create expression structure
   static gd::table::dto::table create_expression_s(); ///< create expression dto table for expression structure


};

inline std::string_view expression::get_expression(uint64_t uRow)
{                                                                             assert(m_ptableExpression); assert(uRow < m_ptableExpression->size());
   std::string_view stringExpression = m_ptableExpression->cell_get_variant_view(uRow, eColumnExpression).as_string_view();
   return stringExpression;
}

inline uint32_t expression::get_type(uint64_t uRow) const
{                                                                              assert(m_ptableExpression); assert(uRow < m_ptableExpression->size());
   auto type_ = m_ptableExpression->cell_get_variant_view(uRow, eColumnType);
   if(type_.is_null()) return 0;
   uint32_t uType = type_.cast_as_uint32();
   return uType;
}
                                                                               
_GD_MODULES_DBMETA_END
