#include "RowCount.h"

void CRowCount::Count(const std::string& stringFile)
{
   std::ifstream ifstreamFile(stringFile);
   std::string stringText;
   int iCount = 0;
   while( std::getline(ifstreamFile, stringText) )
   {
      iCount++;
   }

   std::cout << iCount << " Rows" << std::endl;

   ifstreamFile.close();

   if( m_bCount == true )
   {
      m_iCount += iCount;
   }
}

void CRowCount::Add(const std::string& stringFile)
{
   m_vectorFiles.push_back(stringFile);
}

void CRowCount::List(const std::string& stringDirectory)
{
   for( const auto& it : std::filesystem::directory_iterator(stringDirectory) )
   {
      if( it.is_regular_file() )
      {
         m_vectorFiles.push_back(it.path().string());
      }
   }
}

int CRowCount::Count_all()
{
   m_iCount = 0;
   m_bCount = true;
   
   /*for( int i = 0; i < m_vectorFiles.size(); i++ )
   {
      Count(m_vectorFiles[i]);
   }   */

   for( auto it : m_vectorFiles )
   {
      Count(it);
   }

   m_bCount = false;

   return m_iCount;
}