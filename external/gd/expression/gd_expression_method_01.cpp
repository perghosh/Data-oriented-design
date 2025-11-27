#include <algorithm>
#include <cmath>

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

/// Return the length of a string.
std::pair<bool, std::string> length_g( const std::vector< value >& vectorArgument, value* pvalueResult )
{                                                                                                  assert( vectorArgument.size() > 0 );
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) { return {false, "length_g - Invalid argument type"}; }

   auto string_ = vectorArgument[0].as_string_view();
   *pvalueResult = static_cast<int64_t>( string_.length() );
 
   return { true, "" };
}

/// Return the maximum of two numbers.
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

/// Returns the minimum of two values.
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

/// Sum two numbers and return the result.
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

/// Calculate absolute value
std::pair<bool, std::string> abs_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   if( v_.is_integer() )
   {
      int64_t iValue = v_.as_integer();
      *pvalueResult = (iValue < 0) ? -iValue : iValue;
   }
   else if( v_.is_double() )
   {
      double dValue = v_.as_double();
      *pvalueResult = (dValue < 0.0) ? -dValue : dValue;
   }
   else
   {
      return { false, "abs_g - Invalid argument type" };
   }
   
   return { true, "" };
}

/// Round number to nearest integer or specified decimal places
std::pair<bool, std::string> round_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   int64_t iDecimals = 0;
   if( vectorArgument.size() > 1 && vectorArgument[1].is_integer() )
   {
      iDecimals = vectorArgument[1].as_integer();
   }
   
   if( v_.is_integer() )
   {
      *pvalueResult = v_.as_integer();
   }
   else if( v_.is_double() )
   {
      double dValue = v_.as_double();
      
      if( iDecimals == 0 )
      {
         *pvalueResult = static_cast<int64_t>(std::round(dValue));
      }
      else
      {
         double dMultiplier = std::pow(10.0, static_cast<double>(iDecimals));
         *pvalueResult = std::round(dValue * dMultiplier) / dMultiplier;
      }
   }
   else
   {
      return { false, "round_g - Invalid argument type" };
   }
   
   return { true, "" };
}

/// Calculate floor of number
std::pair<bool, std::string> floor_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];

   if( v_.is_integer() ) { *pvalueResult = v_.as_integer(); }
   else if( v_.is_double() ) { *pvalueResult = static_cast<int64_t>(std::floor(v_.as_double())); }
   else { return { false, "floor_g - Invalid argument type" }; }
   
   return { true, "" };
}

/// Calculate ceiling of number
std::pair<bool, std::string> ceil_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   if( v_.is_integer() ) { *pvalueResult = v_.as_integer(); }
   else if( v_.is_double() ) { *pvalueResult = static_cast<int64_t>(std::ceil(v_.as_double())); }
   else { return { false, "ceil_g - Invalid argument type" }; }
   
   return { true, "" };
}

/// Conditional: returns second argument if first is true, third argument otherwise
std::pair<bool, std::string> if_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& condition_ = vectorArgument[2];
   const auto& true_value_ = vectorArgument[1];
   const auto& false_value_ = vectorArgument[0];
   
   if( condition_.is_bool() == false ) { return {false, "if_g - First argument must be boolean"}; }
   
   *pvalueResult = condition_.as_bool() ? true_value_ : false_value_;
   return { true, "" };
}

/// Check if value is null
std::pair<bool, std::string> is_null_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   *pvalueResult = v_.is_null();
   return { true, "" };
}

/// Check if value is not null
std::pair<bool, std::string> is_not_null_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   *pvalueResult = (v_.is_null() == false);
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

/// Find the position of the first occurrence of word (needle) in text (haystack). Returns -1 if not found.
/// takes three arguments, the third argument is the offset to start searching from (default is 0)
std::pair<bool, std::string> find_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& text_ = vectorArgument[2];
   const auto& word_ = vectorArgument[1];
   const auto& offset_ = vectorArgument[0];

   if( offset_.is_integer() && text_.is_string() && word_.is_string() )
   {
      auto uOffset = static_cast<size_t>( std::max(static_cast<int64_t>(offset_.as_integer()), int64_t{0}) );
      auto stringText = text_.as_string_view();
      auto stringWord = word_.as_string_view();
      auto uPosition = stringText.find(stringWord, uOffset);
      *pvalueResult = (uPosition != std::string_view::npos) ? static_cast<int64_t>(uPosition) : -1;
      return { true, "" };
   }

   return { false, "find_g - Invalid argument type" };
}

