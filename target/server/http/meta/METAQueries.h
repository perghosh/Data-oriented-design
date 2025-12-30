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
   enum enumFormat { eFormatText, eFormatJinja, eFormatXml, eFormatJson };
   enum enumColumn { eColumnId, eColumnTime, eColumnIp4, eColumnIp6 };
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

   bool Empty() const { return m_tableQuery.empty(); }                                            ///< check if there are any active queries

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



};

NAMESPACE_META_END
