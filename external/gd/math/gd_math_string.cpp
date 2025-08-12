#include <algorithm>
#include <sstream>

#include "gd/gd_compiler.h"
#include "gd/gd_types.h"
#include "gd/gd_utf8.h"

#include "gd_math_string.h"

_GD_MATH_STRING_BEGIN


/** ---------------------------------------------------------------------------
 * @brief Extracts a line from stringText based on the specified newline character.
 *
 * This function searches for the first occurrence of the newline character in stringText,
 * extracts the line up to that point, and updates stringText to remove the processed line.
 * If no newline character is found, it returns the entire string as a single line.
 *
 * @param stringText The source string to search within.
 * @param stringLine The output string where the extracted line will be stored.
 * @param iNewLine The character used to identify new lines (default: '\n').
 * @return bool True if a line was successfully extracted, false if stringText is empty.
 * 
 * @code
 * std::string_view text = "Hello\nWorld";
 * std::string line;
 * while(getline(text, line)) {
 *     std::cout << "Extracted line: " << line << std::endl;
 * }
 * @endcode
 */
bool getline(std::string_view& stringText, std::string& stringLine, char iNewLine )
{
   if(stringText.empty()) { return false; }                                   // No text to process

   size_t uPosition = stringText.find(iNewLine);
   if(uPosition == std::string_view::npos) 
   {
      stringLine = std::string(stringText); // No newline found, return the whole string
      stringText = {}; // Clear the original string
      return true;
   }

   // ## Extract line and prepare for next read
   stringLine = std::string(stringText.substr(0, uPosition));
   stringText.remove_prefix(uPosition + 1);                                   // Remove processed part including the newline character

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Extracts a line from stringText based on the specified newline character.
 *
 * This function searches for the first occurrence of the newline character in stringText,
 * extracts the line up to that point, and updates stringText to remove the processed line.
 * If no newline character is found, it returns the entire string as a single line.
 *
 * @param stringText The source string to search within.
 * @param stringLine The output string_view where the extracted line will be stored.
 * @param iNewLine The character used to identify new lines (default: '\n').
 * @return bool True if a line was successfully extracted, false if stringText is empty.
 */
bool getline(std::string_view& stringText, std::string_view& stringLine, char iNewLine )
{
   if(stringText.empty()) { return false; }                                   // No text to process
   size_t uPosition = stringText.find(iNewLine);
   if(uPosition == std::string_view::npos) 
   {
      stringLine = stringText; // No newline found, return the whole string
      stringText = {}; // Clear the original string
      return true;
   }
   // ## Extract line and prepare for next read
   stringLine = stringText.substr(0, uPosition);
   stringText.remove_prefix(uPosition + 1);                                   // Remove processed part including the newline character
   return true;
}

/// return line from stringText based on the specified newline character
std::string_view getline(std::string_view& stringText, char iNewLine)
{
   std::string_view stringLine;
   getline(stringText, stringLine, iNewLine);                                 // Use the overloaded getline function
   return stringLine;
}

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

/** ---------------------------------------------------------------------------
 * @brief Indents text with specified number of spaces, with optional first line indentation.
 *
 * This function adds the specified number of spaces at the beginning of each line in the text.
 * The first line can optionally be excluded from indentation. This is useful for console output
 * where you want to maintain visual hierarchy and show that multi-line content belongs together.
 *
 * @param stringText The source text to indent.
 * @param uIndentSpaces Number of spaces to indent each line.
 * @param bIndentFirstLine If true, indents the first line; if false, leaves first line unchanged.
 * @param iNewLine The newline character to use for line detection (default: '\n').
 * @return std::string The indented text.
 * 
 * @code
 * // Example 1: Indent all lines including first
 * std::string text = "First line\nSecond line\nThird line";
 * std::string result = format_indent(text, 4, true);
 * // result = "    First line\n    Second line\n    Third line"
 * 
 * // Example 2: Skip first line indentation (common for labeled content)
 * std::string text2 = "Label: First line\nSecond line\nThird line";
 * std::string result2 = format_indent(text2, 7, false);
 * // result2 = "Label: First line\n       Second line\n       Third line"
 * 
 * // Example 3: Single line text
 * std::string text3 = "Only one line";
 * std::string result3 = format_indent(text3, 2, true);
 * // result3 = "  Only one line"
 * 
 * // Example 4: Empty lines preserved
 * std::string text4 = "Line 1\n\nLine 3";
 * std::string result4 = format_indent(text4, 2, true);
 * // result4 = "  Line 1\n  \n  Line 3"
 * @endcode
 */
std::string format_indent(const std::string_view& stringText, size_t uIndentSpaces, bool bIndentFirstLine, char iNewLine )
{                                                                                                  assert(stringText.empty() == false);
   std::string stringIndent(uIndentSpaces, ' '); // indentation string with specified number of spaces
   std::string stringResult; // Result string to hold the indented text
   stringResult.reserve(stringText.size() + ( uIndentSpaces * 10 ));          // Rough estimate for performance

   bool bFirstLine = true;
   size_t uStart = 0;

   while( uStart < stringText.size() )
   {
      size_t uEnd = stringText.find(iNewLine, uStart);                        // Find the end of current line
      bool bHasNewline = ( uEnd != std::string_view::npos );

      if( bHasNewline == false ) { uEnd = stringText.size(); }                // Last line without newline

      if( bFirstLine == false || bIndentFirstLine == true ) { stringResult += stringIndent; } // Add indentation if needed

      stringResult += stringText.substr(uStart, uEnd - uStart);               // Add the line content

      if( bHasNewline == true )                                               // Add newline if it was present in original
      {
         stringResult += iNewLine;
         uStart = uEnd + 1;
      }
      else { break; }

      bFirstLine = false;
   }

   return stringResult;
}


/** ---------------------------------------------------------------------------
 * @brief Prepends comment marker to each line of text, with optional first line exclusion.
 *
 * This function adds the specified comment marker string at the beginning of each line in the text.
 * The first line can optionally be excluded from commenting. This is useful for commenting out
 * code blocks in editors or adding comment markers to documentation text.
 *
 * @param stringText The source text to add comment markers to.
 * @param stringCommentMarker The comment marker string to prepend (e.g., "// ", "# ", "<!-- ").
 * @param bCommentFirstLine If true, comments the first line; if false, leaves first line unchanged.
 * @param iNewLine The newline character to use for line detection (default: '\n').
 * @return std::string The text with comment markers prepended.
 * 
 * @code
 * // Example 1: Comment all lines including first (C++ style)
 * std::string text = "int main() {\n    return 0;\n}";
 * std::string result = format_comment(text, "// ", true);
 * // result = "// int main() {\n//     return 0;\n// }"
 * 
 * // Example 2: Skip first line commenting (for mixed content)
 * std::string text2 = "Function description:\nint calculate() {\n    return 42;\n}";
 * std::string result2 = format_comment(text2, "// ", false);
 * // result2 = "Function description:\n// int calculate() {\n//     return 42;\n// }"
 * 
 * // Example 3: Python style comments
 * std::string text3 = "def hello():\n    print('world')";
 * std::string result3 = format_comment(text3, "# ", true);
 * // result3 = "# def hello():\n#     print('world')"
 * 
 * // Example 4: Empty lines preserved with comment markers
 * std::string text4 = "Line 1\n\nLine 3";
 * std::string result4 = format_comment(text4, "// ", true);
 * // result4 = "// Line 1\n// \n// Line 3"
 * @endcode
 */
std::string format_comment(const std::string_view& stringText, const std::string_view& stringCommentMarker, bool bCommentFirstLine, char iNewLine)
{                                                                                                  assert(stringText.empty() == false);
   std::string stringResult; // Result string to hold the commented text
   stringResult.reserve(stringText.size() + ( stringCommentMarker.size() * 10 )); // Rough estimate for performance
   bool bFirstLine = true;
   size_t uStart = 0;
   while( uStart < stringText.size() )
   {
      size_t uEnd = stringText.find(iNewLine, uStart);                        // Find the end of current line
      bool bHasNewline = ( uEnd != std::string_view::npos );
      if( bHasNewline == false ) { uEnd = stringText.size(); }                // Last line without newline
      if( bFirstLine == false || bCommentFirstLine == true ) { stringResult += stringCommentMarker; } // Add comment marker if needed
      stringResult += stringText.substr(uStart, uEnd - uStart);               // Add the line content
      if( bHasNewline == true )                                               // Add newline if it was present in original
      {
         stringResult += iNewLine;
         uStart = uEnd + 1;
      }
      else { break; }
      bFirstLine = false;
   }
   return stringResult;
}


/** ---------------------------------------------------------------------------
 * @brief Creates a formatted header line with specified characters, total length, and text alignment.
 *
 * This function generates a header line with configurable alignment for the header text.
 * The structure varies based on alignment:
 * - LEFT: first_char + " " + header_name + " " + fill_chars + last_char
 * - CENTER: first_char + fill_chars + " " + header_name + " " + fill_chars + last_char
 * - RIGHT: first_char + fill_chars + " " + header_name + " " + last_char
 * 
 * If the header name is too long to fit within the specified length, it will be truncated.
 * Minimum length is 4 characters to accommodate the basic structure.
 *
 * @param stringHeaderName The header text to display.
 * @param eAlignment The text alignment (LEFT, CENTER, or RIGHT).
 * @param uTotalLength The total length of the generated line (default: 70).
 * @param iFirstChar The character to use at the beginning of the line (default: '+').
 * @param iFillChar The character to use for padding (default: '-').
 * @param iLastChar The character to use at the end of the line (default: '+').
 * @return std::string The formatted header line.
 * 
 * @code
 * // Center aligned (original behavior)
 * std::string center = format_header_line("Configuration", TextAlignment::CENTER, 60, '+', '-', '+');
 * // Result: "+----------- Configuration -----------+"
 *
 * // Left aligned
 * std::string left = format_header_line("Configuration", TextAlignment::LEFT, 60, '+', '-', '+');
 * // Result: "+- Configuration -----------------------+"
 *
 * // Right aligned
 * std::string right = format_header_line("Configuration", TextAlignment::RIGHT, 60, '+', '-', '+');
 * // Result: "+---------------------- Configuration -+"
 * @endcode
 */
std::string format_header_line(const std::string_view& stringHeaderName, enumAlignment eAlignment, size_t uTotalLength, char iFirstChar, char iFillChar, char iLastChar)
{
    if(uTotalLength < 6) { uTotalLength = 6; }                                  // Handle minimum length requirements

    auto add_text = [](std::string& s_, const std::string_view& text_, char iFillChar) {
       if( text_.empty() == true ) { s_.append(4, iFillChar); return; }                                            // No text to add, return early
       s_ += iFillChar; // Add fill character before text
       s_ += ' '; 
       s_ += text_;
       s_ += ' ';
       s_ += iFillChar; // Add fill character after text
    };
    
    std::string stringResult;
    stringResult.reserve(uTotalLength);
    
    // ## Calculate available space for header and fill characters
    //    Structure varies by alignment, but we need: first_char + 2 spaces + last_char = 4 minimum
    size_t uHeaderSpace = uTotalLength - 6; // Subtract: first_char + 2 spaces + last_char
    
    std::string stringHeader(stringHeaderName);
    
    if(stringHeader.length() > uHeaderSpace) { stringHeader = stringHeader.substr(0, uHeaderSpace); } // Truncate header if it's too long to fit
    
    // ## Calculate fill characters needed
    size_t uTotalFill = uHeaderSpace - stringHeader.length();
    
    stringResult += iFirstChar;                                               // Add first character

    switch(eAlignment)
    {
        case enumAlignment::eAlignmentLeft:
        {
            // Structure: first_char + " " + header + " " + fill_chars + last_char

            add_text(stringResult, stringHeader, iFillChar);                  // Add header
            
            // Add all remaining fill characters
            for(size_t u = 0; u < uTotalFill; ++u) { stringResult += iFillChar; }
            break;
        }

        case enumAlignment::eAlignmentRight:
        {
            // Structure: first_char + fill_chars + " " + header + " " + last_char
            // Add all fill characters first
            for(size_t u = 0; u < uTotalFill; ++u) { stringResult += iFillChar; }
            
            add_text(stringResult, stringHeader, iFillChar);                  // Add header
            break;
        }

        case enumAlignment::eAlignmentCenter:
        default:
        {
            // Structure: first_char + fill_chars + " " + header + " " + fill_chars + last_char
            // Split fill characters as evenly as possible
            size_t uLeftFill = uTotalFill / 2;
            size_t uRightFill = uTotalFill - uLeftFill;
            
            // Add left fill characters
            for(size_t u = 0; u < uLeftFill; ++u) { stringResult += iFillChar; }

            add_text(stringResult, stringHeader, iFillChar);                  // Add header
            // Add right fill characters
            for(size_t u = 0; u < uRightFill; ++u) { stringResult += iFillChar; }
            break;
        }
    }
    
    stringResult += iLastChar; // Last character
    
    return stringResult;
}

/** ---------------------------------------------------------------------------
 * @brief Creates a formatted header line with specified characters and total length.
 *
 * This function generates a header line in the format: first_char + fill_char + " " + header_name + " " + fill_chars + last_char.
 * The header name is padded with spaces and surrounded by fill characters to reach the specified total length.
 * If the header name is too long to fit within the specified length, it will be truncated.
 * Minimum length is 4 characters to accommodate the basic structure.
 *
 * @param stringHeaderName The header text to display in the center.
 * @param uTotalLength The total length of the generated line (default: 70).
 * @param iFirstChar The character to use at the beginning of the line (default: '+').
 * @param iFillChar The character to use for padding (default: '-').
 * @param iLastChar The character to use at the end of the line (default: '+').
 * @return std::string The formatted header line.
 * 
 * @code
 * std::string header = format_header_line("Configuration", 60, '+', '-', '+');
 * // Result: "+- Configuration ---------------------------------------------+"
 *
 * std::string header2 = format_header_line("Short", 20, '[', '=', ']');
 * // Result: "[= Short ==========]"
 * @endcode
 */
std::string format_header_line(const std::string_view& stringHeaderName, size_t uTotalLength, char iFirstChar, char iFillChar, char iLastChar)
{
   return format_header_line(stringHeaderName, enumAlignment::eAlignmentLeft, uTotalLength, iFirstChar, iFillChar, iLastChar);
}

/// Overloaded to format line from string
std::string format_header_line(const std::string_view& stringHeaderName, enumAlignment eAlignment, size_t uTotalLength, std::string_view stringLine ) 
{
   uint8_t puLine[3] = { '+', '-', '+' }; // Default characters for first, fill, and last

   if( stringLine.length() == 1 )                                              // same character for first, fill, and last
   {
      puLine[0] = stringLine[0]; // First character
      puLine[1] = stringLine[0]; // Fill character
      puLine[2] = stringLine[0]; // Last character
   }
   else if( stringLine.length() == 2 )                                         // two characters, first and last
   {
      puLine[0] = stringLine[0]; // First character
      puLine[2] = stringLine[1]; // Last character
   }
   else if( stringLine.length() > 2 )
   {
      puLine[0] = stringLine[0]; // First character
      puLine[1] = stringLine[1]; // Fill character
      puLine[2] = stringLine[2]; // Last character
   }

   return format_header_line(stringHeaderName, eAlignment, uTotalLength, puLine[0], puLine[1], puLine[2]);
}


/** ---------------------------------------------------------------------------
 * @brief Formats text to fit within a specified width, filling with a character.
 *
 * This function takes a string and formats it to fit within a specified width.
 * If the text exceeds the width, it wraps to the next line. Each line is filled
 * with a specified character until it reaches the desired width. Newlines in the
 * original text are preserved, and the first line is treated specially if needed.
 *
 * @param stringText The source text to format.
 * @param uWidth The desired width of each line (minimum 1).
 * @param iFillChar The character used to fill remaining space in each line (default: ' ').
 * @return std::string The formatted text with lines adjusted to the specified width.
 * 
 * @code
 * std::string text = "This is a long text that needs to be wrapped and formatted.";
 * std::string result = format_text_width(text, 20, '-');
 * // result contains formatted lines with '-' filling up to 20 characters
 * @endcode
 */
std::string format_text_width(std::string_view stringText, size_t uWidth, char iFillChar)
{
   assert(stringText.empty() == false); assert(uWidth > 0);

   std::string stringResult;
   stringResult.reserve(stringText.size() * 2); // More reasonable reserve size

   std::string stringLine; // Current line being processed
   bool bFirstLine = true; // Flag to track if we are processing the first line

   // ## Process each line from the original text
   while( getline(stringText, stringLine) == true ) 
   {
      if(bFirstLine == false) { stringResult += '\n'; }
      bFirstLine = false;

      // ### Handle empty lines
      if( stringLine.empty() == true ) { stringResult.append(uWidth, iFillChar); continue; }

      // ## Word wrap the current line
      std::istringstream stringLineStream(stringLine);
      std::string stringWord;
      std::string stringCurrentLine;

      while(stringLineStream >> stringWord) 
      {
         // ## Check if adding this word would exceed width
         size_t uNeededSpace = stringWord.length();
         if(stringCurrentLine.empty() == false) 
         {
            uNeededSpace += 1; // Space before word
         }

         if(stringCurrentLine.length() + uNeededSpace > uWidth) 
         {
            if(stringCurrentLine.empty() == false) 
            {
               // ## Pad current line and add to result
               stringCurrentLine.append(uWidth - stringCurrentLine.length(), iFillChar);
               stringResult += stringCurrentLine + '\n';
               stringCurrentLine.clear();
            }

            // ## Handle words longer than width
            while(stringWord.length() > uWidth) 
            {
               stringResult += stringWord.substr(0, uWidth) + '\n';
               stringWord = stringWord.substr(uWidth);
            }
            stringCurrentLine = stringWord;
         } 
         else 
         {
            if(stringCurrentLine.empty() == false) 
            {
               stringCurrentLine += ' ';
            }
            stringCurrentLine += stringWord;
         }
      }

      // ## Handle the last line segment
      if(stringCurrentLine.empty() == false) 
      {
         stringCurrentLine.append(uWidth - stringCurrentLine.length(), iFillChar);
         stringResult += stringCurrentLine;
      }
   }

   return stringResult;
}



/** ---------------------------------------------------------------------------
 * @brief Removes consecutive duplicate characters beyond a specified limit.
 *
 * This function processes the input string and removes consecutive occurrences
 * of the same character that exceed the specified maximum count. For example,
 * if uMaxRepeated is 2 and the string contains "aaaa", it will be trimmed
 * to "aa". The function preserves the order of characters and only removes
 * excess consecutive duplicates.
 *
 * @param stringText The source string to process.
 * @param uMaxRepeated Maximum allowed consecutive occurrences of the same character (default: 2).
 * @return std::string A new string with excess consecutive duplicates removed.
 * 
 * @code
std::string text = "aaabbbccccdddd";
std::string result = trim_repeated_chars(text, 2);
// result contains "aabbccdd"
 * @endcode
 */
std::string trim_repeated_chars(const std::string_view& stringText, size_t uMaxRepeated)
{                                                                                                  assert( uMaxRepeated > 0 );
   std::string stringResult;
   stringResult.reserve(stringText.length()); // Reserve space to avoid reallocations

   char iPreviousChar = '\0';     // Previous character processed
   size_t uCount = 0;             // Count of repeated identical characters

   for(char iCurrentChar : stringText) 
   {
      // ## Check if current character is the same as previous
      if(iCurrentChar == iPreviousChar) 
      {
         uCount++;

         if(uCount <= uMaxRepeated) { stringResult += iCurrentChar;  }         // Only add character if within the allowed limit
      }
      else 
      {
         // ### New character found, reset counter and add character
         iPreviousChar = iCurrentChar;
         uCount = 1;
         stringResult += iCurrentChar;
      }
   }

   return stringResult;
}


/** ---------------------------------------------------------------------------
 * @brief Converts a hexadecimal string to its ASCII representation.
 *
 * This function takes a string containing hexadecimal characters (0-9, A-F) and converts
 * it to its ASCII representation. Each pair of hex characters is converted to a single ASCII character.
 * If the input string length is not even, it asserts an error condition.
 *
 * @param stringHex The input hexadecimal string to convert.
 * @return std::string The resulting ASCII string.
 * 
 * @code
 * std::string hex = "48656c6c6f20576f726c64"; // "Hello World" in hex
 * std::string ascii = convert_hex_to_ascii(hex);
 * // ascii contains "Hello World"
 * @endcode
 */
std::string convert_hex_to_ascii(const std::string_view& stringHex)
{                                                                                   assert(stringHex.empty() == false); assert(stringHex.length() % 2 == 0);
   char piszHex[3] = { '\0', '\0', '\0' }; // Buffer for 2 hex characters + null terminator
   std::string stringResult;
   stringResult.reserve(stringHex.length() / 2); // Reserve space for ASCII characters

   for(size_t u = 0; u < stringHex.length(); u += 2) 
   {
      piszHex[0] = stringHex[u];
      piszHex[1] = stringHex[u + 1];

      try 
      {
         // Convert hex pair to unsigned char first to handle full 0-255 range
         unsigned char uByte = static_cast<unsigned char>(std::stoi(piszHex, nullptr, 16));
         stringResult += static_cast<char>(uByte);
      }
      catch(const std::exception&) 
      {
         assert(false && "Invalid hex character encountered");
         return {}; // Return empty string on error
      }
   }

   return stringResult;
}

/** ---------------------------------------------------------------------------
 * @brief Merges two delimited strings, removing duplicates and preserving the separator.
 *
 * This function combines two strings containing values separated by a delimiter character,
 * removes duplicate values, and returns a single string with unique values using the same
 * separator. The order of values is preserved from the first string, followed by unique
 * values from the second string. Empty values are preserved if present in the input.
 *
 * @param stringFirst The first delimited string to merge.
 * @param stringSecond The second delimited string to merge.
 * @param iSeparator The delimiter character used to separate values (e.g., ';', ',').
 * @return std::string The merged string containing unique values separated by the same delimiter.
 * 
 * @code
 * // Example 1: Basic merge with semicolon separator
 * std::string first = "apple;banana;cherry";
 * std::string second = "banana;date;apple;elderberry";
 * std::string result = merge_delimited(first, second, ';');
 * // result = "apple;banana;cherry;date;elderberry"
 * 
 * // Example 2: Comma-separated values
 * std::string list1 = "red,green,blue";
 * std::string list2 = "blue,yellow,red,purple";
 * std::string merged = merge_delimited(list1, list2, ',');
 * // merged = "red,green,blue,yellow,purple"
 * 
 * // Example 3: With empty values preserved
 * std::string str1 = "a;;b;c";
 * std::string str2 = ";b;d;";
 * std::string combined = merge_delimited(str1, str2, ';');
 * // combined = "a;;b;c;d;"
 * 
 * // Example 4: No duplicates case
 * std::string unique1 = "x;y;z";
 * std::string unique2 = "a;b;c";
 * std::string final = merge_delimited(unique1, unique2, ';');
 * // final = "x;y;z;a;b;c"
 * @endcode
 */
std::string merge_delimited(const std::string_view& stringFirst, const std::string_view& stringSecond, char iSeparator)
{
   std::vector<std::string_view> vectorFirst;                                 // Vector to hold first string values
   std::vector<std::string_view> vectorSecond;                                // Vector to hold second string values
   if( stringFirst.empty() == false ) vectorFirst = gd::utf8::split(stringFirst, iSeparator); // Split first string into vector
   if( stringSecond.empty() == false ) vectorSecond = gd::utf8::split(stringSecond, iSeparator); // Split second string into vector

   auto vectorUnique = vectorFirst;                                           // Copy first vector for unique processing

   for( const auto& stringValue : vectorSecond )
   {
      if( std::find(vectorUnique.begin(), vectorUnique.end(), stringValue) == vectorUnique.end() )
      {
         vectorUnique.push_back(stringValue);
      }
   }

   // ## Build the result string from the unique vector
   std::string stringResult;
   size_t uTotalSize = 0;
   for( const auto& stringValue : vectorUnique ) { uTotalSize += stringValue.size() + 1; } // Estimate size including separators
   stringResult.reserve(uTotalSize);                                            // Reserve estimated size for performance
   for( size_t uIndex = 0; uIndex < vectorUnique.size(); ++uIndex )
   {
      stringResult += vectorUnique[uIndex];
      if( uIndex < vectorUnique.size() - 1 ) { stringResult += iSeparator; } // Add separator except for last element
   }
   
   return stringResult;
}

/// Overloaded to merge a vector of strings with a separator
std::string merge_delimited(const std::vector<std::string_view> vectorString, char iSeparator)
{
   if( vectorString.empty() == true ) { return {}; }                          // Handle empty input case

   std::string stringResult( vectorString[0] );

   for( size_t uIndex = 1; uIndex < vectorString.size(); ++uIndex )
   {
      auto string_ = vectorString[uIndex]; // Current string to merge
      if( string_.empty() == true ) { continue; }                             // Skip empty strings
      stringResult = merge_delimited(stringResult, vectorString[uIndex], iSeparator); // Merge each string with the result
   }
   
   return stringResult;
}


_GD_MATH_STRING_END