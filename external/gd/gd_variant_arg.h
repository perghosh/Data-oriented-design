// @FILE [tag: variant, arg] [description: Argument objects for variant key-value pairs] [type: header]
//
/*


## args_view
Container for non-owning arg_view elements

| Area | Methods (Examples) | Description |
|------|--------------------|-------------|
| Construction | `args_view()`, `args_view(initializer_list)` | Constructors for creating args_view instances |
| Iteration | `begin()`, `end()`, `rbegin()`, `rend()`, `cbegin()`, `cend()` | Iterator methods for traversing the container |
| Access | `at()`, `operator[]`, `front()`, `back()`, `data()` | Methods for accessing elements in the container |
| Capacity | `empty()`, `size()`, `capacity()`, `reserve()` | Methods for querying and managing container capacity |
| Modifiers | `push_back()`, `pop_back()`, `insert()`, `erase()`, `clear()`, `resize()` | Methods for modifying the contents of the container |
| Comparison | `operator==`, `operator!=` | Equality comparison operators for args_view instances |
| Operations | `swap()`, `find()`, `contains()`, `count()` | Utility methods for searching and comparing |
| Algorithms | `find_if()`, `find_if_reverse()`, `any_of()`, `all_of()`, `none_of()` | Template methods for applying algorithms to elements |

| Area | Methods (Examples) | Description |
|------|--------------------|-------------|
| Construction | `arg()`, `arg(stringKey)`, `arg(stringKey, value_)`, `arg(argview_)` | Constructors for creating arg instances with various initialization options |
| Assignment | `set(...)`, `set_key(...)`, `set_value(...)` | Methods for updating the key, value, or both of the arg |
| Comparison | `operator==`, `operator!=` | Equality comparison operators for arg instances |
| Retrieval | `get_key()`, `get_value()`, `first()`, `second()` | Methods for accessing the key and value stored in the arg |
| Queries | `empty()`, `empty_key()`, `empty_value()` | Methods for checking the state of the arg components |
| Conversion | `operator arg_view()` (from arg) | Conversion operator from arg to arg_view |

## args
   Container for owning arg elements

| Area | Methods (Examples) | Description |
|------|--------------------|-------------|
| Construction | `args()`, `args(initializer_list)`, `args(argsview_)` | Constructors for creating args instances with various initialization options |
| Iteration | `begin()`, `end()`, `rbegin()`, `rend()`, `cbegin()`, `cend()` | Iterator methods for traversing the container |
| Access | `at()`, `operator[]`, `front()`, `back()`, `data()` | Methods for accessing elements in the container |
| Capacity | `empty()`, `size()`, `capacity()`, `reserve()` | Methods for querying and managing container capacity |
| Modifiers | `push_back()`, `pop_back()`, `insert()`, `erase()`, `clear()`, `resize()` | Methods for modifying the contents of the container |
| Comparison | `operator==`, `operator!=` | Equality comparison operators for args instances |
| Operations | `swap()`, `find()`, `contains()`, `count()`, `remove()` | Utility methods for searching and modifying |
| Conversion | `operator args_view()` | Conversion operator from args to args_view |
| Algorithms | `find_if()`, `find_if_reverse()`, `any_of()`, `all_of()`, `none_of()` | Template methods for applying algorithms to elements |
| Batch Operations | `assign()` | Methods for batch assignment of elements |

## class args
   Container for owning arg elements

| Area | Methods (Examples) | Description |
|------|--------------------|-------------|
| Construction | `args()`, `args(initializer_list)`, `args(argsview_)` | Constructors for creating args instances with various initialization options |
| Iteration | `begin()`, `end()`, `rbegin()`, `rend()`, `cbegin()`, `cend()` | Iterator methods for traversing the container |
| Access | `at()`, `operator[]`, `front()`, `back()`, `data()` | Methods for accessing elements in the container |
| Capacity | `empty()`, `size()`, `capacity()`, `reserve()` | Methods for querying and managing container capacity |
| Modifiers | `push_back()`, `pop_back()`, `insert()`, `erase()`, `clear()`, `resize()` | Methods for modifying the contents of the container |
| Comparison | `operator==`, `operator!=` | Equality comparison operators for args instances |
| Operations | `swap()`, `find()`, `contains()`, `count()`, `remove()` | Utility methods for searching and modifying |
| Conversion | `operator args_view()` | Conversion operator from args to args_view |
| Algorithms | `find_if()`, `find_if_reverse()`, `any_of()`, `all_of()`, `none_of()` | Template methods for applying algorithms to elements |
| Batch Operations | `assign()` | Methods for batch assignment of elements |

 */

