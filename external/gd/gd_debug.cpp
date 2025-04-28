#include <cstdint>

#include "gd_debug.h"

_GDD_BEGIN



/** ---------------------------------------------------------------------------
 * @brief Try to find sequence within buffer range
 * @param pubuffer pointer to buffer where sequence is searched for
 * @param vectorFind vector with sequence to look for
 * @param uEnd last position in buffer to stop searching
 * @return bool true if found, false if not found
 */
bool buffer_find(const uint8_t* pubuffer, const std::vector<uint8_t>& vectorFind, size_t uEnd)
{
   const uint8_t* pubufferEnd = pubuffer;
   const uint8_t* pufind = vectorFind.data();

   const uint8_t* puposition = pubuffer;
   while(puposition < pubufferEnd)
   {
      if(*puposition == *pufind && memcmp(puposition, pufind, vectorFind.size()) == 0)
      {
         return true;
      }

      puposition++;
   }

   return false;
}

bool buffer_find(const uint8_t* pubuffer, uint32_t uFind, size_t uEnd)
{
   std::vector<uint8_t> vectorFind;
   uint8_t uAdd = (uFind >> 24) & 0xFF;
   vectorFind.push_back( uAdd );
   uAdd = (uFind >> 16) & 0xFF;
   vectorFind.push_back( uAdd );
   uAdd = (uFind >> 8) & 0xFF;
   vectorFind.push_back( uAdd );
   uAdd = uFind & 0xFF;
   vectorFind.push_back( uAdd );

   return buffer_find( pubuffer, vectorFind, uEnd );
}

_GDD_END