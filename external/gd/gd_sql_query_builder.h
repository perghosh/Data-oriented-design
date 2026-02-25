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
 *
 * // Compact form — variadic field names
 * query q2;
 * q2 << table_g("users")
 *    << fields_g("users", "name", "age", "email", "created_at", "updated_at").select();
 * std::cout << q2.sql_get(eSqlSelect) << "\n";
 *
 * // Compact form — name+alias pairs
 * query q3;
 * q3 << table_g("users")
 *    << fields_g("users", {{"name","alias_name"}, {"age","alias_age"}}).select();
 * std::cout << q3.sql_get(eSqlSelect) << "\n";
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

#define BUILDER_SETTER(class_, key_, param_) \
    class_& param_(std::string_view v_) &    { m_arguments.set(key_, v_); return *this; } \
    class_&& param_(std::string_view v_) &&  { m_arguments.set(key_, v_); return std::move(*this); }

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
   explicit table_builder( std::string_view stringName ) { m_arguments.append( "name", stringName ); }
   explicit table_builder( std::string_view stringName, std::string_view stringAlias ) { m_arguments.append( "name", stringName ); m_arguments.append( "alias", stringAlias ); }

   template<typename CONTAINER>
   explicit table_builder( std::string_view stringName, CONTAINER container_ ) : m_arguments( container_ ) {
      m_arguments.append( "name", stringName );
   }

   // @API [tag: operator] [summary: simplify with operators for use in methods]

    /// Implicit conversion to arguments reference for passing to query methods
   operator gd::argument::arguments& ( ) { return m_arguments; }
   /// Implicit conversion to arguments reference for passing to query methods
   operator const gd::argument::arguments& ( ) const { return m_arguments; }

   // @API [tag: attribute] [summary: set attribute values for table]

   BUILDER_SETTER( table_builder, "alias", as );
   BUILDER_SETTER( table_builder, "parent", parent );
   BUILDER_SETTER( table_builder, "schema", schema );
   BUILDER_SETTER( table_builder, "owner", owner );
   BUILDER_SETTER( table_builder, "join", join );
   BUILDER_SETTER( table_builder, "key", key );
   BUILDER_SETTER( table_builder, "fk", fk );


   gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create table builder: table_g("users").as("u").join("LEFT JOIN orders ON u.id = orders.user_id");
inline table_builder table_g( std::string_view stringName ) { return table_builder{ stringName }; }

/// global method to create table builder: table_g("users", "user_alias").join("LEFT JOIN orders ON u.id = orders.user_id");
inline table_builder table_g( std::string_view stringName, std::string_view stringAlias ) { return table_builder{ stringName, stringAlias }; }


/// global method to create table builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
   requires ( !std::convertible_to<CONTAINER, std::string_view> )
inline table_builder table_g( std::string_view stringName, CONTAINER& buffer_ )
{
   return table_builder{ stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof( typename CONTAINER::value_type )} };
}

/// global method to create table builder using raw C buffer
inline table_builder table_g( std::string_view stringName, void* pBuffer_, std::size_t uSize )
{
   return table_builder{ stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize} };
}


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
inline query& operator<<( query& query_, table_builder&& tablebuilder_ )
{
   query_.table_add( tablebuilder_, tag_arguments{} );
   return query_;
}


/** ==========================================================================
 * @brief Fluent builder for multiple field definitions in SQL queries.
 *
 * A proxy object that accumulates field entries and applies a single part-type
 * (SELECT, INSERT, UPDATE, etc.) to all of them at once. Fields can carry up
 * to three attributes: name, alias, and value.
 *
 * @par Example
 * @code
 * query << fields_g("u").add("id", "uid").add("name", "full_name").select();
 * query << fields_g().add("status", gd::variant_view("active")).add("score", gd::variant_view(42)).insert();
 * @endcode
 *
 | Area               | fields_builder Methods (Examples)                                          | Description                                                        |
 |--------------------|------------------------------------------------------------------------|--------------------------------------------------------------------|
 | Construction       | `fields_g()`, `fields_g("table")`                                      | Creates empty builder with optional table qualification.           |
 |                    | `fields_g("table", "f1", "f2", ...)`                                   | Creates builder pre-loaded with name-only fields (variadic).       |
 |                    | `fields_g("table", {{"f1","a1"}, {"f2","a2"}})`                        | Creates builder pre-loaded with name+alias pairs (initializer list). |
 | Field Add          | `add("name")`, `add("name","alias")`, `add("name", value)`             | Adds a field with name-only, name+alias, or name+value.            |
 |                    | `add("name", "alias", value)`                                          | Adds a field with all three attributes.                            |
 | Type Setter        | `type(variant_view)`                                                   | Sets a default SQL type applied to all fields (before or after add). |
 | SQL Part Setters   | `select()`, `insert()`, `update()`, `orderby()`, `groupby()`, `returning()` | Sets the SQL clause for all accumulated fields.               |
 | Conversion         | `operator<<` on query                                                  | Streams all fields into the query in one step.                     |
 */
