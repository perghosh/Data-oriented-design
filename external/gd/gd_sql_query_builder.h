// @FILE [tag: sql, query, builder] [description: logic to build SQL queries] [type: header] [name: gd_sql_query_builder.h]

/**
 * \file gd_sql_query_builder.h
 *
 * \brief Fluent builder classes for constructing SQL queries with type-safe interfaces
 *
 * This header provides three fluent builder classes (table_builder, field_builder, condition_builder) 
 * that allow constructing SQL queries using method chaining. Builders support custom memory allocation
 * through containers and provide convenient shortcuts for common SQL operations.
 *
 * \par Example
 * \code
 * query q;
 * q << table_g("users").as("u") 
 *   << table_g("orders").as("o").join("LEFT JOIN orders ON u.id = o.user_id")
 *   << field_g("u", "name").select()
 *   << field_g("o", "amount").select()
 *   << condition_g("u", "id").value(1).eq();
 * std::cout << q.sql_get(eSqlSelect) << "\n";
 * \endcode
 *
 | Area                | table_builder Methods (Examples)                                 | Description                                                                                   |
 |---------------------|--------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `table_g("name")`, `table_g("name", buffer_)`                  | Creates table builder with name and optional custom memory container.                               |
 | Attribute Setters   | `as("alias")`, `parent("table")`, `schema("public")`          | Sets table alias, parent table, database schema, owner, join conditions, and key relationships.      |
 |                    | `join("SQL")`, `key("id")`, `fk("user_id")`, `owner("admin")` |                                                                                              |
 | Conversion         | `operator arguments&()`                                        | Implicit conversion to arguments for passing to query methods.                                      |
 *
 | Area                | field_builder Methods (Examples)                                 | Description                                                                                   |
 |---------------------|--------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `field_g("name")`, `field_g("table", "name")`                 | Creates field builder with optional table qualification and custom memory container.                    |
 | Attribute Setters   | `as("alias")`, `value(123)`, `type("INTEGER")`, `raw("NOW()")` | Sets field alias, value for INSERT/UPDATE, data type, or raw SQL expression.                   |
 | SQL Part Setters    | `select()`, `orderby()`, `groupby()`, `insert()`, `update()`, `returning()` | Specifies which SQL clause the field belongs to (SELECT, ORDER BY, GROUP BY, etc.).               |
 | Conversion         | `operator arguments&()`                                        | Implicit conversion to arguments for passing to query methods.                                      |
 *
 | Area                | condition_builder Methods (Examples)                             | Description                                                                                   |
 |---------------------|--------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `condition_g("name")`, `condition_g("table", "name")`           | Creates condition builder with optional table qualification and custom memory container.               |
 | Attribute Setters   | `value(123)`, `raw("> '2024-01-01'")`, `type("INTEGER")`, `op("=")` | Sets condition value, raw SQL, data type, or custom comparison operator.                          |
 | Comparison Ops      | `eq()`, `ne()`, `lt()`, `le()`, `gt()`, `ge()`, `like()`, `in()`, `is_null()`, `is_not_null()` | Sets comparison operator (=, !=, <, <=, >, >=, LIKE, IN, IS NULL, IS NOT NULL).               |
 | Logical Grouping    | `and_()`, `or_()`, `not_()`                                  | Sets logical grouping (AND, OR, NOT) for combining multiple conditions.                           |
 | Conversion         | `operator arguments&()`                                        | Implicit conversion to arguments for passing to query methods.                                      |
 */

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_sql_types.h"
#include "gd_arguments.h"

#include "gd_sql_query.h"

#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 26812 )
#endif

#ifndef _GD_SQL_QUERY_BEGIN
#define _GD_SQL_QUERY_BEGIN namespace gd { namespace sql {
#define _GD_SQL_QUERY_END } }
#endif

_GD_SQL_QUERY_BEGIN

