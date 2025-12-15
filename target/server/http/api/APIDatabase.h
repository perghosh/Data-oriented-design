// @FILE [tag: api, database] [summary: API Database command class] [type: header]

#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_arguments.h"


/**
 * \brief
 *
 *
 *
 \code
 \endcode
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


protected:

public:


// ## attributes ----------------------------------------------------------------
public:
   std::vector<std::string_view> m_vectorCommand;   ///< command path segments 
   gd::argument::arguments m_argumentsParameter;    ///< parameters for api database command


// ## free functions ------------------------------------------------------------
public:



};