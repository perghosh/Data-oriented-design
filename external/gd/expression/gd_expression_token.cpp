#include <stack>

#include "gd_expression_operator.h"
#include "gd_expression_runtime.h"
#include "gd_expression_token.h"
#include "gd_expression_method_01.h"

_GD_EXPRESSION_BEGIN

constexpr uint8_t WHITESPACE_BIT = 0x01;            ///<  \t, \n, \r, \f, \v, \0 >
constexpr uint8_t DIGIT_BIT = 0x02;                 ///< 0-9
constexpr uint8_t ALPHABETIC_BIT = 0x04;            ///< A-Z, a-z
constexpr uint8_t OPERATOR_BIT = 0x08;              ///< +, -, *, /, %, &, |, ^, ~, !, =, <, >, ?, :, ;, (, ), [, ], {, }, ., ,, @
constexpr uint8_t SEPARATOR_BIT = 0x10;             ///  , ;, :, ., ?, !, @
constexpr uint8_t STRING_DELIMITER_BIT = 0x20;      ///< ", ', `
constexpr uint8_t SPECIAL_CHAR_BIT = 0x40;
constexpr uint8_t DEFAULT_BIT = 0x00;

constexpr uint8_t puCharacterGroup_g[0x100] =
{
   //         0,   1,   2,   3,    4,   5,   6,   7,    8,   9,   A,   B,    C,   D,   E,   F
   /* 0 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x01,0x01,0x01,0x01, 0x01,0x01,0x00,0x00,  /* 0   - 15  */
   /* 1 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 16  - 31  */
   /* 2 */ 0x01,0x48,0x20,0x40, 0x40,0x48,0x40,0x20, 0x40,0x40,0x48,0x48, 0x10,0x48,0x12,0x48,  /* 32  - 47   ' ',!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x44,0x10, 0x48,0x48,0x48,0x40,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

   /* 4 */ 0x40,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,  /* 64  - 79  @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O */
   /* 5 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x40, 0x40,0x40,0x48,0x40,  /* 80  - 95  P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_ */
   /* 6 */ 0x40,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,  /* 96  - 111 `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o */
   /* 7 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x40, 0x40,0x40,0x40,0x00,  /* 112 - 127 p,q,r,s,t,u,v,w,x,y,z,{,|,},~,DEL */

   /* 8 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 128 - 143 */
   /* 9 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 144 - 159 */
   /* A */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 160 - 175 */
   /* B */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 176 - 191 */

   /* C */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 192 - 207 */
   /* D */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 208 - 223 */
   /* E */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 224 - 239 */
   /* F */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00   /* 240 - 255 */
};


/**
 * @brief Returns the type of the token as value
 * 
 * value is a variant type that can hold different types of values, based on the token type value gets the proper type
 * 
 * @return value for token
 */
value token::as_value() const 
{
   if( get_token_type() == eTokenTypeValue) 
   {
      switch (get_value_type()) 
      {
      case eValueTypeBoolean:
         return value(m_stringName == "true");
      case eValueTypeInteger:
         return value((int64_t)std::stoll(std::string(m_stringName)));
      case eValueTypeDecimal:
         return value(std::stod(std::string(m_stringName)));
      case eValueTypeString:
         return value(std::string(m_stringName));
      default:
         assert(false); throw std::invalid_argument("Unsupported value type");
      }
   }
   return value();
}


/** --------------------------------------------------------------------------
 * @brief Skips whitespace characters in the input string.
 *
 * This function skips whitespace characters in the input string and returns
 * a pointer to the first non-whitespace character.
 *
 * @code
 * // sample usage
 * const char* input = "   Hello, World!";
 * const char* end = input + strlen(input);
 * const char* result = token::skip_whitespace_s(input, end);
 * std::cout << "First non-whitespace character: " << *result << std::endl;
 * @endcode
 *
 * @param piszBegin Pointer to the beginning of the input string.
 * @param piszEnd Pointer to the end of the input string.
 * @return Pointer to the first non-whitespace character.
 */
