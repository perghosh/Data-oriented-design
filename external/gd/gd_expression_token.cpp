#include <cassert>

#include "gd/gd_types.h"

#include "gd_expression_token.h"

_GD_EXPRESSION_BEGIN

/**
* @brief Lookup table used to convert hexadecimal value in text to
* value stored in byte.
* It takes the ascii code for character and use the ascii number to find
* what hex value it is in this table.
*/
static const uint8_t pHexValue_s[0x100] =
{
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x00-0x0F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x10-0x1F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x20-0x2F */
   0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0, /* 0x30-0x3F (0 - 9) */
   0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x40-0x4F (A - F) */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x50-0x5F */
   0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0, /* 0x60-0x6F (a - f) */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x70-0x7F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xC0-0xCF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xD0-0xDF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xE0-0xEF */
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
};


constexpr uint8_t SP = eCharacterClassSpace;       ///< skip character, no meaning
constexpr uint8_t NO = eCharacterClassNumber;      ///< number characters
constexpr uint8_t NA = eCharacterClassName;        ///< characters for names
constexpr uint8_t OP = eCharacterClassOperator;    ///< operator characters, some sort of operation 
constexpr uint8_t PU = eCharacterClassPunctuator;  ///< characters that divide
constexpr uint8_t EN = eCharacterClassEnd;

const uint8_t pTokenJsClass_g[256] =
{
   //       0, 1, 2, 3,  4, 5, 6, 7,  8, 9, A, B,  C, D, E, F,
   /* 0 */ 00,00,00,00, 00,00,00,00, SP,SP,00,00, 00,SP,00,00,  /* 0   - 15  */
   /* 1 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 16  - 31  */
   /* 2 */ 00,00,00,00, 00,00,00,00, 00,PU,PU,00, PU,OP,PU,00,  /* 32  - 47  PU = (),.*/
   /* 3 */ 00,NO,NO,NO, NO,NO,NO,NO, NO,NO,00,EN, 00,00,00,PU,  /* 48  - 63  PU = ? */  

   /* 4 */ 00,NA,NA,NA, NA,NA,NA,NA, NA,NA,NA,NA, NA,NA,NA,NA,  /* 64  - 79  */
   /* 5 */ 00,NA,NA,NA, 00,NA,NA,NA, 00,NA,NA,00, 00,00,00,00,  /* 80  - 95  */
   /* 6 */ 00,NA,NA,NA, NA,NA,NA,NA, NA,NA,NA,NA, NA,NA,NA,NA,  /* 96  - 111 */
   /* 7 */ NA,NA,NA,NA, NA,NA,NA,NA, NA,NA,00,00, 00,00,00,00,  /* 112 - 127 */

   /* 8 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 128 - 143 */
   /* 9 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 144 - 159 */
   /* A */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 160 - 175 */
   /* B */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 176 - 191 */

   /* C */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 192 - 207 */
   /* D */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 208 - 223 */
   /* E */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 224 - 239 */
   /* F */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00   /* 240 - 255 */
};



const char* token::next_s(const char* pbszBegin, const char* pbszEnd, token* ptoken)
{
   using namespace gd::types;
   const char* pbszPosition = pbszBegin;

   while(pbszPosition < pbszEnd)
   {
      uint8_t uType = puCharType_g[*pbszPosition];
      switch(uType)
      {
      case cchartype_g( "space", tag_main_type{}):
         pbszPosition++;
         break;

      case cchartype_g( "digit", tag_main_type{}):                             // found digit, then read number and return
         pbszPosition = read_s( pbszPosition, pbszEnd, ptoken, tag_digit{});
         return pbszPosition;

      case cchartype_g( "alphabet", tag_main_type{}):
         break;
      }
   }

   return pbszPosition;
}

const char* token::read_s(const char* pbszBegin, const char* pbszEnd, token* ptoken, tag_digit)
{
   using namespace gd::types;
   uint16_t uCType = 0;
   const char* pbszPosition = pbszBegin;
   while(is_ctype_g(*pbszPosition, "decimal"_ctype) && pbszPosition < pbszEnd)
   {
      uCType |= get_ctype_g( *pbszPosition );
      pbszPosition++;
   }

   ptoken->set( pbszBegin, pbszPosition );
   //ptoken->set( pbszBegin, pbszPosition );

   return pbszPosition;
}


inline bool compare_token(const uint8_t* p_, uint8_t uToken) {
   bool b_ = (pTokenJsClass_g[*p_] & uToken) != 0;
   return b_;
}

/**
 * @brief 
 * @param pbszBegin 
 * @param pbszEnd 
 * @param uNextToken 
 * @return 
*/
const char* token::next_s(const char* pbszBegin, const char* pbszEnd, uint8_t& uNextToken)
{                                                                                                  assert( pbszBegin < pbszEnd );
   const uint8_t* puPosition = (const uint8_t*)pbszBegin;
   while( puPosition < (const uint8_t*)pbszEnd )
   {
      uint8_t uToken = pTokenJsClass_g[*puPosition];
      if( uToken != 0 )
      {
         uNextToken = uToken;
         return (const char*)puPosition;
      }
      
      puPosition++;
   }

   uNextToken = 0;
   return nullptr;
}

const char* token::skip_s(const char* pbszBegin, const char* pbszEnd, uint8_t uTokenClass )
{
   const uint8_t* puPosition = (const uint8_t*)pbszBegin;

   if( (uTokenClass & eCharacterClassString) == 0)
   {
      while( (void*)puPosition < (void*)pbszEnd && compare_token( puPosition, uTokenClass ) == true ) puPosition++;
      
      if( (void*)puPosition < (void*)pbszEnd ) return (const char*)puPosition;
   }
   else
   {

   }

   return nullptr;
}

const char* token::read_s(const char* pbszBegin, const char* pbszEnd, uint8_t uClass, token* ptoken)
{
   if(uClass & eCharacterClassNumber)
   {
      read_number_s( pbszBegin, pbszEnd, ptoken );
   }

   return nullptr;
}

const char* token::read_number_s(const char* pbszBegin, const char* pbszEnd, token* ptoken)
{
   unsigned uType = eTokenTypeNumber;
   bool bIsNegative = false;
   bool bFoundDecimal = false;
   unsigned uLength = 0;
   const uint8_t* puPosition = (const uint8_t*)pbszBegin;

   if(*puPosition == '-')
   {
      puPosition++;
      bIsNegative = true;
      uLength++;
   }

   while( puPosition < (const uint8_t*)pbszEnd )
   {
      uint8_t uToken = pTokenJsClass_g[*puPosition];
      if( uToken == eCharacterClassNumber ) { uLength++; }
      else if(uToken == eCharacterClassPunctuator && *puPosition == '.')
      {
         if( bFoundDecimal == true ) break;

         uLength++;
         bFoundDecimal = true;
         uType &= eTokenGroupDecimal;
      }

      puPosition++;
   }

   *ptoken = { uType, pbszBegin, uLength };

   return (const char*)puPosition;
}



_GD_EXPRESSION_END

