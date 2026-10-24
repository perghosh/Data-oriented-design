// @FILE [tag: window] [description: Window class for portable window handling windows] [name: gdwin_window.h] [type: header] 

#pragma once

/**
 * @file gdwin_window.h
 * @brief Window class for portable window handling on Windows.
 *
 *    attributes__            Attributes of the lresult struct
 *    construct__             Constructors and factories
 *    conversion__            Conversion to/from the raw int64_t OnEvent return
 *    getter_setter__         Getter and setter functions for lresult
 *    window__                Window class
 *    on_event__              Main event handler
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

#include "config/gdwin_config_return.h"

#include "gdwin_event.h"


#ifndef _GDWIN_BEGIN
#define _GDWIN_BEGIN namespace gdwin {
#define _GDWIN_END }
#endif

_GDWIN_BEGIN
int run_s();
std::expected<bool, std::string> register_window_g( const gd::argument::arguments& argumentsWindow, std::string_view stringClass);
std::expected<bool, std::string> register_window_g(const gd::argument::arguments& argumentsWindow);
bool create_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass, std::string_view stringTitle, int iX, int iY, int iWidth, int iHeight, uint64_t uStyle, HWND hwndParent = nullptr, void* puserdata_ = nullptr);
void crack_message_s(gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsMessage);
_GDWIN_END




_GDWIN_BEGIN

/** ----------------------------------------------------------------------------
 * @brief The window class represents a portable window for handling events on Windows. It provides mechanisms for creating, displaying, and managing a window, as well as handling messages and events in a structured manner.
 * @API [tag: window] [summary: The window class represents a portable window for handling events on Windows.] [jump: window__]
 */
template<typename EVENT_RESULT = lresult>
class window
{
public:
   using handler = std::function<LRESULT(WPARAM, LPARAM)>;

   window() = default;
   window(const window&) = delete;            // the HWND and the bound handlers point at this object
   window& operator=(const window&) = delete;
   virtual ~window();

/// @API [tag: event] [summary: The on_event function is a virtual function that handles window events.] [jump: on_event__]
   virtual EVENT_RESULT on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage) { return lresult(lresult::enumState::eNotHandled, 0 ); }
   virtual bool on_get_error( gd::argument::arguments& argumentsError ) const { return false; }
   virtual std::pair<bool, std::string_view> get_window_class() const { return { true, std::string_view() }; }
   virtual void* get_callback() { return (void*)nullptr; }

   void bind(UINT uMessage);

   /// Create the window, message bindings should be done before this call so WM_CREATE is seen
   bool create(const std::string& stringTitle, gd::argument::arguments argumentsWindow);

   HWND get_handle() const { return m_hwnd; }
   void show(int iCommand = SW_SHOW) { ShowWindow(m_hwnd, iCommand); UpdateWindow(m_hwnd); }
   void invalidate(bool bErase = true) { InvalidateRect(m_hwnd, nullptr, bErase ? TRUE : FALSE); }

   EVENT_RESULT call(UINT uMessage, WPARAM uParam, LPARAM iParam);

   /// Call from a handler when default processing should run as well
   EVENT_RESULT call_default(UINT uMessage, WPARAM uParam, LPARAM iParam) const;

   EVENT_RESULT dispatch(UINT uMessage, WPARAM uParam, LPARAM iParam);


public:
   static void* get_callback_s() { return (void*)window_procedure_s; }

private:
   /// The one and only window procedure, finds the window for the HWND and forwards
   static LRESULT CALLBACK window_procedure_s(HWND hwnd, UINT uMessage, WPARAM uParam, LPARAM iParam);
   /// Crack a native message into a portable event and arguments. This is for debugging and testing, as the framework will only call OnEvent for messages that have been bound.
   static void crack_message_s( gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsMessage);


public:
   std::bitset<100> m_bitsetMessageMap{};
   HWND m_hwnd = nullptr;
};

/// Destructor for the window class, ensuring that the associated HWND is properly destroyed and detached from the window object.
template <typename EVENT_RESULT>
window<EVENT_RESULT>::~window()
{
   if(m_hwnd != nullptr)
   {
      SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, 0); // detach first, derived part is already gone
      DestroyWindow(m_hwnd);
   }
}

/// @brief Bind a native message to this window, so that it will be dispatched to OnEvent. This is for debugging and testing, as the framework will only call OnEvent for messages that have been bound.
template <typename EVENT_RESULT>
inline void window<EVENT_RESULT>::bind(UINT uMessage)
{                                                                                                  assert(uMessage < gdwin::uMaxMessageId);
   m_bitsetMessageMap.set(uMessage);                                           // for debugging, see dispatch()
}