struct fields_builder
{
   fields_builder() = default;
   explicit fields_builder( std::string_view stringTable ) : m_stringTable( stringTable ) {}

   // @API [tag: operator] [summary: implicit conversion to the field arguments vector]

   operator const std::vector<gd::argument::arguments>& ( ) const { return m_vectorFieldArguments; }

   // @API [tag: getter] [summary: get part type and table name]

   [[nodiscard]] unsigned          get_parttype() const noexcept { return m_uPartType; }
   [[nodiscard]] std::string_view  get_table()    const noexcept { return m_stringTable; }

   // @API [tag: field, add] [summary: add fields with flexible attribute combinations]

   /// Add a field by name only
   fields_builder&  add( std::string_view stringName )&  { add_( stringName, {}, {} ); return *this; }
   fields_builder&& add( std::string_view stringName )&& { add_( stringName, {}, {} ); return std::move( *this ); }

   /// Add a field with name + alias  (typical SELECT alias case)
   fields_builder&  add( std::string_view stringName, std::string_view stringAlias )&  { add_( stringName, stringAlias, {} ); return *this; }
   fields_builder&& add( std::string_view stringName, std::string_view stringAlias )&& { add_( stringName, stringAlias, {} ); return std::move( *this ); }

   /// Add a field with name + value  (typical INSERT/UPDATE case)
   fields_builder&  add( std::string_view stringName, gd::variant_view variantviewValue )&  { add_( stringName, {}, variantviewValue ); return *this; }
   fields_builder&& add( std::string_view stringName, gd::variant_view variantviewValue )&& { add_( stringName, {}, variantviewValue ); return std::move( *this ); }

   /// Add a field with name + alias + value  (all three attributes)
   fields_builder&  add( std::string_view stringName, std::string_view stringAlias, gd::variant_view variantviewValue )&  { add_( stringName, stringAlias, variantviewValue ); return *this; }
   fields_builder&& add( std::string_view stringName, std::string_view stringAlias, gd::variant_view variantviewValue )&& { add_( stringName, stringAlias, variantviewValue ); return std::move( *this ); }

   // @API [tag: sql, type] [summary: Part-type shortcuts — applied to all fields on stream]

   fields_builder&  select()&    { m_uPartType = eSqlPartSelect;    return *this; }
   fields_builder&& select()&&   { m_uPartType = eSqlPartSelect;    return std::move( *this ); }

   fields_builder&  orderby()&   { m_uPartType = eSqlPartOrderBy;   return *this; }
   fields_builder&& orderby()&&  { m_uPartType = eSqlPartOrderBy;   return std::move( *this ); }

   fields_builder&  groupby()&   { m_uPartType = eSqlPartGroupBy;   return *this; }
   fields_builder&& groupby()&&  { m_uPartType = eSqlPartGroupBy;   return std::move( *this ); }

   fields_builder&  insert()&    { m_uPartType = eSqlPartInsert;    return *this; }
   fields_builder&& insert()&&   { m_uPartType = eSqlPartInsert;    return std::move( *this ); }

   fields_builder&  update()&    { m_uPartType = eSqlPartUpdate;    return *this; }
   fields_builder&& update()&&   { m_uPartType = eSqlPartUpdate;    return std::move( *this ); }

   fields_builder&  returning()& { m_uPartType = eSqlPartReturning; return *this; }
   fields_builder&& returning()&&{ m_uPartType = eSqlPartReturning; return std::move( *this ); }

   // @API [tag: attribute] [summary: set a default SQL type stamped onto all fields]

   /// Set type on already-added fields, and remember it for fields added afterward.
   /// Calling type() before or after add() both work correctly.
   fields_builder&  type( gd::variant_view variantviewType )&  { set_type_( variantviewType ); return *this; }
   fields_builder&& type( gd::variant_view variantviewType )&& { set_type_( variantviewType ); return std::move( *this ); }

