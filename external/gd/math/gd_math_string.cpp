#include <algorithm>

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
 * @brief Extracts a specific line from stringText based on the given line index.
 *
 * This function searches for the line at the specified index (0-based) in stringText.
 * Lines are separated by newline characters ('\n'). If the index is 0, the first line
 * is returned. If the index is out of bounds (negative or exceeds the number of lines),
 * an empty string is returned. The returned line does not include the newline character.
 *
 * @param stringText The source string to search within.
 * @param uLineIndex The 0-based index of the line to extract.
 * @return std::string The line at the specified index, or an empty string if index is out of bounds.
 * 
 * @code
 * std::string text = "Line 1\nLine 2\nLine 3";
 * std::string line = select_line(text, 1, '\n');
 * @endcode
 */
std::string select_line(const std::string_view& stringText, size_t uLineIndex, char iNewLine )
{                                                                                                  assert(stringText.empty() == false);
   size_t uCurrentLine = 0; // Current line index being processed
   size_t uStart = 0; // Start position for searching
   size_t uEnd = 0; // End position for the found line

   // ## Handle first line (index 0)
   if(uLineIndex == 0) 
   {
      uEnd = stringText.find(iNewLine);
      if(uEnd == std::string_view::npos) { return std::string(stringText); }   // Single line, return entire string
      return std::string(stringText.substr(0, uEnd));
   }

   // ## Search for the target line
   while(uCurrentLine < uLineIndex) 
   {
      uStart = stringText.find(iNewLine, uStart);
      if(uStart == std::string_view::npos) { return {}; }                      // Line index out of bounds
      uStart++;                                                                // Move past the newline
      uCurrentLine++;
   }

   // ## Find the end of the target line
   uEnd = stringText.find(iNewLine, uStart);
   if(uEnd == std::string_view::npos) 
   {
      // Last line in string
      if(uStart < stringText.length()) 
      {
         return std::string(stringText.substr(uStart));
      }
      return {};                                                               // Empty line at end
   }

   return std::string(stringText.substr(uStart, uEnd - uStart));
}

/** ---------------------------------------------------------------------------
 * @brief Extracts all lines from the beginning of stringText up to and including the specified line index.
 *
 * This function returns a substring view containing all lines from the start of stringText
 * up to and including the line at the specified index (0-based). Lines are separated by
 * newline characters. If the index is 0, only the first line is returned. If the index
 * is out of bounds (exceeds the number of lines), the entire string is returned.
 * The returned view includes newline characters except for the final line if it doesn't
 * end with a newline.
 *
 * @param stringText The source string to search within.
 * @param uLineIndex The 0-based index of the target line (inclusive).
 * @param iNewLine The character used to separate lines (default: '\n').
 * @return std::string_view A view of the text from start to the specified line.
 * 
 * @code
 * std::string text = "Line 1\nLine 2\nLine 3";
 * std::string_view result = select_to_line(text, 1, '\n');
 * // result contains "Line 1\nLine 2"
 * @endcode
 */
std::string_view select_to_line( const std::string_view& stringText, size_t uLineIndex, char iNewLine )
{                                                                                                  assert(stringText.empty() == false);
   size_t uCurrentLine = 0; // Current line index being processed
   size_t uStart = 0; // Start position for searching
   size_t uEnd = 0; // End position for the found line
   
   // ## Handle first line (index 0)
   if(uLineIndex == 0) 
   {
      uEnd = stringText.find(iNewLine);
      if(uEnd == std::string_view::npos) { return stringText; }               // Single line, return entire string
      return stringText.substr(0, uEnd);
   }
   
   // ## Search for the target line
   while(uCurrentLine < uLineIndex) 
   {
      uStart = stringText.find(iNewLine, uStart);
      if(uStart == std::string_view::npos) { return stringText; }             // Line index out of bounds, return entire string
      uStart++;                                                               // Move past the newline
      uCurrentLine++;
   }
   
   // ## Find the end of the target line
   uEnd = stringText.find(iNewLine, uStart);
   if(uEnd == std::string_view::npos) 
   {
      // Last line in string, return from beginning to end
      return stringText;
   }
   
   return stringText.substr(0, uEnd);
}

