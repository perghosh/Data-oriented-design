// @FILE [tag: window] [description: Window class for portable window handling windows] [name: gdwin_window.h] [type: header] 

#pragma once

/**
 * @file gdwin_window.h
 * @brief Window class for portable window handling on Windows.
 *
 *    lresult__               Result object
 *    window__                Window class
 */

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif


#include <bitset>
#include <expected>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>

#include <windows.h>
#include <windowsx.h>

#include "gd/gd_arguments.h"

#include "gdwin_event.h"

#ifndef _GDWIN_BEGIN
#define _GDWIN_BEGIN namespace gdwin {
#define _GDWIN_END }
#endif

_GDWIN_BEGIN
std::expected<bool, std::string> register_window_g( const gd::argument::arguments& argumentsWindow, std::string_view stringClass);
bool create_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass, std::string_view stringTitle, int iWidth, int iHeight, DWORD uStyle = WS_OVERLAPPEDWINDOW, HWND hwndParent = nullptr);
void crack_message_s(gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut);
_GDWIN_END

_GDWIN_BEGIN

/** @API [tag: result] [summary: The lresult struct represents the result of an event handling operation.] [jump: lresult__]
 * @brief The lresult struct represents the result of an event handling operation in a windowing framework. It encapsulates the outcome of an event processing function, allowing for clear communication of whether the event was handled, not handled, or resulted in an error.
 */
struct lresult
{
   /// Three outcomes an OnEvent override can produce.
   enum class enumState
   {
      eHandled,     ///< we processed it, value is the LRESULT to return to Windows
      eNotHandled, ///< we decline it, framework should call DefWindowProc
      eError,       ///< we failed, framework may call OnGetError for details
   };

   // ---------------------------------------------------------------------
   // Constructors / factories
   // ---------------------------------------------------------------------

   constexpr lresult() = default;

   /// "I handled this, and the LRESULT to return is `value`."
   static constexpr lresult handled(int64_t iValue = 0) noexcept { return lresult{ enumState::eHandled, iValue }; }

   /// "I didn't handle this — run DefWindowProc."
   static constexpr lresult not_handled() noexcept { return lresult{ enumState::eNotHandled, 0 }; }

   /// "I failed — the framework may call OnGetError for details."
   static constexpr lresult error() noexcept { return lresult{ enumState::eError, 0 }; }

   [[nodiscard]] constexpr bool is_handled()     const noexcept { return m_eState == enumState::eHandled; }
   [[nodiscard]] constexpr bool is_not_handled() const noexcept { return m_eState == enumState::eNotHandled; }
   [[nodiscard]] constexpr bool is_error()       const noexcept { return m_eState == enumState::eError; }

   /// The LRESULT to return to Windows. Only meaningful when is_handled().
   [[nodiscard]] constexpr int64_t value() const noexcept { return m_iValue; }

   // ---------------------------------------------------------------------
   // Conversion to/from the raw int64_t OnEvent return
   // ---------------------------------------------------------------------

   /// Map this result to the raw int64_t the framework expects.
   [[nodiscard]] constexpr int64_t as_int64() const noexcept
   {
      switch(m_eState)
      {
      case enumState::eHandled:     return m_iValue;
      case enumState::eNotHandled:  return m_iNotHandled_s;
      case enumState::eError:       return m_iError_s;
      }
      return m_iNotHandled_s; // unreachable, silences warnings
   }

   /// Decode a raw int64_t returned by OnEvent into an lresult.
   [[nodiscard]] static constexpr lresult from_int64(int64_t iRaw) noexcept
   {
      if(iRaw == m_iNotHandled_s) return not_handled();
      if(iRaw == m_iError_s)      return error();
      return handled(iRaw);
   }

   enumState m_eState = enumState::eNotHandled; ///< default to not handled, so uninitialized lresult is safe
   int64_t m_iValue = 0; ///< meaningful only when m_state == enumState::eHandled

   // Reserved sentinels. Chosen outside any plausible LRESULT value.
   // INT64_MIN and INT64_MIN+1 are safe on every platform we target.
   static constexpr int64_t m_iNotHandled_s = INT64_MIN;
   static constexpr int64_t m_iError_s = INT64_MIN + 1;

private:
   constexpr lresult(enumState s, int64_t v) noexcept : m_eState(s), m_iValue(v) {}
};

_GDWIN_END

_GDWIN_BEGIN

// ## CWindow -------------------------------------------------------------------------------------

/** @API [tag: window] [summary: The window class represents a portable window for handling events on Windows.] [jump: window__]
 * @brief The window class represents a portable window for handling events on Windows. It provides mechanisms for creating, displaying, and managing a window, as well as handling messages and events in a structured manner.
 */
