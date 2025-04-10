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

struct method
{
   using method_1 = std::pair<bool, std::string>(*)(const std::vector<variant_t>&, value*);
   using method_2 = std::pair<bool, std::string>(*)(const std::vector<variant_t>&, std::vector<variant_t>& );

   bool operator<(const std::string_view& stringName) const { return std::string_view(m_piName) < stringName; }

   const char* name() const { return m_piName; } ///< get name of the method
   unsigned in_count() const { return m_uInCount; } ///< get number of input arguments
   unsigned out_count() const { return m_uOutCount; } ///< get number of output arguments

   void* m_pmethod;        ///< Pointer to the method
   const char* m_piName;   ///< Name of the method
   unsigned m_uInCount;    ///< Number of arguments
   unsigned m_uOutCount;   ///< Number of returned arguments
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
   void add(const std::pair<unsigned, const method*>& pair_) { m_vectorMethod.push_back(pair_); }
   void add(const std::string& stringError, tag_error ) { m_stringError.push_back(stringError); }

   const method* find_method(const std::string_view& stringName) const;

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
   /// @brief vector of methods
   std::vector<std::pair<unsigned,const method*>> m_vectorMethod; ///< vector of methods
   /// @brief error strings, colleting error messsages
   std::vector<std::string> m_stringError;

// ## free functions ----------------------------------------------------------

};

_GD_EXPRESSION_END