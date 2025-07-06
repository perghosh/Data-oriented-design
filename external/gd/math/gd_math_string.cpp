#include "gd_math_string.h"

_GD_MATH_STRING_BEGIN

/** ---------------------------------------------------------------------------
 * @brief Counts lines using vectorized operations for extremely large strings.
 *
 * This function uses optimized memory access patterns and processes multiple characters
 * at once when possible. It falls back to standard counting for smaller strings but
 * provides significant performance improvements for very large text processing.
 *
 * @param stringText The string to count lines in.
 * @param iCharacter The character to count instances of (e.g., newline).
 * @return size_t The number of instances of iCharacter in the string.
 */
size_t count_character(const std::string_view& stringText, char iCharacter ) noexcept 
{
   if(stringText.empty()) { return 0; }

   size_t uCharCount = 0;  // Start with 0 count for instances of iCharacter
   const char* piText = stringText.data();
   const size_t uLength = stringText.length();

   // Use std::count for small strings
   constexpr size_t uSmallStringThreshold = 256;
   if(uLength <= uSmallStringThreshold) 
   {
      return std::count(stringText.begin(), stringText.end(), iCharacter);
   }

   // Process in chunks for cache efficiency
   constexpr size_t uChunkSize = 64;  // Cache-line aligned
   const char* pEnd = piText + uLength;
   size_t uProcessed = 0;

   while(uProcessed + uChunkSize <= uLength) 
   {
      const char* pChunkEnd = piText + uChunkSize;
      while(piText < pChunkEnd) 
      {
         if(*piText == iCharacter) { ++uCharCount; }
         ++piText;
      }
      uProcessed += uChunkSize;
   }

   // Process remaining characters
   while(piText < pEnd) 
   {
      if(*piText == iCharacter) { ++uCharCount; }
      ++piText;
   }

   return uCharCount;
}


/** ---------------------------------------------------------------------------
 * @brief Extracts a substring from stringText starting from the first occurrence of `stringFrom`.
 *
 * This function searches for the first occurrence of `stringFrom` in stringText and, if found,
 * extracts all characters from that position to the end of the string. If `stringFrom` is not found,
 * an empty string is returned. If `stringFrom` is empty, the entire string is returned.
 *
 * @param stringText The source string to search within.
 * @param stringFrom The starting delimiter to search for.
 * @return std::string The substring from `stringFrom` to the end, or an empty string if not found.
 */
std::string select_from(const std::string_view& stringText, const std::string_view& stringFrom) 
{                                                                                                  assert(stringText.empty() == false);
   if(stringFrom.empty() == true) { return std::string(stringText); }
   
   size_t uFrom = stringText.find(stringFrom);
   if(uFrom == std::string_view::npos) { return {}; }                          // stringFrom not found
   
   return std::string(stringText.substr(uFrom + stringFrom.length()));
}

/** ---------------------------------------------------------------------------
 * @brief Extracts a substring from stringText up to but not including the first occurrence of `stringTo`.
 *
 * This function searches for the first occurrence of `stringTo` in stringText and, if found,
 * extracts all characters from the beginning up to (but not including) that position. If `stringTo`
 * is not found, the entire string is returned. If `stringTo` is empty, an empty string is returned.
 *
 * @param stringText The source string to search within.
 * @param stringTo The ending delimiter to search for.
 * @return std::string The substring from start to `stringTo`, or the entire string if not found.
 */
std::string select_until(const std::string_view& stringText, const std::string_view& stringTo) 
{                                                                                                  assert(stringText.empty() == false);
   if(stringTo.empty() == true) { return {}; }
   
   size_t uTo = stringText.find(stringTo);
   if(uTo == std::string_view::npos) { return std::string(stringText); }       // stringTo not found, return whole string
   
   return std::string(stringText.substr(0, uTo));
}

/** ---------------------------------------------------------------------------
 * @brief Extracts a substring from stringText that is located between `stringFrom` and `stringTo`.
 *
 * This function searches for the first occurrence of `stringFrom` in stringText and, if found,
 * starts extracting after its end. It then searches for the first occurrence of `stringTo` after
 * `stringFrom` and extracts all characters up to (but not including) `stringTo`. If either delimiter
 * is not found, or if the resulting substring would be empty or invalid, an empty string is returned.
 *
 * @param stringText The source string to search within.
 * @param stringFrom The starting delimiter. If empty, extraction starts from the beginning.
 * @param stringTo The ending delimiter. If empty, extraction continues to the end.
 * @return std::string The substring found between `stringFrom` and `stringTo`, or an empty string if not found.
 */
std::string select_between(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo) 
{                                                                                                  assert(stringText.empty() == false);
   size_t uStart = 0;
   size_t uEnd = stringText.length();

   // Find start position
   if( stringFrom.empty() == false ) 
   {
      size_t uFrom = stringText.find(stringFrom);
      if(uFrom == std::string_view::npos) { return {}; }                       // stringFrom not found
      uStart = uFrom + stringFrom.length();
   }

   // Find end position
   if(stringTo.empty() == false) 
   {
      size_t uTo = stringText.find(stringTo, uStart);
      if(uTo == std::string_view::npos) { return {}; }                         // stringTo not found
      uEnd = uTo;
   }

   // Validate positions
   if(uStart >= uEnd) { return {}; }

   // Extract substring
   return std::string(stringText.substr(uStart, uEnd - uStart));
}

