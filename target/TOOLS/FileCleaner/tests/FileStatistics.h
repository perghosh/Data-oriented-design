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

class CFiles
{
public:
   CFiles() {}
   CFiles(const CFiles& o) {m_vectorFiles = o.m_vectorFiles};
   ~CFiles() {}

   void Add(const std::string& stringFile) { m_vectorFiles.push_back(stringFile); }
   std::string Get(size_t iIndex) { return m_vectorFiles[iIndex]; }
   size_t Count() { return m_vectorFiles.size(); }
   size_t Size() { return m_vectorFiles.size(); }
   bool Empty() const { return m_vectorFiles.empty(); }
   void Clear() { m_vectorFiles.clear(); }



   std::vector<std::string> m_vectorFiles;

};


class CFilter
{
public:
   CFilter() {}
   ~CFilter() {}

   void Add(const std::string& stringFile) { m_vectorFilter.push_back(stringFile); }
   void Add(std::string&& stringFile) { m_vectorFilter.push_back(std::move(stringFile)); }
   std::string Get(size_t iIndex) { return m_vectorFilter[iIndex]; }
   size_t Count() { return m_vectorFilter.size(); }
   size_t Size() { return m_vectorFilter.size(); }
   bool Empty() const { return m_vectorFilter.empty(); }
   void Clear() { m_vectorFilter.clear(); }


   std::vector<std::string> m_vectorFilter;
};

CFiles FilterFiles(const CFiles& files, const CFilter& filter)
{
   CFiles filesFiltered;
   for( const auto& file : files.m_vectorFiles )
   {
      for( const auto& filterItem : filter.m_vectorFilter )
      {
         if( file.find(filterItem) != std::string::npos )
         {
            filesFiltered.Add(file);
            break;
         }
      }
   }
   return filesFiltered;
}