   unsigned                              m_uPartType = 0;            ///< part type applied to all fields on stream
   std::string_view                      m_stringTable;              ///< optional table qualifier (used for disambiguation in joins)
   gd::variant_view                      m_variantviewType;          ///< default type stamped onto each field on add
   std::vector<gd::argument::arguments>  m_vectorFieldArguments;     ///< accumulated field argument sets

private:
   /// Internal helper — builds one field arguments entry and appends it to the vector.
   void add_( std::string_view stringName, std::string_view stringAlias, gd::variant_view variantviewValue )
   {
      gd::argument::arguments argumentsField;
      argumentsField.append( "name", stringName );
      if( stringAlias.empty()             == false ) { argumentsField.append( "alias", stringAlias ); }
      if( variantviewValue.is_null()      == false ) { argumentsField.append_argument( "value", variantviewValue ); }
      if( m_variantviewType.is_null()     == false ) { argumentsField.append_argument( "type",  m_variantviewType ); }
      m_vectorFieldArguments.push_back( std::move( argumentsField ) );
   }

   /// Stamps the type onto all existing entries and caches it for future add() calls.
   void set_type_( gd::variant_view variantviewType )
   {
      m_variantviewType = variantviewType;
      for( auto& argumentsField : m_vectorFieldArguments ) { argumentsField.set( "type", variantviewType ); }
   }
};

/// global method to create fields builder: fields_g().add("name").add("name","alias").add("name", value).select()
inline fields_builder fields_g()                               { return {}; }
inline fields_builder fields_g( std::string_view stringTable ) { return fields_builder{ stringTable }; }

/// global method — table + any number of field names:
/// fields_g("users", "name", "age", "email").select()
template<typename... NAMES>
   requires ( (std::convertible_to<NAMES, std::string_view>) && ... )
inline fields_builder fields_g( std::string_view stringTable, NAMES&&... names_ )
{
   fields_builder fieldsbuilder_{ stringTable };
   ( fieldsbuilder_.add( std::string_view( names_ ) ), ... );
   return fieldsbuilder_;
}

/// global method — table + initializer list of {name, alias} pairs:
/// fields_g("users", {{"name","alias_name"}, {"age","alias_age"}}).select()
inline fields_builder fields_g( std::string_view stringTable, std::initializer_list<std::pair<std::string_view, std::string_view>> listFields_ )
{
   fields_builder fieldsbuilder_{ stringTable };
   for( const auto& [stringName, stringAlias] : listFields_ ) { fieldsbuilder_.add( stringName, stringAlias ); }
   return fieldsbuilder_;
}

/// global method — table + any STL-compatible container of {name, alias} pairs:
/// fields_g("users", my_vector_of_pairs).select()
template <typename Container>
   requires std::ranges::input_range<Container> && 
            std::same_as<std::pair<std::string_view, std::string_view>, std::remove_cvref_t<decltype(*std::declval<typename Container::iterator>())>>
inline fields_builder fields_g( std::string_view stringTable, const Container& containerFields_ )
{
   fields_builder fieldsbuilder{ stringTable };
   for( const auto& [stringName, stringAlias] : containerFields_ ) { fieldsbuilder.add( stringName, stringAlias ); }
   return fieldsbuilder;
}

/// global method — table + any STL-compatible container of {name, value} pairs:
/// fields_g("users", my_vector_of_pairs).select()
template <typename Container>
   requires std::ranges::input_range<Container> && 
            std::same_as<std::pair<std::string_view, gd::variant_view>, std::remove_cvref_t<decltype(*std::declval<typename Container::iterator>())>>
inline fields_builder fields_g( std::string_view stringTable, const Container& containerFields_ )
{
   fields_builder fieldsbuilder{ stringTable };
   for( const auto& [stringName, variantviewValue] : containerFields_ ) { fieldsbuilder.add( stringName, variantviewValue ); }
   return fieldsbuilder;
}


