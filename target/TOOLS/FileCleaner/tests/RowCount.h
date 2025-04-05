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

#include "../Document.h"
#include "../Application.h"

#include "catch2/catch_amalgamated.hpp"

class CRowCount
{
public:

   void Count(const std::string& stringFile);

   void Add(const std::string& stringFile);
   void List(const std::string& stringDirectory);

   int Count_all();

   std::vector<std::string> m_vectorFiles;
   bool m_bCount = false;
   size_t m_iCount = 0;

};