#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <type_traits>
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

/** @CLASS [tag: variant, arg_view] [description: Argument object with owning key-value pairs] [name: arg_view]
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

   bool operator!=(const arg_view& o) const { return !(*this == o); }

   std::string_view get_key() const { return m_stringKey; }
   const gd::variant_view& get_value() const { return m_VVValue; }

   std::string_view first() const { return m_stringKey; }
   const gd::variant_view& second() const { return m_VVValue; }

   void set( std::string_view stringKey, const variant_view& value_ );
   void set(const variant_view& value_) { m_VVValue = value_; }
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


/** @CLASS [tag: variant, arg] [description: Argument object with owning key-value pairs] [name: arg]
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

   bool operator!=(const arg& o) const { return !(*this == o); }

   const std::string& get_key() const { return m_stringKey; }
   const gd::variant& get_value() const { return m_VValue; }
   gd::variant& get_value() { return m_VValue; }

   const std::string& first() const { return m_stringKey; }
   const gd::variant& second() const { return m_VValue; }

   void set( const std::string& key, const gd::variant& value );
   void set( std::string&& key, gd::variant&& value );
   void set(const gd::variant& value) { m_VValue = value; }
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


/** @CLASS [tag: variant, args_view] [description: Container for non-owning arg_view objects] [name: args_view]
 * @brief Container for arg_view objects
 *
 * This container holds multiple arg_view objects without owning the data.
 * It's useful when you need to work with a collection of arguments without
 * taking ownership of the data.
 *
 * This is particularly useful for:
 * - Passing multiple arguments to functions efficiently
 * - Temporary collections of arguments
 * - Avoiding unnecessary copies of argument data
 */
struct args_view
{
   using iterator = std::vector<arg_view>::iterator;
   using const_iterator = std::vector<arg_view>::const_iterator;
   using iterator_category = std::forward_iterator_tag;
   using value_type = arg_view;
   using pointer = arg_view*;
   using const_pointer = const arg_view*;
   using reference = arg_view&;
   using const_reference = const arg_view&;
   using size_type = size_t;
   using difference_type = std::ptrdiff_t;

   // ## construction ------------------------------------------------------------
   args_view() = default;
   args_view(std::initializer_list<arg_view> list) : m_vectorArgs(list) {}
   ~args_view() = default;

   // ## Forward iteration
   iterator begin() { return m_vectorArgs.begin(); }
   iterator end() { return m_vectorArgs.end(); }
   const_iterator begin() const { return m_vectorArgs.begin(); }
   const_iterator end() const { return m_vectorArgs.end(); }
   const_iterator cbegin() const { return m_vectorArgs.cbegin(); }
   const_iterator cend() const { return m_vectorArgs.cend(); }

   // ## Reverse iteration
   using reverse_iterator = std::vector<arg_view>::reverse_iterator;
   using const_reverse_iterator = std::vector<arg_view>::const_reverse_iterator;

   reverse_iterator rbegin() { return m_vectorArgs.rbegin(); }
   reverse_iterator rend() { return m_vectorArgs.rend(); }
   const_reverse_iterator rbegin() const { return m_vectorArgs.rbegin(); }
   const_reverse_iterator rend() const { return m_vectorArgs.rend(); }
   const_reverse_iterator crbegin() const { return m_vectorArgs.crbegin(); }
   const_reverse_iterator crend() const { return m_vectorArgs.crend(); }

