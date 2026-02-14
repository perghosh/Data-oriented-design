// @FILE [tag: convert] [description: convert between core data objects used in http project] ] [type: source] [name: CONVERTCore.cpp]
 

#include "CONVERTCore.h"

namespace CONVERT { 

gd::variant AsVariant( const jsoncons::json& json )
{
    if( json.is_null() )            return gd::variant();
    else if( json.is_bool() )    return gd::variant( json.as_bool() );
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

gd::variant_view AsVariantView( const jsoncons::json& json )
{
   if( json.is_null() )            return gd::variant_view();
   else if( json.is_bool() )    return gd::variant_view( json.as_bool() );
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

/** --------------------------------------------------------------------------
 * @brief Convert a string representation of a type used in databases to to a gd::types::enumType
 * 
 * Database types are things like varchar, char, binary, uniqueidentifier, etc. that are used in 
 * databases to define the type of a column. This function takes a string representation of such a 
 * type and converts it to the corresponding gd::types::enumType value, which is an enumeration used 
 * in the codebase to represent different data types.
 * 
 * @param stringType The string representation of the type (e.g., "int", "string", etc.)
 * @return The corresponding gd::types::enumType value
 */
gd::types::enumType DatabaseTypeToGdType( std::string_view stringType )
{
   // extract the base type name by taking characters until a non-alphabetic character (e.g., '(', space, etc.) and convert to lower case for case-insensitive comparison
   std::string stringLower;
   for(char i : stringType) {
      if(std::isalpha(static_cast<unsigned char>(i))) {
         stringLower += static_cast<char>(std::tolower(static_cast<unsigned char>(i)));
      } 
      else { break; }
   }
   std::string_view sv_ = stringLower;

   using namespace gd::types;
   using namespace gd::types::detail;

   uint64_t uHash = hash_type64(sv_);
   switch(uHash)
   {
   case hash_type64("bigint"): return eTypeInt64;
   case hash_type64("binary"): return eTypeBinary;
   case hash_type64("bit"): return eTypeBit;
   case hash_type64("blob"): return eTypeBinary;
   case hash_type64("char"): return eTypeString;
   case hash_type64("date"): return eTypeCDouble;
   case hash_type64("datetime"): return eTypeCDouble;
   case hash_type64("decimal"): return eTypeCDouble;
   case hash_type64("double"): return eTypeCDouble;
   case hash_type64("float"): return eTypeCFloat;
   case hash_type64("image"): return eTypeBinary;
   case hash_type64("int"): return eTypeInt32;
   case hash_type64("integer"): return eTypeInt32;
   case hash_type64("json"): return eTypeJson;
   case hash_type64("nchar"): return eTypeWString;
   case hash_type64("ntext"): return eTypeWString;
   case hash_type64("numeric"): return eTypeCDouble;
   case hash_type64("nvarchar"): return eTypeWString;
   case hash_type64("real"): return eTypeCFloat;
   case hash_type64("smallint"): return eTypeInt16;
   case hash_type64("text"): return eTypeString;
   case hash_type64("time"): return eTypeCDouble;
   case hash_type64("tinyint"): return eTypeInt8;
   case hash_type64("uniqueidentifier"): return eTypeGuid;
   case hash_type64("uuid"): return eTypeGuid;
   case hash_type64("varbinary"): return eTypeBinary;
   case hash_type64("varchar"): return eTypeString;
   case hash_type64("xml"): return eTypeXml;
   default: return eTypeUnknown;
   }
}

}
