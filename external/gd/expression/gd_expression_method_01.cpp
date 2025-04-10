#include "gd_expression_method_01.h"

_GD_EXPRESSION_BEGIN 

std::pair<bool, std::string> average_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& l_ = vectorArgument[0]; 
   const auto& r_ = vectorArgument[1];

   if(  l_.is_integer() == true && r_.is_integer() == true )
   {
      *pvalueResult = ( l_.as_integer() + r_.as_integer() ) / 2;
   }
   else if( l_.is_double() == true && r_.is_double() == true )
   {
      *pvalueResult = ( l_.as_double() + r_.as_double() ) / 2.0;
   }
   else
   {
      return { false, "average_g - Invalid argument type" };
   }

   return { true, "" };
}

std::pair<bool, std::string> length_g( const std::vector< value >& vectorArgument, value* pvalueResult )
{                                                                                                  assert( vectorArgument.size() > 0 );
   if( vectorArgument[0].is_string() == false ) { return {false, "length_g - Invalid argument type"}; }

   auto string_ = vectorArgument[0].as_string_view();
   *pvalueResult = static_cast<int64_t>( string_.length() );
 
   return { true, "" };
}

std::pair<bool, std::string> max_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& l_ = vectorArgument[0];
   const auto& r_ = vectorArgument[1];

   if( l_.is_integer() == true && r_.is_integer() == true )
   {
      *pvalueResult = std::max(l_.as_integer(), r_.as_integer());
   }
   else if( l_.is_double() == true && r_.is_double() == true )
   {
      *pvalueResult = std::max(l_.as_double(), r_.as_double());
   }
   else
   {
      return { false, "max_g - Invalid argument type" };
   }
  
   return { true, "" };
}

std::pair<bool, std::string> min_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& l_ = vectorArgument[0];
   const auto& r_ = vectorArgument[1];
   
   if( l_.is_integer() == true && r_.is_integer() == true )
   {
      *pvalueResult = std::min(l_.as_integer(), r_.as_integer());
   }
   else if( l_.is_double() == true && r_.is_double() == true )
   {
      *pvalueResult = std::min(l_.as_double(), r_.as_double());
   }
   else
   {
      return { false, "min_g - Invalid argument type" };
   }

   return { true, "" };
}

std::pair<bool, std::string> sum_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& l_ = vectorArgument[0];
   const auto& r_ = vectorArgument[1];
   
   if( l_.is_integer() == true && r_.is_integer() == true )
   {
      *pvalueResult = l_.as_integer() + r_.as_integer();
   }
   else if( l_.is_double() == true && r_.is_double() == true )
   {
      *pvalueResult = l_.as_double() + r_.as_double();
   }
   else
   {
      return { false, "sum_g - Invalid argument type" };
   }

   return { true, "" };
}

_GD_EXPRESSION_END