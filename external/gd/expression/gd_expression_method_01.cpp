#include <algorithm>
#include <cmath>

#include "gd/gd_binary.h"
#include "gd_expression_method_01.h"

_GD_EXPRESSION_BEGIN 

//============================================================================
//============================================================ default methods
//============================================================================


/// 
std::pair<bool, std::string> average_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "average requires at least 1 argument" };
   double dSum = 0.0;
   for( const auto& v : vectorArgument ) { dSum += v.as_double(); }
   *pvalueResult = value(dSum / static_cast<double>(vectorArgument.size()));
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


/// Return the maximum of multiple values. If the first argument is a string, all arguments will be treated
/// as strings and the lexicographically maximum string will be returned. Otherwise, all arguments will be
/// treated as numbers and the maximum number will be returned.
std::pair<bool, std::string> max_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "max requires at least 1 argument" };

   if( vectorArgument.front().is_string() == true )
   {
      // ## Find lexicographically maximum string
      std::string stringMax = vectorArgument[0].as_string();
      for( size_t i = 1; i < vectorArgument.size(); ++i )
      {
         std::string stringCurrent = vectorArgument[i].as_string();
         if( stringCurrent > stringMax ) { stringMax = stringCurrent; }
      }
      *pvalueResult = value(stringMax);
   }
   else
   {
      // ## Find maximum number
      double dMax = vectorArgument[0].as_double();
      for( size_t i = 1; i < vectorArgument.size(); ++i )
      {
         double d_ = vectorArgument[i].as_double();
         if( d_ > dMax ) { dMax = d_; }
      }
      *pvalueResult = value(dMax);
   }
   
   return { true, "" };
}

/// Return the minimum of multiple values. If the first argument is a string, all arguments will be treated
/// as strings and the lexicographically minimum string will be returned. Otherwise, all arguments will be
/// treated as numbers and the minimum number will be returned.
std::pair<bool, std::string> min_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "min requires at least 1 argument" };

   // Check first value if it is string or number
   if( vectorArgument.front().is_string() == true )
   {
      // Find lexicographically minimum string
      std::string stringMin = vectorArgument[0].as_string();
      for( size_t i = 1; i < vectorArgument.size(); ++i )
      {
         std::string stringCurrent = vectorArgument[i].as_string();
         if( stringCurrent < stringMin ) { stringMin = stringCurrent; }
      }
      *pvalueResult = value(stringMin);
   }
   else
   {
      // ## Find minimum number
      double dMin = vectorArgument[0].as_double();
      for( size_t i = 1; i < vectorArgument.size(); ++i )
      {
         double d_ = vectorArgument[i].as_double();
         if( d_ < dMin ) { dMin = d_; }
      }
      *pvalueResult = value(dMin);
   }
   
   return { true, "" };
}

/// Return true if all arguments evaluate to true (logical AND across all arguments).
/// Arguments are treated as booleans according to the value's truthiness.
std::pair<bool, std::string> all_true_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "all_true requires at least 1 argument" };

   for( const auto& v : vectorArgument )
   {
      bool bValue = v.as_bool(); // Convert value to boolean using its truthiness
      if( bValue == false )
      {
         *pvalueResult = value(false);
         return { true, "" };
      }
   }
   *pvalueResult = value(true);
   return { true, "" };
}

/// Return true if any argument evaluates to true (logical OR across all arguments).
/// Arguments are treated as booleans according to the value's truthiness.
std::pair<bool, std::string> any_true_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "any_true requires at least 1 argument" };

   for( const auto& v : vectorArgument )
   {
      bool bValue = v.as_bool(); // Convert value to boolean using its truthiness
      if( bValue == true )
      {
         *pvalueResult = value(true);
         return { true, "" };
      }
   }
   *pvalueResult = value(false);
   return { true, "" };
}

/// Sum multiple numbers or concatenate strings and return the result. If the first argument is a
/// string, all arguments will be treated as strings and concatenated. Otherwise, all arguments
/// will be treated as numbers and summed.
std::pair<bool, std::string> sum_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "sum requires at least 1 argument" };

   // ## Check first value if it is string or number
   if( vectorArgument.front().is_string() == true )                           // if first value is string, we will concatenate all values as strings, otherwise we will sum them as numbers
   {
      std::string stringResult;
      // ## Concatenate all values as strings, note that vector is in reverse order, so we will concatenate them in reverse order to get the correct result
      for( auto it = vectorArgument.rbegin(), itEnd = vectorArgument.rend(); it != itEnd; ++it )
      {
         stringResult += it->as_string();
      }
      *pvalueResult = value(stringResult);
   }
   else
   {
      double dSum = 0.0;
      for( const auto& v : vectorArgument ) { dSum += v.as_double(); }
      *pvalueResult = value(dSum);
   }

   return { true, "" };
}

