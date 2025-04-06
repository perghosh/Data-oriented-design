#include "RowCount.h"


void CRowCount::Add(const std::string& stringFile)
{
   m_vectorFiles.push_back(stringFile);
}

void CRowCount::AddFilter(const std::string& stringFile)
{
   m_vectorFilter.push_back(stringFile);
}

void CRowCount::List(const std::string& stringDirectory)
{
   for( const auto& it : std::filesystem::directory_iterator(stringDirectory) )
   {
      if( it.is_regular_file() )
      {
         std::string stringFilePath = it.path().string();                      // full path to file
         std::string stringFileType = it.path().extension().string();          // .txt, .csv, .json, etc holds the file type

         if( m_vectorFilter.empty() )
         {
            m_vectorFiles.push_back(stringFilePath);
            continue;
         }
         else
         {
            for( int u = 0; u < m_vectorFilter.size(); u++ )
            {
               if( stringFileType == m_vectorFilter[u] )
               {
                  m_vectorFiles.push_back(stringFilePath);
               }
               else
               {
                  continue;
               }
            }
         }

         //m_vectorFiles.push_back(stringFilePath);
      }
   }
}

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


int CRowCount::CountAll()
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