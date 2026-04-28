// @FILE [tag: query] [description: http session] [type: header] [name: METAQueries.h]

/**
 * @file METAQueries.h
 * @brief Header file for the CQueries class.   
 */

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

#include "gd_modules/dbmeta/gd__statement.h"

#ifndef NAMESPACE_META_BEGIN

#  define NAMESPACE_META_BEGIN namespace META {
#  define NAMESPACE_META_END  }

#endif

NAMESPACE_META_BEGIN

/** @CLASS [name: CQueries] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CQueries
{
public:

   /// The type of query
   enum enumType { eTypeUnknown = 0, eTypeSelect = 1, eTypeInsert = 2, eTypeUpdate = 3, eTypeDelete = 4, eTypeAsk = 5, eTypeBatch = 6 };
   /// The format of how query is store before it is generated
   enum enumFormat { eFormatUnknown = 0, eFormatText = 1, eFormatJinja = 2, eFormatXml = 3, eFormatJson = 4 }; // 
   /// Column indexes for fixed columns
   enum enumColumn { eColumnId, eColumnFlags, eColumnType, eColumnFormat, eColumnName, eColumnQuery, eColumnMeta };
   // @API [tag: construction]
public:
   CQueries() {}
   // copy
   CQueries( const CQueries& o ) { common_construct( o ); }
   // assign
   CQueries& operator=( const CQueries& o ) { common_construct( o ); return *this; }

   ~CQueries() {}
private:
   // common copy
  void common_construct(const CQueries &o);

  // @API [tag: operator]
public:


   // ## methods ------------------------------------------------------------------
public:
   // @API [tag: get, set]

   // @API [tag: operation]
   std::pair<bool, std::string> Initialize( const gd::argument::arguments& arguments_ );           ///< initialize query manager

   std::pair<bool, std::string> Add( std::string_view stringQuery, enumFormat eFormat = eFormatText, const gd::argument::arguments* parguments_ = nullptr );  ///< add new query
   std::pair<bool, std::string> Add( std::string_view stringId, std::string_view stringType, std::string_view stringFormat, std::string_view stringQuery );  ///< add new query

   std::pair<bool, std::string> Delete( const std::pair<std::string_view,std::string_view>& pair_ );

   bool Empty() const { return m_statement.empty(); }                        ///< check if there are any active queries

   int64_t Find( const gd::argument::arguments& arguments_ ) const;           ///< find query by arguments, returns row index or -1 if not found

   // @API [tag: access, row] [description: Methods to return query row data]
   gd::types::uuid GetQueryId( uint64_t uRow );
   std::pair<bool, std::string> GetQuery( std::string_view stringName, std::string& stringQuery );
   std::string GetQuery( std::string_view stringName ) const; ///< get query text for query at specified row index, returns empty string if not found
   std::string_view GetQuery( uint64_t uRow ) const; ///< get query text for query at specified row index, returns empty string if not found
   std::string_view GetTable( uint64_t uRow ) const; ///< get table name for query at specified row index, returns empty string if not found

   int64_t GetQueryRow( std::string_view stringName ) const; ///< get row index for query with specified name, returns -1 if not found
   const gd::argument::shared::arguments* GetQueryArguments( uint64_t uRow ) const; ///< get arguments for query at specified row index, returns nullptr if not found

   std::vector< gd::variant_view > GetArgumentsValues( std::string_view stringName, std::string_view stringKey ) const; 
   std::vector< gd::variant_view > GetArgumentsValues( uint64_t uRow, std::string_view stringKey ) const; ///< return values from attached arguments with selected name

   // @API [tag: load, save]
   
   std::pair<bool, std::string> Load( std::string_view stringPath );

   size_t Size() const { return m_statement.size(); } ///< get number of active queries
   size_t Count( const gd::types::uuid& uuidKey ) const { return m_statement.count( uuidKey ); }///< count number of queries with specified key
   
protected:
   // @API [tag: internal]

public:
   // @API [tag: debug]

   // ## attributes ----------------------------------------------------------------
public:
   gd::argument::shared::arguments m_argumentProperty; ///< properties for session management

   std::mutex m_mutexStatement;
   gd::modules::dbmeta::statement m_statement; ///< statement object holding list of statements, this is used to generate queries from templates

// @API [tag: free-functions]
public:
   static std::pair<bool, std::string> Load_s( std::string_view stringFilename, gd::modules::dbmeta::statement& statement_, gd::types::tag_xml ); ///< Load queries from xml file


};

/// @brief Retrieves the query text for a query with the specified name. If no query is found, an empty string is returned.
inline std::string CQueries::GetQuery( std::string_view stringName ) const
{
   auto iIndex = m_statement.find( stringName );
   if( iIndex != -1 ) { return std::string( m_statement.get_statement( iIndex ) ); }
   return {};
}

/// @brief Retrieves the query text for a query at the specified row index
inline std::string_view CQueries::GetQuery( uint64_t uRow ) const
{                                                                                                  assert( uRow < m_statement.size() );
   return m_statement.get_statement( uRow );
}

inline std::string_view CQueries::GetTable( uint64_t uRow ) const
{                                                                                                  assert( uRow < m_statement.size() );
   return m_statement.get_table( uRow );
}

/// @brief Retrieves the row index of a query based on its name.
inline int64_t CQueries::GetQueryRow( std::string_view stringName ) const
{
   auto iIndex = m_statement.find( stringName );
   return iIndex;
}

/// @brief Retrieves the arguments associated with a query at the specified row index.
inline const gd::argument::shared::arguments* CQueries::GetQueryArguments( uint64_t uRow ) const
{
   return m_statement.find_arguments( uRow );
}

/// @brief Retrieves the values of arguments associated with a query based on its name.
inline std::vector< gd::variant_view > CQueries::GetArgumentsValues( std::string_view stringName, std::string_view stringKey ) const
{
   auto iIndex = m_statement.find( stringName );
   if( iIndex < 0 ) { return {}; }
   return GetArgumentsValues( static_cast<uint64_t>(iIndex), stringKey );
}

NAMESPACE_META_END
