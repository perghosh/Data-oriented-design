#pragma once

#include <array>
#include <mutex>
#include <iostream>
#ifdef _MSC_VER
#  include <corecrt_io.h>
#else
#  include <unistd.h>
#endif
#include <string_view>

#ifdef _WIN32
#include "windows.h"
#endif

#include "gd_log_logger.h"
#include "gd_table_table.h"

#ifndef _GD_LOG_LOGGER_BEGIN

#  define _GD_LOG_LOGGER_BEGIN namespace gd { log {
#  define _GD_LOG_LOGGER_END } }

#endif


_GD_LOG_LOGGER_BEGIN

// ================================================================================================
// ================================================================================ printer_csvfile
// ================================================================================================

/**
 * \brief prints log information to specified file in csv format
 *
 * file operations use  ::_wsopen_s , ::_write to write to file.
 *
 \code
 \endcode
 */
class printer_csvfile : public i_printer
{
// ## constants ----------------------------------------------------------------
private:
   enum unumFlag { eFlagBenchmark = 0x0001, eFlagBenchmarkText = 0x0002 };
   enum enumError { 
      eErrorOpenFile = 0x0000'0001,    // internal error flag/bit if file wasn't opened
   };

// ## construction -------------------------------------------------------------
public:
   printer_csvfile() {}
   printer_csvfile(const std::string_view& stringFileName);
   printer_csvfile(const std::string_view& stringName, const std::string_view& stringFileName); 
   printer_csvfile(const std::wstring_view& stringFileName) : m_stringFileName(stringFileName) {}
   printer_csvfile(unsigned uSeverity, const std::wstring_view& stringFileName) : i_printer(uSeverity), m_stringFileName(stringFileName) {}
   // copy
   printer_csvfile( const printer_csvfile& o ) { common_construct( o ); }
   printer_csvfile( printer_csvfile&& o ) noexcept { common_construct( o ); }
   // assign
   printer_csvfile& operator=( const printer_csvfile& o ) { common_construct( o ); return *this; }
   printer_csvfile& operator=( printer_csvfile&& o ) noexcept { common_construct( o ); return *this; }
   
   ~printer_csvfile();
protected:
   // common copy
   void common_construct(const printer_csvfile& o) { 
      i_printer::common_construct(o); m_uInternalError = o.m_uInternalError; m_stringFileName = o.m_stringFileName; m_iFileHandle = o.m_iFileHandle; m_messageError = o.m_messageError;
   }

   void common_construct( printer_csvfile&& o ) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## override -----------------------------------------------------------------
public:
   virtual bool print(const message& message);
   virtual bool flush();
   virtual unsigned error( message& message );


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   void set_flags( unsigned uSet, unsigned uClear ) { m_uFlags |= uSet; m_uFlags &= ~uClear;  }
   void set_flags( const std::pair<unsigned,unsigned>& pairFlags ) { set_flags( pairFlags.first, pairFlags.second ); }
   const std::wstring& get_filename() const { return m_stringFileName; }
   void set_maxrowcount( unsigned uCount ) { m_uMaxRowCount = uCount; }
//@}

/** \name OPERATION
*///@{
   /// checks if valid file handle, if handle is valid (above 0) then the file is open
   bool is_open() const { return m_iFileHandle >= 0; }

   /// dump table data to file and clear rows
   void dump();

   
//@}

protected:
/** \name INTERNAL
*///@{
   /// check if internal error
   /// \param uErrorCode code to check, usually a bit that is tested
   bool is_error(unsigned uErrorCode) const { return ((uErrorCode & m_uInternalError) != 0); }
//@}


// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uFlags = 0;           ///< flags marking how printer should behave
   unsigned m_uInternalError = 0;   ///< internal error states
   int m_iFileHandle = -1;          ///< used as file handle to log file that is written to
   uint64_t m_uCounter = 0;         ///< counter to get some sort of feeling where in the log sequence information is
   unsigned m_uMaxRowCount = 10;    ///< number of rows before flush rows to file
   std::wstring m_stringFileName;   ///< file log information is written to
   gd::log::message m_messageError; ///< temporary storage for internal error information
   gd::table::table m_tableCSV;     ///< table storing log information that is written to file

   std::chrono::steady_clock::time_point m_timepointStart;
   std::chrono::steady_clock::time_point m_timepointCurrent;

   //auto start = std::chrono::high_resolution_clock::now();
   
// ## free functions ------------------------------------------------------------
public:
   // ## flags logic
   static consteval std::pair<unsigned,unsigned> flags_s( std::string_view stringFlag );

   // ## Internal table operations
   void create_table_s( gd::table::table& table_ );

   // ## File operations (open, write and close)
   static std::pair<int, std::string> file_open_s(const std::wstring_view& stringFileName);
   static std::pair<bool, std::string> file_write_s(int iFileHandle, const std::string_view& stringText);
   static std::pair<bool, std::string> file_write_s(int iFileHandle, const std::wstring_view& stringText, gd::utf8::tag_utf8 );
   static void file_close_s(int iFileHandle);
};

/// generate flags for set and clear interal data
consteval std::pair<unsigned,unsigned> printer_csvfile::flags_s( std::string_view stringFlag )
{
   unsigned uSet = 0;
   unsigned uClear = 0;
   if( stringFlag.find( "+benchmark" ) != std::string_view::npos ) uSet |= eFlagBenchmark;
   if( stringFlag.find( "-benchmark" ) != std::string_view::npos ) uClear |= eFlagBenchmark;

   if( stringFlag.find( "+benchmark-text" ) != std::string_view::npos ) uSet |= (eFlagBenchmark|eFlagBenchmarkText);
   if( stringFlag.find( "-benchmark-text" ) != std::string_view::npos ) uClear |= eFlagBenchmarkText;

   return std::pair<unsigned,unsigned>( uSet, uClear );
}




_GD_LOG_LOGGER_END