/** ==========================================================================
 * @brief Fluent builder for table definitions in SQL queries.
 * 
 * A proxy object that allows chaining of method calls to set table attributes 
 * such as alias, parent, schema, owner, join conditions, and key relationships.
 * 
 * @par Example
 * @code
 * query << table_g("users").as("u").join("LEFT JOIN orders ON u.id = orders.user_id");
 * query << table_g("customers").schema("public").key("id").fk("customer_id");
 * @endcode
 */
struct table_builder
{
    explicit table_builder(std::string_view stringName)  { m_arguments.append("name", stringName); }
    explicit table_builder(std::string_view stringName, std::string_view stringAlias)  { m_arguments.append("name", stringName); m_arguments.append("alias", stringAlias ); }
    
    template<typename CONTAINER>
    explicit table_builder(std::string_view stringName, CONTAINER container_): m_arguments(container_) {  
       m_arguments.append("name", stringName); }    

    // @API [tag: operator] [summary: simplify with operators for use in methods]
    
     /// Implicit conversion to arguments reference for passing to query methods
    operator gd::argument::arguments&() { return m_arguments; }
     /// Implicit conversion to arguments reference for passing to query methods
    operator const gd::argument::arguments&() const { return m_arguments; }
    
    // @API [tag: attribute] [summary: set attribute values for table]

    table_builder& as(std::string_view v_) &       { m_arguments.set("alias", v_);   return *this; }
    table_builder&& as(std::string_view v_) &&     { m_arguments.set("alias", v_);   return std::move(*this); }
    
    table_builder& parent(std::string_view v_) &   { m_arguments.set("parent", v_); return *this; }
    table_builder&& parent(std::string_view v_) && { m_arguments.set("parent", v_); return std::move(*this); }
    
    table_builder& schema(std::string_view v_) &  { m_arguments.set("schema", v_); return *this; }
    table_builder&& schema(std::string_view v_) && { m_arguments.set("schema", v_); return std::move(*this); }
    
    table_builder& owner(std::string_view v_) &    { m_arguments.set("owner", v_);   return *this; }
    table_builder&& owner(std::string_view v_) &&  { m_arguments.set("owner", v_);   return std::move(*this); }
    
    table_builder& join(std::string_view v_) &     { m_arguments.set("join", v_);    return *this; }
    table_builder&& join(std::string_view v_) &&   { m_arguments.set("join", v_);    return std::move(*this); }
    
    table_builder& key(std::string_view v_) &      { m_arguments.set("key", v_);     return *this; }
    table_builder&& key(std::string_view v_) &&    { m_arguments.set("key", v_);     return std::move(*this); }
    
    table_builder& fk(std::string_view v_) &       { m_arguments.set("fk", v_);      return *this; }
    table_builder&& fk(std::string_view v_) &&     { m_arguments.set("fk", v_);      return std::move(*this); }
    
    gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create table builder: table_g("users").as("u").join("LEFT JOIN orders ON u.id = orders.user_id");
inline table_builder table_g(std::string_view stringName) { return table_builder{stringName}; }

/// global method to create table builder: table_g("users", "user_alias").join("LEFT JOIN orders ON u.id = orders.user_id");
inline table_builder table_g(std::string_view stringName, std::string_view stringAlias) { return table_builder{stringName, stringAlias}; }


/// global method to create table builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
requires (!std::convertible_to<CONTAINER, std::string_view>)
inline table_builder table_g(std::string_view stringName, CONTAINER& buffer_)
{ return table_builder{stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof(typename CONTAINER::value_type)}}; }

/// global method to create table builder using raw C buffer
inline table_builder table_g(std::string_view stringName, void* pBuffer_, std::size_t uSize)
{ return table_builder{stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize}}; }