/// Check if word (needle) is contained in text (haystack).
std::pair<bool, std::string> has_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& haystack_ = vectorArgument[1];
   const auto& needle_ = vectorArgument[0];
   if( haystack_.is_string() && needle_.is_string() )
   {
      auto stringText = haystack_.as_string_view();
      auto stringWord = needle_.as_string_view();
      *pvalueResult = (stringText.find(stringWord) != std::string_view::npos);
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

// Helper function to extract tags from a string
// A tag consists of alphanumeric characters, hyphens, and underscores.
// Returns a vector of strings, where each string is a tag.
std::vector<std::string> extract_tags(std::string_view text) 
{
   std::vector<std::string> vectorTag;
   std::string stringTag;

   for(char c : text) 
   {
      if( std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' ) { stringTag += c;} 
      else 
      {
         if( stringTag.empty() == false) 
         {
            vectorTag.push_back(stringTag);
            stringTag.clear();
         }
      }
   }

   if(stringTag.empty() == false) { vectorTag.push_back(stringTag); }
   return vectorTag;
}

/// Trim whitespace from both ends of string
std::pair<bool, std::string> trim_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) 
   { 
      if( v_.is_null() == true ) 
      {
         *pvalueResult = v_;
         return { true, "" };
      }
      return {false, "trim_g - Invalid argument type"}; 
   }

   auto stringText = v_.as_string_view();
   size_t uStart = 0;
   size_t uEnd = stringText.length();
   
   while( uStart < uEnd && std::isspace(static_cast<unsigned char>(stringText[uStart])) ) { ++uStart; }
   while( uEnd > uStart && std::isspace(static_cast<unsigned char>(stringText[uEnd - 1])) ) { --uEnd; }
   
   *pvalueResult = std::string(stringText.substr(uStart, uEnd - uStart));
   return { true, "" };
}

/// Trim whitespace from left side of string
std::pair<bool, std::string> ltrim_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) 
   { 
      if( v_.is_null() == true ) 
      {
         *pvalueResult = v_;
         return { true, "" };
      }
      return {false, "ltrim_g - Invalid argument type"}; 
   }

   auto stringText = v_.as_string_view();
   size_t uStart = 0;
   
   while( uStart < stringText.length() && std::isspace(static_cast<unsigned char>(stringText[uStart])) ) { ++uStart; }
   
   *pvalueResult = std::string(stringText.substr(uStart));
   return { true, "" };
}

/// Trim whitespace from right side of string
std::pair<bool, std::string> rtrim_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) 
   { 
      if( v_.is_null() == true ) 
      {
         *pvalueResult = v_;
         return { true, "" };
      }
      return {false, "rtrim_g - Invalid argument type"}; 
   }

   auto stringText = v_.as_string_view();
   size_t uEnd = stringText.length();
   
   while( uEnd > 0 && std::isspace(static_cast<unsigned char>(stringText[uEnd - 1])) ) { --uEnd; }
   
   *pvalueResult = std::string(stringText.substr(0, uEnd));
   return { true, "" };
}

/// Extract substring from text starting at position with given length
std::pair<bool, std::string> substring_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& text_ = vectorArgument[2];
   const auto& start_ = vectorArgument[1];
   const auto& length_ = vectorArgument[0];
   
   if( text_.is_string() && start_.is_integer() && length_.is_integer() )
   {
      auto stringText = text_.as_string_view();
      auto uStart = static_cast<size_t>(std::max(static_cast<int64_t>(0), start_.as_integer()));
      auto uLength = static_cast<size_t>(std::max(static_cast<int64_t>(0), length_.as_integer()));
      
      if( uStart >= stringText.length() )
      {
         *pvalueResult = std::string("");
      }
      else
      {
         *pvalueResult = std::string(stringText.substr(uStart, uLength));
      }
      return { true, "" };
   }
   return { false, "substring_g - Invalid argument type" };
}

/// Replace all occurrences of search string with replacement string
std::pair<bool, std::string> replace_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& text_ = vectorArgument[2];
   const auto& search_ = vectorArgument[1];
   const auto& replace_ = vectorArgument[0];
   
   if( text_.is_string() && search_.is_string() && replace_.is_string() )
   {
      std::string stringResult = text_.as_string();
      auto stringSearch = search_.as_string_view();
      auto stringReplace = replace_.as_string_view();
      
      if( stringSearch.empty() == true )
      {
         *pvalueResult = stringResult;
         return { true, "" };
      }
      
      size_t uPosition = 0;
      while( (uPosition = stringResult.find(stringSearch, uPosition)) != std::string::npos )
      {
         stringResult.replace(uPosition, stringSearch.length(), stringReplace);
         uPosition += stringReplace.length();
      }
      
      *pvalueResult = stringResult;
      return { true, "" };
   }
   return { false, "replace_g - Invalid argument type" };
}

