// @FILE [tag: api, database] [summary: API Database command class] [type: header]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_arguments.h"

class CApplication;


/**
 * @brief API Database command class for executing database operations.
 *
 * This class provides a command interface for performing database operations
 * through an API. It supports creating and connecting to databases, with
 * SQLite as the currently supported database type.
 *
 * The class processes command vectors and parameters to execute specific
 * database operations.
 *
 * Example usage:
 * @code
 * // Create a new SQLite database
 * CAPIDatabase dbCmd({"db", "create"}, {{"type", "sqlite"}, {"name", "mydatabase"}});
 * auto result = dbCmd.Execute();
 *
 * // Connect to an existing database
 * CAPIDatabase connectCmd({"db", "connect"}, {{"name", "existingdb"}});
 * auto connectResult = connectCmd.Execute();
 * @endcode
 */
class CAPIDatabase
{
    // ## construction -------------------------------------------------------------
public:
    CAPIDatabase() {}
    CAPIDatabase( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
        : m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ) {}
    CAPIDatabase( std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter )
       : m_vectorCommand( std::move( vectorCommand ) ), m_argumentsParameter( std::move( argumentsParameter ) ) { }
	 CAPIDatabase(CApplication* pApplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
        : m_pApplication( pApplication ), m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ) {}
    // copy
    CAPIDatabase( const CAPIDatabase& o ) { common_construct( o ); }
    CAPIDatabase( CAPIDatabase&& o ) noexcept { common_construct( std::move( o ) ); }
    // assign
    CAPIDatabase& operator=( const CAPIDatabase& o ) { common_construct( o ); return *this; }
    CAPIDatabase& operator=( CAPIDatabase&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

    ~CAPIDatabase() {}
private:
    // common copy
    void common_construct( const CAPIDatabase& o );
    void common_construct( CAPIDatabase&& o ) noexcept;

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
    /** \name GET/SET
    *///@{

    //@}

   std::pair<bool, std::string> Execute();

   std::pair<bool, std::string> Execute_Create();

   std::pair<bool, std::string> Execute_Open();

   std::pair<bool, std::string> Execute_Query();

   std::pair<bool, std::string> Execute_Select();



protected:

public:


// ## attributes ----------------------------------------------------------------
public:
	CApplication* m_pApplication{};               ///< application pointer, access application that is used as object root for server
   std::vector<std::string_view> m_vectorCommand;   ///< command path segments
   gd::argument::arguments m_argumentsParameter;    ///< parameters for api database command


// ## free functions ------------------------------------------------------------
public:



};