/// ---------------------------------------------------------------------------
/// @brief Stream operator to add a table builder to a query
/// @param query_ The query to add the table to
/// @param tablebuilder_ The table builder (moved from)
/// @return Reference to the query for chaining
/// @par Example
/// @code
/// query << table_g("users").as("u") 
///       << table_g("orders").join("LEFT JOIN users ON orders.user_id = users.id");
/// @endcode
inline query& operator<<(query& query_, table_builder&& tablebuilder_)
{
   query_.table_add(tablebuilder_, tag_arguments{});
   return query_;
}


/** ==========================================================================
 * @brief Fluent builder for field definitions in SQL queries.
 * 
 * A proxy object that allows chaining of method calls to set field attributes 
 * and specify the SQL query part it belongs to.
 * 
 * @par Example
 * @code
 * query << field_g("name").as("alias").orderby();
 * query << field_g("id").select().type("INTEGER");
 * @endcode
 */
struct field_builder
{
    explicit field_builder(std::string_view stringName)  { m_arguments.append("name", stringName); }
    explicit field_builder(std::string_view stringTable, std::string_view stringName): m_stringTable(stringTable) { m_arguments.append("name", stringName); }
    
    template<typename CONTAINER>
    explicit field_builder(std::string_view stringName, CONTAINER container_): m_arguments(container_) {  
       m_arguments.append("name", stringName); }    
    template<typename CONTAINER>
    explicit field_builder(std::string_view stringTable, std::string_view stringName, CONTAINER container_): m_stringTable(stringTable), m_arguments(container_) {  
       m_arguments.append("name", stringName); }    

    // @API [tag: operator] [summary: simplify with operators for use in methods]
    
     /// Implicit conversion to arguments reference for passing to query methods
    operator gd::argument::arguments&() { return m_arguments; }
     /// Implicit conversion to arguments reference for passing to query methods
    operator const gd::argument::arguments&() const { return m_arguments; }
    
    // @API [tag: getter, setter] [summary: get and set members]

    [[nodiscard]] unsigned get_parttype() const noexcept { return m_uPartType; }
    [[nodiscard]] std::string_view get_table() const noexcept { return m_stringTable; }
    
    // @API [tag: attribute] [summary: set attribute values for field]

    field_builder& as(std::string_view v_) &       { m_arguments.set("alias", v_);   return *this; }
    field_builder&& as(std::string_view v_) &&     { m_arguments.set("alias", v_);   return std::move(*this); }
    
    field_builder& value(gd::variant_view v_) &    { m_arguments.set("value", v_);   return *this; }
    field_builder&& value(gd::variant_view v_) &&  { m_arguments.set("value", v_);   return std::move(*this); }
    
    field_builder& type(gd::variant_view v_) &     { m_arguments.set("type", v_);    return *this; }
    field_builder&& type(gd::variant_view v_) &&   { m_arguments.set("type", v_);    return std::move(*this); }
    
    field_builder& raw(std::string_view v_) &      { m_arguments.set("raw", v_);     return *this; }
    field_builder&& raw(std::string_view v_) &&    { m_arguments.set("raw", v_);     return std::move(*this); }

    // @API [tag: sql, type] [summary: Part-type shortcuts]

    field_builder& select() &       { m_uPartType = eSqlPartSelect;    return *this; }
    field_builder&& select() &&     { m_uPartType = eSqlPartSelect;    return std::move(*this); }
    
    field_builder& orderby() &      { m_uPartType = eSqlPartOrderBy;   return *this; }
    field_builder&& orderby() &&    { m_uPartType = eSqlPartOrderBy;   return std::move(*this); }

    field_builder& groupby() &      { m_uPartType = eSqlPartGroupBy;   return *this; }
    field_builder&& groupby() &&    { m_uPartType = eSqlPartGroupBy;   return std::move(*this); }

    field_builder&  insert() &      { m_uPartType = eSqlPartInsert;    return *this; }
    field_builder&& insert() &&     { m_uPartType = eSqlPartInsert;    return std::move(*this); }

    field_builder& update() &       { m_uPartType = eSqlPartUpdate;    return *this; }
    field_builder&& update() &&     { m_uPartType = eSqlPartUpdate;    return std::move(*this); }