/// ---------------------------------------------------------------------------
/// @brief Stream operator to add all fields in a fields_builder to a query
/// @param query_ The query to add the fields to
/// @param fieldsbuilder_ The fields builder (moved from)
/// @return Reference to the query for chaining
/// @par Example
/// @code
/// query << fields_g("u").add("id", "uid").add("name", "full_name").select();
/// @endcode
inline query& operator<<( query& query_, fields_builder&& fieldsbuilder_ )
{
   const unsigned uPartType = fieldsbuilder_.get_parttype();

   /// Dispatches a single field arguments set into the query, respecting table and part-type state.
   auto add_one_ = [&]( const gd::argument::arguments& argumentsField, const query::table* ptable_ )
   {
      if( ptable_ != nullptr )
      {
         if( uPartType != 0 ) { query_.field_add_parttype( uPartType, ptable_->get_key(), argumentsField, tag_arguments{} ); }
         else                 { query_.field_add( ptable_->get_key(), argumentsField, tag_arguments{} ); }
      }
      else
      {
         if( uPartType != 0 ) { query_.field_add_parttype( uPartType, argumentsField, tag_arguments{} ); }
         else                 { query_.field_add( argumentsField, tag_arguments{} ); }
      }
   };

   if( fieldsbuilder_.get_table().empty() == false )
   {
      const auto* ptable_ = query_.table_get( fieldsbuilder_.get_table() );            assert( ptable_ != nullptr && "Table not found in query" );
      for( const auto& it : fieldsbuilder_.m_vectorFieldArguments ) { add_one_( it, ptable_ ); }
   }
   else
   {
      for( const auto& it : fieldsbuilder_.m_vectorFieldArguments ) { add_one_( it, nullptr ); }
   }

   return query_;
}

/** ==========================================================================
 * @brief Fluent builder for querystring-formatted field definitions.
 *
 * Wraps one or more querystring specs (e.g. "name=qname") and streams them
 * into a query via field_add(..., tag_querystring). Follows the same
 * builder/operator<< convention as field_builder and fields_builder.
 *
 * @par Example
 * @code
 * query q;
 * q << table_g("users")
 *   << fields_g("users", "name", "age", "email").select()
 *   << fieldqs_g("users", "name=qname")                       // single, table-qualified
 *   << fieldqs_g("users", "name=qname", "age=qage");          // multiple, variadic
 *   << fieldqs_g("email=qemail");                             // implicit first table
 * @endcode
 *
 | Area               | fieldsqs_builder Methods                                            | Description                                                   |
 |--------------------|---------------------------------------------------------------------|---------------------------------------------------------------|
 | Construction       | `fieldqs_g("qs")`, `fieldqs_g("table", "qs")`                       | Single querystring, with or without table qualifier.          |
 |                    | `fieldqs_g("table", "qs1", "qs2", ...)`                             | Variadic: multiple querystrings for the same table.           |
 */
struct fieldsqs_builder
{
   explicit fieldsqs_builder( std::string_view stringQueryString )
      : m_stringTable{} { m_vectorQueryString.emplace_back( stringQueryString ); }

   explicit fieldsqs_builder( std::string_view stringTable, std::string_view stringQueryString )
      : m_stringTable( stringTable ) { m_vectorQueryString.emplace_back( stringQueryString ); }

   [[nodiscard]] std::string_view get_table()    const noexcept { return m_stringTable; }
   [[nodiscard]] unsigned         get_parttype() const noexcept { return m_uPartType; }

   // @API [tag: sql, type] [summary: Part-type shortcuts — applied to all fields on stream]

   fieldsqs_builder&  select()&    { m_uPartType = eSqlPartSelect;    return *this; }
   fieldsqs_builder&& select()&&   { m_uPartType = eSqlPartSelect;    return std::move( *this ); }

   fieldsqs_builder&  orderby()&   { m_uPartType = eSqlPartOrderBy;   return *this; }
   fieldsqs_builder&& orderby()&&  { m_uPartType = eSqlPartOrderBy;   return std::move( *this ); }

   fieldsqs_builder&  groupby()&   { m_uPartType = eSqlPartGroupBy;   return *this; }
   fieldsqs_builder&& groupby()&&  { m_uPartType = eSqlPartGroupBy;   return std::move( *this ); }

   fieldsqs_builder&  insert()&    { m_uPartType = eSqlPartInsert;    return *this; }
   fieldsqs_builder&& insert()&&   { m_uPartType = eSqlPartInsert;    return std::move( *this ); }

   fieldsqs_builder&  update()&    { m_uPartType = eSqlPartUpdate;    return *this; }
   fieldsqs_builder&& update()&&   { m_uPartType = eSqlPartUpdate;    return std::move( *this ); }

