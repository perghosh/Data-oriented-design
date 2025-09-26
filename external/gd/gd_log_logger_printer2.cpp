#include <chrono>
#ifdef _MSC_VER
#  include "io.h"
#endif
#include <clocale>
//#include <format>
#include <fcntl.h>
#include <sys/stat.h>

#include <inttypes.h>

#include "gd_utf8.h"

#include "gd_log_logger_printer.h"
#include "gd_log_logger_printer2.h"
#include "gd_parse.h"
#include "gd_table_io.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #pragma clang diagnostic ignored "-Wswitch"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
   #pragma GCC diagnostic ignored "-Wswitch"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996  )
#endif

#if GD_COMPILER_HAS_CPP20_SUPPORT

_GD_LOG_LOGGER_BEGIN


// ================================================================================================
// =================================================================================== printer_file
// ================================================================================================

printer_csvfile::~printer_csvfile() 
{ 
   if( m_iFileHandle >= 0 )
   {
      dump();
      file_close_s(m_iFileHandle); 
   }
}

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
   using namespace gd::table;

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

      // ## create time values
      m_timepointStart = std::chrono::steady_clock::now();
      m_timepointCurrent = m_timepointStart;

      // ## Print column headers to csv file
      std::string stringCsv;
      gd::table::to_string( m_tableCSV, 0, 0, gd::argument::arguments(), nullptr, stringCsv, tag_io_header{}, tag_io_csv{});
      stringCsv += "\n";

      file_write_s( m_iFileHandle, stringCsv );                                // write text to file
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

   auto uRow = m_tableCSV.get_row_count();
   m_tableCSV.row_add();
   m_tableCSV.row_set_null( uRow );

   // ## write message text to table
   std::string stringMessage = message.to_string();
   if( is_extra_columns() == true )
   {
      stringMessage = set_extra_columns( uRow, stringMessage );
   }

   m_tableCSV.cell_set( uRow, 0, severity_get_name_g( message.get_severity() ) );
   m_tableCSV.cell_set( uRow, 1, stringMessage );
   m_tableCSV.cell_set( uRow, 2, m_uCounter );

   if( m_uFlags & eFlagBenchmark )
   {
      auto timepoint_ = std::chrono::steady_clock::now();

      // Calculate the duration from start
      int64_t iDuration = std::chrono::duration_cast<std::chrono::microseconds>(timepoint_ - m_timepointStart).count();
      m_tableCSV.cell_set( uRow, 3, iDuration );
      // Calculate the duration from previous message
      iDuration = std::chrono::duration_cast<std::chrono::microseconds>(timepoint_ - m_timepointCurrent).count();
      m_tableCSV.cell_set( uRow, 4, iDuration );
      m_timepointCurrent = timepoint_;

      if( m_uFlags & eFlagBenchmarkText )
      {
         auto duration_ = m_timepointCurrent - m_timepointStart;
         //auto seconds_ = std::chrono::duration_cast<std::chrono::seconds>(duration_).count();
         //auto milliseconds_ = std::chrono::duration_cast<std::chrono::milliseconds>(duration_).count();
         auto microseconds_ = std::chrono::duration_cast<std::chrono::microseconds>(duration_).count();
         int64_t seconds_ = microseconds_ / 1'000'000; 
         microseconds_ %= 1'000'000;
         int64_t milliseconds_ = microseconds_ / 1'000; 
         microseconds_ %= 1'000;

         //std::string stringDuration = std::format("{:02d}s:{:02d}ms:{:03d}us", seconds_, milliseconds_, microseconds_);
         char piBuffer[30];
         //sprintf( piBuffer, "%02ld:%02ld:%03ld", seconds_, milliseconds_, microseconds_ );
         // Use PRId64 for portable int64_t formatting
         sprintf(piBuffer, "%02" PRId64 ":%02" PRId64 ":%03" PRId64, seconds_, milliseconds_, microseconds_);
         std::string stringDuration = piBuffer;
         m_tableCSV.cell_set( uRow, 5, stringDuration );

         //std::string formatted_time = std::format("{:02d}:{:02d}:{:02d}.{:03d}{:03d}", seconds / 3600, (seconds % 3600) / 60, seconds % 60, milliseconds, microseconds);
      }


   }




   m_uCounter++;

   if( m_tableCSV.get_row_count() > m_uMaxRowCount )
   {
      dump();
   }

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

