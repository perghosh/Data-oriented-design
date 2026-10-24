#include "gdwin_event.h"

_GDWIN_BEGIN

void event_registry::add(const std::string& stringId, std::initializer_list<std::string_view> stringEvent)
{
   std::bitset<uMaxMessageId> bitset_;
   for(const auto& eventName : stringEvent)
   {
      eWindowEvent eId = to_event_g(eventName.data());
      if(eId != eWindowEvent::eNone)
      {
         bitset_.set(static_cast<size_t>(eId));
      }
   }
   add(stringId, bitset_);
}

/// Insert or update an entry by identifier ----------------------------------- insert
std::pair<std::vector<event_registry::entry>::iterator, bool> event_registry::insert( const std::string& stringKey, const entry& entry_)
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
std::vector<event_registry::entry>::iterator event_registry::find(const std::string& stringKey)
{
   int iIndex = find_index_s(*this, stringKey);
   return (iIndex != -1) ? (m_vectorEntry.begin() + iIndex) : m_vectorEntry.end();
}

std::vector<event_registry::entry>::const_iterator event_registry::find(const std::string& stringKey) const
{
   int iIndex = find_index_s(*this, stringKey);
   return (iIndex != -1) ? (m_vectorEntry.begin() + iIndex) : m_vectorEntry.end();
}

/// Check if entry exists ----------------------------------------------------- contains
bool event_registry::contains(const std::string& stringKey) const
{
   return find_index_s(*this, stringKey) != -1;
}

/// Remove entry by identifier ------------------------------------------------ erase
size_t event_registry::erase(const std::string& stringKey)
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
int event_registry::find_index_s(const event_registry& registry_, const std::string& stringKey)
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


_GDWIN_END
