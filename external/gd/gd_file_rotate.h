/**
 * \file gd_file_rotate.h
 * 
 * \brief Functionality used to rotate files
 * 
 * Rotating files may be used when you want to keep a certain amount of files for 
 * some time or where you do not need to produce to many files if files are generated
 * with unique names.
 * 
 *

-------------------------------------------------
*use backup history to rotate a certain amount of files, active files is store in named file*
```cpp
using namespace ::gd::file::rotate;

// ## read file history from file
std::string stringHistoryLogFileName = "C:\\..folder..\\log_history.txt";
auto vectorHistory = gd::file::rotate::backup_history::file_read_date_name_s(stringHistoryLogFileName);

// ## read max index from list of files
int iMaxIndex = backup_history::find_max_index( vectorHistory );
iMaxIndex += 1;

// ## backup file and store the generated backup name
std::string stringLogFileName = "C:\\..folder..\\log.txt";
auto [bOk, stringResult] = backup_history::file_backup_log_s( 
   stringLogFileName, "backup", iMaxIndex, eOptionIndex | eOptionExtension | eOptionCopy
);

std::string stringDateTime = gd::file::rotate::backup_history::datetime_now_s();// generate time value
vectorHistory.push_back({ stringDateTime, stringResult });                      // store in vector 

// ## sort on time, latest time is placed first and oldest last
std::sort(std::begin(vectorHistory), std::end(vectorHistory), [](const std::pair< std::string, std::string>& v1, const std::pair< std::string, std::string>& v2) {
   return v1.first > v2.first;
});

// ## keep 30 files
if( vectorHistory.size() > 30 ) { backup_history::file_delete_backup_s( vectorHistory, 30 ); }

// ## update history file
backup_history::file_write_date_name_s( stringHistoryLogFileName, vectorHistory, false );
```

 * 
 */
#pragma once

#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>



#ifndef _GD_FILE_ROTATE_BEGIN
#define _GD_FILE_ROTATE_BEGIN namespace gd { namespace file { namespace rotate {
#define _GD_FILE_ROTATE_END } } }
_GD_FILE_ROTATE_BEGIN
#else
_GD_FILE_ROTATE_BEGIN
#endif

#pragma warning( disable : 26495 )

/// tag dispatcher for making data file name friendly
struct tag_filename {};


/*
TODO:
- Read files from configuration file
- check date

*/

enum enumOption
{
   eOptionNone       = (1 << 0),    ///< no option
   eOptionIndex      = (1 << 1),    ///< index counter
   eOptionExtension  = (1 << 2),    ///< file extension
   eOptionTime       = (1 << 3),    ///< time value
   eOptionDate       = (1 << 4),    ///< date value
   eOptionDateTime   = (1 << 5),    ///< date and time value
   eOptionSize       = (1 << 6),    ///< size information
   eOptionCopy       = (1 << 7),    ///< should we copy
};

/**
 * \brief 
 *
 *
 *
 \code
 \endcode
 */
class backup_history 
{
public:
   /*-----------------------------------------*/ /**
    * \brief information about backup files
    *
    * Read information about backup files from configuration and store it in 
    * file objects.
    */
   struct file
   {
      file() {}
      file( time_t iBackupTime, const std::string_view& stringFileName ): m_iBackupTime(iBackupTime), m_stringFileName(stringFileName) {}
      // attributes
      public:
         time_t m_iBackupTime;         ///< time when file was backup was done (64 bit integer)
         std::string m_stringFileName; ///< backup file name
   };

// ## construction -------------------------------------------------------------
public:
   backup_history() {}
   explicit backup_history(unsigned uMaxHistoryCount) : m_uMaxHistoryCount(uMaxHistoryCount) {}
   backup_history( const std::string_view& stringConfigureFileName ): m_stringConfigureFileName(stringConfigureFileName) {}
   // copy
   backup_history( const backup_history& o ) { common_construct( o ); }
   backup_history( backup_history&& o ) noexcept { common_construct( o ); }
   // assign
   backup_history& operator=( const backup_history& o ) { common_construct( o ); return *this; }
   backup_history& operator=( backup_history&& o ) noexcept { common_construct( o ); return *this; }
   
	~backup_history() {}
private:
   // common copy
   void common_construct(const backup_history& o);
   void common_construct( backup_history&& o ) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   

// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   
//@}

/** \name OPERATION
*///@{
   
//@}

protected:
/** \name INTERNAL
*///@{
   
//@}

public:
/** \name DEBUG
*///@{
   
//@}


// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uMaxHistoryCount;
   std::string m_stringFolder;
   std::string m_stringConfigureFileName;
   
// ## free functions ------------------------------------------------------------
public:
   // ## file operations.  read and write values from files. we are working with dates and file names

   // reads date value and file name from each line
   static std::vector< std::pair< std::string, std::string> > file_read_date_name_s(const std::string_view& stringFileName);
   static std::vector< std::pair< std::string, std::string> > file_read_date_name_s(const std::wstring_view& stringFileName);

   // write text line or lines from vector to file
   static std::pair<bool, std::string> file_write_text_s(const std::string_view& stringFileName, const std::vector< std::string >& vectorText, bool bAppend);
   // write date and name found in vector to file
   static std::pair<bool, std::string> file_write_date_name_s(const std::string_view& stringFileName, const std::vector< std::pair< std::string, std::string> >& vectorDateName, bool bAppend);

   // backup file to temporary file, backing up file to temporary if you need to restore it, maybe something has gone wrong
   static std::string file_backup_as_temporary_s(const std::string_view& stringFileName);
   // backup file to temporary file, backing up file to temporary if you need to restore it, maybe something has gone wrongs
   static std::string file_backup_as_temporary_s(const std::wstring_view& stringFileName);

   // backup log file and generate some sort of sequence to name backed up log file
   static  std::pair<bool, std::string> file_backup_log_s(const std::string_view& stringFileName, const std::string_view& stringBackupName, unsigned uIndex) { return file_backup_log_s(stringFileName, stringBackupName, uIndex, (eOptionIndex | eOptionExtension | eOptionCopy)); }
   // backup log file and generate some sort of sequence to name backed up log file
   static std::pair<bool, std::string> file_backup_log_s( const std::string_view& stringFileName, const std::string_view& stringBackupName, unsigned uIndex, unsigned uOptions );

   // delete file
   static bool file_delete_backup_s( const std::string_view& stringFileName );
   // delete file with name in vector if more than keep count sent
   static void file_delete_backup_s( std::vector< std::pair< std::string, std::string> >& vectorBackup, unsigned uKeepCount );

   static std::pair<bool, std::string> file_stash_log_s(const std::string_view& stringHistoryFileName, const std::string_view& stringLogFileName, const std::string_view& stringBackupName, unsigned uCount);

   // ## history statistics, find information in files containing history information

   // calculate largest number among files (tries to find largest number in names)
   static int find_max_index( const std::vector< std::pair< std::string, std::string> >& vectorDateName );

   // ## time functionality, get current time in selected format
   static std::string datetime_now_s();
   static std::string date_now_s();
   static std::string time_now_s();
   static std::string time_now_s( tag_filename );
   static std::string year_now_s();

   // ## helper methods

   /// test flag in value, true if flag is set, false if not 
   static inline bool has_flag_s(unsigned uValue, unsigned uFlag) { return (uValue & uFlag) != 0; }


};


_GD_FILE_ROTATE_END