   // ## Access
   arg_view& at(size_t uIndex) { return m_vectorArgs.at(uIndex); }
   const arg_view& at(size_t uIndex) const { return m_vectorArgs.at(uIndex); }
   arg_view& operator[](size_t uIndex) { return m_vectorArgs[uIndex]; }
   const arg_view& operator[](size_t uIndex) const { return m_vectorArgs[uIndex]; }

   bool operator==(const args_view& o) const;
   bool operator!=(const args_view& o) const { return !(*this == o); }

   // ## Capacity
   bool empty() const { return m_vectorArgs.empty(); }
   size_t size() const { return m_vectorArgs.size(); }
   void reserve(size_t uCapacity) { m_vectorArgs.reserve(uCapacity); }
   size_t capacity() const { return m_vectorArgs.capacity(); }
   void clear() { m_vectorArgs.clear(); }

   // ## Modifiers
   void push_back(const arg_view& argview_) { m_vectorArgs.push_back(argview_); }
   void push_back(arg_view&& argview_) { m_vectorArgs.push_back(std::move(argview_)); }
   void pop_back() { m_vectorArgs.pop_back(); }

   template<typename... Args>
   void emplace_back(Args&&... args) { m_vectorArgs.emplace_back(std::forward<Args>(args)...); }

   void resize(size_t uSize) { m_vectorArgs.resize(uSize); }
   void resize(size_t uSize, const arg_view& argview_) { m_vectorArgs.resize(uSize, argview_); }

   iterator insert(const_iterator it, const arg_view& argview_) { return m_vectorArgs.insert(it, argview_); }
   iterator insert(const_iterator it, arg_view&& argview_) { return m_vectorArgs.insert(it, std::move(argview_)); }
   iterator insert(const_iterator it, size_t uCount, const arg_view& argview_) { return m_vectorArgs.insert(it, uCount, argview_); }
   iterator insert(const_iterator it, std::initializer_list<arg_view> list_) { return m_vectorArgs.insert(it, list_); }

   template<typename... Args>
   iterator emplace(const_iterator it, Args&&... args) { return m_vectorArgs.emplace(it, std::forward<Args>(args)...); }

   iterator erase(const_iterator it) { return m_vectorArgs.erase(it); }
   iterator erase(const_iterator itFirst, const_iterator itLast) { return m_vectorArgs.erase(itFirst, itLast); }

   // ## Operations
   void swap(args_view& o) noexcept { m_vectorArgs.swap(o.m_vectorArgs); }

   // ## Finding
   const_iterator find(std::string_view stringKey) const;

   bool contains(std::string_view stringKey) const { return find(stringKey) != end();  }

   // ## Utility methods
   template<typename PREDICATE>
   const_iterator find_if(PREDICATE predicate_) const
   {
      for(const_iterator it = begin(); it != end(); ++it)
      {
         if(predicate_(*it)) return it;
      }
      return end();
   }

   template<typename PREDICATE>
   const_reverse_iterator find_if_reverse(PREDICATE predicate_) const
   {
      for(const_reverse_iterator it = rbegin(); it != rend(); ++it)
      {
         if(predicate_(*it)) return it;
      }
      return rend();
   }

   template<typename PREDICATE>
   bool any_of(PREDICATE predicate_) const { return std::any_of(begin(), end(), predicate_); }

   template<typename PREDICATE>
   bool all_of(PREDICATE predicate_) const { return std::all_of(begin(), end(), predicate_); }

   template<typename PREDICATE>
   bool none_of(PREDICATE predicate_) const { return std::none_of(begin(), end(), predicate_); }

   size_t count(std::string_view stringKey) const;

   // ## Front/back access
   arg_view& front() { return m_vectorArgs.front(); }
   const arg_view& front() const { return m_vectorArgs.front(); }
   arg_view& back() { return m_vectorArgs.back(); }
   const arg_view& back() const { return m_vectorArgs.back(); }

   // ## Data access
   arg_view* data() { return m_vectorArgs.data(); }
   const arg_view* data() const { return m_vectorArgs.data(); }

   std::vector<arg_view> m_vectorArgs; // Container for arg_view objects
};

/// Equality operator for args_view
inline bool args_view::operator==(const args_view& o) const
{
   if(size() != o.size()) return false;
   for(size_t i = 0; i < size(); ++i)
   {
      if(m_vectorArgs[i] != o[i]) return false;
   }
   return true;
}

