#pragma once

#include <fstream>
#include <filesystem>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"


/**
 * @brief Counts the number of lines in a files
 */
class CRowCount
{
public:

   CRowCount(std::vector<std::string> &vectorFiles, std::vector<std::string> &vectorFilter)
   {
      m_vectorFiles = vectorFiles;
      m_vectorFilter = vectorFilter;
   }

   CRowCount() {}

   CRowCount(const CRowCount& o) 
   {
      m_vectorFiles = o.m_vectorFiles;
      m_vectorFilter = o.m_vectorFilter;
   }

   CRowCount& operator=(const CRowCount& o)
   {
      if( this != &o )
      {
         m_vectorFiles = o.m_vectorFiles;
         m_vectorFilter = o.m_vectorFilter;
      }
      return *this;
   }

   /// @brief Adds a file to the list of files to be counted
   void Add(const std::string& stringFile);
   /// @brief Adds a filter to the list of file types to be counted
   void AddFilter(const std::string& stringFilter);
   /// @brief Lists all files in a directory and adds them to the list of files to be counted
   void List(const std::string& stringDirectory);

   /// @brief Counts number of lines in a file
   int Count(const std::string& stringFile);
   /// @brief Returns the count of lines in all files
   int CountAll();

   std::vector<std::string> m_vectorFiles;   ///< holds the list of files to be line counted
   std::vector<std::string> m_vectorFilter;  ///< filters to mach against files that is to be counted
   //bool m_bCount = false;                    ///< If files have been counted or not
   //size_t m_iCount = 0;                      ///< holds the count of lines in all files
};

// TODO: PerG
// - Add a filter object with information about filters and that are able to match file names
//   The problem now is that CRowCount does many things (both filter and count) (difficulty 4)  
// - Add constructors that takes a list of files and a filter object (difficulty 3)     --
// - Add copy constructors and assignment operators (difficulty 3)                      --
// - Remove the variables m_bCount and m_iCount from CRowCount, not needed (difficulty 1)  --
// - Count enmpty lines (only spaces) (difficulty 2)
// - Count lines with only comments (difficulty 7)
// - Count lines with only code. (difficulty 8)
// - Count methods. (difficulty 8)