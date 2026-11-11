// @FILE [tag: arguments, helper, template] [description: Helper objects that wrap arguments objects] [type: header]

#pragma once

#include <cstddef>
#include <string_view>
#include <utility>

#ifndef _GD_ARGUMENT_BEGIN
#define _GD_ARGUMENT_BEGIN namespace gd::argument {
#define _GD_ARGUMENT_END }
#endif

_GD_ARGUMENT_BEGIN

/** @CLASS [name: dictionary] [description: Helper object that presents arguments as a key-value map] [type: header]
 *
 * \brief Wraps an arguments container with a simple key-value interface.
 */
template <typename ARGUMENTS>
class dictionary
{
public:
   using argument_type = typename ARGUMENTS::argument;
   using argument_proxy_type = typename ARGUMENTS::argument_proxy;

   dictionary();
   explicit dictionary(const ARGUMENTS& arguments);
   explicit dictionary(ARGUMENTS&& arguments);
   dictionary(const dictionary& o);
   dictionary(dictionary&& o);
   dictionary& operator=(const dictionary& o);
   dictionary& operator=(dictionary&& o);

   argument_proxy_type operator[](std::string_view stringKey);
   argument_type operator[](std::string_view stringKey) const;

   template <typename VALUE>
   dictionary& set(std::string_view stringKey, VALUE&& value);

   argument_type get_argument(std::string_view stringKey) const;

   template <typename VALUE>
   VALUE get(std::string_view stringKey) const;

   bool contains(std::string_view stringKey) const;
   bool empty() const;
   std::size_t size() const;
   void clear();

   ARGUMENTS& get_arguments();
   const ARGUMENTS& get_arguments() const;

private:
   ARGUMENTS m_arguments;
};

template <typename ARGUMENTS>
dictionary<ARGUMENTS>::dictionary() = default;

template <typename ARGUMENTS>
dictionary<ARGUMENTS>::dictionary(const ARGUMENTS& arguments) : m_arguments(arguments) {}

template <typename ARGUMENTS>
dictionary<ARGUMENTS>::dictionary(ARGUMENTS&& arguments) : m_arguments(std::move(arguments)) {}

template <typename ARGUMENTS>
dictionary<ARGUMENTS>::dictionary(const dictionary& o) = default;

template <typename ARGUMENTS>
dictionary<ARGUMENTS>::dictionary(dictionary&& o) = default;

template <typename ARGUMENTS>
dictionary<ARGUMENTS>& dictionary<ARGUMENTS>::operator=(const dictionary& o) = default;

template <typename ARGUMENTS>
dictionary<ARGUMENTS>& dictionary<ARGUMENTS>::operator=(dictionary&& o) = default;

template <typename ARGUMENTS>
typename dictionary<ARGUMENTS>::argument_proxy_type dictionary<ARGUMENTS>::operator[](std::string_view stringKey)
{
   return m_arguments[stringKey];
}

template <typename ARGUMENTS>
typename dictionary<ARGUMENTS>::argument_type dictionary<ARGUMENTS>::operator[](std::string_view stringKey) const
{
   return m_arguments[stringKey];
}

template <typename ARGUMENTS>
template <typename VALUE>
dictionary<ARGUMENTS>& dictionary<ARGUMENTS>::set(std::string_view stringKey, VALUE&& value)
{
   m_arguments[stringKey] = std::forward<VALUE>(value);
   return *this;
}

template <typename ARGUMENTS>
typename dictionary<ARGUMENTS>::argument_type dictionary<ARGUMENTS>::get_argument(std::string_view stringKey) const
{
   return m_arguments.get_argument(stringKey);
}

template <typename ARGUMENTS>
template <typename VALUE>
VALUE dictionary<ARGUMENTS>::get(std::string_view stringKey) const
{
   return get_argument(stringKey).template get<VALUE>();
}

template <typename ARGUMENTS>
bool dictionary<ARGUMENTS>::contains(std::string_view stringKey) const
{
   return m_arguments.exists(stringKey);
}

template <typename ARGUMENTS>
bool dictionary<ARGUMENTS>::empty() const
{
   return m_arguments.empty();
}

template <typename ARGUMENTS>
std::size_t dictionary<ARGUMENTS>::size() const
{
   return m_arguments.size();
}

template <typename ARGUMENTS>
void dictionary<ARGUMENTS>::clear()
{
   m_arguments.clear();
}

template <typename ARGUMENTS>
ARGUMENTS& dictionary<ARGUMENTS>::get_arguments()
{
   return m_arguments;
}

template <typename ARGUMENTS>
const ARGUMENTS& dictionary<ARGUMENTS>::get_arguments() const
{
   return m_arguments;
}

_GD_ARGUMENT_END