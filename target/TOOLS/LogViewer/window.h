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

#include "os/OS_Event.h"


// ## Message cracking ----------------------------------------------------------------------------
// CMessage<> turns WPARAM/LPARAM into the arguments a handler actually wants.
// Messages that are not listed here get the raw (WPARAM, LPARAM) pair.
// Support a new message by specializing CMessage<> with `arguments` and `Crack_s`.

template<UINT uMessage>
struct CMessage
{
   using arguments = std::tuple<WPARAM, LPARAM>;
   static arguments Crack_s(WPARAM uParam, LPARAM iParam) { return { uParam, iParam }; }
};

struct CMessageNone
{
   using arguments = std::tuple<>;
   static arguments Crack_s(WPARAM, LPARAM) { return {}; }
};

struct CMessageMouse // ( key flags, client position )
{
   using arguments = std::tuple<UINT, POINT>;
   static arguments Crack_s(WPARAM uParam, LPARAM iParam)
   {
      POINT point = { GET_X_LPARAM(iParam), GET_Y_LPARAM(iParam) }; // signed, works on negative coordinates
      return { static_cast<UINT>(uParam), point };
   }
};

struct CMessageKey // ( virtual key, repeat count )
{
   using arguments = std::tuple<UINT, UINT>;
   static arguments Crack_s(WPARAM uParam, LPARAM iParam)
   {
      return { static_cast<UINT>(uParam), static_cast<UINT>(LOWORD(iParam)) };
   }
};

template<> struct CMessage<WM_PAINT> : CMessageNone {};
template<> struct CMessage<WM_CLOSE> : CMessageNone {};
template<> struct CMessage<WM_DESTROY> : CMessageNone {};
template<> struct CMessage<WM_LBUTTONDOWN> : CMessageMouse {};
template<> struct CMessage<WM_LBUTTONUP> : CMessageMouse {};
template<> struct CMessage<WM_RBUTTONDOWN> : CMessageMouse {};
template<> struct CMessage<WM_RBUTTONUP> : CMessageMouse {};
template<> struct CMessage<WM_MOUSEMOVE> : CMessageMouse {};
template<> struct CMessage<WM_KEYDOWN> : CMessageKey {};
template<> struct CMessage<WM_KEYUP> : CMessageKey {};

template<> struct CMessage<WM_SIZE> // ( size type, client size )
{
   using arguments = std::tuple<UINT, SIZE>;
   static arguments Crack_s(WPARAM uParam, LPARAM iParam)
   {
      SIZE size = { LOWORD(iParam), HIWORD(iParam) };
      return { static_cast<UINT>(uParam), size };
   }
};

template<> struct CMessage<WM_COMMAND> // ( command id, notification code, control window )
{
   using arguments = std::tuple<UINT, UINT, HWND>;
   static arguments Crack_s(WPARAM uParam, LPARAM iParam)
   {
      return { LOWORD(uParam), HIWORD(uParam), reinterpret_cast<HWND>(iParam) };
   }
};

namespace detail {
   // true if METHOD can be called as method( object, arguments... ), used to give a readable compile error
   template<typename METHOD, typename OBJECT, typename TUPLE> struct CBindable : std::false_type {};
   template<typename METHOD, typename OBJECT, typename... ARGUMENTS>
   struct CBindable<METHOD, OBJECT, std::tuple<ARGUMENTS...>> : std::is_invocable<METHOD, OBJECT, ARGUMENTS...> {};
}

// ## CPaint --------------------------------------------------------------------------------------
// RAII for BeginPaint/EndPaint, converts to HDC so it can be passed directly to GDI functions

class CPaint
{
public:
   explicit CPaint(HWND hwnd) : m_hwnd(hwnd) { m_hdc = BeginPaint(m_hwnd, &m_paintstruct); }
   ~CPaint() { EndPaint(m_hwnd, &m_paintstruct); }
   CPaint(const CPaint&) = delete;
   CPaint& operator=(const CPaint&) = delete;

