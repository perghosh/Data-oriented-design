#include "window.h"


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
