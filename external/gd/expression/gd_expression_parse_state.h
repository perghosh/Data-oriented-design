/**
 * @file gd_expression_parse_state.h
 * @brief Defines the `state` class and its associated `rule` struct for managing parsing states in expressions.
 * @TAG #gd::expression
 *
 * This file provides the implementation of a state machine for parsing expressions. It includes:
 * - The `state` class, which manages a collection of parsing rules and provides methods to activate, deactivate, and query states.
 * - The `rule` struct, which defines individual parsing rules with start, end, and escape sequences.
 *
 * ### Key Features:
 * - Supports multiple parsing states such as comments, strings, numbers, and operators.
 * - Allows adding custom rules for parsing expressions.
 * - Provides efficient state transitions using marker hints.
 * - Includes utility methods for comparing input text with parsing rules.
 *
 * ### Example Usage:
 * @code
 * #include "gd_expression_parse_state.h"
 * 
 * using namespace gd::expression::parse;
 * 
 * int main() {
 *     state stateParse;
 * 
 *     // Add rules for parsing
 *     stateParse.add("STRING", "\"", "\""); // Rule for string literals
 *     stateParse.add("LINECOMMENT", "//", "\n"); // Rule for line comments
 * 
 *     // Activate a state based on input text
 *     const char* input = "\"Hello, World!\"";
 *     size_t length = stateParse.activate(input);
 * 
 *     // Check if the parser is in a state
 *     if (stateParse.in_state()) {
 *         std::cout << "Parser is in state: " << stateParse.get_state() << std::endl;
 *     }
 * 
 *     // Deactivate the state
 *     unsigned endLength = 0;
 *     if (parserState.deactivate(input + length, &endLength)) {
 *         std::cout << "State deactivated successfully." << std::endl;
 *     }
 * 
 *     return 0;
 * }
 * @endcode
 *
 * ### Dependencies:
 * - `gd_expression.h`: Provides core expression-related functionality.
 *
 * ### Namespace:
 * All functionality is encapsulated within the `gd::expression::parse` namespace.
 *
 * ### Notes:
 * - This file is part of a larger parsing framework and is designed to work with other components in the `gd` library.
 * - The file uses C++20 features and assumes a modern compiler.
 */

#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>


#include "gd_expression.h"

#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( _MSC_VER )
   #pragma warning(push)
#endif

#ifndef _GD_EXPRESSION_PARSE_BEGIN
#define _GD_EXPRESSION_PARSE_BEGIN namespace gd { namespace expression { namespace parse {
#define _GD_EXPRESSION_PARSE_END } } }
#endif

_GD_EXPRESSION_PARSE_BEGIN


/**
 * @class state
 * @brief Represents a parsing state machine for managing and transitioning between different parsing states.
 *
 * The `state` class is designed to handle parsing logic by maintaining a collection of rules, 
 * each defining a specific parsing state and its associated start, end, and escape sequences. 
 * It provides methods to activate, deactivate, and query states based on input text.
 *
 * ### Features:
 * - Supports multiple parsing states such as comments, strings, numbers, and operators.
 * - Allows adding custom rules for parsing.
 * - Provides methods to activate and deactivate states based on input text.
 * - Maintains a marker hint array for efficient state transitions.
 */
class state
{
public:
   /// @brief Tag for manual processing to fix internal state issues. 
   struct tag_manual {};
public:
   
   /**
    * @brief Enumeration for parsing state numbers.
    * 
    * This enumeration defines various parsing state numbers used to identify different states
    */
   enum enumStateNumber
   {
      eStateNumberNone = 0,         ///< no state
      eStateNumberLineComment,      ///< comment state
      eStateNumberWhitespace,       ///< whitespace state
      eStateNumberString,           ///< string state
      eStateNumberNumber,           ///< number state
      eStateNumberIdentifier,       ///< identifier state
      eStateNumberOperator,         ///< operator state
      eStateNumberEnd,              ///< end of expression
      // multiline states        
      eStateNumberBlockComment,     ///< block comment state
      eStateNumberRawString,        ///< raw string state
      eStateNumberScriptCode,       ///< script that differs from the rest

