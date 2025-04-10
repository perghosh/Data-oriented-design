#include "gd_expression_method_01.h"

_GD_EXPRESSION_BEGIN 

std::pair<bool, std::string> average_g(const std::vector<variant_t>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& l_ = vectorArgument[0]; 
   const auto& r_ = vectorArgument[1];

   if( std::holds_alternative<int64_t>( l_ ) == true && std::holds_alternative<int64_t>( r_ ) == true )
   {
      *pvalueResult = ( std::get<int64_t>( l_ ) + std::get<int64_t>( r_ ) ) / 2;
   }
   else if( std::holds_alternative<double>( l_ ) == true && std::holds_alternative<double>( r_ ) == true )
   {
      *pvalueResult = ( std::get<double>( l_ ) + std::get<double>( r_ ) ) / 2.0;
   }
   else
   {
      return { false, "average_g - Invalid argument type" };
   }

   return { true, "" };
}

std::pair<bool, std::string> length_g( const std::vector< variant_t>& vectorArgument, value* pvalueResult )
{                                                                                                  assert( vectorArgument.size() > 0 );
   if( std::holds_alternative<std::string_view>(vectorArgument[0]) == false ) { return { false, "length_g - Invalid argument type" }; }

   auto string_ = std::get<std::string_view>(vectorArgument[0]);
   *pvalueResult = static_cast<int64_t>( string_.length() );
 
   return { true, "" };
}

std::pair<bool, std::string> max_g(const std::vector<variant_t>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& l_ = vectorArgument[0];
   const auto& r_ = vectorArgument[1];

   if( std::holds_alternative<int64_t>(l_) == true && std::holds_alternative<int64_t>(r_) == true )
   {
      *pvalueResult = std::max(std::get<int64_t>(l_), std::get<int64_t>(r_));
   }
   else if( std::holds_alternative<double>(l_) == true && std::holds_alternative<double>(r_) == true )
   {
      *pvalueResult = std::max(std::get<double>(l_), std::get<double>(r_));
   }
   else
   {
      return { false, "max_g - Invalid argument type" };
   }
  
   return { true, "" };
}

std::pair<bool, std::string> min_g(const std::vector<variant_t>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& l_ = vectorArgument[0];
   const auto& r_ = vectorArgument[1];
   
   if( std::holds_alternative<int64_t>(l_) == true && std::holds_alternative<int64_t>(r_) == true )
   {
      *pvalueResult = std::min(std::get<int64_t>(l_), std::get<int64_t>(r_));
   }
   else if( std::holds_alternative<double>(l_) == true && std::holds_alternative<double>(r_) == true )
   {
      *pvalueResult = std::min(std::get<double>(l_), std::get<double>(r_));
   }
   else
   {
      return { false, "min_g - Invalid argument type" };
   }

   return { true, "" };
}

std::pair<bool, std::string> sum_g(const std::vector<variant_t>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& l_ = vectorArgument[0];
   const auto& r_ = vectorArgument[1];
   
   if( std::holds_alternative<int64_t>(l_) == true && std::holds_alternative<int64_t>(r_) == true )
   {
      *pvalueResult = std::get<int64_t>(l_) + std::get<int64_t>(r_);
   }
   else if( std::holds_alternative<double>(l_) == true && std::holds_alternative<double>(r_) == true )
   {
      *pvalueResult = std::get<double>(l_) + std::get<double>(r_);
   }
   else
   {
      return { false, "sum_g - Invalid argument type" };
   }

   return { true, "" };
}

_GD_EXPRESSION_END