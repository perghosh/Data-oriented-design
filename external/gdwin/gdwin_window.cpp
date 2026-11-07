// @FILE [tag: window] [description: Window class for portable window handling windows] [name: gdwin_window.cpp] [type: source]

/**
 * @file gdwin_window.h
 * @brief Window class for portable window handling on Windows.
 *
 *    register__            register a window class
 *    create__              create a window
 *    message__             message dispatching
 */


#include "gdwin_window.h"

_GDWIN_BEGIN

int run_s()
{
   MSG msg = {};
   while(GetMessageW(&msg, nullptr, 0, 0) > 0)
   {
      TranslateMessage(&msg);
      DispatchMessageW(&msg);
   }
   return static_cast<int>(msg.wParam);
}

/** ----------------------------------------------------------------------------
 * @brief Registers a Win32 window class (Unicode or ANSI build) using values from an arguments container, and skips registration if the class is already registered.
 * @param argumentsWindow Input arguments used to configure registration. It must provide an "instance" pointer (HINSTANCE) and a "callback" pointer (window procedure).
 * @param stringClass The window class name to check and register.
 * @return An `std::expected<bool, std::string>` containing `true` if the class already exists or registration succeeds, and `false` if registration fails.
 * @API [tag: register, window] [summary: Registers a window class with the specified name and arguments.] [jump: register__]
 */
std::expected<bool, std::string> register_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass)
{
   assert(stringClass.size() > 0);
   HINSTANCE hinstance = (HINSTANCE)argumentsWindow["instance"].as_pointer();                      assert(hinstance != nullptr);

   bool bResult;
#if defined(UNICODE) || defined(_UNICODE)
   WNDCLASSEXW wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   decltype(wndclassex.lpfnWndProc) pfnWndProc = (decltype(wndclassex.lpfnWndProc))argumentsWindow["callback"].as_pointer(); assert(pfnWndProc != nullptr);

   std::wstring stringClassW(stringClass.begin(), stringClass.end());
   if(::GetClassInfoExW(hinstance, stringClassW.data(), &wndclassex) != FALSE) return true; // already registered

   wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   wndclassex.lpfnWndProc = pfnWndProc;
   wndclassex.hInstance = hinstance;
   wndclassex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, the macro is ANSI unless UNICODE is defined
   wndclassex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
   wndclassex.lpszClassName = stringClassW.data();
   bResult = RegisterClassExW(&wndclassex) != 0;
#else
   WNDCLASSEXA wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXA);
   decltype(wndclassex.lpfnWndProc) pfnWndProc = (decltype(wndclassex.lpfnWndProc))argumentsWindow["callback"].as_pointer(); assert(pfnWndProc != nullptr);

   wndclassex = {};
   if(::GetClassInfoExA(hinstance, stringClass.data(), &wndclassex) != FALSE) return true; // already registered


   wndclassex.cbSize = sizeof(WNDCLASSEXA);
   wndclassex.lpfnWndProc = pfnWndProc;
   wndclassex.hInstance = hinstance;
   wndclassex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, the macro is ANSI unless UNICODE is defined
   wndclassex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
   wndclassex.lpszClassName = stringClass.data();
   bResult = RegisterClassExA(&wndclassex) != 0;
#endif

   return bResult;
}

/** ----------------------------------------------------------------------------
 * @brief Registers a window using the class name stored in the provided arguments and forwards the call to the overload that takes an explicit class name.
 * @param argumentsWindow A set of window arguments that must include a non-empty "class" string value.
 * @return An expected result containing a boolean success value on success, or an error message string on failure.
 */
std::expected<bool, std::string> register_window_g(const gd::argument::arguments& argumentsWindow)
{
   std::string_view stringClass = argumentsWindow["class"].as_string_view(); assert(stringClass.size() > 0);
   return register_window_g(argumentsWindow, stringClass);
}

