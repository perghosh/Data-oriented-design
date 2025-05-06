#include <stdlib.h>
#include <algorithm>
#include <fstream>
#include <filesystem>


#include "gd_utf8.h"

#include "gd_file_rotate.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wswitch"
   #pragma clang diagnostic ignored "-Wformat"
   #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wswitch"
   #pragma GCC diagnostic ignored "-Wformat"
   #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#elif defined( _MSC_VER )
   #pragma warning( disable : 4996 26812 )
#endif


_GD_FILE_ROTATE_BEGIN


void backup_history::common_construct(const backup_history& o)
{
   m_uMaxHistoryCount = o.m_uMaxHistoryCount;
   m_stringConfigureFileName = o.m_stringConfigureFileName;
}

/*----------------------------------------------------------------------------- file_read_date_name_s */ /**
 * Read list of files from file history.
 * Format for file history read is
 * 
 * date format read, same as javascript `toISOString`
 * sample `2011-10-05T14:48:00.000Z` [link](https://tc39.es/ecma262/multipage/numbers-and-dates.html#sec-date-time-string-format)
- `YYYY` : 4-digit year.
- `MM` : 2-digit month (where January is 01 and December is 12)
- `DD` : 2-digit date (0 to 31)
- `-` : Date delimiters.
- `T` : Indicates the start of time.
- `HH` : 24-digit hour (0 to 23)
- `mm` : Minutes (0 to 59)
- `ss` : Seconds (0 to 59) 
- `Z`: If Z is present, date will be set to UTC. If Z is not present, it�ll be Local Time. (This only applies if time is provided.)

*DATETIME FILENAME*
 ```
2011-10-05T14:48:00Z C:\backup\filename_01.log
2011-10-06T18:25:00Z C:\backup\filename_02.log
2011-10-06T20:11:00Z C:\backup\filename_03.log
2011-10-07T04:04:00Z C:\backup\filename_04.log
 ```
 * \param stringFileName file name containing history information
 * \return std::vector< std::pair< std::string, std::string> > vector having dates and filenames, like history
 */
std::vector<std::pair< std::string, std::string>> backup_history::file_read_date_name_s(const std::string_view& stringFileName)
{
   // vector where time value as string and file name is places from each read line
   std::vector<std::pair<std::string, std::string>> vectorDateAndName;

   std::fstream fstreamDateName(stringFileName.data(), std::ios::in);            // open history file

   if( (bool)fstreamDateName.is_open() == true )
   {
      std::string stringTime;    // read time value
      std::string stringFile;    // read file name
      std::string stringLine;    // read line from file
      while( std::getline(fstreamDateName, stringLine) )
      {
         if( stringLine.empty() == false && std::isdigit(stringLine[0]) != 0 )
         {
            // ## try to read time and file name from line
            const char* pbsz;
            const char* pbszLine = stringLine.c_str();                           // ->2011-10-05T14:48:00.000Z C:\backup\filename_01.log
            if( *pbszLine > '9' || *pbszLine < '0' ) continue;                   // first character in line needs to be a number
            pbsz = gd::utf8::move::next_space(pbszLine);                         // -> C:\backup\filename_01.log
            if( pbsz == pbszLine ) continue;                                     

            stringTime.assign(pbszLine, pbsz);
            pbszLine = pbsz;
            pbsz = gd::utf8::move::next_non_space(pbszLine);                     // ->C:\backup\filename_01.log
            if( pbsz == pbszLine ) continue;
            
            pbszLine = pbsz;
            stringFile.assign(pbszLine, stringLine.c_str() + stringLine.length());

            vectorDateAndName.push_back( {stringTime, stringFile} );             // add time and file name to vector (pair value)
         }
      }
   }

   return vectorDateAndName;
}

std::vector<std::pair< std::string, std::string>> backup_history::file_read_date_name_s(const std::wstring_view& stringFileName)
{
   return file_read_date_name_s(gd::utf8::convert_unicode_to_ascii(stringFileName));
}


