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

string& string::assign(const char* piData, size_t ulength)
{
   if( piData == NULL )
   {
      return *this;
   }

   //delete[] m_piData;
   //m_piData = nullptr;
   allocate(ulength);

   memcpy(m_piData, piData, ulength);

   /*for( uint64_t u = 0; u < ulength; u++ )
   {
      m_piData[u] = piData[u];
   }*/

   m_piData[ulength] = '\0';
   m_uLength = ulength;

   return *this;
}

string& string::assign(const char* piData)
{
   uint64_t ulength = strlen(piData);

   assign(piData, ulength);

   return *this;
}

string& string::insert(size_t uPosition, const char* piData, size_t uLength)
{
   if( uPosition > m_uLength )
   {
      return *this;
   }
   if( piData == NULL )
   {
      return *this;
   }
   
   allocate(uLength);

   for( uint64_t u = m_uLength; u > uPosition - 1; --u )
   {
      m_piData[u + uLength] = m_piData[u];
   }

   for( uint64_t u = 0; u < uLength; u++ )
   {
      m_piData[uPosition + u] = piData[u];
   }

   m_uLength += uLength;
   m_piData[m_uLength] = '\0';

   return *this;
}

string& string::insert(size_t uPosition, const char* piData)
{
   uint64_t uLength = strlen(piData);

   insert(uPosition, piData, uLength);

   return *this;
}

string& string::replace(size_t uPosition, const char* piData, size_t uLength)
{
   // TODO: insert return statement here

   if( uPosition > m_uLength )
   {
      return *this;
   }
   if( piData == NULL )
   {
      return *this;
   }

   allocate(uLength);

   for( uint64_t u = 0; u < uLength; u++ )
   {
      m_piData[uPosition + u] = piData[u];
   }

   m_uLength = uPosition + uLength;
   m_piData[m_uLength] = '\0';

   return *this;
}

/*size_t string::size(const char* piData)
{
   const size_t uLength = strlen(piData);

   return uLength;
}*/

// ## TODO
// remove piData, replace with string
// control length and positions

string string::substr(size_t uPosition, size_t uLength)
{
   if( uPosition >= m_uLength )
   {
      return string();
   }

   if( uPosition + uLength > m_uLength )
   {
      uLength = m_uLength - uPosition;
   }

   return string(m_piData + uPosition, uLength);
}

/*
string string::substr(size_t uPosition, size_t uLength)
{
   if( uPosition >= m_uLength ) return string();                              // if position is out of bounds, return empty string

   // check length is within bounds
   if( uPosition + uLength > m_uLength ) uLength = m_uLength - uPosition;     // adjust length to fit within bounds

   return string(m_piData + uPosition, uLength);                              // create a new string from the substring
}

*/

const char* string::c_str() const
{
   if( m_piData == NULL )
   {
      return "";
   }

   return m_piData;
}

void string::clear()
{
   m_piData = nullptr;
   m_uLength = 0;
}