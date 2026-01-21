#pragma once
#include <cassert>
#include <string>
#include <string_view>

#include "gd_arguments.h"
#include "gd_com.h"
#include "gd_variant.h"

#ifndef _GD_DATABASE_BEGIN
#define _GD_DATABASE_BEGIN namespace gd { namespace database {
#define _GD_DATABASE_END } }
_GD_DATABASE_BEGIN
#else
_GD_DATABASE_BEGIN
#endif

/// Forward declare record, record is used to store data from one single row
class record;

/**
* @brief Enum class representing SQL transaction types.
*/
enum enumTransaction
{
   eTransactionBegin,         ///< Begin a new transaction.
   eTransactionCommit,        ///< Commit the current transaction.
   eTransactionRollback,      ///< Rollback the current transaction.
   eTransactionMerge,         ///< Merge uncommitted changes into the database.
};


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */

struct cursor_i : public com::unknown_i
{

/** \name OVERLOAD
*///@{
   virtual unsigned get_column_count() = 0;
   virtual bool is_valid_row() = 0;
   virtual std::pair<bool, std::string> prepare(const std::string_view& stringSql ) = 0;
   virtual std::pair<bool, std::string> prepare(const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue ) = 0;
   virtual std::pair<bool, std::string> bind( const std::vector< gd::variant_view >& vectorValue ) = 0;
   virtual std::pair<bool, std::string> bind( unsigned uIndex, const std::vector< gd::variant_view >& vectorValue ) = 0;
   virtual std::pair<bool, std::string> open() = 0;
   virtual std::pair<bool, std::string> open( const std::string_view& stringStatement ) = 0;
   virtual std::pair<bool, std::string> next() = 0;
   virtual std::pair<bool, std::string> execute() = 0;
   virtual bool is_open() = 0;
   virtual std::pair<bool, std::string> get_record( record** ppRecord ) = 0;
   virtual record* get_record() = 0;
   virtual const record* get_record() const = 0;
   virtual void close() = 0;
//@}

public:
/** \name DEBUG
*///@{

//@}


};

// {98E01E5F-08E7-47D3-B048-DC9F70B97B66}
constexpr COMPONENT_GUID COMPONENT_CURSOR = { 0x98e01e5f, 0x8e7, 0x47d3, { 0xb0, 0x48, 0xdc, 0x9f, 0x70, 0xb9, 0x7b, 0x66 } };



/**
 * \brief Interface for database connection
 *
 * Methods used to connect, execute and sql queries to connected database
 *
 \code
 \endcode
 */
struct database_i : public com::unknown_i
{

/** \name OVERLOAD
*///@{
   virtual std::string_view name() const = 0;
   virtual std::string_view dialect() const = 0;
   virtual void set( const std::string_view& stringName, const gd::variant_view& value_ ) = 0;

   /// open connection to database, the format for connect database is different depending on what database or technology that is used
   virtual std::pair<bool, std::string> open( const std::string_view& stringDriverConnect ) = 0;
   /// open connection to database, arguments to connect database is different depending on what database or technology that is used
   virtual std::pair<bool, std::string> open( const gd::argument::arguments& argumentsConnect ) = 0;
   /// execute sql statement
   virtual std::pair<bool, std::string> execute( const std::string_view& stringStatement ) = 0;
   /// execute sql statement and return specified generated values
   virtual std::pair<bool, std::string> execute( const std::string_view& stringStatement, std::function<bool( const gd::argument::arguments* )> callback_ ) = 0;
   /// execute sql statement that returns one single value, handy i many situations to get database information without having to use cursor
   virtual std::pair<bool, std::string> ask( const std::string_view& stringStatement, gd::variant* pvariantValue ) = 0;
   /// create cursor for database, remember to close/release cursor when done
   virtual std::pair<bool, std::string> get_cursor( cursor_i** ppCursor ) = 0;
   /// execute operattion related to transaction logic
   virtual std::pair<bool, std::string> transaction( const gd::variant_view& transaction_ ) = 0;
   /// close connection to database
   virtual void close() = 0;
   virtual void erase() = 0;
   /// return raw pointer to database connection (this depends on what database or technology that is used)
   virtual void* get_pointer() = 0;
   /// return last change count
   virtual gd::variant get_change_count() = 0;
   /// return last insert key
   virtual gd::variant get_insert_key() = 0;
   
//@}

public:
/** \name DEBUG
*///@{

//@}


};

// {902B5974-EEBC-4EA2-90E7-5C43A2BABFA8}
constexpr COMPONENT_GUID COMPONENT_DATABASE = { 0x902b5974, 0xeebc, 0x4ea2, { 0x90, 0xe7, 0x5c, 0x43, 0xa2, 0xba, 0xbf, 0xa8 } };


_GD_DATABASE_END