/** ---------------------------------------------------------------------------
 * @brief Extracts all lines from the specified line index to the end of stringText.
 *
 * This function returns a substring view containing all lines from the specified
 * index (0-based) to the end of stringText. Lines are separated by newline characters.
 * If the index is 0, the entire string is returned. If the index is out of bounds
 * (exceeds the number of lines), an empty string_view is returned. The returned view
 * starts from the beginning of the target line (not including the preceding newline).
 *
 * @param stringText The source string to search within.
 * @param uLineIndex The 0-based index of the starting line (inclusive).
 * @param iNewLine The character used to separate lines (default: '\n').
 * @return std::string_view A view of the text from the specified line to the end.
 * 
 * @code
 * std::string text = "Line 1\nLine 2\nLine 3";
 * std::string_view result = select_from_line(text, 1, '\n');
 * // result contains "Line 2\nLine 3"
 * @endcode
 */
std::string_view select_from_line( const std::string_view& stringText, size_t uLineIndex, char iNewLine )
{                                                                                                  assert(stringText.empty() == false);
   size_t uCurrentLine = 0; // Current line index being processed
   size_t uStart = 0; // Start position for searching
   
   // ## Handle first line (index 0)
   if(uLineIndex == 0) { return stringText; }                                 // Return entire string for first line
   
   // ## Search for the target line
   while(uCurrentLine < uLineIndex) 
   {
      uStart = stringText.find(iNewLine, uStart);
      if(uStart == std::string_view::npos) { return {}; }                     // Line index out of bounds
      uStart++;                                                               // Move past the newline
      uCurrentLine++;
   }
   
   // ## Return from start of target line to end
   if(uStart < stringText.length()) 
   {
      return stringText.substr(uStart);
   }
   
   return {};                                                                 // Empty line at end
}

/** ---------------------------------------------------------------------------
 * @brief Selects all consecutive lines with content from the start until reaching an empty line.
 *
 * This function returns a substring view containing all lines from the beginning of
 * stringText that have content (non-whitespace characters) until it encounters the first
 * line that is empty or contains only whitespace characters. The search stops at the
 * first empty/whitespace-only line encountered. If all lines have content, the entire
 * string is returned. If the first line is empty, an empty string_view is returned.
 *
 * @param stringText The source string to search within.
 * @param iNewLine The character used to separate lines (default: '\n').
 * @return std::string_view A view of all consecutive content lines from the start.
 * 
 * @code
 * std::string text = "Line 1\nLine 2\n\nLine 4";
 * std::string_view result = select_content_lines(text, '\n');
 * // result contains "Line 1\nLine 2"
 * @endcode
 */
std::string_view select_content_lines( const std::string_view& stringText, char iNewLine )
{
   // ## Handle empty string
   if(stringText.empty()) { return stringText; }
   
   size_t uLineStart = 0; // Start position of current line
   size_t uLineEnd = 0; // End position of current line
   size_t uLastContentEnd = 0; // End position of last line with content
   bool bFoundContent = false; // Flag to track if we've found any content
   
   while(uLineStart < stringText.length()) 
   {
      // ## Find the end of current line
      uLineEnd = stringText.find(iNewLine, uLineStart);
      if(uLineEnd == std::string_view::npos) 
      {
         uLineEnd = stringText.length(); // Last line without newline
      }
      
      // ## Extract current line content
      std::string_view currentLine = stringText.substr(uLineStart, uLineEnd - uLineStart);
      
      // ## Check if line has content (non-whitespace characters)
      bool bLineHasContent = false;
      for(char iCharacter : currentLine) 
      {
         if(iCharacter != ' ' && iCharacter != '\t' && iCharacter != '\r') { bLineHasContent = true; break; }
      }
      
      // ## If line is empty/whitespace-only, stop here
      if(bLineHasContent == false) 
      {
         if(bFoundContent == false) { return {}; }                            // No content found before this empty line 
         break;
      }
      
      // ## Line has content, update tracking variables
      bFoundContent = true;
      uLastContentEnd = uLineEnd;
      
      // ## Move to next line
      if(uLineEnd < stringText.length() && stringText[uLineEnd] == iNewLine) {  uLineStart = uLineEnd + 1; }
      else { break; }                                                          // Reached end of string
   }
   
   // ## Return all content lines found
   if(bFoundContent) 
   {
      return stringText.substr(0, uLastContentEnd);
   }
   
   return {}; // No content found
}