   operator HDC() const { return m_hdc; }
   const RECT& GetRect() const { return m_paintstruct.rcPaint; }

private:
   HWND m_hwnd;
   HDC m_hdc = nullptr;
   PAINTSTRUCT m_paintstruct = {};
};

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

   /// Connect a message to a method: Bind<WM_LBUTTONDOWN>( this, &CView::OnLButtonDown );
   /// The method gets the cracked arguments from CMessage<uMessage>, it can return void or LRESULT.
   /// void means "handled, result 0". Messages without a binding go to DefWindowProc.
   template<UINT uMessage, typename OBJECT, typename METHOD>
   void Bind(OBJECT* pobject, METHOD pmethod)
   {
      using cracker = CMessage<uMessage>;
      static_assert(detail::CBindable<METHOD, OBJECT*, typename cracker::arguments>::value,
         "handler signature does not match the arguments of this message, see CMessage<> for the message");

      m_mapHandler[uMessage] = [pobject, pmethod](WPARAM uParam, LPARAM iParam) -> LRESULT
         {
            auto tupleArgument = std::tuple_cat(std::make_tuple(pobject), cracker::Crack_s(uParam, iParam));
            if constexpr(std::is_void_v<decltype(std::apply(pmethod, tupleArgument))>)
            {
               std::apply(pmethod, tupleArgument);
               return 0;
            }
            else
            {
               return static_cast<LRESULT>(std::apply(pmethod, tupleArgument));
            }
         };
   }

   /// Create the window, message bindings should be done before this call so WM_CREATE is seen
   bool Create(const std::wstring& stringTitle, int iWidth, int iHeight, DWORD uStyle = WS_OVERLAPPEDWINDOW, HWND hwndParent = nullptr)
   {
      HINSTANCE hinstance = GetModuleHandleW(nullptr);
      if(Register_s(hinstance) == false) return false;

      // `this` travels in lpParam and is picked up in WM_NCCREATE by WindowProc_s
      HWND hwnd = CreateWindowExW(0, pwszClassName_s, stringTitle.c_str(), uStyle,
         CW_USEDEFAULT, CW_USEDEFAULT, iWidth, iHeight, hwndParent, nullptr, hinstance, this);
      return hwnd != nullptr;
   }

   HWND GetHandle() const { return m_hwnd; }
   void Show(int iCommand = SW_SHOW) { ShowWindow(m_hwnd, iCommand); UpdateWindow(m_hwnd); }
   void Invalidate(bool bErase = true) { InvalidateRect(m_hwnd, nullptr, bErase ? TRUE : FALSE); }

   /// Call from a handler when default processing should run as well
   LRESULT Default(UINT uMessage, WPARAM uParam, LPARAM iParam) const { return DefWindowProcW(m_hwnd, uMessage, uParam, iParam); }

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
      if(eEvent != gd_win::eEventNone)
      {
         if (m_pbitsetMessageMap->test(eEvent) == false)
         {
            OutputDebugStringA((std::to_string(uMessage) + " " + std::to_string(eEvent) + "\n").c_str());
         }
      }


      auto it = m_mapHandler.find(uMessage);
      if(it == m_mapHandler.end()) return Default(uMessage, uParam, iParam);
      return it->second(uParam, iParam);
   }

   static bool Register_s(HINSTANCE hinstance);

   /// The one and only window procedure, finds the CWindow for the HWND and forwards
   static LRESULT CALLBACK WindowProc_s(HWND hwnd, UINT uMessage, WPARAM uParam, LPARAM iParam);

private:
   static constexpr const wchar_t* pwszClassName_s = L"win::CWindow";
   std::bitset<100>* m_pbitsetMessageMap{};
   HWND m_hwnd = nullptr;
   std::unordered_map<UINT, handler> m_mapHandler;
};

