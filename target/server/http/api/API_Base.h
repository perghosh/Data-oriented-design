// @FILE [tag: api, base] [summary: Base class for API commands] [type: header] [name: API_Base.h]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../Types.h"

#include "gd/gd_arguments.h"
#include "gd/gd_variant_view.h"

class CApplication;
class CDocument;


/**
 * @brief Base class for API command classes.
 *
 * This class provides common functionality for API command classes,
 * including command processing, argument handling, and error management.
 *
 * The class serves as a foundation for specific API command implementations
 * like CAPIDatabase, providing shared member variables and methods.
 */
class CAPI_Base
{
// ## construction -----------------------------------------------------------
public:
    CAPI_Base() {}
    CAPI_Base( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter )
        : m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ) {}
    CAPI_Base( const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex )
        : m_vectorCommand( vectorCommand ), m_uCommandIndex( uCommandIndex ), m_argumentsParameter( argumentsParameter ) { assert( uCommandIndex < vectorCommand.size() ); }
    CAPI_Base( std::vector<std::string_view>&& vectorCommand, gd::argument::arguments&& argumentsParameter )
       : m_vectorCommand( std::move( vectorCommand ) ), m_argumentsParameter( std::move( argumentsParameter ) ) { }
    CAPI_Base(CApplication* papplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter)
        : m_vectorCommand( vectorCommand ), m_argumentsParameter( argumentsParameter ), m_papplication( papplication ) {}
    CAPI_Base(CApplication* papplication, const std::vector<std::string_view>& vectorCommand, const gd::argument::arguments& argumentsParameter, unsigned uCommandIndex)
        : m_vectorCommand( vectorCommand ), m_uCommandIndex( uCommandIndex ), m_argumentsParameter( argumentsParameter ), m_papplication( papplication ) { assert( uCommandIndex < vectorCommand.size() ); }

    // copy - explicitly deleted to make class move-only
    CAPI_Base( const CAPI_Base& ) = delete;
    CAPI_Base& operator=( const CAPI_Base& ) = delete;
    
    // move - only move operations allowed
    CAPI_Base( CAPI_Base&& o ) noexcept { common_construct( std::move( o ) ); }
    CAPI_Base& operator=( CAPI_Base&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

    virtual ~CAPI_Base() {}
private:
    // common move implementation
    void common_construct( CAPI_Base&& o ) noexcept;

// ## operator ---------------------------------------------------------------
public:
   /// get argument by name, this method returns first value for the name from read uri parameters
   gd::variant_view operator[](const char* piName) { return m_argumentsParameter.get_argument(std::string_view(piName)).as_variant_view(); }
   gd::variant_view operator[]( std::tuple<const char*, size_t> index_ ) { 
      return m_argumentsParameter.find_argument( std::string_view( std::get<0>( index_ ) ), (unsigned)std::get<1>( index_ ) ).as_variant_view(); 
   }

// ## methods ----------------------------------------------------------------
public:
   // @API [tag: get] [description: Get methods]

   const CDocument* GetDocument() const { return m_pdocument; }
   CDocument* GetDocument();

   /// Get current command being processed, this is the command at m_uCommandIndex in m_vectorCommand
   std::string_view GetCommand() const { assert( m_uCommandIndex < m_vectorCommand.size() ); return m_stringCommand; }
   /// Set current command being processed, this is the command at m_uCommandIndex in m_vectorCommand
   void SetCommand( std::string_view stringCommand ) { m_stringCommand = stringCommand; }
   /// Get currect command index value
   unsigned GetCommandIndex() const { assert( m_uCommandIndex <= m_vectorCommand.size() ); return m_uCommandIndex; }
   /// Sets the current command index
   void SetCommandIndex( unsigned uIndex ) { m_uCommandIndex = uIndex; assert( m_uCommandIndex <= m_vectorCommand.size() ); }
   /// Checks to se if there is more commands left to process
   bool IsLastCommand() const { return m_uCommandIndex >= (unsigned)m_vectorCommand.size(); }

   /// Helper to simplify getting arguments from arguments object
   gd::variant_view Get( std::string_view stringName ) const { return m_argumentsParameter.get_argument(stringName); }
   gd::variant_view GetArgument( std::string_view stringName ) const { return m_argumentsParameter.get_argument(stringName); }

   std::string GetLastError() const { return m_stringLastError; }

   /// Count the keys used based on current command index
   size_t GetArgumentIndex( const std::string_view& stringName ) const;
   [[deprecated]]
   size_t GetArgumentIndex( const std::string_view& stringFirst, const std::string_view& stringSecond ) const;

   /// For some arguments that may be used for different operations and if multiple operations are used 
   /// this is used to know what argument to use in a serie.
   void IncrementArgumentCounter( std::string_view stringName );
   
   /// Check argument name exists
   bool Exists(const std::string_view& stringName) const;

   /// Get pointer objects result container
   Types::Objects* GetObjects() { return &m_objects; }

   /// Execute the command
   virtual std::pair<bool, std::string> Execute() = 0;

// ## attributes -------------------------------------------------------------
public:
   std::string_view m_stringCommand;                  ///< Current command being processed, this is the command at m_uCommandIndex in m_vectorCommand
   std::vector<std::string_view> m_vectorCommand;     ///< command path segments
   unsigned m_uCommandIndex{};                        ///< current command index being processed, command index are the index within m_vectorCommand
   gd::argument::arguments m_argumentsParameter;      ///< parameters for api command
   gd::argument::arguments m_argumentsGlobal;         ///< store global variablies, prefixed with "g_"
   Types::Objects m_objects;                          ///< objects used to store result objects
   std::string m_stringLastError;                     ///< last error message 
   CApplication* m_papplication{};                    ///< application pointer, access application that is used as object root for server
   CDocument* m_pdocument{};                          ///< document pointer, access document for active user
   std::array<std::byte, 64> m_arrayBufferCounter;    ///< Internal buffer used for argument counting to avoid dynamic allocations
   gd::argument::arguments m_argumentsArgumentCount{ m_arrayBufferCounter };

// ## free functions ---------------------------------------------------------
public:

};
