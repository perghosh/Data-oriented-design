/**
 * @file gd_expression_runtime.h
 * @TAG #gd::expression
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
#include <tuple>
#include <utility>
#include <vector>

#include "gd_expression.h"
#include "gd_expression_value.h"


#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN 

struct runtime; ///< forward declaration of runtime struct

/**
 * @brief Represents a callable method in the expression runtime.
 *
 * The method struct encapsulates metadata and function pointers for methods
 * that can be invoked by the expression runtime. It supports different
 * function pointer signatures for methods with varying input/output
 * requirements, and provides flags to describe method properties such as
 * whether the runtime context is required or if the method returns a value.
 *
 * Members include the method name, argument counts, flags, and a pointer to
 * the actual method implementation. Utility functions are provided for
 * querying method properties and for use in method lookup and sorting.
 */
struct method
{
   enum enumFlags : unsigned
   {
      eFlagUnknown = 0x00, ///< no flags
      eFlagRuntime = 0x01, ///< pass runtime as first argument
      eFlagVoid    = 0x02, ///< no return value
   };

   using method_0 = std::pair<bool, std::string>(*)(const std::vector<value>&);
   /// @brief Function pointer type for a method that processes input values and produces none or single output value. 
   using method_1 = std::pair<bool, std::string>(*)(const std::vector<value>&, value*);
   /// @brief Function pointer type for a method that processes input values and produces multiple output values.
   using method_2 = std::pair<bool, std::string>(*)(const std::vector<value>&, std::vector<value>* );
   /// @brief Function pointer type for a method that processes input values with runtime context but no return values
   using method_runtime_0 = std::pair<bool, std::string>(*)( runtime*, const std::vector<value>& );
   /// @brief Function pointer type for a method that processes input values with runtime context and produces multiple output values.
   using method_runtime_1 = std::pair<bool, std::string>(*)( runtime*, const std::vector<value>&, value* );
   /// @brief Function pointer type for a method that processes input values with runtime context and produces multiple output values.
   using method_runtime_2 = std::pair<bool, std::string>(*)( runtime*, const std::vector<value>&, std::vector<value>* );

   bool operator<(const std::string_view& stringName) const { return std::string_view(m_piName) < stringName; }
   bool operator==(const std::string_view& stringName) const { return std::string_view(m_piName) == stringName; }

   bool is_runtime() const { return ( m_uFlags & eFlagRuntime ) != 0; } ///< check if method has runtime as first argument
   bool is_void() const { return ( m_uFlags & eFlagVoid ) != 0; } ///< check if method has no return value

   const char* name() const { return m_piName; } ///< get name of the method
   unsigned in_count() const { return m_uInCount; } ///< get number of input arguments
   unsigned out_count() const { return m_uOutCount; } ///< get number of output arguments
   unsigned flags() const { return m_uFlags; } ///< get flags of the method

   void* m_pmethod;           ///< Pointer to the method
   //const char* m_piNamespace; ///< Namespace of the method
   const char* m_piName;      ///< Name of the method
   unsigned m_uInCount;       ///< Number of arguments
   unsigned m_uOutCount;      ///< Number of returned arguments
   unsigned m_uFlags = 0;     ///< Flags for the method
};


/**
 * @brief Manages the runtime environment for evaluating expressions.
 * 
 * The `runtime` struct provides functionality to manage variables, methods, 
 * and error handling during the evaluation of expressions. It supports adding 
 * variables and methods, finding variables and methods by name, and debugging 
 * the runtime state.
 */
struct runtime
{
   enum enumFlags : unsigned
   {
      eFlagUnknown = 0x00, ///< no flags
      eFlagDebug   = 0x01, ///< debug mode enabled
   };

   enum enumGlobalState : unsigned
   {
      eGlobalStateUnknown = 0x00, ///< unknown state
      eGlobalStateActive  = 0x01, ///< global object is active
   };

