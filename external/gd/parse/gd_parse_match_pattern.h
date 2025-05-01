/**
 * @file gd_parse_match_pattern.h
 */


#pragma once

#include <cassert>
#include <cstring>
#include <span>
#include <string>
#include <string_view>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_BEGIN
#  define _GD_PARSE_BEGIN namespace gd { namespace parse {
#  define _GD_PARSE_END } }
#  define _GD_PARSE_WINDOW_BEGIN namespace gd { namespace parse { namespace window {
#  define _GD_PARSE_WINDOW_END } } }
#endif

_GD_PARSE_WINDOW_BEGIN



/**
 * @class pattern
 * @brief A lightweight utility class for matching and comparing string patterns.
 *
 * The `pattern` class provides functionality to match and compare string 
 * patterns against input text. It maintains a collection of pattern rules and 
 * provides methods to check if input text matches any of the stored patterns.
 *
 * ### Features:
 * - Simple pattern matching for strings
 * - Efficient lookup with character hint array
 * - Support for escaped sequences
 * - Methods to check if text matches any stored pattern
 */
class pattern
{
public:
   /**
    * @struct pattern
    * @brief Represents a string pattern with optional escape sequence.
    *
    * The `pattern` struct defines a string to match and an optional escape sequence.
    * It provides methods to compare input text with the pattern and check if text is escaped.
    *
    * ### Features:
    * - Defines a pattern with a string and optional escape sequence
    * - Provides comparison methods for matching input text with the pattern
    * - Supports equality comparison between patterns and strings
    *
    * ### Example Usage:
    * @code
    * pattern::pattern pattern("\"", "\\");
    *
    * const char* input = "\"Hello, World!\"";
    *
    * // Check if the input matches the pattern
    * if (pattern.compare(input)) {
    *     std::cout << "Input matches the pattern." << std::endl;
    * }
    * @endcode
    */
   struct pattern
   {
      // ## construction -------------------------------------------------------------
      pattern() {}
      pattern(const std::string_view& stringPattern)
         : m_stringPattern(stringPattern) {}
      pattern(const std::string& stringPattern)
         : m_stringPattern(stringPattern) {}
      pattern(const std::string_view& stringPattern, const std::string_view& stringEscape)
         : m_stringPattern(stringPattern), m_stringEscape(stringEscape) {}
      pattern(const pattern& o): m_stringPattern(o.m_stringPattern), m_stringEscape(o.m_stringEscape) {}
      pattern(pattern&& o) noexcept : m_stringPattern(std::move(o.m_stringPattern)), m_stringEscape(std::move(o.m_stringEscape)) {}
      pattern& operator=(const pattern& o) {
         assert(this != &o); m_stringPattern = o.m_stringPattern; m_stringEscape = o.m_stringEscape;
         return *this;
      }
      pattern& operator=(pattern&& o) noexcept {
         assert(this != &o); m_stringPattern = std::move(o.m_stringPattern); m_stringEscape = std::move(o.m_stringEscape);
         return *this;
      }
      ~pattern() {}
      bool operator==(const pattern& o) const { return m_stringPattern == o.m_stringPattern; }
      bool operator==(const std::string_view& stringPattern) const { return m_stringPattern == stringPattern; }
      
      /// convert to uint8_t, this will return the first character in the pattern string and is used to identify markers
      operator uint8_t() const { return static_cast<uint8_t>(m_stringPattern[0]); } 

      const std::string& get_pattern() const { return m_stringPattern; } ///< get pattern string
      const std::string& get_escape() const { return m_stringEscape; } ///< get escape string

      /// compares if equal to pattern string and only the sequence of characters in the pattern string
      bool compare(const char* piText) const {
         if (std::strncmp(piText, m_stringPattern.c_str(), m_stringPattern.length()) == 0) {
            return true;
         }
         return false;
      }

      /// check if text is escaped, this will check if the text is escaped with the escape character
      bool is_escaped(const char* piText) const {
         if (m_stringEscape.empty()) return false; // no escape character
         if (std::strncmp(piText - m_stringEscape.length(), m_stringEscape.c_str(), m_stringEscape.length()) == 0) {
            return true;
         }
         return false;
      }

