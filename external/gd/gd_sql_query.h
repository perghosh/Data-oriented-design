// @FILE [tag: sql, query] [description: Core logic for SQL queries] [type: header] [name: gd_sql_query.h]

/*
## class query
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | query(), query(query&), query(query&&)                                              | Constructors for creating query objects with copy and move support.                                |
| Assignment          | operator=(query&), operator=(query&&)                                                | Copy and move assignment operators for query objects.                                             |
| Table Management    | table_add(...), table_get(...), table_exists(), table_size(), table_empty()         | Add, retrieve, check existence and count tables in query.                              |
| Table Iteration     | table_begin(), table_end()                                                          | Iterator methods for traversing tables in query.                                                |
| Field Management    | field_add(...), field_add_many(...), field_add_as_orderby(...), field_get(...), field_size(), field_empty() | Add, retrieve and count fields in query, including special orderby fields. |
| Field Iteration     | field_begin(), field_end()                                                          | Iterator methods for traversing fields in query.                                                |
| Condition Management | condition_add(...), condition_add_raw(...), condition_empty(), condition_size() | Add conditions, raw conditions and check status of conditions.                         |
| Condition Iteration | condition_begin(), condition_end()                                                    | Iterator methods for traversing conditions in query.                                             |
| Query Building      | add(...), operator+=(), set_attribute(...)                                        | Build and modify query components like tables, fields, conditions.                      |
| SQL Generation      | sql_get(), sql_get_select(), sql_get_from(), sql_get_where(), ...            | Generate SQL statements for different query parts and complete queries.                  |
| Dialect Support     | sql_set_dialect(...)                                                              | Set SQL dialect (SQLite, SQL Server, PostgreSQL, MySQL).                            |
| Attribute Access    | distinct(), limit()                                                                | Methods to access and modify query attributes like DISTINCT and LIMIT.                             |
| Key Management     | next_key()                                                                         | Method to generate unique keys for query components.                                             |
| Utility            | get_join_type_s(...), sql_get_join_text_s(...), get_where_operator_number_s(...) | Static utility methods for SQL syntax elements and operators.                                      |

## struct table (nested in query)
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | table(), table(unsigned, stringView), table(..., ...)                               | Constructors for table objects with various initialization options.                                |
| Assignment          | operator=(table&), operator=(table&&)                                                | Copy and move assignment operators for table objects.                                             |
| Property Access     | name(), alias(), alias(stringView), parent(), schema(), owner(), join(), key(), fk() | Access and modify table properties like name, alias, and relationships.                          |
| Arguments Access    | get_arguments()                                                                     | Access internal argument storage for table metadata.                                   |
| Modification       | append(...), append_if(...), set(...)                                            | Modify table properties and add new attributes.                                       |
| Comparison         | compare(...), has(...)                                                              | Compare table properties and check for specific attributes.                            |
| Reference Counting  | add_reference()                                                                     | Manage reference counting for table objects.                                          |

## struct field (nested in query)
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | field(), field(..., ...)                                                            | Constructors for field objects with various initialization options.                                 |
| Assignment          | operator=(field&), operator=(field&&)                                                | Copy and move assignment operators for field objects.                                             |
| Property Access     | operator[](...), name(), alias(), raw(), get_useandtype(), set_useandtype(...), get_table_key(), get_value() | Methods to access and modify field properties like name, alias, and type.                       |
| Arguments Access    | get_arguments()                                                                     | Access internal argument storage for field metadata.                                   |
| Modification       | append(...), append_argument(...), set(...)                                        | Field properties and add new attributes.                                       |
| Comparison         | compare(...), has(...)                                                              | Compare field properties and check for specific attributes.                            |
| Special Flags      | is_groupby(), is_orderby(), is_select()                                            | Check special field usage flags for GROUP BY, ORDER BY, and SELECT clauses.          |

## struct condition (nested in query)
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | condition(), condition(..., ...)                                                     | Constructors for condition objects with various initialization options.                              |
| Assignment          | operator=(condition&), operator=(condition&&)                                         | Copy and move assignment operators for condition objects.                                          |
| Property Access     | value(), name(), value_string(), raw(), get_table_key(), get_operator()            | Access condition properties like field name, operator, and value.                     |
| Arguments Access    | get_arguments()                                                                     | Access internal argument storage for condition metadata.                                |
| Modification       | append(...), append_argument(...), set(...)                                        | Modify condition properties and add new attributes.                                   |
| Comparison         | compare(...), has(...)                                                              | Compare condition properties and check for specific attributes.                          |

## Enums and Constants
| Area                | Values                                                                            | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| SQL Dialect        | eSqlDialectSqlServer, eSqlDialectSqlite, eSqlDialectPostgreSql, eSqlDialectMySql | Supported SQL dialects for query generation.                                                   |
| Format Options      | eFormatUseQuotes, eFormatAddASKeyword, eFormatAddINNERKeyword                    | Formatting options for SQL generation syntax.                                                    |
| Join Types          | eJoinInner, eJoinLeft, eJoinRight, eJoinFull                                     | Supported SQL JOIN types for table relationships.                                                 |
| Operator Types      | eOperatorEqual, eOperatorNotEqual, eOperatorLess, eOperatorLike, ...        | Supported SQL operators for WHERE conditions.                                                    |
| Query Parts          | eSqlSelect, eSqlInsert, eSqlUpdate, eSqlDelete, eSqlFrom, ...                | SQL statement part identifiers for query building.                                               |

*/

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_sql_types.h"
#include "gd_arguments.h"
#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_vector.h"


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
_GD_SQL_QUERY_BEGIN
#else
_GD_SQL_QUERY_BEGIN
#endif

