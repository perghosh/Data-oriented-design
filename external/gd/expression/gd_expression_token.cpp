#include <stack>

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
   /* 2 */ 0x01,0x40,0x20,0x40, 0x40,0x40,0x40,0x20, 0x40,0x40,0x40,0x48, 0x10,0x48,0x12,0x40,  /* 32  - 47   ' ',!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 0x02,0x02,0x02,0x02, 0x02,0x02,0x02,0x02, 0x02,0x02,0x40,0x10, 0x40,0x40,0x40,0x40,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

   /* 4 */ 0x40,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,  /* 64  - 79  @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O */
   /* 5 */ 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x40, 0x40,0x40,0x40,0x40,  /* 80  - 95  P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_ */
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

      if( uCharacterType & ALPHABETIC_BIT )                                             // Identifier
      {
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
         //vectorToken.emplace_back(std
      }
   }

   return { true, "" };
}

std::pair<bool, std::string> token::convert_s(const std::vector<token>& vectorIn, std::vector<token>& vectorOut, tag_postfix)
{
   std::stack<token> stackOperator;

   for( const auto& token_ : vectorIn )
   {
      uint32_t uTokenType = token_.get_token_type();
      switch( uTokenType )
      {
      case token_type_s("OPERATOR"):
         stackOperator.push(token_);
         break;
      case token_type_s("VALUE"):
         vectorOut.emplace_back(token_);
         break;
      case token_type_s("VARIABLE"):
         vectorOut.emplace_back(token_);
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







_GD_EXPRESSION_END