// @FILE [tag: api, database] [summary: API Database command class] [type: header] [name: APIDatabase.h]

#pragma once


#include "API_Base.h"



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
class CAPIDatabase : public CAPI_Base
{
// ## construction -------------------------------------------------------------
public:
    CAPIDatabase() {}
    CAPIDatabase( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
        : CAPI_Base( vectorCommand, argumentsParameter ) {}
    CAPIDatabase( std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter )
       : CAPI_Base( std::move( vectorCommand ), std::move( argumentsParameter ) ) { }
	 CAPIDatabase(CApplication* pApplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
        : CAPI_Base( pApplication, vectorCommand, argumentsParameter ) {}
    // copy - explicitly deleted to make class move-only (inherited from CAPI_Base)
    CAPIDatabase( const CAPIDatabase& ) = delete;
    CAPIDatabase& operator=( const CAPIDatabase& ) = delete;
    
    // move - only move operations allowed (inherited from CAPI_Base)
    CAPIDatabase( CAPIDatabase&& o ) noexcept : CAPI_Base( std::move( o ) ) {}
    CAPIDatabase& operator=( CAPIDatabase&& o ) noexcept { CAPI_Base::operator=( std::move( o ) ); return *this; }

    ~CAPIDatabase() override {}

// ## methods ------------------------------------------------------------------
public:
   // @API [tag: database] [description: Database operation methods]

   std::pair<bool, std::string> Execute() override;

   /// Create new database
   std::pair<bool, std::string> Execute_Create();

   /// Open existing database
   std::pair<bool, std::string> Execute_Open();

   /// Rund any database query
   std::pair<bool, std::string> Execute_Query();

   /// Select data from database
   std::pair<bool, std::string> Execute_Select();

   /// Insert data to database
   std::pair<bool, std::string> Execute_Insert();


protected:

public:


// ## attributes ----------------------------------------------------------------
public:


// ## free functions ------------------------------------------------------------
public:



};
