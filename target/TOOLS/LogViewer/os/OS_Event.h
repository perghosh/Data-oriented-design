// event_translate.h

namespace ui {

   enum eEvent : uint16_t
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

      eEventCount
   };

   // Dense translation table. Index is the native WM_* id, value is our eEvent.
   // Range 0x0000-0x03FF covers all WM_* messages this demo maps.
   // Anything outside the range falls through to eEventNone.
   inline constexpr UINT kNativeMax = 0x400;

   inline constexpr std::array<eEvent, kNativeMax> gaNativeToEvent = []
      {
         std::array<eEvent, kNativeMax> a = {};    // all eEventNone by default
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
   inline eEvent Translate_s(UINT uNative)
   {
      if(uNative >= kNativeMax) return eEventNone;
      return gaNativeToEvent[uNative];
   }

} // namespace ui
