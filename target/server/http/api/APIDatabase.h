// @FILE [tag: api, database] [summary: API Database command class] [type: header] [name: APIDatabase.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../Types.h"

#include "gd/gd_arguments.h"

class CApplication;
class CDocument;


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
   /// get argument by name, this method returns first value for the name from read uri parameters
   gd::variant_view operator[](const char* piName) { return m_argumentsParameter.get_argument(std::string_view(piName)).as_variant_view(); }
   gd::variant_view operator[]( std::tuple<const char*, size_t> index_) { 
      return m_argumentsParameter.find_argument( std::string_view( std::get<0>( index_ ) ), (unsigned)std::get<1>( index_ ) ).as_variant_view(); 
   }



// ## methods ------------------------------------------------------------------
public:
   // @API [tag: get] [description: Get methods]

   CDocument* GetDocument();

   std::string GetLastError() const { return m_stringLastError; }

   /// Count the keys used based on current command index
   size_t GetArgumentIndex( const std::string_view& stringName ) const;

   /// Get pointer objects result container
   Types::Objects* GetObjects() { return &m_objects; }


   std::pair<bool, std::string> Execute();

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
   std::vector<std::string_view> m_vectorCommand;///< command path segments
   unsigned m_uCommandIndex{};                   ///< current command index being processed
   gd::argument::arguments m_argumentsParameter; ///< parameters for api database command
   Types::Objects m_objects;                     ///< objects used to store result objects
   std::string m_stringLastError;                ///< last error message 
	CApplication* m_pApplication{};               ///< application pointer, access application that is used as object root for server


// ## free functions ------------------------------------------------------------
public:



};