using namespace gd::argument;


/**
 * @brief Generate sql queries
 *
 *
 *
 @code
TEST_CASE( "Create sql for the FROM part between two tables", "[sql]" ) {
   using namespace gd::sql;
   query queryJoin;

	auto ptable_ = queryJoin.table_add( "parent" ); // table is called "parent"
	ptable_->set( "key", "parent_k" );              // set primary key name for parent table

	queryJoin.table_add( "child", "", "parent" );   // add table that has foreign key to "parent", is a type of child table for "parent"

	auto stringJoin = queryJoin.sql_get_from();     // generate join sql
	std::cout << stringJoin << "\n";
}

 @endcode
 */
class query
{

public:
   /*-----------------------------------------*/ /**
    * \brief information for tables used in query
    *
    *
    */
   struct table
   {
      table() {}
      explicit table( unsigned uTable ): m_uKey(uTable) {}
      table(unsigned uTable, const std::string_view& stringName): m_uKey(uTable) { append("name", stringName); }
      table(unsigned uTable, const std::string_view& stringName, const std::string_view& stringAlias): m_uKey(uTable) { append("name", stringName); append_if("alias", stringAlias); }
      table(unsigned uTable, const std::string_view& stringName, const std::string_view& stringAlias, const std::string_view& stringParent): m_uKey(uTable) { append("name", stringName); append_if("alias", stringAlias); append_if("parent", stringParent); }
      table( const table& o ) { common_construct( o ); }
      table( table&& o ) noexcept { common_construct( o ); }
      table& operator=( const table& o ) { common_construct( o ); return *this; }
      table& operator=( table&& o ) noexcept { common_construct( o ); return *this; }

      void common_construct(const table& o) { m_uKey = o.m_uKey; m_iReferenceCount = o.m_iReferenceCount; m_argumentsTable = o.m_argumentsTable; }
      void common_construct( table&& o ) noexcept { m_uKey = o.m_uKey; m_iReferenceCount = o.m_iReferenceCount; m_argumentsTable = std::move( o.m_argumentsTable ); }

      operator unsigned() const { return m_uKey;  }


      std::string name() const { return m_argumentsTable["name"].get_string(); }
      std::string alias() const { return m_argumentsTable["alias"].get_string(); }
      void alias(const std::string_view& stringAlias) { m_argumentsTable.set( "alias", stringAlias ); }
      std::string parent() const { return m_argumentsTable["parent"].get_string(); }
      std::string schema() const { return m_argumentsTable["schema"].get_string(); }
      std::string owner() const { return m_argumentsTable["owner"].get_string(); }
      std::string join() const { return m_argumentsTable["join"].get_string(); }
      /// get key name
      std::string key() const { return m_argumentsTable["key"].get_string(); }
      /// get foreign key name
      std::string fk() const { return m_argumentsTable["fk"].get_string(); }

      arguments& get_arguments() { return m_argumentsTable; }
      const arguments& get_arguments() const { return m_argumentsTable; }

