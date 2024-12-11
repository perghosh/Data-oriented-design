#include <chrono>
#ifdef _MSC_VER
#  include "io.h"
#endif
#include <clocale>
#include <fcntl.h>
#include <sys/stat.h>

#include "gd_utf8.h"

#include "gd_log_logger_printer.h"

#include "gd_log_logger_printer2.h"

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


// ================================================================================================
// =================================================================================== printer_file
// ================================================================================================

printer_csvfile::printer_csvfile( const std::string_view& stringFileName ) 
{
   constexpr unsigned uBufferSize = 512;                                                           assert( stringFileName.length() < uBufferSize );
   wchar_t pwszBuffer[uBufferSize];
   size_t uLength = stringFileName.length();

   std::mbstowcs(pwszBuffer, stringFileName.data(), uLength);
   m_stringFileName.assign( pwszBuffer, uLength );
}

printer_csvfile::printer_csvfile(const std::string_view& stringName, const std::string_view& stringFileName):
   i_printer( stringName )
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
bool printer_csvfile::print(const message& message)
{
   if( is_open() == false )                                                      // check if file has been opened, if not then open file
   {
      if( is_error(eErrorOpenFile) == true ) return true;
      auto [iFileHandle, stringError] = file_open_s(m_stringFileName);
      m_iFileHandle = iFileHandle;

      if( m_tableCSV.empty() == true ) { create_table_s( m_tableCSV ); }
       
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
      /*
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
      */
   }

   // ## write message text to table
   std::string stringMessage = message.to_string();
   auto uRow = m_tableCSV.get_row_count();
   m_tableCSV.row_add();
   m_tableCSV.cell_set( uRow, 0, severity_get_name_g( message.get_severity() ) );
   m_tableCSV.cell_set( uRow, 1, stringMessage );
   m_tableCSV.cell_set( uRow, 2, m_uCounter );
   m_tableCSV.cell_set( uRow, 3, 0.0 );
   m_uCounter++;

#ifndef NDEBUG
   auto stringTabke_d = gd::table::debug::print( m_tableCSV );
#endif

   /*
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
   */

   return true;
}

bool printer_csvfile::flush()
{
   //char pbsz[3];
   if( is_open() == true  )
   {
      /*
      if( m_stringNewLine.length() < 3 )
      {
         pbsz[0] = (char)m_stringNewLine[0];
         pbsz[1] = (char)m_stringNewLine[1];
         pbsz[2] = 0;
         file_write_s(m_iFileHandle, pbsz);
      }
      */
   }

   return true;
}

unsigned printer_csvfile::error(message& message)
{
   if( m_messageError.empty() == false )
   {
      message = std::move(m_messageError);
      return 1;
   }
   return 0;
}


/// Creates table to store log information
void printer_csvfile::create_table_s( gd::table::table& table_ )
{                                                                                                  assert( table_.empty() == true );
   table_.set_flags( gd::table::table::eTableFlagNull32 );
   //m_tableField.set_reserved_row_count( 10 );
   table_.column_prepare();
   table_.column_add( { 
      {"string", 20, "severity"}, 
      {"string", 200, "description"}, 
      {"uint64", 0, "counter" }, 
      {"double", 0, "time" } }, 
      gd::table::tag_type_name{}
   );
   table_.prepare();
}


/*----------------------------------------------------------------------------- file_open_s */ /**
 * open selected log file log information is written to
 * \param stringFileName name of log file to open
 * \return std::pair<int, std::string> if ok (first is valid filehandle), then no string information. otherwise return error imformation in string
 */
std::pair<int, std::string> printer_csvfile::file_open_s(const std::wstring_view& stringFileName)
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

std::pair<bool, std::string> printer_csvfile::file_write_s(int iFileHandle, const std::string_view& stringText)
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
std::pair<bool, std::string> printer_csvfile::file_write_s(int iFileHandle, const std::wstring_view& stringText, gd::utf8::tag_utf8)
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

void printer_csvfile::file_close_s(int iFileHandle)
{
#ifdef _MSC_VER   
   ::_close( iFileHandle );
#else
   close( iFileHandle );
#endif   
}

_GD_LOG_LOGGER_END