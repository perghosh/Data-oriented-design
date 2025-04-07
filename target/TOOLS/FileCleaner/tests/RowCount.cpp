#include "RowCount.h"


void CRowCount::Add(const std::string& stringFile)
{
   m_vectorFiles.push_back(stringFile);
}

void CRowCount::AddFilter(const std::string& stringFilter)
{
   m_vectorFilter.push_back(stringFilter);
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

int CRowCount::Count(const std::string& stringFile)
{
   std::ifstream ifstreamFile(stringFile);
   std::string stringText;
   int iCount = 0;
   while( std::getline(ifstreamFile, stringText) )
   {
      iCount++;
   }

   //std::cout << iCount << " Rows" << std::endl;

   ifstreamFile.close();

   return iCount;
}


int CRowCount::CountAll()
{
   int iCount = 0;
   
   /*for( int i = 0; i < m_vectorFiles.size(); i++ )
   {
      Count(m_vectorFiles[i]);
   }   */

   for( auto it : m_vectorFiles )
   {
      iCount += Count(it);
   }

   //bCount = false;

   return iCount;
}