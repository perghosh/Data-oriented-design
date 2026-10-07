// window.h : thin Win32 window wrapper, messages are bound to methods with Bind<message>( object, method )

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif

#include <bitset>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include <windows.h>
#include <windowsx.h>

#include "gd/gd_arguments.h"

#include "os/OS_Event.h"

// ## CWindow -------------------------------------------------------------------------------------

class CWindow
{
public:
   using handler = std::function<LRESULT(WPARAM, LPARAM)>;

   CWindow() = default;
   CWindow(const CWindow&) = delete;            // the HWND and the bound handlers point at this object
   CWindow& operator=(const CWindow&) = delete;
   virtual ~CWindow()
   {
      if(m_hwnd != nullptr)
      {
         SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, 0); // detach first, derived part is already gone
         DestroyWindow(m_hwnd);
      }
   }

   virtual void Process( gd_win::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage );

   void Bind(UINT uMessage)
   {                                                                                                assert(uMessage < gd_win::uMaxMessageId);
      m_bitsetMessageMap.set(uMessage); // for debugging, see Dispatch()
   }

   /// Create the window, message bindings should be done before this call so WM_CREATE is seen
   bool Create(const std::wstring& stringTitle, int iWidth, int iHeight, DWORD uStyle = WS_OVERLAPPEDWINDOW, HWND hwndParent = nullptr)
   {
      HINSTANCE hinstance = GetModuleHandleW(nullptr);
      if(Register_s(hinstance) == false) return false;

      // `this` travels in lpParam and is picked up in WM_NCCREATE by WindowProc_s
      HWND hwnd = CreateWindowExW(0, pwszClassName_s, stringTitle.c_str(), uStyle, CW_USEDEFAULT, CW_USEDEFAULT, iWidth, iHeight, hwndParent, nullptr, hinstance, this);
      return hwnd != nullptr;
   }

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
   LRESULT Dispatch(UINT uMessage, WPARAM uParam, LPARAM iParam)
   {
      auto eEvent = gd_win::translate_s(uMessage); // translate to portable event, ignored here but useful for debugging
      if(eEvent != gd_win::eNone)
      {
         if (m_bitsetMessageMap.test(eEvent) == true)
         {
            return Call(uMessage, uParam, iParam);
         }
      }

      return CallDefault(uMessage, uParam, iParam);
   }

   static bool Register_s(HINSTANCE hinstance);


   /// The one and only window procedure, finds the CWindow for the HWND and forwards
   static LRESULT CALLBACK WindowProc_s(HWND hwnd, UINT uMessage, WPARAM uParam, LPARAM iParam);

   static void CrackMessage_s( gd_win::eWindowEvent eEvent, WPARAM uParam, LPARAM iParam, gd::argument::arguments& argumentsOut);


private:
   static constexpr const wchar_t* pwszClassName_s = L"win::CWindow";
   std::bitset<100> m_bitsetMessageMap{};
   HWND m_hwnd = nullptr;
};

