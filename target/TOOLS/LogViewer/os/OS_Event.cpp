#include "OS_Event.h"

_GD_WIN_BEGIN


void registry::append(const entry& entry_)
{
   m_vectorEntry.push_back(entry_);
}

#include "OS_Event.h"

#include "OS_Event.h"



/// Insert or update an entry by identifier ----------------------------------- insert
std::pair<std::vector<registry::entry>::iterator, bool> registry::insert( const std::string& stringKey, const entry& entry_)
{
   int iIndex = find_index_s(*this, stringKey);
   if(iIndex != -1)
   {
      // Update existing entry
      m_vectorEntry[iIndex] = entry_;
      return { m_vectorEntry.begin() + iIndex, false };
   }

   // Add new entry
   m_vectorEntry.push_back(entry_);
   return { std::prev(m_vectorEntry.end()), true };
}

/// Find entry by identifier -------------------------------------------------- find
std::vector<registry::entry>::iterator registry::find(const std::string& stringKey)
{
   int iIndex = find_index_s(*this, stringKey);
   return (iIndex != -1) ? (m_vectorEntry.begin() + iIndex) : m_vectorEntry.end();
}

std::vector<registry::entry>::const_iterator registry::find(const std::string& stringKey) const
{
   int iIndex = find_index_s(*this, stringKey);
   return (iIndex != -1) ? (m_vectorEntry.begin() + iIndex) : m_vectorEntry.end();
}

/// Check if entry exists ----------------------------------------------------- contains
bool registry::contains(const std::string& stringKey) const
{
   return find_index_s(*this, stringKey) != -1;
}

/// Remove entry by identifier ------------------------------------------------ erase
size_t registry::erase(const std::string& stringKey)
{
   int iIndex = find_index_s(*this, stringKey);
   if(iIndex != -1)
   {
      m_vectorEntry.erase(m_vectorEntry.begin() + iIndex);
      return 1;
   }
   return 0;
}

/// Find index by identifier ------------------------------------------------- find_index_s
int registry::find_index_s(const registry& registry_, const std::string& stringKey)
{
   for(size_t uIndex = 0; uIndex < registry_.m_vectorEntry.size(); ++uIndex)
   {
      if(registry_.m_vectorEntry[uIndex].m_stringId == stringKey)
      {
         return static_cast<int>(uIndex);
      }
   }
   return -1;
}


_GD_WIN_END
