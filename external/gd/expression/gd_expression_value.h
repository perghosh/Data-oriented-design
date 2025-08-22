/**
 * @file gd_expression_value.h
 * @TAG #gd::expression* 
 * 
 * @brief 
 * 
 */

#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "gd_expression.h"

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
struct any_pointer
{
// ## construction ------------------------------------------------------------
   any_pointer() : m_piName(nullptr), m_pValue(nullptr) {}
   explicit any_pointer(void* pValue) : m_piName(nullptr), m_pValue(pValue) {}
   explicit any_pointer(const char* piName, void* pValue) : m_piName(piName), m_pValue(pValue) {}
   any_pointer(std::pair<const char*, void*> pair_) : m_piName(pair_.first), m_pValue(pair_.second) { assert(m_pValue != nullptr); } ///< construct from pair of name and pointer
   // copy
   any_pointer(const any_pointer& o) { common_construct(o); }
   any_pointer(any_pointer&& o) noexcept { common_construct(std::move(o)); }
   // assign
   any_pointer& operator=(const any_pointer& o) { common_construct(o); return *this; }
   any_pointer& operator=(any_pointer&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~any_pointer() {}
   // common copy
   void common_construct(const any_pointer& o) {
      m_piName = o.m_piName;
      m_pValue = o.m_pValue;
   }
   void common_construct(any_pointer&& o) noexcept {
      m_piName = o.m_piName;
      m_pValue = o.m_pValue;
   }

   // ## operator ----------------------------------------------------------------
   operator void* ( ) const { return m_pValue; } ///< convert to void pointer
   operator bool() const { return m_pValue != nullptr; } ///< convert to bool, true if pointer is not null
   operator std::pair<const char*, void*>() const { return std::make_pair(m_piName, m_pValue); } ///< convert to pair of name and pointer
   /// @brief compare two pointers, returns true if equal
   bool operator==(const any_pointer& o) const 
   {
      if( m_pValue == nullptr && o.m_pValue == nullptr ) return true; // both are null
      if( m_pValue == nullptr || o.m_pValue == nullptr ) return false; // one is null, other is not
      return m_pValue == o.m_pValue; // compare pointers
   }

   bool operator!=(const any_pointer& o) const { return !(*this == o); } ///< compare pointers     
   
// ## methods -----------------------------------------------------------------

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   const char* m_piName; ///< name of the pointer, used for debugging and type identification
   void* m_pValue; ///< pointer to the value, can be any type of pointer

// ## free functions ----------------------------------------------------------

};


/**
 * @brief value manages the value used when evaluating the expression
 *
 */
struct value
{
   using variant_t = std::variant<int64_t, double, std::string, bool, std::pair<const char*, void*>, std::nullptr_t>; ///< value type
   using value_type = variant_t; ///< value type

// ## construction ------------------------------------------------------------
   value(): m_value( nullptr ) {}
   explicit value( int64_t iValue ): m_value( iValue ) {}
   explicit value( double dValue ): m_value( dValue ) {}
   explicit value( const std::string stringValue ): m_value( stringValue ) {}
   explicit value( bool bValue ): m_value( bValue ) {}
   explicit value( const std::string_view& stringValue) : m_value(std::string(stringValue)) {}
   explicit value( const std::pair<const char*, void*>& pair_) : m_value(pair_) {}
   explicit value( const variant_t& value_ ): m_value( value_ ) {}
   explicit value(variant_t&& value_) : m_value(std::move(value_)) {}
   // copy
   value(const value& o) { common_construct(o); }
   value(value&& o) noexcept { common_construct(std::move(o)); }
   // assign
   value& operator=(bool bValue) { m_value = bValue; return *this; }
   value& operator=(int64_t iValue) { m_value = iValue; return *this; }
   value& operator=(double dValue) { m_value = dValue; return *this; }
   value& operator=(const std::string& stringValue) { m_value = stringValue; return *this; }
   value& operator=(const value& o) { common_construct(o); return *this; }
   value& operator=(value&& o) noexcept { common_construct(std::move(o)); return *this; }
   value& operator=(const variant_t& value_) noexcept { m_value = value_; return *this; }
   value& operator=(variant_t&& value_) noexcept { m_value = std::move(value_); return *this; }

