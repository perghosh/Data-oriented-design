#include "History.h"

void CHistory::Add(const std::string& stringFile, const std::string& stringDescription)
{
   std::filesystem::path pathFile(stringFile);

   if( std::filesystem::exists(pathFile) )
   {
      if( m_vectorList.size() < m_iCapacity )
      {
         m_vectorList.emplace_back(stringFile, stringDescription);
      }
      else
      {
         m_vectorList.erase(m_vectorList.begin());
         m_vectorList.emplace_back(stringFile, stringDescription);
      }
   }
   else
   {
      return;
   }
}

void CHistory::Write(gd::io::stream::archive& archive_)
{

   size_t iCount = m_vectorList.size();
   archive_ << iCount;

   for( int i = 0; i < iCount; i++ )
   {
      archive_ << std::get<0>(m_vectorList[i]) << std::get<1>(m_vectorList[i]);
   }
}

void CHistory::Read(gd::io::stream::archive& archive_)
{
   m_vectorList.clear();

   size_t iCount = 0;
   archive_ >> iCount;

   for( size_t i = 0; i < iCount; i++ )
   {
      std::string stringTemp, stringTemp2;
      archive_ >> stringTemp >> stringTemp2;
      m_vectorList.emplace_back(stringTemp, stringTemp2);
   }
}