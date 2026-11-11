// @FILE [tag: arguments, helper, template] [description: Helper objects that wrap arguments objects] [type: header]

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "gd_variant.h"
#include "gd_variant_view.h"

#ifndef _GD_ARGUMENT_BEGIN
#define _GD_ARGUMENT_BEGIN namespace gd::argument {
#define _GD_ARGUMENT_END }
#endif

_GD_ARGUMENT_BEGIN

/** Default policy; dictionary compiles out its key check when this policy is used. */
struct dictionary_allow_all_keys
{
   static constexpr bool contains(std::string_view);
};

constexpr bool dictionary_allow_all_keys::contains(std::string_view)
{
   return true;
}

/** @CLASS [name: dictionary] [description: Helper object that presents arguments as a key-value map] [type: header]
 *
 * \brief Wraps an arguments container with a simple key-value interface.
 *
 * KEY_POLICY must provide a static contains(std::string_view) function. Disallowed keys throw std::invalid_argument.
 * Reads return gd::variant_view; use get_variant() when an owning gd::variant is needed.
 *
 * \par Unrestricted keys
 * \code
 * gd::argument::arguments arguments;
 * gd::argument::dictionary<gd::argument::arguments> values;
 * values["count"] = 42;
 * values["name"] = "Ada";
 * int count = values.get<int>("count");
 * std::string_view name = values.get_variant_view("name").as_string_view();
 * gd::variant ownedCount = values.get_variant("count");
 * \endcode
 *
 * \par Restricting keys
 * \code
 * struct user_keys
 * {
 *    static bool contains(std::string_view stringKey)
 *    {
 *       return stringKey == "name" || stringKey == "active";
 *    }
 * };
 *
 * gd::argument::dictionary<gd::argument::arguments, user_keys> user;
 * user["name"] = "Ada";
 * user["active"] = true;
 * // user["email"] throws std::invalid_argument.
 * \endcode
 *
 * @tparam ARGUMENTS Arguments container used for storage.
 * @tparam KEY_POLICY Policy that allows or rejects each key; defaults to allowing all keys.
 */
template <typename ARGUMENTS, typename KEY_POLICY = dictionary_allow_all_keys>
class dictionary
{
public:
   using value_type = gd::variant_view;
   using argument_proxy_type = typename ARGUMENTS::argument_proxy;

   dictionary();
   explicit dictionary(const ARGUMENTS& arguments);
   explicit dictionary(ARGUMENTS&& arguments);
   dictionary(const dictionary& dictionaryOther);
   dictionary(dictionary&& dictionaryOther);
   dictionary& operator=(const dictionary& dictionaryOther);
   dictionary& operator=(dictionary&& dictionaryOther);

   argument_proxy_type operator[](std::string_view stringKey);
   value_type operator[](std::string_view stringKey) const;

   template <typename VALUE>
   dictionary& set(std::string_view stringKey, VALUE&& value);

   value_type get_variant_view(std::string_view stringKey) const;
   gd::variant get_variant(std::string_view stringKey) const;

   template <typename VALUE>
   VALUE get(std::string_view stringKey) const;

   bool contains(std::string_view stringKey) const;
   bool empty() const;
   std::size_t size() const;
   void clear();

   const ARGUMENTS& get_arguments() const;

private:
   void validate_key(std::string_view stringKey) const;

   ARGUMENTS m_arguments;
};

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>::dictionary() = default;

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>::dictionary(const ARGUMENTS& arguments) : m_arguments(arguments) {}

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>::dictionary(ARGUMENTS&& arguments) : m_arguments(std::move(arguments)) {}

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>::dictionary(const dictionary& dictionaryOther) = default;

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>::dictionary(dictionary&& dictionaryOther) = default;

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>& dictionary<ARGUMENTS, KEY_POLICY>::operator=(const dictionary& dictionaryOther) = default;

template <typename ARGUMENTS, typename KEY_POLICY>
dictionary<ARGUMENTS, KEY_POLICY>& dictionary<ARGUMENTS, KEY_POLICY>::operator=(dictionary&& dictionaryOther) = default;

template <typename ARGUMENTS, typename KEY_POLICY>
typename dictionary<ARGUMENTS, KEY_POLICY>::argument_proxy_type dictionary<ARGUMENTS, KEY_POLICY>::operator[](std::string_view stringKey)
{
   if constexpr( !std::is_same_v<KEY_POLICY, dictionary_allow_all_keys> ) { validate_key(stringKey); }
   return m_arguments[stringKey];
}

template <typename ARGUMENTS, typename KEY_POLICY>
typename dictionary<ARGUMENTS, KEY_POLICY>::value_type dictionary<ARGUMENTS, KEY_POLICY>::operator[](std::string_view stringKey) const
{
   return get_variant_view(stringKey);
}

template <typename ARGUMENTS, typename KEY_POLICY>
template <typename VALUE>
dictionary<ARGUMENTS, KEY_POLICY>& dictionary<ARGUMENTS, KEY_POLICY>::set(std::string_view stringKey, VALUE&& value)
{
   if constexpr( !std::is_same_v<KEY_POLICY, dictionary_allow_all_keys> ) { validate_key(stringKey); }
   m_arguments[stringKey] = std::forward<VALUE>(value);
   return *this;
}

template <typename ARGUMENTS, typename KEY_POLICY>
typename dictionary<ARGUMENTS, KEY_POLICY>::value_type dictionary<ARGUMENTS, KEY_POLICY>::get_variant_view(std::string_view stringKey) const
{
   if constexpr( !std::is_same_v<KEY_POLICY, dictionary_allow_all_keys> ) { validate_key(stringKey); }
   return m_arguments.get_variant_view(stringKey);
}

template <typename ARGUMENTS, typename KEY_POLICY>
gd::variant dictionary<ARGUMENTS, KEY_POLICY>::get_variant(std::string_view stringKey) const
{
   return get_variant_view(stringKey).as_variant();
}

template <typename ARGUMENTS, typename KEY_POLICY>
template <typename VALUE>
VALUE dictionary<ARGUMENTS, KEY_POLICY>::get(std::string_view stringKey) const
{
   return get_variant_view(stringKey).template as<VALUE>();
}

template <typename ARGUMENTS, typename KEY_POLICY>
bool dictionary<ARGUMENTS, KEY_POLICY>::contains(std::string_view stringKey) const
{
   if constexpr( !std::is_same_v<KEY_POLICY, dictionary_allow_all_keys> ) { validate_key(stringKey); }
   return m_arguments.exists(stringKey);
}

template <typename ARGUMENTS, typename KEY_POLICY>
bool dictionary<ARGUMENTS, KEY_POLICY>::empty() const
{
   return m_arguments.empty();
}

template <typename ARGUMENTS, typename KEY_POLICY>
std::size_t dictionary<ARGUMENTS, KEY_POLICY>::size() const
{
   return m_arguments.size();
}

template <typename ARGUMENTS, typename KEY_POLICY>
void dictionary<ARGUMENTS, KEY_POLICY>::clear()
{
   m_arguments.clear();
}

template <typename ARGUMENTS, typename KEY_POLICY>
const ARGUMENTS& dictionary<ARGUMENTS, KEY_POLICY>::get_arguments() const
{
   return m_arguments;
}

template <typename ARGUMENTS, typename KEY_POLICY>
void dictionary<ARGUMENTS, KEY_POLICY>::validate_key(std::string_view stringKey) const
{
   if( !KEY_POLICY::contains(stringKey) ) { throw std::invalid_argument("dictionary key is not allowed"); }
}

_GD_ARGUMENT_END