   /**
    * \brief Represents a global object in the runtime.
    *
    * Global is a named object that can be accessed globally within the runtime.
    * Used to somhow manage global objects that may be needed for methods added to the runtime.
    * Remember that expression runtime is not logic that are able to manage object,
    */
   struct global
   {
      // ## construction -------------------------------------------------------------

      global() {}
      global(const std::string_view& stringName, void* pObject) : m_uState{}, m_stringName(stringName), m_pObject(pObject) {}
      global(unsigned uState, const std::string_view& stringName, void* pObject) : m_uState(uState), m_stringName(stringName), m_pObject(pObject) {}
      global(const global& o) : m_uState(o.m_uState), m_stringName(o.m_stringName), m_pObject(o.m_pObject) {}
      global(global&& o) noexcept : m_uState(o.m_uState), m_stringName(std::move(o.m_stringName)), m_pObject(o.m_pObject) { o.m_pObject = nullptr; } // move constructor
      ~global() {}

      // Comparison operators for use with std::lower_bound and associative containers
      bool operator<(const global& other) const { return m_stringName < other.m_stringName; }
      bool operator<(const std::string_view& stringName) const { return m_stringName < stringName; }
      friend bool operator<(const std::string_view& stringName, const global& global_) { return stringName < global_.m_stringName; }

      const std::string& name() const { return m_stringName; } ///< get name of the global object
      bool is_active() const { return ( m_uState & eGlobalStateActive ) != 0; } ///< check if global object is active
      void set_active() { m_uState |= eGlobalStateActive; } ///< set global object as active
      void set_inactive() { m_uState &= ~eGlobalStateActive; } ///< set global object as inactive

      void* get_object() const { return m_pObject; } ///< get pointer to the global object
      void set_object(void* pObject) { m_pObject = pObject; } ///< set pointer to the global object

      // ## attributes
      unsigned m_uState; ///< 
      std::string m_stringName; ///< name of the global object
      void* m_pObject; ///< pointer to the global object
   };



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

   void set_debug(bool bDebug = true) { if( bDebug ) m_uFlags |= eFlagDebug; else m_uFlags &= ~eFlagDebug; } ///< set debug mode
   bool is_debug() const { return ( m_uFlags & eFlagDebug ) != 0; } ///< check if debug mode is enabled

// ## methods -----------------------------------------------------------------
   /**
    * @brief Adds a variable to the runtime.
    * 
    * @param stringName The name of the variable.
    * @param value_ The value of the variable.
    */
   void add(const std::string_view& stringName, const value::variant_t& value_) { m_vectorVariable.push_back(std::make_pair(std::string(stringName), value_)); }
   
   /**
    * @brief Adds a methods to the runtime.
    * Methods need to be sorted because runtime uses binary search to find them.
    * @param pair_ A pair containing the method count and pointer to first method
    */
   void add(const std::tuple<unsigned, const method*, std::string>& methods_) { m_vectorMethod.push_back(methods_); }

   /**
    * @brief Adds an error message to the runtime.
    *
    * @param stringError The error message.
    * @param tag_error A tag indicating the type of error (default is empty).
    */
   void add(const std::string& stringError, tag_error ) { m_stringError.push_back(stringError); }

   // ## methods logic

   const method* find_method(const std::string_view& stringName) const;
   const method* find_method(const std::string_view& stringName, tag_namespace) const;

   // ## variables logic

   int find_variable(const std::string_view& stringName) const;
   const value::variant_t& get_variable(size_t uIndex) const;
   void set_variable(size_t uIndex, const value::variant_t& value_) { m_vectorVariable[uIndex].second = value_; }
   void set_variable(const std::string_view& stringName, const value::variant_t& value_);

   void get_all_variables(std::vector<std::pair<std::string, value::variant_t>>& vectorVariable) const;
   void get_all_variables(std::vector<std::pair<std::string, value::variant_t>>&& vectorVariable) const;
   std::vector< std::pair<std::string, value::variant_t>> get_all_variables() const { return m_vectorVariable; }


