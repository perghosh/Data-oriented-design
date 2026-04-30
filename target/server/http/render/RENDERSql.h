// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.h]

/*

*/

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_sql_types.h"
#include "gd/gd_arguments.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_sql_query.h"

#include "../Types.h"

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"


class CDocument;

/** @CLASS [name: CRENDERSql] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CRENDERSql
{
public:   

   /**
    * @brief Specifies the type of part in a database query.
    */
   enum enumPartType
   {
      ePartTypeUnknown,
      ePartTypeSelect,           ///< select part of query
      ePartTypeValue,            ///< value part of query, used for insert and update queries
      ePartTypeWhere,            ///< where part of query, used for select, update and delete queries
      ePartTypeReturning,        ///< returning part of query, used for select queries to specify columns to return
      enumPartType_Max
   };

   enum enumSqlRenderState
   {
      eSqlRenderStateInitial,
      eSqlRenderStatePrepared,
      eSqlRenderStateValidated,
      enumSqlRenderState_Max
   };

   enum enumSqlQueryType
   {
      eSqlQueryTypeInsert,
      eSqlQueryTypeUpdate,
      eSqlQueryTypeDelete,
      eSqlQueryTypeSelect,
      eSqlQueryTypeCount,
      enumSqlQueryType_Max
   };

   
   enum enumColumnField
   {
      eColumnFieldId,            ///< column id (key), used for internal purposes
      eColumnFieldSchema,        ///< schema for table field belongs to
      eColumnFieldTable,         ///< name for table field belongs to
      eColumnFieldName,        ///< name for column in table
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
   CRENDERSql(): m_tableField(8, gd::table::tag_full_meta{}) {}
   CRENDERSql( const CDocument* pdocument );
   CRENDERSql( const CDocument* pdocument, uint64_t uStatementRow );
   CRENDERSql( const CDocument* pdocument, gd::sql::enumSqlDialect eSqlDialect ): m_pdocument(pdocument), m_eSqlDialect(eSqlDialect), m_tableField(8, gd::table::tag_full_meta{}) {}
   CRENDERSql( const CDocument* pdocument, std::string_view stringDialect ): m_pdocument(pdocument), m_eSqlDialect( gd::sql::sql_get_dialect_g(stringDialect) ), m_tableField(8, gd::table::tag_full_meta{}) {}
   // copy
   CRENDERSql( const CRENDERSql& o ) { common_construct( o ); }
   CRENDERSql( CRENDERSql&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CRENDERSql& operator=( const CRENDERSql& o ) { common_construct( o ); return *this; }
   CRENDERSql& operator=( CRENDERSql&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CRENDERSql() {}
private:
   // common copy
   void common_construct( const CRENDERSql& o );
   void common_construct( CRENDERSql&& o ) noexcept;

// @API [tag: operator]
   CRENDERSql& operator()( gd::sql::enumSqlDialect eSqlDialect ) { m_eSqlDialect = eSqlDialect; return *this; }
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]
   void SetDialect( gd::sql::enumSqlDialect dialect ) noexcept { m_eSqlDialect = dialect; }
   void SetRowStatement( int64_t iRowStatement ) noexcept { m_iRowStatement = iRowStatement; } 
   int64_t GetRowStatement() const noexcept { return m_iRowStatement; }

// @API [tag: operation]

   /// Initializes the internal state, you have to call this before using the object
   void Initialize();

   std::pair<bool,std::string> Add( const pugi::xml_node& xmlnodeValues );

   /// Simplest form, adds value with name
   void AddColumn( std::string_view stringName, gd::variant_view variantviewValue );
   /// Add single column to internal table with columns, keys are matched against column names
   void AddColumn( const gd::argument::arguments& argumentsField );
   /// Add json formated object column to internal table with columns, keys are matched against column names
   std::pair<bool,std::string> AddColumn( std::string_view stringJson, gd::types::tag_json );

   std::pair<bool, std::string> SetColumn( std::string_view stringColumn, const gd::argument::arguments& argumentsColumn );
   void SetColumn( uint64_t uRow, const gd::argument::arguments& argumentsColumn );

   /// Gets value for row, returns empty string view if value is not string or row is out of range
   std::string_view GetValue( uint64_t uRow ) const;

   /// Finds row for column name, returns -1 if not found
   int64_t FindRowForColumnName( std::string_view stringName ) const;
   /// Finds row for column name and part type, returns -1 if not found
   int64_t Find( const gd::argument::arguments& argumentsColumn ) const;
   /// Remove row from internal tabö7
   void Remove( uint64_t uRow ) { assert( uRow < m_tableField.size() ); m_tableField.erase( uRow ); }

   // @API [tag: add, simple] [description: Add values for columns, simple and only use key value to identify column value is added for]

   /// Add multiple values for columns
   void AddValues( const gd::argument::arguments& argumentsField );
   std::pair<bool,std::string> AddValues( std::string_view stringJson, gd::types::tag_json );
   /// Add information to internal table storing data to generate query in column format (at least table and column) to connect to metadata
   std::pair<bool,std::string> AddColumns( std::string_view stringJson, gd::types::tag_json );


   void AddCondition( const gd::argument::arguments& argumentsCondition );
   void AddCondition( gd::argument::arguments&& argumentsCondition );
   size_t GetConditionCount() const { return m_vectorCondition.size(); }


   /// Adds data for a complete record for specified table
   std::pair<bool,std::string> AddRecord( std::string_view stringJson, gd::types::tag_json );

   std::pair<bool, std::string> Preprocess( std::string_view stringSqlTemplate );
   std::pair<bool, std::string> Prepare();
   std::pair<bool, std::string> Validate();

   void AddProperty( std::string_view key_, gd::variant_view value_ ) { m_argumentsProperty.append_argument( key_, value_ ); }
   gd::variant_view GetProperty( std::string_view key_ ) const { return m_argumentsProperty.get_argument( key_ ); }

   void Add( std::string_view stringName, std::string_view stringValue );
   void Add( std::string_view stringName, gd::variant_view variantviewValue );

   /// Counts the number of columns for the specified part type in the query.
   /// Useful to check if there is where values etc
   std::size_t CountPartType( enumPartType ePartType ) const;

   void SetColumnValue( std::string_view stringName, gd::variant_view variantviewValue );

   /// Fill up values in internal table where they are empty
   void FillColumn( enumColumnField eColumnField, gd::variant_view variantviewValue );

   // @API [tag: utility] [description: Various utility functions]

   size_t Size() const { return m_tableField.size(); }
   bool Empty() const { return m_tableField.get_row_count() == 0u; }

   // @API [tag: query] [description: prepare query object]

   std::pair<bool, std::string> Query_AddFields( gd::sql::query* pquery );

   // @API [tag: sql] [description: Generate SQL queries]
   
   std::pair<bool,std::string> GetQuery( enumSqlQueryType eSqlQueryType, std::string& stringQuery );
   std::pair<bool, std::string> GetQuery( std::string_view stringQueryType, std::string& stringQuery ) { return GetQuery( QueryType_s( stringQueryType ), stringQuery ); }
   
   std::pair<bool, std::string> ToSqlSelect( std::string& stringQuery );
   std::pair<bool, std::string> ToSqlInsert( std::string& stringQuery );
   std::pair<bool, std::string> ToSqlUpdate( std::string& stringQuery );
   std::pair<bool, std::string> ToSqlDelete( std::string& stringQuery );
   std::pair<bool, std::string> ToSql( std::string_view stringType, std::string& stringQuery );
   std::pair<bool, std::string> ToSqlFromTemplate( std::string_view stringTemplate, std::string& stringQuery );

   /// @API [tag: arguments] [description: Arguments methods]

   void ToArguments( gd::argument::arguments& arguments ) const;
   void ToArguments( gd::argument::shared::arguments& arguments ) const;

   // @API [tag: validate]

   /// Validates if the provided arguments are valid adding value to renderer
   std::pair<bool, std::string> Validate( gd::argument::arguments argumentsValue, unsigned* puFound = nullptr ) const;
   /// Validates if the provided arguments are valid for condition fields in renderer
   std::pair<bool, std::string> ValidateCondition( gd::argument::arguments argumentsValue ) const;

   //std::string Dump() const;
protected:
// @API [tag: internal]
   

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   const CDocument* m_pdocument = nullptr; ///< pointer to document, used to get arguments for query, acces internal data in web server
   gd::sql::enumSqlDialect m_eSqlDialect{ gd::sql::eSqlDialectUnknown }; /// database dialect, used to determine how the syntax of sql statements should be
   int64_t m_iRowStatement{ -1 };         ///< current row used active stament if this is connected.
   gd::table::arguments::table m_tableField;   ///< Values or Names used to produce query
   std::vector<gd::argument::arguments> m_vectorCondition; ///< arguments used for condition fields
   gd::argument::shared::arguments m_argumentsProperty; ///< arguments used for specific properties of query, this is used for example to store table name, where conditions, etc. as arguments instead of columns in table

   inline static gd::table::detail::columns* m_pcolumnsField_s = nullptr; ///< static columns for body
   static constexpr unsigned m_uMaxStringBufferLength_s = 16; ///< Maximum length for string names if not placed as arguments in table

// @API [tag: free-functions]
public:
   static constexpr enumSqlQueryType QueryType_s( std::string_view stringQueryType ) noexcept;

   /// Destroy the static columns used by this class, note that you have to call this before the program exits
   static void Destroy_s();
};

/// return value for row, returns empty string view if value is not string or row is out of range
inline std::string_view CRENDERSql::GetValue( uint64_t uRow ) const {
   if( uRow >= m_tableField.size() ) return std::string_view{};
   gd::variant_view value_ = m_tableField.cell_get_variant_view( uRow, eColumnFieldValue, gd::table::tag_not_null{} );  assert( value_.is_string() == true );
   return value_.as_string_view();
}

/// --------------------------------------------------------------------------
/// Converts a string representation of a SQL query type to its corresponding enum value. The comparison is case-insensitive.
constexpr CRENDERSql::enumSqlQueryType CRENDERSql::QueryType_s( std::string_view stringQueryType ) noexcept
{
   auto equals_no_case_ = []( std::string_view a_, std::string_view b_ ) constexpr noexcept -> bool
   {
      if( a_.size() != b_.size() ) return false;
      for( size_t i = 0; i < a_.size(); ++i )
      {
         unsigned char ca_ = static_cast<unsigned char>( a_[i] );
         unsigned char cb_ = static_cast<unsigned char>( b_[i] );

         if( ca_ >= 'A' && ca_ <= 'Z' ) ca_ = static_cast<unsigned char>( ca_ - 'A' + 'a' );
         if( cb_ >= 'A' && cb_ <= 'Z' ) cb_ = static_cast<unsigned char>( cb_ - 'A' + 'a' );

         if( ca_ != cb_ ) return false;
      }
      return true;
   };

   if( equals_no_case_( stringQueryType, "insert" ) ) return eSqlQueryTypeInsert;
   if( equals_no_case_( stringQueryType, "update" ) ) return eSqlQueryTypeUpdate;
   if( equals_no_case_( stringQueryType, "delete" ) ) return eSqlQueryTypeDelete;
   if( equals_no_case_( stringQueryType, "select" ) ) return eSqlQueryTypeSelect;
   if( equals_no_case_( stringQueryType, "count" ) )  return eSqlQueryTypeCount;

   return enumSqlQueryType_Max;
}

