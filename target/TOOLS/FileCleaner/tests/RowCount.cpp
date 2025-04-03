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

int CRowCount::Count_all()
{
   m_bCount = true;
   
   for( int i = 0; i < m_vectorFiles.size(); i++ )
   {
      Count(m_vectorFiles[i]);
   }

   return m_iCount;

   m_bCount = false;
   m_iCount = 0;

}