inline const char* token::skip_whitespace_s(const char* piszBegin, const char* piszEnd)
{
   const char* pisz_ = piszBegin;
   while(pisz_ < piszEnd && (puCharacterGroup_g[static_cast<uint8_t>(*pisz_)] & WHITESPACE_BIT))
   {
      ++pisz_;
   }
   return pisz_;
}

/** --------------------------------------------------------------------------
 * @brief Reads a number from the input string.
 *
 * This function reads a number from the input string and returns its type.
 *
 * The function reads until it finds a non-digit character. It also handles
 * the sign of the number.
 *
 * @code
 * // sample usage
 * const char* input = "12345";
 * const char* end = input + strlen(input);
 * std::string_view result;
 * uint32_t type = token::read_number_s(input, end, result);
 * std::cout << "Read number: " << result << std::endl;
 * @endcode
 *
 * @param piszBegin Pointer to the beginning of the input string.
 * @param piszEnd Pointer to the end of the input string.
 * @param string_ Reference to a string_view to store the read number.
 * @return The type of the token.
 */
uint32_t token::read_number_s(const char* piszBegin, const char* piszEnd, std::string_view& string_)
{
   uint32_t uType = 0;
   const char* pisz_ = piszBegin;
   if( pisz_ < piszEnd && (*pisz_ == '-' || *pisz_ == '+') ) { ++pisz_; }      // Skip the sign

   while( pisz_ < piszEnd && (puCharacterGroup_g[static_cast<uint8_t>(*pisz_)] & DIGIT_BIT) ) 
   { 
      uType |= puCharacterGroup_g[static_cast<uint8_t>(*pisz_)];               // Set the type
      ++pisz_; 
   }

   string_ = std::string_view(piszBegin, pisz_ - piszBegin);
   return uType;
}


/** --------------------------------------------------------------------------
 * @brief Reads a string from the input string.
 *
 * This function reads a string from the input string and returns its type.
 * 
 * First character is used to determine the delimiter, and the function reads until it
 * finds the matching delimiter. If there are multiple delimiters, it will read until
 * finds the same number of delimiters. Like `'''` or `"""` will be treated as a single
 * delimiter. And reads until the next matching delimiter.
 * 
 * @code
 * // sample usage 
 * const char* input = "\"Hello, World!\"";
 * const char* end = input + strlen(input);
 * std::string_view result;
 * const char* readTo;
 * uint32_t type = token::read_string_s(input, end, result, &readTo);
 * std::cout << "Read string: " << result << std::endl;
 * std::cout << "Next position: " << (readTo - input) << std::endl;
 * @endcode
 *
 * @param piszBegin Pointer to the beginning of the input string.
 * @param piszEnd Pointer to the end of the input string.
 * @param string_ Reference to a string_view to store the read string.
 * @param ppiszReadTo Pointer to a pointer to store the position after the read string.
 * @return The type of the token.
 */
uint32_t token::read_string_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char** ppiszReadTo) 
{
   char iDelimiter = *piszBegin;

   unsigned uDelimiterLength = 1;
   while( piszBegin[uDelimiterLength] == iDelimiter ) uDelimiterLength++;

   const char* pisz_ = uDelimiterLength != 2 ? piszBegin + uDelimiterLength : nullptr;

   if( pisz_ != nullptr )
   {
      const char* piszText = pisz_;

      while( pisz_ < piszEnd )
      {
         if( *pisz_ != iDelimiter ) { ++pisz_; continue; }
         else
         {
            unsigned uLength = 1;
            while( (pisz_ + uLength) < piszEnd && pisz_[uLength] == iDelimiter ) { uLength++; }
            if( uDelimiterLength == uLength )
            {
               string_ = std::string_view(piszText, pisz_ - piszText);
               break;
            }
         }
      }
   }

   if( ppiszReadTo != nullptr ) { *ppiszReadTo = pisz_ + uDelimiterLength; }

   return STRING_DELIMITER_BIT;
}


 /** --------------------------------------------------------------------------
  * @brief Reads a variable, function or label name from the input string.
  *
  * This function reads a variable or function name from the input string and
  * returns its type and the corresponding token type.
  *
  * @param piszBegin Pointer to the beginning of the input string.
  * @param piszEnd Pointer to the end of the input string.
  * @param string_ Reference to a string_view to store the read name.
  * @param ppiszReadTo Pointer to a pointer to store the position after the read name.
  * @return A pair containing the type of the token and its token type.
  */