template<typename EVENT_RESULT = lresult>
class window
{
public:
   using handler = std::function<LRESULT(WPARAM, LPARAM)>;

   window() = default;
   window(const window&) = delete;            // the HWND and the bound handlers point at this object
   window& operator=(const window&) = delete;
   virtual ~window()
   {
      if(m_hwnd != nullptr)
      {
         SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, 0); // detach first, derived part is already gone
         DestroyWindow(m_hwnd);
      }
   }

   virtual EVENT_RESULT on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage) { return EVENT_RESULT::not_handled(); }
   virtual bool on_get_error( gd::argument::arguments& argumentsError ) const { return false; }
   virtual std::pair<bool, std::string_view> get_window_class() const { return { true, std::string_view() }; }
   virtual void* get_callback() { return (void*)nullptr; }

   void bind(UINT uMessage);

   /// Create the window, message bindings should be done before this call so WM_CREATE is seen
   bool create(const std::string& stringTitle, gd::argument::arguments argumentsWindow);

   HWND GetHandle() const { return m_hwnd; }
   void Show(int iCommand = SW_SHOW) { ShowWindow(m_hwnd, iCommand); UpdateWindow(m_hwnd); }
   void Invalidate(bool bErase = true) { InvalidateRect(m_hwnd, nullptr, bErase ? TRUE : FALSE); }


   LRESULT Call(UINT uMessage, WPARAM uParam, LPARAM iParam);

   /// Call from a handler when default processing should run as well
   LRESULT CallDefault(UINT uMessage, WPARAM uParam, LPARAM iParam) const { return DefWindowProcW(m_hwnd, uMessage, uParam, iParam); }

   /// Message loop, returns the exit code from PostQuitMessage
   static int Run_s()
   {
      MSG msg = {};
      while(GetMessageW(&msg, nullptr, 0, 0) > 0)
      {
         TranslateMessage(&msg);
         DispatchMessageW(&msg);
      }
      return static_cast<int>(msg.wParam);
   }

private:
   LRESULT dispatch(UINT uMessage, WPARAM uParam, LPARAM iParam)
   {
      auto eEvent = gdwin::translate_s(uMessage); // translate to portable event, ignored here but useful for debugging
      if(eEvent != gdwin::eNone)
      {
         if (m_bitsetMessageMap.test(eEvent) == true)
         {
            return Call(uMessage, uParam, iParam);
         }
      }

      return CallDefault(uMessage, uParam, iParam);
   }


   /// The one and only window procedure, finds the window for the HWND and forwards
   static LRESULT CALLBACK WindowProc_s(HWND hwnd, UINT uMessage, WPARAM uParam, LPARAM iParam);

   static void CrackMessage_s( gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut);


private:
   std::bitset<100> m_bitsetMessageMap{};
   HWND m_hwnd = nullptr;
};

/// @brief Bind a native message to this window, so that it will be dispatched to OnEvent. This is for debugging and testing, as the framework will only call OnEvent for messages that have been bound.
template <typename EVENT_RESULT>
inline void window<EVENT_RESULT>::bind(UINT uMessage)
{                                                                                                  assert(uMessage < gdwin::uMaxMessageId);
   m_bitsetMessageMap.set(uMessage);                                           // for debugging, see dispatch()
}

template <typename EVENT_RESULT>
bool window<EVENT_RESULT>::create(const std::string& stringTitle, gd::argument::arguments argumentsWindow)
{
   HINSTANCE hinstance = GetModuleHandleW(nullptr);

   auto [bRegistered, stringClass] = get_window_class();
   if(bRegistered == false && stringClass.empty() == false)
   {
      auto pfnCallback = get_callback();
      gd::argument::arguments argumentsRegister;
      argumentsRegister["instance"] = (void*)hinstance;
      argumentsRegister["class"] = stringClass.data();
      argumentsRegister["callback"] = pfnCallback;
      if(register_window_g(argumentsRegister, stringClass).has_value() == false)
      {
         return false;
      }
   }

   std::array<std::byte, 128> buffer_{};
   gd::argument::arguments argumentsCreate(buffer_);
   int iX = argumentsWindow["x"].as_int();
   int iY = argumentsWindow["y"].as_int();
   int iWidth = argumentsWindow["width"].as_int();
   int iHeight = argumentsWindow["height"].as_int();

   return create_window_g(argumentsWindow, stringClass, stringTitle, iX, iY, iWidth, iHeight);
}




_GDWIN_END