      template<typename VALUE>
      table& append(std::string_view stringName, const VALUE& v) { m_argumentsTable.append(stringName, v); return *this; }
      template<typename VALUE>
      table& append_if( std::string_view stringName, const VALUE& v ) { gd::variant_view VV( v ); if( VV.is_true() ) { m_argumentsTable.append_argument( stringName, v ); } return *this; }
      table& set(std::string_view stringName, const gd::variant_view& v) { m_argumentsTable.set(stringName, v); return *this; }
      table& set(const arguments& v) { m_argumentsTable = v; return *this; }
      //table& set(const std::string_view& stringName, const arguments::argument& v) { m_argumentsTable.set(stringName, v); return *this; }
      bool has(std::string_view stringName) const { return m_argumentsTable.find(stringName) != nullptr; }
      bool compare(const std::pair<std::string_view, gd::variant_view>& pairMatch) const { return m_argumentsTable.find(pairMatch) != nullptr; }

      bool compare(unsigned uKey) const { return m_uKey == uKey; }

      void add_reference() { m_iReferenceCount++; }

      // attributes
      public:
         unsigned m_uKey = 0;        ///< key to table used by other object in query (field belongs to table)
         int m_iReferenceCount = 0;  ///< if table is in use by other items
         arguments m_argumentsTable; ///< all table properties
   };


   /*-----------------------------------------*/ /**
    * \brief information for fields used in query
    *
    *
    */
   struct field
   {
      field() {}
      explicit field(unsigned uTable) : m_uTableKey(uTable) {}
      explicit field( unsigned uTable, unsigned uUseAndType ) : m_uTableKey( uTable ), m_uUseAndType{ uUseAndType } {}
      field( unsigned uTable, const arguments& arguments_ ): m_uTableKey(uTable), m_uUseAndType(0), m_argumentsField(arguments_) {}
      field( unsigned uTable, unsigned uUseAndType, const arguments& arguments_ ): m_uTableKey(uTable), m_uUseAndType(uUseAndType), m_argumentsField(arguments_) {}
      field(unsigned uTable, std::string_view stringName): m_uTableKey(uTable) { append("name", stringName); }
      field(unsigned uTable, std::string_view stringName, std::string_view stringAlias): m_uTableKey(uTable) { append("name", stringName); append("alias", stringAlias); }
      field( const field& o ) { common_construct( o ); }
      field( field&& o ) noexcept { common_construct( o ); }
      field& operator=( const field& o ) { common_construct( o ); return *this; }
      field& operator=( field&& o ) noexcept { common_construct( o ); return *this; }

      void common_construct(const field& o) { m_uTableKey = o.m_uTableKey; m_uUseAndType = o.m_uUseAndType; m_argumentsField = o.m_argumentsField; }
      void common_construct( field&& o ) noexcept { m_uTableKey = o.m_uTableKey; m_uUseAndType = o.m_uUseAndType; m_argumentsField = std::move( o.m_argumentsField ); }

      arguments::argument operator[](const std::string_view& stringName) const noexcept { return m_argumentsField[stringName]; }

      unsigned get_useandtype() const noexcept { return m_uUseAndType; }
      void set_useandtype( unsigned uSet, unsigned uClear ) { m_uUseAndType |= uSet; m_uUseAndType &= ~uClear;  }

      unsigned get_table_key() const { return m_uTableKey; }
      arguments::argument get_value(const std::string_view& stringName) const noexcept { return m_argumentsField[stringName]; }

      std::string name() const { return m_argumentsField["name"].get_string(); }
      std::string alias() const { return m_argumentsField["alias"].get_string(); }
      std::string raw() const { return m_argumentsField["raw"].get_string(); }
      arguments::argument value() const { return m_argumentsField["value"]; }

      arguments& get_arguments() { return m_argumentsField; }
      const arguments& get_arguments() const { return m_argumentsField; }

      template<typename VALUE>
      field& append(std::string_view stringName, const VALUE& v) { m_argumentsField.append(stringName, v); return *this; }
      field& append_argument( std::string_view stringName, gd::variant_view v) { m_argumentsField.append_argument(stringName, v); return *this; }
      template<typename VALUE>
      field& set(std::string_view stringName, const VALUE& v) { m_argumentsField.set(stringName, v); return *this; }
      bool has(std::string_view stringName) const { return (m_argumentsField.find(stringName) != nullptr); }
      bool compare( const std::pair<std::string_view, gd::variant_view>& pairMatch) const { return m_argumentsField.find(pairMatch) != nullptr; }
      bool compare(const table* pTable) const { return m_uTableKey == *pTable; }

