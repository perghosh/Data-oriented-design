#pragma once

#include <cstring>
#include <string>
#include <string_view>


#ifndef _GD_EXPRESSION_BEGIN
#define _GD_EXPRESSION_BEGIN namespace gd { namespace expression {
#define _GD_EXPRESSION_END } }
#endif

_GD_EXPRESSION_BEGIN

/// tag dispatcher used for digit values
struct tag_digit {};


enum enumCharacterClass
{
   eCharacterClassSpace       = 0b0000'0001,
   eCharacterClassNumber      = 0b0000'0010,
   eCharacterClassName        = 0b0000'0100,
   eCharacterClassOperator    = 0b0000'1000,
   eCharacterClassPunctuator  = 0b0001'0000,
   eCharacterClassString      = 0b0010'0000,
   eCharacterClassEnd         = 0b0100'0000,
   eCharacterClassSpecial     = 0b1000'0000,
};


/**
 * \brief
 *
 *
 */
struct token
{
   enum enumTokenType 
   {
      eTokenTypeNumberBoolean    = 1,
      eTokenTypeNumber,
      eTokenTypeString,
      eTokenTypeOperator,
      eTokenTypeLabel,
      eTokenTypeSpecial,
      eTokenTypeKeyword       = 10,
   };

   enum enumTokenGroup
   {
      eTokenGroupNumber          = 0x0000'0100,
      eTokenGroupInteger         = 0x0000'0200,
      eTokenGroupDecimal         = 0x0000'0400,
      eTokenGroupSigned          = 0x0000'0800,
   };

   union value
   {
      value(): u64(0) {}

      bool        b;
      uint64_t    u64;
      int64_t     i64;
      double      d;
      const char* pbsz;
      const uint8_t* pu;
   };

// ## construction ------------------------------------------------------------
   token() {}
   token( unsigned uType, const char* pbsz, unsigned uLength ): m_uType{uType}, m_pbszData{pbsz}, m_uLength{ uLength } {}
   // copy
   token(const token& o) { common_construct(o); }
   // assign
   token& operator=(const token& o) { common_construct(o); return *this; }

   ~token() {}
   // common copy
   void common_construct(const token& o) { memcpy( this, &o, sizeof(token) ); }

   unsigned type() const { return m_uType; }
   unsigned length() const { return m_uLength; }

// ## methods -----------------------------------------------------------------
   void set( const char* pbszBegin, unsigned uLength ) { m_pbszData = pbszBegin, m_uLength = uLength; }
   void set( const char* pbszBegin, const char* pbszEnd ) { set( pbszBegin, (unsigned)(pbszEnd - pbszBegin) ); }

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------

   unsigned m_uType = 0;
   const char* m_pbszData = nullptr;
   unsigned m_uLength = 0;
   //value m_value;

// ## free functions ----------------------------------------------------------
   static const char* next_s( const char* pbszBegin, const char* pbszEnd, uint8_t& uNextToken );
   static const char* next_s( const std::string_view& stringExpression, uint8_t& uNextToken );

   static const char* next_s( const char* pbszBegin, const char* pbszEnd, token* ptoken );
   static const char* next_s( const std::string_view& stringExpression, token* ptoken );
   //static const char* next_s( const char* pbszBegin, const char* pbszEnd, token* ptoken );

   /// Skip token 
   static const char* skip_s( const char* pbszBegin, const char* pbszEnd, uint8_t uTokenClass );
   static const char* skip_s( const std::string_view& stringExpression, uint8_t uTokenClass );
   static void skip_s( uint8_t uTokenClass, std::string_view& stringExpression );

   static const char* read_s( const char* pbszBegin, const char* pbszEnd, token* ptoken, tag_digit );

   static const char* read_s( const char* pbszBegin, const char* pbszEnd, uint8_t uClass, token* ptoken );

   static const char* read_number_s( const char* pbszBegin, const char* pbszEnd, token* ptoken );

   static constexpr std::string_view get_token_name_s( uint8_t uTokem );

};

inline const char* token::next_s(const std::string_view& stringExpression, token* ptoken ) {
   return next_s( stringExpression.data(), stringExpression.data() + stringExpression.length(), ptoken );
}

inline const char* token::next_s(const std::string_view& stringExpression, uint8_t& uNextToken) {
   return token::next_s( stringExpression.data(), stringExpression.data() + stringExpression.length(), uNextToken );
}

inline const char* token::skip_s(const std::string_view& stringExpression, uint8_t uTokenClass) {
   return token::skip_s( stringExpression.data(), stringExpression.data() + stringExpression.length(), uTokenClass );
}

inline void token::skip_s(uint8_t uTokenClass, std::string_view& stringExpression) {
   const auto* p_ = token::skip_s( stringExpression.data(), stringExpression.data() + stringExpression.length(), uTokenClass );
   if( p_ != nullptr ) {
      size_t uLength = (stringExpression.data() + stringExpression.length()) - p_;
      stringExpression = std::string_view( p_, uLength );
   }
   else stringExpression = std::string_view();
}


inline constexpr std::string_view token::get_token_name_s(uint8_t uTokem)
{
   if( (uTokem & eCharacterClassNumber) != 0 ) return "number";
   else if( (uTokem & eCharacterClassOperator) != 0 ) return "operator";
   else if( (uTokem & eCharacterClassName) != 0 ) return "name";

   return "unknown";
}

_GD_EXPRESSION_END