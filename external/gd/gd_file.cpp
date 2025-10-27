#include <stdio.h>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>
#include <chrono>

//#undef stat

//#include <sys/types.h>
#include <sys/stat.h>

#include "gd_utf8.hpp"

#ifdef WIN32

#include <windows.h>
#include <ShlObj_core.h>

#endif

#if defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#endif

#include "gd_file.h"


_GD_FILE_BEGIN

/** ---------------------------------------------------------------------------
 * @brief load text file into stl string object
 * \param stringFileName name of file read into string
 * \param stringFile string reference where file data is placed
 */
std::pair<bool, std::string> read_file_g( const std::string_view& stringFileName, std::string& stringFile )
{
   std::filesystem::path pathFile( stringFileName );
   std::error_code errorcode_;
   auto uFileSize = std::filesystem::file_size( pathFile, errorcode_ );
   if( errorcode_  ) return { false, std::string( errorcode_.message() ) + " " + stringFileName.data() }; // return error if file doesn't exist
   auto uCurrentSize = stringFile.length();
   auto uTotalSize = uFileSize + uCurrentSize;

   stringFile.resize( uTotalSize );

   std::ifstream ifstreamRead( pathFile, std::ios::in | std::ios::binary );

   ifstreamRead.read( stringFile.data() + uCurrentSize, uFileSize );

   return { true, "" };
}