    field_builder& returning() &    { m_uPartType = eSqlPartReturning; return *this; }
    field_builder&& returning() &&  { m_uPartType = eSqlPartReturning; return std::move(*this); }
    
    unsigned m_uPartType = 0;            ///< part type in sql query
    std::string_view m_stringTable;      ///< optional table name for field (used for disambiguation in joins)
    gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create field builder: field_g("name").as("alias").orderby()
inline field_builder field_g(std::string_view stringName) { return field_builder{stringName}; }
inline field_builder field_g(std::string_view stringTable, std::string_view stringName) { return field_builder{stringTable, stringName}; }

/// global method to create field builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
requires (!std::convertible_to<CONTAINER, std::string_view>)
inline field_builder field_g(std::string_view stringName, CONTAINER& buffer_) { 
   return field_builder{stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof(typename CONTAINER::value_type)}}; }

/// global method to create field builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
requires (!std::convertible_to<CONTAINER, std::string_view>)
inline field_builder field_g(std::string_view stringTable, std::string_view stringName, CONTAINER& buffer_) { 
   return field_builder{stringTable, stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof(typename CONTAINER::value_type)}}; }


/// global method to create field builder using raw C buffer
inline field_builder field_g(std::string_view stringName, void* pBuffer_, std::size_t uSize) { 
   return field_builder{stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize}}; }

/// global method to create field builder using raw C buffer
inline field_builder field_g(std::string_view stringTable,std::string_view stringName, void* pBuffer_, std::size_t uSize) { 
   return field_builder{stringTable, stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize}}; }


/// ---------------------------------------------------------------------------
/// @brief Stream operator to add a field builder to a query
/// @param query_ The query to add the field to
/// @param fieldbuilder_ The field builder (moved from)
/// @return Reference to the query for chaining
/// @par Example
/// @code
/// query << field_g("id").as("user_id").select() 
///       << field_g("name").orderby();
/// @endcode
inline query& operator<<(query& query_, field_builder&& fieldbuilder_)
{
   if( fieldbuilder_.get_table().empty() == false )
   {
      const auto* ptable_ = query_.table_get( fieldbuilder_.get_table() );                         assert( ptable_ != nullptr && "Table not found in query" );

      if(fieldbuilder_.m_uPartType != 0)
      {
         query_.field_add_parttype( fieldbuilder_.m_uPartType, *ptable_, fieldbuilder_, tag_arguments{});
      }
      else { query_.field_add( *ptable_, fieldbuilder_, tag_arguments{}); }

   }
   else
   {
      if(fieldbuilder_.m_uPartType != 0)
      {
         query_.field_add_parttype(fieldbuilder_.m_uPartType, fieldbuilder_, tag_arguments{});
      }
      else { query_.field_add(fieldbuilder_, tag_arguments{}); }
   }
   
   return query_;
}

/** ==========================================================================
 * @brief Fluent builder for condition definitions in SQL queries.
 * 
 * A proxy object that allows chaining of method calls to set condition attributes 
 * such as field name, value, comparison operator, grouping, and data type.
 * 
 * @par Example
 * @code
 * query << condition_g("users", "name").value("John").eq();
 * query << condition_g("orders", "amount").value(100).greater().or_();
 * @endcode
 */
struct condition_builder
{
    explicit condition_builder(std::string_view stringName) { m_arguments.append("name", stringName); }
    explicit condition_builder(std::string_view stringTable, std::string_view stringName): m_stringTable(stringTable) { m_arguments.append("name", stringName); }
    
    template<typename CONTAINER>
    explicit condition_builder(std::string_view stringName, CONTAINER container_): m_arguments(container_) {  
       m_arguments.append("name", stringName); }    
    template<typename CONTAINER>
    explicit condition_builder(std::string_view stringTable, std::string_view stringName, CONTAINER container_): m_stringTable(stringTable), m_arguments(container_) {  
       m_arguments.append("name", stringName); }    