   /// @brief try to find variable value by name and use callback function to find it
   bool find_value( const std::string_view& stringName, value::variant_t* pvariant_ ) ;

   /// add global object
   void add_global(const std::string_view& stringName, void* pObject) { m_vectorGlobal.push_back( { std::string(stringName), pObject } ); }
   /// set global object for name or if not found add it to the vector
   void set_global(const std::string_view& stringName, void* pObject);

   void get_global(const std::string_view& stringName, void** ppObject) const;
   void* get_global(const std::string_view& stringName) const;
   void* find_global(const std::string_view& stringName) const { return get_global(stringName); } ///< get global object by name, if not found then return nullptr

   template<typename TYPE>
   TYPE* get_global_as(const std::string_view& stringName) const;

/** \name DEBUG
*///@{
   std::string dump() const;
//@}

// ## attributes --------------------------------------------------------------
   unsigned m_uFlags = 0; ///< flags for the runtime, currently not used
   /// @brief vector of variables
   std::vector< std::pair<std::string, value::variant_t>> m_vectorVariable; ///< vector of values  
   /// @brief callback function used to find variable from external source
   std::function<bool (const std::string_view&, value::variant_t* )> m_functionFind; ///< function to find variable
   /// @brief vector of methods
   std::vector<std::tuple<unsigned,const method*,std::string>> m_vectorMethod; ///< vector of methods
   /// @brief vector of global objects, its just named void* pointers
   std::vector<global> m_vectorGlobal; ///< vector of global objects
   /// @brief error strings, colleting error messsages
   std::vector<std::string> m_stringError;

// ## free functions ----------------------------------------------------------

};

/// @brief set variable value by name, if not found then add it to the vector
inline void runtime::set_variable(const std::string_view& stringName, const value::variant_t& value_)
{
   int iIndex = find_variable(stringName);
   if( iIndex >= 0 ) { m_vectorVariable[iIndex].second = value_; }
   else { add(stringName, value_); }
}

/// @brief get all variables and copy them to the vector
inline void runtime::get_all_variables(std::vector<std::pair<std::string, value::variant_t>>& vectorVariable) const
{
   vectorVariable = m_vectorVariable;
}

/// @brief get all variables and move them to the vector
inline void runtime::get_all_variables(std::vector<std::pair<std::string, value::variant_t>>&& vectorVariable) const
{
   vectorVariable = std::move(m_vectorVariable);
}

/// @brief get global object by name and cast it to the type
template<typename TYPE>
TYPE* runtime::get_global_as(const std::string_view& stringName) const
{
   for (const auto& global_ : m_vectorGlobal)
   {
      if( global_.name() == stringName )  return static_cast<TYPE*>(global_.get_object());
   }
   return nullptr;
}


/// @brief get void * object by name, if not found then set it to nullptr
inline void runtime::get_global(const std::string_view& stringName, void** ppObject) const
{                                                                                                  assert( ppObject != nullptr );
   for( const auto& global_ : m_vectorGlobal )
   {
      if( global_.name() == stringName ) { *ppObject = global_.get_object(); return; }
   }
   *ppObject = nullptr;
}

/// @brief get global object by name, if not found then return nullptr
inline void* runtime::get_global(const std::string_view& stringName) const
{
   for(const auto& global_ : m_vectorGlobal)
   {
      if( global_.name() == stringName )  return global_.get_object();
   }
   return nullptr;
}

/// @brief set global object for name or if not found add it to the vector
inline void runtime::set_global(const std::string_view& stringName, void* pObject)
{
   for( auto& global_ : m_vectorGlobal )
   {
      if( global_.name() == stringName ) { global_.set_object( pObject ); return; }
   }
   add_global(stringName, pObject);
}




_GD_EXPRESSION_END