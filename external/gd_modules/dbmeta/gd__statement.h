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
   eFormatRaw  = 1, // SQL statements
   eFormatCsv  = 2, // Comma-Separated statement information
   eFormatJson = 3, // 
   eFormatXml  = 4,
};

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
   std::pair<bool, std::string> add_statement( std::string_view stringName, std::string_view stringStatement, enumFormat eFormat = eFormatRaw, uint32_t uType = 0, uint32_t uRule = 0 ); ///< add statement to statement object

   
   size_t size() const noexcept { return m_ptableStatement ? m_ptableStatement->size() : 0; } ///< get number of statements in statement object


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

   static constexpr enumFormat get_format_s( std::string_view stringName ) noexcept;



};

/// Return enumFormat id for given name (constexpr) ---------------------------- get_format_s
constexpr statement::enumFormat statement::get_format_s( std::string_view stringName ) noexcept
{
   // Accept a few common textual representations
   return (stringName == "raw"  || stringName == "sql"  || stringName == "eFormatRaw")  ? eFormatRaw
        : (stringName == "csv"  || stringName == "eFormatCsv")                       ? eFormatCsv
        : (stringName == "json" || stringName == "eFormatJson")                      ? eFormatJson
        : (stringName == "xml"  || stringName == "eFormatXml")                       ? eFormatXml
        : eFormatRaw; ///< default
}



_GD_MODULES_DBMETA_END