std::pair<uint32_t, uint32_t> token::read_variable_and_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char** ppiszReadTo)
{
   enumTokenType eTokenPartType = eTokenTypeVariable; // default is variable
   uint32_t uType = 0;
   const char* pisz_ = piszBegin;

   // ## read all characters, characters can be a-z, A-Z, 0-9, _, @, :
   while( pisz_ < piszEnd && ( puCharacterGroup_g[static_cast<uint8_t>(*pisz_)] & ALPHABETIC_BIT ) )
   {
      uType |= puCharacterGroup_g[static_cast<uint8_t>(*pisz_)] & (ALPHABETIC_BIT|SPECIAL_CHAR_BIT);               // Set the type
      ++pisz_;
   }

   if( pisz_ < piszEnd && *pisz_ == '(' ) { string_ = std::string_view(piszBegin, pisz_ - piszBegin); ++pisz_; eTokenPartType = eTokenTypeFunction; } // Function
   else if( pisz_ < piszEnd && *pisz_ == ':' ) { string_ = std::string_view(piszBegin, pisz_ - piszBegin); ++pisz_; eTokenPartType = eTokenTypeLabel; } // Label is like a name that can be jumped to
   else if( pisz_ < piszEnd && *pisz_ == '.' ) { string_ = std::string_view(piszBegin, pisz_ - piszBegin); ++pisz_; eTokenPartType = eTokenTypeMember; } // Member is like a name that can be accessed
   else
   {
      string_ = std::string_view(piszBegin, pisz_ - piszBegin);
   }

   if( ppiszReadTo != nullptr ) { *ppiszReadTo = pisz_; }

   return { uType, eTokenPartType };
}

/** ---------------------------------------------------------------------------
 * @brief Parses a string into tokens based on the provided formula.
 *
 * This function processes the input string and generates a vector of tokens.
 * It handles different types of tokens such as numbers, identifiers, operators,
 * string literals, and special characters.
 *
 * @param piszBegin Pointer to the beginning of the input string.
 * @param piszEnd Pointer to the end of the input string.
 * @param vectorToken Vector to store the generated tokens.
 * @param tag_formula Tag to indicate the parsing mode (formula in this case).
 * @return A pair containing a boolean indicating success and a string with an error message if any.
 */
