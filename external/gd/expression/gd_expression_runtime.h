/**
 * @file gd_expression_runtime.h
 * 
 * @brief 
 * 
 */



#pragma once

#include <cassert>
#include <functional>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd_expression.h"
#include "gd_expression_value.h"


#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 

/**
 * \brief
 *
 *
 */
struct method
{
// ## construction ------------------------------------------------------------
   method() : m_pmethod(nullptr), m_stringName(""), m_uArgumentCount(0) {}
   method(void(*methodPtr)(), const std::string& name, size_t paramCount)
      : m_pmethod(methodPtr), m_stringName(name), m_uArgumentCount(paramCount) {}

   // copy
   method(const method& o) { common_construct(o); }
   method(method&& o) noexcept { common_construct(std::move(o)); }

   // assign
   method& operator=(const method& o) { common_construct(o); return *this; }
   method& operator=(method&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~method() {}

   // common copy
   void common_construct(const method& o) {
      m_pmethod = o.m_pmethod;
      m_stringName = o.m_stringName;
      m_uArgumentCount = o.m_uArgumentCount;
   }

   void common_construct(method&& o) noexcept {
      m_pmethod = std::move(o.m_pmethod);
      m_stringName = std::move(o.m_stringName);
      m_uArgumentCount = o.m_uArgumentCount;
   }

// ## methods -----------------------------------------------------------------
/** \name DEBUG
*///@{
   std::string dump() const {
      return "Method Name: " + m_stringName + ", Parameter Count: " + std::to_string(m_uArgumentCount);
   }
//@}

// ## operators ---------------------------------------------------------------

   /// @brief Overload the < operator for sorting by name
   bool operator<(const method& o) const { return m_stringName < o.m_stringName; }
   /// @brief Overload the == operator for equality comparison.
   bool operator==(const method& o) const { return m_stringName == o.m_stringName; }

// ## attributes --------------------------------------------------------------
   void (*m_pmethod)(); ///< Pointer to the method.
   std::string m_stringName;        ///< Name of the method.
   size_t m_uArgumentCount;       ///< Number of parameters the method takes.

// ## free functions ----------------------------------------------------------
};

/**
 * \brief
 *
 *
 */
struct runtime
{
// ## construction ------------------------------------------------------------
   runtime() {}
   runtime( const std::function<bool (const std::string_view&, value::variant_t* )>& callback_ ): m_functionFind(callback_) {}
   runtime( const std::vector< std::pair<std::string, value::variant_t>>& vectorVariable ) : m_vectorVariable(vectorVariable) {}
   runtime( const std::function<bool (const std::string_view&, value::variant_t* )>& callback_, const std::vector< std::pair<std::string, value::variant_t>>& vectorVariable ): m_functionFind(callback_), m_vectorVariable(vectorVariable) {}
   // copy
   runtime(const runtime& o) { common_construct(o); }
   runtime(runtime&& o) noexcept { common_construct(std::move(o)); }
   // assign
   runtime& operator=(const runtime& o) { common_construct(o); return *this; }
   runtime& operator=(runtime&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~runtime() {}
   // common copy
   void common_construct(const runtime& o) { m_vectorVariable = o.m_vectorVariable; m_functionFind = o.m_functionFind; m_stringError = o.m_stringError; }
   void common_construct(runtime&& o) noexcept { m_vectorVariable = std::move( o.m_vectorVariable ); m_functionFind = std::move( o.m_functionFind ); m_stringError = std::move( o.m_stringError ); }

// ## methods -----------------------------------------------------------------
   void add(const std::string_view& stringName, const value::variant_t& value_) { m_vectorVariable.push_back(std::make_pair(std::string(stringName), value_)); }
   void add(const std::string& stringError, tag_error ) { m_stringError.push_back(stringError); }

   int find_variable(const std::string_view& stringName) const;
   const value::variant_t& get_variable(size_t uIndex) const;

   /// @brief try to find variable value by name and use callback function to find it
   bool find_value( const std::string_view& stringName, value::variant_t* pvariant_ ) ;

/** \name DEBUG
*///@{
   std::string dump() const;
//@}

// ## attributes --------------------------------------------------------------
   /// @brief vector of variables
   std::vector< std::pair<std::string, value::variant_t>> m_vectorVariable; ///< vector of values  
   /// @brief callback function used to find variable from external source
   std::function<bool (const std::string_view&, value::variant_t* )> m_functionFind; ///< function to find variable
   /// @brief error strings, colleting error messsages
   std::vector<std::string> m_stringError;

// ## free functions ----------------------------------------------------------

};

_GD_EXPRESSION_END