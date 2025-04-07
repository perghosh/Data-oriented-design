#include <stack>

#include "gd_expression_operator.h"
#include "gd_expression_runtime.h"
#include "gd_expression_token.h"

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
   /* 2 */ 0x01,0x40,0x20,0x40, 0x40,0x48,0x40,0x20, 0x40,0x40,0x48,0x48, 0x10,0x48,0x12,0x48,  /* 32  - 47   ' ',!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x40,0x10, 0x48,0x48,0x48,0x40,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

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
         return value(std::stoll(std::string(m_stringName)));
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


inline const char* token::skip_whitespace_s(const char* piszBegin, const char* piszEnd)
{
   const char* pisz_ = piszBegin;
   while(pisz_ < piszEnd && (puCharacterGroup_g[static_cast<uint8_t>(*pisz_)] & WHITESPACE_BIT))
   {
      ++pisz_;
   }
   return pisz_;
}

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

std::pair<uint32_t, enumTokenType> token::read_variable_and_s(const char* piszBegin, const char* piszEnd, std::string_view& string_, const char** ppiszReadTo)
{
   enumTokenType eTokenPartType = eTokenTypeVariable;
   uint32_t uType = 0;
   const char* pisz_ = piszBegin;

   // ## read all characters, characters can be a-z, A-Z, 0-9, _, @
   while( pisz_ < piszEnd && ( puCharacterGroup_g[static_cast<uint8_t>(*pisz_)] & ALPHABETIC_BIT ) )
   {
      ++pisz_;
   }

   if( pisz_ < piszEnd && *pisz_ == '(' ) { ++pisz_; eTokenPartType = eTokenTypeFunction; } // Function
   else if( pisz_ < piszEnd && *pisz_ == ':' ) { ++pisz_; eTokenPartType = eTokenTypeLabel; } // Label is like a name that can be jumped to
   else if( pisz_ < piszEnd && *pisz_ == '.' ) { ++pisz_; eTokenPartType = eTokenTypeMember; } // Member is like a name that can be accessed

   string_ = std::string_view(piszBegin, pisz_ - piszBegin);
   if( ppiszReadTo != nullptr ) { *ppiszReadTo = pisz_; }

   return { ALPHABETIC_BIT, eTokenPartType };
}

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
               vectorToken.emplace_back(token(uTokenType, string_));
               // TODO: manage how to handle function calls
            }
            
            piszPosition = piszEnd_;
         }
         continue;
      }

      if( uCharacterType & OPERATOR_BIT )                                      // Operator
      {
         uint32_t uTokenType = token::token_type_s( "OPERATOR" );
         vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         piszPosition++;
         continue;
      }

      if( uCharacterType & SEPARATOR_BIT )                                     // Separator
      {
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

/**
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
      if (stringOperator.size() > 1 && stringOperator[1] == '=') { return equal(valueLeft, valueRight, pruntime); }
      break;

   case '!': 
      // Check for "!="
      if (stringOperator.size() > 1 && stringOperator[1] == '=') { return not_equal(valueLeft, valueRight, pruntime); }
      break;

   case '<': 
      // Check for "<=" or just "<"
      if (stringOperator.size() > 1 && stringOperator[1] == '=') { return less_equal(valueLeft, valueRight, pruntime); } 
      else { return less(valueLeft, valueRight, pruntime); }

   case '>': 
      // Check for ">=" or just ">"
      if (stringOperator.size() > 1 && stringOperator[1] == '=') { return greater_equal(valueLeft, valueRight, pruntime); } 
      else { return greater(valueLeft, valueRight, pruntime); }
   }

   // If we get here, the operator wasn't recognized
   if (pruntime != nullptr) { pruntime->add("[evaluate_operator_g] - Unsupported operator: " + std::string(stringOperator), tag_error{}); }

   throw std::invalid_argument("Unsupported operator: " + std::string(stringOperator));
}


std::pair<bool, std::string> token::calculate_s(const std::vector<token>& vectorToken, value* pvalueResult, runtime& runtime_ )
{
   std::stack<value> stackValue;

   for( const auto& token_ : vectorToken )
   {
      switch( token_.get_token_type() )
      {
      case token::token_type_s("OPERATOR"):
         {
            auto stringOerator = token_.get_name();

            value valueRight = stackValue.top(); 
            stackValue.pop();
            value valueLeft = stackValue.top();
            stackValue.pop();
            value result_ = evaluate_operator_g(stringOerator, valueLeft, valueRight, &runtime_ );
            stackValue.push( result_ );
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
               assert(false);
               //if( runtime_.pr_ != nullptr ) { runtime_.pr_->add("[calculate_s] - Variable not found: " + std::string(stringVariable), tag_error{}); }
               //return { false, "Variable not found: " + std::string(stringVariable) };
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




_GD_EXPRESSION_END