// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_sql_types.h"
#include "gd/gd_arguments.h"
#include "gd/gd_table_arguments.h"

#include "../Types.h"

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
      eColumnFieldId,
      eColumnFieldTable,
      eColumnFieldColumn,
      eColumnFieldAlias,
      eColumnFieldValue,
      eColumnFieldType,
      eColumnField_Max
   };
   
// @API [tag: construction]
public:
   CRENDERSql(): m_tableField(8, gd::table::tag_full_meta{}) {}
   CRENDERSql( gd::sql::enumSqlDialect eSqlDialect ): m_eSqlDialect(eSqlDialect), m_tableField(8, gd::table::tag_full_meta{}) {}
   CRENDERSql( std::string_view stringDialect ): m_eSqlDialect( gd::sql::sql_get_dialect_g(stringDialect) ), m_tableField(8, gd::table::tag_full_meta{}) {}
   // copy
   CRENDERSql( const CRENDERSql& o ) { common_construct( o ); }
   CRENDERSql( CRENDERSql&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CRENDERSql& operator=( const CRENDERSql& o ) { common_construct( o ); return *this; }
   CRENDERSql& operator=( CRENDERSql&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CRENDERSql() {}
private:
   // common copy
   void common_construct( const CRENDERSql& o ) {}
   void common_construct( CRENDERSql&& o ) noexcept {}

// @API [tag: operator]
   CRENDERSql& operator()( gd::sql::enumSqlDialect eSqlDialect ) { m_eSqlDialect = eSqlDialect; return *this; }
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]
   void SetDialect( gd::sql::enumSqlDialect dialect ) noexcept { m_eSqlDialect = dialect; }

// @API [tag: operation]

   void Initialize();
   void AddValue( const gd::argument::arguments argumentsField );
   std::pair<bool,std::string> AddValue( std::string_view stringJson, gd::types::tag_json );
   /// Adds data for a complete record for specified table
   std::pair<bool,std::string> AddRecord( std::string_view stringJson, gd::types::tag_json );

   void Add( std::string_view stringName, std::string_view stringValue );
   void Add( std::string_view stringName, gd::variant_view variantviewValue );

   void SetColumnValue( std::string_view stringName, gd::variant_view variantviewValue );
   
   std::pair<bool,std::string> GetQuery( enumSqlQueryType eSqlQueryType, std::string& stringQuery );
   std::pair<bool, std::string> GetQuery( std::string_view stringQueryType, std::string& stringQuery ) { return GetQuery( QueryType_s( stringQueryType ), stringQuery ); }
   
   std::pair<bool,std::string> ToSqlInsert( std::string& stringQuery );

   //std::string Dump() const;
protected:
// @API [tag: internal]
   

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   static constexpr unsigned m_uMaxStringBufferLength_s = 16; ///< Maximum length for string names if not placed as arguments in table

   gd::sql::enumSqlDialect m_eSqlDialect = gd::sql::eSqlDialectUnknown;
   gd::table::arguments::table m_tableField;   ///< Values or Names used to produce query

   inline static gd::table::detail::columns* m_pcolumnsField_s = nullptr; ///< static columns for body

// @API [tag: free-functions]
public:
   static constexpr enumSqlQueryType QueryType_s( std::string_view stringQueryType ) noexcept;

   /// Destroy the static columns used by this class, note that you have to call this before the program exits
   static void Destroy_s();
};


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