      // States for plain text parsing can be added here
      eStateNumberText,             ///< plain text state
      eStateNumberHeading,          ///< heading state 
      eStateNumberTable,            ///< table state
      eStateNumberSummary,          ///< summary state
      eStateNumberConfiguration,    ///< configuration state
   };


   /// @brief Enumeration for group parsing state.
   enum enumGroup
   {
      eGroupNone = 0x0000,       ///< no group
      eGroupComment = 0x0100,    ///< comment group
      eGroupString = 0x0200,	   ///< string group
      eGroupOutside = 0x0400,	   ///< outside group, this is used for code that is not in a state at all. Like external code
   };

   /**
    * @brief Enumeration of parsing states.
    * 
    * State is a combination of state number and group.
    * 
    * @note The order of the states is important for determining multiline states.
    */
   enum enumState
   {
      eStateNone          = eStateNumberNone          | eGroupNone,       ///< no state
      eStateLineComment   = eStateNumberLineComment   | eGroupComment,    ///< comment state
      eStateWhitespace    = eStateNumberWhitespace    | eGroupNone,       ///< whitespace state
      eStateString        = eStateNumberString        | eGroupString,     ///< string state
      eStateNumber        = eStateNumberNumber        | eGroupNone,       ///< number state
      eStateIdentifier    = eStateNumberIdentifier    | eGroupNone,       ///< identifier state
      eStateOperator      = eStateNumberOperator      | eGroupNone,       ///< operator state
      eStateEnd           = eStateNumberEnd           | eGroupNone,       ///< end of expression
      // multiline states
      eStateBlockComment  = eStateNumberBlockComment  | eGroupComment,    ///< block comment state
      eStateRawString     = eStateNumberRawString     | eGroupString,     ///< raw string state
      eStateScriptCode    = eStateNumberScriptCode    | eGroupOutside,    ///< script that differs from the rest

	  // States for plain text parsing can be added here
      eStateText          = eStateNumberText          | eGroupOutside,    ///< plain text state
      eStateHeading       = eStateNumberHeading       | eGroupOutside,    ///< heading state 
      eStateTable         = eStateNumberTable         | eGroupOutside,    ///< table state
      eStateSummary       = eStateNumberSummary       | eGroupOutside,    ///< summary state
	  eStateConfiguration = eStateNumberConfiguration | eGroupOutside,    ///< configuration state
   };


   /**
    * @struct rule
    * @brief Represents a parsing rule for managing specific parsing states.
    *
    * The `rule` struct defines the characteristics of a parsing state, including its start, 
    * end, and escape sequences. It provides utility methods to compare input text with 
    * the rule's start and end sequences and to check if the text is escaped.
    *
    * ### Features:
    * - Defines a parsing state with start, end, and optional escape sequences.
    * - Provides comparison methods for matching input text with the rule's sequences.
    * - Supports equality comparison between rules and strings.
    *
    * ### Example Usage:
    * @code
    * #include "gd_expression_parse_state.h"
    * 
    * using namespace gd::expression::parse;
    * 
    * int main() {
    *     // Create a rule for string literals
    *     state::rule rule_(state::eStateString, "\"", "\"", "\\");
    * 
    *     const char* input = "\"Hello, World!\"";
    * 
    *     // Check if the input matches the start of the rule
    *     if (rule_.compare(input)) {
    *         std::cout << "Input matches the start of the rule." << std::endl;
    *     }
    * 
    *     // Check if the input matches the end of the rule
    *     if (rule_.compare_end(input + 13)) {
    *         std::cout << "Input matches the end of the rule." << std::endl;
    *     }
    * 
    *     return 0;
    * }
    * @endcode
    */
   struct rule
   {
      // ## construction -------------------------------------------------------------
      rule() : m_eState(eStateNone) {}
      rule(enumState state, const std::string_view& stringStart, const std::string_view& stringEnd)
         : m_eState(state), m_stringStart(stringStart), m_stringEnd(stringEnd) {}
      rule(enumState state, const std::string& stringStart, const std::string& stringEnd)
         : m_eState(state), m_stringStart(stringStart), m_stringEnd(stringEnd) {}
      rule(enumState state, const std::string_view& stringStart, const std::string_view& stringEnd, const std::string_view& stringEscape) :
         m_eState(state), m_stringStart(stringStart), m_stringEnd(stringEnd), m_stringEscape(stringEscape) {}
      rule(const rule& o): m_eState(o.m_eState), m_stringStart(o.m_stringStart), m_stringEnd(o.m_stringEnd), m_stringEscape(o.m_stringEscape) {}
      rule(rule&& o) noexcept : m_eState(o.m_eState), m_stringStart(std::move(o.m_stringStart)), m_stringEnd(std::move(o.m_stringEnd)), m_stringEscape(std::move(o.m_stringEscape)) {}
      rule& operator=(const rule& o) {
         assert(this != &o); m_eState = o.m_eState; m_stringStart = o.m_stringStart; m_stringEnd = o.m_stringEnd; m_stringEscape = o.m_stringEscape;
         m_vectorConvert = o.m_vectorConvert; // copy conversion rules
         return *this;
      }
      rule& operator=(rule&& o) noexcept { assert(this != &o); 
         m_eState = o.m_eState;  m_stringStart = std::move(o.m_stringStart); m_stringEnd = std::move(o.m_stringEnd); m_stringEscape = std::move(o.m_stringEscape);
         m_vectorConvert = std::move(o.m_vectorConvert); // move conversion rules
         return *this;
      }
      ~rule() {}
      bool operator==(const rule& o) const { return m_eState == o.m_eState && m_stringStart == o.m_stringStart && m_stringEnd == o.m_stringEnd; }
      bool operator==(const std::string_view& stringStart) const { return m_stringStart == stringStart; } ///< compare with start string
      bool operator==(const std::pair<std::string_view, std::string_view>& o) const { return m_stringStart == o.first && m_stringEnd == o.second; } ///< compare with start and end string
      /// convert to uint8_t, this will return the first character in the start string and is used to identify markers
      operator uint8_t() const { return static_cast<uint8_t>( m_stringStart[0] ); } 