/// Reverse the characters in a string
std::pair<bool, std::string> reverse_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   if( v_.is_string() == false ) 
   { 
      if( v_.is_null() == true ) 
      {
         *pvalueResult = v_;
         return { true, "" };
      }
      return {false, "reverse_g - Invalid argument type"}; 
   }

   std::string stringResult = v_.as_string();
   std::reverse(stringResult.begin(), stringResult.end());
   
   *pvalueResult = stringResult;
   return { true, "" };
}

/// Repeat string N times
std::pair<bool, std::string> repeat_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& text_ = vectorArgument[1];
   const auto& count_ = vectorArgument[0];
   
   if( text_.is_string() && count_.is_integer() )
   {
      auto stringText = text_.as_string();
      auto iCount = count_.as_integer();
      
      if( iCount < 0 )
      {
         return { false, "repeat_g - Count cannot be negative" };
      }
      
      std::string stringResult;
      stringResult.reserve(stringText.length() * static_cast<size_t>(iCount));
      
      for( int64_t i = 0; i < iCount; ++i )
      {
         stringResult += stringText;
      }
      
      *pvalueResult = stringResult;
      return { true, "" };
   }
   return { false, "repeat_g - Invalid argument type" };
}

/// Check if string represents a numeric value
std::pair<bool, std::string> is_numeric_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   if( v_.is_string() == false ) { *pvalueResult = false; return { true, "" }; }

   auto stringText = v_.as_string_view();
   if( stringText.empty() == true ) { *pvalueResult = false; return { true, "" }; }
   
   size_t uPos = 0;
   if( stringText[0] == '-' || stringText[0] == '+' ) { ++uPos; }
   
   bool bHasDigit = false;
   bool bHasDot = false;
   
   for( ; uPos < stringText.length(); ++uPos )
   {
      if( std::isdigit(static_cast<unsigned char>(stringText[uPos])) )
      {
         bHasDigit = true;
      }
      else if( stringText[uPos] == '.' && bHasDot == false )
      {
         bHasDot = true;
      }
      else
      {
         *pvalueResult = false;
         return { true, "" };
      }
   }
   
   *pvalueResult = bHasDigit;
   return { true, "" };
}

/// Check if string contains only alphabetic characters
std::pair<bool, std::string> is_alpha_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   if( v_.is_string() == false ) { *pvalueResult = false; return { true, "" }; }

   auto stringText = v_.as_string_view();
   if( stringText.empty() == true ) { *pvalueResult = false; return { true, "" }; }
   
   for( char i : stringText )
   {
      if( std::isalpha(static_cast<unsigned char>(i)) == false )
      {
         *pvalueResult = false;
         return { true, "" };
      }
   }
   
   *pvalueResult = true;
   return { true, "" };
}

/// Check if string is empty or contains only whitespace
std::pair<bool, std::string> is_empty_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& v_ = vectorArgument[0];
   
   if( v_.is_null() == true ) { *pvalueResult = true; return { true, "" }; }
   
   if( v_.is_string() == false ) { *pvalueResult = false; return { true, "" }; }

   auto stringText = v_.as_string_view();
   if( stringText.empty() == true ) { *pvalueResult = true; return { true, "" }; }
   
   for( char c : stringText )
   {
      if( std::isspace(static_cast<unsigned char>(c)) == false )
      {
         *pvalueResult = false;
         return { true, "" };
      }
   }
   
   *pvalueResult = true;
   return { true, "" };
}

/// Get character at specific position in string
std::pair<bool, std::string> char_at_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& text_ = vectorArgument[1];
   const auto& position_ = vectorArgument[0];
   
   if( text_.is_string() && position_.is_integer() )
   {
      auto stringText = text_.as_string_view();
      auto iPosition = position_.as_integer();
      
      if( iPosition < 0 || static_cast<size_t>(iPosition) >= stringText.length() )
      {
         *pvalueResult = std::string("");
      }
      else
      {
         *pvalueResult = std::string(1, stringText[static_cast<size_t>(iPosition)]);
      }
      return { true, "" };
   }
   return { false, "char_at_g - Invalid argument type" };
}

/// Get N characters from left side of string
std::pair<bool, std::string> left_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& text_ = vectorArgument[1];
   const auto& count_ = vectorArgument[0];
   
   if( text_.is_string() && count_.is_integer() )
   {
      auto stringText = text_.as_string_view();
      auto iCount = static_cast<size_t>(std::max(static_cast<int64_t>(0), count_.as_integer()));
      
      *pvalueResult = std::string(stringText.substr(0, std::min(iCount, stringText.length())));
      return { true, "" };
   }
   return { false, "left_g - Invalid argument type" };
}

