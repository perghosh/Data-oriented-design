// @FILE [tag: variant, arg] [description: Argument objects for variant key-value pairs] [type: header]

#pragma once
#include <string>
#include <string_view>
#include <vector>

#include "gd_variant.h"
#include "gd_variant_view.h"

#ifndef _GD_BEGIN_VARIANT_ARG
#  define GD_BEGIN_VARIANT_ARG
#endif

// ## Arg objects for storing key-value pairs
// This file defines two argument objects that can hold key-value pairs.
// These objects are commonly used throughout the codebase to pass arguments 
// to functions and methods in a standardized way.

#if defined( __clang__ )
   #pragma GCC diagnostic push
   #pragma clang diagnostic ignored "-Wunused-value"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 )
#endif

#ifndef _GD_BEGIN
namespace gd {
#else
_GD_BEGIN
#endif

/**
 * @brief Argument object with string_view key and variant_view value
 *
 * This lightweight object is used when the key and value are both views
 * into existing data, without ownership of the data.
 *
 * This is particularly useful for:
 * - Function parameters where data is read-only
 * - Temporary objects for lookups
 * - Avoiding unnecessary copies
 */
struct arg_view
{
   // ## construction ------------------------------------------------------------
   arg_view() = default;

   arg_view(std::string_view stringKey): m_stringKey(stringKey) {}
   arg_view(std::string_view stringKey, const variant_view& value_): m_stringKey(stringKey), m_VVValue(value_) {}

   ~arg_view() = default;

   // ## methods -----------------------------------------------------------------

   bool operator==(const arg_view& o) const
   {
      if( m_stringKey != o.m_stringKey ) { return false; }
      if( m_VVValue.type() != o.m_VVValue.type() ) { return false; }
      if( m_VVValue.is_null() && o.m_VVValue.is_null() ) { return true; }

      return m_VVValue.compare(o.m_VVValue) == 0;
   }
   
   bool operator!=(const arg_view& other) const { return !(*this == other); }

   std::string_view get_key() const { return m_stringKey; }
   const gd::variant_view& get_value() const { return m_VVValue; }

   std::string_view first() const { return m_stringKey; }
   const gd::variant_view& second() const { return m_VVValue; }

   void set( std::string_view stringKey, const variant_view& value_ );
   void set_key(std::string_view stringKey) { m_stringKey = stringKey; }
   void set_value(const variant_view& value_) { m_VVValue = value_; }

   bool empty() const { return m_stringKey.empty() || m_VVValue.empty(); }
   bool empty_key() const { return m_stringKey.empty() == true; }
   bool empty_value() const { return m_VVValue.empty() == true; }

   std::string_view m_stringKey; // Key as string_view (non-owning)
   gd::variant_view m_VVValue;   // Value as variant_view (non-owning)
};

/// @brief Set the key and value of the arg_view
inline void arg_view::set( std::string_view stringKey, const variant_view& value_ )
{
   m_stringKey = stringKey;
   m_VVValue = value_;
}


/*-----------------------------------------*/ /**
 * @brief Argument object with string key and variant value
 *
 * This object owns its data and is used when you need to store
 * or modify the key-value pairs.
 *
 * This is particularly useful for:
 * - Storing arguments in containers
 * - Modifying values after creation
 * - Passing ownership of data to functions
 */
struct arg
{
   // ## construction ------------------------------------------------------------
   arg() = default;
   
   arg(const std::string& stringKey): m_stringKey(stringKey){ }
   arg(const std::string& stringKey, const gd::variant& value_): m_stringKey(stringKey), m_VValue(value_){ }
   
   arg(std::string&& stringKey, gd::variant&& value_) : m_stringKey(std::move(stringKey)), m_VValue(std::move(value_)) {}
   
   arg(const arg_view& argview_)                                            // Construct from arg_view (copies data)
      : m_stringKey(argview_.get_key()), m_VValue(argview_.get_value().as_variant()) {}

   ~arg() = default;

   // ## methods -----------------------------------------------------------------