/// @brief Find an argument by key
inline args_view::const_iterator args_view::find(std::string_view stringKey) const
{
   for(const_iterator it = begin(); it != end(); ++it)
   {
      if(it->get_key() == stringKey)
         return it;
   }
   return end();
}

/// @brief Count the number of arguments with a given key
inline size_t args_view::count(std::string_view stringKey) const
{
   size_t uCount = 0;
   for(const auto& argview_ : m_vectorArgs)
   {
      if(argview_.get_key() == stringKey) ++uCount;
   }
   return uCount;
}



/** @CLASS [tag: variant, args] [description: Container for owning arg objects] [name: args]
 * @brief Container for arg objects
 *
 * This container holds multiple arg objects and owns the data.
 * It's used when you need to store, modify, or pass ownership of
 * a collection of arguments.
 *
 * This is particularly useful for:
 * - Storing argument collections for long-term use
 * - Modifying argument values
 * - Passing ownership of argument data to functions
 */
struct args
{
   using iterator = std::vector<arg>::iterator;
   using const_iterator = std::vector<arg>::const_iterator;
   using iterator_category = std::forward_iterator_tag;
   using value_type = arg;
   using pointer = arg*;
   using const_pointer = const arg*;
   using reference = arg&;
   using const_reference = const arg&;
   using size_type = size_t;
   using difference_type = std::ptrdiff_t;

   // ## construction ------------------------------------------------------------
   args() = default;
   args(std::initializer_list<arg> list) : m_vectorArgs(list) {}
   args(const args_view& argsview_) {
      m_vectorArgs.reserve(argsview_.size());
      for(const auto& argview_ : argsview_) {
         m_vectorArgs.emplace_back(argview_);
      }
   }
   ~args() = default;

   // ## methods -----------------------------------------------------------------

   // ## Forward iteration
   iterator begin() { return m_vectorArgs.begin(); }
   iterator end() { return m_vectorArgs.end(); }
   const_iterator begin() const { return m_vectorArgs.begin(); }
   const_iterator end() const { return m_vectorArgs.end(); }
   const_iterator cbegin() const { return m_vectorArgs.cbegin(); }
   const_iterator cend() const { return m_vectorArgs.cend(); }

   // ## Reverse iteration
   using reverse_iterator = std::vector<arg>::reverse_iterator;
   using const_reverse_iterator = std::vector<arg>::const_reverse_iterator;

   reverse_iterator rbegin() { return m_vectorArgs.rbegin(); }
   reverse_iterator rend() { return m_vectorArgs.rend(); }
   const_reverse_iterator rbegin() const { return m_vectorArgs.rbegin(); }
   const_reverse_iterator rend() const { return m_vectorArgs.rend(); }
   const_reverse_iterator crbegin() const { return m_vectorArgs.crbegin(); }
   const_reverse_iterator crend() const { return m_vectorArgs.crend(); }

   // ## Access
   arg& at(size_t uIndex) { return m_vectorArgs.at(uIndex); }
   const arg& at(size_t uIndex) const { return m_vectorArgs.at(uIndex); }
   arg& operator[](size_t uIndex) { return m_vectorArgs[uIndex]; }
   const arg& operator[](size_t uIndex) const { return m_vectorArgs[uIndex]; }

   // ## Capacity
   bool empty() const { return m_vectorArgs.empty(); }
   size_t size() const { return m_vectorArgs.size(); }
   void reserve(size_t uCapacity) { m_vectorArgs.reserve(uCapacity); }
   size_t capacity() const { return m_vectorArgs.capacity(); }
   void clear() { m_vectorArgs.clear(); }

   // ## Modifiers
   void push_back(const arg& arg_) { m_vectorArgs.push_back(arg_); }
   void push_back(arg&& arg_) { m_vectorArgs.push_back(std::move(arg_)); }
   void pop_back() { m_vectorArgs.pop_back(); }

   template<typename... Args>
   auto& emplace_back(Args&&... args) { return m_vectorArgs.emplace_back(std::forward<Args>(args)...); }

