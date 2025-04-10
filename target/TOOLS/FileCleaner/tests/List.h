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

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CFiles 
{
   // ## construction -------------------------------------------------------------
public:
   CFiles() {}
   // copy
   CFiles(const CFiles& o) { common_construct(o); }
   CFiles(CFiles&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CFiles& operator=(const CFiles& o) { common_construct(o); return *this; }
   CFiles& operator=(CFiles&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CFiles() {}
private:
   // common copy
   void common_construct(const CFiles& o) {}
   void common_construct(CFiles&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   void Add(const std::string& stringFile) { m_vectorFiles.push_back(stringFile); }
   std::string Get(size_t iIndex) { return m_vectorFiles[iIndex]; }
   size_t Count() { return m_vectorFiles.size(); }
   size_t Size() { return m_vectorFiles.size(); }
   bool Empty() const { return m_vectorFiles.empty(); }
   void Clear() { m_vectorFiles.clear(); }


//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   std::vector<std::string> m_vectorFiles;

// ## free functions ------------------------------------------------------------
public:



};

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CFilter
{
   // ## construction -------------------------------------------------------------
public:
   CFilter() {}
   // copy
   CFilter(const CFilter& o) { common_construct(o); }
   CFilter(CFilter&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CFilter& operator=(const CFilter& o) { common_construct(o); return *this; }
   CFilter& operator=(CFilter&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CFilter() {}
private:
   // common copy
   void common_construct(const CFilter& o) { m_vectorFilter = o.m_vectorFilter; }
   void common_construct(CFilter&& o) noexcept { m_vectorFilter = std::move(o.m_vectorFilter); }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   void Add(const std::string& stringFile) { m_vectorFilter.push_back(stringFile); }
   void Add(std::string&& stringFile) { m_vectorFilter.push_back( std::move( stringFile ) ); }
   std::string Get(size_t iIndex) { return m_vectorFilter[iIndex]; }
   size_t Count() { return m_vectorFilter.size(); }
   size_t Size() { return m_vectorFilter.size(); }
   bool Empty() const { return m_vectorFilter.empty(); }
   void Clear() { m_vectorFilter.clear(); }



//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   std::vector<std::string> m_vectorFilter;

// ## free functions ------------------------------------------------------------
public:



};


void test() 
{
   CFilter  filter;
   std::string stringFile = "*.txt";
   filter.Add( std::move( stringFile ) );
   if( stringFile.empty() == true )
   {
      std::cout << "stringFile is empty" << std::endl;
   }
   else
   {
      std::cout << "stringFile is not empty" << std::endl;
   }
}


CFiles FilterFiles( const CFiles& files, const CFilter& filter )
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

uint64_t CountLines(const CFiles& files)
{
   uint64_t count = 0;
   for( const auto& file : files.m_vectorFiles )
   {
      std::ifstream inFile(file);
      if( inFile.is_open() )
      {
         std::string line;
         while( std::getline(inFile, line) )
         {
            ++count;
         }
         inFile.close();
      }
   }
   return count;
}
