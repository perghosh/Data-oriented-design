#pragma once

// event_translate.h
#include <array>
#include <bitset>
#include <initializer_list>
#include <vector>

#include "window.h"

#ifndef _GD_WIN_BEGIN
#define _GD_WIN_BEGIN namespace gd_win {
#define _GD_WIN_END }
#endif

_GD_WIN_BEGIN

// @API [tag: event, id] [description: Event identifiers for portable event handling.] [jump: event__] 

/* # 
 */

enum eWindowEvent
{
   eNone = 0,

   ePaint,
   eResize,
   eMove,
   eClose,
   eDestroy,
   eCreate,

   eKeyDown,
   eKeyUp,
   eChar,

   eMouseMove,
   eMouseDown,
   eMouseUp,
   eMouseWheel,
   eMouseEnter,
   eMouseLeave,

   eFocusIn,
   eFocusOut,

   eTimer,

   eMouseDoubleClick,
   eMouseHover,
   eScroll,
   eCommand,
   eNotification,
   eContextMenu,
   eDrop,
   eHotkey,
   eWindowActivate,
   eWindowVisibility,
   eClipboard,
   eTouch,
   eGesture,
   ePointerMove,
   ePointerDown,
   ePointerUp,
   eWindowEnabled,

   e_Count
};

// @API [tag: event, translate] [description: Translate OS messages to portable event IDs. O(1).] [jump: translate__]

// Translate selected standard Windows messages below WM_USER to shared meanings.
// Private WM_USER/WM_APP messages are application-specific and are not mapped.
inline constexpr UINT uMaxMessageId = 0x400;

inline constexpr std::array<eWindowEvent, uMaxMessageId> garrayNativeToEvent = []
{
   std::array<eWindowEvent, uMaxMessageId> arrayEvents = {}; // Initialize all entries to eEventNone
   // ## Map native messages to portable events ...............................
   const auto map_message_ = [&arrayEvents](eWindowEvent ePortableEvent, std::initializer_list<UINT> initializerListMessages)
   {
      for(const UINT uNativeMessage : initializerListMessages) { arrayEvents[uNativeMessage] = ePortableEvent; }
   };

   map_message_(ePaint, { WM_PAINT });

   map_message_(eCreate, { WM_NCCREATE, WM_CREATE });
   map_message_(eDestroy, { WM_DESTROY, WM_NCDESTROY });
   map_message_(eClose, { WM_CLOSE });
   map_message_(eMove, { WM_MOVE, WM_MOVING });
   map_message_(eResize, { WM_SIZE, WM_SIZING });
   map_message_(eWindowActivate, { WM_ACTIVATE, WM_ACTIVATEAPP });
   map_message_(eWindowVisibility, { WM_SHOWWINDOW });
   map_message_(eWindowEnabled, { WM_ENABLE });

   map_message_(eFocusIn, { WM_SETFOCUS });
   map_message_(eFocusOut, { WM_KILLFOCUS });

   map_message_(eKeyDown, { WM_KEYDOWN, WM_SYSKEYDOWN });
   map_message_(eKeyUp, { WM_KEYUP, WM_SYSKEYUP });
   map_message_(eChar, { WM_CHAR, WM_SYSCHAR, WM_UNICHAR, WM_DEADCHAR, WM_SYSDEADCHAR });

   map_message_(eMouseMove, { WM_MOUSEMOVE });
   map_message_(eMouseDown, { WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN, WM_XBUTTONDOWN });
   map_message_(eMouseUp, { WM_LBUTTONUP, WM_RBUTTONUP, WM_MBUTTONUP, WM_XBUTTONUP });
   map_message_(eMouseDoubleClick, { WM_LBUTTONDBLCLK, WM_RBUTTONDBLCLK, WM_MBUTTONDBLCLK, WM_XBUTTONDBLCLK });
   map_message_(eMouseWheel, { WM_MOUSEWHEEL, WM_MOUSEHWHEEL });
   map_message_(eMouseHover, { WM_MOUSEHOVER });
   map_message_(eMouseLeave, { WM_MOUSELEAVE });

   map_message_(eScroll, { WM_HSCROLL, WM_VSCROLL });
   map_message_(eTimer, { WM_TIMER });
   map_message_(eCommand, { WM_COMMAND, WM_SYSCOMMAND });
   map_message_(eNotification, { WM_NOTIFY });
   map_message_(eContextMenu, { WM_CONTEXTMENU });
   map_message_(eDrop, { WM_DROPFILES });
   map_message_(eHotkey, { WM_HOTKEY });
   map_message_(eClipboard, { WM_CUT, WM_COPY, WM_PASTE, WM_CLEAR, WM_UNDO });

   map_message_(eTouch, { WM_TOUCH });
   map_message_(eGesture, { WM_GESTURE });
   map_message_(ePointerMove, { WM_POINTERUPDATE });
   map_message_(ePointerDown, { WM_POINTERDOWN });
   map_message_(ePointerUp, { WM_POINTERUP });

   return arrayEvents;
}();

