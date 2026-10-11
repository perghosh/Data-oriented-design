// @FILE [tag: window] [description: Window class for portable window handling windows] [name: gdwin_window.cpp] [type: source]

#include "gdwin_window.h"

_GDWIN_BEGIN

std::expected<bool, std::string> register_window_g(const gd::argument::arguments& argumentsWindow )
{                                                                                                  assert( stringClass.empty() == false );
   HINSTANCE hinstance = (HINSTANCE)argumentsWindow["instance"].as_pointer();                      assert(hinstance != nullptr);
   WNDCLASSEXW wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);

   std::string_view  stringClass = argumentsWindow["class"].as_string_view();                      assert(stringClass.size() > 0);
   if(::GetClassInfoExW(hinstance, stringClass.data(), &wndclassex) != FALSE) return true; // already registered

   decltype(wndclassex.lpfnWndProc) pfnWndProc = (decltype(wndclassex.lpfnWndProc))argumentsWindow["callback"].as_pointer(); assert(pfnWndProc != nullptr);

   bool bResult;
#if defined(UNICODE) || defined(_UNICODE)
   std::wstring stringClassW(stringClass.begin(), stringClass.end());
   wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   wndclassex.lpfnWndProc = pfnWndProc;
   wndclassex.hInstance = hinstance;
   wndclassex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, the macro is ANSI unless UNICODE is defined
   wndclassex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
   wndclassex.lpszClassName = stringClassW.data();
   bResult = RegisterClassExW(&wndclassex) != 0;
#else
   wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   wndclassex.lpfnWndProc = pfnWndProc;
   wndclassex.hInstance = hinstance;
   wndclassex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, the macro is ANSI unless UNICODE is defined
   wndclassex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
   wndclassex.lpszClassName = stringClass.data();
   bResult = RegisterClassExA(&wndclassex) != 0;
#endif

   return bResult;
}

bool create_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass, std::string_view stringTitle, int iX, int iY, int iWidth, int iHeight, DWORD uStyle = WS_OVERLAPPEDWINDOW, HWND hwndParent)
{
    HINSTANCE hinstance = (HINSTANCE)argumentsWindow["hinstance"].as_pointer();
    if(hinstance == nullptr) { hinstance = GetModuleHandle(nullptr); }

#if defined(UNICODE) || defined(_UNICODE)
    std::wstring stringClassW(stringClass.begin(), stringClass.end());
    std::wstring stringTitleW(stringTitle.begin(), stringTitle.end());
    HWND hwnd = CreateWindowExW(0, stringClassW.data(), stringTitleW.data(), uStyle, iX, iY, iWidth, iHeight, hwndParent, nullptr, hinstance, nullptr);
#else
    HWND hwnd = CreateWindowExA(0, stringClass.data(), stringTitle.data(), uStyle, iX, iY, iWidth, iHeight, hwndParent, nullptr, hinstance, nullptr);
#endif   
    return hwnd != nullptr;
}

void crack_message_s(gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut)
{
   switch(eEvent)
   {
   case gd_win::ePaint:
      break;
   case gd_win::eResize:
      argumentsOut.append("width", static_cast<int32_t>(LOWORD(iParam)));
      argumentsOut.append("height", static_cast<int32_t>(HIWORD(iParam)));
      break;
   case gd_win::eMove:
      argumentsOut.append("x", static_cast<int32_t>(LOWORD(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(HIWORD(iParam)));
      break;
   case gd_win::eClose:
      break;
   case gd_win::eDestroy:
      break;
   case gd_win::eCreate:
   {
      const auto* pcreate = reinterpret_cast<const CREATESTRUCTW*>(iParam);
      if(pcreate != nullptr)
      {
         argumentsOut.append("x", static_cast<int32_t>(pcreate->x));
         argumentsOut.append("y", static_cast<int32_t>(pcreate->y));
         argumentsOut.append("width", static_cast<int32_t>(pcreate->cx));
         argumentsOut.append("height", static_cast<int32_t>(pcreate->cy));
      }
   }
   break;
   case gd_win::eKeyDown:                                                       // WM_KEYDOWN, WM_SYSKEYDOWN
   case gd_win::eKeyUp:
      argumentsOut.append("key", static_cast<uint32_t>(uParam));
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam) & 0xFFu));
      argumentsOut.append("scan_code", static_cast<uint32_t>((HIWORD(iParam) & 0xFFu)));
      break;
   case gd_win::eChar:                                                          // WM_CHAR, WM_SYSCHAR, WM_UNICHAR, WM_DEADCHAR, WM_SYSDEADCHAR
      argumentsOut.append("char", static_cast<uint32_t>(uParam));
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam)));
      break;
   case gd_win::eMouseMove:
      argumentsOut.append("buttons", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseDown:                                                     // WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN, WM_XBUTTONDOWN
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseUp:
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseDoubleClick:
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseWheel:
      argumentsOut.append("delta", static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(uParam)));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseHover:
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseLeave:
      break;
   case gd_win::eFocusIn:
   case gd_win::eFocusOut:
      argumentsOut.append("hwnd", reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(iParam)));
      break;
   case gd_win::eTimer:
      argumentsOut.append("id", static_cast<uint32_t>(uParam));
      break;
   case gd_win::eScroll:
      argumentsOut.append("code", static_cast<uint32_t>(LOWORD(uParam)));
      argumentsOut.append("position", static_cast<uint32_t>(HIWORD(uParam)));
      argumentsOut.append("target", reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(iParam)));
      break;
   case gd_win::eCommand:
      argumentsOut.append("id", static_cast<uint32_t>(LOWORD(uParam)));
      argumentsOut.append("notify_code", static_cast<uint32_t>(HIWORD(uParam)));
      argumentsOut.append("target", reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(iParam)));
      break;
   case gd_win::eNotification:
      argumentsOut.append("code", static_cast<uint32_t>(reinterpret_cast<const NMHDR*>(iParam) != nullptr ? reinterpret_cast<const NMHDR*>(iParam)->code : 0u));
      argumentsOut.append("target", reinterpret_cast<uintptr_t>(reinterpret_cast<const NMHDR*>(iParam) != nullptr ? reinterpret_cast<const NMHDR*>(iParam)->hwndFrom : nullptr));
      break;
   case gd_win::eContextMenu:
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("hit_test", static_cast<uint32_t>(uParam));
      break;
   case gd_win::eDrop:
      break;
   case gd_win::eHotkey:
      argumentsOut.append("id", static_cast<uint32_t>(LOWORD(uParam)));
      break;
   case gd_win::eWindowActivate:
      argumentsOut.append("state", static_cast<uint32_t>(LOWORD(uParam)));
      break;
   case gd_win::eWindowVisibility:
      argumentsOut.append("shown", static_cast<uint32_t>(static_cast<bool>(uParam)));
      break;
   case gd_win::eWindowEnabled:
      argumentsOut.append("enabled", static_cast<uint32_t>(static_cast<bool>(uParam)));
      break;
   case gd_win::eClipboard:
      argumentsOut.append("message", static_cast<uint32_t>(uParam));
      break;
   case gd_win::ePointerMove:
   case gd_win::ePointerDown:
   case gd_win::ePointerUp:
      argumentsOut.append("pointer_id", static_cast<uint32_t>(GET_POINTERID_WPARAM(uParam)));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   default:
      break;
   }
}









_GDWIN_END
