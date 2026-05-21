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
      eColumnId,            ///< expression id, this is used to identify expression and also used in query templates to refer to expression
      eColumnColumn,        ///< expression name, this is used to identify expression and also used in query templates to refer to expression
      eColumnType,          ///< expression type
      eColumnExpression,    ///< raw data for expression, this is used to store the actual sql expression template, this can also be used to store other types of expressions if needed{}
      eColumnDescription,   ///< optional description for statement, this can be used to provide additional information about statement, like what it does, or how to use it
      eColumn_Max
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
   int64_t find(const gd::argument::arguments& argumentsFind) const;

   size_t size() const { return m_ptableExpression ? m_ptableExpression->size() : 0; }
   bool empty() const { return size() == 0; }

// ## attributes ----------------------------------------------------------------
public:
   std::unique_ptr<gd::table::arguments::table> m_ptableExpression;  ///< table holding list of expressions

   // @API [tag: free-functions]
public:
   static void create_expression_s(gd::table::arguments::table& tableExpression);///< create expression structure
   static gd::table::dto::table create_expression_s(); ///< create expression dto table for expression structure


};

inline std::string_view expression::get_expression(uint64_t uRow)
{                                                                          
   // Implementation for getting the expression by row
   // This is a placeholder, actual implementation should access m_ptableExpression
   return {};
}

inline uint32_t expression::get_type(uint64_t uRow) const
{                                                                              assert(m_ptableExpression); assert(uRow < m_ptableExpression->size());
   uint32_t uType = m_ptableExpression->cell_get_variant_view(uRow, eColumnExpression).type();
   return uType;
}
                                                                               
_GD_MODULES_DBMETA_END
