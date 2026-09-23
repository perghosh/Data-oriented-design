#pragma once

// event_translate.h
#include <array>
#include <bitset>
#include <vector>

#include "window.h"

#ifndef _GD_WIN_BEGIN
#define _GD_WIN_BEGIN namespace gd_win {
#define _GD_WIN_END }
#endif

_GD_WIN_BEGIN

// @API [tag: event, id] [description: Event identifiers for portable event handling.] [jump: event__] 

enum eEvent
{
   eEventNone = 0,

   eEventPaint,
   eEventResize,
   eEventMove,
   eEventClose,
   eEventDestroy,
   eEventCreate,

   eEventKeyDown,
   eEventKeyUp,
   eEventChar,

   eEventMouseMove,
   eEventMouseDown,
   eEventMouseUp,
   eEventMouseWheel,
   eEventMouseEnter,
   eEventMouseLeave,

   eEventFocusIn,
   eEventFocusOut,

   eEventTimer,

   eEvent_Count
};

// @API [tag: event, translate] [description: Translate native message id into portable event id. O(1).] [jump: translate__] 

// Dense translation table. Index is the native WM_* id, value is our eEvent.
// Range 0x0000-0x03FF covers all WM_* messages this demo maps.
// Anything outside the range falls through to eEventNone.
inline constexpr UINT uMaxMessageId = 0x400;

inline constexpr std::array<eEvent, uMaxMessageId> gaNativeToEvent = []
   {
      std::array<eEvent, uMaxMessageId> a = {};    // all eEventNone by default
      a[WM_PAINT] = eEventPaint;
      a[WM_SIZE] = eEventResize;
      a[WM_MOVE] = eEventMove;
      a[WM_CLOSE] = eEventClose;
      a[WM_DESTROY] = eEventDestroy;
      a[WM_CREATE] = eEventCreate;
      a[WM_KEYDOWN] = eEventKeyDown;
      a[WM_KEYUP] = eEventKeyUp;
      a[WM_CHAR] = eEventChar;
      a[WM_MOUSEMOVE] = eEventMouseMove;
      a[WM_LBUTTONDOWN] = eEventMouseDown;
      a[WM_LBUTTONUP] = eEventMouseUp;
      a[WM_MOUSEWHEEL] = eEventMouseWheel;
      a[WM_MOUSEHOVER] = eEventMouseEnter;
      a[WM_MOUSELEAVE] = eEventMouseLeave;
      a[WM_SETFOCUS] = eEventFocusIn;
      a[WM_KILLFOCUS] = eEventFocusOut;
      a[WM_TIMER] = eEventTimer;
      return a;
   }();

// Translate native message id into portable event id. O(1).
inline eEvent translate_s(UINT uNative)
{
   if(uNative >= uMaxMessageId) return eEventNone;
   return gaNativeToEvent[uNative];
}

// @API [tag: event, map, container] [description: Manage registration of event maps.] [jump: event_map__] 

class registry
{
   static constexpr unsigned m_uMaxEventId_s = 0x400;

   struct entry
   {
      std::string m_stringId;
      std::bitset<m_uMaxEventId_s> m_bitset;
   };

public:
   registry() = default;
   ~registry() = default;

   /// Legacy append method
   void append(const entry& entry_) { m_vectorEntry.push_back(entry_); }

   /// Insert or update an entry by identifier
   std::pair<std::vector<entry>::iterator, bool> insert(const std::string& stringKey, const entry& entry_);

   /// Find entry by identifier
   std::vector<entry>::iterator find(const std::string& stringKey);
   std::vector<entry>::const_iterator find(const std::string& stringKey) const;

   /// Check if entry exists
   bool contains(const std::string& stringKey) const;

   /// Remove entry by identifier, returns number of entries removed
   size_t erase(const std::string& stringKey);

   /// Remove all entries
   void clear() { m_vectorEntry.clear(); }

   /// Get number of entries
   size_t size() const { return m_vectorEntry.size(); }

   /// Check if registry is empty
   bool empty() const { return m_vectorEntry.empty(); }

   /// Iterator support
   std::vector<entry>::iterator begin() { return m_vectorEntry.begin(); }
   std::vector<entry>::const_iterator begin() const { return m_vectorEntry.begin(); }
   std::vector<entry>::iterator end() { return m_vectorEntry.end(); }
   std::vector<entry>::const_iterator end() const { return m_vectorEntry.end(); }


public:
   std::vector<entry> m_vectorEntry;

private:
   /// Find index by identifier
   static int find_index_s(const registry& registry_, const std::string& stringKey);

};



_GD_WIN_END