/// flush all data from table to file and clear table
bool printer_csvfile::flush()
{
   if( is_open() == true  )
   {
      dump();
   }

   return true;
}

/// write error information to message that is passed as argument
unsigned printer_csvfile::error(message& message)
{
   if( m_messageError.empty() == false )
   {
      message = std::move(m_messageError);
      return 1;
   }
   return 0;
}

/// create internal table to store log information
void printer_csvfile::create( std::function< void( gd::table::table& table )> callback_ )
{
   m_uFlags |= create_table_s(m_tableCSV, callback_);
}

/** ---------------------------------------------------------------------------
 * @brief set extra column values from message text
 * message should be generated as a url querystring to set values in extra columns
 * @param stringMessage text for message
 * @param uRow index for row where values are set
 * @return std::string main message text
 */
std::string printer_csvfile::set_extra_columns( uint64_t uRow, const std::string_view& stringMessage)
{
   std::string stringCleanedMessage;

   // ## find `?` and then parse  named values
   auto position_ = stringMessage.find('?');
   if( position_ != std::string_view::npos )
   {
      stringCleanedMessage = stringMessage.substr( 0,  position_ );
      position_++;
      // ### parse named values
      std::string_view stringQueryString( stringMessage.data() + position_  );
      std::vector<std::pair<std::string,std::string>> vectorValue;
      gd::parse::read_line_g( stringQueryString, vectorValue, gd::parse::querystring() );

      for( auto itCell : vectorValue )
      {
         auto uColumn = m_tableCSV.column_find_index( itCell.first );
         if( uColumn != (unsigned)-1 )
         {
            m_tableCSV.cell_set(uRow, uColumn, itCell.second, gd::table::tag_convert{});
         }
      }
   }
   else
   {
      stringCleanedMessage = stringMessage;                                    // no extra columns
   }

   return stringCleanedMessage;
}



/// dump table data to file and clear rows
void printer_csvfile::dump()
{
   using namespace gd::table;
   std::string stringCsv;
   gd::table::to_string( m_tableCSV, 0, m_tableCSV.get_row_count(), gd::argument::arguments(), nullptr, stringCsv, tag_io_csv{});

   file_write_s( m_iFileHandle, stringCsv );                                   // write text to file
   m_tableCSV.row_clear();                                                     // clear rows
}


/** ---------------------------------------------------------------------------
 * @brief creates internal table to store log information
 * Table will be created automatically when first log message is written to file if not created before
 * @param table_ reference to table object
 * @param callback_ callback method to customize table
 */
unsigned printer_csvfile::create_table_s( gd::table::table& table_, std::function< void( gd::table::table& table )> callback_ )
{                                                                                                  assert( table_.empty() == true );
   unsigned uFlags = 0;
   table_.set_flags( gd::table::table::eTableFlagNull32 );
   //m_tableField.set_reserved_row_count( 10 );
   table_.column_prepare();                                                    // creates columns object
   table_.column_add( { 
      {"string", 20, "severity"}, 
      {"string", 200, "description"}, 
      {"uint64", 0, "counter" }, 
      {"int64", 0, "from start" }, 
      {"int64", 0, "from previous" }, 
      {"string", 30, "time text" } }, 
      gd::table::tag_type_name{}
   );

   auto uCount = table_.get_column_count();
   if( callback_ ) { callback_(table_); }                                      // customization for table if more columns are needed

   // if more columns are added, then set flag to indicate that there are extra columns
   if( table_.get_column_count() > uCount ) { uFlags |= eFlagExtraColumns; }

   table_.prepare();

   return uFlags;
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

#endif // GD_COMPILER_HAS_CPP20_SUPPORT