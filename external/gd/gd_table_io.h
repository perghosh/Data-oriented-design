/**
 * \file gd_table_io.h
 * 
 * \brief "io" means that table data are streamed from or to other storage items
 * 
 * 
 * 
 * 
 * 
 */

#pragma once

#include <cassert>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include "gd_types.h"
#include "gd_utf8.hpp"
#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_table.h"
#include "gd_table_column-buffer.h"
#include "gd_table_table.h"
#include "gd_table_aggregate.h"



#if defined( __clang__ )
   #pragma GCC diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wreorder-ctor"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 6387 26495 26812 )
#endif

_GD_TABLE_BEGIN

/// tag dispatcher for header information
struct tag_io_header {};
/// tag dispatcher for body information
struct tag_io_body {};
/// tag dispatcher for naming, value is in some way attached to name
struct tag_io_name {};
/// tag dispatcher for column information
struct tag_io_column {};





/// tag dispatcher used for csv formatting
struct tag_io_csv {};
/// tag dispatcher used for csv formatting
struct tag_io_json {};
/// tag dispatcher used for command line interface
struct tag_io_cli {};
/// tag dispatcher for uri formatting
struct tag_io_uri {};
/// tag dispatcher for sql formatting
struct tag_io_sql {};


bool format_if( const std::string_view& stringText, std::string& stringNew, tag_io_uri );

// ## Table IO ----------------------------------------------------------------

// ## CSV IO ------------------------------------------------------------------
// Two different version have logic to produce csv output, dto::table and table, the dto::table is more sutable to use for transfer of data
// and only table is used to store internal member information. Where the format do not need to change

// ### `dto::table`
void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_csv );
void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_csv );

inline void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_csv) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_csv{});
}
inline void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_csv) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_header{}, tag_io_csv{});
}

void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, bool (*format_text_)(unsigned uColumn, unsigned uType, const gd::variant_view&, std::string& stringNew), std::string& stringOut, tag_io_csv );

inline void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, bool ( *pformat_text_ )( unsigned uColumn, unsigned uType, const gd::variant_view&, std::string& stringNew ), std::string& stringOut, tag_io_csv) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), pformat_text_, stringOut, tag_io_csv{});
}

// ### `table`
void to_string( const table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_csv );
void to_string( const table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_csv );

inline void to_string(const table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_csv) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_csv{});
}
inline void to_string(const table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_csv) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_header{}, tag_io_csv{});
}

void to_string( const table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, bool (*format_text_)(unsigned uColumn, unsigned uType, const gd::variant_view&, std::string& stringNew), std::string& stringOut, tag_io_csv );

inline void to_string(const table& table, uint64_t uBegin, uint64_t uCount, bool ( *pformat_text_ )( unsigned uColumn, unsigned uType, const gd::variant_view&, std::string& stringNew ), std::string& stringOut, tag_io_csv) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), pformat_text_, stringOut, tag_io_csv{});
}


// ### read csv information into table
std::pair<bool, const char*> read_g( dto::table& table, const std::string_view& stringCsv, char chSeparator, char chNewLine, tag_io_csv );
std::pair<bool, const char*> read_g( dto::table& table, const std::string_view& stringCsv, const std::vector<unsigned>& vectorColumn, char chSeparator, char chNewLine, tag_io_csv );

// ### read csv information into vector
std::pair<bool, const char*> read_g( std::vector<std::string>& vectorHeader, const std::string_view& stringCsv, char chSeparator, char chNewLine, tag_io_csv );


// ## JSON IO -----------------------------------------------------------------

void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json );
void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_json );

void to_string( const dto::table& table, const std::vector<uint64_t>& vectorRow, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json );
void to_string( const dto::table& table, const std::vector<uint64_t>& vectorRow, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_json );

inline void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_json{});
}
inline void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_header, tag_io_json) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_header{}, tag_io_json{});
}

inline void to_string( const dto::table& table, const std::function<bool( const std::string_view&, std::string& stringNew )>& format_text_, std::string& stringOut, tag_io_json ) {
   to_string( table, uint64_t(0), table.get_row_count(), format_text_, stringOut, tag_io_json{});
}

inline std::string to_string( const dto::table& table, const std::function<bool( const std::string_view&, std::string& stringNew )>& format_text_, tag_io_json ) {
   std::string stringOut;
   to_string( table, uint64_t(0), table.get_row_count(), format_text_, stringOut, tag_io_json{});
   return stringOut;
}

/// convert complete table to json formated array
inline std::string to_string( const dto::table& table, tag_io_json ) {
   std::string stringOut;
   to_string( table, uint64_t(0), table.get_row_count(), nullptr, stringOut, tag_io_json{});
   return stringOut;
}