std::pair<bool, std::string> token::parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_formula )
{
   const char* piszPosition = piszBegin;                                       // Current position

   /// ## Loop through the string and parse the tokens
   while( piszPosition < piszEnd )
   {
      piszPosition = skip_whitespace_s(piszPosition, piszEnd);                 // Skip whitespace
      if( piszPosition >= piszEnd ) { break; }  

      uint8_t uCharacterType = puCharacterGroup_g[static_cast<uint8_t>(*piszPosition)];  // Get the type of the character

      if( uCharacterType & DIGIT_BIT )                                         // Number
      {
         uint32_t uTokenType = token::token_type_s( "VALUE" );
         std::string_view string_;
         uint32_t uType = read_number_s(piszPosition, piszEnd, string_);
         if( uType & SEPARATOR_BIT ) { uTokenType += to_type_s( eValueTypeDecimal, eTokenPartType ); }
         else { uTokenType += to_type_s(eValueTypeInteger, eTokenPartType); }

         vectorToken.emplace_back( token( uTokenType, string_ ) );
         piszPosition += string_.length();
         continue;
      }

      // ### Check for variable, method or keyword
      //     Characters can be different things where variable is just a "word"
      //     and method is a "method" with a "(" at the end. Keyword is a reserved word
      //     and needs to be checked against the keyword list.
      if( uCharacterType & ALPHABETIC_BIT )                                    // Identifier
      {
         std::string_view string_;
         const char* piszEnd_ = nullptr;
         auto [uType, uTokenType] = read_variable_and_s(piszPosition, piszEnd, string_, &piszEnd_);
         if( uType != 0 )
         {
            if( uTokenType == token::token_type_s("VARIABLE") )
            {
               vectorToken.emplace_back(token(uTokenType, string_));
            }
            else if( uTokenType == token::token_type_s("FUNCTION") )
            {
               if( uType & SPECIAL_CHAR_BIT )                                  // If special char this has to have a ':' character
               {
                  uTokenType |= uint32_t(eFunctionNamespace);                  // add namespace flag to find method amoung namespaced methods
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
               else
               {
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
               // TODO: manage how to handle function calls
            }
            
            piszPosition = piszEnd_;
         }
         continue;
      }

      if( uCharacterType & OPERATOR_BIT )                                      // Operator
      {
         uint32_t uTokenType = token::token_type_s( "OPERATOR" );
         if( uCharacterType & SPECIAL_CHAR_BIT )
         {
            if( piszPosition[1] == '=' )                                      // Handle special operators that have two characters like >=, <=, == etc
            {
               vectorToken.emplace_back(token(uTokenType, std::string_view(piszPosition, 2)));
               piszPosition += 2;
               continue;
            }
            else if( piszPosition[0] == '-' )
            {
               auto type_ = vectorToken.empty() == false ? vectorToken.back().get_token_type() : token::token_type_s("OPERATOR");
               if( type_ == token::token_type_s("OPERATOR") )                  // Was previous token an operator
               {
                  // ## this has to be a unary operator for negative number try to read number
                  uTokenType = token::token_type_s( "VALUE" );                 // value token
                  std::string_view string_; // string that gets value
                  uint32_t uType = read_number_s(piszPosition, piszEnd, string_);// read number
                  if( uType & SEPARATOR_BIT ) { uTokenType += to_type_s( eValueTypeDecimal, eTokenPartType ); } // is it decimal?
                  else { uTokenType += to_type_s(eValueTypeInteger, eTokenPartType); } // integer

                  vectorToken.emplace_back( token( uTokenType, string_ ) );
                  piszPosition += string_.length();
                  continue;
               }
            }
         }

         vectorToken.emplace_back(token(uTokenType, std::string_view(piszPosition, 1)));
         piszPosition++;
         continue;
      }

      if( uCharacterType & STRING_DELIMITER_BIT )                              // String delimiter
      {
         std::string_view string_;
         const char* piszEnd_ = nullptr;
         uint32_t uType = read_string_s(piszPosition, piszEnd, string_, &piszEnd_);
         if( uType != 0 )
         {
            uint32_t uTokenType = token::token_type_s("VALUE") + to_type_s(eValueTypeString, eTokenPartType);
            vectorToken.emplace_back( token( uTokenType, string_ ) );
            piszPosition = piszEnd_;
         }
         continue;
      }

      if( uCharacterType & SPECIAL_CHAR_BIT )                                           // Special character
      {
         uint32_t uTokenType = token::token_type_s( "SPECIAL_CHAR" );
         vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         piszPosition++;
         continue;
      }


      if( uCharacterType & SEPARATOR_BIT )                                           // Special character
      {
         uint32_t uTokenType = token::token_type_s( "SEPARATOR" );
         vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         piszPosition++;
         continue;
      }

      piszPosition++;
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Compiles the input tokens into postfix notation.
 *
 * This function takes a vector of tokens and converts them into postfix
 * notation using the Shunting Yard algorithm. It handles operators, values,
 * variables, and special characters.
 *
 * @param vectorIn Input vector of tokens to be compiled.
 * @param vectorOut Output vector to store the compiled tokens in postfix notation.
 * @param tag_postfix Tag to indicate the compilation mode (postfix in this case).
 * @return A pair containing a boolean indicating success and a string with an error message if any.
 */
std::pair<bool, std::string> token::compile_s(const std::vector<token>& vectorIn, std::vector<token>& vectorOut, tag_postfix)
{
   std::stack<token> stackOperator;

   for( const auto& token_ : vectorIn )
   {
#ifndef NDEBUG
      auto stringToken_d = token_.get_name();
#endif

      uint32_t uTokenType = token_.get_token_type();
      switch( uTokenType )
      {
      case token_type_s("OPERATOR"):
         {
            while( stackOperator.empty() == false )
            {
               auto stringToken = token_.get_name();
               auto stringOperator = stackOperator.top().get_name();
               char iOperator = stringOperator[0];
               if( stringOperator == "(" ) { break; }

               int iTokenPrecedence = to_precedence_g(stringToken[0], tag_optimize{});
               int iPrecedence = to_precedence_g(iOperator, tag_optimize{} );
               if( iTokenPrecedence > iPrecedence ) { break; }

               vectorOut.push_back(stackOperator.top());
               stackOperator.pop();
            }

            stackOperator.push(token_);
         }
      break;
      case token_type_s("VALUE"):
         vectorOut.push_back(token_);
      break;
      case token_type_s("VARIABLE"):
         vectorOut.push_back(token_);
      break;
      case token_type_s("FUNCTION"):
         stackOperator.push(token_);
      break;
      case token::token_type_s("SEPARATOR"):
         {
            auto stringToken = token_.get_name();
            char iCharacter = stringToken[0];
            if( iCharacter == ',' )
            {
               if( stackOperator.empty() == false )
               {
                  auto& tokenTop = stackOperator.top();
                  // test for value or variable and if any the put that value in vector with sorted values for postfix
                  if( tokenTop.get_token_type() == token_type_s("VALUE") || tokenTop.get_token_type() == token_type_s("VARIABLE") ) 
                  { 
                     vectorOut.push_back(std::move(tokenTop));
                     stackOperator.pop();
                  }
               }
            }
            else if( iCharacter == ';' )                                       // found ; and that means end of statement, clear result
            {
               // ## pop all operators from stack and put them in vector
               while( stackOperator.empty() == false )
               {
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
               vectorOut.push_back(std::move(token_));                         // ; add to out
            }
            else { return { false, "[compile_s] - Unsupported separator: " + std::string(stringToken) }; }
         }
      break;

      case token_type_s("SPECIAL_CHAR"):
         {
            vectorOut.push_back(token_);
            auto stringToken = token_.get_name();
            char iCharacter = stringToken[0];
            if( iCharacter == '(' )
            {
               stackOperator.push(token_);
            }
            else if( iCharacter == ')' )
            {
               while( stackOperator.empty() == false )
               {
                  auto stringOperator = stackOperator.top().get_name();
                  if( stringOperator == "(" ) { break; }
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
               if( stackOperator.empty() == false ) { stackOperator.pop(); }
            }
         }
      break;

      default:
         assert( false );
         break;

      }
   }

   while( stackOperator.empty() == false )
   {
      vectorOut.push_back( stackOperator.top() );
      stackOperator.pop();
   }


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Evaluates binary operations based on the operator string
 *
 * This function dispatches to the appropriate operation function based on the
 * provided operator string. It uses a switch statement on the first character
 * for efficiency, with nested checks for multi-character operators.
 *
 * @param stringOperator The operator to evaluate ("+", "-", "*", etc.)
 * @param valueLeft Left operand
 * @param valueRight Right operand
 * @param pruntime Pointer to runtime environment for error reporting
 * @return value Result of the operation
 * @throws std::invalid_argument If the operator is not supported
 */
value evaluate_operator_g(const std::string_view& stringOperator, value& valueLeft, value& valueRight, runtime* pruntime)
{                                                                                                  assert( stringOperator.empty() == false );
   // Switch on the first character for efficiency
   switch (stringOperator[0]) {
   case '+': 
      return add(valueLeft, valueRight, pruntime);

   case '-': 
      return subtract(valueLeft, valueRight, pruntime);

   case '*': 
      return multiply(valueLeft, valueRight, pruntime);

   case '/': 
      return divide(valueLeft, valueRight, pruntime);

   case '%': 
      return modulo(valueLeft, valueRight, pruntime);

   case '=': 
      // Check for "=="
      if(stringOperator.size() > 1 && stringOperator[1] == '=') { return equal(valueLeft, valueRight, pruntime); }
      break;

   case '!': 
      // Check for "!="
      if(stringOperator.size() > 1 && stringOperator[1] == '=') { return not_equal(valueLeft, valueRight, pruntime); }
      break;

   case '<': 
      // Check for "<=" or just "<"
      if(stringOperator.size() > 1 && stringOperator[1] == '=') { return less_equal(valueLeft, valueRight, pruntime); } 
      else { return less(valueLeft, valueRight, pruntime); }

   case '>': 
      // Check for ">=" or just ">"
      if(stringOperator.size() > 1 && stringOperator[1] == '=') { return greater_equal(valueLeft, valueRight, pruntime); } 
      else { return greater(valueLeft, valueRight, pruntime); }

   case '&':
      // Check for "&&" or just "&"
      if( stringOperator.size() > 1 && stringOperator[1] == '&' ) { return logical_and(valueLeft, valueRight, pruntime); }
      else { return bitwise_and(valueLeft, valueRight, pruntime); }

   case '|':
      // Check for "||" or just "|"
      if( stringOperator.size() > 1 && stringOperator[1] == '|' ) { return logical_or(valueLeft, valueRight, pruntime); }
      else { return bitwise_or(valueLeft, valueRight, pruntime); }
   }

   // If we get here, the operator wasn't recognized
   if(pruntime != nullptr) { pruntime->add("[evaluate_operator_g] - Unsupported operator: " + std::string(stringOperator), tag_error{}); }

   throw std::invalid_argument("Unsupported operator: " + std::string(stringOperator));
}


/** -----------------------------------------------------------------------------
 * @brief Calculates the result of an expression from postfix tokens using runtime context
 * 
 * Evaluates a postfix expression by processing tokens sequentially:
 * - For operators: pops two values from stack, applies operator, pushes result
 * - For values: pushes token value to stack
 * - For variables: looks up value in runtime context and pushes to stack
 * 
 * @param vectorToken Vector of tokens in postfix notation
 * @param pvalueResult Pointer to store the resulting value
 * @param runtime_ Runtime context for variable resolution
 * @return Pair of success flag and error message (if any)
 */
std::pair<bool, std::string> token::calculate_s(const std::vector<token>& vectorToken, value* pvalueResult, runtime& runtime_ )
{
   std::vector<value> vectorArguments;
   std::stack<value> stackValue;
   std::string stringAssignVariable; // special case when we need to assign variable

   for( const auto& token_ : vectorToken )
   {
      switch( token_.get_token_type() )
      {
      case token::token_type_s("OPERATOR"):
         {
            auto stringOerator = token_.get_name();                            // get operator character

            if( stringOerator == "=" )                                         // special case for assignment
            {                                                                                      assert( stackValue.empty() == false );
               value value_ = stackValue.top();                                // get stack value, make sure that there are at last one value
               stackValue.pop();                                               // pop it
               if( stringAssignVariable.empty() == false )
               {
                  runtime_.set_variable(stringAssignVariable, value_);         // set variable
                  stringAssignVariable.clear();                                // clear variable name, important to make the logic work, it may be multiple variables that are assigned
               }
               else
               {                                                                                   assert(false);
                  return { false, "[calculate_s] - No variable name for assignment" };
               }
               continue;                                                       // continue to next token
            }

            value valueRight = stackValue.top();                               // get right value
            stackValue.pop();                                                  // pop it
            value valueLeft = stackValue.top();                                // get left value
            stackValue.pop();                                                  // pop it

            // call operator function
            value result_ = evaluate_operator_g(stringOerator, valueLeft, valueRight, &runtime_ );
            stackValue.push(result_);                                          // push result to stack
         }
         break;
      case token::token_type_s("VALUE"):
         stackValue.push(token_.as_value());
         break;
      case token::token_type_s("VARIABLE"):
         {
            auto stringVariable = token_.get_name();
            int iIndex = runtime_.find_variable(stringVariable);
            if( iIndex >= 0 )
            {
               stackValue.push(value( runtime_.get_variable(iIndex) ));
            }
            else
            {
               // ## try to find variable using callback
               auto stringVariable = token_.get_name();
               value::variant_t variantValue;
               bool bFound = runtime_.find_value(stringVariable, &variantValue);
               if( bFound == true )
               {
                  stackValue.push(value( variantValue ));
               }
               else if( stringAssignVariable.empty() == true )
               {
                  stringAssignVariable.assign( stringVariable );              // prepare this for assign operator, only one assign value can be active at any given time
               }
               else
               {
                  return { false, "[calculate_s] - Variable not found: " + std::string(stringVariable) };
               }
            }
         }
         break;
      case token::token_type_s("FUNCTION"):
         {
            vectorArguments.clear();
            auto stringMethod = token_.get_name();
            const method* pmethod_ = nullptr;
            if( token_.get_function_type() == 0 ) 
            { 
               pmethod_ = runtime_.find_method(stringMethod); 
            }
            else if( token_.get_function_type() & eFunctionNamespace )
            {
               pmethod_ = runtime_.find_method(stringMethod, tag_namespace{} );
            }

            if( pmethod_ != nullptr )
            {
               unsigned uCount = pmethod_->in_count();
               for( unsigned i = 0; i < uCount; i++ )
               {
                  if( stackValue.empty() == false )
                  {
                     vectorArguments.push_back( std::move( stackValue.top() ) );
                     stackValue.pop();
                  }
               }

               if( pmethod_->flags() == 0 )                                    // default methods, only use arguments and return value
               {
                  if( pmethod_->out_count() == 1 )
                  {
                     value valueResult;
                     auto result_ = reinterpret_cast<method::method_1>(pmethod_->m_pmethod)( vectorArguments, &valueResult );
                     if( result_.first == true ) { stackValue.push(valueResult); }
                     else { return { false, "[calculate_s] - Method call failed: " + std::string(stringMethod) + " - " + result_.second }; }
                  }
               }
               else
               {
                  if( pmethod_->is_runtime() == true )                         // method needs runtime, maybe to find global data
                  {
                     if( pmethod_->out_count() == 0 )
                     {
                        auto result_ = reinterpret_cast<method::method_runtime_0>(pmethod_->m_pmethod)( &runtime_, vectorArguments );
                        if( result_.first == false ) return { false, "[calculate_s] - Method call failed: " + std::string(stringMethod) + " - " + result_.second };
                     }
                     else if( pmethod_->out_count() == 1 )
                     {
                        value valueResult;
                        auto result_ = reinterpret_cast<method::method_runtime_1>(pmethod_->m_pmethod)( &runtime_, vectorArguments, &valueResult );
                        if( result_.first == true ) { stackValue.push(valueResult); }
                        else { return { false, "[calculate_s] - Method call failed: " + std::string(stringMethod) + " - " + result_.second }; }
                     }
                  }
               }
            }
            else
            {                                                                                      assert(false);
               return { false, "[calculate_s] - Method not found: " + std::string(stringMethod) };
            }
         }
         break;
      case token::token_type_s("SEPARATOR"):
         {
            auto stringSeparator = token_.get_name();
            char iCharacter = stringSeparator[0];
            if( iCharacter == ',' )
            {
               // ## do nothing
            }
            else if( iCharacter == ';' )
            {
               std::stack<value>().swap( stackValue );                         // clear stack value
            }

         }
         break;
      }
      
   }

   if( stackValue.empty() == false )
   {
      *pvalueResult = stackValue.top();
      stackValue.pop();
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Simplified one-liner expression evaluator
 * 
 * Convenience method that combines parsing, compilation and calculation in one step.
 * Only performs compile-time checks and throws exceptions on errors.
 * 
 * @param stringExpression The expression to evaluate
 * @param vectorVariable Vector of variable name/value pairs for evaluation
 * @return The calculated result
 * @throws std::invalid_argument if parsing, compilation or calculation fails
 */
value token::calculate_s( const std::string_view& stringExpression, const std::vector< std::pair<std::string, value::variant_t>>& vectorVariable )
{
   // ## convert string to tokens
   std::vector<token> vectorToken;
   std::pair<bool, std::string> result = parse_s(stringExpression, vectorToken, tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that menas to convert tokens to postfix, place them in correct order to be processed
   std::vector<token> vectorPostfix;
   result = compile_s(vectorToken, vectorPostfix, tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## calculate the result
   runtime runtime_(vectorVariable);
   runtime_.add( { 4, pmethodDefault_g, ""});
   runtime_.add( { 3, pmethodString_g, std::string("str")});
   value valueResult;
   result = calculate_s(vectorPostfix, &valueResult, runtime_);
   if( result.first == false ) { throw std::invalid_argument(result.second); }
   return valueResult;
}

value token::calculate_s( const std::string_view& stringExpression, const std::vector< std::pair<std::string, value::variant_t>>& vectorVariable, std::function< void( runtime& runtime )> callback_ )
{
   // ## convert string to tokens
   std::vector<token> vectorToken;
   std::pair<bool, std::string> result = parse_s(stringExpression, vectorToken, tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that menas to convert tokens to postfix, place them in correct order to be processed
   std::vector<token> vectorPostfix;
   result = compile_s(vectorToken, vectorPostfix, tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## calculate the result
   runtime runtime_(vectorVariable);
   runtime_.add( { 4, pmethodDefault_g, ""});
   runtime_.add( { 3, pmethodString_g, std::string("str")});

   callback_(runtime_);                                                        // add more runtime context to the callback

   value valueResult;
   result = calculate_s(vectorPostfix, &valueResult, runtime_);
   if( result.first == false ) { throw std::invalid_argument(result.second); }
   return valueResult;
}

/** ---------------------------------------------------------------------------
 * @brief Simplified one-liner expression evaluator using runtime context
 * 
 * Convenience method that combines parsing, compilation and calculation in one step.
 * Uses a runtime context for variable resolution.
 * 
 * @param stringExpression The expression to evaluate
 * @param runtime_ Runtime context for variable resolution
 * @return The calculated result
 * @throws std::invalid_argument if parsing, compilation or calculation fails
 */
value token::calculate_s( const std::string_view& stringExpression, runtime& runtime_ )
{
   // ## convert string to tokens
   std::vector<token> vectorToken;
   std::pair<bool, std::string> result = token::parse_s(stringExpression, vectorToken, tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that menas to convert tokens to postfix, place them in correct order to be processed
   std::vector<token> vectorPostfix;
   result = token::compile_s(vectorToken, vectorPostfix, tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## calculate the result
   value valueResult;
   result = token::calculate_s(vectorPostfix, &valueResult, runtime_);
   if( result.first == false ) { throw std::invalid_argument(result.second); }
   return valueResult;
}

/// @brief Simplified wrapper for one-liner expression evaluator
value token::calculate_s( const std::string_view& stringExpression )
{
   return calculate_s(stringExpression, std::vector<std::pair<std::string, value::variant_t>>());
}

/// @brief Simplified wrapper for one-liner expression evaluator and possibility to get the runtime context
value token::calculate_s(const std::string_view& stringExpression, std::unique_ptr<runtime>& pruntime )
{
   if( pruntime == nullptr ) 
   { 
      pruntime = std::make_unique<runtime>();
      pruntime->add({ 4, pmethodDefault_g, ""});
      pruntime->add({ 3, pmethodString_g, "str"});
   }

   return calculate_s(stringExpression, *pruntime.get() );
}


_GD_EXPRESSION_END