/// Return the median value from a list of numbers.
/// If the first argument is a string, all arguments will be treated as strings and the median
/// string (lexicographically middle) will be returned. Otherwise, numeric median is returned.
std::pair<bool, std::string> median_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "median requires at least 1 argument" };

   if( vectorArgument.front().is_string() == true )                           // Check if first value is string or number
   {
      // ## Collect all strings
      std::vector<std::string> vectorStrings;
      for( const auto& v_ : vectorArgument ) { vectorStrings.push_back(v_.as_string()); }
      
      std::sort(vectorStrings.begin(), vectorStrings.end());                  // Sort strings lexicographically
      
      // ## Find median
      size_t uSize = vectorStrings.size();
      std::string stringMedian;
      if( uSize % 2 == 0 )
      {
         stringMedian = vectorStrings[uSize / 2 - 1];                         // Even number of strings - return the lower median (or could average? Usually pick lower for strings)
      }
      else { stringMedian = vectorStrings[uSize / 2]; }                       // Odd number of strings
      
      *pvalueResult = value(stringMedian);
   }
   else
   {
      // ## Collect all numbers
      std::vector<double> vectorNumbers;
      for( const auto& v : vectorArgument ) { vectorNumbers.push_back(v.as_double()); }
      
      std::sort(vectorNumbers.begin(), vectorNumbers.end());                 // Sort numbers
      
      // ## Find median
      size_t size = vectorNumbers.size();
      double dMedian;
      if( size % 2 == 0 ) { dMedian = (vectorNumbers[size / 2 - 1] + vectorNumbers[size / 2]) / 2.0; } // Even number of values - average the two middle values
      else { dMedian = vectorNumbers[size / 2]; }                             // Odd number of values
      
      *pvalueResult = value(dMedian);
   }
   
   return { true, "" };
}


/// Calculate the sample standard deviation of a list of numbers.
/// Only works with numeric arguments.
std::pair<bool, std::string> stddev_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "stddev requires at least 1 argument" };
   
   // Calculate mean first
   double dSum = 0.0;
   for( const auto& v : vectorArgument ) { dSum += v.as_double(); }
   double dMean = dSum / vectorArgument.size();
   
   // Calculate sum of squared differences
   double dSumSqDiff = 0.0;
   for( const auto& v : vectorArgument )
   {
      double dDiff = v.as_double() - dMean;
      dSumSqDiff += dDiff * dDiff;
   }
   
   // Sample standard deviation (use n-1 for sample variance)
   double variance = dSumSqDiff / (vectorArgument.size() - 1);
   double dStdDev = std::sqrt(variance);
   
   *pvalueResult = value(dStdDev);
   return { true, "" };
}


/// Return the product of multiple numbers.
/// If the first argument is a string, returns an error since product doesn't make sense for strings.
std::pair<bool, std::string> product_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "product requires at least 1 argument" };
 
   double dProduct = 1.0;
   for( const auto& v : vectorArgument ) { dProduct *= v.as_double(); }
   *pvalueResult = value(dProduct);
   return { true, "" };
}

/// Calculate the sample variance of a list of numbers.
/// Only works with numeric arguments.
std::pair<bool, std::string> variance_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "variance requires at least 1 argument" };
   
   // Calculate mean first
   double dSum = 0.0;
   for( const auto& v : vectorArgument ) { dSum += v.as_double(); }
   double dMean = dSum / vectorArgument.size();
   
   // Calculate sum of squared differences
   double dSumSqDiff = 0.0;
   for( const auto& v : vectorArgument )
   {
      double dDiff = v.as_double() - dMean;
      dSumSqDiff += dDiff * dDiff;
   }
   
   // Sample variance (use n-1 for sample variance)
   double dVariance = dSumSqDiff / (vectorArgument.size() - 1);
   
   *pvalueResult = value(dVariance);
   return { true, "" };
}

/// Return the first non-null value from the list of arguments.
/// If all arguments are null, returns null.
/// This function handles any value types (strings, numbers, booleans, etc.)
std::pair<bool, std::string> coalesce_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{
   if( vectorArgument.empty() ) return { false, "coalesce requires at least 1 argument" };

   // Iterate through all arguments and return the first non-null value
   for( const auto& v : vectorArgument )
   {
      if( !v.is_null() )
      {
         *pvalueResult = v;
         return { true, "" };
      }
   }
   
   *pvalueResult = value();                                                  // If all arguments are null, return null
   return { true, "" };
}

