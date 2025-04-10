/**
 * @file gd_expression_value.h
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
 * @brief value manages the value used when evaluating the expression
 *
 */
struct value
{
   using variant_t = std::variant<int64_t, double, std::string, bool>; ///< value type
   using value_type = variant_t; ///< value type

// ## construction ------------------------------------------------------------
   value(): m_value( int64_t{0} ) {}
   explicit value( int64_t iValue ): m_value( iValue ) {}
   explicit value( double dValue ): m_value( dValue ) {}
   explicit value( const std::string stringValue ): m_value( stringValue ) {}
   explicit value( bool bValue ): m_value( bValue ) {}
   explicit value( const variant_t& value_ ): m_value( value_ ) {}
   explicit value(variant_t&& value_) : m_value(std::move(value_)) {}
   // copy
   value(const value& o) { common_construct(o); }
   value(value&& o) noexcept { common_construct(std::move(o)); }
   // assign
   value& operator=(const value& o) { common_construct(o); return *this; }
   value& operator=(value&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~value() {}
   // common copy
   void common_construct(const value& o) { m_value = o.m_value; }
   void common_construct(value&& o) noexcept { m_value = std::move(o.m_value); }

// ## operator ----------------------------------------------------------------
   value& operator=(int64_t v_) { m_value = v_; return *this; }
   value& operator=(double v_) { m_value = v_; return *this; }

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
   /// @brief get string value, converts other types to string if needed
   const std::string& get_string() const { assert( is_string() ); return std::get<std::string>(m_value); }
   /// @brief get boolean value, converts other types if possible
   bool get_bool() const;
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
   std::variant<int64_t, double, std::string, bool> m_value; ///< value

   // ## free functions ----------------------------------------------------------
};



_GD_EXPRESSION_END
