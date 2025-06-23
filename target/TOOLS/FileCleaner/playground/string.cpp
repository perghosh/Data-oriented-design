#include "string.h"

void string::allocate(const uint64_t uLength)
{
   if( (uLength + m_uLength) > m_uBufferSize )
   {
      uint64_t uNewSize = m_uLength + uLength;
      uNewSize += (uNewSize >> 1);

      m_uBufferSize = uNewSize;
      char* piData = new char[m_uBufferSize];   
      
      if( m_piData != NULL )
      {
         memcpy( piData, m_piData, m_uLength + 1 );  
      }
                            // copy from old buffer to new buffer
      delete [] m_piData;                                                      // delete old buffer
      m_piData = piData;                                                       // set to new buffer
   }
}

string& string::append(const char* piData)
{
   uint64_t uLength = strlen(piData);

   allocate(uLength);

   char* piInsert = m_piData + m_uLength;
   for( uint64_t uPosition = 0; uPosition < uLength; uPosition++ )
   {
      piInsert[uPosition] = piData[uPosition];
   }

   piInsert[uLength] = '\0';
   m_uLength += uLength;
   return *this;
}

string& string::append(const char* piData, size_t uLength)
{
   // TODO: insert return statement here

   allocate(uLength);

   char* piInsert = m_piData + m_uLength;
   for( uint64_t uPosition = 0; uPosition < uLength; uPosition++ )
   {
      piInsert[uPosition] = piData[uPosition];
   }

   piInsert[uLength] = '\0';
   m_uLength += uLength;
   return *this;
}

/*size_t string::size(const char* piData)
{
   const size_t uLength = strlen(piData);

   return uLength;
}*/

string string::substr(size_t uPosition, size_t uLength)
{
   char* piData = new char[uLength + 1];

   for( int i = 0; i < uLength; i++ )
   {
      piData[i] = m_piData[uPosition + i];
   }

   piData[uLength] = '\0'; 

   string stringResult(piData);
   delete[] piData;

   //delete[] m_piData;
   //append(piData);
   
   return stringResult;
}