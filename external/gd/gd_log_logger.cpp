#include <cassert>
#include <cstdarg>
#include <chrono>
#include <stdarg.h>

#include "gd_log_logger.h"


#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wdeprecated-enum-compare"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning( disable: 4996 26812 )
#endif


_GD_LOG_LOGGER_BEGIN

// ================================================================================================
// ================================================================================= printf
// ================================================================================================


printf::printf(const char* pbszFormat, ...)
{
   va_list va_listArgument;
   va_start(va_listArgument, pbszFormat);                                        // initialize va_list (where to start)

   m_pbszText = message::printf_s(pbszFormat, va_listArgument);
   va_end(va_listArgument);
}

#ifdef _MSC_VER
printf::printf(const wchar_t* pwszFormat, ...)
{
   va_list va_listArgument;
   va_start(va_listArgument, pwszFormat);                                        // initialize va_list (where to start)

   m_pbszText = message::printf_s(pwszFormat, va_listArgument);
   va_end(va_listArgument);
}
#endif

// ================================================================================================
// ========================================================================================== ascii
// ================================================================================================

/// List of string pointers that is added to 
ascii& ascii::append(const std::pair<int, const char**>& pair_)
{
   for(int i = 0, iMax = pair_.first; i < iMax; i++)
   {
      const char* pbsz_ = pair_.second[i];
      m_stringAscii += pbsz_;
   }

   return *this;
}

ascii& ascii::append(const std::pair<int, char**>& pair_)
{
   for(int i = 0, iMax = pair_.first; i < iMax; i++)
   {
      const char* pbsz_ = pair_.second[i];
      m_stringAscii += pbsz_;
   }

   return *this;
}


ascii& ascii::append(const std::pair<int, const char**>& pair_, const std::string_view& stringSeparator )
{
   for(int i = 0, iMax = pair_.first; i < iMax; i++)
   {
      if( m_stringAscii.empty() == false ) m_stringAscii += stringSeparator;
      const char* pbsz_ = pair_.second[i];
      m_stringAscii += pbsz_;
   }

   return *this;
}

ascii& ascii::append(const std::pair<int, char**>& pair_, const std::string_view& stringSeparator )
{
   for(int i = 0, iMax = pair_.first; i < iMax; i++)
   {
      if( m_stringAscii.empty() == false ) m_stringAscii += stringSeparator;
      const char* pbsz_ = pair_.second[i];
      m_stringAscii += pbsz_;
   }

   return *this;
}


/*
   01 = letter
   02 = vowel
   04 = consonant
   08 = space
   10 = digit
*/
constexpr uint8_t puCharType_g[0x80] =
{
   //        0,   1,   2,   3,    4,   5,   6,   7,    8,   9,   A,   B,    C,   D,   E,   F,
   /* 0 */ 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00, 0X08,0X08,0X00,0X00, 0X00,0X08,0X00,0X00,  /* 0   - 15  */
   /* 1 */ 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00,  /* 16  - 31  */
   /* 2 */ 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00, 0X00,0X00,0X00,0X00,  /* 32  - 47   ,!,",#,$,%,&,',(,),*,+,,,-,.,/ */
   /* 3 */ 0X10,0X10,0X10,0X10, 0X10,0X00,0X00,0X10, 0X10,0X10,0X00,0X00, 0X00,0X00,0X00,0X00,  /* 48  - 63  0,1,2,3,4,5,6,7,8,9,:,;,<,=,>,? */  

   /* 4 */ 0X00,0X03,0X05,0X05, 0X05,0X03,0X05,0X05, 0X05,0X03,0X05,0X05, 0X05,0X05,0X05,0X03,  /* 64  - 79  */
   /* 5 */ 0X05,0X05,0X05,0X05, 0X05,0X03,0X05,0X05, 0X05,0X03,0X05,0X00, 0X00,0X00,0X00,0X00,  /* 80  - 95  */
   /* 6 */ 0X00,0X03,0X05,0X05, 0X05,0X03,0X05,0X05, 0X05,0X03,0X05,0X05, 0X05,0X05,0X05,0X03,  /* 96  - 111 */
   /* 7 */ 0X05,0X05,0X05,0X05, 0X05,0X03,0X05,0X05, 0X05,0X03,0X05,0X00, 0X00,0X00,0X00,0X00   /* 112 - 127 */

};

