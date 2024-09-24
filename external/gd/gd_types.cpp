#include "gd_types.h"

_GD_TYPES_BEGIN

constexpr uint8_t puCharType_g[0x100] =
{
   //       0, 1, 2, 3,  4, 5, 6, 7,  8, 9, A, B,  C, D, E, F,
   /* 0 */ 00,00,00,00, 00,00,00,00, 01,01,00,00, 00,01,00,00,  /* 0   - 15  */
   /* 1 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 16  - 31  */
   /* 2 */ 01,00,07,00, 00,00,00,07, 00,00,00,00, 00,00,00,00,  /* 32  - 47   ,!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 02,02,02,02, 02,02,02,02, 02,02,00,00, 00,00,00,00,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

   /* 4 */ 00,03,03,03, 03,03,03,03, 03,03,03,03, 03,03,03,03,  /* 64  - 79  */
   /* 5 */ 03,03,03,03, 03,03,03,03, 03,03,03,00, 00,00,00,00,  /* 80  - 95  */
   /* 6 */ 07,03,03,03, 03,03,03,03, 03,03,03,03, 03,03,03,03,  /* 96  - 111 */
   /* 7 */ 03,03,03,03, 03,03,03,03, 03,03,03,00, 00,00,00,00,  /* 112 - 127 */

   /* 8 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 128 - 143 */
   /* 9 */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 144 - 159 */
   /* A */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 160 - 175 */
   /* B */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 176 - 191 */

   /* C */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 192 - 207 */
   /* D */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 208 - 223 */
   /* E */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00,  /* 224 - 239 */
   /* F */ 00,00,00,00, 00,00,00,00, 00,00,00,00, 00,00,00,00   /* 240 - 255 */
};




/// 256 word values with bits set to mark different character classes used in parse logic
/// 0x0020 = CHAR_GROUP_DECIMAL | CHAR_GROUP_SCIENTIFIC
/// 0x04E2 = CHAR_GROUP_ALNUM | CHAR_GROUP_SCIENTIFIC | CHAR_GROUP_HEX | CHAR_GROUP_DECIMAL | CHAR_GROUP_DIGIT (number) 
/// 0x0404 = CHAR_GROUP_ALNUM | CHAR_GROUP_ALPHABET
/// 0x0444 = CHAR_GROUP_ALNUM | CHAR_GROUP_HEX | CHAR_GROUP_ALPHABET
/// 0x04E4 = CHAR_GROUP_ALNUM | CHAR_GROUP_SCIENTIFIC | CHAR_GROUP_HEX | CHAR_GROUP_DECIMAL | CHAR_GROUP_ALPHABET
/// 0x0010 = CHAR_GROUP_QUOTE (quote)
constexpr uint16_t puCharGroup_g[0x100] =
{
   //   0,     1,     2,     3,     4,     5,     6,     7,     8,     9,     A,     B,     C,     D,     E,     F
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0001,0x0001,0x0000,0x0001,0x0000,0x0000,0x0000, /* 0x00-0x0F */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0001,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x10-0x1F */
   0x0001,0x0000,0x0010,0x0000,0x0000,0x0000,0x0000,0x0010,0x0000,0x0001,0x0008,0x0008,0x0000,0x0008,0x00A0,0x1008, /* 0x20-0x2F  ,!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x04E2,0x0000,0x0000,0x0008,0x0008,0x0008,0x0000, /* 0x30-0x3F 0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */
   0x0000,0x0444,0x0444,0x0444,0x0444,0x04E4,0x0444,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404, /* 0x40-0x4F @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O */
   0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0008,0x1000,0x0000,0x0008,0x0008, /* 0x50-0x5F P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_ */
   0x0000,0x0444,0x0444,0x0444,0x0444,0x04E4,0x0444,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404, /* 0x60-0x6F `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o */
   0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0404,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x70-0x7F p,q,r,s,t,u,v,w,x,y,z,{,|,},~*/
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x80-0x8F */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0x90-0x9F */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xA0-0xAF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xB0-0xBF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xC0-0xCF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xD0-0xDF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xE0-0xEF */
   0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000, /* 0xF0-0xFF */
};

// static_assert( ctype_g("digit") & CHAR_GROUP_DIGIT, "Wrong type for '1'");
// static_assert( (puCharGroup_g[uint8_t('1')] && ctype_g("digit")) == ctype_g("digit"), "Wrong type for '1'");
// static_assert( ctype_g("alphabet") == CHAR_GROUP_ALPHABET );
// static_assert( is_ctype( 'a', ctype_g("alphabet") ) == true, "Wrong type for 'a'");


