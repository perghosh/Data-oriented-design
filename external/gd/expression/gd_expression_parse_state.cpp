/**
 * @file gd_expression_parse_state.cpp
 */

#include "gd_expression_parse_state.h"

_GD_EXPRESSION_PARSE_BEGIN

///< get index of rule that matches text
int state::get_rule_index(const char* piText) const 
{                                                                                                  assert(piText != nullptr); 
   for( size_t u = 0; u < m_vectorRule.size(); u++ ) 
   { 
      if( m_vectorRule[u].compare(piText) ) return static_cast<int>(u); 
   } 
   return -1; 
}



/** ---------------------------------------------------------------------------
 * @brief Finds the first occurrence of a rule in the given text.
 *
 * Scans the input text, skipping whitespace, and checks for a marker hint and rule existence.
 * If a matching rule is found, returns its index and the position in the text.
 * Optionally sets the length of the matched rule's start string.
 * If no rule is found, returns -1 and the current position.
 * 
 * @code
// sample usage
gd::expression::parse::state state_;
state_.add(std::string_view("LINECOMMENT"), "#", "\n");
gd::parse::window::line lineBuffer(256, gd::types::tag_create{});          // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

{
   std::string stringTest = "  1 2 3 4 5 6 7  # Test string";
   auto [ iRule, piPosition ] = state_.find_first(stringTest); // Find first comment in line
   assert( iRule == -1 );
   assert( *piPosition == '1');

   auto [ iRule2, stringValue ] = state_.read_first( stringTest );
   assert(stringValue == "1 2 3 4 5 6 7  ");
}
 * @endcode
 *
 * @param piText   Pointer to the input text.
 * @param uLength  Length of the input text.
 * @param puLength Optional pointer to store the length of the matched rule's start string.
 * @return         A pair containing the rule index (or -1 if not found) and the position in the text.
 */
std::pair<int, const char*> state::find_first(const char* piText, size_t uLength, unsigned* puLength) const
{                                                                                                  assert(piText != nullptr && uLength > 0);
   const char* piPosition = piText;
   const char* piEnd = piText + uLength;
   
   while(piPosition < piEnd) 
   {
      if( *piPosition > 0x20 )                                                // skip whitespace
      {
         if( check_marker_hint((uint8_t)*piPosition) != 0 && exists( piPosition ) == true )
         {
            // find index of rule that matches text
            int iRuleIndex = get_rule_index(piPosition);                                           assert(iRuleIndex >= 0 ); assert(iRuleIndex < static_cast<int>(m_vectorRule.size()) );
            if( puLength != nullptr ) 
            {
               const rule& rule_ = m_vectorRule[iRuleIndex];
               *puLength = (unsigned)rule_.get_start().length();
            }
            return { iRuleIndex, piPosition };
         }

         return { -1, piPosition }; // no rule found, return position
      }

      piPosition++;
   }

   return { -1, nullptr };
}


/** ---------------------------------------------------------------------------
 * @brief Reads the first value or rule from the input text.
 *
 * Scans the input text, skipping whitespace, and attempts to match a rule at the first non-whitespace character.
 * If a rule is found, extracts the value between the rule's start and end markers.
 * If no rule is found, extracts the value up to the next rule or whitespace.
 *
 * @code
 * // Example usage:
 * gd::expression::parse::state state_;
 * state_.add(std::string_view("LINECOMMENT"), "#", "\n");
 * std::string stringTest = "# comment";
 * auto [iRule, stringValue] = state_.read_first(stringTest.c_str(), stringTest.length());
 * // iRule == 0, stringValue == " comment"
 * @endcode
 *
 * @param piText   Pointer to the input text.
 * @param uLength  Length of the input text.
 * @return         A pair containing the rule index (or -1 if not found) and the extracted value as std::string_view.
 */
std::pair<int, std::string_view> state::read_first(const char* piText, size_t uLength) const
{                                                                                                  assert(piText != nullptr && uLength > 0);
   const char* piPosition = piText;
   const char* piEnd = piText + uLength;

   std::string_view stringReturn;
   
   while(piPosition < piEnd) 
   {
      if( *piPosition > 0x20 )                                                // skip whitespace
      {
         const auto* piValueStart = piPosition;                               // remember start position

         if( check_marker_hint((uint8_t)*piPosition) != 0 && exists( piPosition ) == true )
         {
            // find index of rule that matches text
            int iRuleIndex = get_rule_index(piPosition);                                           assert(iRuleIndex >= 0 ); assert(iRuleIndex < static_cast<int>(m_vectorRule.size()) );
            const rule& rule_ = m_vectorRule[iRuleIndex];

            piPosition += rule_.get_start().length();                         // move position to end of rule
            piValueStart = piPosition;                                        // remember start position

            while( piPosition < piEnd )
            {
               if( rule_.compare_end(piPosition) == true && rule_.is_escaped( piPosition ) == false )
               {
                  break; // end of rule found
               }
               piPosition++;
            }

            stringReturn = std::string_view(piValueStart, piPosition - piValueStart);

            return { iRuleIndex, stringReturn };
         }

         while( piPosition < piEnd ) // skip until whitespace
         {
            piPosition++;
            if( check_marker_hint((uint8_t)*piPosition) != 0 && exists( piPosition ) == true )
            {
               break;                                                         // found next rule
            }
         }

         stringReturn = std::string_view(piValueStart, piPosition - piValueStart);

         return { -1, stringReturn }; // no rule found, return position
      }

      piPosition++;
   }

   return { -1, stringReturn };
}



_GD_EXPRESSION_PARSE_END