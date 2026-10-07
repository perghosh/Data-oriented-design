#include <string>

#include "gd/gd_arguments.h"

#include "window.h"

LRESULT CWindow::Call(UINT uMessage, WPARAM uParam, LPARAM iParam)
{
   std::array<std::byte, 128> array_;
   gd::argument::arguments arguments_ = gd::argument::arguments( array_ );

   auto eEvent = gd_win::translate_s(uMessage); // translate to portable event, ignored here but useful for debugging
   if(eEvent != gd_win::eNone)
   {

      CrackMessage_s(eEvent, uParam, iParam, arguments_);
      Process(eEvent, &arguments_, uMessage);

   }  

   return CallDefault(uMessage, uParam, iParam);
}

void CWindow::Process(gd_win::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   OutputDebugStringA("Process: ");
   OutputDebugStringA(std::to_string(static_cast<uint64_t>(eEvent)).c_str());
   OutputDebugStringA("\n");  
}

bool CWindow::Register_s(HINSTANCE hinstance)
{
   WNDCLASSEXW wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   if(GetClassInfoExW(hinstance, pwszClassName_s, &wndclassex) != FALSE) return true; // already registered

   wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   wndclassex.lpfnWndProc = WindowProc_s;
   wndclassex.hInstance = hinstance;
   wndclassex.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW, the macro is ANSI unless UNICODE is defined
   wndclassex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
   wndclassex.lpszClassName = pwszClassName_s;
   return RegisterClassExW(&wndclassex) != 0;
}

/// The one and only window procedure, finds the CWindow for the HWND and forwards
LRESULT CALLBACK CWindow::WindowProc_s(HWND hwnd, UINT uMessage, WPARAM uParam, LPARAM iParam)
{
   CWindow* pwindow = nullptr;
   if(uMessage == WM_NCCREATE)
   {
      pwindow = static_cast<CWindow*>(reinterpret_cast<CREATESTRUCTW*>(iParam)->lpCreateParams);
      if(pwindow != nullptr)
      {
         pwindow->m_hwnd = hwnd;
         SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pwindow));
      }
   }
   else
   {
      pwindow = reinterpret_cast<CWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
   }

   if(pwindow == nullptr) return DefWindowProcW(hwnd, uMessage, uParam, iParam); // before WM_NCCREATE or after detach

   LRESULT iResult = pwindow->Dispatch(uMessage, uParam, iParam);

   if(uMessage == WM_NCDESTROY) // last message a window gets
   {
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
      pwindow->m_hwnd = nullptr;
   }
   return iResult;
}

void CWindow::CrackMessage_s(gd_win::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut)
{
   switch (eEvent)
   {
   case gd_win::ePaint:
      break;
   case gd_win::eResize:
      break;
   case gd_win::eMove:
      break;
   case gd_win::eClose:
      break;
   case gd_win::eDestroy:
      break;
   case gd_win::eCreate:
      break;
   case gd_win::eKeyDown:                                                       // WM_KEYDOWN, WM_SYSKEYDOWN
   case gd_win::eKeyUp:
      argumentsOut.append("key", static_cast<uint32_t>(uParam));
      break;
   case gd_win::eChar:                                                          // WM_CHAR, WM_SYSCHAR, WM_UNICHAR, WM_DEADCHAR, WM_SYSDEADCHAR
      argumentsOut.append("char", static_cast<uint32_t>(uParam));
      argumentsOut.append("repeat", static_cast<uint32_t>(LOWORD(iParam)));
      break;
   case gd_win::eMouseMove:
      break;
   case gd_win::eMouseDown:                                                     // WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN, WM_XBUTTONDOWN
      argumentsOut.append("button", static_cast<uint32_t>(uParam));
      argumentsOut.append("x", static_cast<int32_t>(GET_X_LPARAM(iParam)));
      argumentsOut.append("y", static_cast<int32_t>(GET_Y_LPARAM(iParam)));
      break;
   case gd_win::eMouseUp:
      break;
   case gd_win::eMouseWheel:
      break;
   default:
      break;
   }
}