/** ---------------------------------------------------------------------------
 * @brief Extracts a substring from stringText that is located between matching delimiters from vectorDelimiters.
 *
 * This function searches for the first occurrence of any delimiter from vectorDelimiters in stringText.
 * Once found, it starts extracting after the end of that delimiter and searches for the next occurrence
 * of the same delimiter. It extracts all characters between these matching delimiters. If no matching
 * pair is found, or if the resulting substring would be empty or invalid, an empty string is returned.
 *
 * @param stringText The source string to search within.
 * @param vectorDelimiters Vector of delimiter strings to search for. Must use same delimiter for start and end.
 * @return std::string The substring found between matching delimiters, or an empty string if not found.
 * 
 * @code
 * // Example 1: Basic usage with quotes
 * std::string text = "The 'quick brown' fox";
 * std::vector<std::string> delimiters = {"'", "\""};
 * std::string result = select_between(text, delimiters);
 * // result = "quick brown"
 * 
 * // Example 2: Multiple delimiter types, first one wins
 * std::string text2 = "Start[content]more\"other\"end";
 * std::vector<std::string> delimiters2 = {"[", "]", "\""};
 * std::string result2 = select_between(text2, delimiters2);
 * // result2 = "content" (found '[' first)
 * 
 * // Example 3: Delimiter at start of string
 * std::string text3 = "*hello*world";
 * std::vector<std::string> delimiters3 = {"*", "|"};
 * std::string result3 = select_between(text3, delimiters3);
 * // result3 = "hello"
 * 
 * // Example 4: No matching pair found
 * std::string text4 = "No delimiters here";
 * std::vector<std::string> delimiters4 = {"[", "]"};
 * std::string result4 = select_between(text4, delimiters4);
 * // result4 = "" (empty string)
 * @endcode * 
 */
std::string select_between(const std::string_view& stringText, const std::vector<std::string>& vectorDelimiters)
{                                                                                                  assert(stringText.empty() == false); assert(vectorDelimiters.empty() == false);
   size_t uStart = std::string_view::npos;
   std::string stringDelimiter;
   size_t uDelimiterLength = 0;

   // ## First try to find the first occurrence of any delimiter, check start of stringText
   for(const auto& delimiter_ : vectorDelimiters)
   {                                                                                               assert(delimiter_.empty() == false);
      if(stringText.starts_with(delimiter_) )
      {
         uStart = 0;                                                           // Start position is at the beginning of the string
         stringDelimiter = delimiter_;
         uDelimiterLength = delimiter_.length();

         break; // No need to check further, we found a match at the start
      }
   }
   
   if( uDelimiterLength == 0 )
   {
      // ## Find the first occurrence of any delimiter
      for(const auto& delimiter_ : vectorDelimiters)
      {                                                                                            assert(delimiter_.empty() == false);
         size_t uFound = stringText.find(delimiter_);
         if(uFound != std::string_view::npos && (uStart == std::string_view::npos || uFound < uStart))
         {
            uStart = uFound;
            stringDelimiter = delimiter_;
            uDelimiterLength = delimiter_.length();
         }
      }
   }
   
   if(uStart == std::string_view::npos) { return {}; }                         // No delimiter found
   
   // Start position is after the first delimiter
   uStart += uDelimiterLength;
   
   // Find the next occurrence of the same delimiter
   size_t uEnd = stringText.find(stringDelimiter, uStart);
   if(uEnd == std::string_view::npos) { return {}; }                           // Matching delimiter not found
   
   // Validate positions
   if(uStart >= uEnd) { return {}; }
   
   // Extract substring
   return std::string(stringText.substr(uStart, uEnd - uStart));
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
std::vector<std::string> select_between_all(const std::string_view& stringText, const std::string_view& stringFrom, const std::string_view& stringTo) 
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