/** --------------------------------------------------------------------------- create
 * @brief Create window instance
 * 
 * Creates a native window instance, registering the window class first if needed, and applies position, size, and style settings from the provided arguments.
 * 
 * @tparam EVENT_RESULT The event-result type associated with the window specialization.
 * @param stringTitle The window title text.
 * @param argumentsWindow A collection of window creation options, including style (string or numeric), x, y, width, and height, plus any additional values needed by the underlying window creation call.
 * @return true if the window class is available/registered as needed and the window is created successfully; otherwise false.
 */
template <typename EVENT_RESULT>
bool window<EVENT_RESULT>::create(const std::string& stringTitle, gd::argument::arguments argumentsWindow)
{
#if defined(UNICODE) || defined(_UNICODE)
   HINSTANCE hinstance = GetModuleHandleW(nullptr);
#else
   HINSTANCE hinstance = GetModuleHandleA(nullptr);
#endif

   auto [bRegistered, stringClass] = get_window_class();
   if(bRegistered == false && stringClass.empty() == false)
   {
      auto pfnCallback = get_callback_s();
      gd::argument::arguments argumentsRegister;
      argumentsRegister["instance"] = (void*)hinstance;
      argumentsRegister["class"] = stringClass.data();
      argumentsRegister["callback"] = pfnCallback;
      if(register_window_g(argumentsRegister, stringClass).has_value() == false)
      {
         return false;
      }
   }

   uint32_t uStyle = 0;
   auto argumentsStyle = argumentsWindow["style"];
   if (argumentsStyle.is_string() == true)                                     // value are passed as string, so we need to convert them to valid style values
   {
      std::string_view stringStyle = argumentsStyle.as_string_view();
      if(stringStyle == "popup") { uStyle = WS_POPUP; }
      else if(stringStyle == "child") { uStyle = WS_CHILD; }
      else if(stringStyle == "minimized") { uStyle = WS_MINIMIZE; }
      else if(stringStyle == "maximized") { uStyle = WS_MAXIMIZE; }
      else if(stringStyle == "disabled") { uStyle = WS_DISABLED; }
      else if(stringStyle == "visible") { uStyle = WS_VISIBLE; }
      else if(stringStyle == "clipchildren") { uStyle = WS_CLIPCHILDREN; }
      else if(stringStyle == "clipsiblings") { uStyle = WS_CLIPSIBLINGS; }
      else if(stringStyle == "overlappedwindow") { uStyle = WS_OVERLAPPEDWINDOW; }
      else if(stringStyle == "popupwindow") { uStyle = WS_POPUPWINDOW; }
      else if(stringStyle == "childwindow") { uStyle = WS_CHILDWINDOW; }
   }
   else
   {
      uStyle = argumentsStyle.as_uint();                                         // value are passed as integer, not string and this neeed to be a valid style value, otherwise the window will not be created
   }

   
   // ## If the style includes a system menu or caption, check for menu argument 
   if( (uStyle & (WS_SYSMENU | WS_CAPTION)) != 0)
   {
      HMENU hMenu = nullptr;
      hMenu = (HMENU)argumentsWindow["menu"].as_pointer();
      if(hMenu != nullptr) { argumentsWindow.append("menu", hMenu); }
   }

   int iX = argumentsWindow["x"].as_int();
   int iY = argumentsWindow["y"].as_int();
   int iWidth = argumentsWindow["width"].as_int();
   int iHeight = argumentsWindow["height"].as_int();

   return create_window_g(argumentsWindow, stringClass, stringTitle, iX, iY, iWidth, iHeight, uStyle, nullptr, this);
}

/** --------------------------------------------------------------------------- call
 * @brief Translates a window message into an internal event
 * 
 * Builds its argument payload, dispatches it to the event handler, and returns the handler result. 
 *
 * @tparam EVENT_RESULT The return type produced by the window event handler.
 * @param uMessage The Windows message identifier to process.
 * @param uParam The WPARAM value associated with the message.
 * @param iParam The LPARAM value associated with the message.
 * @return The value returned by on_event for the translated event.
 */
template <typename EVENT_RESULT>
EVENT_RESULT window<EVENT_RESULT>::call(UINT uMessage, WPARAM uParam, LPARAM iParam)
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsMessage(buffer_);
   auto eEvent = gdwin::translate_s(uMessage);
   crack_message_s(eEvent, uParam, iParam, argumentsMessage);
   auto result_ = on_event(eEvent, &argumentsMessage, (uint64_t)uMessage);
   return result_;
}

/** --------------------------------------------------------------------------- call_default
 * @brief Invokes the platform default window procedure for the current window and converts the native result to the function's event result type. 
 * @tparam EVENT_RESULT The logical event result type to return after converting from the native DefWindowProc result.
 * @param uMessage The Windows message identifier to process.
 * @param uParam The message-specific WPARAM value.
 * @param iParam The message-specific LPARAM value.
 * @return The default-processing result converted from the native LRESULT to EVENT_RESULT via event_result_traits<EVENT_RESULT>::from_native.
 */