   bool operator==(const arg& o) const
   {
      if( m_stringKey != o.m_stringKey ) { return false; }
      if( m_VValue.type() != o.m_VValue.type() ) { return false; }
      if( m_VValue.is_null() && o.m_VValue.is_null() ) { return true; }

      return m_VValue.compare(o.m_VValue) == 0;
   }
   
   bool operator!=(const arg& other) const { return !(*this == other); }

   const std::string& get_key() const { return m_stringKey; }
   const gd::variant& get_value() const { return m_VValue; }
   gd::variant& get_value() { return m_VValue; }

   const std::string& first() const { return m_stringKey; }
   const gd::variant& second() const { return m_VValue; }

   void set( const std::string& key, const gd::variant& value );
   void set( std::string&& key, gd::variant&& value );
   void set_key(const std::string& key) { m_stringKey = key; }
   void set_key(std::string&& key) { m_stringKey = std::move(key); }
   void set_value(const gd::variant& value) { m_VValue = value; }
   void set_value(gd::variant&& value) { m_VValue = std::move(value); }

   bool empty() const { return m_stringKey.empty() || m_VValue.empty(); }
   bool empty_key() const { return m_stringKey.empty() == true; }
   bool empty_value() const { return m_VValue.empty() == true; }

   operator arg_view() const { return arg_view(m_stringKey, m_VValue.as_variant_view()); }

   std::string m_stringKey; // Key as string (owning)
   gd::variant m_VValue;   // Value as variant (owning)
};

/// @brief Set the key and value of the arg (copy version)
inline void arg::set( const std::string& key, const gd::variant& value )
{
   m_stringKey = key;
   m_VValue = value;
}

/// @brief Set the key and value of the arg (move version)
inline void arg::set( std::string&& key, gd::variant&& value )
{
   m_stringKey = std::move(key);
   m_VValue = std::move(value);
}

// ## Utility functions for working with arg objects @API [tag: variant, arg, utility] [description: Utility functions for creating arg and arg_view objects]

/*-----------------------------------------*/ /**
 * @brief Create an arg_view from key and value
 *
 * This function provides a convenient way to create an arg_view object
 * from a key and value.
 *
 * @param stringKey The key as string_view
 * @param variantviewValue The value as variant_view
 * @return arg_view object with the provided key and value
 */
inline arg_view make_arg_view(std::string_view stringKey, const variant_view& variantviewValue)
{
   return arg_view(stringKey, variantviewValue);
}

/*-----------------------------------------*/ /**
 * @brief Create an arg_view from key and variant
 *
 * This function provides a convenient way to create an arg_view object
 * from a key and a variant.
 *
 * @param stringKey The key as string_view
 * @param variantValue The value as variant
 * @return arg_view object with the provided key and value
 */
inline arg_view make_arg_view(std::string_view stringKey, const gd::variant& variantValue)
{
   return arg_view(stringKey, variantValue.as_variant_view());
}

/*-----------------------------------------*/ /**
 * @brief Create an arg from key and value
 *
 * This function provides a convenient way to create an arg object
 * from a key and value.
 *
 * @param stringKey The key as string
 * @param variantValue The value as variant
 * @return arg object with the provided key and value
 */
inline arg make_arg(const std::string& stringKey, const gd::variant& variantValue)
{
   return arg(stringKey, variantValue);
}

/*-----------------------------------------*/ /**
 * @brief Create an arg from key and value (move version)
 *
 * This function provides a convenient way to create an arg object
 * from a key and value using move semantics.
 *
 * @param stringKey The key as string (moved)
 * @param variantValue The value as variant (moved)
 * @return arg object with the provided key and value
 */
inline arg make_arg(std::string&& stringKey, gd::variant&& variantValue)
{
   return arg(std::move(stringKey), std::move(variantValue));
}

} // namespace gd

// Reset any warning changes for the rest of the compilation unit
#if defined( __clang__ )
   #pragma GCC diagnostic pop
#elif defined( __GNUC__ )
   #pragma GCC diagnostic pop
#elif defined( _MSC_VER )
   #pragma warning(pop)
#endif
