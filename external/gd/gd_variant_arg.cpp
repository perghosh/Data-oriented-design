// @FILE [tag: variant, arg] [description: Implementation for argument objects] [type: source]

#include "gd_variant_arg.h"

_GD_BEGIN


// ## Factory functions for arg_view with primitive values


arg_view make_arg_view(std::string_view stringKey, variant_view value_)
{
   return arg_view(stringKey, value_);
}

// ## Factory functions for args and args_view with primitive values

args_view make_args_view(std::initializer_list<arg_view> list)
{
   return args_view(list);
}

args make_args(std::initializer_list<arg> list)
{
   return args(list);
}

// ## Utility functions for args and args_view with key-value pairs

args_view make_args_view_from_pairs(std::initializer_list<std::pair<std::string_view, variant_view>> pairs)
{
   args_view result;
   result.reserve(pairs.size());
   for(const auto& pair : pairs)
   {
      result.emplace_back(pair.first, pair.second);
   }
   return result;
}

/** ----------------------------------------- */ /**
 * @brief Create args from key-value pairs
 *
 * This function creates an args container from an initializer list
 * of key-value pairs, where the key is a string and the value is a variant.
 *
 * @param pairs An initializer list of key-value pairs
 * @return An args container containing the provided key-value pairs
 */
args make_args_from_pairs(std::initializer_list<std::pair<std::string, variant>> pairs)
{
   args result;
   result.reserve(pairs.size());
   for(const auto& pair : pairs)
   {
      result.emplace_back(pair.first, pair.second);
   }
   return result;
}

// ## Additional utility functions for args and args_view

/** -----------------------------------------*/ /**
 * @brief Find a value by key in args_view
 *
 * This function provides a convenient way to find a value by key
 * in an args_view container.
 *
 * @param argsview_ The args_view container to search
 * @param stringKey The key to search for
 * @return variant_view of the found value, or an empty variant_view if not found
 */
variant_view find_value(const args_view& argsview_, std::string_view stringKey)
{
   auto it = argsview_.find(stringKey);
   if(it != argsview_.end())
      return it->get_value();
   return variant_view();
}

/** -----------------------------------------*/ /**
 * @brief Find a value by key in args
 *
 * This function provides a convenient way to find a value by key
 * in an args container.
 *
 * @param args_ The args container to search
 * @param stringKey The key to search for
 * @return variant of the found value, or an empty variant if not found
 */
variant find_value(const args& args_, std::string_view stringKey)
{
   auto it = args_.find(stringKey);
   if(it != args_.end())
      return it->get_value();
   return variant();
}

/** -----------------------------------------*/ /**
 * @brief Get a value by key from args_view, with default value
 *
 * This function provides a convenient way to get a value by key
 * from an args_view container, with a default value if the key is not found.
 *
 * @param argsview_ The args_view container to search
 * @param stringKey The key to search for
 * @param variantviewDefault The default value to return if the key is not found
 * @return variant_view of the found value, or the default value if not found
 */
variant_view get_value_or(const args_view& argsview_, std::string_view stringKey, const variant_view& variantviewDefault)
{
   auto it = argsview_.find(stringKey);
   if(it != argsview_.end())
      return it->get_value();
   return variantviewDefault;
}

/** -----------------------------------------*/ /**
 * @brief Get a value by key from args, with default value
 *
 * This function provides a convenient way to get a value by key
 * from an args container, with a default value if the key is not found.
 *
 * @param args_ The args container to search
 * @param stringKey The key to search for
 * @param variantDefault The default value to return if the key is not found
 * @return variant of the found value, or the default value if not found
 */
variant get_value_or(const args& args_, std::string_view stringKey, const variant& variantDefault)
{
   auto it = args_.find(stringKey);
   if(it != args_.end())
      return it->get_value();
   return variantDefault;
}

/** -----------------------------------------*/ /**
 * @brief Convert args_view to args
 *
 * This function provides a convenient way to convert an args_view
 * container to an args container.
 *
 * @param argsview_ The args_view container to convert
 * @return args container with the same arguments
 */
args to_args(const args_view& argsview_)
{
   return args(argsview_);
}

/** -----------------------------------------*/ /**
 * @brief Filter args_view by predicate
 *
 * This function provides a convenient way to filter an args_view
 * container by a predicate.
 *
 * @param argsview_ The args_view container to filter
 * @param pred The predicate to apply to each element
 * @return args_view container with the filtered elements
 */
args_view filter_args_view(const args_view& argsview_, std::function<bool(const arg_view&)> pred)
{
   args_view result;
   for(const auto& argview_ : argsview_)
   {
      if(pred(argview_))
         result.push_back(argview_);
   }
   return result;
}