/*----------------------------------------------------------------------------- file_write_text_s */ /**
 * Write text to file, text written is stored in vector and all strings in vector is
 * written to file where newline is added to each string written to file
 * \param stringFileName name of file written to
 * \param vectorText string that will be written to file
 * \param bAppend if true text is appended
 * \return std::pair<bool, std::string>
 */
std::pair<bool, std::string> backup_history::file_write_text_s(const std::string_view& stringFileName, const std::vector<std::string>& vectorText, bool bAppend)
{
   enum { eBufferErrorSize = 100 };
   unsigned uOpenMode = std::ios::out;                                           // how to open file (in what mode, write and maybe append)
   if( bAppend == true ) uOpenMode |= std::ios::app;

   std::fstream fstreamText(stringFileName.data(), (std::ios_base::openmode)uOpenMode);


   if( (bool)fstreamText.is_open() == true )
   {
      for( auto it : vectorText )
      {
         fstreamText << it;
         fstreamText << "\n";
      }

      fstreamText.close();
   }
   else
   {
      std::string stringError;
      stringError = strerror(errno);
      return {false, stringError };
   }


   return {true, ""};
}

/*----------------------------------------------------------------------------- file_write_date_name_s */ /**
 * write date and name information from vector into file
 * \param stringFileName name of file to write to
 * \param vectorDateName date and name values to write to file
 * \param bAppend if true then append to file
 * \return std::pair<bool, std::string> true if ok, false and error information if error
 */
std::pair<bool, std::string> backup_history::file_write_date_name_s(const std::string_view& stringFileName, const std::vector<std::pair<std::string, std::string>>& vectorDateName, bool bAppend )
{
   enum { eBufferErrorSize = 100 };
   unsigned uOpenMode = std::ios::out;                                           // how to open file (in what mode, write and maybe append)
   if( bAppend == true ) uOpenMode |= std::ios::app;

   std::fstream fstreamDateName(stringFileName.data(), (std::ios_base::openmode)uOpenMode);

   if( (bool)fstreamDateName.is_open() == true )
   {
      for( auto it : vectorDateName )
      {
         fstreamDateName << std::get<0>(it);
         fstreamDateName << "  ";
         fstreamDateName << std::get<1>(it);
         fstreamDateName << "\n";
      }

      fstreamDateName.close();
   }
   else
   {
      std::string stringError = strerror(errno);
      return { false, stringError };
   }

   return { true, "" };
}



/*----------------------------------------------------------------------------- file_backup_as_temporary */ /**
 * copy file to temporary file as backup.
 * remember that this file is only temporary and should not been seen as permanent backup.
 * just functionality if you need to modify a file and need to revert if file modification
 * wasn't successful.
 * 
 * \param stringFileName file to backup
 * \return std::string name of backup file, keep for later if restore is needed or to delete if everything went well
 */
std::string backup_history::file_backup_as_temporary_s(const std::string_view& stringFileName)
{
#if defined(_MSC_VER)
   char pbszBuffer[L_tmpnam];
   std::tmpnam(pbszBuffer);                     // generate temporary file name
   const char* pbszTemporaryFileName = pbszBuffer;
   if( pbszTemporaryFileName != nullptr )
   {
      auto bOk = true;
      // TODO: remove above and restore this auto bOk = std::filesystem::copy_file(stringFileName, pbszTemporaryFileName);// create clone as generated temp file
      return bOk ? pbszTemporaryFileName : "";                                   //
   }
#else
#endif
                                                                               assert( false );
   return "";
}

/*----------------------------------------------------------------------------- file_backup_as_temporary_s */ /**
 * clone file to temporary file as backup.
 * \param stringFileName unicode file name copied to as temporary file
 * \return std::string name for clone
 */
std::string backup_history::file_backup_as_temporary_s(const std::wstring_view& stringFileName)
{
   std::string stingBackupName;
   auto uLength = stringFileName.length() + 1;
   stingBackupName.resize(stringFileName.length() + 1);
   auto [pwszUnicodeEnd, pbszAsciiEnd] = gd::utf8::convert_unicode_to_ascii(stringFileName.data(), stingBackupName.data(), stingBackupName.data() + uLength);
   stingBackupName.resize( pbszAsciiEnd - stingBackupName.c_str() );
   return file_backup_as_temporary_s(stingBackupName);
}

