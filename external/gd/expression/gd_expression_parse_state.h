


#pragma once

#include <array>
#include <cassert>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>


#include "gd_expression.h"

#ifndef _GD_EXPRESSION_PARSE_BEGIN
#define _GD_EXPRESSION_PARSE_BEGIN namespace gd { namespace expression { namespace parse {
#define _GD_EXPRESSION_PARSE_END } } }
#endif

_GD_EXPRESSION_PARSE_BEGIN


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class state
{
public:
   enum enumState
   {
      eStateNone = 0,		///< no state
      eStateLineComment,	///< comment state
      eStateWhitespace,	   ///< whitespace state
      eStateString,			///< string state
      eStateNumber,			///< number state
      eStateIdentifier,		///< identifier state
      eStateOperator,	   ///< operator state
      eStateEnd,			   ///< end of expression
      eStateBlockComment,	///< block comment state
      eStateRawString,		///< raw string state
      eStateScriptCode     ///< script that differs from the rest
   };

   /**
    * \brief
    *
    *
    */
   struct rule
   {
      // ## construction -------------------------------------------------------------

      rule() : m_eState(eStateNone) {}
      rule(enumState state, const std::string_view& stringStart, const std::string_view& stringEnd)
         : m_eState(state), m_stringStart(stringStart), m_stringEnd(stringEnd) {}
      rule(enumState state, const std::string& stringStart, const std::string& stringEnd)
         : m_eState(state), m_stringStart(stringStart), m_stringEnd(stringEnd) {}
      rule(const rule& o): m_eState(o.m_eState), m_stringStart(o.m_stringStart), m_stringEnd(o.m_stringEnd) {}
      rule(rule&& o) noexcept : m_eState(o.m_eState), m_stringStart(std::move(o.m_stringStart)), m_stringEnd(std::move(o.m_stringEnd)) {}
      rule& operator=(const rule& o) {
         assert(this != &o); m_eState = o.m_eState; m_stringStart = o.m_stringStart; m_stringEnd = o.m_stringEnd; m_stringEscape = o.m_stringEscape;
         return *this;
      }
      rule& operator=(rule&& o) noexcept {
         assert(this != &o); m_eState = o.m_eState;  m_stringStart = std::move(o.m_stringStart); m_stringEnd = std::move(o.m_stringEnd); m_stringEscape = std::move(o.m_stringEscape);
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

      // ## attributes
      enumState m_eState;  ///< state of the rule
      std::string m_stringStart; ///< start of the rule
      std::string m_stringEnd;   ///< end of the rule
      std::string m_stringEscape; ///< escape character
   };

public:
// ## types ------------------------------------------------------------------
   using value_type = rule; ///< type of value stored in vector
   using iterator = std::vector<rule>::iterator; ///< iterator type
   using const_iterator = std::vector<rule>::const_iterator; ///< const iterator type
   using reverse_iterator = std::vector<rule>::reverse_iterator; ///< reverse iterator type
   using const_reverse_iterator = std::vector<rule>::const_reverse_iterator; ///< const reverse iterator type
   using size_type = std::vector<rule>::size_type; ///< size type
   using difference_type = std::vector<rule>::difference_type; ///< difference type



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
   bool is_multiline() const { return (decltype(m_uFirstMultiline_s))get_state() >= m_uFirstMultiline_s; } ///< check if state is multiline
   enumState get_state() const { return m_vectorRule[m_iActive].get_state(); } ///< get current state
   void set_state(int64_t iActive) { m_iActive = iActive; } ///< set current state rule
   const std::vector<rule>& get_rule() const { return m_vectorRule; } ///< get vector of rules
   const std::array<uint8_t, 256>& get_marker_hint() const { return m_arrayMarkerHint; } ///< get marker hint
   const rule& get_rule(size_t uIndex) const { assert(uIndex < m_vectorRule.size()); return m_vectorRule[uIndex]; } ///< get rule at index
//@}

/** \name OPERATION
*///@{
   void add(const rule& o) { m_vectorRule.push_back(o); add_marker_hint( o ); } ///< add rule to vector
   /// add rule to vector, this will add the rule to the vector and add the first character of the start string to the marker hint

   void add(const std::string_view& stringState, const std::string_view& stringStart, const std::string_view& stringEnd) {
      m_vectorRule.emplace_back(rule( to_state_s(stringState), stringStart, stringEnd));
      add_marker_hint(stringStart[0]);
   } 
   void clear() { m_vectorRule.clear(); m_arrayMarkerHint = { 0 }; } ///< clear vector of rules
   bool empty() const { return m_vectorRule.empty(); } ///< check if vector of rules is empty
   size_t size() const { return m_vectorRule.size(); } ///< get size of vector of rules
   bool exists(const char* piText) const; ///< check if text exists in vector of rules
   bool exists(const uint8_t* puText) const { return exists(reinterpret_cast<const char*>( puText )); } ///< check if text exists in vector of rules

   size_t activate(const char* piText); ///< activate state, this will set the state to the state of the rule that matches the text
   size_t activate(const uint8_t* puText) { return activate(reinterpret_cast<const char*>( puText )); } ///< activate state, this will set the state to the state of the rule that matches the text
   bool deactivate(const char* piText);
   bool deactivate(const uint8_t* puText) { return deactivate(reinterpret_cast<const char*>( puText )); } ///< check if state is deactivated based on text passed, this matches the end of the rule for active state

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

};

/// check if text exists in vector of rules
bool state::exists(const char* piText) const {
   if( piText == nullptr ) return false;
   for( auto it : m_vectorRule )
   {
      // Compare with specified lentgth
      if( it.compare(piText) == true ) { return true; }
   }
   return false;
}

/// activate state, this will set the state to the state of the rule that matches the text
size_t state::activate(const char* piText) {
   for( auto it = std::begin( m_vectorRule ), itEnd = std::end( m_vectorRule ); it != itEnd; it++ )
   {
      // Compare with specified lentgth
      if( it->compare(piText) == true ) { m_iActive = std::distance( m_vectorRule.begin(), it ); return it->m_stringStart.length(); }
   }                                                                                               assert(false); // should never happen
   return 0;
}

/// check if state is deactivated based on text passed, this matches the end of the rule for active state
bool state::deactivate(const char* piText) {                                                       assert( m_iActive != -1);
   auto it = m_vectorRule.begin() + m_iActive;
   if( it->compare_end(piText) == true && it->is_escaped( piText) == false ) { 
      m_iActive = -1; 
      return true; 
   }
   return false;
}



_GD_EXPRESSION_PARSE_END