      bool is_groupby() const { return m_uUseAndType & eSqlPartGroupBy; }
      bool is_orderby() const { return m_uUseAndType & eSqlPartOrderBy; }
      bool is_select() const { return m_uUseAndType == 0 || m_uUseAndType & eSqlPartSelect; }


      // attributes
      public:
         unsigned m_uTableKey = 0;   ///< table that owns field
         unsigned m_uUseAndType = 0; ///< Field has specific rules (format or where to place it) flags and use type is used
                                     ///< default (m_uUseAndType = 0) and field is only used in select part
         arguments m_argumentsField; ///< all field properties
   };

   /*-----------------------------------------*/ /**
    * \brief condition item
    *
    *
    */
   struct condition
   {
      condition() {}
      explicit condition(unsigned uTable) : m_uTableKey(uTable) {}
      condition( unsigned uTable, const arguments& arguments_ ): m_uTableKey(uTable), m_argumentsCondition(arguments_) {}
      condition( const condition& o ) { common_construct( o ); }
      condition( condition&& o ) noexcept { common_construct( o ); }
      condition& operator=( const condition& o ) { common_construct( o ); return *this; }
      condition& operator=( condition&& o ) noexcept { common_construct( o ); return *this; }

      operator arguments() const { return m_argumentsCondition; }

      void common_construct(const condition& o) { m_uTableKey = o.m_uTableKey; m_argumentsCondition = o.m_argumentsCondition; }
      void common_construct(condition&& o) noexcept { m_uTableKey = o.m_uTableKey; m_argumentsCondition = std::move(o.m_argumentsCondition); }

      /// return value for conditions, this is places in arguments named to "value"
      arguments::argument value() const { return m_argumentsCondition["value"]; }

      std::string name() const { return m_argumentsCondition["name"].get_string(); }
      std::string value_string() const { return m_argumentsCondition["value"].get_string(); }
      std::string raw() const { return m_argumentsCondition["raw"].get_string(); }

      unsigned get_table_key() const { return m_uTableKey; }
      unsigned get_operator() const { return m_argumentsCondition["operator"].get_uint(); }

      arguments& get_arguments() { return m_argumentsCondition; }
      const arguments& get_arguments() const { return m_argumentsCondition; }


      template<typename VALUE>
      condition& append(std::string_view stringName, const VALUE& v) { m_argumentsCondition.append(stringName, v); return *this; }
      condition& append_argument(std::string_view stringName, gd::variant_view v) { m_argumentsCondition.append_argument(stringName, v); return *this; }
      template<typename VALUE>
      condition& set(std::string_view stringName, const VALUE& v) { m_argumentsCondition.set(stringName, v); return *this; }
      bool has(std::string_view stringName) const { return (m_argumentsCondition.find(stringName) != nullptr); }
      bool compare(const std::pair<std::string_view, gd::variant_view>& pairMatch) const { return m_argumentsCondition.find(pairMatch) != nullptr; }
      /// compare named value with sent condition, if both condition values for name match return true, otherwise false
      bool compare(const std::string_view& stringName, const condition* pconditionCompareTo) const { return m_argumentsCondition.compare( stringName, *pconditionCompareTo ); }