/** ---------------------------------------------------------------------------
 * @brief select characters to keep in ascii string
 * @code
 * LOG_FATAL( gd::log::ascii("1 2 3 4 5 6 7 8 9 0").keep( gd::log::ascii::eGroupDigit ) );
 * @endcode
 * @param uKeep - eGroupLetter = 0x01, eGroupVowel = 0x02, eGroupConsonant = 0x04, eGroupSpace = 0x08, eGroupDigit = 0x10
 * @return ascii& reference to it self
 */
ascii& ascii::keep(unsigned uKeep) 
{
   std::size_t uSet = 0;
   for( auto it = std::begin( m_stringAscii ), itEnd = std::end( m_stringAscii ); it != itEnd; it++ )
   {
      if( *it >= 0x80 ) continue;

      uint8_t uCharacter = (uint8_t)*it;
      if( (puCharType_g[uCharacter] & uKeep) == 0 ) continue;

      m_stringAscii[ uSet ] = *it;
      uSet++;
   }

   m_stringAscii.resize( uSet );

   return *this;    
}

/** ---------------------------------------------------------------------------
 * @brief Generate line for first character in stringLine or spaces if empty string
 * @code
 * LOG_DEBUG( gd::log::ascii().line( "=\n", 100 ) );
 * @endcode
 * @param stringLine character used to generate line, if more than one character rest is added in end on line
 * @param uLength how long line is
 * @return ascii& reference to it self
 */
ascii& ascii::line(const std::string_view& stringLine, unsigned uLength)
{
   char iLineCharacter = ' ';
   if( stringLine.empty() == false ) iLineCharacter = stringLine[0];
   while( uLength )
   {
      m_stringAscii += iLineCharacter;
      uLength--;
   }

   if( stringLine.length() > 1 ) { m_stringAscii += stringLine.substr( 1 ); }

   return *this;    
}

// ================================================================================================
// ======================================================================================== message
// ================================================================================================

void message::set_text(std::string_view stringText)
{
   auto uUtf8Length = gd::utf8::size(stringText);
   m_pbszText.reset( new_s( stringText.data() ));
}


/*----------------------------------------------------------------------------- append */ /**
 * append ascii text to message, adds separator if text is already set
 * \param stringAppend text to add
 * \return gd::log::message& reference to message for chaining
 */
message& message::append(const std::string_view& stringAppend)
{
   m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, stringAppend.data()));
   m_uCount++;

   return *this;
}

/*----------------------------------------------------------------------------- append */ /**
 * append ascii text to message, adds separator if text is already set
 * \param stringAppend text to add
 * \return message& reference to message for chaining
 */
message& message::append(const std::wstring_view& stringAppend)
{
   m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, stringAppend.data()));
   m_uCount++;

   return *this;
}

#if defined(__cpp_char8_t)
/*----------------------------------------------------------------------------- append */ /**
 * append ascii text to message, adds separator if text is already set
 * \param pbszUtf8Append text to add
 * \return message& reference to message for chaining
 */
message& message::append(const char8_t* pbszUtf8Append)
{
   m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, pbszUtf8Append));
   m_uCount++;

   return *this;
}
#endif

/*----------------------------------------------------------------------------- append */ /**
 * append ascii text to message, adds separator if text is already set
 * \param messageAppend text to add
 * \return message& reference to message for chaining
 */
message& message::append(const message& messageAppend)
{
   const char* pbszMessage = messageAppend.get_text();
   if( pbszMessage != nullptr ) m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, pbszMessage, gd::utf8::tag_utf8{}));
   return *this;
}

/*----------------------------------------------------------------------------- append */ /**
 * append ascii text to message, adds separator if text is already set
 * \param streamAppend text to add
 * \return message& reference to message for chaining
 */
message& message::append(const stream& streamAppend)
{
   m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, streamAppend.get_string()));
   m_uCount++;

   return *this;
}

/*----------------------------------------------------------------------------- append */ /**
 * append unicode text to message, adds separator if text is already set
 * \param streamAppend text to add
 * \return message& reference to message for chaining
 */
message& message::append(const wstream& streamAppend)
{
   m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, streamAppend.get_string()));
   m_uCount++;

   return *this;
}

