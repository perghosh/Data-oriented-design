#include "List.h"

void CList::AddFilter(const std::string& stringFilter)
{
   m_vectorFilter.push_back(stringFilter);
}

void CList::Sort(const std::string& stringDirectory)
{
   for( const auto& it : std::filesystem::directory_iterator(stringDirectory) )
   {
      if( it.is_regular_file() )
      {
         std::string stringFilePath = it.path().string();
         std::string stringFileType = it.path().extension().string();

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
      }
   }
}