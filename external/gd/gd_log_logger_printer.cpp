/**
 * \file gd_log_logger_printer.h
 * 
 * \brief Modify logger output by selecting one or some of the "Printer" classes found here.
 * 
 * Generate log output needs one or more printers used to transform log messages
 * to some sort of readable log information. 
 * Each logger created can have one or more printers attached. Messages sent to
 * logger is spread to these attached printers.
 * 
 */

/*

|  |  |
| - | - |
|  |  |
|  |  |






*/

#include <chrono>
#ifdef _MSC_VER
#  include "io.h"
#endif
#include <clocale>
#include <fcntl.h>
#include <sys/stat.h>

#include "gd_utf8.h"

#include "gd_log_logger_printer.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #pragma clang diagnostic ignored "-Wswitch"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
   #pragma GCC diagnostic ignored "-Wswitch"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996  )
#endif

_GD_LOG_LOGGER_BEGIN

const char* color_get_code_s(enumColor eColor);
const wchar_t* color_get_wcode_s(enumColor eColor);

std::mutex& printer_get_mutex_g()
{
   static std::mutex mutexPrinter;
   return mutexPrinter;
}

// ================================================================================================
// ================================================================================ printer_console
// ================================================================================================

#ifdef _MSC_VER
/*----------------------------------------------------------------------------- print */ /**
 * print is overridden from i_print and is called when logger prints something and sends it
 * to attached printers. Here `printer_console` converts information to text and sends it to the console.
 * \param message printed message
 */
bool printer_console::print(const message& message)
{
   bool bChangeColor = false;
   std::wstring stringMessage;

   if( message.is_message_type_set() == true )
   {
      if( message.is_severity() == true )
      {
         if(get_margin_color() != 0)
         {
            stringMessage.append(L"\033[");
            stringMessage.append( color_get_wcode_s( get_margin_color()  ) );
            stringMessage.append(L"m");
         }

         if( m_uSeverityMargin == 0 )
         {
            gd::utf8::convert_utf8_to_uft16( (const uint8_t*)severity_get_name_g( message.get_severity_number() ), stringMessage );
            stringMessage += std::wstring_view{ L"  " };
         }
         else
         {
            stringMessage += L'[';
            unsigned uMarginLength = m_uSeverityMargin + (unsigned)stringMessage.length();
            gd::utf8::convert_utf8_to_uft16( (const uint8_t*)severity_get_short_name_g( message.get_severity_number() ), stringMessage );
            // pad string if needed
            if( stringMessage.length() < uMarginLength ) { stringMessage.insert(stringMessage.end(), uMarginLength - stringMessage.length(), L'.' );  }
            stringMessage += L"] ";
         }

         if( get_margin_color() != 0 )
         {
            stringMessage += std::wstring_view{ L"\033[39m" };// reset color
         }
      }

      // ## if severity has color then change color
      if(is_color(message.get_severity_number()) == true)
      {
         stringMessage.append(L"\033[");
         stringMessage.append( color_get_wcode_s( get_color( message.get_severity_number() )  ) );
         stringMessage.append(L"m");
         bChangeColor = true;
      }

      if( m_bTime == true )                                                    // time can be turned on or off globaly and is checked here
      {
         if( message.is_time() == true )
         {
            std::wstring stringTime = message::get_now_time_as_wstring_s();
            stringMessage += stringTime;
            stringMessage += std::wstring_view{ L"  " };
         }
         else if( message.is_date() == true )
         {
            std::wstring stringDate = message::get_now_date_as_wstring_s();
            stringMessage += stringDate;
            stringMessage += std::wstring_view{ L"  " };
         }
      }
   }
   
   auto stringPrintMessage = message.to_wstring(); // added log message
   if( m_uSeverityMargin == 0 ) { stringMessage += stringPrintMessage; }
   else
   {
      // ## insert margin to format log output
      if( stringPrintMessage.find( '\n' ) != std::wstring::npos )
      {
         std::wstring stringNargin( m_uSeverityMargin + 3, ' ' ); // create margin (margin size + two characters that enclose severity name and one extra space)
         std::wstring temp_;
         for( auto it = std::cbegin( stringPrintMessage ), itEnd = std::cend( stringPrintMessage ); it != itEnd; it++  )
         {  
            if( *it != L'\n' ) temp_ += *it;
            else
            {
               temp_ += *it;
               temp_ += stringNargin;
            }
         }
         stringPrintMessage = std::move( temp_ );
      }
   }

   stringMessage += stringPrintMessage;                                        // append log message to final message to print

   if( bChangeColor == true ) stringMessage += std::wstring_view{ L"\033[39m" };// reset color

   if( m_uMessageCounter > 0 ) print(std::wstring_view{ L"  " });              // print separator if there have been more messages before flush method is called

   print( stringMessage );

   return true;
}