/** -----------------------------------------*/ /**
 * @brief Filter args by predicate
 *
 * This function provides a convenient way to filter an args
 * container by a predicate.
 *
 * @param args_ The args container to filter
 * @param pred The predicate to apply to each element
 * @return args container with the filtered elements
 */
args filter_args(const args& args_, std::function<bool(const arg&)> pred)
{
   args result;
   for(const auto& arg_ : args_)
   {
      if(pred(arg_))
         result.push_back(arg_);
   }
   return result;
}

// ## Implementation of additional utility functions

/** -----------------------------------------*/ /**
 * @brief Transform args_view by applying a function to each element
 *
 * This function provides a convenient way to transform an args_view
 * container by applying a function to each element.
 *
 * @param argsview_ The args_view container to transform
 * @param func The transformation function to apply to each element
 * @return args_view container with the transformed elements
 */
args_view transform_args_view(const args_view& argsview_, std::function<arg_view(const arg_view&)> func)
{
   args_view result;
   result.reserve(argsview_.size());
   for(const auto& argview_ : argsview_)
   {
      result.push_back(func(argview_));
   }
   return result;
}

/** -----------------------------------------*/ /**
 * @brief Transform args by applying a function to each element
 *
 * This function provides a convenient way to transform an args
 * container by applying a function to each element.
 *
 * @param args_ The args container to transform
 * @param func The transformation function to apply to each element
 * @return args container with the transformed elements
 */
args transform_args(const args& args_, std::function<arg(const arg&)> callback_)
{
   args result;
   result.reserve(args_.size());
   for(const auto& arg_ : args_)
   {
      auto argTransformed = callback_( arg_ );
      result.push_back( argTransformed );
   }
   return result;
}

/** -----------------------------------------*/ /**
 * @brief Check if any argument in args_view has a specific key
 *
 * This function provides a convenient way to check if any argument
 * in an args_view container has a specific key.
 *
 * @param argsview_ The args_view container to search
 * @param stringKey The key to search for
 * @return true if any argument has the specified key, false otherwise
 */
bool has_key(const args_view& argsview_, std::string_view stringKey)
{
   return argsview_.contains(stringKey);
}

/** -----------------------------------------*/ /**
 * @brief Check if any argument in args has a specific key
 *
 * This function provides a convenient way to check if any argument
 * in an args container has a specific key.
 *
 * @param args_ The args container to search
 * @param stringKey The key to search for
 * @return true if any argument has the specified key, false otherwise
 */
bool has_key(const args& args_, std::string_view stringKey)
{
   return args_.contains(stringKey);
}

/** -----------------------------------------*/ /**
 * @brief Get all keys from args_view
 *
 * This function provides a convenient way to extract all keys
 * from an args_view container.
 *
 * @param argsview_ The args_view container to extract keys from
 * @return vector of string_view containing all keys
 */
std::vector<std::string_view> get_keys(const args_view& argsview_)
{
   std::vector<std::string_view> vectorResult;
   vectorResult.reserve(argsview_.size());
   for(const auto& argview_ : argsview_)
   {
      vectorResult.push_back(argview_.get_key());
   }
   return vectorResult;
}

/** -----------------------------------------*/ /**
 * @brief Get all keys from args
 *
 * This function provides a convenient way to extract all keys
 * from an args container.
 *
 * @param args_ The args container to extract keys from
 * @return vector of strings containing all keys
 */
std::vector<std::string> get_keys(const args& args_)
{
   std::vector<std::string> vectorResult;
   vectorResult.reserve(args_.size());
   for(const auto& arg_ : args_)
   {
      vectorResult.push_back(arg_.get_key());
   }
   return vectorResult;
}

/** -----------------------------------------*/ /**
 * @brief Get all values from args_view
 *
 * This function provides a convenient way to extract all values
 * from an args_view container.
 *
 * @param argsview_ The args_view container to extract values from
 * @return vector of variant_view containing all values
 */
std::vector<variant_view> get_values(const args_view& argsview_)
{
   std::vector<variant_view> vectorResult;
   vectorResult.reserve(argsview_.size());
   for(const auto& argview_ : argsview_)
   {
      vectorResult.push_back(argview_.get_value());
   }
   return vectorResult;
}

/** -----------------------------------------*/ /**
 * @brief Get all values from args
 *
 * This function provides a convenient way to extract all values
 * from an args container.
 *
 * @param args_ The args container to extract values from
 * @return vector of variant containing all values
 */
std::vector<variant> get_values(const args& args_)
{
   std::vector<variant> vectorResult;
   vectorResult.reserve(args_.size());
   for(const auto& arg_ : args_)
   {
      vectorResult.push_back(arg_.get_value());
   }
   return vectorResult;
}

_GD_END