/// Get N characters from right side of string
std::pair<bool, std::string> right_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& text_ = vectorArgument[1];
   const auto& count_ = vectorArgument[0];
   
   if( text_.is_string() && count_.is_integer() )
   {
      auto stringText = text_.as_string_view();
      auto iCount = static_cast<size_t>(std::max(static_cast<int64_t>(0), count_.as_integer()));
      
      if( iCount >= stringText.length() )
      {
         *pvalueResult = std::string(stringText);
      }
      else
      {
         *pvalueResult = std::string(stringText.substr(stringText.length() - iCount, iCount));
      }
      return { true, "" };
   }
   return { false, "right_g - Invalid argument type" };
}

/// Extract substring 
std::pair<bool, std::string> mid_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& text_ = vectorArgument[2];
   const auto& start_ = vectorArgument[1];
   const auto& length_ = vectorArgument[0];
   
   if( text_.is_string() && start_.is_integer() && length_.is_integer() )
   {
      auto stringText = text_.as_string_view();
      auto iStart = start_.as_integer();
      auto iLength = length_.as_integer();
      
      if( iStart < 0 || iLength < 0 ) { *pvalueResult = std::string(""); }
      else
      {
         auto uStart = static_cast<size_t>(iStart);
         auto uLength = static_cast<size_t>(iLength);
         
         if( uStart >= stringText.length() ) { *pvalueResult = std::string(""); }
         else { *pvalueResult = std::string(stringText.substr(uStart, uLength)); }
      }
      return { true, "" };
   }
   return { false, "mid_g - Invalid argument type" };
}

namespace detail
{

   // Helper function to extract tags from a string_view
   // A tag consists of alphanumeric characters, hyphens, and underscores.
   // Returns a vector of string_views, where each string_view refers to a tag
   // within the original 'text' string_view.
   //
   // IMPORTANT: The returned std::vector<std::string_view> is only valid as long
   // as the original 'text' string_view (and its underlying string data) remains
   // alive and unchanged.
   std::vector<std::string_view> read_tags(std::string_view stringText)
   {
      std::vector<std::string_view> vectorTagView;
      size_t uTagStart = std::string_view::npos; // Sentinel value to indicate no tag currently being tracked

      for(size_t u = 0; u < stringText.length(); ++u)
      {
         char iCharacter = stringText[u];

         if (std::isalnum(static_cast<unsigned char>(iCharacter)) || iCharacter == '-' || iCharacter == '_')
         {
            // If we are not currently tracking a tag, start one
            if (uTagStart == std::string_view::npos)
            {
               uTagStart = u;
            }
         }
         else // Non-tag character (delimiter)
         {
            // If a tag was being tracked, add it to the vector
            if (uTagStart != std::string_view::npos)
            {
               vectorTagView.push_back(stringText.substr(uTagStart, u - uTagStart));
               uTagStart = std::string_view::npos; // Reset tag tracking
            }
         }
      }

      // After the loop, check if there's an unfinished tag at the end of the string
      if (uTagStart != std::string_view::npos)
      {
         vectorTagView.push_back(stringText.substr(uTagStart));
      }

      return vectorTagView;
   }
}

/// Check if tag (needle) is contained in the tags of text (haystack).
std::pair<bool, std::string> has_tag_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& tag_ = vectorArgument[0];
   const auto& text_ = vectorArgument[1];
   if( text_.is_string() && tag_.is_string() )
   {
      auto stringText = text_.as_string_view();
      auto stringTag = tag_.as_string_view();
      auto vectorText = detail::read_tags(stringText);
      auto vectorTags = detail::read_tags(stringTag);
      for(const auto& tagText : vectorText)
      {
         for( const auto& tag_ : vectorTags )
         {
            if(tagText == tag_)
            {
               *pvalueResult = true;
               return { true, "" };
            }
         }
      }

      *pvalueResult = false;
      return { true, "" };
   }
   else if( (text_.is_null() == true) || (tag_.is_null() == true) )
   {
      *pvalueResult = false;
      return { true, "" };
   }
   return { false, "has_tag_g - Invalid argument type" };
}

/// Returns a comma-separated list of unique tags from text.
std::pair<bool, std::string> list_tags_g(const std::vector< value >& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 0);
   const auto& text_ = vectorArgument[0];
   if( text_.is_string() )
   {
      auto stringText = text_.as_string_view();
      auto vectorTags = detail::read_tags(stringText);
      if(vectorTags.empty() == true) { *pvalueResult = std::string(""); return { true, "" }; }
      std::sort(vectorTags.begin(), vectorTags.end());
      auto it = std::unique(vectorTags.begin(), vectorTags.end());
      vectorTags.erase(it, vectorTags.end());
      std::string stringResult;
      for(size_t i = 0; i < vectorTags.size(); ++i)
      {
         if(i > 0) stringResult += ",";
         stringResult += std::string(vectorTags[i]);
      }
      *pvalueResult = stringResult;
      return { true, "" };
   }
   return { false, "list_tags_g - Invalid argument type" };
}



_GD_EXPRESSION_END