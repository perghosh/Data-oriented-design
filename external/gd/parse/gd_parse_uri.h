// @FILE {tag: parse, uri, read} [summary: URI parsing and manipulation utilities] [type: header] [name: gd_parse_uri.h]

#pragma once

#include "gd/gd_compiler.h"

#include <array>
#include <cassert>
#include <cstring>
#if GD_COMPILER_HAS_CPP20_SUPPORT
#include <span>
#endif
#include <string>
#include <string_view>
#include <vector>

#include "../gd_types.h"
#include "../gd_arguments.h"
#include "../gd_arguments_shared.h"


#ifndef _GD_PARSE_URI_BEGIN
#  define _GD_PARSE_URI_BEGIN namespace gd { namespace parse { namespace uri {
#  define _GD_PARSE_URI_END } } }
#endif

_GD_PARSE_URI_BEGIN

std::pair<bool, std::string> parse( std::string_view stringUri, gd::argument::arguments& argumentsUri );
std::pair<bool, std::string> parse( std::string_view stringUri, gd::argument::shared::arguments& argumentsUri );

std::pair<bool, std::string> parse_path( std::string_view stringPath, std::vector<std::string_view>& vectorSegments );

std::pair<bool, std::string> parse_query( std::string_view stringQuery, gd::argument::arguments& argumentsQuery );
std::pair<bool, std::string> parse_query( std::string_view stringQuery, gd::argument::shared::arguments& argumentsQuery );

/** --------------------------------------------------------------------------
 * \brief Helper to parse path from URI arguments
 * 
 * Convenience method that extracts and parses the "path" field from URI arguments.
 * 
 * @param argumentsUri The parsed URI arguments containing a "path" field
 * @param vectorSegments Output vector for path segments
 * @return Pair of success flag and error message
 */
template<typename ARGUMENTS>
std::pair<bool, std::string> parse_path_from_uri( const ARGUMENTS& argumentsUri, std::vector<std::string_view>& vectorSegments )
{
   // Find path in arguments
   for( std::size_t i = 0; i < argumentsUri.size(); i++ )
   {
      if( argumentsUri.name(i) == "path" )
      {
         return parse_path( argumentsUri[i].as_string_view(), vectorSegments );
      }
   }
   return { true, "" }; // No path found, not an error
}

/** --------------------------------------------------------------------------
 * \brief Helper to parse query from URI arguments
 * 
 * Convenience method that extracts and parses the "query" field from URI arguments.
 * 
 * @param argumentsUri The parsed URI arguments containing a "query" field
 * @param argumentsQuery Output arguments for parsed query parameters
 * @return Pair of success flag and error message
 */
template<typename ARGUMENTS_IN, typename ARGUMENTS_OUT>
std::pair<bool, std::string> parse_query_from_uri( const ARGUMENTS_IN& argumentsUri, ARGUMENTS_OUT& argumentsQuery )
{
   // Find query in arguments
   for( std::size_t i = 0; i < argumentsUri.size(); i++ )
   {
      if( argumentsUri.name(i) == "query" )
      {
         return parse_query_implementation( argumentsUri[i].as_string_view(), argumentsQuery );
      }
   }
   return { true, "" }; // No query found, not an error
}

std::pair< std::vector<std::string_view>, gd::argument::arguments > parse_path_and_query( std::string_view stringPathAndQuery );


_GD_PARSE_URI_END