   fieldsqs_builder&  returning()& { m_uPartType = eSqlPartReturning; return *this; }
   fieldsqs_builder&& returning()&&{ m_uPartType = eSqlPartReturning; return std::move( *this ); }

   unsigned                        m_uPartType = 0;
   std::string_view                m_stringTable;
   std::vector< std::string_view > m_vectorQueryString;
};


/// Stream querystring fields into `query_` ------------------------------------ operator<<
///
/// Adds each querystring from `fieldsqsbuilder_` via `field_add(...)`, asserts
/// that parsing succeeds, and applies the part-type to each produced field when
/// `uPartType` is non-zero.
inline query& operator<<( query& query_, fieldsqs_builder&& fieldsqsbuilder_ )
{
   const unsigned uPartType = fieldsqsbuilder_.get_parttype();

   for( const auto& stringQueryString : fieldsqsbuilder_.m_vectorQueryString )
   {
      auto [pfield_, stringError_] = fieldsqsbuilder_.get_table().empty()
         ? query_.field_add( stringQueryString,                               tag_querystring{} )
         : query_.field_add( fieldsqsbuilder_.get_table(), stringQueryString, tag_querystring{} );

      assert( stringError_.empty() && "field_add querystring parse error" );

      if( pfield_ != nullptr && uPartType != 0 ) pfield_->set_useandtype( uPartType );
   }
   return query_;
}

/// Single querystring, implicit first table: fieldqs_g("name=qname")
inline fieldsqs_builder fieldqs_g( std::string_view stringQueryString )
{
   return fieldsqs_builder{ stringQueryString };
}

/// Single querystring, table-qualified: fieldqs_g("users", "name=qname")
inline fieldsqs_builder fieldqs_g( std::string_view stringTable, std::string_view stringQueryString )
{
   return fieldsqs_builder{ stringTable, stringQueryString };
}

/// Variadic: multiple querystrings for one table: fieldqs_g("users", "name=qname", "age=qage")
template<typename... QS>
   requires ( (std::convertible_to<QS, std::string_view>) && ... )