/*----------------------------------------------------------------------------- append */ /**
 * append `printf` text to message, adds separator if text is already set
 * \param printfAppend text to add
 * \return message& reference to message for chaining
 */
message& message::append(const gd::log::printf& printfAppend)
{
   m_pbszText.reset(new_s(m_pbszText.get(), std::string_view{ "  " }, (const char*)printfAppend));
   m_uCount++;

   return *this;
}

/*----------------------------------------------------------------------------- append */ /**
 * append ascii text to message, adds separator if text is already set
 * \param pbszFormat format string with information how to format added text
 * \return gd::log::message& reference to message for chaining
 */
message& message::printf(const char* pbszFormat, ...)
{
   va_list va_listArgument;
   va_start(va_listArgument, pbszFormat);                                        // initialize va_list (where to start)

   char* pbszOldText = nullptr;                                                  // if text is set then keep pointer to old text to apend la

   // if text is set, then store it and append this later
   if( m_pbszText != nullptr ) pbszOldText = m_pbszText.release();

   auto pbszPrintfText = printf_s(pbszFormat, va_listArgument);

   if( pbszOldText != nullptr )
   {
      auto pbszText = pbszPrintfText.release();
      m_pbszText.reset( join_s(&pbszOldText, std::string_view{"  "}, &pbszText) );
      clear_s(&pbszOldText);
   }
   else
   {
      m_pbszText = std::move(pbszPrintfText);
   }
   m_uCount++;

   return *this;
}

#ifdef _MSC_VER
/*----------------------------------------------------------------------------- append */ /**
 * append unicode text to message, adds separator if text is already set
 * \param pwszFormat format string with information how to format added text
 * \return message& reference to message for chaining
 */
message& message::printf(const wchar_t* pwszFormat, ...)
{
   va_list va_listArgument;
   va_start(va_listArgument, pwszFormat);                                        // initialize va_list (where to start)

   char* pbszOldText = nullptr;

   // if text is set, then store it and append this later
   if( m_pbszText != nullptr ) pbszOldText = m_pbszText.release();

   auto pbszPrintfText = printf_s(pwszFormat, va_listArgument);

   if( pbszOldText != nullptr )
   {
      auto pbszText = pbszPrintfText.release();
      m_pbszText.reset(join_s(&pbszOldText, std::string_view{ "  " }, &pbszText));
      clear_s(&pbszOldText);
   }
   else
   {
      m_pbszText = std::move(pbszPrintfText);
   }


   return *this;
}
#endif


/*----------------------------------------------------------------------------- to_string */ /**
 * get message text as std string object (utf8 formated internally)
 * \return std::string
 */
std::string message::to_string() const
{
   std::string stringMessage;
   if( m_pbszTextView != nullptr ) stringMessage += m_pbszTextView;
   if( m_pbszText != nullptr ) stringMessage += m_pbszText.get();

   return stringMessage;
}


//
// ## FREE FUNCTIONS -----------------------------------------------------------------------------
//

/*----------------------------------------------------------------------------- new_s */ /**
 * Allocates text on heap and inserts text in utf8 format from string sent 
 * \param stringUnicode text copied as utf8 to allocated buffer
 * \param pbszCurrent pointer to old buffer if one is allocated
 * \return char* heap ponter to buffer, remember to handle this otherwise it is a memory leak
 */
char* message::new_s(const std::wstring_view& stringUnicode, char* pbszCurrent)
{
   auto uLength = gd::utf8::size(stringUnicode.data(), stringUnicode.data() + stringUnicode.length());
   char* pbsz = allocate_s(uLength, pbszCurrent);
   gd::utf8::convert_utf16_to_uft8((const wchar_t*)stringUnicode.data(), pbsz, gd::utf8::tag_utf8{});
   return pbsz;
}

/*----------------------------------------------------------------------------- new_s */ /**
 * Allocates text on heap and inserts text in utf8 format from string sent 
 * \param stringAscii text copied as utf8 to allocated buffer
 * \param pbszCurrent pointer to old buffer if one is allocated
 * \return char* heap ponter to buffer, remember to handle this otherwise it is a memory leak
 */
char* message::new_s(const std::string_view& stringAscii, char* pbszCurrent )
{
   auto uLength = gd::utf8::size(stringAscii);
   char* pbsz = allocate_s(uLength, pbszCurrent);
   gd::utf8::convert_ascii(stringAscii.data(), pbsz);
   return pbsz;
}