/// 256 byte values with bits set to mark different character classes used in parse logic
//static const uint16_t pCharGroup__s[0x100] =
//{
//   // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
//   00,00,00,00,00,00,00,00,00,64,64,00,00,00,00,00, /* 0x00-0x0F */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x10-0x1F */
//   64,00,128,00,00,00,00,128,00,00,00,02,00,10, 8,00, /* 0x20-0x2F  ,!,",#,$,%,&,',(,),*,+,,,-,.,/*/
//   31,31,31,31,31,31,31,31,31,31,00,00,00,00,00,00, /* 0x30-0x3F 0,1,2,3,4,5,6,7,8,9 ... */
//   00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,01, /* 0x40-0x4F @,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O*/
//   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x50-0x5F P,Q,R,S,T,U,V,W,X,Y,Z,[,\,],^,_*/
//   00,01,01,01,01,01,01,01,01,01,01,01,01,01,01,01, /* 0x60-0x6F `,a,b,c,d,e,f,g,h,i,j,k,l,m,n,o*/
//   01,01,01,01,01,01,01,01,01,01,01,00,00,00,00,00, /* 0x70-0x7F p,q,r,s,t,u,v,W,x,y,z,{,|,},~*/
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x80-0x8F */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0x90-0x9F */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xA0-0xAF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xB0-0xBF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xC0-0xCF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xD0-0xDF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xE0-0xEF */
//   00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00, /* 0xF0-0xFF */
//};

/// Get constant for specified character type name
uint8_t ctype_g(const std::string_view& stringCType, tag_main_type )
{
   using namespace detail;
   uint32_t uCTypeName = hash_type( stringCType ); 
   switch( uCTypeName )
   {
   case hash_type("space"):            return CHAR_TYPE_SPACE;
   case hash_type("digit"):            return CHAR_TYPE_DIGIT;
   case hash_type("alphabet"):         return CHAR_TYPE_ALPHABET;
   case hash_type("operator"):         return CHAR_TYPE_OPERATOR;
   case hash_type("quote"):            return CHAR_GROUP_QUOTE;
   case hash_type("decimal"):          return CHAR_TYPE_PUNCTUATOR;
   }

   return 0;
}

/// Get constant for specified character type name
uint16_t ctype_g(const std::string_view& stringCType)
{
   using namespace detail;
   uint32_t uCTypeName = hash_type( stringCType ); 
   switch( uCTypeName )
   {
   case hash_type("space"):            return CHAR_GROUP_SPACE;
   case hash_type("digit"):            return CHAR_GROUP_DIGIT;
   case hash_type("alphabet"):         return CHAR_GROUP_ALPHABET;
   case hash_type("operator"):         return CHAR_GROUP_OPERATOR;
   case hash_type("quote"):            return CHAR_GROUP_QUOTE;
   case hash_type("decimal"):          return CHAR_GROUP_DECIMAL;
   case hash_type16("hex"):            return CHAR_GROUP_HEX;
   case hash_type("scientific"):       return CHAR_GROUP_SCIENTIFIC;
   case hash_type("punctuation"):      return CHAR_GROUP_PUNCTUATION;
   case hash_type("bracket"):          return CHAR_GROUP_BRACKET;
   case hash_type("alnum"):            return CHAR_GROUP_ALNUM;
   case hash_type16("xml"):            return CHAR_GROUP_XML;
   case hash_type16("file"):           return CHAR_GROUP_FILE;
   }

   return 0;
}

/** ---------------------------------------------------------------------------
 * @brief Check if string buffer is text, number or decimal
 * This method tries to figure out if value is integer, decimal or text, just those
 * types form text.
 * @param puText start of text
 * @param uLength text length
 * @return constant for the type group, eTypeGroupInteger, eTypeGroupDecimal or eTypeGroupString
 */
unsigned detect_ctypegroup_g( const uint8_t* puText, unsigned uLength )
{
   unsigned uType = 0;
   unsigned uCharType;
   const uint8_t* puPosition = puText;
   const uint8_t* puEnd = puText + uLength;

   if( uLength == 0 ) return 0;

   // ## special case, check for starting - character
   uCharType = get_ctype_g( *puPosition );
   if( *puPosition == '-' )
   {
      uType = CHAR_GROUP_DIGIT;
      puPosition++;
   }
   else if( *puPosition == '.' )
   {
      uType = CHAR_GROUP_DECIMAL;
      puPosition++;
   }
   else if( uCharType & CHAR_GROUP_DIGIT )
   {
      uType = CHAR_GROUP_DIGIT;
      puPosition++;
   }
   else { return eTypeGroupString; }

   // ## Check rest to find out if it is a digit, decimal or string
   //    If we have found a digit and then finds a decimal character type is
   //    modified to decimal. 
   //    But if type is decimal and it finds another decimal type is string
   const uint16_t uFlagToCheck = (CHAR_GROUP_DIGIT | CHAR_GROUP_DECIMAL);
   while( puPosition != puEnd )
   {
      auto uCharType = get_ctype_g( *puPosition );
      uCharType &= uFlagToCheck; 
      if( uCharType != 0 )
      {
         // Test if only decimal, if no digit and only decimal we have to check for proper decimal value
         if( uCharType == CHAR_GROUP_DECIMAL )
         {
            if( uType == CHAR_GROUP_DIGIT ) uType = CHAR_GROUP_DECIMAL;
            else 
            {
               uType = CHAR_GROUP_ALPHABET;                                    // set to text because we have more than one decimal value
               break;
            }
         }
      }
      else                                                                     // not a number or decimal char, then it is probably text
      {
         uType = CHAR_GROUP_ALPHABET;
         break;
      }

      puPosition++;
   }

   // ## return type group
   if( uType == CHAR_GROUP_DIGIT ) return eTypeGroupInteger;
   else if( uType == CHAR_GROUP_DECIMAL ) return eTypeGroupDecimal;

   return eTypeGroupString;
}




_GD_TYPES_END