bool printer_console::flush()
{
   if( m_uMessageCounter > 0 )                                                   // one or more messages printed?
   {
      print(std::wstring_view{ L"\n" });
   }

   m_uMessageCounter = 0;
   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Print message to console
 * @param stringMessage text printed to console
 */
void printer_console::print(const std::wstring_view& stringMessage)
{
   if( m_bConsole == true )
   {
      std::size_t uSize = stringMessage.length();
      const auto* ptext_ = stringMessage.data();

      if( uSize > 4096 )
      {
         for( std::size_t uOffset = 0; (uOffset + 4096) < uSize; uOffset += 4096 )
         {
            ::WriteConsoleW(m_hOutput, ptext_, static_cast<DWORD>(4096), NULL, NULL);
            ptext_ += 4096;
         }
      }

      uSize %= 4096;
      ::WriteConsoleW(m_hOutput, ptext_, static_cast<DWORD>(uSize), NULL, NULL);
      
#     ifdef _DEBUG
      ::OutputDebugStringW(stringMessage.data());
#     endif // _DEBUG
   }
   else
   {
      m_wostreamOutput << stringMessage << std::flush;
   }

   m_uMessageCounter++;                                                          // add message counter, number of messages before flush is called
}

#else 
/*----------------------------------------------------------------------------- print */ /**
 * print is overridden from i_print and is called when logger prints something and sends it
 * to attached printers. Here `printer_console` converts information to text and sends it to the console.
 * \param message printed message
 */
bool printer_console::print(const message& message)
{
   bool bChangeColor = false;
   std::wstring stringMessage;

   if( message.is_message_type_set() == true )
   {

      if( message.is_severity() == true )
      {
         if(get_margin_color() != 0)
         {
            stringMessage.append(L"\033[");
            stringMessage.append( color_get_wcode_s( get_margin_color()  ) );
            stringMessage.append(L"m");
         }

         if( m_uSeverityMargin == 0 )
         {
            gd::utf8::convert_utf8_to_uft16( (const uint8_t*)severity_get_name_g( message.get_severity_number() ), stringMessage );
            stringMessage += std::wstring_view{ L"  " };
         }
         else
         {
            stringMessage += L'[';
            unsigned uMarginLength = m_uSeverityMargin + stringMessage.length();
            gd::utf8::convert_utf8_to_uft16( (const uint8_t*)severity_get_short_name_g( message.get_severity_number() ), stringMessage );
            // pad string if needed
            if( stringMessage.length() < uMarginLength ) { stringMessage.insert(stringMessage.end(), uMarginLength - stringMessage.length(), L'.' );  }
            stringMessage += L"] ";
         }

         if( get_margin_color() != 0 )
         {
            stringMessage += std::wstring_view{ L"\033[39m" };// reset color
         }
      }

      // ## if severity has color then change color
      if(is_color(message.get_severity_number()) == true)
      {
         stringMessage.append(L"\033[");
         stringMessage.append( color_get_wcode_s( get_color( message.get_severity_number() )  ) );
         stringMessage.append(L"m");
         bChangeColor = true;
      }

      if( m_bTime == true )                                                    // time can be turned on or off globaly and is checked here
      {
         if( message.is_time() == true )
         {
            std::wstring stringTime = message::get_now_time_as_wstring_s();
            stringMessage += stringTime;
            stringMessage += std::wstring_view{ L"  " };
         }
         else if( message.is_date() == true )
         {
            std::wstring stringDate = message::get_now_date_as_wstring_s();
            stringMessage += stringDate;
            stringMessage += std::wstring_view{ L"  " };
         }
      }
   }

   auto stringPrintMessage = message.to_wstring(); // added log message
   if( m_uSeverityMargin == 0 ) { stringMessage += stringPrintMessage; }
   else
   {
      // ## insert margin to format log output
      if( stringPrintMessage.find( '\n' ) != std::wstring::npos )
      {
         std::wstring stringNargin( m_uSeverityMargin + 3, ' ' ); // create margin (margin size + two characters that enclose severity name and one extra space)
         std::wstring temp_;
         for( auto it = std::cbegin( stringPrintMessage ), itEnd = std::cend( stringPrintMessage ); it != itEnd; it++  )
         {  
            if( *it != L'\n' ) temp_ += *it;
            else
            {
               temp_ += *it;
               temp_ += stringNargin;
            }
         }
         stringPrintMessage = std::move( temp_ );
      }

      stringMessage += stringPrintMessage;                                     // append log message to final message to print
   }


   if( bChangeColor == true ) stringMessage += std::wstring_view{ L"\033[39m" };// reset color

   if( m_uMessageCounter > 0 ) print(std::wstring_view{ L"  " });                // print separator if there have been more messages before flush method is called

   print( stringMessage );

   return true;
}

bool printer_console::flush()
{
   if( m_uMessageCounter > 0 )                                                   // one or more messages printed?
   {
      print(std::wstring_view{ L"\n" });
   }

   m_uMessageCounter = 0;
   return true;
}

void printer_console::print(const std::wstring_view& stringMessage)
{
   std::string stringMessageAscii = gd::utf8::convert_unicode_to_ascii( stringMessage );
   auto uWriteLength = write( STDOUT_FILENO, (const void*)stringMessageAscii.c_str(), stringMessageAscii.length() );

   m_uMessageCounter++;                                                          // add message counter, number of messages before flush is called
}

#endif

// ================================================================================================
// =================================================================================== printer_file
// ================================================================================================

printer_file::printer_file( const std::string_view& stringFileName ): m_stringSplit{ L"  " }, m_stringNewLine{ L"\n" } 
{
   constexpr unsigned uBufferSize = 512;                                                           assert( stringFileName.length() < uBufferSize );
   wchar_t pwszBuffer[uBufferSize];
   size_t uLength = stringFileName.length();

   std::mbstowcs(pwszBuffer, stringFileName.data(), uLength);
   m_stringFileName.assign( pwszBuffer, uLength );
}

/*----------------------------------------------------------------------------- print */ /**
 * print is overridden from i_print and is called when logger prints something and sends it
 * to attached printers.
 * \param message printed message
 */
bool printer_file::print(const message& message)
{
   std::wstring stringMessage;

   if( is_open() == false )                                                      // check if file has been opened, if not then open file
   {
      if( is_error(eErrorOpenFile) == true ) return true;
      auto [iFileHandle, stringError] = file_open_s(m_stringFileName);
      m_iFileHandle = iFileHandle;
       
      if( is_open() == false )                                                   // still not open? then internal error
      {
         // ## Failed to open log file, generate error message for `logger`, `logger` may fetch this using `error` method
         m_uInternalError |= eErrorOpenFile;                                     // set internal error state
         m_messageError.set_severity(eSeverityError);                            // mark message as **error**
         m_messageError << "failed to create or open log file. log file name is \"" << m_stringFileName << "\""; // error message
         return false;
      }
   }

   if( message.is_message_type_set() == true )                                   // check message "if type of message" is set, then go through message settings to add fixed information
   {
      if( message.is_severity() == true )                                        // is severity set ?
      {
         gd::utf8::convert_utf8_to_uft16((const uint8_t*)severity_get_name_g(message.get_severity_number()), stringMessage);
         cover_text( stringMessage );
         stringMessage += m_stringSplit;
      }

      if( message.is_time() == true )                                            // add time ?
      {
         std::wstring stringTime = message::get_now_time_as_wstring_s();
         stringMessage += get_cover_text(stringTime);
         stringMessage += m_stringSplit;
      }
      else if( message.is_date() == true )                                       // add date ?
      {
         std::wstring stringDate = message::get_now_date_as_wstring_s();
         stringMessage += get_cover_text(stringDate);
         stringMessage += m_stringSplit;
      }
   }

   // ## write message text to file there is any text to write

   if( stringMessage.empty() == false )
   {
      auto [bOk, stringError] = file_write_s(m_iFileHandle, stringMessage, gd::utf8::tag_utf8{});
      if( bOk == false )
      {
         // TODO: manage error, get information from string and 
         return false;
      }
   }


   const char* pbszMessage = message.get_text();

   auto [ bOk, stringError] = file_write_s(m_iFileHandle, pbszMessage);
   if( bOk == false )
   {
      // TODO: manage error, get information from string and 
      return false;
   }

   return true;
}

bool printer_file::flush()
{
   char pbsz[3];
   if( is_open() == true && m_stringNewLine.empty() == false )
   {
      if( m_stringNewLine.length() < 3 )
      {
         pbsz[0] = (char)m_stringNewLine[0];
         pbsz[1] = (char)m_stringNewLine[1];
         pbsz[2] = 0;
         file_write_s(m_iFileHandle, pbsz);
      }
   }

   return true;
}

unsigned printer_file::error(message& message)
{
   if( m_messageError.empty() == false )
   {
      message = std::move(m_messageError);
      return 1;
   }
   return 0;
}



/*----------------------------------------------------------------------------- cover_text */ /**
 * cover or wrap text within characters to make it nicer to view
 * \param stringText text to wrap within start and end character
 */
void printer_file::cover_text(std::wstring& stringText) const
{
   stringText = message::wrap_s(m_wchBeginWrap, stringText, m_wchEndWrap);
}

/*----------------------------------------------------------------------------- get_cover_text */ /**
 * cover or wrap text within characters to make it nicer to view
 * \param stringText text to wrap within start and end character
 * \return std::wstring wrapped text
 */
std::wstring printer_file::get_cover_text(const std::wstring_view& stringText) const
{
   std::wstring stringWrappedText = message::wrap_s(m_wchBeginWrap, stringText, m_wchEndWrap);
   return stringWrappedText;
}


/*----------------------------------------------------------------------------- open_s */ /**
 * open selected log file log information is written to
 * \param stringFileName name of log file to open
 * \return std::pair<int, std::string> if ok (first is valid filehandle), then no string information. otherwise return error imformation in string
 */
std::pair<int, std::string> printer_file::file_open_s(const std::wstring_view& stringFileName)
{                                                                                                  assert( stringFileName.length() > 3 ); // realistic filename
   // TODO: lock this (thread safety)

   int iFileHandle = 0;
#  if defined(_WIN32)
   ::_wsopen_s(&iFileHandle, stringFileName.data(), _O_CREAT | _O_WRONLY | _O_BINARY | _O_NOINHERIT, _SH_DENYWR, _S_IREAD | _S_IWRITE); assert( iFileHandle >= 0 );
   if( iFileHandle >= 0 ) _lseek( iFileHandle, 0, SEEK_END );
#  else
   std::string stringFileName_ = gd::utf8::convert_unicode_to_ascii( stringFileName );
   iFileHandle = open(stringFileName_.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); assert( iFileHandle >= 0 );
   if( iFileHandle >= 0 ) lseek( iFileHandle, 0, SEEK_END );
#  endif

   if( iFileHandle < 0 )
   {                                                                             // assert( false );
      std::string stringError("FILE OPEN ERROR: ");
      stringError += std::strerror(errno);
      return { iFileHandle, stringError };
   }

   return { iFileHandle, std::string() };
}

std::pair<bool, std::string> printer_file::file_write_s(int iFileHandle, const std::string_view& stringText)
{
   // TODO: lock this (thread safety)
#ifdef _MSC_VER
   int iWriteCount = ::_write( iFileHandle, (const void*)stringText.data(), (unsigned int)stringText.length() );
#else
   int iWriteCount = write( iFileHandle, (const void*)stringText.data(), (unsigned int)stringText.length() );
#endif   
   if( iWriteCount != (int)stringText.length() )
   {                                                                                               assert( false );
      std::string stringError("FILE WRITE ERROR: ");

      stringError += std::strerror(errno);
      return { false, stringError };
   }

   return { true, std::string() };
}

/*----------------------------------------------------------------------------- file_write_s */ /**
 * write unicode text to file but before writing it will convert unicode text to utf8
 * \param iFileHandle file handle to file written to
 * \param stringText unicode text to write
 * \return std::pair<bool, std::string> true if ok, otherwise false and error information
 */
std::pair<bool, std::string> printer_file::file_write_s(int iFileHandle, const std::wstring_view& stringText, gd::utf8::tag_utf8)
{                                                                                                  assert( iFileHandle >= 0 );
   enum { eBufferSize = 100 };
   char pBuffer[eBufferSize];
   std::unique_ptr<char> pHeap;
   char* pbszUtf8Text = pBuffer;

   // ## convert unicode text to utf8
   auto uUtf8Size = gd::utf8::size(stringText.data());                           // how big buffer is needed to store unicode as utf8 text
   uUtf8Size++;                                                                  // make room for zero terminator
   if( uUtf8Size > static_cast<decltype(uUtf8Size)>(eBufferSize) )
   {  // cant fit in local buffer, allocate on heap
      pHeap.reset(new char[uUtf8Size]);
      pbszUtf8Text = pHeap.get();
   }
#ifndef NDEBUG
   memset( pbszUtf8Text, '0', uUtf8Size - 1 );
   pbszUtf8Text[uUtf8Size] = '\0';
   auto pwszText_d = stringText.data();
#endif   

   gd::utf8::convert_unicode(stringText.data(), pbszUtf8Text, pbszUtf8Text + uUtf8Size );

   return file_write_s( iFileHandle, pbszUtf8Text );
}

void printer_file::file_close_s(int iFileHandle)
{
#ifdef _MSC_VER   
   ::_close( iFileHandle );
#else
   close( iFileHandle );
#endif   
}




/** ---------------------------------------------------------------------------
 * @brief get console color code for color
 * @param eColor color code is returned for
 * @return 
 */
const char* color_get_code_s(enumColor eColor)
{
   switch(eColor)
   {
   case eColorBlack:          return "30";
   case eColorRed:            return "31";
   case eColorGreen:          return "32";
   case eColorYellow:         return "33";
   case eColorBlue:           return "34";
   case eColorMagneta:        return "35";
   case eColorCyan:           return "36";
   case eColorWhite:          return "37";
   case eColorBrightBlack:    return "90";
   case eColorBrightRed:      return "91";
   case eColorBrightGreen:    return "92";
   case eColorBrightYellow:   return "93";
   case eColorBrightBlue:     return "94";
   case eColorBrightMagneta:  return "95";
   case eColorBrightCyan:     return "96";
   case eColorBrightWhite:    return "97";
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
* @brief get console color code for color
* @param eColor color code is returned for
* @return 
*/
const wchar_t* color_get_wcode_s(enumColor eColor)
{
   switch(eColor)
   {
   case eColorBlack:          return L"30";
   case eColorRed:            return L"31";
   case eColorGreen:          return L"32";
   case eColorYellow:         return L"33";
   case eColorBlue:           return L"34";
   case eColorMagneta:        return L"35";
   case eColorCyan:           return L"36";
   case eColorWhite:          return L"37";
   case eColorBrightBlack:    return L"90";
   case eColorBrightRed:      return L"91";
   case eColorBrightGreen:    return L"92";
   case eColorBrightYellow:   return L"93";
   case eColorBrightBlue:     return L"94";
   case eColorBrightMagneta:  return L"95";
   case eColorBrightCyan:     return L"96";
   case eColorBrightWhite:    return L"97";
   }

   return nullptr;
}



_GD_LOG_LOGGER_END
