// @FILE [tag: convert] [description: convert between core data objects used in http project] ] [type: source] [name: CONVERTCore.cpp]
 

#include "CONVERTCore.h"

namespace CONVERT { 

/** --------------------------------------------------------------------------
 * @brief Convert json to variant
 * @param json The json object to convert
 * @return A variant representing the json value
 **/
gd::variant AsVariant( const jsoncons::json& json )
{
    if( json.is_null() )            return gd::variant();
    else if( json.is_bool() )       return gd::variant( json.as_bool() );
    else if( json.is_int64() )      return gd::variant( json.as<int64_t>() );
    else if( json.is_uint64() )     return gd::variant( json.as<uint64_t>() );
    else if( json.is_number() )     return gd::variant( json.as_double() );
    else if( json.is_string() )     return gd::variant( json.as_string() );
    else if( json.is_array() )
    {
       // ## only first value ................................................
       if( json.size() > 0 )        return AsVariant( json[0] );
    }
    else if( json.is_object() )
    {
       // ## only first value ................................................
       if( json.size() > 0 )        return AsVariant( json.object_range().begin()->value() );
    }

    return gd::variant();
}

/** --------------------------------------------------------------------------
 * @brief Convert json to variant view
 * @param json The json object to convert
 * @return A variant view representing the json value
 */
gd::variant_view AsVariantView( const jsoncons::json& json )
{
   if( json.is_null() )            return gd::variant_view();
   else if( json.is_bool() )       return gd::variant_view( json.as_bool() );
   else if( json.is_int64() )      return gd::variant_view( json.as<int64_t>() );
   else if( json.is_uint64() )     return gd::variant_view( json.as<uint64_t>() );
   else if( json.is_number() )     return gd::variant_view( json.as_double() );
   else if( json.is_string() )     return gd::variant_view( json.as_string() );
   else if( json.is_array() )
   {
      // ## only first value ................................................
      if( json.size() > 0 )        return AsVariantView( json[0] );
   }
   else if( json.is_object() )
   {
      // ## only first value ................................................
      if( json.size() > 0 )        return AsVariantView( json.object_range().begin()->value() );
   }
   

   return gd::variant_view();
}

}
