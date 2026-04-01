// @FILE [tag: database, sql, meta] [description: Store information about statements and rules how to use these] [type: header] [name: gd__statement.h]

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_types.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_uuid.h"
#include "gd/gd_log_logger.h"
#include "gd/gd_table_arguments.h"

#ifndef _GD_MODULES_DBMETA_BEGIN

#  define _GD_MODULES_DBMETA_BEGIN namespace gd { namespace modules { namespace dbmeta {
#  define _GD_MODULES_DBMETA_END } } }

#endif

_GD_MODULES_DBMETA_BEGIN


/** @CLASS [name: statement] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class statement
{
public:
// @API [tag: constants]
 
/**
 * @brief Statement format need to know how to parse out statment parts 
 * 
 * Format describes how parts in sql statement are presented. There are different 
 * formats that is supported to be able to store and parse statements as well as how 
 * they will be executed. Below are some samples on how to store statements and 
 * be able to parse them.
 * 
 * Simplest form may be like a comman separated list of field names.
 * T:TCustomer,CustomerK,FName,FPhone or TCustomer.CustomerK,TCustomer.FName,TCustomer.FPhone
 * 
 * Sample with multiple tables as customers and persons on the company
 * T:TCustomer,CustomerK,FName,FPhone,T:TContact,FName,FLastName or
 *  TCustonmer.CustemrK,TCustomer.FName,TCustomer.FPhone,TContact.FName,TContact.FLastName
 * 
 * // This contains three tables
 * "T:TCustomer,CustomerK,T:TContact,ContactK,FName,T:TAddress,Street,City"
 * 
 * // sample with json data
 * {
 *   {
 *     "fields": [
 *       {"table": "TCustomer", "field": "CustomerK", "alias": "id"},
 *       {"table": "TCustomer", "field": "FName", "alias": "name"},
 *       {"table": "TCustomer", "field": "FPhone"},
 *       {"table": "TContact", "field": "FName"},
 *       {"table": "TContact", "field": "FLastName"},
 *       {"table": "TAddress", "field": "Street"},
 *       {"table": "TAddress", "field": "City"},
 *       {
 *          "table": "TAddress",
 *          "expression": "CONCAT(Street, ', ', City)",
 *          "alias": "fullAddress"
 *       }
 *     ],
 *    "where": [
 *     { "expression":"(TCustomer.CustomerK = {customer_variable})"}, // TCustoner table is required
 *    "group": "[TCustomer.CustomerK,City]",
 *     ...

 *   }
 * }
 */
enum enumFormat {
   eFormatUnknown = 0,
   eFormatRaw     = 1, // SQL statements
   eFormatJinja   = 2, // Jinja template with SQL statement and variables
   eFormatCsv     = 3, // Comma-Separated statement information
   eFormatJson    = 4, // 
   eFormatXml     = 5,
   eFormatMAX     = 6, // max value for format, this is used to validate format values
};

/// Statement type, this is used to know how to execute statement and also for some form of meta data
enum enumType { eTypeUnknown = 0, eTypeSelect = 1, eTypeInsert = 2, eTypeUpdate = 3, eTypeDelete = 4, eTypeAsk = 5, eTypeBatch = 6, eTypeMAX = 7 };

/// Column indexes for fixed columns in statement table
enum enumColumn { eColumnKey, eColumnUuid, eColumnName, eColumnType, eColumnFormat, eColumnRule, eColumnStatement, eColumnDescription };

