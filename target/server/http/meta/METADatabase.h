/** @CLASS [name: CDatabase] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_types.h"
#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

#ifndef NAMESPACE_META_BEGIN

#  define NAMESPACE_META_BEGIN namespace META {
#  define NAMESPACE_META_END  }

#endif

NAMESPACE_META_BEGIN

class CDatabase
{
   enum enumColumnTable {
      // eColumnTableKey = 0,
   };
   
   enum enumColumnConnection {
      // eColumnTableKey = 0,
   };
   
   // @API [tag: construction]
public:
   CDatabase() {}
   // copy
   CDatabase( const CDatabase& o ) { common_construct( o ); }
   CDatabase( CDatabase&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CDatabase& operator=( const CDatabase& o ) { common_construct( o ); return *this; }
   CDatabase& operator=( CDatabase&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CDatabase() {}
private:
   // common copy
   void common_construct( const CDatabase& o ) {}
   void common_construct( CDatabase&& o ) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]
   std::pair<bool, std::string> Initialize();
   
   std::pair<bool, std::string> Add( gd::table::dto::table& tableTable, gd::types::tag_table );
   std::pair<bool, std::string> Add( gd::table::dto::table& tableTable, gd::types::tag_column );

   /// Check if tables are ready to be connected
   bool IsReadyToLinkTables() const;
   /// Connect tables to be able to find metadata for different situations
   std::pair<bool, std::string> LinkTablesTables();


   /// Compute max length of text field in vector of string views
   std::pair<bool, std::string> ComputeTextLength( std::string_view stringTable, std::vector<std::string_view> vectorField, uint64_t* puMaxLength ) const;


protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   std::unique_ptr<gd::table::arguments::table> m_ptableTable;  ///< table holding list of tables
   std::unique_ptr<gd::table::arguments::table> m_ptableColumn;  ///< table holding column information
   std::unique_ptr<gd::table::arguments::table> m_ptableJoin;  ///< table with connections between tables
   std::unique_ptr<gd::table::arguments::table> m_ptableComputed;  ///< table with computed columns



// @API [tag: free-functions]
public:
   static void CreateTable_s( gd::table::arguments::table& tableTable );    ///< create session table structure
   static void CreateColumn_s( gd::table::arguments::table& tableColumn ); ///< create table structure for column information
   static void CreateJoin_s( gd::table::arguments::table& tableJoin ); ///< create session table structure
   static void CreateComputed_s( gd::table::arguments::table& tableComputed );

};

NAMESPACE_META_END