/*----------------------------------------------------------------------------- file_delete_backup */ /**
 * Remove backup file (or any other file)
 * \param stringFileName name of file to be removed
 * \return bool true if file was removed, false if not
 */
bool backup_history::file_delete_backup_s(const std::string_view& stringFileName)
{
   int iResult = std::remove( stringFileName.data() );
   if( iResult == 0 ) return true;
   return false;
}

/*----------------------------------------------------------------------------- file_delete_backup_s */ /**
 * Delete files and items in vector if over keep count
 * \param vectorBackup list of files
 * \param uKeepCount how many to keep
 */
void backup_history::file_delete_backup_s(std::vector< std::pair< std::string, std::string> >& vectorBackup, unsigned uKeepCount)
{
   if( vectorBackup.size() <= uKeepCount ) return;                               // if less items compare dot number of items to keep then do not delete any

   for( auto it = std::begin(vectorBackup) + uKeepCount, itEnd = std::end(vectorBackup); it != itEnd; it++ )
   {
      std::remove( it->second.c_str() );
   }

   vectorBackup.erase( std::begin(vectorBackup) + uKeepCount, std::end(vectorBackup) ); // erase item from vector
}

/*----------------------------------------------------------------------------- find_max_index */ /**
 * find max index for file names in vector, check the second string value in pair for index in file name
 * \param vectorPairText vector of pair strings where name is checked for index
 * \return int integer value with largest index value read from file name
 */
int backup_history::find_max_index(const std::vector< std::pair< std::string, std::string> >& vectorPairText)
{
   auto _get_index = [](const std::string& stringText) -> int {
      auto uPosition = stringText.rfind('.');
      if( uPosition != std::string::npos )
      {
         uPosition--;                                                            // move to character before "."
         auto pbszBegin = stringText.c_str();
         auto pbszDigit = pbszBegin + uPosition + 1;                             // pointer to digit
         auto pbszDigitEnd = pbszDigit;
         pbszDigit--;
         while( pbszDigit != pbszBegin && std::isdigit(*pbszDigit) != 0 ) { pbszDigit--; }
         pbszDigit++;

         auto iIndex = std::atoi(pbszDigit);
         return iIndex;
      }
      return -1;
   };


   int iMax = 0;
   for( auto it : vectorPairText )
   {
      const auto& stringFile = it.second;
      int iIndex = _get_index( stringFile );
      if( iIndex > iMax ) iMax = iIndex;
   }
   return iMax;
}

/*----------------------------------------------------------------------------- file_backup_log_s */ /**
 * Backup log file, generates a new name for log and copies the main log file to the new file with generated name
 * ## steps
 * - test if log file to backup exists
 * - test if "backup to name" is not a full path (has folders)
 *   - no folders then copy path from file to backup
 * - if directory is extracted then set it to file to backup to, if no directory then assign to same directory as file to backup
 * - fix extension, if extension is found in backup name then use that, default is ".txt"
 * 
 * \param stringLogFileName log file that is backed up
 * \param stringBackupName name for backup log file, this name is modified with index and time before copy is done
 * \param uBackupIndex new index for backup log
 * \param uOptions flags on how to generate backup file name
 * \return std::pair<bool, std::string> true and name if ok, false and error information if error
 */