/// Check if a value exists (is not null) and return the value if it exists, otherwise return null.
std::pair<bool, std::string> exists_g(const std::vector<value>& vectorArgument, value* pvalueResult)
{                                                                                                  assert(vectorArgument.size() > 1);
   const auto& v_ = vectorArgument[1];

   bool bExists = false;

   if( v_.is_bool() == true ) { bExists = v_.as_bool(); }
   else if( v_.is_null() == false ) { bExists = true; }
   
   if( bExists == true )
   {
      *pvalueResult = vectorArgument[0];
      return { true, "" };
   }
   else
   {
      *pvalueResult = value();                                                  // return null if value does not exist
      return { true, "" };
   }
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

/// Check if any of the needles (all arguments except last, that is first)
std::pair<bool, std::string> has_g( const std::vector< value >& vectorArgument, value* pvalueResult )
{
   if( vectorArgument.empty() ) return { false, "has requires at least 1 argument" };

   std::string stringTemporary;
   std::string_view stringHaystack;
   const auto& haystack_ = vectorArgument.back();
   if( haystack_.is_string() == false ) 
   { 
      stringTemporary = haystack_.as_string();                                // convert to string if not string, this allows us to check for substrings in non-string values
      stringHaystack = stringTemporary;                                       // use string view for searching, this allows us to avoid unnecessary string copies
   }
   else { stringHaystack = haystack_.as_string_view(); }
  

   // ## check if haystack has any of the needles, check backwards since haystack is last argument and needles are all arguments before haystack
   for( size_t u = 0; u < vectorArgument.size() - 1; ++u )
   {
      const auto& needle_ = vectorArgument[u];
      if( needle_.is_string() == true )
      { 
         std::string_view s_ = needle_.as_string_view();                      // get string view of needle, this allows us to avoid unnecessary string copies
         if( stringHaystack.find(s_) != std::string_view::npos )
         {
            *pvalueResult = true;
            return { true, "" };
         }
      }
      else
      {
         auto stringNeedle = needle_.as_string();                             // get string of needle
         if( stringHaystack.find(stringNeedle) != std::string_view::npos )
         {
            *pvalueResult = true;
            return { true, "" };
         } 
      }
   }

   *pvalueResult = false;
   return { true, "" };
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

/// Format IP address as hex string, optionally truncating to specified size (number of characters)
std::pair<bool, std::string> ip_format_g( const std::vector<value>& vectorArgument, value* pvalueResult )
{                                                                                                  assert(vectorArgument.size() > 2);
   const auto& ip_ = vectorArgument[2];
   const auto& format_ = vectorArgument[1];
   const auto& size_ = vectorArgument[0];

   if( ip_.is_string() && format_.is_string() && size_.is_integer() )
   {
      auto stringIP = ip_.as_string_view();
      auto stringFormat = format_.as_string_view();
      auto iSize = size_.as_integer();

      std::string stringResult;

      if( stringFormat == "hex" || stringFormat == "uuid" )
      {
         uint8_t buffer_[16] = {}; // Buffer to hold raw bytes of IP address, large enough for IPv6
         int iBytes = 0; // Number of bytes parsed into buffer_

         // ## Try IPv4: segments separated by '.', exactly 4 parts, each 0-255
         if( stringIP.find( '.' ) != std::string_view::npos )
         {
            std::string_view sv_ = stringIP;
            int iParts = 0;
            bool bValid = true;

            while( !sv_.empty() && iParts < 4 )
            {
               auto pos = sv_.find( '.' );
               auto token = (pos == std::string_view::npos) ? sv_ : sv_.substr( 0, pos );

               if( token.empty() || token.size() > 3 ) { bValid = false; break; }

               uint32_t uValue = 0;
               for( char c : token )
               {
                  if( c < '0' || c > '9' ) { bValid = false; break; }
                  uValue = uValue * 10 + static_cast<uint32_t>( c - '0' );
               }
               if( !bValid || uValue > 255 ) { bValid = false; break; }

               buffer_[iParts++] = static_cast<uint8_t>( uValue );
               sv_ = (pos == std::string_view::npos) ? std::string_view{} : sv_.substr( pos + 1 );
            }

            if( bValid && iParts == 4 && sv_.empty() ) { iBytes = 4; }
            else { return { false, "ip_format - Invalid IPv4 address" }; }
         }
         // ## Try IPv6: contains ':', parse groups with '::' expansion
         else if( stringIP.find( ':' ) != std::string_view::npos )
         {
            // Split on '::' to get left and right halves
            auto dpos = stringIP.find( "::" );
            std::string_view left  = (dpos != std::string_view::npos) ? stringIP.substr( 0, dpos ) : stringIP;
            std::string_view right = (dpos != std::string_view::npos) ? stringIP.substr( dpos + 2 ) : std::string_view{};

            // ## Parse one half into 16-bit groups, returns false on any invalid token
            auto parseGroups = []( std::string_view sv, uint16_t* pGroups, int& iCount ) -> bool
            {
               iCount = 0;
               if( sv.empty() ) return true;
               while( !sv.empty() )
               {
                  auto pos   = sv.find( ':' );
                  auto token = (pos == std::string_view::npos) ? sv : sv.substr( 0, pos );

                  if( token.empty() || token.size() > 4 ) return false;

                  uint32_t uVal = 0;
                  for( char c : token )
                  {
                     uint32_t uDigit;
                     if     ( c >= '0' && c <= '9' ) uDigit = static_cast<uint32_t>( c - '0' );
                     else if( c >= 'a' && c <= 'f' ) uDigit = static_cast<uint32_t>( c - 'a' + 10 );
                     else if( c >= 'A' && c <= 'F' ) uDigit = static_cast<uint32_t>( c - 'A' + 10 );
                     else return false;
                     uVal = (uVal << 4) | uDigit;
                  }
                  if( iCount >= 8 ) return false;
                  pGroups[iCount++] = static_cast<uint16_t>( uVal );
                  sv = (pos == std::string_view::npos) ? std::string_view{} : sv.substr( pos + 1 );
               }
               return true;
            };

            uint16_t leftGroups_[8]  = {};
            uint16_t rightGroups_[8] = {};
            int iLeftCount  = 0;
            int iRightCount = 0;

            if( !parseGroups( left, leftGroups_, iLeftCount ) )   { return { false, "ip_format - Invalid IPv6 address" }; }
            if( !parseGroups( right, rightGroups_, iRightCount ) ) { return { false, "ip_format - Invalid IPv6 address" }; }
            if( dpos == std::string_view::npos && iLeftCount != 8 ) { return { false, "ip_format - Invalid IPv6 address" }; }
            if( iLeftCount + iRightCount > 8 ) { return { false, "ip_format - Invalid IPv6 address" }; }

            // ## Expand '::' zero gap into full 8-group array, big-endian byte order
            uint16_t groups_[8] = {};
            for( int i = 0; i < iLeftCount;  ++i ) groups_[i] = leftGroups_[i];
            for( int i = 0; i < iRightCount; ++i ) groups_[8 - iRightCount + i] = rightGroups_[i];

            for( int i = 0; i < 8; ++i )
            {
               buffer_[i * 2]     = static_cast<uint8_t>( groups_[i] >> 8 );
               buffer_[i * 2 + 1] = static_cast<uint8_t>( groups_[i] & 0xFF );
            }
            iBytes = 16;
         }
         else { return { false, "ip_format - Invalid IP address" }; }

         if( stringFormat == "hex" )
         {
            // ## Convert raw bytes to uppercase hex using gd_binary helper
            stringResult = gd::binary_to_hex_g( buffer_, static_cast<size_t>( iBytes ), true );

            // ## Pad with leading zeros to full hex width if no size constraint
            // IPv4 = 8 hex chars, IPv6 = 32 hex chars
            size_t uHexWidth = static_cast<size_t>( iBytes ) * 2;
            if( stringResult.length() < uHexWidth ) { stringResult.insert( 0, uHexWidth - stringResult.length(), '0' ); }
         }
         else // "uuid"
         {
            // ## For IPv4, right-align 4 bytes into 16-byte UUID buffer (zero-prefixed)
            uint8_t uuid_[16] = {};
            if( iBytes == 4 ) std::memcpy( uuid_ + 12, buffer_, 4 );
            else              std::memcpy( uuid_, buffer_, 16 );

            char szUUID[37];
            std::snprintf( szUUID, sizeof( szUUID ),
               "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
               uuid_[0],  uuid_[1],  uuid_[2],  uuid_[3],
               uuid_[4],  uuid_[5],  uuid_[6],  uuid_[7],
               uuid_[8],  uuid_[9],  uuid_[10], uuid_[11],
               uuid_[12], uuid_[13], uuid_[14], uuid_[15] );
            stringResult = szUUID;
         }
      }
      else { return { false, "ip_format - Invalid format argument" }; }       // invalid format, return error

      // ## Pad with leading zeros if result is shorter than requested size
      if( iSize > 0 && stringResult.length() < static_cast<size_t>( iSize ) )
      {
         stringResult.insert( 0, static_cast<size_t>( iSize ) - stringResult.length(), '0' );
      }

      // ## Truncate from the left if result is longer than requested size
      else if( iSize > 0 && static_cast<size_t>( iSize ) < stringResult.length() )
      {
         stringResult = stringResult.substr( stringResult.length() - static_cast<size_t>( iSize ) );
      }

      *pvalueResult = stringResult;
      return { true, "" };
   }

   return { false, "ip_format - Invalid argument type" };
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
