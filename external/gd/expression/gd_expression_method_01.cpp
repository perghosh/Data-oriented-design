#include <algorithm>

#include "gd_expression_method_01.h"

_GD_EXPRESSION_BEGIN 

//============================================================================
//============================================================ default methods
//============================================================================


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
const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) { return {false, "length_g - Invalid argument type"}; }

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

//============================================================================
//============================================================= string methods
//============================================================================

/// Converts the first argument to lower case and returns it as a string.
std::pair<bool, std::string> tolower_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);                       
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) 
   { 
      if( v_.is_null() == true ) 
      {
         *pvalueResult = v_;                                                  // do nothing for null values
         return { true, "" };
      }
      return {false, "tolower_g - Invalid argument type"}; 
   }

   std::string stringResult = v_.as_string();
   std::transform(stringResult.begin(), stringResult.end(), stringResult.begin(), [](unsigned char i) { return std::tolower(i); });

   *pvalueResult = stringResult;

   return { true, "" };
}

/// Converts the first argument to upper case and returns it as a string.
std::pair<bool, std::string> toupper_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);                       
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) 
   { 
      if( v_.is_null() == true ) 
      {
         *pvalueResult = v_;                                                  // do nothing for null values
         return { true, "" };
      }
      return {false, "toupper_g - Invalid argument type"}; 
   }

   std::string stringResult = v_.as_string();
   std::transform(stringResult.begin(), stringResult.end(), stringResult.begin(), [](unsigned char i) { return std::toupper(i); });

   *pvalueResult = stringResult;

   return { true, "" };
}

/// Count the number of occurrences of a word (needle) in text (haystack).
std::pair<bool, std::string> count_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& haystack_ = vectorArgument[1];
   const auto& needle_ = vectorArgument[0];
   if( haystack_.is_string() && needle_.is_string() )
   {
      auto stringText = haystack_.as_string_view();
      auto stringWord = needle_.as_string_view();
      size_t uCount = 0;
      size_t uPosition = stringText.find(stringWord);
      while(uPosition != std::string_view::npos)
      {
         ++uCount;
         uPosition = stringText.find(stringWord, uPosition + stringWord.size());
      }
      *pvalueResult = static_cast<int64_t>(uCount);
      return { true, "" };
   }

   return { false, "count_g - Invalid argument type" };
}

/// Check if word (needle) is contained in text (haystack).
std::pair<bool, std::string> has_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& haystack_ = vectorArgument[1];
   const auto& needle_ = vectorArgument[0];
   if( haystack_.is_string() && needle_.is_string() )
   {
      auto text_ = haystack_.as_string_view();
      auto word_ = needle_.as_string_view();
      *pvalueResult = (text_.find(word_) != std::string_view::npos);
      return { true, "" };
   }
   return { false, "has_g - Invalid argument type" };
}

/// Check if word (needle) is not contained in text (haystack).
std::pair<bool, std::string> missing_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& haystack_ = vectorArgument[1];
   const auto& needle_ = vectorArgument[0];
   if( haystack_.is_string() && needle_.is_string() )
   {
      auto text_ = haystack_.as_string_view();
      auto word_ = needle_.as_string_view();
      *pvalueResult = (text_.find(word_) == std::string_view::npos);
      return { true, "" };
   }
   return { false, "missing_g - Invalid argument type" };
}

/// Check if text (haystack) starts with prefix (start).
std::pair<bool, std::string> starts_with_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& haystack_ = vectorArgument[1];
   const auto& prefix_ = vectorArgument[0];
   if( haystack_.is_string() && prefix_.is_string() )
   {
      auto text_ = haystack_.as_string_view();
      auto start_ = prefix_.as_string_view();
      *pvalueResult = (text_.substr(0, start_.size()) == start_);
      return { true, "" };
   }
   return { false, "starts_with_g - Invalid argument type" };
}

/// Check if text (haystack) ends with suffix (end).
std::pair<bool, std::string> ends_with_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& haystack_ = vectorArgument[1];
   const auto& suffix_ = vectorArgument[0];
   if( haystack_.is_string() && suffix_.is_string() )
   {
      auto text_ = haystack_.as_string_view();
      auto end_ = suffix_.as_string_view();
      if (end_.size() > text_.size())
      {
         *pvalueResult = false;
      }
      else
      {
         *pvalueResult = (text_.substr(text_.size() - end_.size(), end_.size()) == end_);
      }
      return { true, "" };
   }
   return { false, "ends_with_g - Invalid argument type" };
}



_GD_EXPRESSION_END