inline fieldsqs_builder fieldqs_g( std::string_view stringTable, std::string_view stringFirst, QS&&... rest_ )
{
   fieldsqs_builder builder_{ stringTable, stringFirst };
   ( builder_.m_vectorQueryString.emplace_back( std::string_view( rest_ ) ), ... );
   return builder_;
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
 *
 | Area               | field_builder Methods (Examples)                                           | Description                                                        |
 |--------------------|------------------------------------------------------------------------|--------------------------------------------------------------------|
 | Construction       | `field_g("name")`, `field_g("table", "name")`                          | Creates field builder with optional table qualification.           |
 | Attribute Setters  | `as("alias")`, `value(123)`, `type("INTEGER")`, `raw("NOW()")`         | Sets field alias, value for INSERT/UPDATE, data type, or raw SQL.  |
 | SQL Part Setters   | `select()`, `orderby()`, `groupby()`, `insert()`, `update()`, `returning()` | Specifies which SQL clause the field belongs to.              |
 | Conversion         | `operator arguments&()`                                                | Implicit conversion to arguments for passing to query methods.     |
 */
struct field_builder
{
   explicit field_builder( std::string_view stringName ) { m_arguments.append( "name", stringName ); }
   explicit field_builder( std::string_view stringTable, std::string_view stringName ) : m_stringTable( stringTable ) { m_arguments.append( "name", stringName ); }

   template<typename CONTAINER>
   explicit field_builder( std::string_view stringName, CONTAINER container_ ) : m_arguments( container_ ) {
      m_arguments.append( "name", stringName );
   }
   template<typename CONTAINER>
   explicit field_builder( std::string_view stringTable, std::string_view stringName, CONTAINER container_ ) : m_stringTable( stringTable ), m_arguments( container_ ) {
      m_arguments.append( "name", stringName );
   }

   // @API [tag: operator] [summary: simplify with operators for use in methods]

   /// Implicit conversion to arguments reference for passing to query methods
   operator gd::argument::arguments& ( ) { return m_arguments; }
   /// Implicit conversion to arguments reference for passing to query methods
   operator const gd::argument::arguments& ( ) const { return m_arguments; }

   // @API [tag: getter, setter] [summary: get and set members]

   [[nodiscard]] unsigned get_parttype() const noexcept { return m_uPartType; }
   [[nodiscard]] std::string_view get_table() const noexcept { return m_stringTable; }

   // @API [tag: attribute] [summary: set attribute values for field]

   field_builder& as( std::string_view v_ )& { m_arguments.set( "alias", v_ );   return *this; }
   field_builder&& as( std::string_view v_ )&& { m_arguments.set( "alias", v_ );   return std::move( *this ); }

   field_builder& value( gd::variant_view v_ )& { m_arguments.set( "value", v_ );   return *this; }
   field_builder&& value( gd::variant_view v_ )&& { m_arguments.set( "value", v_ );   return std::move( *this ); }

   field_builder& type( gd::variant_view v_ )& { m_arguments.set( "type", v_ );    return *this; }
   field_builder&& type( gd::variant_view v_ )&& { m_arguments.set( "type", v_ );    return std::move( *this ); }

   field_builder& raw( std::string_view v_ )& { m_arguments.set( "raw", v_ );     return *this; }
   field_builder&& raw( std::string_view v_ )&& { m_arguments.set( "raw", v_ );     return std::move( *this ); }

   // @API [tag: sql, type] [summary: Part-type shortcuts]

   field_builder& select()& { m_uPartType = eSqlPartSelect;    return *this; }
   field_builder&& select()&& { m_uPartType = eSqlPartSelect;    return std::move( *this ); }

   field_builder& orderby()& { m_uPartType = eSqlPartOrderBy;   return *this; }
   field_builder&& orderby()&& { m_uPartType = eSqlPartOrderBy;   return std::move( *this ); }

   field_builder& groupby()& { m_uPartType = eSqlPartGroupBy;   return *this; }
   field_builder&& groupby()&& { m_uPartType = eSqlPartGroupBy;   return std::move( *this ); }

   field_builder& insert()& { m_uPartType = eSqlPartInsert;    return *this; }
   field_builder&& insert()&& { m_uPartType = eSqlPartInsert;    return std::move( *this ); }

   field_builder& update()& { m_uPartType = eSqlPartUpdate;    return *this; }
   field_builder&& update()&& { m_uPartType = eSqlPartUpdate;    return std::move( *this ); }

   field_builder& returning()& { m_uPartType = eSqlPartReturning; return *this; }
   field_builder&& returning()&& { m_uPartType = eSqlPartReturning; return std::move( *this ); }

   unsigned m_uPartType = 0;            ///< part type in sql query
   std::string_view m_stringTable;      ///< optional table name for field (used for disambiguation in joins)
   gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create field builder: field_g("name").as("alias").orderby()
inline field_builder field_g( std::string_view stringName ) { return field_builder{ stringName }; }
inline field_builder field_g( std::string_view stringTable, std::string_view stringName ) { return field_builder{ stringTable, stringName }; }

/// global method to create field builder using any container
template<typename CONTAINER>
   requires ( !std::convertible_to<CONTAINER, std::string_view> )
inline field_builder field_g( std::string_view stringName, CONTAINER& buffer_ ) {
   return field_builder{ stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof( typename CONTAINER::value_type )} };
}

/// global method to create field builder using any container
template<typename CONTAINER>
   requires ( !std::convertible_to<CONTAINER, std::string_view> )
inline field_builder field_g( std::string_view stringTable, std::string_view stringName, CONTAINER& buffer_ ) {
   return field_builder{ stringTable, stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof( typename CONTAINER::value_type )} };
}

/// global method to create field builder using raw C buffer
inline field_builder field_g( std::string_view stringName, void* pBuffer_, std::size_t uSize ) {
   return field_builder{ stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize} };
}