   void resize(size_t uSize) { m_vectorArgs.resize(uSize); }
   void resize(size_t uSize, const arg& arg_) { m_vectorArgs.resize(uSize, arg_); }

   iterator insert(const_iterator it, const arg& arg_) { return m_vectorArgs.insert(it, arg_); }
   iterator insert(const_iterator it, arg&& arg_) { return m_vectorArgs.insert(it, std::move(arg_)); }
   iterator insert(const_iterator it, size_t uCount, const arg& arg_) { return m_vectorArgs.insert(it, uCount, arg_); }
   iterator insert(const_iterator it, std::initializer_list<arg> list) { return m_vectorArgs.insert(it, list); }

   template<typename... Args>
   iterator emplace(const_iterator it, Args&&... args) { return m_vectorArgs.emplace(it, std::forward<Args>(args)...); }

   iterator erase(const_iterator it) { return m_vectorArgs.erase(it); }
   iterator erase(const_iterator itFirst, const_iterator itLast) { return m_vectorArgs.erase(itFirst, itLast); }

   // ## Operations
   void swap(args& o) noexcept { m_vectorArgs.swap(o.m_vectorArgs); }

   // ## Finding
   iterator find(std::string_view stringKey);
   const_iterator find(std::string_view stringKey) const;
   bool contains(std::string_view stringKey) const { return find(stringKey) != end(); }

   // ## Conversion
   operator args_view() const
   {
      args_view argsview_;
      argsview_.reserve(size());
      for(const auto& arg_ : *this)
      {
         argsview_.push_back(arg_);
      }
      return argsview_;
   }

   // ## Comparison operators
   bool operator==(const args& o) const;
   bool operator!=(const args& o) const { return !(*this == o); }

   // ## Utility methods
   template<typename PREDICATE>
   auto find_if(PREDICATE predicate_)
   {
      for(auto it = begin(); it != end(); ++it)
      {
         if(predicate_(*it)) return it;
      }
      return end();
   }

   template<typename PREDICATE>
   auto find_if(PREDICATE predicate_) const
   {
      for(auto it = begin(); it != end(); ++it)
      {
         if(predicate_(*it)) return it;
      }
      return end();
   }

   template<typename PREDICATE>
   reverse_iterator find_if_reverse(PREDICATE predicate_)
   {
      for(reverse_iterator it = rbegin(); it != rend(); ++it)
      {
         if(predicate_(*it)) return it;
      }
      return rend();
   }

   template<typename PREDICATE>
   const_reverse_iterator find_if_reverse(PREDICATE predicate_) const
   {
      for(const_reverse_iterator it = rbegin(); it != rend(); ++it)
      {
         if(predicate_(*it)) return it;
      }
      return rend();
   }

   template<typename PREDICATE>
   bool any_of(PREDICATE pred) const
   {
      return std::any_of(begin(), end(), pred);
   }

   template<typename PREDICATE>
   bool all_of(PREDICATE predicate_) const
   {
      return std::all_of(begin(), end(), predicate_);
   }

   template<typename PREDICATE>
   bool none_of(PREDICATE predicate_) const
   {
      return std::none_of(begin(), end(), predicate_);
   }

   // ## Batch operations
   template<typename InputIt>
   void assign(InputIt first, InputIt last)
   {
      m_vectorArgs.assign(first, last);
   }

   void assign(size_type uCount, const arg& arg_)
   {
      m_vectorArgs.assign(uCount, arg_);
   }

   void assign(std::initializer_list<arg> list)
   {
      m_vectorArgs.assign(list);
   }

   size_t count(std::string_view stringKey) const;
   iterator remove(std::string_view stringKey);

   // ## Front/back access
   arg& front() { return m_vectorArgs.front(); }
   const arg& front() const { return m_vectorArgs.front(); }
   arg& back() { return m_vectorArgs.back(); }
   const arg& back() const { return m_vectorArgs.back(); }

   // ## Data access
   arg* data() { return m_vectorArgs.data(); }
   const arg* data() const { return m_vectorArgs.data(); }

