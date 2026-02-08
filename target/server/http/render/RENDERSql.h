// @FILE [tag: sql, build] [description: sql render] ] [type: source] [name: RENDERSql.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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
   // @API [tag: construction]
public:
   CRENDERSql() {}
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
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]

   void Initialize();
   void AddValue( const gd::argument::arguments argumentsField );
   void Add( std::string_view stringName, std::string_view stringValue );
   void Add( std::string_view stringName, gd::variant_view variantviewValue );


protected:
// @API [tag: internal]
   

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   static constexpr unsigned m_uMaxStringBufferLength_s = 16; ///< Maximum length for string names if not placed as arguments in table

   gd::table::arguments::table m_tableField;   ///< Values or Names used to produce query

   inline static gd::table::detail::columns* m_pcolumnsField_s = nullptr; ///< static columns for body

// @API [tag: free-functions]
public:
   /// Destroy the static columns used by this class, note that you have to call this before the program exits
   static void Destroy_s();
};