/// global method to create field builder using raw C buffer
inline field_builder field_g( std::string_view stringTable, std::string_view stringName, void* pBuffer_, std::size_t uSize ) {
   return field_builder{ stringTable, stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize} };
}


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
inline query& operator<<( query& query_, field_builder&& fieldbuilder_ )
{
   if( fieldbuilder_.get_table().empty() == false )
   {
      const auto* ptable_ = query_.table_get( fieldbuilder_.get_table() );                         assert( ptable_ != nullptr && "Table not found in query" );

      if( fieldbuilder_.m_uPartType != 0 )
      {
         query_.field_add_parttype( fieldbuilder_.m_uPartType, *ptable_, fieldbuilder_, tag_arguments{} );
      }
      else { query_.field_add( *ptable_, fieldbuilder_, tag_arguments{} ); }

   }
   else
   {
      if( fieldbuilder_.m_uPartType != 0 )
      {
         query_.field_add_parttype( fieldbuilder_.m_uPartType, fieldbuilder_, tag_arguments{} );
      }
      else { query_.field_add( fieldbuilder_, tag_arguments{} ); }
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
   explicit condition_builder( std::string_view stringName ) { m_arguments.append( "name", stringName ); }
   explicit condition_builder( std::string_view stringTable, std::string_view stringName ) : m_stringTable( stringTable ) { m_arguments.append( "name", stringName ); }

   template<typename CONTAINER>
   explicit condition_builder( std::string_view stringName, CONTAINER container_ ) : m_arguments( container_ ) {
      m_arguments.append( "name", stringName );
   }
   template<typename CONTAINER>
   explicit condition_builder( std::string_view stringTable, std::string_view stringName, CONTAINER container_ ) : m_stringTable( stringTable ), m_arguments( container_ ) {
      m_arguments.append( "name", stringName );
   }

   // @API [tag: operator] [summary: simplify with operators for use in methods]

    /// Implicit conversion to arguments reference for passing to query methods
   operator gd::argument::arguments& ( ) { return m_arguments; }
   /// Implicit conversion to arguments reference for passing to query methods
   operator const gd::argument::arguments& ( ) const { return m_arguments; }

   // @API [tag: getter, setter] [summary: get and set members]

   [[nodiscard]] std::string_view get_table() const noexcept { return m_stringTable; }

   // @API [tag: attribute] [summary: set attribute values for condition]

   condition_builder& value( gd::variant_view v_ )& { m_arguments.set( "value", v_ );   return *this; }
   condition_builder&& value( gd::variant_view v_ )&& { m_arguments.set( "value", v_ );   return std::move( *this ); }

   condition_builder& raw( std::string_view v_ )& { m_arguments.set( "raw", v_ );     return *this; }
   condition_builder&& raw( std::string_view v_ )&& { m_arguments.set( "raw", v_ );     return std::move( *this ); }

   condition_builder& type( gd::variant_view v_ )& { m_arguments.set( "type", v_ );    return *this; }
   condition_builder&& type( gd::variant_view v_ )&& { m_arguments.set( "type", v_ );    return std::move( *this ); }

   condition_builder& op( std::string_view v_ )& { m_arguments.set( "operator", v_ );   return *this; }
   condition_builder&& op( std::string_view v_ )&& { m_arguments.set( "operator", v_ );   return std::move( *this ); }

   condition_builder& group( std::string_view v_ )& { m_arguments.set( "group", v_ );   return *this; }
   condition_builder&& group( std::string_view v_ )&& { m_arguments.set( "group", v_ );   return std::move( *this ); }

   // @API [tag: operator, shortcut] [summary: Comparison operator shortcuts]

   condition_builder& eq()& { m_arguments.set( "operator", "=" );    return *this; }
   condition_builder&& eq()&& { m_arguments.set( "operator", "=" );    return std::move( *this ); }

   condition_builder& ne()& { m_arguments.set( "operator", "!=" );   return *this; }
   condition_builder&& ne()&& { m_arguments.set( "operator", "!=" );   return std::move( *this ); }

   condition_builder& lt()& { m_arguments.set( "operator", "<" );    return *this; }
   condition_builder&& lt()&& { m_arguments.set( "operator", "<" );    return std::move( *this ); }

   condition_builder& le()& { m_arguments.set( "operator", "<=" );   return *this; }
   condition_builder&& le()&& { m_arguments.set( "operator", "<=" );   return std::move( *this ); }

   condition_builder& gt()& { m_arguments.set( "operator", ">" );    return *this; }
   condition_builder&& gt()&& { m_arguments.set( "operator", ">" );    return std::move( *this ); }

   condition_builder& ge()& { m_arguments.set( "operator", ">=" );   return *this; }
   condition_builder&& ge()&& { m_arguments.set( "operator", ">=" );   return std::move( *this ); }

   condition_builder& like()& { m_arguments.set( "operator", "like" ); return *this; }
   condition_builder&& like()&& { m_arguments.set( "operator", "like" ); return std::move( *this ); }

   condition_builder& in()& { m_arguments.set( "operator", "in" );   return *this; }
   condition_builder&& in()&& { m_arguments.set( "operator", "in" );   return std::move( *this ); }

   condition_builder& is_null()& { m_arguments.set( "operator", "null" );   return *this; }
   condition_builder&& is_null()&& { m_arguments.set( "operator", "null" );   return std::move( *this ); }

   condition_builder& is_not_null()& { m_arguments.set( "operator", "notnull" ); return *this; }
   condition_builder&& is_not_null()&& { m_arguments.set( "operator", "notnull" ); return std::move( *this ); }

   // @API [tag: logical, operator, shortcut] [summary: Logical grouping shortcuts]

   condition_builder& and_()& { m_arguments.set( "logical", "and" );  return *this; }
   condition_builder&& and_()&& { m_arguments.set( "logical", "and" );  return std::move( *this ); }

   condition_builder& or_()& { m_arguments.set( "logical", "or" );   return *this; }
   condition_builder&& or_()&& { m_arguments.set( "logical", "or" );   return std::move( *this ); }

   condition_builder& not_()& { m_arguments.set( "logical", "not" );  return *this; }
   condition_builder&& not_()&& { m_arguments.set( "logical", "not" );  return std::move( *this ); }

   condition_builder& and_( std::string_view stringGroup )& { m_arguments.set( "logical", "and" ); m_arguments.set( "group", stringGroup );  return *this; }
   condition_builder&& and_( std::string_view stringGroup )&& { m_arguments.set( "logical", "and" ); m_arguments.set( "group", stringGroup );  return std::move( *this ); }

   condition_builder& or_( std::string_view stringGroup )& { m_arguments.set( "logical", "or" ); m_arguments.set( "group", stringGroup );   return *this; }
   condition_builder&& or_( std::string_view stringGroup )&& { m_arguments.set( "logical", "or" ); m_arguments.set( "group", stringGroup );   return std::move( *this ); }

   condition_builder& not_( std::string_view stringGroup )& { m_arguments.set( "logical", "not" ); m_arguments.set( "group", stringGroup );  return *this; }
   condition_builder&& not_( std::string_view stringGroup )&& { m_arguments.set( "logical", "not" ); m_arguments.set( "group", stringGroup );  return std::move( *this ); }


   std::string_view m_stringTable;      ///< optional table name for condition (used for disambiguation in joins)
   gd::argument::arguments m_arguments; ///< arguments used to pass into query
};

/// global method to create condition builder: condition_g("name").value(100).gt();
inline condition_builder condition_g( std::string_view stringName ) { return condition_builder{ stringName }; }
inline condition_builder condition_g( std::string_view stringTable, std::string_view stringName ) { return condition_builder{ stringTable, stringName }; }

/// global method to create condition builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
   requires ( !std::convertible_to<CONTAINER, std::string_view> )
inline condition_builder condition_g( std::string_view stringName, CONTAINER& buffer_ ) {
   return condition_builder{ stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof( typename CONTAINER::value_type )} };
}

