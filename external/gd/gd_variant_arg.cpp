// @FILE [tag: variant, arg] [description: Implementation for argument objects] [type: source]

#include "gd_variant_arg.h"

_GD_BEGIN


// ## Factory functions for arg_view with primitive values


arg_view make_arg_view(std::string_view stringKey, bool bValue)
{
   return arg_view(stringKey, variant_view(bValue));
}

arg_view make_arg_view(std::string_view stringKey, int32_t iValue)
{
   return arg_view(stringKey, variant_view(iValue));
}

arg_view make_arg_view(std::string_view stringKey, uint32_t uValue)
{
   return arg_view(stringKey, variant_view(uValue));
}

arg_view make_arg_view(std::string_view stringKey, int64_t iValue)
{
   return arg_view(stringKey, variant_view(iValue));
}

arg_view make_arg_view(std::string_view stringKey, uint64_t uValue)
{
   return arg_view(stringKey, variant_view(uValue));
}

arg_view make_arg_view(std::string_view stringKey, double dValue)
{
   return arg_view(stringKey, variant_view(dValue));
}

arg_view make_arg_view(std::string_view stringKey, const char* pszValue)
{
   return arg_view(stringKey, variant_view(pszValue));
}


_GD_END