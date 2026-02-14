// @FILE [tag: convert] [description: convert between core data objects used in http project] ] [type: source] [name: CONVERTCore.cpp]
 

#include "CONVERTCore.h"

namespace CONVERT { 

gd::variant AsVariant( const jsonpath::json& json )
{
    if( json.is_null() )            return gd::variant();
    else if( json.is_boolean() )    return gd::variant( json.as_bool() );
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
       if( json.size() > 0 )        return AsVariant( json.begin()->second );
    }

    return gd::variant();
}

gd::variant_view AsVariantView( const jsonpath::json& json )
{
   if( json.is_null() )            return gd::variant_view();
   else if( json.is_boolean() )    return gd::variant_view( json.as_bool() );
   else if( json.is_number() )     return gd::variant_view( json.as_double() );
   else if( json.is_string() )     return gd::variant_view( json.as_string() );
   else if( json.is_array() )
   {
      // ## only first value ................................................
      if( json.size() > 0 )        return Asvariant_view( json[0] );
   }
   else if( json.is_object() )
   {
      // ## only first value ................................................
      if( json.size() > 0 )        return Asvariant_view( json.begin()->second );
   }

   return gd::variant_view();
}

}
