// @FILE [tag: dto, http] [description: Data transfer object for HTTP response] [type: header] [name: CDTOResponse.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_arguments.h"
#include "gd/gd_table_arguments.h"

#include "../Types.h"

/**
 * \file CDTOResponse.h
 *
 * \brief Data transfer object for HTTP response
 *
 *
 */

 /** @CLASS [name: CDTOResponse] [description: Store result information and pack it before sending to client ]
  * 
  * \brief Data transfer object for HTTP response
  *
  * Holds response data in a table, there are pointers to objects used to store response
  * data or text data directly in the table.
  * Information stored in table will be used to create the actual HTTP response.
  * 
  * Note that each respons may also add some sort of context information to help
  * the client to figure out what to do with result returned. This context may be
  * information that is passed with the request information.
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

   ~CDTOResponse();
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

void AddContext( std::string_view stringName, gd::variant_view value_ );

std::pair<bool, std::string> AddTransfer( Types::Objects* pobjects_ );

std::pair<bool, std::string> PrintXml( std::string& stringXml, const gd::argument::arguments* parguments_ );

/// Check if response body is empty
bool Empty() const noexcept { return m_tableBody.size() == 0; }

/// Clear response body (table with objects)
void Clear();

protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   gd::argument::arguments m_argumentsContext;  ///< response arguments that holds context information for response
   gd::table::arguments::table m_tableBody;   ///< response headers

   inline static gd::table::detail::columns* m_pcolumnsBody_s = nullptr; ///< static columns for body
   inline static std::string m_stringResults_s = "results";  ///< default container name for results
   inline static std::string m_stringResult_s = "result";   ///< default item name for each result


// @API [tag: free-functions]
public:
   static void Destroy_s();

};

/// Add context to response, this are values that in some way will append information to the generated result returned to client
inline void CDTOResponse::AddContext( std::string_view stringName, gd::variant_view value_ ) {
   m_argumentsContext.append_argument( stringName, value_ );
}