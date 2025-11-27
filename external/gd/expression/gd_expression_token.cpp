/**
 * @file gd_expression_token.cpp
 * @brief Token implementations for expression parsing and evaluation
 */

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
constexpr uint8_t KEYWORD_OPERATOR_START_BIT = 0x80;

/// Lookup table for character classification based om symbolic operators
constexpr uint8_t puCharacterSymbolicGroup_g[0x100] =
{
   //         0,   1,   2,   3,    4,   5,   6,   7,    8,   9,   A,   B,    C,   D,   E,   F
   /* 0 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x01,0x01,0x01,0x01, 0x01,0x01,0x00,0x00,  /* 0   - 15  */
   /* 1 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 16  - 31  */
   /* 2 */ 0x01,0x48,0x20,0x40, 0x40,0x48,0x48,0x20, 0x40,0x40,0x48,0x48, 0x10,0x48,0x12,0x48,  /* 32  - 47   ' ',!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x44,0x10, 0x48,0x48,0x48,0x40,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

   /* 4 */ 0x40,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,  /* 64  - 79  @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O */
   /* 5 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x40, 0x40,0x40,0x48,0x04,  /* 80  - 95  P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_ */
   /* 6 */ 0x40,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,  /* 96  - 111 `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o */
   /* 7 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x40, 0x48,0x40,0x40,0x00,  /* 112 - 127 p,q,r,s,t,u,v,w,x,y,z,{,|,},~,DEL */

   /* 8 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 128 - 143 */
   /* 9 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 144 - 159 */
   /* A */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 160 - 175 */
   /* B */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 176 - 191 */

   /* C */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 192 - 207 */
   /* D */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 208 - 223 */
   /* E */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 224 - 239 */
   /* F */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00   /* 240 - 255 */
};