/** ---------------------------------------------------------------------------
 * @brief Creates a Win32 window using options from an argument map and explicit position, size, style, and parent values.
 * @param argumentsWindow Optional window-creation options. Uses "hinstance" (module instance pointer) and "styleex" (extended style as string name or integer value).
 * @param stringClass Registered window class name.
 * @param stringTitle Window title text.
 * @param iX Initial horizontal position of the window.
 * @param iY Initial vertical position of the window.
 * @param iWidth Initial window width, in pixels.
 * @param iHeight Initial window height, in pixels.
 * @param uStyle Window style flags (passed to CreateWindowEx as dwStyle).
 * @param hwndParent Handle to the parent or owner window, or null if none.
 * @param puserdata_ User data pointer passed as lpParam to window creation.
 * @return true if the window handle was created successfully; otherwise false.
 * @API [tag: create, window] [summary: Creates a window with the specified class, title, position, size, style, and parent.] [jump: create__]
 */
bool create_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass, std::string_view stringTitle, int iX, int iY, int iWidth, int iHeight, uint64_t uStyle, HWND hwndParent, void* puserdata_)
{
   HINSTANCE hinstance = (HINSTANCE)argumentsWindow["hinstance"].as_pointer(); // optional, if not provided, we will get the current module handle
   if(hinstance == nullptr) { hinstance = GetModuleHandle(nullptr); }

   HMENU hMenu = (HMENU)argumentsWindow["menu"].as_pointer(); // menu handle, optional, if not provided, we will use nullptr

   uint64_t uStyleEx = 0;
   auto argumentsStyleEx = argumentsWindow["styleex"];
   if(argumentsStyleEx.is_string() == true)                                   // value are passed as string, so we need to convert them to valid style values
   {
      std::string_view stringStyleEx = argumentsStyleEx.as_string_view();
      if(stringStyleEx == "dlgmodalframe") { uStyleEx = WS_EX_DLGMODALFRAME; }
      else if(stringStyleEx == "noparentnotify") { uStyleEx = WS_EX_NOPARENTNOTIFY; }
      else if(stringStyleEx == "topmost") { uStyleEx = WS_EX_TOPMOST; }
      else if(stringStyleEx == "acceptfiles") { uStyleEx = WS_EX_ACCEPTFILES; }
      else if(stringStyleEx == "transparent") { uStyleEx = WS_EX_TRANSPARENT; }
   }
   else if(argumentsStyleEx.is_integer() == true)
   {
      uStyleEx = argumentsStyleEx.as_uint();                                   // value are passed as integer, not string and this neeed to be a valid style value, otherwise the window will not be created
   }

#if defined(UNICODE) || defined(_UNICODE)
   std::wstring stringClassW(stringClass.begin(), stringClass.end());
   std::wstring stringTitleW(stringTitle.begin(), stringTitle.end());
   HWND hwnd = CreateWindowExW(static_cast<DWORD>(uStyleEx), stringClassW.data(), stringTitleW.data(), static_cast<DWORD>(uStyle), iX, iY, iWidth, iHeight, hwndParent, hMenu, hinstance, puserdata_);
#else
   std::string stringClassA(stringClass.begin(), stringClass.end());
   std::string stringTitleA(stringTitle.begin(), stringTitle.end());
   HWND hwnd = CreateWindowExA(static_cast<DWORD>(uStyleEx), stringClassA.data(), stringTitleA.data(), static_cast<DWORD>(uStyle), iX, iY, iWidth, iHeight, hwndParent, hMenu, hinstance, puserdata_);
#endif   
   bool bOk = hwnd != nullptr;
   if(bOk == false)
   {
      DWORD uError = GetLastError();
   }
   return bOk;
}

namespace
{
   /// Get keyboard modifiers as portable bit flags (gdwin::eModifier). Reflects the state when the message was sent
   uint32_t get_modifiers_g()
   {
      uint32_t uModifiers = gdwin::eModifierNone;
      if(::GetKeyState(VK_SHIFT) < 0)   uModifiers |= gdwin::eModifierShift;
      if(::GetKeyState(VK_CONTROL) < 0) uModifiers |= gdwin::eModifierControl;
      if(::GetKeyState(VK_MENU) < 0)    uModifiers |= gdwin::eModifierAlt;
      if(::GetKeyState(VK_LWIN) < 0 || ::GetKeyState(VK_RWIN) < 0) uModifiers |= gdwin::eModifierMeta;
      return uModifiers;
   }