// @API [tag: construction]
public:
   statement() {}
   // copy
   statement( const statement& o ) { common_construct( o ); }
   statement( statement&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   statement& operator=( const statement& o ) { common_construct( o ); return *this; }
   statement& operator=( statement&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~statement() {}
private:
   // common copy
   void common_construct( const statement& o ) {}
   void common_construct( statement&& o ) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]
   void initialize(); ///< initialize statement object, this will prepare internal table for storing statements

   std::pair<bool, std::string> add( std::string_view stringName, std::string_view stringStatement, enumFormat eFormat = eFormatRaw, uint32_t uType = 0, uint32_t uRule = 0 ); ///< add statement to statement object
   std::pair<bool, std::string> add( std::string_view stringName, std::string_view stringStatement, std::string_view stingFormat, std::string_view stringType, std::string_view stringRule = ""); ///< add statement to statement object
   std::pair<bool, std::string> add( const gd::argument::arguments& argumentsStatement ); ///< add statement to statement object using arguments object, this is used when adding statement from query template

   gd::types::uuid get_id( uint64_t uRow ) const; ///< get statement uuid by row index
   std::string_view get_name( uint64_t uRow ) const; ///< get statement name by row index
   std::string_view get_statement( uint64_t uRow ) const; ///< get statement by row index


   int64_t find( const gd::types::uuid* puuid ) const; ///< find statement by uuid, returns row index or -1 if not found
   int64_t find( gd::types::uuid& uuid ) const { return find( &uuid ); } ///< find statement by uuid, returns row index or -1 if not found
   int64_t find( std::string_view stringName ) const; ///< find statement by name, returns row index or -1 if not found
   
   size_t size() const noexcept { return m_ptableStatement ? m_ptableStatement->size() : 0; } ///< get number of statements in statement object
   size_t count( const gd::types::uuid& uuidKey ) const; ///< count number of statements with specified key
   bool empty() const noexcept { return size() == 0; } ///< check if there are any active statements

   void erase( uint64_t uRow ); ///< erase statement by row index, this will mark row as deleted


protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   std::unique_ptr<gd::table::arguments::table> m_ptableStatement;  ///< table holding list of statements


// @API [tag: free-functions]
public:
   static void create_statement_s( gd::table::arguments::table& tableStatement );    ///< create statement structure

   static constexpr enumType to_type_s( std::string_view stringType ) noexcept;
   static constexpr enumFormat to_format_s( std::string_view stringName ) noexcept;
   static constexpr enumFormat to_format_s( std::string_view stringName, gd::types::tag_validate ) noexcept;

};

/// Return statement type for given name (constexpr) ------------------------- to_type_s
constexpr statement::enumType statement::to_type_s( std::string_view stringType ) noexcept
{
   // Accept a few common textual representations
   return (stringType == "select" ) ? eTypeSelect
        : (stringType == "insert" ) ? eTypeInsert
        : (stringType == "update" ) ? eTypeUpdate
        : (stringType == "delete" ) ? eTypeDelete
        : (stringType == "ask"    ) ? eTypeAsk
        : (stringType == "batch"  )  ? eTypeBatch
        : eTypeUnknown; ///< default
}

/// Return enumFormat id for given name (constexpr) -------------------------- to_format_s
constexpr statement::enumFormat statement::to_format_s( std::string_view stringName ) noexcept
{
   // Accept a few common textual representations
   return (stringName == "raw"  || stringName == "sql" ) ? eFormatRaw
        : (stringName == "jinja" )                       ? eFormatJinja
        : (stringName == "csv" )                         ? eFormatCsv
        : (stringName == "json" )                        ? eFormatJson
        : (stringName == "xml" )                         ? eFormatXml
        : eFormatRaw; ///< default
}

/// Return enumFormat id for given name (constexpr) -------------------------- to_format_s
constexpr statement::enumFormat statement::to_format_s( std::string_view stringName, gd::types::tag_validate ) noexcept
{
   // Accept a few common textual representations
   return (stringName == "raw"  || stringName == "sql" ) ? eFormatRaw
        : (stringName == "jinja" )                       ? eFormatJinja
        : (stringName == "csv" )                         ? eFormatCsv
        : (stringName == "json" )                        ? eFormatJson
        : (stringName == "xml" )                         ? eFormatXml
        : eFormatUnknown; ///< error, unknown format
}




_GD_MODULES_DBMETA_END