   ~value() {}
   // common copy
   void common_construct(const value& o) { m_value = o.m_value; }
   void common_construct(value&& o) noexcept { m_value = std::move(o.m_value); }

// ## operator ----------------------------------------------------------------
   operator variant_t&() { return m_value; } ///< convert to variant_t
   operator int64_t() const { return as_integer(); } ///< convert to int64_t
   /// @brief compare two values, returns true if equal
   bool operator==(const value& o) const 
   {
      if( m_value.index() != o.m_value.index() ) return false;
      return m_value == o.m_value;
   }
   /// @brief compare two values, returns true if not equal
   bool operator!=(const value& o) const { return !(*this == o); }


// ## methods -----------------------------------------------------------------

   size_t index() const { return m_value.index(); } ///< get index of current value type

/** \name TYPE CHECKING
*///@{
   /// @brief check if value holds an integer
   bool is_integer() const { return std::holds_alternative<int64_t>(m_value); }
   /// @brief check if value holds a double
   bool is_double() const { return std::holds_alternative<double>(m_value); }
   /// @brief check if value holds a string
   bool is_string() const { return std::holds_alternative<std::string>(m_value); }
   /// @brief check if value holds a boolean
   bool is_bool() const { return std::holds_alternative<bool>(m_value); }
   /// @brief check if value holds a pointer
   bool is_pointer() const { return std::holds_alternative<std::pair<const char*, void*>>(m_value); }
   /// @brief check if value holds a null pointer
   bool is_null() const { return std::holds_alternative<std::nullptr_t>(m_value); }
//@}

/** \name GETTERS
*///@{
   /// @brief get integer value, returns 0 if not integer
   int64_t get_integer() const { return is_integer() ? std::get<int64_t>(m_value) : 0; }
   /// @brief get double value, converts integer if needed, returns 0.0 if not numeric
   double get_double() const 
   {
      if( is_double() ) return std::get<double>(m_value);
      if( is_integer() ) return static_cast<double>(std::get<int64_t>(m_value));
      return 0.0;
   }
   /// @brief get string value, converts integer and double to string if needed
   const std::string& get_string() const { assert( is_string() ); return std::get<std::string>(m_value); }
   /// @brief get boolean value, converts integer and double to boolean if needed 
   bool get_bool() const;
   /// @brief get pointer value
   void* get_pointer() const { assert( is_pointer() ); return std::get<std::pair<const char*, void*>>(m_value).second; }  
//@}

/** \name SETTERS
*///@{
   /// @brief set integer value
   void set( int64_t iValue ) { m_value = iValue; }
   /// @brief set double value
   void set( double dValue ) { m_value = dValue; }
   /// @brief set string value
   void set( const std::string& stringValue ) { m_value = stringValue; }
   void set( const std::string_view& stringValue ) { m_value = std::string( stringValue ); }
   /// @brief set boolean value
   void set( bool bValue ) { m_value = bValue; }
//@}

/** \name AS methods, converts value to type if not same
*///@{
   int64_t as_integer() const;
   double as_double() const;
   std::string as_string() const;
   std::string_view as_string_view() const;
   bool as_bool() const;
   gd::expression::variant_t as_variant() const;
//@}


/** \name CONVERSION
*///@{
   /// @brief attempt to convert current value to integer
   bool to_integer();
   /// @brief attempt to convert current value to double
   bool to_double();
   /// @brief attempt to convert current value to string
   bool to_string();
   /// @brief attempt to convert current value to boolean
   bool to_bool();
   /// @brief attempt to convert current value to same type as this
   bool synchronize( value& value_, void* );
//@}

   // ## attributes --------------------------------------------------------------
   std::variant<int64_t, double, std::string, bool, std::pair<const char*, void*>, std::nullptr_t > m_value; ///< value

   // ## free functions ----------------------------------------------------------
};



_GD_EXPRESSION_END