/// global method to create condition builder using any container — std::array, std::vector, std::span, gd::memory::arena span
template<typename CONTAINER>
   requires ( !std::convertible_to<CONTAINER, std::string_view> )
inline condition_builder condition_g( std::string_view stringTable, std::string_view stringName, CONTAINER& buffer_ ) {
   return condition_builder{ stringTable, stringName, std::span<std::byte>{(std::byte*)buffer_.data(), buffer_.size() * sizeof( typename CONTAINER::value_type )} };
}

/// global method to create condition builder using raw C buffer
inline condition_builder condition_g( std::string_view stringName, void* pBuffer_, std::size_t uSize ) {
   return condition_builder{ stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize} };
}

/// global method to create condition builder using raw C buffer
inline condition_builder condition_g( std::string_view stringTable, std::string_view stringName, void* pBuffer_, std::size_t uSize ) {
   return condition_builder{ stringTable, stringName, std::span<std::byte>{(std::byte*)pBuffer_, uSize} };
}


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
inline query& operator<<( query& query_, condition_builder&& conditionbuilder_ )
{
   if( conditionbuilder_.get_table().empty() == false )
   {
      const auto* ptable_ = query_.table_get( conditionbuilder_.get_table() );                      assert( ptable_ != nullptr && "Table not found in query" );
      query_.condition_add( *ptable_, conditionbuilder_, tag_arguments{} );
   }
   else
   {
      query_.condition_add( conditionbuilder_, tag_arguments{} );
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