      // attributes
      public:
         unsigned m_uTableKey = 0;   ///< table that owns condition
         arguments m_argumentsCondition; ///< all condition properties
   };

// ## construction -------------------------------------------------------------
public:
   query() {}
   query( unsigned uFormatOptions ): m_uFormatOptions(uFormatOptions) {}
   query( std::string_view stringTable, tag_table ) { add( stringTable, tag_table{}); }
   query( unsigned uFormatOptions, std::string_view stringTable, tag_table ): m_uFormatOptions(uFormatOptions) { add( stringTable, tag_table{}); }
   // copy
   query( const query& o ) { common_construct( o ); }
   query( query&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   query& operator=( const query& o ) { common_construct( o ); return *this; }
   query& operator=( query&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

	~query() {}
private:
   // common copy
   void common_construct(const query& o);
   void common_construct(query&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:

   query& operator+=( const query& v ) { return add( v ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name TABLE
*///@{
   /// get pointer to first table in query (this should be the root table)
   const table* table_get() const { return &m_vectorTable[0]; }
   const table* table_get(const gd::variant_view& variantTable) const;
   table* table_get( const std::pair<std::string_view, gd::variant_view>& pairField );
   const table* table_get( const table& tableFind ) const noexcept;
   const table* table_get_for_key(unsigned uTableKey) const;
   table* table_add(const std::string_view& stringName);
   table* table_add(const std::string_view& stringName, const std::string_view& stringAlias);
   table* table_add(const std::string_view& stringName, const std::string_view& stringAlias, const std::string_view& stringparent );
   table* table_add(const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorTable );
   table* table_add(const table& tableAdd );
   bool table_exists( const table& tableExists ) const noexcept { return table_get( tableExists ) != nullptr; }
   std::size_t table_size() const { return m_vectorTable.size(); }
   bool table_empty() const { return m_vectorTable.empty(); }

   std::vector<table>::iterator table_begin() { return m_vectorTable.begin(); }
   std::vector<table>::const_iterator table_begin() const { return m_vectorTable.cbegin(); }
   std::vector<table>::iterator table_end() { return m_vectorTable.end(); }
   std::vector<table>::const_iterator table_end() const { return m_vectorTable.cend(); }
//@}


/** \name FIELD
*///@{
   // ## add fields to query
   field* field_add(const std::string_view& stringName) { return field_add(gd::variant_view(0u), stringName, std::string_view()); }
   void field_add(const std::vector<std::string_view>& vectorName, tag_name );
   field* field_add(const gd::variant_view& variantTable, std::string_view stringName) { return field_add(gd::variant_view(0u), stringName, std::string_view()); }
   field* field_add(const gd::variant_view& variantTable, std::string_view stringName, std::string_view stringAlias);
   field* field_add(const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorField) { return field_add( gd::variant_view(0u), vectorField ); }
   field* field_add(const gd::variant_view& variantTable, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorField );
   field* field_add( const field& fieldAdd ) { m_vectorField.push_back( fieldAdd ); return &m_vectorField.back(); }
   field* field_add( field&& fieldAdd ) { m_vectorField.push_back( std::move( fieldAdd ) ); return &m_vectorField.back(); }

   void field_add_many(const std::vector< std::vector< std::pair<std::string_view, gd::variant_view> > >& vectorVectorField );

   field* field_add_as_orderby(const gd::variant_view& variantviewField ) { return field_add_as_orderby( gd::variant_view(0u), variantviewField ); }
   field* field_add_as_orderby(const gd::variant_view& variantTable, const gd::variant_view& variantviewField );

   // ## get field in query
   const field* field_get(unsigned uIndex) const { assert(uIndex < m_vectorField.size()); return &m_vectorField[uIndex]; }
   field* field_get(unsigned uIndex) { assert(uIndex < m_vectorField.size()); return &m_vectorField[uIndex]; }
   field* field_get( const std::pair<std::string_view, gd::variant_view>& pairField );

   // ## various field operations and iterator

   std::size_t field_size() const { return m_vectorField.size(); }
   bool field_empty() const { return m_vectorField.empty(); }

   std::vector<field>::iterator field_begin() { return m_vectorField.begin();  }
   std::vector<field>::const_iterator field_begin() const { return m_vectorField.cbegin();  }
   std::vector<field>::iterator field_end() { return m_vectorField.end(); }
   std::vector<field>::const_iterator field_end() const { return m_vectorField.cend(); }
//@}

/** \name CONDITION
*///@{
   // ## add condition to query
   condition* condition_add(std::string_view stringName, const gd::variant_view& variantValue) { return condition_add( stringName, gd::variant_view(), variantValue ); }
   condition* condition_add(std::string_view stringName, const gd::variant_view& variantOperator, const gd::variant_view& variantValue);
   condition* condition_add(const gd::variant_view& variantTable, std::string_view stringName, const gd::variant_view& variantOperator, const gd::variant_view& variantValue);
   condition* condition_add(const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorCondition) { return condition_add( gd::variant_view(0u), vectorCondition ); }
   condition* condition_add(const gd::variant_view& variantTable, const std::vector< std::pair<std::string_view, gd::variant_view> >& vectorCondition );
   condition* condition_add( const condition& conditionAdd ) { m_vectorCondition.push_back( conditionAdd ); return &m_vectorCondition.back(); }
   condition* condition_add( condition&& conditionAdd ) { m_vectorCondition.push_back( std::move( conditionAdd ) ); return &m_vectorCondition.back(); }
   condition* condition_add_(const table* ptable, std::string_view stringName, const gd::variant_view& variantOperator, const gd::variant_view& variantValue);
   condition* condition_add_raw(const gd::variant_view& variantTable, const std::string_view& stringCondition);

   bool condition_empty() const noexcept { return m_vectorCondition.empty(); }
   size_t condition_size() const noexcept { return m_vectorCondition.size(); }
   std::vector<condition>::iterator condition_begin() { return m_vectorCondition.begin();  }
   std::vector<condition>::const_iterator condition_begin() const { return m_vectorCondition.cbegin();  }
   std::vector<condition>::iterator condition_end() { return m_vectorCondition.end(); }
   std::vector<condition>::const_iterator condition_end() const { return m_vectorCondition.cend(); }

//@}

/** \name ADD - simplified add operations wrapping other methods
*///@{
   query& add( const std::string_view& stringName, tag_table );
   query& add( const std::string_view& stringName, const std::string_view& stringAlias, tag_table );

   query& add( const gd::variant_view& stringTable, const std::string_view& stringName, tag_field );
   query& add( const gd::variant_view& stringTable, const std::string_view& stringName, const std::string_view& stringAlias, tag_field );
   query& add( const gd::variant_view& stringTable, const std::initializer_list< const char* > listField, tag_field );
   //query& add( const gd::variant_view& stringTable, std::initializer_list< std::pair<const std::string_view, const std::string_view> > listField, tag_field );
   query& add( const gd::variant_view& stringTable, std::initializer_list< std::pair<const char*, const char*> > listField, tag_field );
   query& add( const gd::variant_view& stringTable, const std::vector< std::pair<const std::string_view, const std::string_view> >& vectorField, tag_field );

   query& add( const gd::variant_view& stringTable, const std::string_view& stringCondition, tag_condition );

   query& add( const query& queryFrom );
//@}




/** \name OPERATION
*///@{
   template <typename VALUE>
   void set_attribute( const std::string_view& stringName, const VALUE& value_ ) { m_argumentsAttribute.set( stringName, value_ ); }
   gd::variant_view distinct() const { return m_argumentsAttribute["distinct"].get_variant_view(); }
   gd::variant_view limit() const { return m_argumentsAttribute["limit"].get_variant_view(); }


   /// Generate key values for internal data in query
   unsigned next_key() { return ++m_uNextKey; };
   // sql_update(), sql_update( iDbType )
   // sql_insert(), sql_insert( iDbType )

   void sql_set_dialect( enumSqlDialect eSqlDialect ) { m_eSqlDialect = eSqlDialect; }

   std::string sql_get_join_for_table( const table* ptable, const table* ptableParent ) const;
   std::string sql_get_join_for_table( const table* ptable ) const { return sql_get_join_for_table( ptable, nullptr ); }
   std::string sql_get_join_for_table( const std::string_view& stringTable ) const { return sql_get_join_for_table( table_get( stringTable ) ); }
   std::string sql_get_join_for_table( const table* ptable, const std::string_view& stringParentTable ) const { return sql_get_join_for_table( ptable, table_get( stringParentTable ) ); }

   [[nodiscard]] std::string sql_get_select() const;
   [[nodiscard]] std::string sql_get_from() const;
   [[nodiscard]] std::string sql_get_where() const;
   [[nodiscard]] std::string sql_get_insert() const;
   [[nodiscard]] std::string sql_get_update() const;
   [[nodiscard]] std::string sql_get_update( const std::vector< gd::variant_view >& vectorValue ) const;
   [[nodiscard]] std::string sql_get_delete() const;
   [[nodiscard]] std::string sql_get_groupby() const;
   [[nodiscard]] std::string sql_get_orderby() const;
   [[nodiscard]] std::string sql_get_limit() const;
   [[nodiscard]] std::string sql_get_with() const;
   [[nodiscard]] std::string sql_get_returning() const;

   [[nodiscard]] std::string sql_get( enumSql eSql ) const;
   [[nodiscard]] std::string sql_get( enumSql eSql, const unsigned* puPartOrder ) const;


//@}

protected:
/** \name INTERNAL
*///@{

//@}

// ## attributes ----------------------------------------------------------------
public:
   enumSqlDialect m_eSqlDialect;    ///< sql dialect (brand) used to generate sql
   unsigned m_uNextKey = 0;         ///< used to generate keys
   unsigned m_uFormatOptions;       ///< How to format query, has flags from `enumFormat`
   std::vector<table> m_vectorTable;///< list of tables used to generate query
   std::vector<field> m_vectorField;///< list of fields used to generate query
   std::vector<condition> m_vectorCondition;///< list of conditions used to generate query
   arguments m_argumentsAttribute;  ///< Attributes are values like `limit`, `distinct`


   static unsigned m_puPartOrder_s[];

// ## free functions ------------------------------------------------------------------
public:
   // ## SQL key words and type numbers
   static enumJoin get_join_type_s(const std::string_view& stringJoin);
   static std::string_view sql_get_join_text_s(enumJoin eJoinType);

   // ## SQL WHERE operator
   static enumOperator get_where_operator_number_s(std::string_view stringOperator);
   static enumOperator get_where_operator_number_s(const gd::variant_view& variantOperator);
   static unsigned get_where_operator_text_s(unsigned uOperator, char* pbBuffer);
   static bool operator_validate_s( int iOperator );
   static void print_condition_values_s( const std::vector<const condition*>& vectorCondition, std::string& stringValues );

   // ## Condition methods
   /// Find all conditions for same field and same operator
   static std::vector<std::size_t> condition_find_all_for_operator_s(const std::vector<condition>& vectorCondtion, const condition* pconditionMatch, unsigned uBegin);

   // ## flag methods
   /// Helper method to test if flag is set in unsigned 32 bit value
   template<typename FLAG>
   static bool flag_has_s(unsigned uTest, FLAG uFlag);

   // ## format methods
   /// add text and surround it with specified character, common operation when sql is generated
   static void format_add_and_surround_s(std::string& stringValue, const std::string_view& stringAdd, char chCharacter);

   // ## values methods, values used to build edit queries, values are formatted
   //    to work in sql queries
   static std::pair<bool, std::string> value_get_s( gd::variant_view& variantviewValue );
   static void value_get_s( const gd::variant_view& variantviewValue, std::string& stringSql );
   static std::pair<bool, std::string> values_get_s( const std::vector< gd::variant_view >& vectorValue );
   static std::pair<bool, std::string> values_get_s( const std::vector< gd::variant_view >& vectorValue, tag_value_view ) { return values_get_s( vectorValue ); }
   static std::pair<bool, std::string> values_get_s( const std::vector< gd::variant >& vectorValue );
   static std::pair<bool, std::string> values_get_s( const std::vector< gd::variant_view >& vectorValue, tag_value ) { return values_get_s( vectorValue ); }
   static void values_get_s( const std::vector< gd::variant_view >& vectorValue, std::string& stringValues );
   static std::pair<bool, std::string> values_get_s( const std::vector< std::pair<std::string, gd::variant> >& vectorValue );

   /// Generate `VALUES` compatible section that formats values to work in sql queries
   static std::pair<bool, std::string> values_get_s( std::vector< std::pair<uint32_t, gd::variant_view> >& vectorValue, unsigned uDialect );


   static std::pair<bool, std::string> add_s( query& queryTo, const query& queryFrom );

   static void returning_get_s( const gd::borrow::vector< std::pair< std::string_view, std::string_view > >& vectorValue, std::string& stringReturning, unsigned uDialect );
   static void returning_get_s( std::string_view stringColumn, std::string& stringReturning, unsigned uDialect, char iSplitColumn = ';', char iSplitAlias = ',');

};

/// Add table
/// @param stringName name for table added to query
inline query::table* query::table_add(const std::string_view& stringName) {
   m_vectorTable.push_back( table( next_key(), stringName ) );
   return &m_vectorTable.back();
}

/// Add table
/// @param stringName name for table added to query
inline query::table* query::table_add(const std::string_view& stringName, const std::string_view& stringAlias) {
   m_vectorTable.push_back( table( next_key(), stringName, stringAlias ) );
   return &m_vectorTable.back();
}

inline query::table* query::table_add( const std::string_view& stringName, const std::string_view& stringAlias, const std::string_view& stringparent ) {
   m_vectorTable.push_back( table( next_key(), stringName, stringAlias, stringparent ) );
   return &m_vectorTable.back();
}


/// Return table pointer for table key if found, nullptr if not found
inline const query::table* query::table_get_for_key(unsigned uTableKey) const {
   for( auto it = m_vectorTable.begin(); it != m_vectorTable.end(); it++ ) {
      if( it->compare(uTableKey) == true ) return &(*it);
   }
   return nullptr;
}

/// add field names only using the column name in database
inline void query::field_add( const std::vector<std::string_view>& vectorName, tag_name ) {
   for( auto it : vectorName ) field_add( it );
}

/// Add field with name and alias
inline void query::field_add_many(const std::vector< std::vector< std::pair<std::string_view, gd::variant_view> > >& vectorVectorField) {
   for( auto& it : vectorVectorField ) field_add(it);
}

/**----------------------------------------------------------------------------
 * @brief Check if operator number is within limitis
 * @param iOperator operator number to be checked
 * @return true if ok, false if not
*/
inline bool query::operator_validate_s( int iOperator ) {
   return ( iOperator >= 0 && eOperatorTypeNumberEND > (int)(iOperator & eOperatorMaskNumber));
}

/// Add table name to query
/// @param stringName name for table added to query
inline query& query::add( const std::string_view& stringName, tag_table ) {
   table_add( stringName ); return *this;
}

/// Add table name to query
/// @param stringName name for table added to query
inline query& query::add( const std::string_view& stringName, const std::string_view& stringAlias, tag_table ) {
   table_add( stringName, stringAlias ); return *this;
}


/// Add field to query
/// @param variantTable table field belongs to
/// @param stringName field name
inline query& query::add( const gd::variant_view& variantTable, const std::string_view& stringName, tag_field ) {
   field_add( variantTable, stringName ); return *this;
}

inline query& query::add( const gd::variant_view& variantTable, const std::string_view& stringName, const std::string_view& stringAlias, tag_field ) {
   field_add( variantTable, stringName, stringAlias ); return *this;
}

inline query& query::add( const gd::variant_view& variantTable, const std::initializer_list< const char* > listField, tag_field ) {
   for( const auto& it : listField ) {                                                             assert( it != nullptr );
      field_add( variantTable, it );
   }
   return *this;
}

/*
inline query& query::add( const gd::variant_view& variantTable, std::initializer_list< std::pair<const std::string_view, const std::string_view> > listField, tag_field ) {
   for( const auto& it : listField ) {                                                             assert( it.first.empty() == false );
      field_add( variantTable, it.first, it.second );
   }
   return *this;
}
*/

inline query& query::add( const gd::variant_view& variantTable, std::initializer_list< std::pair<const char*, const char*> > listField, tag_field ) {
   for( const auto& it : listField ) {
      assert( it.first != nullptr );
      if( it.second != nullptr ) field_add( variantTable, it.first, it.second );
      else                       field_add( variantTable, it.first );
   }
   return *this;
}

inline query& query::add( const gd::variant_view& variantTable, const std::vector< std::pair<const std::string_view, const std::string_view> >& vectorField, tag_field ) {
   for( const auto& it : vectorField ) {                                                           assert( it.first.empty() == false );
      if( it.second.empty() == false ) field_add( variantTable, it.first, it.second );
      else                             field_add( variantTable, it.first );
   }
   return *this;
}




inline query& query::add( const gd::variant_view& variantTable, const std::string_view& stringCondition, tag_condition ) {
   condition_add_raw( variantTable, stringCondition );  return *this;
}



template<typename FLAG>
bool query::flag_has_s(unsigned uTest, FLAG uFlag) {
   static_assert( sizeof(FLAG) >= 4, "Value isn't compatible with unsigned (4 byte)");
   return (uTest & (unsigned)uFlag) != 0;
}

/// add surrounded value to string, like XXXX => "XXXX"
inline void query::format_add_and_surround_s(std::string& stringText, const std::string_view& stringAdd, char chCharacter) {
   stringText += chCharacter;
   stringText += stringAdd;
   stringText += chCharacter;
}



_GD_SQL_QUERY_END // namespace _GD_CALCULATE_PARSE_BEGIN

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