std::pair<bool, std::string> backup_history::file_backup_log_s(const std::string_view& stringLogFileName, const std::string_view& stringBackupName, unsigned uBackupIndex, unsigned uOptions )
{                                                                                assert( uBackupIndex < 1000000 ); // realistic !!
   enum { eMaxExtensionLength = 12, eMaxIndexTextLength = 7 };

   // ## check if file exists
   bool bExist = std::filesystem::exists(stringLogFileName);                        
   if( bExist == false )                                                         // check if file to backup exists, if not then return (nothing to backup)
   {
      std::string stringError{ "file not found: log file with name \"" };
      stringError += stringLogFileName;
      stringError += "\" is missing";
      return { false, stringError };
   }

   // ## build new file name that specified log file is copied to

   std::string stringBackupToName; // generated backup file name
   std::string stringDirectory; // directory to file that should be backed up

   // ### check if backup file only is a name (don't has the full path)
   if( gd::utf8::is_found(stringBackupName.find_first_of("\\/")) == false )      // if no slash character in file backup file, the extract path from log file
   {  // no path in backup file name, extract path from  log file name
      size_t uSlashPosition = stringLogFileName.rfind('\\');
      if( gd::utf8::is_found(uSlashPosition) == false ) { uSlashPosition = stringLogFileName.rfind('/'); }

      stringDirectory = stringLogFileName.substr(0, uSlashPosition + 1);         // copy directory (bring the slash)
   }


   if( stringDirectory.empty() == false ) { stringBackupToName = stringDirectory; } // if extracted directory, then assign to generated backup file

   // ## append core name to generated backup file name, if name has file extension
   //    then just take the name part and add file extension later

   std::string_view stringExtension{ ".txt" };
   std::string_view::size_type uStopSearch = stringBackupName.length() > eMaxExtensionLength ? eMaxExtensionLength : stringBackupName.length();

   // ### append name, if name contains file extension then save that for later
   auto uDotPosition = stringBackupName.rfind('.', uStopSearch);
   if( gd::utf8::is_found(uDotPosition) == true )                                // found a dot in backup name? if found then name has extension and that is added later
   {
      stringBackupToName += stringBackupName.substr( 0, uDotPosition );          // extract name from backup name that also have extension
      stringExtension = stringBackupName.data() + uDotPosition;                  // save file extension for later
   }
   else
   {
      stringBackupToName += stringBackupName;                                    // add name (name here has no extension)
   }

   if( has_flag_s(uOptions, eOptionIndex) )                                      // add index number to file name ?
   {
      // ## generate index string, pads index with zeros
      std::string stringIndex = std::to_string(uBackupIndex);
      char pbszNumber[eMaxIndexTextLength] = { '0','0','0','0','0','0','\0' };

      if( stringIndex.length() < eMaxIndexTextLength )
      {
         unsigned uIndex = eMaxIndexTextLength - 2;
         for( auto it = std::rbegin(stringIndex); it != std::rend(stringIndex); it++ )
         {
            pbszNumber[uIndex] = *it;
            uIndex--;
         }
      }
      else { return { false, "index value for backup file is to large" }; }

      stringBackupToName += pbszNumber;                                          // append backup index to file
   }

   if( has_flag_s( uOptions, eOptionExtension ) ) stringBackupToName += stringExtension; // add file extension

   if( has_flag_s(uOptions, eOptionCopy) )
   {
      bool bOk = false;
      try
      {
         bOk = std::filesystem::copy_file(stringLogFileName, stringBackupToName);// create clone as generated temp file
      }
      catch( ... ) { bOk = false; }

      if( bOk == false )
      {
         std::string stringError{ "copy failed - from \"" };
         stringError += stringLogFileName;
         stringError += "\" to \"";
         stringError += stringBackupToName;
         stringError += "\"";
         return { bOk, stringError };
      }
   }


   return { true, stringBackupToName };
}

/*----------------------------------------------------------------------------- datetime_now_s */ /**
 * Generate date time value in ISO 8601 format
 * \note information about strftime - https://cplusplus.com/reference/ctime/strftime/
 * \return std::string current time in as text
 */
std::string backup_history::datetime_now_s()
{
   time_t uCurrentTime;                                                        // has current time
   time(&uCurrentTime);                                                        // get current time
   char pbszTime[sizeof "2000-01-01T01:01:01Z"];                               // buffer storing date and time value as text
   strftime(pbszTime, sizeof(pbszTime), "%FT%TZ", localtime(&uCurrentTime));   // format date and time as text

   return pbszTime;
}

/*----------------------------------------------------------------------------- datetime_now_s */ /**
 * Generate date value in ISO 8601 format
 * \note information about strftime - https://cplusplus.com/reference/ctime/strftime/
 * \return std::string current time in as text
 */
std::string backup_history::date_now_s()
{
   time_t uCurrentTime;                                                        // has current time
   time(&uCurrentTime);                                                        // get current time
   char pbszTime[sizeof "2000-01-01 "];                                        // buffer storing date and time value as text
   strftime(pbszTime, sizeof(pbszTime), "%F", localtime(&uCurrentTime));       // format date

   return pbszTime;
}


