/**
 * \file gd_sql_value.h
 * 
 * \brief Sql functions working on sql values in sql expressions
 * 
 */


#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_types.h"
#include "gd_arguments.h"
#include "gd_variant.h"
#include "gd_variant_view.h"

#ifndef _GD_SQL_QUERY_BEGIN
#  define _GD_SQL_QUERY_BEGIN namespace gd { namespace sql {
#  define _GD_SQL_QUERY_END } }
#endif

_GD_SQL_QUERY_BEGIN

struct tag_raw {};                                                             // tag dispatcher setting data without internal logic
struct tag_brace {};                                                           // tag dispatcher setting data without internal logic
struct tag_keep_not_found{};                                                   // tag for methods to keep something if not found/missing
struct tag_preprocess{};                                                       // tag to preprocess text before inserting values, using this with replacement adds flexibility 

/// Append ascii text as utf8 to string
void append_ascii( const uint8_t* pbszAscii, std::string& stringSql );
/// Append ascii text as utf8 to string
void append_ascii( const uint8_t* pbszAscii, size_t uLength, std::string& stringSql );
/// Append utf8 (that is the default) text to string object
void append_utf8( const uint8_t* pbszUft8, std::string& stringSql );

void append_g( const gd::variant& variantValue, std::string& stringSql );
void append_g( const gd::variant_view& variantValue, std::string& stringSql );
void append_g( const gd::variant_view& variantValue, std::string& stringSql, tag_raw );

inline void append_g( const gd::variant& variantValue, std::string& stringSql, tag_raw ) { append_g( gd::variant_view( variantValue ), stringSql, tag_raw{}); }

/// Make bulk text suitable for parameterized sql insert or updates
std::tuple<uint64_t,std::string,std::string> make_bulk_g( const std::string_view& stringFixed, const std::string_view& stringParameter, uint64_t uCount, uint64_t uBulkCount );

std::string replace_g( const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, tag_brace );
std::string replace_g( const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, tag_brace, tag_keep_not_found );

_GD_SQL_QUERY_END