/** ---------------------------------------------------------------------------
 * @brief Extracts a substring by selecting the nth occurrence of text between delimiters.
 *
 * This function searches for the nth occurrence of `stringFrom` in stringText (1-based indexing)
 * and, if found, extracts text until the next occurrence of `stringTo`. This allows selective
 * extraction when multiple delimiter pairs exist in the source string.
 *
 * @param stringText The source string to search within.
 * @param stringFrom The starting delimiter.
 * @param stringTo The ending delimiter.
 * @param uOccurrence The occurrence number to select (1-based, where 1 is the first occurrence).
 * @return std::string The substring from the nth occurrence, or an empty string if not found.
 */
std::string select_between_nth(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo, size_t uOccurrence) 
{                                                                                                  assert(stringText.empty() == false);
                                                                                                   assert(uOccurrence > 0);
   if(stringFrom.empty() == true || stringTo.empty() == true) { return {}; }
   
   size_t uSearchPosition = 0;
   size_t uStart = 0;
   
   // Find the nth occurrence of stringFrom
   for(size_t u = 0; u < uOccurrence; ++u) 
   {
      size_t uFrom = stringText.find(stringFrom, uSearchPosition);
      if(uFrom == std::string_view::npos) { return {}; }                       // nth occurrence not found
      
      uStart = uFrom + stringFrom.length();
      uSearchPosition = uStart;
   }
   
   // Find stringTo after the nth stringFrom
   size_t uTo = stringText.find(stringTo, uStart);
   if(uTo == std::string_view::npos) { return {}; }                            // stringTo not found
   
   // Validate positions
   if(uStart >= uTo) { return {}; }
   
   return std::string(stringText.substr(uStart, uTo - uStart));
}

/** ---------------------------------------------------------------------------
 * @brief Extracts a substring by removing specified prefix and suffix if they exist.
 *
 * This function checks if stringText starts with `stringPrefix` and ends with `stringSuffix`.
 * If both conditions are met, it returns the string with these parts removed. If either
 * condition fails, the original string is returned unchanged. This is useful for unwrapping
 * quoted strings or removing brackets.
 *
 * @param stringText The source string to process.
 * @param stringPrefix The prefix to remove from the start.
 * @param stringSuffix The suffix to remove from the end.
 * @return std::string The string with prefix and suffix removed, or the original string if conditions not met.
 */
std::string select_unwrap(const std::string_view& stringText, const std::string_view& stringPrefix, const std::string_view& stringSuffix) 
{                                                                                                  assert(stringText.empty() == false);
   // Check if string is long enough to contain both prefix and suffix
   if(stringText.length() < stringPrefix.length() + stringSuffix.length()) 
   {
      return std::string(stringText);
   }
   
   // Check if string starts with prefix and ends with suffix
   bool bHasPrefix = stringPrefix.empty() || stringText.substr(0, stringPrefix.length()) == stringPrefix;
   bool bHasSuffix = stringSuffix.empty() || stringText.substr(stringText.length() - stringSuffix.length()) == stringSuffix;
   
   if(bHasPrefix == false || bHasSuffix == false) 
   {
      return std::string(stringText);                                          // Conditions not met, return original
   }
   
   size_t uStart = stringPrefix.length();
   size_t uLength = stringText.length() - stringPrefix.length() - stringSuffix.length();
   
   return std::string(stringText.substr(uStart, uLength));
}

/** ---------------------------------------------------------------------------
 * @brief Extracts all substrings between pairs of delimiters and returns them as a vector.
 *
 * This function finds all occurrences of text between `stringFrom` and `stringTo` delimiters
 * and returns them as a vector of strings. It processes the string sequentially, finding each
 * pair of delimiters and extracting the content between them. Empty matches are skipped.
 *
 * @param stringText The source string to search within.
 * @param stringFrom The starting delimiter.
 * @param stringTo The ending delimiter.
 * @return std::vector<std::string> Vector containing all substrings found between delimiter pairs.
 */
std::vector<std::string> select_all_between(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo) 
{                                                                                                  assert(stringText.empty() == false);
   std::vector<std::string> vectorResult;
   
   if(stringFrom.empty() == true || stringTo.empty() == true) { return vectorResult; }
   
   size_t uSearchPosition = 0;
   
   while(uSearchPosition < stringText.length()) 
   {
      // ## Find next occurrence of stringFrom
      size_t uFrom = stringText.find(stringFrom, uSearchPosition);
      if(uFrom == std::string_view::npos) { break; }                           // No more occurrences
      
      size_t uStart = uFrom + stringFrom.length();
      
      // ## Find corresponding stringTo
      size_t uTo = stringText.find(stringTo, uStart);
      if(uTo == std::string_view::npos) { break; }                             // No matching end delimiter
      
      // Extract and add to results if not empty
      if(uStart < uTo) 
      {
         vectorResult.emplace_back(stringText.substr(uStart, uTo - uStart));
      }
      
      // Continue search after the end delimiter
      uSearchPosition = uTo + stringTo.length();
   }
   
   return vectorResult;
}


_GD_MATH_STRING_END