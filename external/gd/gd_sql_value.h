// @FILE [tag: sql, format] [description: Format SQL values for SQL statements] [type: header] [name: gd_sql_value.h]
/**
 * \file gd_sql_value.h
 * 
 * \brief Sql functions working on sql values in sql expressions
 *
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0replace.sql` - replace arguments in string like std::format but with sql style
 * 
 */


#pragma once

#include <cassert>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include "gd_types.h"
#include "gd_sql_types.h"
#include "gd_arguments.h"
#include "gd_arguments_shared.h"
#include "gd_variant.h"
#include "gd_variant_view.h"

#ifndef _GD_SQL_QUERY_BEGIN
#  define _GD_SQL_QUERY_BEGIN namespace gd { namespace sql {
#  define _GD_SQL_QUERY_END } }
#endif

_GD_SQL_QUERY_BEGIN

extern char iBeginBrace_g;
extern char iEndBrace_g;
extern char iQuestion_g;
extern char iSemicolon_g;

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
void append_utf8( const uint8_t* puUft8, std::size_t uLength, std::string& stringSql );

void append_g( const gd::variant& variantValue, std::string& stringSql );
void append_g( const gd::variant_view& variantValue, std::string& stringSql );
void append_g( const gd::variant_view& variantValue, std::string& stringSql, tag_raw );

void append_g( std::string_view stringValue, unsigned uType, unsigned uDialect, std::string& stringSql );

inline void append_g( const gd::variant& variantValue, std::string& stringSql, tag_raw ) { append_g( gd::variant_view( variantValue ), stringSql, tag_raw{}); }

/// Make bulk text suitable for parameterized sql insert or updates
std::tuple<uint64_t,std::string,std::string> make_bulk_g( const std::string_view& stringFixed, const std::string_view& stringParameter, uint64_t uCount, uint64_t uBulkCount );

std::pair<bool,std::string> replace_g( const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, std::string& stringNew, tag_brace );
std::pair<bool,std::string> replace_g( const std::string_view& stringSource, const gd::argument::shared::arguments& argumentsValue, std::string& stringNew, tag_brace );

/// Replace values in string with values from arguments object, arguments replaced are in braces
inline std::string replace_g(const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, tag_brace) {
   std::string stringNew;
   replace_g(stringSource, argumentsValue, stringNew, tag_brace{});
   return stringNew;
}

/// Replace values in string with values from arguments object, arguments replaced are in braces
inline std::string replace_g(const std::string_view& stringSource, const gd::argument::shared::arguments& argumentsValue, tag_brace) {
   std::string stringNew;
   replace_g(stringSource, argumentsValue, stringNew, tag_brace{});
   return stringNew;
}


/// Replace values in string with values from arguments object, arguments replaced are in braces, if argument is not found it is kept. That enables multiple replacements.
std::string replace_g( const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, tag_brace, tag_keep_not_found );

std::string replace_g(const std::string_view& stringSource, std::function<gd::variant_view (const std::string_view&)> find_, bool* pbError, tag_preprocess);
inline std::string replace_g(const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, bool* pbError, tag_preprocess) {
   return replace_g(stringSource, [&argumentsValue] ( const auto& name_ ) -> gd::variant_view {
      return argumentsValue[name_].as_variant_view();
   }, pbError, tag_preprocess{});
}
inline std::string replace_g(const std::string_view& stringSource, const gd::argument::arguments& argumentsValue, tag_preprocess) {
   return replace_g( stringSource, argumentsValue, nullptr, tag_preprocess{});
}

_GD_SQL_QUERY_END
