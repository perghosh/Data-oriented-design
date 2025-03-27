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

   void Add(std::string stringFile)
   {
      m_vectorList.push_back(stringFile);
   }
   
   void Write(gd::io::stream::archive& archive_)
   {
      
      size_t iCount = m_vectorList.size();
      archive_ << iCount;

      for( int i = 0; i < iCount; i++ )
      {
         archive_ << m_vectorList[i];
      }
   }

   void Read(gd::io::stream::archive& archive_)
   {
      m_vectorList.clear();

      size_t iCount = 0;
      archive_ >> iCount;

      for( int i = 0; i < iCount; i++ )
      {
         std::string stringTemp;
         archive_.read(stringTemp);
         m_vectorList.push_back(stringTemp);
      }
   }

   std::vector<std::string> m_vectorList;

};