/// convert complete table with header to json formated array
inline std::string to_string( const dto::table& table, tag_io_header, tag_io_json ) {
   std::string stringOut;
   to_string( table, uint64_t(0), table.get_row_count(), nullptr, stringOut, tag_io_header{}, tag_io_json{});
   return stringOut;
}

/// print table values as json where each object on row is placed in json object with name, not as json array
void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, const std::function<bool (const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json, tag_io_name );

inline void to_string(const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::function<bool(const std::string_view&, std::string& stringNew)>& format_text_, std::string& stringOut, tag_io_json, tag_io_name) {
   return to_string( table, uBegin, uCount, gd::argument::arguments(), format_text_, stringOut, tag_io_header{}, tag_io_json{});
}

inline void to_string( const dto::table& table, const std::function<bool( const std::string_view&, std::string& stringNew )>& format_text_, std::string& stringOut, tag_io_json, tag_io_name ) {
   to_string( table, uint64_t(0), table.get_row_count(), format_text_, stringOut, tag_io_json{}, tag_io_name{});
}

inline void to_string( const dto::table& table, std::string& stringOut, tag_io_json, tag_io_name ) {
   to_string( table, uint64_t(0), table.get_row_count(), nullptr, stringOut, tag_io_json{}, tag_io_name{});
}


inline std::string to_string( const dto::table& table, const std::function<bool( const std::string_view&, std::string& stringNew )>& format_text_, tag_io_json, tag_io_name ) {
   std::string stringOut;
   to_string( table, uint64_t(0), table.get_row_count(), format_text_, stringOut, tag_io_json{}, tag_io_name{});
   return stringOut;
}

inline std::string to_string( const dto::table& table, tag_io_json, tag_io_name ) {
   std::string stringOut;
   to_string( table, uint64_t(0), table.get_row_count(), nullptr, stringOut, tag_io_json{}, tag_io_name{});
   return stringOut;
}

void to_string( const dto::table& table, std::string& stringOut, tag_io_json, tag_io_column );
void to_string( const dto::table& table, std::string& stringOut, const std::vector<gd::argument::arguments>& vectorExtra, tag_io_json, tag_io_column );


// ## CLI IO (command line interface) -----------------------------------------
//    CLI output is to produce good formating for command line interface

/// convert table header information to string that works for console
void to_string( const dto::table& table, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_header, tag_io_cli );
inline void to_string(const dto::table& table, std::string& stringOut, tag_io_header, tag_io_cli) {
   to_string( table, {}, stringOut, tag_io_header{}, tag_io_cli{});
}
inline std::string to_string(const dto::table& table, tag_io_header, tag_io_cli) {
   std::string stringOut;
   to_string( table, {}, stringOut, tag_io_header{}, tag_io_cli{});
   return stringOut;
}


void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli );
void to_string( const table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli );

void to_string( const dto::table& table, uint64_t uBegin, uint64_t uCountconst, std::vector<unsigned> vectorWidth, const std::vector<unsigned>& vectorcolumn, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli );
void to_string( const table& table, uint64_t uBegin, uint64_t uCount, std::vector<unsigned> vectorWidth, const std::vector<unsigned>& vectorcolumn, const gd::argument::arguments& argumentOption, std::string& stringOut, tag_io_cli );

inline void to_string( const dto::table& table, const std::vector<unsigned> vectorWidth, std::string& stringOut, tag_io_cli ) {
   to_string( table, 0, table.get_row_count(), vectorWidth, {}, stringOut, tag_io_cli{} );
}

