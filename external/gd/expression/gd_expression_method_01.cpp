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
      auto vectorTags = detail::read_tags(stringText);
      bool bHas = false;
      for(const auto& tagView : vectorTags)
      {
         if(tagView == stringTag)
         {
            bHas = true;
            break;
         }
      }
      *pvalueResult = bHas;
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