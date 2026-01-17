#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_arguments_shared.h"
#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

class CApplication;
class CDocument;

#ifndef NAMESPACE_SERVICE_BEGIN

#  define NAMESPACE_SERVICE_BEGIN namespace SERVICE {
#  define NAMESPACE_SERVICE_END  }

#endif

NAMESPACE_SERVICE_BEGIN

/** @CLASS [name: CSqlBuilder] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CSqlBuilder
{
public:

   /// The type of query
   enum enumType { eTypeUnknown = 0, eTypeSelect = 1, eTypeInsert = 2, eTypeUpdate = 3, eTypeDelete = 4, eTypeAsk = 5, eTypeBatch = 6 };
// @API [tag: construction]
public:
   CSqlBuilder() {}
   CSqlBuilder( gd::argument::shared::arguments& arguments_ ): m_argumentsValues( arguments_ ) {}
   // copy
   CSqlBuilder( const CSqlBuilder& o ) { common_construct( o ); }
   // assign
   CSqlBuilder& operator=( const CSqlBuilder& o ) { common_construct( o ); return *this; }

   ~CSqlBuilder() {}
private:
   // common copy
  void common_construct(const CSqlBuilder &o);
  void common_construct( CSqlBuilder&& o ) noexcept;
  
// @API [tag: operator]
public:
   CSqlBuilder& operator=( std::string_view stringSql ) { m_stringSql = stringSql; return *this; }
   CSqlBuilder& operator=( gd::argument::shared::arguments& argumentsValues ) { m_argumentsValues = argumentsValues; return *this; }


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]
   std::pair<bool, std::string> Initialize( gd::argument::shared::arguments arguments_ ); ///< initialize query manager
   std::pair<bool, std::string> Initialize( gd::argument::shared::arguments arguments_, std::string_view stringSql ); ///< initialize query manager
   
   std::pair<bool, std::string> Build( std::string& stringSqlReady ); ///< build query

protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uUserIndex{};            ///< user index for user in session table for users logged in
   const CDocument* m_pDocument = nullptr;
   enumType m_typeSql = eTypeUnknown; ///< type of query
   std::string m_stringSql;           ///< query template or raw query
   gd::argument::shared::arguments m_argumentsValues;

// @API [tag: free-functions]
public:

};

NAMESPACE_SERVICE_END