/*----------------------------------------------------------------------------- time_now_s */ /**
 * Generate time value in ISO 8601 format
 * \return std::string current time in as text
 */
std::string backup_history::time_now_s()
{
   time_t uCurrentTime;                                                        // has current time
   time(&uCurrentTime);                                                        // get current time
   char pbszTime[sizeof "01:01:01"];                                           // buffer storing time value as text
   strftime(pbszTime, sizeof(pbszTime), "%T", localtime(&uCurrentTime));       // format time as text

   return pbszTime;
}

/*----------------------------------------------------------------------------- time_now_s */ /**
 * Generate time value similar to ISO 8601 format but no separator
 * \return std::string current time in as text
 */
std::string backup_history::time_now_s( tag_filename )
{
   time_t uCurrentTime;                                                        // has current time
   time(&uCurrentTime);                                                        // get current time
   char pbszTime[sizeof "010101"];                                             // buffer storing time value as text (time minutes seconds with two characters for each)
   strftime(pbszTime, sizeof(pbszTime), "%H%M%S", localtime(&uCurrentTime));   // format time as text

   return pbszTime;
}


/*----------------------------------------------------------------------------- year_now_s */ /**
 * Generate year value in ISO 8601 format
 * \return std::string current year as text
 */
std::string backup_history::year_now_s()
{
   time_t uCurrentTime;                                                          // has current time
   time(&uCurrentTime);                                                          // get current time
   char pbszTime[sizeof "2000"];                                                 // buffer storing year value as text
   strftime(pbszTime, sizeof(pbszTime), "%Y", localtime(&uCurrentTime));         // format year as text

   return pbszTime;
}



/*----------------------------------------------------------------------------- file_stash_log_s */ /**
 * update history containing backup files
 * \param stringHistoryFileName name of history file where time and filenames are found
 * \param stringLogFileName active log file that is stashed
 * \param stringBackupName name for stashed log files
 * \param uCount max number of files in history
 * \return std::pair<bool, std::string>
 */
std::pair<bool, std::string> backup_history::file_stash_log_s(const std::string_view& stringHistoryFileName, const std::string_view& stringLogFileName, const std::string_view& stringBackupName, unsigned uCount)
{
   if( uCount == 0 ) uCount = 20;

   int iMaxIndex = 1;
   std::string stringBackupHistory; // keep backup name for history file if there is one

   // ## read history file containing date and backed up log files
   auto vectorHistory = file_read_date_name_s(stringHistoryFileName);            // read history information
   if( vectorHistory.empty() == false )
   {
      iMaxIndex = gd::file::rotate::backup_history::find_max_index(vectorHistory); // find max index
      iMaxIndex += 1;                                                            // add one to new backup name for log file

      stringBackupHistory = file_backup_as_temporary_s(stringHistoryFileName);   // backup history file
   }

   // backup log file to file with generated name based on parameters sent to `file_backup_log_s`
   auto [bOk, stringBackupTo] = file_backup_log_s(stringLogFileName, stringBackupName, iMaxIndex, eOptionIndex | eOptionExtension | eOptionCopy);

   if( bOk == true )
   {
      std::string stringDateTime = gd::file::rotate::backup_history::datetime_now_s(); // get current time
      vectorHistory.push_back({ stringDateTime, stringBackupTo });               // adds date and filename to vector
   }

   // ## sort on time, latest time is placed first and oldest last
   std::sort(std::begin(vectorHistory), std::end(vectorHistory), [](const std::pair< std::string, std::string>& v1, const std::pair< std::string, std::string>& v2) {
      return v1.first > v2.first;
   });

   file_delete_backup_s(vectorHistory, uCount);                                  // delete entries from vector (and files)

   file_write_date_name_s(stringHistoryFileName, vectorHistory, false);          // update history file

   if( stringBackupHistory.empty() == false ) file_delete_backup_s(stringBackupHistory); // delete backup

   return { true, "" };
}

_GD_FILE_ROTATE_END