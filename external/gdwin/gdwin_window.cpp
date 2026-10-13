// @FILE [tag: window] [description: Window class for portable window handling windows] [name: gdwin_window.cpp] [type: source]

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

std::expected<bool, std::string> register_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass)
{                                                                                                  assert(stringClass.size() > 0);
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

std::expected<bool, std::string> register_window_g(const gd::argument::arguments& argumentsWindow)
{ 
   std::string_view stringClass = argumentsWindow["class"].as_string_view(); assert(stringClass.size() > 0);
   return register_window_g(argumentsWindow, stringClass);
}

bool create_window_g(const gd::argument::arguments& argumentsWindow, std::string_view stringClass, std::string_view stringTitle, int iX, int iY, int iWidth, int iHeight, DWORD uStyle, HWND hwndParent, void* puserdata_)
{
    HINSTANCE hinstance = (HINSTANCE)argumentsWindow["hinstance"].as_pointer();
    if(hinstance == nullptr) { hinstance = GetModuleHandle(nullptr); }

#if defined(UNICODE) || defined(_UNICODE)
    std::wstring stringClassW(stringClass.begin(), stringClass.end());
    std::wstring stringTitleW(stringTitle.begin(), stringTitle.end());
    HWND hwnd = CreateWindowExW(0, stringClassW.data(), stringTitleW.data(), uStyle, iX, iY, iWidth, iHeight, hwndParent, nullptr, hinstance, puserdata_);
#else
    HWND hwnd = CreateWindowExA(0, stringClass.data(), stringTitle.data(), uStyle, iX, iY, iWidth, iHeight, hwndParent, nullptr, hinstance, puserdata_);
#endif   
    return hwnd != nullptr;
}

void crack_message_s(gdwin::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut)
{
   switch(eEvent)
   {
   case gdwin::ePaint:
      break;
   case gdwin::eResize:
      argumentsOut.append("width", static_cast<int32_t>(LOWORD(iParam)));
      argumentsOut.append("height", static_cast<int32_t>(HIWORD(iParam)));
      break;
   case gdwin::eMove:
      argumentsOut.append("x", static_cast<int32_t>(LOWORD(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(HIWORD(iParam)));
      break;
   case gdwin::eClose:
      break;
   case gdwin::eDestroy:
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
   }
   break;
   case gdwin::eKeyDown:                                                       // WM_KEYDOWN, WM_SYSKEYDOWN
   case gdwin::eKeyUp:
      argumentsOut.append("key", static_cast<uint32_t>(uParam));
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam) & 0xFFu));
      argumentsOut.append("scan_code", static_cast<uint32_t>((HIWORD(iParam) & 0xFFu)));
      break;
   case gdwin::eChar:                                                          // WM_CHAR, WM_SYSCHAR, WM_UNICHAR, WM_DEADCHAR, WM_SYSDEADCHAR
      argumentsOut.append("char", static_cast<uint32_t>(uParam));
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam)));
      break;
   case gdwin::eMouseMove:
      argumentsOut.append("buttons", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gdwin::eMouseDown:                                                     // WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN, WM_XBUTTONDOWN
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gdwin::eMouseUp:
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gdwin::eMouseDoubleClick:
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gdwin::eMouseWheel:
      argumentsOut.append("delta", static_cast<int32_t>(GET_WHEEL_DELTA_WPARAM(uParam)));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gdwin::eMouseHover:
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gdwin::eMouseLeave:
      break;
   case gdwin::eFocusIn:
   case gdwin::eFocusOut:
      argumentsOut.append("hwnd", reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(iParam)));
      break;
   case gdwin::eTimer:
      argumentsOut.append("id", static_cast<uint32_t>(uParam));
      break;
   case gdwin::eScroll:
      argumentsOut.append("code", static_cast<uint32_t>(LOWORD(uParam)));
      argumentsOut.append("position", static_cast<uint32_t>(HIWORD(uParam)));
      argumentsOut.append("target", reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(iParam)));
      break;
   case gdwin::eCommand:
      argumentsOut.append("id", static_cast<uint32_t>(LOWORD(uParam)));
      argumentsOut.append("notify_code", static_cast<uint32_t>(HIWORD(uParam)));
      argumentsOut.append("target", reinterpret_cast<uintptr_t>(reinterpret_cast<void*>(iParam)));
      break;
   case gdwin::eNotification:
      argumentsOut.append("code", static_cast<uint32_t>(reinterpret_cast<const NMHDR*>(iParam) != nullptr ? reinterpret_cast<const NMHDR*>(iParam)->code : 0u));
      argumentsOut.append("target", reinterpret_cast<uintptr_t>(reinterpret_cast<const NMHDR*>(iParam) != nullptr ? reinterpret_cast<const NMHDR*>(iParam)->hwndFrom : nullptr));
      break;
   case gdwin::eContextMenu:
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      argumentsOut.append("hit_test", static_cast<uint32_t>(uParam));
      break;
   case gdwin::eDrop:
      break;
   case gdwin::eHotkey:
      argumentsOut.append("id", static_cast<uint32_t>(LOWORD(uParam)));
      break;
   case gdwin::eWindowActivate:
      argumentsOut.append("state", static_cast<uint32_t>(LOWORD(uParam)));
      break;
   case gdwin::eWindowVisibility:
      argumentsOut.append("shown", static_cast<uint32_t>(static_cast<bool>(uParam)));
      break;
   case gdwin::eWindowEnabled:
      argumentsOut.append("enabled", static_cast<uint32_t>(static_cast<bool>(uParam)));
      break;
   case gdwin::eClipboard:
      argumentsOut.append("message", static_cast<uint32_t>(uParam));
      break;
   case gdwin::ePointerMove:
   case gdwin::ePointerDown:
   case gdwin::ePointerUp:
      argumentsOut.append("pointer_id", static_cast<uint32_t>(GET_POINTERID_WPARAM(uParam)));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   default:
      break;
   }
}









_GDWIN_END
