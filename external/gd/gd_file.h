/**
 * \file gd_file.h
 * 
 * \brief Miscellaneous file operations 
 * 
 */


#pragma once

#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#include <fcntl.h>

#include "gd_arguments.h"

#ifndef _GD_FILE_BEGIN

#  define _GD_FILE_BEGIN namespace gd { namespace file {
#  define _GD_FILE_END } }

#endif

_GD_FILE_BEGIN

enum enumOption
{
   eOptionMakePreffered = 0b0000'0000'0000'0000'0000'0000'0000'0001,
   eOptionLowercase     = 0b0000'0000'0000'0000'0000'0000'0000'0010,
   eOptionUppercase     = 0b0000'0000'0000'0000'0000'0000'0000'0100,
   eOptionExists        = 0b0000'0000'0000'0000'0000'0000'0000'1000,
   eOptionRoot          = 0b0000'0000'0000'0000'0000'0000'0001'0000,
   eOptionParent        = 0b0000'0000'0000'0000'0000'0000'0010'0000,
};

// ## file operations

std::pair<bool, std::string> read_file_g( const std::string_view& stringFileName, std::string& stringFile );
std::pair<bool, std::string> write_file_g( const std::string_view& stringFileName, const std::string_view& stringFile );
std::pair<bool, std::string> delete_file_g( const std::string_view& stringFileName );


// ## folder operations

/// gets known folder path for folder name
std::pair<bool, std::string> get_known_folder_path_g(const std::string_view& stringFolderId);
std::pair<bool, std::wstring> get_known_folder_wpath_g(const std::string_view& stringFolderId);

/// fix path to make it work, removes double // or \\ and converts to correct divider based on os
std::string fix_path_g( const std::string_view& stringPath, unsigned uOffset );
inline std::string fix_path_g( const std::string_view& stringPath ) { return fix_path_g( stringPath, 0 ); }


std::string extract_file_name_g( const std::string_view& stringPath );


// ## `closest` are used to find nearest folder in the parent hierarchy

std::pair<bool, std::string> closest_having_file_g(const std::string_view& stringPath, const std::string_view& stringFindFile);
std::pair<bool, std::string> closest_having_file_g(const std::string_view& stringPath, const std::string_view& stringFindFile, const std::string_view& stringAppend);
std::pair<bool, std::string> closest_having_file_g(const std::string_view& stringPath, const std::string_view& stringFindFile, const std::string_view& stringAppend, unsigned uOption );

// ## traverse

/// get parent folder
std::string parent_g( const std::string_view& stringPath, unsigned uLevel );

// ## files in folder

std::vector<std::string> list_files_g( const std::string_view& stringFolder );
std::vector<std::string> list_files_g(const std::string_view& stringFolder, const gd::argument::arguments& argumentsFilter);

// ## `file` operations

// ## `file` path logic
std::string  normalize_path_for_os_g( const std::string_view& stringPath );

// ## file open, write and close logic

std::pair<int, std::string> file_open_g(const std::string_view& stringFileName, bool bEnd );
std::pair<int, std::string> file_open_g(const std::wstring_view& stringFileName, bool bEnd );

std::pair<bool, std::string> file_write_g( int iFileHandle, const std::string_view& stringText );

void file_close_g( int iFileHandle );


// ## `file` name logic
bool is_directory_separator_g( char chCharacter );
bool is_directory_separator_g( const std::string_view& stringPath );

// ### 

std::pair<int, std::string> file_add_reference_g(const std::string_view& stringFindName);

//std::pair<int, std::string> file_release_reference_g(const std::string_view& stringFindName);


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------- path
// ----------------------------------------------------------------------------


/**
 * \brief simplified path logic
 *
 * Handle path values and make sure they are correctly formatted based on operating system.
 */
struct path
{
   // ## construction ------------------------------------------------------------
   path() {}
   path( const char* pbszPath ): m_stringPath( pbszPath ) { normalize_path_s( m_stringPath ); }
   path( const std::string_view& stringPath ): m_stringPath( stringPath ) { normalize_path_s( m_stringPath ); }
   path( std::string&& stringPath ): m_stringPath( std::move(stringPath) ) { normalize_path_s( m_stringPath ); }
   path( std::string&& stringPath, gd::types::tag_raw ): m_stringPath( std::move(stringPath) ) {}
   // copy
   path(const path& o) { common_construct(o); }
   path(path&& o) noexcept { common_construct(std::move(o)); }
   // assign
   path& operator=(const path& o) { common_construct(o); return *this; }
   path& operator=(path&& o) noexcept { common_construct(std::move(o)); return *this; }
   operator std::string() const { return m_stringPath; }
   operator const char* ( ) const { return m_stringPath.c_str(); }
   operator bool() const { return m_stringPath.empty() == false; }

   ~path() {}
   // common copy
   void common_construct(const path& o) { m_stringPath = o.m_stringPath; }
   void common_construct(path&& o) noexcept { m_stringPath = std::move( o.m_stringPath ); }

   path& operator+=(const std::string_view& stringName) { return add(stringName); }
   path operator/(const path& path_) { return concatenate(path_); }

   // ## methods -----------------------------------------------------------------
   /// Checks if path has filename
   bool has_filename() const { return m_stringPath.empty() == false ? std::filesystem::path(m_stringPath).has_filename() : false; }
   /// Checks if path ends with folder separator
   bool has_separator() const { return m_stringPath.back() == m_iPathDivider_s; }
   /// Checks if path starts with folder separator
   bool has_begin_separator() const { return m_stringPath.empty() == false ? m_stringPath[0] == m_iPathDivider_s : false; }

   /// Get path as string
   std::string string() const { return m_stringPath; }
   /// Get filename from path
   path filename() const { return path(std::filesystem::path(m_stringPath).filename().string()); }
   /// Get file extension from path if any
   path extension() const { return path(std::filesystem::path(m_stringPath).extension().string()); }

   /// Add folder or filename to path
   path& add(const std::string_view& stringName);
   /// Concatenate two paths
   path concatenate(const path& path_);

   /// Return if path is empty
   bool empty() const { return m_stringPath.empty(); }
   /// Return path length
   std::size_t length() const { return m_stringPath.length(); }

   /// Remove filename from path
   path& remove_filename() { m_stringPath = std::filesystem::path(m_stringPath).remove_filename().string(); return *this; }
   /// Replace filename in path
   path& replace_filename(const std::string_view& stringName) { m_stringPath = std::filesystem::path(m_stringPath).replace_filename(stringName).string(); return *this; }
   /// Replace extension in path
   path& replace_extension(const std::string_view& stringName) { m_stringPath = std::filesystem::path(m_stringPath).replace_extension(stringName).string(); return *this; }
   /// Clear path
   void clear() { m_stringPath.clear(); }

   /// ## Iterator methods for path

   std::string::iterator begin() { return m_stringPath.begin(); }
   std::string::iterator end() { return m_stringPath.end(); }
   std::string::const_iterator begin() const { return m_stringPath.begin(); }
   std::string::const_iterator end() const { return m_stringPath.end(); }


   /** \name DEBUG
   *///@{

   //@}

   // ## attributes --------------------------------------------------------------
   std::string m_stringPath;
   inline static char m_iPathDivider_s = std::filesystem::path::preferred_separator;

   // ## free functions ----------------------------------------------------------
   static void normalize_path_s( std::string& stringPath);

};


_GD_FILE_END

