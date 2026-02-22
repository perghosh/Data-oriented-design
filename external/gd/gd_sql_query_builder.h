// @FILE [tag: sql, query, builder] [description: logic to build SQL queries] [type: header] [name: gd_sql_query_builder.h]

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
    
    template<typename CONTAINER>
    explicit field_builder(std::string_view stringName, CONTAINER container_): m_arguments(container_) {  
       m_arguments.append("name", stringName); }    

    // @API [tag: operator] [summary: simplify with operators for use in methods]
    
     /// Implicit conversion to arguments reference for passing to query methods
    operator gd::argument::arguments&() { return m_arguments; }
     /// Implicit conversion to arguments reference for passing to query methods
    operator const gd::argument::arguments&() const { return m_arguments; }
    
    // @API [tag: getter, setter] [summary: get and set members]

    unsigned get_parttype() const noexcept { return m_uPartType; }
    
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
    gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create field builder: field_g("name").as("alias").orderby()
inline field_builder field_g(std::string_view stringName) { return field_builder{stringName}; }

/// global method to create field builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
inline field_builder field_g(std::string_view stringName, CONTAINER& buffer_)
{ return field_builder{stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof(typename CONTAINER::value_type)}}; }

/// global method to create field builder using raw C buffer
inline field_builder field_g(std::string_view stringName, void* pBuffer_, std::size_t uSize)
{ return field_builder{stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize}}; }


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
   if(fieldbuilder_.m_uPartType != 0)
   {
      query_.field_add_parttype(fieldbuilder_.m_uPartType, fieldbuilder_, tag_arguments{});
   }
   else { query_.field_add(fieldbuilder_, tag_arguments{}); }
   
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
