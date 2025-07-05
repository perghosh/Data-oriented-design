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

_GD_EXPRESSION_BEGIN 

/** ---------------------------------------------------------------------------
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
    
   return value::value_type(nullptr); // return nullptr if no type matches
}  

/** ---------------------------------------------------------------------------
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
    
   return value::value_type(nullptr); // return nullptr if no type matches
}

_GD_EXPRESSION_END