   /// Convert MK_ flags in wParam from mouse messages to a bit mask where bit position is gdwin::eMouseButton
   uint32_t get_mouse_buttons_g(WPARAM uParam)
   {
      uint32_t uButtons = 0;
      if((uParam & MK_LBUTTON) != 0)  uButtons |= 1u << gdwin::eMouseButtonLeft;
      if((uParam & MK_MBUTTON) != 0)  uButtons |= 1u << gdwin::eMouseButtonMiddle;
      if((uParam & MK_RBUTTON) != 0)  uButtons |= 1u << gdwin::eMouseButtonRight;
      if((uParam & MK_XBUTTON1) != 0) uButtons |= 1u << gdwin::eMouseButtonBack;
      if((uParam & MK_XBUTTON2) != 0) uButtons |= 1u << gdwin::eMouseButtonForward;
      return uButtons;
   }

   /// Get the button (gdwin::eMouseButton) that caused a button down, up or double click message
   uint32_t get_mouse_button_g(UINT uMessage, WPARAM uParam)
   {
      switch(uMessage)
      {
      case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK: return gdwin::eMouseButtonMiddle;
      case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK: return gdwin::eMouseButtonRight;
      case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK: return GET_XBUTTON_WPARAM(uParam) == XBUTTON1 ? gdwin::eMouseButtonBack : gdwin::eMouseButtonForward;
      default: return gdwin::eMouseButtonLeft;
      }
   }

   /// Handles are stored as uint64_t, a fixed width type that is the same on all platforms
   uint64_t to_handle_g(uintptr_t uHandle) { return static_cast<uint64_t>(uHandle); }
}

/** ----------------------------------------------------------------------------
 * @brief Converts a Win32 message into a portable event argument map.
 *
 * Messages that belong together share one event, details about which message it was and what
 * it contained are added as arguments. Coordinates are in the coordinate space that Windows
 * uses for the message, this is noted below. Handles are uint64_t. All argument types are fixed width
 * (int32_t, uint32_t, uint64_t, bool) so the same code builds on other platforms.
 *
 * | event               | arguments                                                                               |
 * |---------------------|-----------------------------------------------------------------------------------------|
 * | eResize             | width, height (client, WM_SIZING: outer window), state (gdwin::eWindowState), live, edge (WM_SIZING) |
 * | eMove               | x, y (signed, WM_SIZING: window rect), live                                            |
 * | eCreate             | x, y, width, height, nc (true for WM_NCCREATE)                                          |
 * | eDestroy            | final (true for WM_NCDESTROY)                                                           |
 * | eKeyDown, eKeyUp    | key (virtual key), repeat, scan_code, extended, previous, alt, system, modifiers        |
 * | eChar               | char, repeat, dead, system, utf32 (WM_UNICHAR code point), modifiers                    |
 * | eMouseMove          | x, y (client), buttons (bit per gdwin::eMouseButton), modifiers                         |
 * | eMouseDown, eMouseUp, eMouseDoubleClick | button (gdwin::eMouseButton), buttons, x, y (client), modifiers     |
 * | eMouseWheel         | delta (WHEEL_DELTA units), horizontal, x, y (screen), buttons, modifiers                |
 * | eMouseHover         | x, y (client), buttons, modifiers                                                       |
 * | eFocusIn, eFocusOut | hwnd (window that lost or gets the focus, 0 if none)                                    |
 * | eTimer              | id                                                                                      |
 * | eScroll             | orientation (0 horizontal, 1 vertical), code, position (thumb only), target (scrollbar control, 0 for window scrollbar) |
 * | eCommand            | id, notify_code, source (gdwin::eCommandSource), target (control)                       |
 * | eNotification       | id (control), code, target (control), header (NMHDR*)                                   |
 * | eContextMenu        | x, y (screen), keyboard (true if from keyboard), target (window that was clicked)       |
 * | eDrop               | drop (HDROP, use DragQueryFile and DragFinish)                                          |
 * | eHotkey             | id, key (virtual key), modifiers                                                        |
 * | eWindowActivate     | app, state (0 inactive, 1 active, 2 click active), minimized, other (window), thread (app) |
 * | eWindowVisibility   | shown, reason                                                                           |
 * | eWindowEnabled      | enabled                                                                                 |
 * | eClipboard          | action (gdwin::eClipboardAction)                                                        |
 * | eTouch              | count, handle (HTOUCHINPUT)                                                             |
 * | eGesture            | handle (HGESTUREINFO)                                                                   |
 * | ePointerMove, ePointerDown, ePointerUp | pointer_id, x, y (screen), modifiers                         |
 *
 * @param eEvent The window event type, here translated to common ids.
 * @param uMessage The native message that was translated to eEvent.
 * @param uParam The WPARAM value associated with the event.
 * @param iParam The LPARAM value associated with the event.
 * @param argumentsOut The argument map to populate with event-specific data.
 * @API [tag: message] [summary: Cracks a window message into its constituent arguments.] [jump: message__]
 */