/// convert table to grid where each column in formated with fixed width, witdth for columns are found in vector
inline std::string to_string( const dto::table& table, const std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, tag_io_cli ) {
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, argumentOption, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to grid where each column in formated with fixed width, witdth for columns are found in vector
inline std::string to_string( const table& table, const std::vector<unsigned> vectorWidth, const gd::argument::arguments& argumentOption, tag_io_cli ) {
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, argumentOption, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to grid where each column in formated with fixed width, witdth for columns are found in vector
inline std::string to_string( const dto::table& table, const std::vector<unsigned> vectorWidth, tag_io_cli ) {
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, {}, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to grid where each column in formated with fixed width, witdth for columns are found in vector
inline std::string to_string( const table& table, const std::vector<unsigned> vectorWidth, tag_io_cli ) {
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, {}, stringResult, tag_io_cli{} );
   return stringResult;
}


/// convert table to string in grid format formated with proper column withds
inline std::string to_string( const dto::table& table, tag_io_cli ) {
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregate_( &table );
   aggregate_.max( vectorWidth, tag_length{} );
   aggregate_.fix( vectorWidth, tag_text{} );
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, {}, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to string in grid format formated with proper column withds
inline std::string to_string( const dto::table& table, uint32_t uMax, tag_io_cli ) {
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregate_( &table );
   aggregate_.max( vectorWidth, tag_length{} );
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, { {"max_column_width", uMax } }, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to string in grid and pass format options to it
/// view implementation of core `to_string_s` method to see valid options
inline std::string to_string( const dto::table& table, const gd::argument::arguments& argumentsOption, tag_io_cli ) {
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregate_( &table );
   aggregate_.max( vectorWidth, tag_length{} );
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, argumentsOption, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to string in grid and pass format options to it
/// view implementation of core `to_string_s` method to see valid options
inline std::string to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const gd::argument::arguments& argumentsOption, tag_io_cli ) {
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregate_( &table );
   aggregate_.max( vectorWidth, uBegin, uCount, tag_length{} );
   std::string stringResult;
   to_string( table, uBegin, uCount, vectorWidth, argumentsOption, stringResult, tag_io_cli{} );
   return stringResult;
}

/// convert table to string in grid and pass format options to it
/// view implementation of core `to_string_s` method to see valid options
inline std::string to_string( const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, const gd::argument::arguments& argumentsOption, tag_io_cli ) {
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregate_( &table );
   aggregate_.max( vectorWidth, uBegin, uCount, vectorColumn, tag_length{} );
   std::string stringResult;
   to_string( table, uBegin, uCount, vectorWidth, vectorColumn, argumentsOption, stringResult, tag_io_cli{} );
   return stringResult;
}


/// convert table to string in grid format formated with proper column withds
inline std::string to_string( const table& table, tag_io_cli ) {
   std::vector<unsigned> vectorWidth;
   gd::table::aggregate aggregate_( &table );
   aggregate_.max( vectorWidth, tag_length{} );
   aggregate_.fix( vectorWidth, tag_text{} );
   // increase size to make space for quote characters for string values
   table.column_for_each( [&vectorWidth]( const auto& c_, auto uColumn  ) {  
      if( (c_.type() & gd::types::eTypeGroupString) != 0 ) vectorWidth[uColumn] += 2;
   });
   std::string stringResult;
   to_string( table, 0, table.get_row_count(), vectorWidth, {}, stringResult, tag_io_cli{} );
   return stringResult;
}

// ## SQL IO ------------------------------------------------------------------

/** \name write_insert_g
* Write table to result string using sql insert query
* 
* `write_insert_g` are able to write table information based on arguments sent and as sql insert into query
*///@{
void write_insert_g( const std::string_view& stringTableName, const dto::table& table, uint64_t uBegin, uint64_t uCount, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql );
inline void write_insert_g(const std::string_view& stringTableName, const dto::table& table, std::string& stringInsert, tag_io_sql) {
   write_insert_g( stringTableName, table, 0, table.get_row_count(), stringInsert, {}, tag_io_sql{});
}

inline void write_insert_g(const std::string_view& stringTableName, const dto::table& table, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql) {
   write_insert_g( stringTableName, table, 0, table.get_row_count(), stringInsert, argumentsOption, tag_io_sql{});
}

void write_insert_g( const std::string_view& stringTableName, const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql );

inline void write_insert_g(const std::string_view& stringTableName, const dto::table& table, uint64_t uBegin, uint64_t uCount, const std::vector<unsigned>& vectorColumn, std::string& stringInsert, tag_io_sql) {
   write_insert_g( stringTableName, table, uBegin, uCount, vectorColumn, stringInsert, {}, tag_io_sql{});
}
inline void write_insert_g(const std::string_view& stringTableName, const dto::table& table, const std::vector<unsigned>& vectorColumn, std::string& stringInsert, tag_io_sql) {
   write_insert_g( stringTableName, table, 0, table.get_row_count(), vectorColumn, stringInsert, tag_io_sql{});
}
inline void write_insert_g(const std::string_view& stringTableName, const dto::table& table, const std::vector<unsigned>& vectorColumn, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql) {
   write_insert_g( stringTableName, table, 0, table.get_row_count(), vectorColumn, stringInsert, argumentsOption, tag_io_sql{});
}

void write_insert_g( const std::string_view& stringTableName, const dto::table& table, const std::vector<uint64_t>& vectorRow, const std::vector<unsigned>& vectorColumn, std::string& stringInsert, const gd::argument::arguments& argumentsOption, tag_io_sql );
/// @}



_GD_TABLE_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