    // @API [tag: operator] [summary: simplify with operators for use in methods]
    
     /// Implicit conversion to arguments reference for passing to query methods
    operator gd::argument::arguments&() { return m_arguments; }
     /// Implicit conversion to arguments reference for passing to query methods
    operator const gd::argument::arguments&() const { return m_arguments; }
    
    // @API [tag: getter, setter] [summary: get and set members]

    [[nodiscard]] std::string_view get_table() const noexcept { return m_stringTable; }
    
    // @API [tag: attribute] [summary: set attribute values for condition]

    condition_builder& value(gd::variant_view v_) &    { m_arguments.set("value", v_);   return *this; }
    condition_builder&& value(gd::variant_view v_) &&  { m_arguments.set("value", v_);   return std::move(*this); }
    
    condition_builder& raw(std::string_view v_) &      { m_arguments.set("raw", v_);     return *this; }
    condition_builder&& raw(std::string_view v_) &&    { m_arguments.set("raw", v_);     return std::move(*this); }
    
    condition_builder& type(gd::variant_view v_) &     { m_arguments.set("type", v_);    return *this; }
    condition_builder&& type(gd::variant_view v_) &&   { m_arguments.set("type", v_);    return std::move(*this); }
    
    condition_builder& op(std::string_view v_) &       { m_arguments.set("operator", v_);   return *this; }
    condition_builder&& op(std::string_view v_) &&     { m_arguments.set("operator", v_);   return std::move(*this); }
    
    condition_builder& group(std::string_view v_) &    { m_arguments.set("group", v_);   return *this; }
    condition_builder&& group(std::string_view v_) &&  { m_arguments.set("group", v_);   return std::move(*this); }
    
    // @API [tag: operator, shortcut] [summary: Comparison operator shortcuts]

    condition_builder& eq() &      { m_arguments.set("operator", "=");    return *this; }
    condition_builder&& eq() &&    { m_arguments.set("operator", "=");    return std::move(*this); }
    
    condition_builder& ne() &     { m_arguments.set("operator", "!=");   return *this; }
    condition_builder&& ne() &&   { m_arguments.set("operator", "!=");   return std::move(*this); }
    
    condition_builder& lt() &     { m_arguments.set("operator", "<");    return *this; }
    condition_builder&& lt() &&   { m_arguments.set("operator", "<");    return std::move(*this); }
    
    condition_builder& le() &     { m_arguments.set("operator", "<=");   return *this; }
    condition_builder&& le() &&   { m_arguments.set("operator", "<=");   return std::move(*this); }
    
    condition_builder& gt() &     { m_arguments.set("operator", ">");    return *this; }
    condition_builder&& gt() &&   { m_arguments.set("operator", ">");    return std::move(*this); }
    
    condition_builder& ge() &     { m_arguments.set("operator", ">=");   return *this; }
    condition_builder&& ge() &&   { m_arguments.set("operator", ">=");   return std::move(*this); }
    
    condition_builder& like() &    { m_arguments.set("operator", "like"); return *this; }
    condition_builder&& like() &&  { m_arguments.set("operator", "like"); return std::move(*this); }
    
    condition_builder& in() &     { m_arguments.set("operator", "in");   return *this; }
    condition_builder&& in() &&   { m_arguments.set("operator", "in");   return std::move(*this); }
    
    condition_builder& is_null() &  { m_arguments.set("operator", "null");   return *this; }
    condition_builder&& is_null() &&{ m_arguments.set("operator", "null");   return std::move(*this); }
    
    condition_builder& is_not_null() & { m_arguments.set("operator", "notnull"); return *this; }
    condition_builder&& is_not_null() &&{ m_arguments.set("operator", "notnull"); return std::move(*this); }
    
    // @API [tag: logical, operator, shortcut] [summary: Logical grouping shortcuts]