      // ## attributes
      std::string m_stringPattern; ///< pattern string to match
      std::string m_stringEscape; ///< escape character sequence
   };

// ## types ------------------------------------------------------------------
public:
   using value_type = pattern; ///< type of value stored in vector
   using iterator = std::vector<pattern>::iterator; ///< iterator type
   using const_iterator = std::vector<pattern>::const_iterator; ///< const iterator type
   using reverse_iterator = std::vector<pattern>::reverse_iterator; ///< reverse iterator type
   using const_reverse_iterator = std::vector<pattern>::const_reverse_iterator; ///< const reverse iterator type
   using size_type = std::vector<pattern>::size_type; ///< size type
   using difference_type = std::vector<pattern>::difference_type; ///< difference type

// ## construction -------------------------------------------------------------
public:
   pattern() { m_arrayMarkerHint = { 0 }; }
   // copy
   pattern(const pattern& o) { common_construct(o); }
   pattern(pattern&& o) noexcept { common_construct(std::move(o)); }
   // assign
   pattern& operator=(const pattern& o) { common_construct(o); return *this; }
   pattern& operator=(pattern&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~pattern() {}
private:
   // common copy
   void common_construct(const pattern& o) {
       m_vectorPattern = o.m_vectorPattern;
       m_arrayMarkerHint = o.m_arrayMarkerHint;
   }
   void common_construct(pattern&& o) noexcept {
       m_vectorPattern = std::move(o.m_vectorPattern);
       m_arrayMarkerHint = std::move(o.m_arrayMarkerHint);
   }

// ## operator -----------------------------------------------------------------
public:
   uint8_t operator[](uint8_t iIndex) const { return m_arrayMarkerHint[iIndex]; } ///< access marker hint

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   const std::vector<pattern>& get_patterns() const { return m_vectorPattern; } ///< get vector of patterns
   const std::array<uint8_t, 256>& get_marker_hint() const { return m_arrayMarkerHint; } ///< get marker hint
   const pattern& get_pattern(size_t uIndex) const { assert(uIndex < m_vectorPattern.size()); return m_vectorPattern[uIndex]; } ///< get pattern at index
//@}

/** \name OPERATION
*///@{
   // ## add patterns
   void add(const pattern& o) { m_vectorPattern.push_back(o); add_marker_hint(o); } ///< add pattern to vector
   
   void add(const std::string_view& stringPattern) {
      m_vectorPattern.emplace_back(pattern(stringPattern));
      add_marker_hint(stringPattern[0]);
   }
   
   void add(const std::string_view& stringPattern, const std::string_view& stringEscape) {
      m_vectorPattern.emplace_back(pattern(stringPattern, stringEscape));
      add_marker_hint(stringPattern[0]);
   }

   void clear() { m_vectorPattern.clear(); m_arrayMarkerHint = { 0 }; } ///< clear vector of patterns
   bool empty() const { return m_vectorPattern.empty(); } ///< check if vector of patterns is empty
   size_t size() const { return m_vectorPattern.size(); } ///< get size of vector of patterns

   /**
    * @brief Check if text matches any pattern in the vector
    * @param piText Pointer to the text to check
    * @return True if the text matches any pattern, false otherwise
    */
   bool exists(const char* piText) const;
   
   /**
    * @brief Check if text matches any pattern in the vector
    * @param puText Pointer to the text as unsigned char to check
    * @return True if the text matches any pattern, false otherwise
    */
   bool exists(const uint8_t* puText) const { return exists(reinterpret_cast<const char*>(puText)); }

   /**
    * @brief Find the matching pattern for the given text
    * @param piText Pointer to the text to check
    * @return Index of the matching pattern, or SIZE_MAX if no match found
    */
   size_t find_match(const char* piText) const;
   
   /**
    * @brief Find the matching pattern for the given text
    * @param puText Pointer to the text as unsigned char to check
    * @return Index of the matching pattern, or SIZE_MAX if no match found
    */
   size_t find_match(const uint8_t* puText) const { return find_match(reinterpret_cast<const char*>(puText)); }

   /**
    * @brief Get the length of the matching pattern
    * @param piText Pointer to the text to check
    * @return Length of the matching pattern, or 0 if no match found
    */
   size_t match_length(const char* piText) const;

   /**
    * @brief Get the length of the matching pattern
    * @param puText Pointer to the text as unsigned char to check
    * @return Length of the matching pattern, or 0 if no match found
    */
   size_t match_length(const uint8_t* puText) const { return match_length(reinterpret_cast<const char*>(puText)); }

   /**
    * @brief Check if text is escaped
    * @param piText Pointer to the text to check
    * @param uIndex Index of the pattern to check
    * @return True if the text is escaped, false otherwise
    */
   bool is_escaped(const char* piText, size_t uIndex) const;

   // ## iterator methods
   iterator begin() { return m_vectorPattern.begin(); } ///< iterator to start of vector
   iterator end() { return m_vectorPattern.end(); } ///< iterator to end of vector
   const_iterator begin() const { return m_vectorPattern.begin(); } ///< const iterator to start of vector
   const_iterator end() const { return m_vectorPattern.end(); } ///< const iterator to end of vector
   const_iterator cbegin() const { return m_vectorPattern.cbegin(); } ///< const iterator to start of vector
   const_iterator cend() const { return m_vectorPattern.cend(); } ///< const iterator to end of vector
//@}

private:
   /// add marker hint, this will add a character to the list of characters that are used to identify pattern changing markers
   void add_marker_hint(uint8_t uCharacter) { m_arrayMarkerHint[uCharacter] = 1; }
   void add_marker_hint(char iCharacter) { add_marker_hint(uint8_t(iCharacter)); }

// ## attributes ----------------------------------------------------------------
public:
   /// Characters to look for to investigate if they are part of pattern changing markers
   std::array<uint8_t, 256> m_arrayMarkerHint;
   std::vector<pattern> m_vectorPattern; ///< vector of patterns to use when matching strings
};

/// check if text matches any pattern in the vector
inline bool pattern::exists(const char* piText) const {
   if (piText == nullptr) return false;
   for (const auto& it : m_vectorPattern) {
      // Compare with specified length
      if (it.compare(piText) == true) { return true; }
   }
   return false;
}

/// find the matching pattern for the given text
inline size_t pattern::find_match(const char* piText) const {                                      asset(piText != nullptr);
   for (size_t i = 0; i < m_vectorPattern.size(); ++i) {
      if (m_vectorPattern[i].compare(piText) == true) {
         return i;
      }
   }
   return SIZE_MAX; // No match found
}

/// get the length of the matching pattern
inline size_t pattern::match_length(const char* piText) const {
   size_t index = find_match(piText);
   if (index == SIZE_MAX) return 0;
   return m_vectorPattern[index].m_stringPattern.length();
}

/// check if text is escaped
inline bool pattern::is_escaped(const char* piText, size_t uIndex) const {
   if (uIndex >= m_vectorPattern.size()) return false;
   return m_vectorPattern[uIndex].is_escaped(piText);
}

_GD_PARSE_END