/// Lookup table for character classification based om keyword operators
constexpr uint8_t puCharacterKeywordGroup_g[0x100] =
{
   //         0,   1,   2,   3,    4,   5,   6,   7,    8,   9,   A,   B,    C,   D,   E,   F
   /* 0 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x01,0x01,0x01,0x01, 0x01,0x01,0x00,0x00,  /* 0   - 15  */
   /* 1 */ 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,  /* 16  - 31  */
   /* 2 */ 0x01,0x48,0x20,0x40, 0x40,0x48,0x48,0x20, 0x40,0x40,0x48,0x48, 0x10,0x48,0x12,0x48,  /* 32  - 47   ' ',!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x44,0x10, 0x48,0x48,0x48,0x40,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */

   /* 4 */ 0x40,0x84,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x84,0x04,0x04, 0x04,0x84,0x84,0x84,  /* 64  - 79  @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O */
   /* 5 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x84,0x04,0x04,0x40, 0x40,0x40,0x48,0x04,  /* 80  - 95  P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_ */
   /* 6 */ 0x40,0x84,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x84,0x04,0x04, 0x04,0x84,0x84,0x84,  /* 96  - 111 `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o */
   /* 7 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x84,0x04,0x04,0x40, 0x48,0x40,0x40,0x00,  /* 112 - 127 p,q,r,s,t,u,v,w,x,y,z,{,|,},~,DEL */

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
   while(pisz_ < piszEnd && (puCharacterSymbolicGroup_g[static_cast<uint8_t>(*pisz_)] & WHITESPACE_BIT))
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

   while( pisz_ < piszEnd && (puCharacterSymbolicGroup_g[static_cast<uint8_t>(*pisz_)] & DIGIT_BIT) ) 
   { 
      uType |= puCharacterSymbolicGroup_g[static_cast<uint8_t>(*pisz_)];               // Set the type
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

   const char* pisz_ = uDelimiterLength != 2 ? piszBegin + uDelimiterLength : nullptr; // move to first character or if empty string, it is set to null

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
   else
   {
      string_ = std::string_view();                                            // empty string
      pisz_ = piszBegin;                                                       // set past first quote character
   }

   if( ppiszReadTo != nullptr ) { *ppiszReadTo = pisz_ + uDelimiterLength; }   // Move past the ending delimiter

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
   while( pisz_ < piszEnd && ( puCharacterSymbolicGroup_g[static_cast<uint8_t>(*pisz_)] & ALPHABETIC_BIT ) )
   {
      uType |= puCharacterSymbolicGroup_g[static_cast<uint8_t>(*pisz_)] & (ALPHABETIC_BIT|SPECIAL_CHAR_BIT);               // Set the type
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

      uint8_t uCharacterType = puCharacterSymbolicGroup_g[static_cast<uint8_t>(*piszPosition)];  // Get the type of the character

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
            if( piszPosition[1] == '=' || piszPosition[1] == '&' || piszPosition[1] == '|' )       // Handle special operators that have two characters like >=, <=, == etc
            {
               vectorToken.emplace_back(token(uTokenType, std::string_view(piszPosition, 2)));
               piszPosition += 2;
               continue;
            }
            else if( piszPosition[0] == '-' )                                  // special case to handle negate
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


/** ---------------------------------------------------------------------------
 * @brief Parses a string into tokens with keyword operator support.
 *
 * This function processes the input string and generates a vector of tokens.
 * It handles keyword operators (and, or, not, in, is, xor, mod) in addition
 * to all standard token types.
 *
 * @param piszBegin Pointer to the beginning of the input string.
 * @param piszEnd Pointer to the end of the input string.
 * @param vectorToken Vector to store the generated tokens.
 * @param tag_formula_keyword Tag to indicate keyword operator parsing mode.
 * @return A pair containing a boolean indicating success and a string with an error message if any.
 */
std::pair<bool, std::string> token::parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_formula_keyword )
{
   const char* piszPosition = piszBegin;                                       // Current position

   /// ## Loop through the string and parse the tokens .......................

   while( piszPosition < piszEnd )
   {
      piszPosition = skip_whitespace_s(piszPosition, piszEnd);                 // Skip whitespace
      if( piszPosition >= piszEnd ) { break; }  

#ifndef NDEBUG
      char iCharacter_d = *piszPosition;
#endif

      uint8_t uCharacterType = puCharacterKeywordGroup_g[static_cast<uint8_t>(*piszPosition)];  // Get the type of the character

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

      // ### Check for keyword operator, variable, method or reserved keyword
      if( uCharacterType & ALPHABETIC_BIT )                                    // Identifier
      {
         // ### Try keyword operator first if character can start one
         if( uCharacterType & KEYWORD_OPERATOR_START_BIT )
         {
            std::string_view stringOperator;
            const char* piszEnd_ = nullptr;
            if( operator_read_keyword_s(piszPosition, piszEnd, stringOperator, &piszEnd_) == true )
            {
               uint32_t uTokenType = token::token_type_s("OPERATOR");
               vectorToken.emplace_back(token(uTokenType, stringOperator));
               piszPosition = piszEnd_;
               continue;
            }
         }

         // ### Process as variable/function/label
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
               if( uType & SPECIAL_CHAR_BIT )                                 // If special char this has to have a ':' character
               {
                  uTokenType |= uint32_t(eFunctionNamespace);                 // add namespace flag to find method among namespaced methods
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
               else
               {
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
               
               // vectorToken.emplace_back(token::token_type_s("SPECIAL_CHAR"), "(");// Add parenthesis token if function: not working
            }

            
            piszPosition = piszEnd_;
         }
         continue;
      }

      if( uCharacterType & OPERATOR_BIT )                                     // Operator
      {
         uint32_t uTokenType = token::token_type_s( "OPERATOR" );
         if( uCharacterType & SPECIAL_CHAR_BIT )
         {
            if(piszPosition[0] == '=' )                                       // Handle = when parsing keyword and add it as ==
            {
               vectorToken.emplace_back(token(uTokenType, token::operator_s(eOperatorEqual)));
               piszPosition++;
               continue;
				}
            else if( (piszPosition[0] == '<' || piszPosition[0] == '>') && piszPosition[1] == '=' ) // Handle special operators that have two characters like >=, <=
            {
               vectorToken.emplace_back(token(uTokenType, std::string_view(piszPosition, 2)));
               piszPosition += 2;
               continue;
            }
            else if( piszPosition[0] == '-' )                                 // special case to handle negate
            {
               auto type_ = vectorToken.empty() == false ? vectorToken.back().get_token_type() : token::token_type_s("OPERATOR");
               if( type_ == token::token_type_s("OPERATOR") )                 // Was previous token an operator
               {
                  // ## this has to be a unary operator for negative number try to read number
                  uTokenType = token::token_type_s( "VALUE" );                // value token
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

      if( uCharacterType & SPECIAL_CHAR_BIT )                                  // Special character
      {
         uint32_t uTokenType = token::token_type_s( "SPECIAL_CHAR" );
         char iCharacter = *piszPosition;
         vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         piszPosition++;
         continue;

/*

         char iCharacter = *piszPosition;
         if( iCharacter == '(' )
         {
            vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         }
         else if( iCharacter == ')' )
         {
            vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         }
         else
         {
            return { false, "[parse_s] - Unsupported special character: " + std::string(1, iCharacter) };
         }

*/
      }

      if( uCharacterType & SEPARATOR_BIT )                                     // Separator
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

/** ---------------------------------------------------------------------------
 * @brief Parses a postfix (RPN) expression string into tokens.
 *
 * This function processes a string that is already in postfix notation and 
 * generates a vector of tokens without any reordering. The expression should
 * already be in the correct evaluation order (operands before operators).
 *
 * Unlike parse_s with tag_formula, this function:
 * - Does not validate operator precedence (assumes correct order)
 * - Treats parentheses as regular special characters (not grouping)
 * - Processes tokens in the exact order they appear
 *
 * @code
 * // sample usage
 * const char* postfix = "'hello' 5 length( 20 <";
 * const char* end = postfix + strlen(postfix);
 * std::vector<token> vectorToken;
 * auto result = token::parse_s(postfix, end, vectorToken, tag_postfix{});
 * // Tokens will be in exact order: 'hello', 5, length(, 20, <
 * @endcode
 *
 * @param piszBegin Pointer to the beginning of the postfix expression string.
 * @param piszEnd Pointer to the end of the postfix expression string.
 * @param vectorToken Vector to store the generated tokens.
 * @param tag_postfix Tag to indicate postfix parsing mode.
 * @return A pair containing a boolean indicating success and a string with an error message if any.
 */
std::pair<bool, std::string> token::parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_postfix)
{
   const char* piszPosition = piszBegin;                                       // Current position

   /// ## Loop through the string and parse the tokens in order ...............
   while( piszPosition < piszEnd )
   {
      piszPosition = skip_whitespace_s(piszPosition, piszEnd);                 // Skip whitespace
      if( piszPosition >= piszEnd ) { break; }  

      uint8_t uCharacterType = puCharacterSymbolicGroup_g[static_cast<uint8_t>(*piszPosition)];  // Get the type of the character

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

      // ### Check for variable, method or keyword (no special processing needed)
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
                  uTokenType |= uint32_t(eFunctionNamespace);                  // add namespace flag to find method among namespaced methods
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
               else
               {
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
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
            // Check for multi-character operators
            if( piszPosition + 1 < piszEnd )
            {
               char iNext = piszPosition[1];
               if( iNext == '=' || iNext == '&' || iNext == '|' )             // Handle special operators that have two characters like >=, <=, == etc
               {
                  vectorToken.emplace_back(token(uTokenType, std::string_view(piszPosition, 2)));
                  piszPosition += 2;
                  continue;
               }
            }

            // Handle negative numbers
            if( piszPosition[0] == '-' )
            {
               auto type_ = vectorToken.empty() == false ? vectorToken.back().get_token_type() : token::token_type_s("OPERATOR");
               if( type_ == token::token_type_s("OPERATOR") || type_ == token::token_type_s("FUNCTION") ) // Was previous token an operator or function
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

      if( uCharacterType & SPECIAL_CHAR_BIT )                                  // Special character
      {
         uint32_t uTokenType = token::token_type_s( "SPECIAL_CHAR" );
         vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         piszPosition++;
         continue;
      }

      if( uCharacterType & SEPARATOR_BIT )                                     // Separator
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

/** ---------------------------------------------------------------------------
 * @brief Parses a postfix expression with keyword operator support.
 *
 * This function processes a string that is already in postfix notation and 
 * handles keyword operators (and, or, not, in, is, xor, mod). The expression 
 * should already be in the correct evaluation order.
 *
 * @code
 * // sample usage
 * const char* postfix = "age 18 > status 'active' == and";
 * const char* end = postfix + strlen(postfix);
 * std::vector<token> vectorToken;
 * auto result = token::parse_s(postfix, end, vectorToken, tag_postfix_keyword{});
 * @endcode
 *
 * @param piszBegin Pointer to the beginning of the postfix expression string.
 * @param piszEnd Pointer to the end of the postfix expression string.
 * @param vectorToken Vector to store the generated tokens.
 * @param tag_postfix_keyword Tag to indicate postfix with keyword parsing mode.
 * @return A pair containing a boolean indicating success and a string with an error message if any.
 */
std::pair<bool, std::string> token::parse_s(const char* piszBegin, const char* piszEnd, std::vector<token>& vectorToken, tag_postfix_keyword)
{
   const char* piszPosition = piszBegin;                                       // Current position

   /// ## Loop through the string and parse the tokens in order ...............
   while( piszPosition < piszEnd )
   {
      piszPosition = skip_whitespace_s(piszPosition, piszEnd);                 // Skip whitespace
      if( piszPosition >= piszEnd ) { break; }  

      uint8_t uCharacterType = puCharacterKeywordGroup_g[static_cast<uint8_t>(*piszPosition)];  // Get the type of the character

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

      // ### Check for keyword operator, variable, method or reserved keyword
      if( uCharacterType & ALPHABETIC_BIT )                                    // Identifier
      {
         // ### Try keyword operator first if character can start one
         if( uCharacterType & KEYWORD_OPERATOR_START_BIT )
         {
            std::string_view stringOperator;
            const char* piszEnd_ = nullptr;
            if( operator_read_keyword_s(piszPosition, piszEnd, stringOperator, &piszEnd_) == true )
            {
               uint32_t uTokenType = token::token_type_s("OPERATOR");
               vectorToken.emplace_back(token(uTokenType, stringOperator));
               piszPosition = piszEnd_;
               continue;
            }
         }

         // ### Process as variable/function/label
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
                  uTokenType |= uint32_t(eFunctionNamespace);                  // add namespace flag to find method among namespaced methods
                  //if( string_.back() != '(' ) string_ = std::string_view(string_.data(), string_.length() + 1); // add ( if missing
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
               else
               {
                  vectorToken.emplace_back(token(uTokenType, string_));
               }
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
            if( piszPosition[0] == '=' )                                       // Handle = when parsing keyword and add it as ==
            {
               vectorToken.emplace_back(token(uTokenType, token::operator_s(eOperatorEqual)));
               piszPosition++;
               continue;
            }
            else if( piszPosition[0] == '-' )                                  // special case to handle negate
            {
               auto type_ = vectorToken.empty() == false ? vectorToken.back().get_token_type() : token::token_type_s("OPERATOR");
               if( type_ == token::token_type_s("OPERATOR") || type_ == token::token_type_s("FUNCTION") ) // Was previous token an operator or function
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

      if( uCharacterType & SPECIAL_CHAR_BIT )                                  // Special character
      {
         uint32_t uTokenType = token::token_type_s( "SPECIAL_CHAR" );
         vectorToken.emplace_back( token( uTokenType, std::string_view( piszPosition, 1 ) ) );
         piszPosition++;
         continue;
      }

      if( uCharacterType & SEPARATOR_BIT )                                     // Separator
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
* variables, functions, and special characters.
*
* Key behaviors:
* - Values and variables go directly to output
* - Operators are pushed to stack based on precedence
* - Functions are pushed to stack and output when closing ) is found
* - Commas pop operators back to ( without removing the (
* - Closing ) pops operators, removes (, and outputs function if present
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
         // ## Reorder based on precedence ...................................
         // Pop operators from stack to output while they have higher or equal precedence (higher precedence number means that operation is done before lower precedence)
         // Sample: if current token is + (precedence 3) and stack top is * (precedence 4), we pop * to output first and * is done before +

         while( stackOperator.empty() == false )
         {
            auto stringToken = token_.get_name();
            auto stringStackOperator = stackOperator.top().get_name();

            if( stringStackOperator == "(" ) { break; }                       // Don't pop past a left parenthesis
            if( stackOperator.top().get_token_type() == token_type_s("FUNCTION") ) { break; } // Don't pop functions during operator precedence checking

            // ## Reorder based on precedence ................................

            int iTokenPrecedence; // Precedence of current token
            if( stringToken.length() == 1 ) { iTokenPrecedence = to_precedence_g(stringToken[0], tag_optimize{}); }
            else { iTokenPrecedence = to_precedence_g(stringToken.data(), tag_optimize{}); }

            int iStackPrecedence; // Precedence of operator on stack
            if( stringStackOperator.length() == 1 ) { iStackPrecedence = to_precedence_g(stringStackOperator[0], tag_optimize{}); }
            else { iStackPrecedence = to_precedence_g(stringStackOperator.data(), tag_optimize{}); }

            if( iTokenPrecedence > iStackPrecedence ) { break; }              // Current token has higher precedence - stop popping

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
            // Pop all operators until we reach a left parenthesis
            // This ensures expressions between commas are fully evaluated
            while( stackOperator.empty() == false )
            {
               if( stackOperator.top().get_name() == "(" ) { break; }
               if( stackOperator.top().get_token_type() == token_type_s("FUNCTION") ) { break; }

               vectorOut.push_back(stackOperator.top());
               stackOperator.pop();
            }
         }
         else if( iCharacter == ';' )
         {
            // Pop all operators from stack and put them in vector
            while( stackOperator.empty() == false )
            {
               vectorOut.push_back(stackOperator.top());
               stackOperator.pop();
            }
            vectorOut.push_back(token_);
         }
         else { return { false, "[compile_s] - Unsupported separator: " + std::string(stringToken) }; }
      }
      break;

      case token_type_s("SPECIAL_CHAR"):
      {
         auto stringToken = token_.get_name();
         char iCharacter = stringToken[0];
         if( iCharacter == '(' )
         {
            stackOperator.push(token_);
         }
         else if( iCharacter == ')' )
         {
            // Pop operators until we find the matching left parenthesis
            while( stackOperator.empty() == false )
            {
               auto stringOperator = stackOperator.top().get_name();
               if( stringOperator == "(" ) { break; }
               vectorOut.push_back(stackOperator.top());
               stackOperator.pop();
            }

            // Remove the left parenthesis from stack
            if( stackOperator.empty() == false && stackOperator.top().get_name() == "(" ) 
            { 
               stackOperator.pop(); 
            }

            // If there's a function on top of the stack, add it to output
            if( stackOperator.empty() == false && 
               stackOperator.top().get_token_type() == token_type_s("FUNCTION") )
            {
               vectorOut.push_back(stackOperator.top());
               stackOperator.pop();
            }
         }
         else
         {
            // Other special characters that are not parentheses
            vectorOut.push_back(token_);
         }
      }
      break;

      default:
         assert( false );
         break;
      }
   }

   // Pop any remaining operators
   while( stackOperator.empty() == false )
   {
      vectorOut.push_back( stackOperator.top() );
      stackOperator.pop();
   }

   return { true, "" };
}
/** --------------------------------------------------------------------------
 * @brief Compiles the input tokens preserving their order (no precedence handling).
 *
 * This function takes a vector of tokens and outputs them in order, only handling
 * parentheses and function calls. Operators are output immediately without
 * precedence-based reordering. Use this when tokens are already in correct order.
 * 
 * Why use this?
 * If you want to write expression in different formats that are simpler for user to write in.
 * There you often need to modify to make it work with the default format used by the engine.
 * This might be helpfull in that type of situation because you will need more flexibility in
 * converting the expression to the default format.
 *
 * @param vectorIn Input vector of tokens to be compiled.
 * @param vectorOut Output vector to store the compiled tokens.
 * @param tag_postfix_no_precedence Tag to indicate order-preserving compilation.
 * @return A pair containing a boolean indicating success and a string with an error message if any.
 */
std::pair<bool, std::string> token::compile_s(const std::vector<token>& vectorIn, std::vector<token>& vectorOut, tag_postfix_no_precedence)
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
            // No precedence checking - just output operator immediately
            vectorOut.push_back(token_);
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
                  if( tokenTop.get_token_type() == token_type_s("VALUE") || tokenTop.get_token_type() == token_type_s("VARIABLE") ) 
                  { 
                     vectorOut.push_back(std::move(tokenTop));
                     stackOperator.pop();
                  }
               }
            }
            else if( iCharacter == ';' )
            {
               while( stackOperator.empty() == false )
               {
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
               vectorOut.push_back(std::move(token_));
            }
            else { return { false, "[compile_s] - Unsupported separator: " + std::string(stringToken) }; }
         }
      break;

      case token_type_s("SPECIAL_CHAR"):
         {
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
            else
            {
               vectorOut.push_back(token_);
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

/*
std::pair<bool, std::string> token::compile_s(const std::vector<token>& vectorIn, std::vector<token>& vectorOut, tag_postfix_no_precedence)
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
            // No precedence checking - just output operator immediately
            vectorOut.push_back(token_);
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
                  if( tokenTop.get_token_type() == token_type_s("VALUE") || tokenTop.get_token_type() == token_type_s("VARIABLE") ) 
                  { 
                     vectorOut.push_back(std::move(tokenTop));
                     stackOperator.pop();
                  }
               }
            }
            else if( iCharacter == ';' )
            {
               while( stackOperator.empty() == false )
               {
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
               vectorOut.push_back(std::move(token_));
            }
            else { return { false, "[compile_s] - Unsupported separator: " + std::string(stringToken) }; }
         }
      break;

      case token_type_s("SPECIAL_CHAR"):
         {
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
            else { vectorOut.push_back(token_); }
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
*/


/** --------------------------------------------------------------------------
 * @brief Compiles tokens to postfix using custom operator precedence.
 *
 * Internal helper that performs Shunting Yard algorithm with custom precedence.
 *
 * @param vectorIn Input vector of tokens.
 * @param vectorOut Output vector for postfix tokens.
 * @param mapPrecedence Custom precedence map.
 * @return Pair of success flag and error message.
 */
std::pair<bool, std::string> token::compile_with_precedence_s( const std::vector<token>& vectorIn, std::vector<token>& vectorOut, const std::map<std::string, int>& mapPrecedence)
{
   std::stack<token> stackOperator;

   for( const auto& token_ : vectorIn )
   {
      uint32_t uTokenType = token_.get_token_type();
      
      switch( uTokenType )
      {
      case token_type_s("OPERATOR"):
         {
            auto stringToken = token_.get_name();
            
            while( stackOperator.empty() == false )
            {
               auto stringStackOperator = stackOperator.top().get_name();
               if( stringStackOperator == "(" ) { break; }

               int iTokenPrecedence = 0;
               int iStackPrecedence = 0;

               auto itToken = mapPrecedence.find(std::string(stringToken));
               if( itToken != mapPrecedence.end() ) { iTokenPrecedence = itToken->second; }
               else
               {
                  if( stringToken.length() == 1 ) { iTokenPrecedence = to_precedence_g(stringToken[0], tag_optimize{}); }
                  else { iTokenPrecedence = to_precedence_g(stringToken.data(), tag_optimize{}); }
               }

               auto itStack = mapPrecedence.find(std::string(stringStackOperator));
               if( itStack != mapPrecedence.end() ) { iStackPrecedence = itStack->second; }
               else
               {
                  if( stringStackOperator.length() == 1 ) { iStackPrecedence = to_precedence_g(stringStackOperator[0], tag_optimize{}); }
                  else { iStackPrecedence = to_precedence_g(stringStackOperator.data(), tag_optimize{}); }
               }

               if( iTokenPrecedence > iStackPrecedence ) { break; }

               vectorOut.push_back(stackOperator.top());
               stackOperator.pop();
            }

            stackOperator.push(token_);
         }
         break;
         
      case token_type_s("VALUE"):
      case token_type_s("VARIABLE"):
         vectorOut.push_back(token_);
         break;
         
      case token_type_s("FUNCTION"):
         stackOperator.push(token_);
         break;
         
      case token_type_s("SEPARATOR"):
         {
            auto stringToken = token_.get_name();
            char iCharacter = stringToken[0];
            if( iCharacter == ',' )
            {
               while( stackOperator.empty() == false && stackOperator.top().get_name() != "(" )
               {
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
            }
            else if( iCharacter == ';' )
            {
               while( stackOperator.empty() == false )
               {
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
               vectorOut.push_back(token_);
            }
         }
         break;
         
      case token_type_s("SPECIAL_CHAR"):
         {
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
               
               if( stackOperator.empty() == false && 
                   stackOperator.top().get_token_type() == token_type_s("FUNCTION") )
               {
                  vectorOut.push_back(stackOperator.top());
                  stackOperator.pop();
               }
            }
         }
         break;
      }
   }

   while( stackOperator.empty() == false )
   {
      vectorOut.push_back(stackOperator.top());
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
 * @param pvectorReturn Pointer to store the resulting values from the calculation
 * @param runtime_ Runtime context for variable resolution
 * @return Pair of success flag and error message (if any)
 */
std::pair<bool, std::string> token::calculate_s(const std::vector<token>& vectorToken, std::vector<value>* pvectorReturn, runtime& runtime_ )
{
   std::pair<bool, std::string> result_ = { true, "" }; // default result
   std::vector<value> vectorArguments;
   std::stack<value> stackValue;
   std::string stringAssignVariable; // special case when we need to assign variable

   for( auto itToken = vectorToken.begin(); itToken != vectorToken.end(); ++itToken )
   {
      const auto& token_ = *itToken; // current token
      switch( token_.get_token_type() )
      {
      case token::token_type_s("OPERATOR"):
         {
            if( stackValue.size() < 2 ) // need at least two values on stack for binary operator
            {
               return { false, "[calculate_s] - Not enough values on stack for operator: " + std::string(token_.get_name()) };
            }

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
               else
               {
                  // peek for assign operator
                  auto itPeek = itToken;
                  ++itPeek;                                                   // peek next token
                  if( itPeek != vectorToken.end() && itPeek->get_token_type() == token::token_type_s("OPERATOR") && itPeek->get_name() == "=" )
                  {
                     // this is an assign operator, prepare variable for assignment
                     stringAssignVariable.assign( stringVariable );          // prepare this for assign operator, only one assign value can be active at any given time
                  }
                  else
                  {
                     // push nullptr value to stack
                     stackValue.push(value());                                   // push empty value to stack
                  }
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

               if( runtime_.is_debug() == true )
               {
                  if( stackValue.size() < pmethod_->in_count() )
                  {
                     return { false, "[calculate_s] - Not enough arguments for method: " + std::string(stringMethod) + " - expected: " + std::to_string(pmethod_->in_count()) + ", got: " + std::to_string(stackValue.size()) };
                  }
               }


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
                  if( pmethod_->out_count() == 0 )
                  {
                     value valueResult;
                     result_ = reinterpret_cast<method::method_0>(pmethod_->m_pmethod)( vectorArguments );
                     if( result_.first == true ) { stackValue.push(valueResult); }
                  }
                  else if( pmethod_->out_count() == 1 )
                  {
                     value valueResult;
                     result_ = reinterpret_cast<method::method_1>(pmethod_->m_pmethod)( vectorArguments, &valueResult );
                     if( result_.first == true ) { stackValue.push(valueResult); }
                  }
                  else if( pmethod_->out_count() > 1 )
                  {
                     std::vector<value> vectorReturn;
                     result_ = reinterpret_cast<method::method_2>(pmethod_->m_pmethod)( vectorArguments, &vectorReturn );
                     if( result_.first == true ) { for( auto it: vectorReturn ) stackValue.push(it); }
                  }
               }
               else
               {
                  if( pmethod_->is_runtime() == true )                         // method needs runtime, maybe to find global data
                  {
                     if( pmethod_->out_count() == 0 )
                     {
                        result_ = reinterpret_cast<method::method_runtime_0>(pmethod_->m_pmethod)( &runtime_, vectorArguments );
                     }
                     else if( pmethod_->out_count() == 1 )
                     {
                        value valueResult;
                        result_ = reinterpret_cast<method::method_runtime_1>(pmethod_->m_pmethod)( &runtime_, vectorArguments, &valueResult );
                        if( result_.first == true ) { stackValue.push(valueResult); }
                     }
                     else if( pmethod_->out_count() > 1 )
                     {
                        std::vector<value> vectorReturn;
                        result_ = reinterpret_cast<method::method_runtime_2>(pmethod_->m_pmethod)( &runtime_, vectorArguments, &vectorReturn );
                        if( result_.first == true ) { for( auto it: vectorReturn ) stackValue.push(it); }
                     }
                  }
               }

               if( result_.first == false ) { return { false, "[calculate_s] - Method call failed: " + std::string(stringMethod) + " - " + result_.second }; }
            }
            else
            {
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

   if( stackValue.empty() == false && pvectorReturn != nullptr )
   {
      // loop from stack and put all values in vector
      while( stackValue.empty() == false )
      {
         pvectorReturn->push_back(stackValue.top());
         stackValue.pop();
      }
   }

   return { true, "" };
}

/** -----------------------------------------------------------------------------
 * @brief Calculates the result of an expression from postfix tokens using runtime context
 * 
 * Wrapper around the main calculate_s function to return only the last value.
 * 
 * @param vectorToken Vector of tokens in postfix notation
 * @param pvalueResult Pointer to store the resulting value from the calculation
 * @param runtime_ Runtime context for variable resolution
 * @return Pair of success flag and error message (if any)
 */
std::pair<bool, std::string> token::calculate_s(const std::vector<token>& vectorToken, value* pvalueResult, runtime& runtime_ )
{
   std::vector<value> vectorReturn;
   auto result_ = calculate_s(vectorToken, &vectorReturn, runtime_);
   if( result_.first == false ) { return result_; }
   if( vectorReturn.empty() == false )
   {
      if( pvalueResult != nullptr ) { *pvalueResult = std::move(vectorReturn.back()); }
      return { true, "" };
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
   runtime_.add( { (unsigned)uMethodDefaultSize_g, pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, pmethodString_g, std::string("str")});
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
   runtime_.add( { (unsigned)uMethodDefaultSize_g, pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, pmethodString_g, std::string("str")});

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
      pruntime->add({ (unsigned)uMethodDefaultSize_g, pmethodDefault_g, ""});
      pruntime->add({ (unsigned)uMethodStringSize_g, pmethodString_g, "str"});
   }

   return calculate_s(stringExpression, *pruntime.get() );
}


/** --------------------------------------------------------------------------
 * @brief Converts an infix expression string directly to postfix notation string.
 *
 * This function takes an infix expression and converts it to postfix (RPN) 
 * notation as a string. Useful for generating SQL WHERE clauses or other
 * contexts where postfix string representation is needed.
 *
 * @code
 * // sample usage
 * std::string infix = "a + b * c";
 * std::string postfix = token::infix_to_postfix_s(infix);
 * // Result: "a b c * +"
 * 
 * std::string sqlWhere = "age > 18 and status = 'active'";
 * std::string postfixWhere = token::infix_to_postfix_s(sqlWhere, tag_formula_keyword{});
 * // Result: "age 18 > status 'active' = and"
 * @endcode
 *
 * @param stringExpression The infix expression to convert.
 * @param tag Optional tag to specify parsing mode (tag_formula or tag_formula_keyword).
 * @return String containing the postfix notation, tokens separated by spaces.
 */
std::string token::infix_to_postfix_s(const std::string_view& stringExpression, tag_formula)
{
   std::vector<token> vectorToken;
   auto result = parse_s(stringExpression, vectorToken, tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   std::vector<token> vectorPostfix;
   result = compile_s(vectorToken, vectorPostfix, tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   return tokens_to_string_s(vectorPostfix);
}

std::string token::infix_to_postfix_s(const std::string_view& stringExpression, tag_formula_keyword)
{
   std::vector<token> vectorToken;
   auto result = parse_s(stringExpression, vectorToken, tag_formula_keyword{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   std::vector<token> vectorPostfix;
   result = compile_s(vectorToken, vectorPostfix, tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   return tokens_to_string_s(vectorPostfix);
}


/** --------------------------------------------------------------------------
 * @brief Converts a vector of tokens to a space-separated string.
 *
 * Helper function that converts tokens back to their string representation,
 * separated by spaces. Handles proper formatting for different token types.
 *
 * @param vectorToken Vector of tokens to convert.
 * @return String representation of tokens separated by spaces.
 */
std::string token::tokens_to_string_s(const std::vector<token>& vectorToken)
{
   std::string stringResult;
   stringResult.reserve(vectorToken.size() * 4);                               // estimate space needed

   for( size_t u = 0; u < vectorToken.size(); ++u )
   {
      const auto& token_ = vectorToken[u];
      auto stringName = token_.get_name();

      switch( token_.get_token_type() )
      {
      case token_type_s("VALUE"):
         {
            if( token_.get_value_type() == eValueTypeString )
            {
               stringResult += "'";
               stringResult += stringName;
               stringResult += "'";
            }
            else
            {
               stringResult += stringName;
            }
         }
         break;

      case token_type_s("VARIABLE"):
      case token_type_s("OPERATOR"):
         stringResult += stringName;
         break;

      case token_type_s("FUNCTION"):
         {
            stringResult += stringName;
            stringResult += "(";
         }
         break;

      case token_type_s("SEPARATOR"):
         {
            char iChar = stringName[0];
            if( iChar == ',' ) { stringResult += ","; }
            else if( iChar == ';' ) { stringResult += ";"; }
         }
         break;

      case token_type_s("SPECIAL_CHAR"):
         {
            char iChar = stringName[0];
            if( iChar == '(' || iChar == ')' ) { stringResult += iChar; }
         }
         break;

      default:
         break;
      }

      if( u < vectorToken.size() - 1 ) { stringResult += " "; }               // add space between tokens
   }

   return stringResult;
}

/** --------------------------------------------------------------------------
 * @brief Converts infix to postfix with custom operator precedence.
 *
 * Extended version that allows specifying custom operator precedence rules.
 * Useful when target system has different precedence than standard C++.
 *
 * @param stringExpression The infix expression to convert.
 * @param mapPrecedence Map of operator strings to precedence values (higher = stronger binding).
 * @param tag Optional tag to specify parsing mode.
 * @return String containing the postfix notation.
 */
std::string token::infix_to_postfix_s( const std::string_view& stringExpression, const std::map<std::string, int>& mapPrecedence, tag_formula)
{
   std::vector<token> vectorToken;
   auto result = parse_s(stringExpression, vectorToken, tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   std::vector<token> vectorPostfix;
   result = compile_with_precedence_s(vectorToken, vectorPostfix, mapPrecedence);
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   return tokens_to_string_s(vectorPostfix);
}


/** --------------------------------------------------------------------------
 * @brief Reads a keyword operator from the input string.
 *
 * This function checks if the current position contains a keyword operator
 * (like 'and', 'or', 'not', 'in', etc.) and returns it if found.
 *
 * @code
 * // sample usage
 * const char* input = "x and y";
 * const char* end = input + strlen(input);
 * std::string_view result;
 * const char* readTo;
 * bool found = token::operator_read_keyword_s(input + 2, end, result, &readTo);
 * if(found) {
 *    std::cout << "Found operator: " << result << std::endl;
 * }
 * @endcode
 *
 * @param piszBegin Pointer to the beginning of the input string.
 * @param piszEnd Pointer to the end of the input string.
 * @param stringOperator Reference to a string_view to store the found operator.
 * @param ppiszReadTo Pointer to a pointer to store the position after the operator.
 * @return true if a keyword operator was found, false otherwise.
 */
bool token::operator_read_keyword_s(const char* piszBegin, const char* piszEnd, std::string_view& stringOperator, const char** ppiszReadTo)
{
   // Define keyword operators - ordered by length (longest first) to match greedily
   static const std::tuple<const char*, size_t, const char*> ppairKeywordOperators[] = {
      {"not", 3, "!="},
      {"and", 3, "&&"},
      {"or", 2, "||"},
      {"in", 2, "in"},
      {"is", 2, "is"},
      {"xor", 3, "^"},
      {"mod", 3, "%"}
   };

   const char* piPosition = piszBegin;

	if(piszEnd - piPosition < 3) { return false; }                             // Minimum length for keyword operators is 2 characters
   
   // ## Try to match each keyword operator ..................................

   for(const auto& [piKeyword, uLength, piSymbol] : ppairKeywordOperators)
   {
      bool bMatch = true; // do we have a match
      for(size_t u = 0; u < uLength; ++u)
      {
         char iChar = piPosition[u];
         char iCharKeyword = piKeyword[u];
         // Simple case-insensitive comparison (ASCII only)
         if(iChar >= 'A' && iChar <= 'Z') { iChar = iChar - 'A' + 'a'; }
         if(iChar != iCharKeyword) { bMatch = false; break; }
      }
      
      if(bMatch == true)
      {
         // Verify that the keyword is not part of a larger identifier
         // Check character before (if not at start)
         if(piPosition > piszBegin)
         {
            uint8_t uPrevType = puCharacterSymbolicGroup_g[static_cast<uint8_t>(piPosition[-1])];
            if(uPrevType & (ALPHABETIC_BIT | DIGIT_BIT)) { continue; }
         }
         
         if(piPosition + uLength < piszEnd)                                   // Check character after
         {
            uint8_t uNextType = puCharacterSymbolicGroup_g[static_cast<uint8_t>(piPosition[uLength])];
            if(uNextType & (ALPHABETIC_BIT | DIGIT_BIT)) { continue; }
         }
         
			// ## Found a valid keyword operator ................................
         stringOperator = std::string_view(piSymbol);
         if(ppiszReadTo != nullptr) { *ppiszReadTo = piPosition + uLength; }
         return true;
      }
   }
   
   return false;
}


// Maps operator enum to string representation
std::string_view token::operator_s(uint32_t uOperator) {
   using enumOperator = token::enumOperator;
   switch(uOperator) 
   {
      case enumOperator::eOperatorAdd:               return "+";
      case enumOperator::eOperatorSubtract:          return "-";
      case enumOperator::eOperatorAddAssign:         return "+=";
      case enumOperator::eOperatorAssign:            return "=";
      case enumOperator::eOperatorBitwiseAnd:        return "&";
      case enumOperator::eOperatorBitwiseAndAssign:  return "&=";
      case enumOperator::eOperatorBitwiseNot:        return "~";
      case enumOperator::eOperatorBitwiseOr:         return "|";
      case enumOperator::eOperatorBitwiseOrAssign:   return "|=";
      case enumOperator::eOperatorBitwiseXor:        return "^";
      case enumOperator::eOperatorBitwiseXorAssign:  return "^=";
      case enumOperator::eOperatorComma:             return ",";
      case enumOperator::eOperatorDecrement:         return "--";
      case enumOperator::eOperatorDivide:            return "/";
      case enumOperator::eOperatorDivideAssign:      return "/=";
      case enumOperator::eOperatorEqual:             return "==";
      case enumOperator::eOperatorGreaterThan:       return ">";
      case enumOperator::eOperatorGreaterThanEqual:  return ">=";
      case enumOperator::eOperatorIncrement:         return "++";
      case enumOperator::eOperatorLeftShift:         return "<<";
      case enumOperator::eOperatorLeftShiftAssign:   return "<<=";
      case enumOperator::eOperatorLessThan:          return "<";
      case enumOperator::eOperatorLessThanEqual:     return "<=";
      case enumOperator::eOperatorLogicalAnd:        return "&&";
      case enumOperator::eOperatorLogicalNot:        return "!";
      case enumOperator::eOperatorLogicalOr:         return "||";
      case enumOperator::eOperatorModulus:           return "%";
      case enumOperator::eOperatorModulusAssign:     return "%=";
      case enumOperator::eOperatorMultiply:          return "*";
      case enumOperator::eOperatorMultiplyAssign:    return "*=";
      case enumOperator::eOperatorNotEqual:          return "!=";
      case enumOperator::eOperatorRightShift:        return ">>";
      case enumOperator::eOperatorRightShiftAssign:  return ">>=";
      case enumOperator::eOperatorNone:              return "";
      default:                                       return "";
   }
}

// Maps string representation to operator enum
uint32_t token::operator_s(std::string_view stringOperator) 
{
    using enumOperator = token::enumOperator;
    if(stringOperator == "+")    return enumOperator::eOperatorAdd;
    if(stringOperator == "-")    return enumOperator::eOperatorSubtract;
    if(stringOperator == "+=")   return enumOperator::eOperatorAddAssign;
    if(stringOperator == "=")    return enumOperator::eOperatorAssign;
    if(stringOperator == "&")    return enumOperator::eOperatorBitwiseAnd;
    if(stringOperator == "&=")   return enumOperator::eOperatorBitwiseAndAssign;
    if(stringOperator == "~")    return enumOperator::eOperatorBitwiseNot;
    if(stringOperator == "|")    return enumOperator::eOperatorBitwiseOr;
    if(stringOperator == "|=")   return enumOperator::eOperatorBitwiseOrAssign;
    if(stringOperator == "^")    return enumOperator::eOperatorBitwiseXor;
    if(stringOperator == "^=")   return enumOperator::eOperatorBitwiseXorAssign;
    if(stringOperator == ",")    return enumOperator::eOperatorComma;
    if(stringOperator == "--")   return enumOperator::eOperatorDecrement;
    if(stringOperator == "/")    return enumOperator::eOperatorDivide;
    if(stringOperator == "/=")   return enumOperator::eOperatorDivideAssign;
    if(stringOperator == "==")   return enumOperator::eOperatorEqual;
    if(stringOperator == ">")    return enumOperator::eOperatorGreaterThan;
    if(stringOperator == ">=")   return enumOperator::eOperatorGreaterThanEqual;
    if(stringOperator == "++")   return enumOperator::eOperatorIncrement;
    if(stringOperator == "<<")   return enumOperator::eOperatorLeftShift;
    if(stringOperator == "<<=")  return enumOperator::eOperatorLeftShiftAssign;
    if(stringOperator == "<")    return enumOperator::eOperatorLessThan;
    if(stringOperator == "<=")   return enumOperator::eOperatorLessThanEqual;
    if(stringOperator == "&&")   return enumOperator::eOperatorLogicalAnd;
    if(stringOperator == "!")    return enumOperator::eOperatorLogicalNot;
    if(stringOperator == "||")   return enumOperator::eOperatorLogicalOr;
    if(stringOperator == "%")    return enumOperator::eOperatorModulus;
    if(stringOperator == "%=")   return enumOperator::eOperatorModulusAssign;
    if(stringOperator == "*")    return enumOperator::eOperatorMultiply;
    if(stringOperator == "*=")   return enumOperator::eOperatorMultiplyAssign;
    if(stringOperator == "!=")   return enumOperator::eOperatorNotEqual;
    if(stringOperator == ">>")   return enumOperator::eOperatorRightShift;
    if(stringOperator == ">>=")  return enumOperator::eOperatorRightShiftAssign;
    if(stringOperator == "")     return enumOperator::eOperatorNone;
    return enumOperator::eOperatorNone;
}

_GD_EXPRESSION_END