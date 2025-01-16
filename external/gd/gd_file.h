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



_GD_FILE_END

