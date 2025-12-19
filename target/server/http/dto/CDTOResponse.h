// @FILE [tag: dto, http] [description: Data transfer object for HTTP response] [type: header] [name: CDTOResponse.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_table_arguments.h"

/**
 * \file CDTOResponse.h
 *
 * \brief Data transfer object for HTTP response
 *
 *
 */

 /** @CLASS [name: CDTOResponse] [description:  ]
  * 
  * \brief Data transfer object for HTTP response
  *
  *
  *
  \code
  \endcode
  */
class CDTOResponse
{
public:
   enum enumType
   {
      eTypeUnknown         = 0,  // unknown type
      eTypeTextPlain       = 1,  // plain text
      eTypeTextXml         = 2,  // xml text
      eTypeTextJson        = 3,
      eTypeTextCsv         = 4,
      eTypeTableDto        = 5,  // dto::table object 
   };

   // @API [tag: construction]
public:
   CDTOResponse(): m_tableBody( gd::table::tag_full_meta{} ) {}
   // copy
   CDTOResponse( const CDTOResponse& o ) { common_construct( o ); }
   CDTOResponse( CDTOResponse&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CDTOResponse& operator=( const CDTOResponse& o ) { common_construct( o ); return *this; }
   CDTOResponse& operator=( CDTOResponse&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CDTOResponse() {}
private:
   // common copy
   void common_construct( const CDTOResponse& o ) {}
   void common_construct( CDTOResponse&& o ) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]

void Initialize();


protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   gd::table::arguments::table m_tableBody;   ///< response headers

   inline static gd::table::detail::columns* m_pcolumnsBody_s = nullptr; ///< static columns for body


   // @API [tag: free-functions]
public:



};