/*----------------------------------------------------------------------------- new_s */ /**
 * Special method for message combining three text values into one buffer, if first text is nullptr only last Add text is inserted.
 * This method works like adding text to existing message text if set
 * \param pbszUtf8First first text (usually text set that will be appended with more text)
 * \param stringIfFirst test added as separator if first text is set
 * \param stringAdd text that is always added
 * \param pbszCurrent pointer to old buffer if one is allocated
 * \return char* pointer to new allocaded text with combined text
 */
char* message::new_s(const char* pbszUtf8First, const std::string_view& stringIfFirst, const std::string_view& stringAdd, char* pbszCurrent)
{
   std::size_t uLength = 0;
   if( pbszUtf8First != nullptr )
   {
      uLength = strlen( pbszUtf8First );
      uLength += stringIfFirst.length();
   }

   std::size_t uFirstLength = uLength;
   uLength += gd::utf8::size( stringAdd.data() );

   char* pbszNew = allocate_s(uLength, pbszCurrent);

   if( pbszUtf8First != nullptr )
   {
      std::memcpy(pbszNew, pbszUtf8First, uFirstLength - stringIfFirst.length()); // copy first string to new buffer
      std::memcpy(pbszNew + (uFirstLength - stringIfFirst.length()), stringIfFirst.data(), stringIfFirst.length());// copy text if first text is set
   }

   gd::utf8::convert_ascii(stringAdd.data(), pbszNew + uFirstLength);

   return pbszNew;
}

/*----------------------------------------------------------------------------- new_s */ /**
 * Special method for message combining three text values into one buffer, if first text is nullptr only last Add text is inserted.
 * This method works like adding text to existing message text if set
 * \param pbszUtf8First first text (usually text set that will be appended with more text)
 * \param stringIfFirst test added as separator if first text is set
 * \param stringAdd text that is always added
 * \param pbszCurrent pointer to old buffer if one is allocated
 * \return char* pointer to new allocaded text with combined text
 */
char* message::new_s(const char* pbszUtf8First, const std::string_view& stringIfFirst, const std::wstring_view& stringAdd, char* pbszCurrent)
{
   std::size_t uLength = 0;
   if( pbszUtf8First != nullptr )                                              // if text is already set we need to make room for this text and concatenate all three
   {
      uLength = strlen( pbszUtf8First );
      uLength += stringIfFirst.length();                                       // separator chars should NOT!!! be above 0x80
   }

   std::size_t uFirstLength = uLength;                                         // store length for later
   uLength += gd::utf8::size( stringAdd.data() );                              // needed space for added unicode text
   uLength++;                                                                  // make room for zero terminator

   char* pbszNew = allocate_s( uLength, pbszCurrent );                         // create new buffer on heap where text is placed

   if( pbszUtf8First != nullptr )
   {
      std::memcpy(pbszNew, pbszUtf8First, uFirstLength - stringIfFirst.length()); // copy first string to new buffer
      std::memcpy(pbszNew + (uFirstLength - stringIfFirst.length()), stringIfFirst.data(), stringIfFirst.length());// copy text if first text is set
   }

   gd::utf8::convert_unicode(stringAdd.data(), pbszNew + uFirstLength, pbszNew + uLength );

   return pbszNew;
}

#if defined(__cpp_char8_t)
/*----------------------------------------------------------------------------- new_s */ /**
 * Special method for message combining three text values into one buffer, if first text is nullptr only last Add text is inserted.
 * This method works like adding text to existing message text if set
 * \param pbszUtf8First first text (usually text set that will be appended with more text)
 * \param stringIfFirst test added as separator if first text is set
 * \param pbszUtf8Add text that is always added
 * \param pbszCurrent pointer to old buffer if one is allocated
 * \return char* pointer to new allocaded text with combined text
 */
