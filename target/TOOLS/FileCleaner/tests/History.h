#pragma once

#include <fstream>


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

class CHistory
{
public:

   void Add(const std::string& stringFile, const std::string& stringDescription);

   void Write(gd::io::stream::archive& archive_);

   void Read(gd::io::stream::archive& archive_);

   std::vector<std::tuple<std::string, std::string>> m_vectorList;
   int m_iCapacity = 10;

};