void crack_message_g(gdwin::eWindowEvent eEvent, UINT uMessage, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut)
{
   switch(eEvent)
   {
   case gdwin::ePaint:
      break;
   case gdwin::eResize:
      if(uMessage == WM_SIZING)                                                // lParam is a RECT with the proposed window rectangle
      {
         const RECT* prect = reinterpret_cast<const RECT*>(iParam);
         argumentsOut.append("width", static_cast<int32_t>(prect != nullptr ? prect->right - prect->left : 0));
         argumentsOut.append("height", static_cast<int32_t>(prect != nullptr ? prect->bottom - prect->top : 0));
         argumentsOut.append("state", static_cast<uint32_t>(gdwin::eWindowStateRestored));
         argumentsOut.append("live", true);
         argumentsOut.append("edge", static_cast<uint32_t>(uParam));           // WMSZ_ value
      }
      else                                                                     // WM_SIZE
      {
         uint32_t uState = gdwin::eWindowStateRestored;
         if(uParam == SIZE_MINIMIZED) uState = gdwin::eWindowStateMinimized;
         else if(uParam == SIZE_MAXIMIZED) uState = gdwin::eWindowStateMaximized;
         argumentsOut.append("width", static_cast<int32_t>(LOWORD(iParam)));
         argumentsOut.append("height", static_cast<int32_t>(HIWORD(iParam)));
         argumentsOut.append("state", uState);
         argumentsOut.append("live", false);
      }
      break;
   case gdwin::eMove:
      if(uMessage == WM_MOVING)                                                // lParam is a RECT with the proposed window rectangle
      {
         const RECT* prect = reinterpret_cast<const RECT*>(iParam);
         argumentsOut.append("x", static_cast<int32_t>(prect != nullptr ? prect->left : 0));
         argumentsOut.append("y", static_cast<int32_t>(prect != nullptr ? prect->top : 0));
         argumentsOut.append("live", true);
      }
      else                                                                     // WM_MOVE, coordinates are signed
      {
         argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
         argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
         argumentsOut.append("live", false);
      }
      break;
   case gdwin::eClose:
      break;
   case gdwin::eDestroy:
      argumentsOut.append("final", uMessage == WM_NCDESTROY);
      break;
   case gdwin::eCreate:
   {
      const auto* pcreate = reinterpret_cast<const CREATESTRUCTW*>(iParam);
      if(pcreate != nullptr)
      {
         argumentsOut.append("x", static_cast<int32_t>(pcreate->x));
         argumentsOut.append("y", static_cast<int32_t>(pcreate->y));
         argumentsOut.append("width", static_cast<int32_t>(pcreate->cx));
         argumentsOut.append("height", static_cast<int32_t>(pcreate->cy));
      }
      argumentsOut.append("nc", uMessage == WM_NCCREATE);
   }
   break;
   case gdwin::eKeyDown:                                                       // WM_KEYDOWN, WM_SYSKEYDOWN
   case gdwin::eKeyUp:
      argumentsOut.append("key", static_cast<uint32_t>(uParam));
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam)));    // repeat count is bit 0-15
      argumentsOut.append("scan_code", static_cast<uint32_t>(HIWORD(iParam) & 0xFFu));
      argumentsOut.append("extended", (HIWORD(iParam) & KF_EXTENDED) != 0);
      argumentsOut.append("previous", (HIWORD(iParam) & KF_REPEAT) != 0);      // key was down before this message, auto repeat when key down
      argumentsOut.append("alt", (HIWORD(iParam) & KF_ALTDOWN) != 0);
      argumentsOut.append("system", uMessage == WM_SYSKEYDOWN || uMessage == WM_SYSKEYUP);
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   case gdwin::eChar:                                                          // WM_CHAR, WM_SYSCHAR, WM_UNICHAR, WM_DEADCHAR, WM_SYSDEADCHAR
      argumentsOut.append("char", static_cast<uint32_t>(uParam));              // WM_CHAR is a UTF-16 code unit, WM_UNICHAR is a UTF-32 code point
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam)));
      argumentsOut.append("dead", uMessage == WM_DEADCHAR || uMessage == WM_SYSDEADCHAR);
      argumentsOut.append("system", uMessage == WM_SYSCHAR || uMessage == WM_SYSDEADCHAR);
      argumentsOut.append("utf32", uMessage == WM_UNICHAR);
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   case gdwin::eMouseMove:
      argumentsOut.append("buttons", get_mouse_buttons_g(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   case gdwin::eMouseDown:                                                     // WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN, WM_XBUTTONDOWN
   case gdwin::eMouseUp:
   case gdwin::eMouseDoubleClick:
      argumentsOut.append("button", get_mouse_button_g(uMessage, uParam));     // the button that caused the message
      argumentsOut.append("buttons", get_mouse_buttons_g(uParam));             // buttons that are down
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   case gdwin::eMouseWheel:                                                    // WM_MOUSEWHEEL, WM_MOUSEHWHEEL
      argumentsOut.append("delta", static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(uParam)));
      argumentsOut.append("horizontal", uMessage == WM_MOUSEHWHEEL);
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));    // screen coordinates, use ScreenToClient
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("buttons", get_mouse_buttons_g(GET_KEYSTATE_WPARAM(uParam)));
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   case gdwin::eMouseHover:
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("buttons", get_mouse_buttons_g(uParam));
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   case gdwin::eMouseLeave:
      break;
   case gdwin::eFocusIn:                                                       // WM_SETFOCUS wParam is the window that lost focus, WM_KILLFOCUS wParam is the window that gets it
   case gdwin::eFocusOut:
      argumentsOut.append("hwnd", to_handle_g(uParam));
      break;
   case gdwin::eTimer:
      argumentsOut.append("id", static_cast<uint32_t>(uParam));
      break;
   case gdwin::eScroll:                                                        // WM_HSCROLL, WM_VSCROLL
      argumentsOut.append("orientation", static_cast<uint32_t>(uMessage == WM_VSCROLL ? 1 : 0));
      argumentsOut.append("code", static_cast<uint32_t>(LOWORD(uParam)));      // SB_ request code
      argumentsOut.append("position", static_cast<uint32_t>(HIWORD(uParam))); // only valid for SB_THUMBTRACK and SB_THUMBPOSITION, 16 bit, use GetScrollInfo for larger ranges
      argumentsOut.append("target", to_handle_g(static_cast<uintptr_t>(iParam)));
      break;
   case gdwin::eCommand:                                                       // WM_COMMAND, WM_SYSCOMMAND
      if(uMessage == WM_SYSCOMMAND)
      {
         argumentsOut.append("id", static_cast<uint32_t>(uParam & 0xFFF0));   // four low bits are used by the system
         argumentsOut.append("notify_code", static_cast<uint32_t>(0));
         argumentsOut.append("source", static_cast<uint32_t>(gdwin::eCommandSourceSystem));
         argumentsOut.append("target", to_handle_g(0));
      }
      else
      {
         const uint32_t uNotifyCode = HIWORD(uParam);
         uint32_t uSource = gdwin::eCommandSourceMenu;
         if(iParam != 0) uSource = gdwin::eCommandSourceControl;
         else if(uNotifyCode == 1) uSource = gdwin::eCommandSourceAccelerator;
         argumentsOut.append("id", static_cast<uint32_t>(LOWORD(uParam)));
         argumentsOut.append("notify_code", uNotifyCode);
         argumentsOut.append("source", uSource);
         argumentsOut.append("target", to_handle_g(static_cast<uintptr_t>(iParam)));
      }
      break;
   case gdwin::eNotification:
   {
      const NMHDR* pnmhdr = reinterpret_cast<const NMHDR*>(iParam);
      argumentsOut.append("id", static_cast<uint32_t>(uParam));
      argumentsOut.append("code", static_cast<uint32_t>(pnmhdr != nullptr ? pnmhdr->code : 0u));
      argumentsOut.append("target", to_handle_g(pnmhdr != nullptr ? reinterpret_cast<uintptr_t>(pnmhdr->hwndFrom) : 0));
      argumentsOut.append("header", reinterpret_cast<void*>(iParam));          // cast to the specific notification structure
   }
   break;
   case gdwin::eContextMenu:
   {
      const int32_t iX = GET_X_LPARAM(iParam);
      const int32_t iY = GET_Y_LPARAM(iParam);
      argumentsOut.append("x", iX);                                            // screen coordinates
      argumentsOut.append("y", iY);
      argumentsOut.append("keyboard", iX == -1 && iY == -1);                   // -1, -1 when invoked from keyboard
      argumentsOut.append("target", to_handle_g(uParam));
   }
   break;
   case gdwin::eDrop:
      argumentsOut.append("drop", to_handle_g(uParam));
      break;
   case gdwin::eHotkey:
   {
      uint32_t uModifiers = gdwin::eModifierNone;
      const uint32_t uNativeModifiers = LOWORD(iParam);
      if((uNativeModifiers & MOD_SHIFT) != 0)   uModifiers |= gdwin::eModifierShift;
      if((uNativeModifiers & MOD_CONTROL) != 0) uModifiers |= gdwin::eModifierControl;
      if((uNativeModifiers & MOD_ALT) != 0)     uModifiers |= gdwin::eModifierAlt;
      if((uNativeModifiers & MOD_WIN) != 0)     uModifiers |= gdwin::eModifierMeta;
      argumentsOut.append("id", static_cast<uint32_t>(uParam));
      argumentsOut.append("key", static_cast<uint32_t>(HIWORD(iParam)));
      argumentsOut.append("modifiers", uModifiers);
   }
   break;
   case gdwin::eWindowActivate:                                                // WM_ACTIVATE, WM_ACTIVATEAPP
      if(uMessage == WM_ACTIVATEAPP)
      {
         argumentsOut.append("app", true);
         argumentsOut.append("state", static_cast<uint32_t>(uParam != 0 ? 1 : 0));
         argumentsOut.append("thread", static_cast<uint32_t>(iParam));         // thread of the window that is deactivated or activated
      }
      else
      {
         argumentsOut.append("app", false);
         argumentsOut.append("state", static_cast<uint32_t>(LOWORD(uParam)));  // WA_INACTIVE, WA_ACTIVE, WA_CLICKACTIVE
         argumentsOut.append("minimized", HIWORD(uParam) != 0);
         argumentsOut.append("other", to_handle_g(static_cast<uintptr_t>(iParam)));
      }
      break;
   case gdwin::eWindowVisibility:
      argumentsOut.append("shown", uParam != 0);
      argumentsOut.append("reason", static_cast<uint32_t>(iParam));            // 0 if ShowWindow was called, otherwise SW_PARENTCLOSING and friends
      break;
   case gdwin::eWindowEnabled:
      argumentsOut.append("enabled", uParam != 0);
      break;
   case gdwin::eClipboard:
   {
      uint32_t uAction = gdwin::eClipboardActionCut;
      if(uMessage == WM_COPY) uAction = gdwin::eClipboardActionCopy;
      else if(uMessage == WM_PASTE) uAction = gdwin::eClipboardActionPaste;
      else if(uMessage == WM_CLEAR) uAction = gdwin::eClipboardActionClear;
      else if(uMessage == WM_UNDO) uAction = gdwin::eClipboardActionUndo;
      argumentsOut.append("action", uAction);
   }
   break;
   case gdwin::eTouch:                                                         // WM_TOUCH, window needs to be registered with RegisterTouchWindow
      argumentsOut.append("count", static_cast<uint32_t>(LOWORD(uParam)));
      argumentsOut.append("handle", to_handle_g(static_cast<uintptr_t>(iParam)));
      break;
   case gdwin::eGesture:                                                       // WM_GESTURE
      argumentsOut.append("handle", to_handle_g(static_cast<uintptr_t>(iParam)));
      break;
   case gdwin::ePointerMove:
   case gdwin::ePointerDown:
   case gdwin::ePointerUp:
      argumentsOut.append("pointer_id", static_cast<uint32_t>(GET_POINTERID_WPARAM(uParam)));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));    // screen coordinates
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("modifiers", get_modifiers_g());
      break;
   default:
      break;
   }
}


_GDWIN_END