   std::vector<arg> m_vectorArgs; // Container for arg objects
};

// ## Out-of-class implementations for args

/// Equality operator for args
inline bool args::operator==(const args& o) const
{
   if(size() != o.size()) return false;
   for(size_t i = 0; i < size(); ++i)
   {
      if(m_vectorArgs[i] != o[i]) return false;
   }
   return true;
}

/// @brief Find an argument by key (non-const version)
inline args::iterator args::find(std::string_view stringKey)
{
   for(iterator it = begin(); it != end(); ++it)
   {
      if(it->get_key() == stringKey)
         return it;
   }
   return end();
}

/// @brief Find an argument by key (const version)
inline args::const_iterator args::find(std::string_view stringKey) const
{
   for(const_iterator it = begin(); it != end(); ++it)
   {
      if(it->get_key() == stringKey)
         return it;
   }
   return end();
}

/// @brief Count the number of arguments with a given key
inline size_t args::count(std::string_view stringKey) const
{
   size_t uCount = 0;
   for(const auto& arg_ : m_vectorArgs)
   {
      if(arg_.get_key() == stringKey) ++uCount;
   }
   return uCount;
}

/// @brief Remove arguments with a given key
inline args::iterator args::remove(std::string_view stringKey)
{
   auto it = std::remove_if(begin(), end(),
      [&stringKey](const arg& arg_) { return arg_.get_key() == stringKey; });
   return m_vectorArgs.erase(it, end());
}

// @API [tag: variant, arg, args, factory] [description: Factory functions for creating args and args_view objects]

/// @brief Create an args_view from initializer list
args_view make_args_view(std::initializer_list<arg_view> list);

/// @brief Create an args from initializer list
args make_args(std::initializer_list<arg> list);

/// @brief Create an args_view from key-value pairs
args_view make_args_view_from_pairs(std::initializer_list<std::pair<std::string_view, variant_view>> pairs);

/// @brief Create an args from key-value pairs
args make_args_from_pairs(std::initializer_list<std::pair<std::string, variant>> pairs);

// @API [tag: variant, arg, args, utility] [description: Additional utility functions for working with args and args_view]

/// @brief Find a value by key in args_view
variant_view find_value(const args_view& argsview_, std::string_view stringKey);

/// @brief Find a value by key in args
variant find_value(const args& args_, std::string_view stringKey);

/// @brief Get a value by key from args_view, with default value
variant_view get_value_or(const args_view& argsview_, std::string_view stringKey, const variant_view& variantviewDefault);

/// @brief Get a value by key from args, with default value
variant get_value_or(const args& args_, std::string_view stringKey, const variant& variantDefault);

/// @brief Convert args_view to args
args to_args(const args_view& argsview_);

/// @brief Filter args_view by predicate
args_view filter_args_view(const args_view& argsview_, std::function<bool(const arg_view&)> pred);

/// @brief Transform args_view by applying a function to each element
args_view transform_args_view(const args_view& argsview_, std::function<arg_view(const arg_view&)> func);

 /// @brief Transform args by applying a function to each element
args transform_args(const args& args_, std::function<arg(const arg&)> func);

/// @brief Check if any argument in args_view has a specific key
bool has_key(const args_view& argsview_, std::string_view stringKey);

/// @brief Check if any argument in args has a specific key
bool has_key(const args& args_, std::string_view stringKey);

/// @brief Get all keys from args_view
std::vector<std::string_view> get_keys(const args_view& argsview_);

/// @brief Get all keys from args
std::vector<std::string> get_keys(const args& args_);

/// @brief Get all values from args_view
std::vector<variant_view> get_values(const args_view& argsview_);

/// @brief Get all values from args
std::vector<variant> get_values(const args& args_);

/// @brief Filter args by predicate
args filter_args(const args& args_, std::function<bool(const arg&)> pred);


_GD_END

// Reset any warning changes for the rest of the compilation unit
#if defined( __clang__ )
   #pragma GCC diagnostic pop
#elif defined( __GNUC__ )
   #pragma GCC diagnostic pop
#elif defined( _MSC_VER )
   #pragma warning(pop)
#endif
