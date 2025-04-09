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

class CList
{
public:

   CList(std::vector<std::string>& vectorFilter)
   {
      m_vectorFilter = vectorFilter;
   }

   CList() {}

   CList(const CList& o)
   {
      m_vectorFiles = o.m_vectorFiles;
      m_vectorFilter = o.m_vectorFilter;
   }

   void AddFilter(const std::string& stringFilter);

   void Sort(const std::string& stringDirectory);

   std::vector<std::string> GetFiles() { return m_vectorFiles; }

   std::vector<std::string> m_vectorFiles;
   std::vector<std::string> m_vectorFilter;
};