      enumState get_state() const { return m_eState; } ///< get state for rule

      const std::string& get_start() const { return m_stringStart; } ///< get start string
      const std::string& get_end() const { return m_stringEnd; } ///< get end string
      const std::string& get_escape() const { return m_stringEscape; } ///< get escape string

      /// compares if equal to start string and only the sequence of characters in the start string (works when parsning)
      bool compare(const char* piText) const {
         if( std::strncmp(piText, m_stringStart.c_str(), m_stringStart.length()) == 0 ) {
            return true;
         }
         return false;
      }

      /// compares if equal to end string and only the sequence of characters in the end string (works when parsning)
      bool compare_end(const char* piText) const {
         if( std::strncmp(piText, m_stringEnd.c_str(), m_stringEnd.length()) == 0 ) {
            return true;
         }
         return false;
      }

      /// check if text is escaped, this will check if the text is escaped with the escape character
      bool is_escaped(const char* piText) const {
         if( m_stringEscape.empty() ) return false; // no escape character
         if( std::strncmp(piText - m_stringEscape.length(), m_stringEscape.c_str(), m_stringEscape.length()) == 0 ) {
            return true;
         }
         return false;
      }

      bool is_escaped_escaped(const char* piText) const {
         if( m_stringEscape.empty() ) return false; // no escape character
         if( std::strncmp(piText - m_stringEscape.length() * 2, m_stringEscape.c_str(), m_stringEscape.length()) == 0 ) {
            return true;
         }
         return false;
      }