// Translate native message id into portable event id. O(1).
inline eWindowEvent translate_s(UINT uNative)
{
   if(uNative >= uMaxMessageId) return eNone;
   return garrayNativeToEvent[uNative];
}

// @API [tag: event, map, container] [description: Manage registration of event maps.] [jump: event_map__] 

/** ---------------------------------------------------------------------------
 * @brief Register maps for windows to set what type of messages that they can respond to
 * 
~~~{.cpp}
~~~
 */
class event_map_registry 
{
public:
   /**
    * @brief Each entry contains an identifier and a bitset of supported events
    */
   struct entry
   {
      std::bitset<uMaxMessageId> get_bitset() const { return m_bitset; }        // Get the bitset that marks enabled events

      std::string m_stringId;
      std::bitset<uMaxMessageId> m_bitset;
   };

public:
   event_map_registry() = default;
   ~event_map_registry() = default;

   /// Legacy append method
   void append(const entry& entry_) { m_vectorEntry.push_back(entry_); }

   /// Add a new entry with a given identifier and bitset of supported events
   void add(const std::string& stringId, const std::bitset<uMaxMessageId>& bitset);
   void add(const std::string& stringId, std::initializer_list<std::string_view> stringEvent);


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
   std::vector<entry> m_vectorEntry; ///< Holds all registered entries, each entry contains an identifier and a bitset of supported events 

   static constexpr unsigned m_uMaxEventId_s = uMaxMessageId;  // largest WM_* message id we support, used for bitset size

private:
   /// Find index by identifier
   static int find_index_s(const event_map_registry& registry_, const std::string& stringKey);

};


/** ---------------------------------------------------------------------------
 * @brief Convert a string identifier to an event ID. O(n) where n is the number of known events.
 * @param stringId The string identifier of the event (e.g., "paint", "resize", "keydown").
 * @return The corresponding eWindowEvent value, or eWindowEventNone if the identifier is not recognized.
 */
constexpr eWindowEvent to_event_g(std::string_view stringId)
{
   // Compare stringId with known event names
   if( stringId == "paint" )        return ePaint;
   if( stringId == "resize" )       return eResize;
   if( stringId == "move" )         return eMove;
   if( stringId == "close" )        return eClose;
   if( stringId == "destroy" )      return eDestroy;
   if( stringId == "create" )       return eCreate;
   if( stringId == "keydown" )      return eKeyDown;
   if( stringId == "keyup" )        return eKeyUp;
   if( stringId == "char" )         return eChar;
   if( stringId == "mousemove" )    return eMouseMove;
   if( stringId == "mousedown" )    return eMouseDown;
   if( stringId == "mouseup" )      return eMouseUp;
   if( stringId == "mousewheel" )   return eMouseWheel;
   if( stringId == "mouseenter" )   return eMouseEnter;
   if( stringId == "mouseleave" )   return eMouseLeave;
   if( stringId == "focusin" )      return eFocusIn;
   if( stringId == "focusout" )     return eFocusOut;
   if( stringId == "timer" )        return eTimer;
   if( stringId == "mousedoubleclick" ) return eMouseDoubleClick;
   if( stringId == "mousehover" )   return eMouseHover;
   if( stringId == "scroll" )       return eScroll;
   if( stringId == "command" )      return eCommand;
   if( stringId == "notification" ) return eNotification;
   if( stringId == "contextmenu" )  return eContextMenu;
   if( stringId == "drop" )         return eDrop;
   if( stringId == "hotkey" )       return eHotkey;
   if( stringId == "windowactivate" ) return eWindowActivate;
   if( stringId == "windowvisibility" ) return eWindowVisibility;
   if( stringId == "clipboard" )    return eClipboard;
   if( stringId == "touch" )        return eTouch;
   if( stringId == "gesture" )      return eGesture;
   if( stringId == "pointermove" )  return ePointerMove;
   if( stringId == "pointerdown" )  return ePointerDown;
   if( stringId == "pointerup" )    return ePointerUp;

   return eNone;
}




_GD_WIN_END