std::pair<bool, std::string> write_file_g( const std::string_view& stringFileName, const std::string_view& stringFile )
{
   std::filesystem::path pathFile( stringFileName );

   std::ofstream ofstream_( stringFileName.data(), std::ios::binary | std::ios::out );

   if( ofstream_.is_open() == true )
   {
      ofstream_.write( stringFile.data(), stringFile.length() );
      ofstream_.close();
   }

   // fwrite(stringFile.data(), sizeof(char), stringFile.length(), ofstream_);

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief delete file on disk
 * If no file for name then nothing is done
 * @param stringFileName name for file to delete
 * @return true if file was deleted, false and error information if failed
*/
std::pair<bool, std::string> delete_file_g( const std::string_view& stringFileName )
{
   if( std::filesystem::exists( stringFileName ) == true )
   {
      int iResult = remove( stringFileName.data() );
      if( iResult != 0 )
      {
         auto error_ = strerror(iResult);
         return { false, error_ };
      }
   }

   return { true, "" };
}

/*----------------------------------------------------------------------------- get_known_folder_path_g */ /**
 * Get full path for known folder. *known* means a folder that has some sort
 * of special functionality in the OS. Only a few folders are supported.
 * \param stringFolderId simple name for folder path is asked for
 * \return std::pair<bool, std::wstring> true and path to folder if success, false and error information if error
 */
std::pair<bool, std::string> get_known_folder_path_g(const std::string_view& stringFolderId)
{                                                                                assert( stringFolderId.length() > 0 );
   std::string stringFolderPath; // gets path to requested folder
   // ## copy first four bytes if requested id holds that many characters, copy is designed to be fast
   char pbFolderId[] = { '\0', '\0', '\0', '\0' }; // buffer used to match the requested folder path     // > variables declared on their own line should be commented
   if( stringFolderId.length() > 3 )
   {
      *(uint32_t*)pbFolderId = *(uint32_t*)stringFolderId.data();                // let compiler optimize
      for( auto it = pbFolderId, itLast = pbFolderId + sizeof(pbFolderId); it != itLast; it++ ) // convert to uppercase
      {
         if( *it >= 'a' ) *it -= ('a' - 'A');                                    // change to uppercase if lowercase letter
      }
   }
   else { pbFolderId[0] = *stringFolderId.data(); }
                                                                                                     
   // ## Find out what type of folder path to return, checks characters in                           
   //    folder id to get the right one.                                                             
                                                                                                     
#  ifdef WIN32                                                                                       
   const GUID* pguidFolderId = nullptr; // pointer to folder guid that path is returned for
   switch( pbFolderId[0] )                                                       // match what folder path to get
   {
   case 'd':
   case 'D' :
      pguidFolderId = &FOLDERID_Documents;                                       // "DOCUMENTS"
      if( pbFolderId[2] == 'W' ) pguidFolderId = &FOLDERID_Downloads;            // "DOWNLOADS"
      else if( pbFolderId[2] == 'V' ) pguidFolderId = &FOLDERID_Device;          // "DEVICE"
      break;
   case 'r':
   case 'R':
      pguidFolderId = &FOLDERID_Recent;                                          // "RECENT"
      if( pbFolderId[3] == 'Y' ) pguidFolderId = &FOLDERID_RecycleBinFolder;     // "RECYCLEBINFOLDER"
      break;
   default:                                                                      assert(false); // don't know this folder
      std::string stringError = "Unknown folder id: ";
      for( auto it: stringFolderId ) stringError += it;
      return { false, stringError };
   }                                                                                                    
                                                                                                        
                                                                                                        
                                                                                                        
   // ## Try to get path to folder                                                                      
   if( pguidFolderId != nullptr )                                                                       
   {
      wchar_t* pwszPath; // gets pointer to allocated buffer with folder path             
      if( FAILED(::SHGetKnownFolderPath(*pguidFolderId, 0, nullptr, &pwszPath)) ) // win32 api to get folder path
      {
         std::string stringError{ "Failed to get known folder name, error is: " };
         stringError += std::to_string(GetLastError());
         return { false, stringError };
      }
      else
      {
         std::wstring string_ = pwszPath;
         stringFolderPath = gd::utf8::convert_unicode_to_ascii( string_ );
         ::CoTaskMemFree(pwszPath);                                              // deallocate buffer with path
      }
   }
#  else
   switch( pbFolderId[0] )                                                       // match what folder path to get
   {
   case 'd':
   case 'D' : 
   {
      const char* pbszHome = getenv("HOME");
      if(pbszHome != nullptr) 
      {
         stringFolderPath = pbszHome;
      }
   }
   break;
      
   default:                                                                      assert(false); // don't know this folder
      std::string stringError = "Unknown folder id: ";
      for( auto it: stringFolderId ) stringError += it;
      return { false, stringError };
   }                                                                                                      // function name and comments at a margin?
#  endif

   return { true, stringFolderPath };
}



/*----------------------------------------------------------------------------- get_known_folder_wpath_g */ /**
 * Get full path for known folder. *known* means a folder that has some sort
 * of special functionality in the OS. Only a few folders are supported.
 * \param stringFolderId simple name for folder path is asked for
 * \return std::pair<bool, std::wstring> true and path to folder if success, false and error information if error
 */
std::pair<bool, std::wstring> get_known_folder_wpath_g(const std::string_view& stringFolderId)
{                                                                                assert( stringFolderId.length() > 0 );
   std::wstring stringFolderPath; // gets path to requested folder
   // ## copy first four bytes if requested id holds that many characters, copy is designed to be fast
   char pbFolderId[] = { '\0', '\0', '\0', '\0' }; // buffer used to match the requested folder path     // > variables declared on their own line should be commented
   if( stringFolderId.length() > 3 )
   {
      *(uint32_t*)pbFolderId = *(uint32_t*)stringFolderId.data();                // let compiler optimize
      for( auto it = pbFolderId, itLast = pbFolderId + sizeof(pbFolderId); it != itLast; it++ ) // convert to uppercase
      {
         if( *it >= 'a' ) *it -= ('a' - 'A');                                    // change to uppercase if lowercase letter
      }
   }
   else { pbFolderId[0] = *stringFolderId.data(); }
                                                                                                       
   // ## Find out what type of folder path to return, checks characters in                             
   //    folder id to get the right one.                                                               
                                                                                                       
#  ifdef WIN32                                                                                         
   const GUID* pguidFolderId = nullptr; // pointer to folder guid that path is returned for
   switch( pbFolderId[0] )                                                       // match what folder path to get
   {
   case 'd':
   case 'D' :
      pguidFolderId = &FOLDERID_Documents;                                       // "DOCUMENTS"
      if( pbFolderId[2] == 'W' ) pguidFolderId = &FOLDERID_Downloads;            // "DOWNLOADS"
      else if( pbFolderId[2] == 'V' ) pguidFolderId = &FOLDERID_Device;          // "DEVICE"
      break;
   case 'r':
   case 'R':
      pguidFolderId = &FOLDERID_Recent;                                          // "RECENT"
      if( pbFolderId[3] == 'Y' ) pguidFolderId = &FOLDERID_RecycleBinFolder;     // "RECYCLEBINFOLDER"
      break;
   default:                                                                      assert(false); // don't know this folder
      std::wstring stringError = L"Unknown folder id: ";
      for( auto it: stringFolderId ) stringError += it;
      return { false, stringError };
   }                                                                                                     
                                                                                                         
                                                                                                         
                                                                                                         
   // ## Try to get path to folder                                                                       
   if( pguidFolderId != nullptr )                                                                        
   {
      wchar_t* pwszPath; // gets pointer to allocated buffer with folder path             
      if( FAILED(::SHGetKnownFolderPath(*pguidFolderId, 0, nullptr, &pwszPath)) ) // win32 api to get folder path
      {
         std::wstring stringError{ L"Failed to get known folder name, error is: " };
         stringError += std::to_wstring(GetLastError());
         return { false, stringError };
      }
      else
      {
         stringFolderPath = pwszPath;
         ::CoTaskMemFree(pwszPath);                                              // deallocate buffer with path
      }
   }
#  else
   switch( pbFolderId[0] )                                                       // match what folder path to get
   {
   case 'd':
   case 'D' : 
   {
      const char* pbszHome = getenv("HOME");
      if(pbszHome != nullptr) 
      {

      }
   }
      
   default:                                                                      assert(false); // don't know this folder
      std::wstring stringError = L"Unknown folder id: ";
      for( auto it: stringFolderId ) stringError += it;
      return { false, stringError };
   }                                                                                                      // function name and comments at a margin?

#  endif

   return { true, stringFolderPath };
}

/** ---------------------------------------------------------------------------
 * @brief  try to fix path for current os
 * Removes double folder separators and convert to right separator for os
 * @param stringPath path to be fixed
 * @param uOffset where to start in string to fix path
 * @return std::string fixed path
 */
std::string fix_path_g( const std::string_view& stringPath, unsigned uOffset )
{
   char chPrevious = 0;
   std::string stringFixedPath;

   if( stringPath.empty() == true ) return stringFixedPath;

   for( auto it = stringPath.begin() + uOffset, itEnd = stringPath.end(); it != itEnd; it++ )
   {
      if( *it == '/' || *it == '\\' ) 
      {
         // if double // or \\ then do not add to final path
         if( chPrevious != *it ) { stringFixedPath += *it; }
      }
      else
      {
         stringFixedPath += *it;
      }

      chPrevious = *it;
   }

   stringFixedPath = normalize_path_for_os_g( stringFixedPath );

   return stringFixedPath;
}

/** ---------------------------------------------------------------------------
 * @brief Extract file name from path
 * @param stringPath path from where to extract file name
 * @return std::string file name
 */
std::string extract_file_name_g(const std::string_view& stringPath) 
{
   return std::filesystem::path(stringPath).filename().string();
}



/*----------------------------------------------------------------------------- closest_having_file_g */ /**
 * Try to find first parent folder containing specified file
 * Walks up in the folder hierarchy and tries to find specified file in folder, if found then return
 * folder, if not found go to parent folder and try to find file there.
 * This is done until no parent folders exists
 * \param stringPath start folder to begin search
 * \param stringFindFile file to search for 
 * \return std::pair<bool, std::string> true and folder name if found, false and empty string if not found
 */
std::pair<bool, std::string> closest_having_file_g(const std::string_view& stringPath, const std::string_view& stringFindFile)
{                                                                                assert(stringPath.empty() == false); assert(stringFindFile.empty() == false);   
   std::filesystem::path pathMatch(stringPath);
   std::string_view stringfolderSeparator{ "\\/" };

   pathMatch = pathMatch.parent_path();
   auto uRootMarkerLength = stringFindFile.length();

   while( pathMatch.root_name().string().length() + 1 < pathMatch.string().length() ) // check length for active folder, if it is longer than root than try to find root file
   {
      for( const auto& it : std::filesystem::directory_iterator(pathMatch) )
      {
         if( it.is_regular_file() == true )
         {
            std::string stringFileName = it.path().string();
            auto uFileNameLength = stringFileName.length();
            if( stringFileName.find(stringFindFile) != std::string::npos && 
                  stringfolderSeparator.find( stringFileName[uFileNameLength - uRootMarkerLength - 1] ) != std::string::npos 
               )
            {
               return  { true, stringFileName.substr(0, stringFileName.length() - stringFindFile.length()) };
            }
         }
      }
      pathMatch = pathMatch.parent_path();
   }

   return {false, std::string()};
}

/** ---------------------------------------------------------------------------
 * @brief Finds parent folder containing file and if found, adds string and return it
 * \param stringPath start folder to begin search
 * \param stringFindFile file to search for 
 * @param stringAppend appends string to found folder and return the final string
 * @return true and genderated path found, false if no folder found
*/
std::pair<bool, std::string> closest_having_file_g( const std::string_view& stringPath, const std::string_view& stringFindFile, const std::string_view& stringAppend )
{
   auto result_ = closest_having_file_g( stringPath, stringFindFile );

   if( result_.first == true && stringAppend.empty() == false )
   {
      if( is_directory_separator_g( stringAppend[0] ) == true && is_directory_separator_g( result_.second.back() ) == true )
      {  // last character in found path is used as separator and first character adding to found 
         // path is also used as separator, skip first character.
         result_.second += std::string_view( stringAppend.data() + 1, stringAppend.length() - 1 );
      }
      else
      {
         result_.second += stringAppend;
      }
      return result_;
   }

   return {false, std::string()};
}

/** ---------------------------------------------------------------------------
 * @brief get parent paths from current
 * @param stringPath from path where to go from
 * @param uParentCount number of parent levels to traverse
 * @return std::string path name for parent folder method traversed to
 */
std::string parent_g( const std::string_view& stringPath, unsigned uParentCount )
{                                                                                                  assert( std::filesystem::exists( stringPath ) == true ); assert( uParentCount < 100 );
   std::filesystem::path pathGoto(stringPath);

   while( uParentCount > 0 )
   {
      pathGoto = pathGoto.parent_path();
      uParentCount--;
   }

   return pathGoto.string();
}


/** ---------------------------------------------------------------------------
 * @brief List files in directory
 * @param stringFolder 
 * @return 
*/
std::vector<std::string> list_files_g(const std::string_view& stringFolder )
{                                                                                                  assert( std::filesystem::is_directory( stringFolder ) == true );
   std::vector<std::string> vectorFile;

   for( const auto& itFile : std::filesystem::directory_iterator(stringFolder) )
   {
      if( itFile.is_regular_file() == false ) continue;

      vectorFile.push_back( itFile.path().string() );
   }

   return vectorFile;
}


/*----------------------------------------------------------------------------- dir */ /**
 * List files in specified folder
 * 
~~~{.cpp}
// list files with the pattern log(xxxx).txt that is at least one day old
auto vectorFile = gd::file::list_files(stringPath, { {"filter", R"(^log[\.\d].*\.txt)"}, {"to_days", -1} });
~~~
 * 
 * \param stringFolder folder where files is listed from
 * \param argumentsFilter different filters to select those files that you want to list, all are optional
 * \param   argumentsFilter["filter"] regular expression used to match file
 * \param   argumentsFilter["to_days"] match days, if file is older compared to days sent then it is a match
 * \param   argumentsFilter["extension"] match file extension
 * \return std::vector<std::string> files found in folder that match filters if any is sent
 */
std::vector<std::string> list_files_g(const std::string_view& stringFolder, const gd::argument::arguments& argumentsFilter )
{
   std::vector<std::string> vectorFile;

   // ## filter method is used when filter is found in arguments, file name is matched against wildcard or regular expression
   auto filter_ = [](const std::string stringFileName, const auto& argumentFilter) -> bool {
      if( argumentFilter.is_text() )
      {
         std::string stringFilterOrRegex = argumentFilter.get_string();

         std::smatch smatchFirst;
         std::regex regexFind(stringFilterOrRegex);
         if( std::regex_search(stringFileName, smatchFirst, regexFind) == false ) return false; // not matched
      }
      return true;
   };

   // ## compare number of days, if file is older compared to days sent then return true
   auto day_count_ = [](const std::filesystem::path& pathFile, const auto& argumentToDays) -> bool {
      if( argumentToDays.is_number() )
      {
         using namespace std::literals::chrono_literals;
         auto dToDays = argumentToDays.get_double();

         time_t timeNow = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
         time_t timeDifference = static_cast<time_t>(dToDays * (60.0 * 60 * 24));

         //struct stat statFile;
         //stat(pathFile.string().c_str(), &statFile);

         //if( statFile.st_mtime >= (timeNow + timeDifference) ) return false;
      }
      return true;
   };

   auto day_count2_ = [](const std::filesystem::path& pathFile, const auto& argumentToDays) -> bool {
      if( argumentToDays.is_number() )
      {
         using namespace std::filesystem;
         using namespace std::chrono;

         // Get the last write time of the file
         file_time_type file_time_ = last_write_time(pathFile.string().c_str()); // get last write time for file

         // Convert file time to system time
         auto file_system_time_ = time_point_cast<system_clock::duration>( file_time_ - file_time_type::clock::now() + system_clock::now() );

         // Get current time and calculate the duration threshold
         auto now_ = system_clock::now();
         auto threshold_ = now_ - hours(argumentToDays.as_int() * 24);

         // Check if the file time is older than the threshold, then false is returned
         if( file_system_time_ <= threshold_ ) return false;
      }
      return true;
   };


   auto extension_ = []( std::string stringFileName, const auto& argumentExtension ) -> bool {
      if( argumentExtension.is_string() )
      {
         std::string stringExtension = argumentExtension.get_string();
         if( stringFileName.length() >= stringExtension.length() )
         {
            std::transform(stringFileName.begin(), stringFileName.end(), stringFileName.begin(), [](unsigned char character_){ return std::tolower(character_); });
            std::transform(stringExtension.begin(), stringExtension.end(), stringExtension.begin(), [](unsigned char character_){ return std::tolower(character_); });
            
            bool bMatch = std::equal( stringExtension.rbegin(), stringExtension.rend(), stringFileName.rbegin() );
            return bMatch;
         }
      }
      
      return true;
   };

   for( const auto& itFile : std::filesystem::directory_iterator(stringFolder) )
   {
      if( itFile.is_regular_file() == false ) continue;

      const std::string stringFile = itFile.path().filename().string();

      if( filter_(stringFile, argumentsFilter["filter"]) == false ) continue;    // filter using regex or wildcard

      if( day_count2_(itFile.path(), argumentsFilter["to_days"]) == false ) continue;// filter on time (days ?)

      if( extension_(stringFile, argumentsFilter["extension"]) == false ) continue;// filter on file extension

      vectorFile.push_back( itFile.path().string() );
   }

   return vectorFile;
}

/** ---------------------------------------------------------------------------
 * @brief Normalize path to work for active OS (operating system)
 * @param stringPath string that is normalized
 * @return normalized string
*/
std::string normalize_path_for_os_g( const std::string_view& stringPath )
{
   std::filesystem::path path(stringPath);
   std::string stringNormalized = path.make_preferred().string();
   return stringNormalized;
}

/** ---------------------------------------------------------------------------
 * @brief open file
 * @param stringFileName name of file to open
 * @param bEnd move to end of file if true
 * @return true if file was opened, false and error information if not
*/
std::pair<int, std::string> file_open_g( const std::string_view& stringFileName, bool bEnd )
{                                                                                                  assert( stringFileName.length() > 3 ); // realistic filename
   int iFileHandle = 0;
#  if defined(_WIN32)
   ::_sopen_s(&iFileHandle, stringFileName.data(), _O_CREAT | _O_WRONLY | _O_BINARY | _O_NOINHERIT, _SH_DENYWR, _S_IREAD | _S_IWRITE); assert( iFileHandle >= 0 );
   if( iFileHandle >= 0 && bEnd == true ) _lseek( iFileHandle, 0, SEEK_END );
#  else
   iFileHandle = open(stringFileName.data(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); assert( iFileHandle >= 0 );
   if( iFileHandle >= 0 && bEnd == true ) lseek( iFileHandle, 0, SEEK_END );
#  endif

   if( iFileHandle < 0 )
   {                                                                             // assert( false );
      std::string stringError("FILE OPEN ERROR: ");
      stringError += std::strerror(errno);
      return { iFileHandle, stringError };
   }

   return { iFileHandle, std::string() };
}

/** ---------------------------------------------------------------------------
 * @brief open file
 * @param stringFileName name of file to open
 * @param bEnd move to end of file if true
 * @return true if file was opened, false and error information if not
*/
std::pair<int, std::string> file_open_g( const std::wstring_view& stringFileName, bool bEnd )
{                                                                                                  assert( stringFileName.length() > 3 ); // realistic filename
   int iFileHandle = 0;
#  if defined(_WIN32)
   ::_wsopen_s(&iFileHandle, stringFileName.data(), _O_CREAT | _O_WRONLY | _O_BINARY | _O_NOINHERIT, _SH_DENYWR, _S_IREAD | _S_IWRITE); assert( iFileHandle >= 0 );
   if( iFileHandle >= 0 && bEnd == true ) _lseek( iFileHandle, 0, SEEK_END );
#  else
   std::string stringFileName_ = gd::utf8::convert_unicode_to_ascii( stringFileName );
   iFileHandle = open(stringFileName_.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); assert( iFileHandle >= 0 );
   if( iFileHandle >= 0 && bEnd == true ) lseek( iFileHandle, 0, SEEK_END );
#  endif

   if( iFileHandle < 0 )
   {                                                                             // assert( false );
      std::string stringError("FILE OPEN ERROR: ");
      stringError += std::strerror(errno);
      return { iFileHandle, stringError };
   }

   return { iFileHandle, std::string() };
}

/** ---------------------------------------------------------------------------
 * @brief write text to file
 * @param iFileHandle file handle to file written to
 * @param stringText text to write
 * @return true if ok, false and error information for errors
*/
std::pair<bool, std::string> file_write_g( int iFileHandle, const std::string_view& stringText )
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

/// close file
void file_close_g( int iFileHandle )
{                                                                                                  assert( iFileHandle > 0 );
#ifdef _MSC_VER   
   ::_close( iFileHandle );
#else
   close( iFileHandle );
#endif   
}



/** ---------------------------------------------------------------------------
 * @brief check if character is used to split folder names
 * @param chCharacter character to check
 * @return true if character is used to split folder names in directories
 */
bool is_directory_separator_g( char chCharacter )
{
   if( chCharacter == '/' || chCharacter == '\\' ) return true;
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief check if character is used to split folder names
 * @code
 * std::string stringPath = "/home/username/test/";
 * bool bSeparator = is_directory_separator_g( stringPath );      assert( bSeparator == true );
 * stringPath = "/home/username/test";
 * bSeparator = is_directory_separator_g( stringPath );           assert( bSeparator == false );
 * @endcode
 * @param stringPath check if last character in string is a directory separator
 * @return true if character is used to split directory names in paths
 */
bool is_directory_separator_g( const std::string_view& stringPath )
{
   if( stringPath.empty() == false )
   {
      return is_directory_separator_g( stringPath.back() );
   }

   return false;
}


/** ---------------------------------------------------------------------------
* @brief Read file permissions
* Read permission bits for file. On Windows this returns a simplified version
* with read, write, and execute flags. On Linux returns full permission bits.
* 
* @code
* std::pair<uint64_t, std::string> pairPermission;
* auto result_ = read_permission_g( "test.txt", &pairPermission );
* if( result_.first == true )
* {
*   std::cout << "Permissions: " << std::oct << pairPermission.first << " (" << pairPermission.second << ")" << std::endl;
* }
* else
* {
*   std::cerr << "Error reading permissions: " << result_.second << std::endl;
* }
* @endcode
* 
* @param stringFile path to file to read permissions from
* @param ppairPermission pointer to pair that gets permission bits and text description if not nullptr
* @return true if permissions were read successfully, false and error message if failed
*/
std::pair<bool, std::string> read_permission_g( const std::string_view& stringFile, std::pair<uint64_t, std::string>* ppairPermission )
{
   if( std::filesystem::exists( stringFile ) == false ) { return { false, "File does not exist: " + std::string(stringFile) }; }

#  ifdef WIN32
   // ## Windows implementation using GetFileAttributes
   DWORD uAttributes = ::GetFileAttributesA( stringFile.data() );
   if( uAttributes == INVALID_FILE_ATTRIBUTES )
   {
      std::string stringError{ "Failed to get file attributes, error is: " };
      stringError += std::to_string(GetLastError());
      return { false, stringError };
   }

   uint64_t uPermission = 0;
   std::string stringDescription;

   // Set read permission (all files are readable if we can get attributes)
   uPermission |= 0x0004; // read bit
   stringDescription += "r";

   // Check write permission
   if( (uAttributes & FILE_ATTRIBUTE_READONLY) == 0 )
   {
      uPermission |= 0x0002; // write bit
      stringDescription += "w";
   }
   else
   {
      stringDescription += "-";
   }

   // Check if executable (simplified check based on extension or attribute)
   std::filesystem::path pathFile( stringFile );
   std::string stringExtension = pathFile.extension().string();
   std::transform(stringExtension.begin(), stringExtension.end(), stringExtension.begin(), [](unsigned char c){ return std::tolower(c); });

   if( stringExtension == ".exe" || stringExtension == ".bat" || stringExtension == ".cmd" || stringExtension == ".com" )
   {
      uPermission |= 0x0001; // execute bit
      stringDescription += "x";
   }
   else
   {
      stringDescription += "-";
   }

   if( ppairPermission != nullptr )
   {
      ppairPermission->first = uPermission;
      ppairPermission->second = stringDescription;
   }

#  else
   // ## Linux/Unix implementation using stat
   struct stat statBuffer;
   if( stat( stringFile.data(), &statBuffer ) != 0 )
   {
      std::string stringError{ "Failed to get file status, error is: " };
      stringError += std::strerror(errno);
      return { false, stringError };
   }

   uint64_t uPermission = statBuffer.st_mode & 0777; // get permission bits

   if( ppairPermission != nullptr )
   {
      ppairPermission->first = uPermission;

      // Build description string (e.g., "rwxr-xr-x")
      std::string stringDescription;

      // Owner permissions
      stringDescription += (statBuffer.st_mode & S_IRUSR) ? "r" : "-";
      stringDescription += (statBuffer.st_mode & S_IWUSR) ? "w" : "-";
      stringDescription += (statBuffer.st_mode & S_IXUSR) ? "x" : "-";

      // Group permissions
      stringDescription += (statBuffer.st_mode & S_IRGRP) ? "r" : "-";
      stringDescription += (statBuffer.st_mode & S_IWGRP) ? "w" : "-";
      stringDescription += (statBuffer.st_mode & S_IXGRP) ? "x" : "-";

      // Other permissions
      stringDescription += (statBuffer.st_mode & S_IROTH) ? "r" : "-";
      stringDescription += (statBuffer.st_mode & S_IWOTH) ? "w" : "-";
      stringDescription += (statBuffer.st_mode & S_IXOTH) ? "x" : "-";

      ppairPermission->second = stringDescription;
   }
#  endif

   return { true, "" };
}



path& path::add(const std::string_view& stringName)
{
   std::string stringPath( stringName );
   normalize_path_s(stringPath);
#ifdef _WIN32
   if( has_separator() == false || stringPath[0] != m_iPathDivider_s ) m_stringPath += m_iPathDivider_s;
#else
   if( has_separator() == false ) m_stringPath += '/';
#endif
   m_stringPath += stringPath;
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add names from list to path
 * @param listName list of names to add to path
 * @return reference to path
 */
path& path::add(const std::initializer_list<std::string_view>& listName)
{
   for ( auto it : listName )
   {
      add(it);
   }
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add names from list to path  
 * @param listName list of names to add 
 * @param callback_ for each added name, callback is called with current path, if callback returns false then adding is stopped
 * @return true if all names were added, false and path if callback returned false
 */
std::pair<bool, std::string> path::add(const std::initializer_list<std::string_view>& listName, std::function<bool(const std::string& stringName)> callback_)
{
   for ( auto it : listName )
   {
      add(it);
      if( callback_(m_stringPath) == false ) return { false, m_stringPath };
   }
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Add names from vector to path
 * @param vectorName vector of names to add to path
 * @return reference to path
 */
path& path::add(const std::vector<std::string_view>& vectorName)
{
   for ( auto it : vectorName )
   {
      add(it);
   }
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Add names from list to path  
 * @param vectorName vector of names to add to path
 * @param callback_ for each added name, callback is called with current path, if callback returns false then adding is stopped
 * @return true if all names were added, false and path if callback returned false
 */
std::pair<bool, std::string> path::add(const std::vector<std::string_view>& vectorName, std::function<bool(const std::string& stringName)> callback_)
{
   for ( auto it : vectorName )
   {
      add(it);
      if( callback_(m_stringPath) == false ) return { false, m_stringPath };
   }
   return { true, "" };
}



/** ---------------------------------------------------------------------------
 * @brief  Concatenate two paths and return new path
 * @param path_ path to concatenate with current path
 * @return path new path with concatenated paths
 */
path path::concatenate(const path& path_) const
{
   std::string stringNewPath = m_stringPath;
   if( has_separator() == false && path_.has_begin_separator() == false )
   {
      stringNewPath += m_iPathDivider_s;
   }

   stringNewPath += path_.m_stringPath;
   return path( std::move( stringNewPath ), gd::types::tag_raw{} );
}

/** ---------------------------------------------------------------------------
 * @brief count number of folder and file in path
 * @code 
 * path pathTest("C:\\Users\\Public\\Documents");
 * assert( pathTest.count() == 4 );
 * pathTest += "my_text.txt";
 * assert( pathTest.count() == 5 );
 * @endcode
 * @return size_t number of folders and files in path
 */
std::size_t path::count() const
{
   if( empty() == true ) return 0;
   std::size_t uCount = 1;
   for( auto it = m_stringPath.begin(), itEnd = m_stringPath.end(); it != itEnd; it++ )
   {
      if( *it == m_iPathDivider_s ) uCount++;
   }
   return uCount;
}

/** ---------------------------------------------------------------------------
* @brief Remove count number of file/folders from end
* @return reference to path
*/
path& path::erase( std::size_t uCount )
{
   const char* pbsz_ = m_stringPath.c_str();
   const char* pbszPosition = pbsz_ + m_stringPath.length();

   while( pbszPosition > pbsz_ && uCount > 0 )
   {
      pbszPosition--;
      if( *pbszPosition == m_iPathDivider_s ) uCount--;
   }

   std::size_t uLength = pbszPosition - pbsz_;                                                     assert( uLength < 0x1000 ); // realistic

   m_stringPath = m_stringPath.substr( 0, uLength );

   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief Erace last part of path
 * @return reference to path
 */
path& path::erase_end()
{
   auto position_ = m_stringPath.rfind( m_iPathDivider_s );
   if( position_ != std::string::npos )
   {
      m_stringPath = m_stringPath.substr( 0, position_ );
      return *this;
   }

   clear();
   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief normalize path
 * @param stringPath path to normalize
 */
void path::normalize_path_s(std::string& stringPath)
{
   char iReplace = m_iPathDivider_s == '/' ? '\\' : '/';
   std::replace(stringPath.begin(), stringPath.end(), iReplace, m_iPathDivider_s );
}


// ----------------------------------------------------------------------------
// ------------------------------------------------------------------ directory
// ----------------------------------------------------------------------------


std::pair<bool, std::string> directory::dir()
{
   for( const auto& it : std::filesystem::directory_iterator( std::filesystem::path(m_stringPath) ) )
   {
      if(it.is_regular_file())
      {
        add( it.path().string() );
      }
   }
   return { true, "" };
}

std::pair<bool, std::string> directory::dir( gd::types::tag_recursive )
{
   /*
   for( const auto& it : std::filesystem::recursive_directory_iterator(std::filesystem::path(m_stringPath)) )
   {
      if( it.is_regular_file() )
      {
         add(it.path().string());
      }
   }
   */
   return std::pair<bool, std::string>();
}


_GD_FILE_END

