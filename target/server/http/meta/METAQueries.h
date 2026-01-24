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

   bool Empty() const { return m_tableQuery.empty(); }                                            ///< check if there are any active queries

   int64_t Find( const gd::argument::arguments& arguments_ ) const;                               ///< find query by arguments, returns row index or -1 if not found

   // @API [tag: access, row] [description: Methods to return query row data]
   gd::types::uuid GetQueryId( uint64_t uRow );
   std::pair<bool, std::string> GetQuery( std::string_view stringName, std::string& stringQuery );

   // @API [tag: load, save]
   
   std::pair<bool, std::string> Load( std::string_view stringPath );
   
protected:
   // @API [tag: internal]

public:
   // @API [tag: debug]

   // ## attributes ----------------------------------------------------------------
public:
   gd::argument::shared::arguments m_argumentProperty; ///< properties for session management

   gd::table::arguments::table m_tableQuery; ///< table holding active queries


   // @API [tag: free-functions]
public:
   static void CreateTable_s( gd::table::arguments::table& tableQuery );    ///< create session table structure
   
   static uint16_t ToType_s( std::string_view stringType );
   static uint16_t ToFormat_s( std::string_view stringFormat );



};

NAMESPACE_META_END