    condition_builder& and_() &   { m_arguments.set("logical", "and");  return *this; }
    condition_builder&& and_() && { m_arguments.set("logical", "and");  return std::move(*this); }
    
    condition_builder& or_() &    { m_arguments.set("logical", "or");   return *this; }
    condition_builder&& or_() &&  { m_arguments.set("logical", "or");   return std::move(*this); }
    
    condition_builder& not_() &   { m_arguments.set("logical", "not");  return *this; }
    condition_builder&& not_() && { m_arguments.set("logical", "not");  return std::move(*this); }
    
    condition_builder& and_( std::string_view stringGroup ) &   { m_arguments.set("logical", "and"); m_arguments.set("group", stringGroup );  return *this; }
    condition_builder&& and_( std::string_view stringGroup ) && { m_arguments.set("logical", "and"); m_arguments.set("group", stringGroup );  return std::move(*this); }
    
    condition_builder& or_( std::string_view stringGroup ) &    { m_arguments.set("logical", "or"); m_arguments.set("group", stringGroup );   return *this; }
    condition_builder&& or_( std::string_view stringGroup ) &&  { m_arguments.set("logical", "or"); m_arguments.set("group", stringGroup );   return std::move(*this); }
    
    condition_builder& not_( std::string_view stringGroup ) &   { m_arguments.set("logical", "not"); m_arguments.set("group", stringGroup );  return *this; }
    condition_builder&& not_( std::string_view stringGroup ) && { m_arguments.set("logical", "not"); m_arguments.set("group", stringGroup );  return std::move(*this); }
    
    
    std::string_view m_stringTable;      ///< optional table name for condition (used for disambiguation in joins)
    gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create condition builder: condition_g("name").value(100).gt();
inline condition_builder condition_g(std::string_view stringName) { return condition_builder{stringName}; }
inline condition_builder condition_g(std::string_view stringTable, std::string_view stringName) { return condition_builder{stringTable, stringName}; }

/// global method to create condition builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
requires (!std::convertible_to<CONTAINER, std::string_view>)
inline condition_builder condition_g(std::string_view stringName, CONTAINER& buffer_) { 
   return condition_builder{stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof(typename CONTAINER::value_type)}}; }

/// global method to create condition builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
requires (!std::convertible_to<CONTAINER, std::string_view>)
inline condition_builder condition_g(std::string_view stringTable, std::string_view stringName, CONTAINER& buffer_) { 
   return condition_builder{stringTable, stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof(typename CONTAINER::value_type)}}; }

/// global method to create condition builder using raw C buffer
inline condition_builder condition_g(std::string_view stringName, void* pBuffer_, std::size_t uSize) { 
   return condition_builder{stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize}}; }

/// global method to create condition builder using raw C buffer
inline condition_builder condition_g(std::string_view stringTable, std::string_view stringName, void* pBuffer_, std::size_t uSize) { 
   return condition_builder{stringTable, stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize}}; }


/// ---------------------------------------------------------------------------
/// @brief Stream operator to add a condition builder to a query
/// @param query_ The query to add the condition to
/// @param conditionbuilder_ The condition builder (moved from)
/// @return Reference to the query for chaining
/// @par Example
/// @code
/// query << condition_g("users", "name").value("John").eq() 
///       << condition_g("orders", "amount").value(100).gt();
/// @endcode
inline query& operator<<(query& query_, condition_builder&& conditionbuilder_)
{
   if( conditionbuilder_.get_table().empty() == false )
   {
      const auto* ptable_ = query_.table_get( conditionbuilder_.get_table() );                         assert( ptable_ != nullptr && "Table not found in query" );
      query_.condition_add( *ptable_, conditionbuilder_, tag_arguments{});
   }
   else
   {
      query_.condition_add( conditionbuilder_, tag_arguments{});
   }
   
   return query_;
}


_GD_SQL_QUERY_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
