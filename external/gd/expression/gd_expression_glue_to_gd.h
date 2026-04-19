// @FILE [tag: gd, expression, glue] [summary: Glue code between GD expressions and GD framework] [description: This header file contains the glue code that allows GD expressions to interact with the GD framework. It includes functions for converting between GD variants and expression values, as well as utilities for handling named arguments in expressions. This code is essential for integrating the expression evaluation capabilities with the rest of the GD system, enabling seamless data exchange and function invocation within expressions. The functions defined here ensure that data types are correctly handled and converted when used in expressions, facilitating a smooth integration between the expression engine and the underlying GD data structures.] [name: gd_expression_glue_to_gd.h] [type: header]

/**
 * @file gd_expression_glue_to_gd.h
 * 
 * This header file contains the glue code between the GD expressions and the GD framework.
 */

#pragma once
#include <string>
#include <vector>
#include "gd_expression.h"
#include "gd_expression_value.h"

#include "../gd_variant.h"
#include "../gd_variant_view.h"
#include "../gd_arguments.h"

_GD_EXPRESSION_BEGIN 

/** ------------------------------------------------------------------------- to_value_g
 * @brief Converts a gd::variant to a value::value_type.
 * 
 * This function checks the type of the gd::variant and converts it to the corresponding value::value_type.
 * If the type does not match any of the expected types, it returns a nullptr value.
 * 
 * @param value_ The gd::variant to convert.
 * @return The converted value::value_type.
 */
inline value::value_type to_value_g(const gd::variant& value_)
{
   if( value_.is_integer() ) return value::value_type( value_.as_int64() );
   if( value_.is_decimal() ) return value::value_type( value_.as_double() );
   if( value_.is_string() ) return value::value_type( value_.as_string() );
   if( value_.is_bool() ) return value::value_type( value_.as_bool() );
   if( value_.is_binary() ) return value::value_type( value_.as_string() ); // convert binary to string for simplicity
    
   return value::value_type(nullptr); // return nullptr if no type matches
}  

/** ------------------------------------------------------------------------- to_value_g
 * @brief Converts a gd::variant_view to a value::value_type.
 * 
 * This function checks the type of the gd::variant_view and converts it to the corresponding value::value_type.
 * If the type does not match any of the expected types, it returns a nullptr value.
 * 
 * @param value_ The gd::variant_view to convert.
 * @return The converted value::value_type.
 */
inline value::value_type to_value_g( const gd::variant_view& value_ )
{
   if( value_.is_integer() ) return value::value_type( value_.as_int64() );
   if( value_.is_decimal() ) return value::value_type( value_.as_double() );
   if( value_.is_string() ) return value::value_type( value_.as_string() );
   if( value_.is_bool() ) return value::value_type( value_.as_bool() );
   if( value_.is_binary() ) return value::value_type( value_.as_string() ); // convert binary to string for simplicity
   return value::value_type(nullptr); // return nullptr if no type matches
}

/** ------------------------------------------------------------------------- to_named_values_g
 * @brief Converts named arguments to a vector of named values.
 * 
 * @param arguments_ The named arguments to convert.
 * @return A vector of named values.
 */
inline std::vector< std::pair<std::string, value::variant_t>> to_named_values_g( const gd::argument::arguments& arguments_ )
{
   std::vector< std::pair<std::string, value::variant_t>> vectorNamedValue;
   for( const auto& [key_, value_] : arguments_.named() )
   {
      vectorNamedValue.emplace_back( key_, to_value_g( value_.as_variant() ) );
   }
   return vectorNamedValue;
}

_GD_EXPRESSION_END