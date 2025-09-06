/**
 * @file gd_expression.cpp
 * 
 * @brief 
 * 
 */

#include <cstring> 

#include "gd_expression.h"

_GD_EXPRESSION_BEGIN


/*----------------------------------------------------------------------------- to_precedence
 * Returns precedence level for an operator to be used in shunting yard algorithm
 * Higher values indicate higher precedence
 * @param piOperator the operator character to check
 * @return int precedence level (0 for non-operators)
 */
int to_precedence_g(const char* piOperator)
{                                                                                                  assert(piOperator != nullptr); assert(piOperator[0] != '\0');
   if( piOperator[1] != '\0' )
   {
      if( strcmp(piOperator, "&&") == 0 ) return 1;
      if( strcmp(piOperator, "||") == 0 ) return 0;
      return 0;
   }

   switch( piOperator[0] )
   {
   case '^':                                                                  // exponentiation
      return 5;

   case '*':                                                                  // multiplication
   case '/':                                                                  // division
   case '%':                                                                  // modulo
      return 4;

   case '+':                                                                  // addition
   case '-':                                                                  // subtraction
      return 3;

   case '<':                                                                  // less than
   case '>':                                                                  // greater than
   case '=':                                                                  // equality
      return 2;

   case '&':                                                                  // bitwise AND (higher than logical)
   case '|':                                                                  // bitwise OR (higher than logical)
      return 1;

   default:                                                                   // not an operator
      return 0;
   }
}

/// get the precedence of the operator
int to_precedence_g(const char iOperator)
{
   char pi_[2] = {iOperator, '\0'};
   auto iPrecedence = to_precedence_g(pi_);
   return iPrecedence;
}

/** --------------------------------------------------------------------------- to_precedence
 * Returns precedence level for an operator using lookup table for maximum speed
 * Higher values indicate higher precedence
 * @param piOperator the operator character to check
 * @return int precedence level (0 for non-operators)
 */
int to_precedence_g(const char* piOperator, tag_optimize )
{ 
   // Static lookup table initialized once
   static const uint8_t puPrecedenceLookup[256] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0-15
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 16-31
      0, 0, 0, 0, 0, 4, 1, 0, 0, 0, 4, 3, 0, 3, 0, 4,  // 32-47  (%, &, *, +, -, /)
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0,  // 48-63  (<, =, >)
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 64-79
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0,  // 80-95  (^)
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 96-111
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  // 112-127 (|)
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 128-143
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 144-159
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 160-175
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 176-191
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 192-207
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 208-223
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 224-239
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // 240-255
   };

   // Handle multi-character operators first
   if(piOperator[1] != '\0')
   {
      // Use first character as a quick filter to reduce strcmp calls
      switch (piOperator[0])
      {
      case '&': if(piOperator[1] == '&') return 1;  // &&
         break;
      case '|': if(piOperator[1] == '|') return 0;  // ||
         break;
      case '<': if(piOperator[1] == '=') return 2;  // <=
         break;
      case '>': if(piOperator[1] == '=') return 2;  // >=
         break;
      case '=': if(piOperator[1] == '=') return 2;  // ==
         break;
      case '!': if(piOperator[1] == '=') return 2;  // !=
         break;
      }
      return 0;  // Unknown multi-character operator
   }

   return puPrecedenceLookup[static_cast<uint8_t>( piOperator[0] )];            // direct lookup - O(1) time complexity
}


/// get the precedence of the operator
int to_precedence_g(const char iOperator, tag_optimize)
{
   char pi_[2] = {iOperator, '\0'};
   auto iPrecedence = to_precedence_g(pi_, tag_optimize{});
   return iPrecedence;
}



/** --------------------------------------------------------------------------- is_code
 * Check if a character is a valid code character (not whitespace or non-code)
 * @param iCharacter the character to check
 * @return int 1 if valid code character, 0 if whitespace or non-code
 */
int is_code_g( char iCharacter )
{
   // Static lookup table initialized once
   // 1 = valid code character excluding space, 0 = whitespace or non-code character
   static const uint8_t puCodeLookup[256] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0-15 (control chars)
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 16-31 (control chars)
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 32-47 (space, !"#$%&'()*+,-./)
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 48-63 (0-9:;<=>?)
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 64-79 (@A-O)
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 80-95 (P-Z[\]^_)
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 96-111 (`a-o)
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,  // 112-127 (p-z{|}~DEL)
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 128-143
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 144-159
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 160-175
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 176-191
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 192-207
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 208-223
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 224-239
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   // 240-255
   };

   // Space character (ASCII 32) is specifically marked as 0 (not a code character)
   // All printable ASCII characters (33-126) are marked as 1 (valid code characters)

   return puCodeLookup[static_cast<uint8_t>(iCharacter)];  // direct lookup - O(1) time complexity
}



_GD_EXPRESSION_END