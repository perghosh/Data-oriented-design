/**
 * @file gd_parse_match_pattern.h
 */


#pragma once

#include <array>
#include <cassert>
#include <cstring>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_types.h"


#ifndef _GD_PARSE_BEGIN
#  define _GD_PARSE_BEGIN namespace gd { namespace parse {
#  define _GD_PARSE_END } }
#endif

_GD_PARSE_BEGIN



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
class patterns
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
    * if(pattern.compare(input)) {
    *     std::cout << "Input matches the pattern." << std::endl;
    * }
    * @endcode
    */
   struct pattern
   {
   // ## construction -------------------------------------------------------------
      pattern() {}
      pattern(const std::string_view& stringPattern) : m_stringPattern(stringPattern) {}
      pattern(const std::string& stringPattern) : m_stringPattern(stringPattern) {}
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

   // ## operator -----------------------------------------------------------------
      bool operator==(const pattern& o) const { return m_stringPattern == o.m_stringPattern; }
      bool operator==(const std::string_view& stringPattern) const { return m_stringPattern == stringPattern; }
      
      /// convert to uint8_t, this will return the first character in the pattern string and is used to identify markers
      operator uint8_t() const { return static_cast<uint8_t>(m_stringPattern[0]); } 
      operator std::string_view() const { return m_stringPattern; } ///< convert to string_view

      const std::string& get_pattern() const { return m_stringPattern; } ///< get pattern string
      const std::string& get_escape() const { return m_stringEscape; } ///< get escape string

      size_t length() const { return m_stringPattern.length(); } ///< get length of pattern string

      /// compares if equal to pattern string and only the sequence of characters in the pattern string
      bool compare(const char* piText) const {
         if(std::strncmp(piText, m_stringPattern.c_str(), m_stringPattern.length()) == 0) {
            return true;
         }
         return false;
      }

      /// check if text is escaped, this will check if the text is escaped with the escape character
      bool is_escaped(const char* piText) const {
         if(m_stringEscape.empty()) return false; // no escape character
         if(std::strncmp(piText - m_stringEscape.length(), m_stringEscape.c_str(), m_stringEscape.length()) == 0) {
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
   using value_type              = pattern;                        ///< type of value stored in vector
   using iterator                = std::vector<pattern>::iterator; ///< iterator type
   using const_iterator          = std::vector<pattern>::const_iterator; ///< const iterator type
   using reverse_iterator        = std::vector<pattern>::reverse_iterator; ///< reverse iterator type
   using const_reverse_iterator  = std::vector<pattern>::const_reverse_iterator; ///< const reverse iterator type
   using size_type               = std::vector<pattern>::size_type; ///< size type
   using difference_type         = std::vector<pattern>::difference_type; ///< difference type


// ## construction -------------------------------------------------------------
public:
   patterns() { m_arrayMarkerHint = { 0 }; }
   patterns( const std::vector<std::string>& vectorPattern); ///< construct with vector of strings
   // copy
   patterns(const patterns& o) { common_construct(o); }
   patterns(patterns&& o) noexcept { common_construct(std::move(o)); }
   // assign
   patterns& operator=(const patterns& o) { common_construct(o); return *this; }
   patterns& operator=(patterns&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~patterns() {}
private:
   // common copy
   void common_construct(const patterns& o) {
       m_vectorPattern = o.m_vectorPattern;
       m_arrayMarkerHint = o.m_arrayMarkerHint;
   }
   void common_construct(patterns&& o) noexcept {
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

   void sort(); ///< sort vector of patterns based on length

   void clear() { m_vectorPattern.clear(); m_arrayMarkerHint = { 0 }; } ///< clear vector of patterns
   bool empty() const { return m_vectorPattern.empty(); } ///< check if vector of patterns is empty
   size_t size() const { return m_vectorPattern.size(); } ///< get size of vector of patterns

   /// check if text matches any pattern in the vector
   bool exists(const char* piText) const;
   bool exists(const uint8_t* puText) const { return exists(reinterpret_cast<const char*>(puText)); }

   /// Try to find the pattern in the text
   int find_pattern(const char* piText, size_t uLength, uint64_t* puOffset = nullptr ) const;
   /// Overloaded function to find pattern
   int find_pattern(const char* piBegin, const char* piEnd, uint64_t* puOffset = nullptr) const { return find_pattern(piBegin, size_t(piEnd - piBegin), puOffset); } ///< find pattern in text
   /// Overloaded function to find pattern in std::string_view
   int find_pattern(const std::string_view& stringText, uint64_t* puOffset = nullptr) const { return find_pattern(stringText.data(), stringText.length(), puOffset); } ///< find pattern in text

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
   int find_(const uint8_t* puBegin, const uint8_t* puEnd ) const;            ///< find pattern in text, not optimized
   int find_(const char* piBegin, const char* piEnd) const { return find_(reinterpret_cast<const uint8_t*>( piBegin ), reinterpret_cast<const uint8_t*>( piEnd ) ); } ///< find pattern in text, not optimized  

// ## attributes ----------------------------------------------------------------
public:
   /// Characters to look for to investigate if they are part of pattern changing markers
   std::array<uint8_t, 256> m_arrayMarkerHint;
   std::vector<pattern> m_vectorPattern; ///< vector of patterns to use when matching strings
};

/**
 * @brief Check if text matches any pattern in the vector
 * @param piText Pointer to the text to check
 * @return True if the text matches any pattern, false otherwise
 */
inline bool patterns::exists(const char* piText) const {                                           assert(piText != nullptr);
if( m_arrayMarkerHint[static_cast<uint8_t>( *piText )] == 0 ) return false;   // no match found
   for (const auto& it : m_vectorPattern) {
      // Compare with specified length
      if(it.compare(piText) == true) { return true; }
   }
   return false;
}

/// check if text is escaped
inline bool patterns::is_escaped(const char* piText, size_t uIndex) const {
   if(uIndex >= m_vectorPattern.size()) return false;
   return m_vectorPattern[uIndex].is_escaped(piText);
}

_GD_PARSE_END