char* message::new_s(const char* pbszUtf8First, const std::string_view& stringIfFirst, const char8_t* pbszUtf8Add, char* pbszCurrent )
{                                                                                assert( pbszUtf8Add != nullptr ); assert( std::strlen((char*)pbszUtf8Add) < 99'999 ); // ok and realistic ?
   std::size_t uLength = 0;
   if( pbszUtf8First != nullptr )                                                // if text is already set we need to make room for this text and concatenate all three
   {
      uLength = strlen(pbszUtf8First);
      uLength += stringIfFirst.length();                                         // separator chars should NOT!!! be above 0x80
   }

   std::size_t uFirstLength = uLength;                                           // store length for later
   std::size_t uUtf8Length = std::strlen( (char*)pbszUtf8Add );                  // needed space added utf8 text
   uLength += uUtf8Length;
   uLength++;                                                                    // make room for zero terminator

   char* pbszNew = allocate_s(uLength, pbszCurrent);                             // create new buffer on heap where text is placed

   if( pbszUtf8First != nullptr )                                                // add to text ?
   {
      std::memcpy(pbszNew, pbszUtf8First, uFirstLength - stringIfFirst.length());// copy first string to new buffer
      std::memcpy(pbszNew + (uFirstLength - stringIfFirst.length()), stringIfFirst.data(), stringIfFirst.length());// copy text if first text is set
   }

   std::memcpy(pbszNew + uFirstLength, pbszUtf8Add, uUtf8Length + 1);            // append utf8 string with zero terminator

   return pbszNew;
}
#endif

/*----------------------------------------------------------------------------- new_s */ /**
 * Special method for message combining three text values into one buffer, if first text is nullptr only last Add text is inserted.
 * This method works like adding text to existing message text if set
 * \param pbszUtf8First first text (usually text set that will be appended with more text)
 * \param stringIfFirst test added as separator if first text is set
 * \param pbszUtf8Add text that is always added
 * \param tag_utf8 tag dispatcher 
 * \param pbszCurrent pointer to old buffer if one is allocated
 * \return char* pointer to new allocaded text with combined text
 */
char* message::new_s(const char* pbszUtf8First, const std::string_view& stringIfFirst, const char* pbszUtf8Add, char* pbszCurrent, gd::utf8::tag_utf8)
{
   assert(pbszUtf8Add != nullptr); assert(std::strlen((char*)pbszUtf8Add) < 99'999); // ok and realistic ?
   std::size_t uLength = 0;
   if( pbszUtf8First != nullptr )                                                // if text is already set we need to make room for this text and concatenate all three
   {
      uLength = strlen(pbszUtf8First);
      uLength += stringIfFirst.length();                                         // separator chars should NOT!!! be above 0x80
   }

   std::size_t uFirstLength = uLength;                                           // store length for later
   std::size_t uUtf8Length = std::strlen((char*)pbszUtf8Add);                  // needed space added utf8 text
   uLength += uUtf8Length;
   uLength++;                                                                    // make room for zero terminator

   char* pbszNew = allocate_s(uLength, pbszCurrent);                             // create new buffer on heap where text is placed

   if( pbszUtf8First != nullptr )                                                // add to text ?
   {
      std::memcpy(pbszNew, pbszUtf8First, uFirstLength - stringIfFirst.length());// copy first string to new buffer
      std::memcpy(pbszNew + (uFirstLength - stringIfFirst.length()), stringIfFirst.data(), stringIfFirst.length());// copy text if first text is set
   }

   std::memcpy(pbszNew + uFirstLength, pbszUtf8Add, uUtf8Length + 1);            // append utf8 string with zero terminator

   return pbszNew;
}




/*----------------------------------------------------------------------------- append_s */ /**
 * appends text to existing buffer, buffer is sent as reference because it is possible
 * that relocation is done and therefore we need to be able to set new pointer
 * \param ppbszText pointer to pointer for text
 * \param stringAdd text to add
 * \param pbszCurrent current text
 * \return char* pointer to new allocaded text with combined text
 */
char* message::append_s(char** ppbszText, const std::string_view& stringAdd, char* pbszCurrent )
{
   // ## count lengths to speed up join for the two texts sent to method
   auto uLengthText = std::strlen(*ppbszText);

   char* pbszNew = new_s(uLengthText + stringAdd.length() + 1);

   // ## insert texts to allocated buffer
   std::memcpy(pbszNew, *ppbszText, uLengthText);
   std::memcpy(pbszNew + uLengthText, stringAdd.data(), stringAdd.length() + 1);

   // ## delete memory
   clear_s(ppbszText);

   return pbszNew;
}


char* message::join_s(char** ppbszText, char** ppbszAdd)
{
   // ## count lengths to speed up join for the two texts sent to method
   auto uLengthText = std::strlen(*ppbszText);
   auto uLengthAdd = std::strlen(*ppbszAdd);

   char* pbszNew = new_s(uLengthText + uLengthAdd + 1);

   // ## insert texts to allocated buffer
   std::memcpy(pbszNew, *ppbszText, uLengthText);
   std::memcpy(pbszNew + uLengthText, *ppbszAdd, uLengthAdd + 1);
   // ## delete memory
   clear_s(ppbszText);
   clear_s(ppbszAdd);

   return pbszNew;
}

char* message::join_s(char** ppbszText, const std::string_view& stringAdd, char** ppbszAdd)
{
   // ## count lengths to speed up join for the two texts sent to method
   auto uLengthText = std::strlen(*ppbszText);
   auto uLengthAdd = std::strlen(*ppbszAdd);

   char* pbszNew = new_s(uLengthText + stringAdd.length() + uLengthAdd + 1);

   // ## insert texts to allocated buffer
   std::memcpy(pbszNew, *ppbszText, uLengthText);
   std::memcpy(pbszNew + uLengthText, stringAdd.data(), stringAdd.length());
   std::memcpy(pbszNew + uLengthText + stringAdd.length(), *ppbszAdd, uLengthAdd + 1);

   // ## delete memory
   clear_s(ppbszText);
   clear_s(ppbszAdd);

   return pbszNew;
}

/*----------------------------------------------------------------------------- get_now_date_as_wstring_s */ /**
 * return current date as wstring
 * \return std::wstring current date
 */
std::wstring message::get_now_date_as_wstring_s()
{
   enum { eBufferSize = 30 };
   auto time_pointNow = std::chrono::system_clock::now();
   std::time_t timeNow = std::chrono::system_clock::to_time_t(time_pointNow);
   wchar_t pBuffer[eBufferSize];
   std::wcsftime(pBuffer, eBufferSize, L"%H:%M:%S", std::localtime(&timeNow));
   return std::wstring(pBuffer);
}

/*----------------------------------------------------------------------------- get_now_time_as_wstring_s */ /**
 * return time as wstring
 * \return std::wstring current time
 */
std::wstring message::get_now_time_as_wstring_s()
{
   enum { eBufferSize = 30 };
   auto time_pointNow = std::chrono::system_clock::now();
   std::time_t timeNow = std::chrono::system_clock::to_time_t(time_pointNow);
   wchar_t pBuffer[eBufferSize];
   std::wcsftime(pBuffer, eBufferSize, L"%Y-%m-%d %H:%M:%S", std::localtime(&timeNow));
   return std::wstring(pBuffer);
}

/*----------------------------------------------------------------------------- wrap_s */ /**
 * wrap text between two characters
 * \param chBefore character to set before text
 * \param stringText wrapped text
 * \param chAfter character placed after wrapped text
 * \return std::wstring final wrapped text
 */
std::wstring message::wrap_s(wchar_t chBefore, const std::wstring_view& stringText, wchar_t chAfter)
{
   std::wstring stringWrapped;

   stringWrapped += chBefore;
   stringWrapped.append(stringText);
   stringWrapped += chAfter;
   return stringWrapped;
}

/*----------------------------------------------------------------------------- printf_s */ /**
 * printf text as normal printf c method and returns string as unique pointer
 * \param pbszFormat format information for how text is generated
 * \param va_listArgument variable argument list to populate string
 * \return std::unique_ptr<char> generated text
 */
std::unique_ptr<char> message::printf_s(const char* pbszFormat, va_list va_listArgument)
{
   std::unique_ptr<char> pbszReturnText;

   char* pbszOldText = nullptr;                                                 // if text is set then keep pointer to old text to apend la

#ifdef _MCS_VER
   int iLength = _vscprintf(pbszFormat, va_listArgument);                       // calculate needed length for buffer
#else
   int iLength = vsnprintf(nullptr, 0, pbszFormat, va_listArgument);            // calculate needed length for buffer
#endif   
   if( iLength < 0 ) { assert(false); return pbszReturnText; }
   iLength++;                                                                    // add zero terminator

   pbszReturnText.reset(new_s(iLength));
   char* pbszText = pbszReturnText.get();
   pbszText[0] = '\0';

   int iReturn = vsnprintf(pbszText, iLength, pbszFormat, va_listArgument); // generate message text

   // ## convert to utf8 if needed, text is stored in internally in utf8 format
   auto uUtf8Length = gd::utf8::size(pbszText) + 1;                              assert(uUtf8Length >= (uint32_t)iLength); // if less something is seriously wrong
   if( uUtf8Length > (uint32_t)iLength )                                         // do we need to convert to utf8
   {
      char* pbsz = new_s(uUtf8Length);                                           // create new buffer
      gd::utf8::convert_ascii(pbszText, pbsz);                                   // convert to utf8
      pbszReturnText.reset(pbsz);                                                // set to new buffer
   }

   return std::move(pbszReturnText);
}

#ifdef _MSC_VER
/*----------------------------------------------------------------------------- printf_s */ /**
 * printf text as normal printf c method and returns string as unique pointer
 * \param pwszFormat format information for how text is generated
 * \param va_listArgument variable argument list to populate string
 * \return std::unique_ptr<char> generated text
 */
std::unique_ptr<char> message::printf_s(const wchar_t* pwszFormat, va_list va_listArgument)
{
   std::unique_ptr<char> pbszReturnText;

   int iLength = _vscwprintf(pwszFormat, va_listArgument);                       // calculate needed length for buffer
   if( iLength < 0 ) { assert(false); return pbszReturnText; }
   iLength++;                                                                    // add zero terminator

   wchar_t* pwszText = (wchar_t*)_alloca(iLength * sizeof(wchar_t));

   int iReturn = _vsnwprintf_s(pwszText, iLength, size_t(iLength) - 1, pwszFormat, va_listArgument); // generate message text

   // ## convert to utf8 if needed, text is stored in internally in utf8 format
   auto uUtf8Length = gd::utf8::size(pwszText) + 1;                              assert(uUtf8Length >= (uint32_t)iLength); // if less something is seriously wrong
   pbszReturnText.reset(new_s(uUtf8Length));
   char* pbsz = pbszReturnText.get();
   gd::utf8::convert_unicode(pwszText, pbsz, pbsz + uUtf8Length);

   return std::move(pbszReturnText);
}
#endif



// ================================================================================================
// ================================================================================= GLOBAL
// ================================================================================================


/*----------------------------------------------------------------------------- severity_get_name_g */ /**
 * return pointer to name for selected severity type name
 * \param uSeverity severity number for requested severity name
 * \return const char* pointer to name
 */
const char* severity_get_name_g(unsigned uSeverity)
{
   switch( uSeverity & static_cast<unsigned>(enumSeverityMask::eSeverityMaskNumber) )
   {
   case enumSeverityNumber::eSeverityNumberFatal:        return "FATAL";
   case enumSeverityNumber::eSeverityNumberError:        return "ERROR";
   case enumSeverityNumber::eSeverityNumberWarning:      return "WARNING";
   case enumSeverityNumber::eSeverityNumberInformation:  return "INFORMATION";
   case enumSeverityNumber::eSeverityNumberDebug:        return "DEBUG";
   case enumSeverityNumber::eSeverityNumberVerbose:      return "VERBOSE";
   default:                                              return "NONE";
   }
}

/*----------------------------------------------------------------------------- severity_get_short_name_g */ /**
 * return pointer to name for selected severity type name, short version
 * \param uSeverity severity number for requested severity name
 * \return const char* pointer to name
 */
const char* severity_get_short_name_g(unsigned uSeverity)
{
   switch( uSeverity & static_cast<unsigned>(enumSeverityMask::eSeverityMaskNumber) )
   {
   case enumSeverityNumber::eSeverityNumberFatal:        return "fatal";
   case enumSeverityNumber::eSeverityNumberError:        return "error";
   case enumSeverityNumber::eSeverityNumberWarning:      return "warn";
   case enumSeverityNumber::eSeverityNumberInformation:  return "info";
   case enumSeverityNumber::eSeverityNumberDebug:        return "debug";
   case enumSeverityNumber::eSeverityNumberVerbose:      return "verbo";
   default:                                              return "none";
   }
}


_GD_LOG_LOGGER_END

