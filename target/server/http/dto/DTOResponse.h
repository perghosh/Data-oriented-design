// @FILE [tag: dto, http] [description: Data transfer object for HTTP response] [type: header] [name: CDTOResponse.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_table_arguments.h"

#include "../Types.h"

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
  * Holds response data in a table, there are pointers to objects used to store response
  * data or text data directly in the table.
  * Information stored in table will be used to create the actual HTTP response.
  *
  \code
  \endcode
  */
class CDTOResponse
{
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

std::pair<bool, std::string> AddTransfer( Types::Objects* pobjects_ );

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