      // ## attributes
      enumState m_eState;  ///< state of the rule
      std::string m_stringStart; ///< start of the rule
      std::string m_stringEnd;   ///< end of the rule
      std::string m_stringEscape; ///< escape character
      std::vector< std::pair<std::string, std::string> > m_vectorConvert; ///< vector of conversion rules, this is used to convert character sequences to other character sequences
   };

// ## types ------------------------------------------------------------------
public:
   using value_type              = rule;                        ///< type of value stored in vector
   using iterator                = std::vector<rule>::iterator; ///< iterator type
   using const_iterator          = std::vector<rule>::const_iterator; ///< const iterator type
   using reverse_iterator        = std::vector<rule>::reverse_iterator; ///< reverse iterator type
   using const_reverse_iterator  = std::vector<rule>::const_reverse_iterator; ///< const reverse iterator type
   using size_type               = std::vector<rule>::size_type; ///< size type
   using difference_type         = std::vector<rule>::difference_type; ///< difference type



// ## construction -------------------------------------------------------------
public:
   state() { m_arrayMarkerHint = { 0 }; }
   // copy
   state(const state& o) { common_construct(o); }
   state(state&& o) noexcept { common_construct(std::move(o)); }
   // assign
   state& operator=(const state& o) { common_construct(o); return *this; }
   state& operator=(state&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~state() {}
private:
   // common copy
   void common_construct(const state& o) {}
   void common_construct(state&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   uint8_t operator[](uint8_t iIndex) const { return m_arrayMarkerHint[iIndex]; } ///< access marker hint

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   bool in_state() const { return m_iActive != -1; } ///< check if in state
   /// test character if it is a marker hint, this will return 1 if the character is a marker hint, 0 otherwise
   uint8_t check_marker_hint(uint8_t uCharacter) const { return m_arrayMarkerHint[uCharacter]; } ///< check marker hint for character
   // check if state is multiline, this will return true if the state is a multiline state
   bool is_multiline() const { return (decltype(m_uFirstMultiline_s))get_state() >= m_uFirstMultiline_s; } ///< check if state is multiline
   /// get current state, this will return the state of the active rule if any, otherwise it will crash
   enumState get_state() const { assert( m_iActive != -1 );  return m_vectorRule[m_iActive].get_state(); } ///< get current state
   enumGroup get_group() const { return enumGroup(get_state() & 0xFF00); } ///< get current group
   enumStateNumber get_state_number() const { return enumStateNumber(get_state() & 0x00FF); } ///< get current state number
   void set_state(int64_t iActive) { m_iActive = iActive; } ///< set current state rule
   const std::vector<rule>& get_rule() const { return m_vectorRule; } ///< get vector of rules
   int get_rule_index(const char* piText) const;
   const std::array<uint8_t, 256>& get_marker_hint() const { return m_arrayMarkerHint; } ///< get marker hint
   const rule& get_rule(size_t uIndex) const { assert(uIndex < m_vectorRule.size()); return m_vectorRule[uIndex]; } ///< get rule at index

   bool is_string() const { return get_state() & eGroupString; } ///< check if rule is string
   bool is_comment() const { return get_state() & eGroupComment; } ///< check if rule is comment

//@}

/** \name OPERATION
*///@{

   // ## add rules

   void add(const rule& o) { m_vectorRule.push_back(o); add_marker_hint( o ); } ///< add rule to vector

   void add(const std::string_view& stringState, const std::string_view& stringStart, const std::string_view& stringEnd) {
      m_vectorRule.emplace_back(rule( to_state_s(stringState), stringStart, stringEnd));
      add_marker_hint(stringStart[0]);
   } 
   void add(const std::string_view& stringState, const std::string_view& stringStart, const std::string_view& stringEnd, const std::string_view& stringEscape) {
      m_vectorRule.emplace_back(rule(to_state_s(stringState), stringStart, stringEnd, stringEscape));
      add_marker_hint(stringStart[0]);
   }

   /// Sets marker to hint for some character, this is a but custom when you need more control or just parse without rules
   void set_marker(uint8_t uMark) { m_arrayMarkerHint[uMark] = 1; } ///< set marker hint for character
   void set_marker(char iMark) { set_marker(uint8_t(iMark)); } ///< set marker hint for character

   void clear() { m_vectorRule.clear(); m_arrayMarkerHint = { 0 }; clear_state(); } ///< clear vector of rules
   void clear_state() { m_iActive = -1; } ///< clear state
   bool empty() const { return m_vectorRule.empty(); } ///< check if vector of rules is empty
   size_t size() const { return m_vectorRule.size(); } ///< get size of vector of rules
   bool exists(const char* piText) const; ///< check if text exists in vector of rules
   bool exists(const uint8_t* puText) const { return exists(reinterpret_cast<const char*>( puText )); } ///< check if text exists in vector of rules

   size_t activate(const char* piText); ///< activate state, this will set the state to the state of the rule that matches the text
   size_t activate(const uint8_t* puText) { return activate(reinterpret_cast<const char*>( puText )); } ///< activate state, this will set the state to the state of the rule that matches the text
   bool deactivate(const char* piText, unsigned* puLength = nullptr);
   bool deactivate(const uint8_t* puText, unsigned* puLength = nullptr ) { return deactivate(reinterpret_cast<const char*>( puText ), puLength ); } ///< check if state is deactivated based on text passed, this matches the end of the rule for active state
   /// deactivate state, this doesn't change the internal state but checks if the text is matches deactivate marker text
   bool deactivate(const char* piText, unsigned* puLength, tag_manual );
   bool deactivate(const uint8_t* puText, unsigned* puLength, tag_manual) { return deactivate(reinterpret_cast<const char*>( puText ), puLength, tag_manual()); } ///< check if state is deactivated based on text passed, this matches the end of the rule for active state

   // ## find in data

   /// find first occurrence of a rule in the text, this will return the index of the first rule that matches the text or -1 if not found
   std::pair<int, const char*> find_first( const char* piText, size_t uLength, unsigned* puLength = nullptr) const;
   std::pair<int, const char*> find_first( const std::string_view& stringText, unsigned* puLength = nullptr ) const;


   /// read first value in text
   std::pair<int, std::string_view> read_first( const char* piText, size_t uLength ) const;
   /// read first value in text, this will return the index of the first rule that matches the text or -1 if not found
   std::pair<int, std::string_view> read_first(const std::string_view& stringText) const; 

   // ## iterator methods

   iterator begin() { return m_vectorRule.begin(); } ///< iterator to start of vector
   iterator end() { return m_vectorRule.end(); } ///< iterator to end of vector
   const_iterator begin() const { return m_vectorRule.begin(); } ///< const iterator to start of vector
   const_iterator end() const { return m_vectorRule.end(); } ///< const iterator to end of vector
   const_iterator cbegin() const { return m_vectorRule.cbegin(); } ///< const iterator to start of vector
   const_iterator cend() const { return m_vectorRule.cend(); } ///< const iterator to end of vector
   

//@}

private:
   /// add marker hint, this will add a character to the list of characters that are used to identify state changing markers
   void add_marker_hint(uint8_t uCharacter) { m_arrayMarkerHint[uCharacter] = 1; } 
   void add_marker_hint(char iCharacter) { add_marker_hint(uint8_t(iCharacter)); } 


public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   //enumState m_eState = eStateNone; ///< current state
   int64_t m_iActive = -1; ///< active state, this is the index of the rule that is active
   /// Characters to look for to investigate if they are part of state changing markers
   std::array<uint8_t, 256> m_arrayMarkerHint;
   std::vector<rule> m_vectorRule; ///< vector of rules to use when parsing the expression

   static constexpr unsigned m_uFirstMultiline_s = eStateBlockComment;

// ## free functions ------------------------------------------------------------
public:

   /// convert string to state, this will convert the string to the enumState value
   static constexpr enumState to_state_s(const std::string_view stringName)
   {
       if(stringName == "NONE")          return eStateNone;
       if(stringName == "LINECOMMENT")   return eStateLineComment;
       if(stringName == "WHITESPACE")    return eStateWhitespace;
       if(stringName == "STRING")        return eStateString;
       if(stringName == "NUMBER")        return eStateNumber;
       if(stringName == "IDENTIFIER")    return eStateIdentifier;
       if(stringName == "OPERATOR")      return eStateOperator;
       if(stringName == "END")           return eStateEnd;
       if(stringName == "BLOCKCOMMENT")  return eStateBlockComment;
       if(stringName == "RAWSTRING" )    return eStateRawString;
       if(stringName == "SCRIPTCODE")    return eStateScriptCode;
       return eStateNone; // Default case for invalid input
   }

   /// convert state to string, this will convert the enumState value to a string
   static consteval std::string_view to_string_s(enumState eState)
   {
      switch( eState )
      {
      case eStateNone:          return "NONE";
      case eStateLineComment:   return "LINECOMMENT";
      case eStateWhitespace:    return "WHITESPACE";
      case eStateString:        return "STRING";
      case eStateNumber:        return "NUMBER";
      case eStateIdentifier:    return "IDENTIFIER";
      case eStateOperator:      return "OPERATOR";
      case eStateEnd:           return "END";
      case eStateBlockComment:  return "BLOCKCOMMENT";
      case eStateRawString:     return "RAWSTRING";
      case eStateScriptCode:    return "SCRIPTCODE";
      }
      return "NONE"; // Default case for invalid input
   }

   /// convert string to group, this will convert the string to the enumGroup value
   static constexpr enumGroup to_group_s(const std::string_view stringName)
   {
      if( stringName == "NONE" )    return eGroupNone;
      if( stringName == "COMMENT" ) return eGroupComment;
      if( stringName == "STRING" )  return eGroupString;
      if( stringName == "OUTSIDE" ) return eGroupOutside;
      return eGroupNone; // Default case for invalid input
   }

   /// convert state to string, this will convert the enumState value to a string
   static std::string_view get_string_s(enumState eState)
   {
      switch( eState )
      {
      case eStateNone:          return "NONE";
      case eStateLineComment:   return "LINECOMMENT";
      case eStateWhitespace:    return "WHITESPACE";
      case eStateString:        return "STRING";
      case eStateNumber:        return "NUMBER";
      case eStateIdentifier:    return "IDENTIFIER";
      case eStateOperator:      return "OPERATOR";
      case eStateEnd:           return "END";
      case eStateBlockComment:  return "BLOCKCOMMENT";
      case eStateRawString:     return "RAWSTRING";
      case eStateScriptCode:    return "SCRIPTCODE";
      }
      return "NONE"; // Default case for invalid input
   }


};

/// check if text exists in vector of rules
inline bool state::exists(const char* piText) const {
   if( piText == nullptr ) return false;
   for( auto it : m_vectorRule )
   {
      // Compare with specified lentgth
      if( it.compare(piText) == true ) { return true; }
   }
   return false;
}

/// activate state, this will set the state to the state of the rule that matches the text
inline size_t state::activate(const char* piText) {
   for( auto it = std::begin( m_vectorRule ), itEnd = std::end( m_vectorRule ); it != itEnd; it++ )
   {
      // Compare with specified lentgth
      if( it->compare(piText) == true ) { m_iActive = std::distance( m_vectorRule.begin(), it ); return it->m_stringStart.length(); }
   }                                                                                               assert(false); // should never happen
   return 0;
}

/// check if state is deactivated based on text passed, this matches the end of the rule for active state
inline bool state::deactivate(const char* piText, unsigned* puLength) {                            assert( m_iActive != -1);
   auto it = m_vectorRule.begin() + m_iActive;
   auto b_ = it->compare_end(piText); // check if text matches end of rule
   if( b_ == true && (it->is_escaped_escaped(piText) == true || it->is_escaped(piText) == false) ) { 
      m_iActive = -1;                                                          // deactivate state
      if( puLength != nullptr ) { *puLength = (unsigned)it->m_stringEnd.length(); } // set next position to the end of the string
      return true; 
   }
   return false;
}

/// deactivate state, this doesn't change the internal state but checks if the text is matches deactivate marker text
inline bool state::deactivate(const char* piText, unsigned* puLength, tag_manual) {                assert( m_iActive != -1);
   auto it = m_vectorRule.begin() + m_iActive;
   auto b_ = it->compare_end(piText); // check if text matches end of rule
   if( b_ == true && (it->is_escaped_escaped(piText) == true || it->is_escaped(piText) == false) ) { 
      if( puLength != nullptr ) { *puLength = (unsigned)it->m_stringEnd.length(); } // set next position to the end of the string
      return true; 
   }
   return false;
}


/// find first occurrence of a rule in the text, this will return the index of the first rule that matches the text or -1 if not found
inline std::pair<int, const char*> state::find_first( const std::string_view& stringText, unsigned* puLength ) const {
   if( stringText.empty() == false ) return find_first(stringText.data(), stringText.length(), puLength);
   return std::make_pair(-1, nullptr); // return -1 if string is empty
} 

/// read first value in text
inline std::pair<int, std::string_view> state::read_first(const std::string_view& stringText) const {
   return read_first(stringText.data(), stringText.length());
}




_GD_EXPRESSION_PARSE_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