template <typename EVENT_RESULT>
EVENT_RESULT window<EVENT_RESULT>::call_default(UINT uMessage, WPARAM uParam, LPARAM iParam) const
{ 
#if defined(UNICODE) || defined(_UNICODE)
   auto lresult_ = ::DefWindowProcW(m_hwnd, uMessage, uParam, iParam);
#else
   auto lresult_ = ::DefWindowProcA(m_hwnd, uMessage, uParam, iParam);
#endif
   return event_result_traits<EVENT_RESULT>::from_native(lresult_);
}

/**
 * @brief Dispatches a window message to a registered event handler when available; otherwise falls back to the default message handling. 
 * @tparam EVENT_RESULT The result type returned by both the custom handler and the default handler.
 * @param uMessage The native window message identifier to dispatch.
 * @param uParam The WPARAM value associated with the message.
 * @param iParam The LPARAM value associated with the message.
 * @return Returns the custom handler result if the translated event is registered and marked as handled; otherwise returns the default handler result.
 */
template <typename EVENT_RESULT>
EVENT_RESULT window<EVENT_RESULT>::dispatch(UINT uMessage, WPARAM uParam, LPARAM iParam)
{
   auto eEvent = gdwin::translate_s(uMessage); // translate to portable event, ignored here but useful for debugging
   if(eEvent != gdwin::eNone)
   {
      if(m_bitsetMessageMap.test(eEvent) == true)                          // Test if message is registered, if so then call OnEvent, otherwise call default
      {
         EVENT_RESULT result_ = call(uMessage, uParam, iParam);
         if(event_result_traits<EVENT_RESULT>::is_handled(result_) == true) return result_;
      }
   }

   return call_default(uMessage, uParam, iParam);
}



/** ---------------------------------------------------------------------------
 * @brief Static window procedure that routes Win32 messages to the associated window object.
 * 
 * This static function serves as the window procedure for all instances of the window class. 
 * It retrieves the associated window object from the GWLP_USERDATA of the HWND and dispatches
 * the message to that instance. If no instance is found, it falls back to DefWindowProc.
 * 
 * @tparam EVENT_RESULT Type returned by the window's dispatch function.
 * @param hwnd Handle to the window receiving the message.
 * @param uMessage Identifier of the Win32 message being processed.
 * @param uParam Additional message information (message-specific WPARAM).
 * @param iParam Additional message information (message-specific LPARAM), including creation data for WM_NCCREATE.
 * @return The message result as an LRESULT: either the converted dispatch result when routed to the window instance, or DefWindowProc's default result when not handled by an instance.
 */
template <typename EVENT_RESULT>
LRESULT CALLBACK window<EVENT_RESULT>::window_procedure_s(HWND hwnd, UINT uMessage, WPARAM uParam, LPARAM iParam)
{
   window<EVENT_RESULT>* pwindow_ = nullptr;
   pwindow_ = reinterpret_cast<window<EVENT_RESULT>*>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA));// retrieve the window object pointer from the user data
   if(pwindow_ != nullptr)
   {
      auto result_ = pwindow_->dispatch(uMessage, uParam, iParam);
      return event_result_traits<EVENT_RESULT>::to_native(result_);
   }
   else
   {
      if(uMessage == WM_NCCREATE)
      {
         CREATESTRUCT* pCreateStruct = reinterpret_cast<CREATESTRUCT*>(iParam);
         pwindow_ = reinterpret_cast<window*>(pCreateStruct->lpCreateParams);
#if defined(UNICODE) || defined(_UNICODE)
         ::SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pwindow_));
#else
         ::SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pwindow_));
#endif
         pwindow_->m_hwnd = hwnd;
          auto result_ = pwindow_->dispatch(uMessage, uParam, iParam);
          return event_result_traits<EVENT_RESULT>::to_native(result_);
      }
   }
#if defined(UNICODE) || defined(_UNICODE)
   LRESULT iResult = ::DefWindowProcW(hwnd, uMessage, uParam, iParam);
#else
   LRESULT iResult = ::DefWindowProcA(hwnd, uMessage, uParam, iParam);
#endif
    return iResult;
}

/** ---------------------------------------------------------------------------
 * @brief Forwards a window event and its raw message parameters to gdwin::crack_message_s to populate an argument list.   
 * @tparam EVENT_RESULT The event result type used by the enclosing window specialization.
 * @param eEvent The window event identifier to decode.
 * @param uParam The WPARAM value associated with the event message.
 * @param iParam The LPARAM value associated with the event message.
 * @param argumentsMessage A reference to the arguments container that receives the decoded message data.
 */
template <typename EVENT_RESULT>
void window<EVENT_RESULT>::crack_message_s(gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsMessage)
{
   gdwin::crack_message_s(eEvent, uParam, iParam, argumentsMessage);
}


_GDWIN_END
