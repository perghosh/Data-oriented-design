// @FILE [tag: window, view] [summary: Implementation of the view class] [type: source] [name: gdwin_view.cpp]

#include <array>
#include <cassert>

#include "gdwin_view.h"

_GDWIN_BEGIN

/** --------------------------------------------------------------------------- view
 * @brief Binds the events a view needs. Events that are not bound go to default processing.
 * @param uFlags Combination of view::enumFlags
 */
view::view(unsigned uFlags) : m_uFlags(uFlags)
{
   m_colorrefBackground = ::GetSysColor(COLOR_WINDOW);

   bind(gdwin::eWindowEvent::eCreate);                                         // WM_NCCREATE and WM_CREATE
   bind(gdwin::eWindowEvent::eDestroy);                                        // WM_DESTROY and WM_NCDESTROY
   bind(gdwin::eWindowEvent::ePaint);
   bind(gdwin::eWindowEvent::eResize);
   bind(gdwin::eWindowEvent::eFocusIn);
   bind(gdwin::eWindowEvent::eFocusOut);
   bind(gdwin::eWindowEvent::eMouseMove);
   bind(gdwin::eWindowEvent::eMouseDown);
   bind(gdwin::eWindowEvent::eMouseUp);
   bind(gdwin::eWindowEvent::eMouseDoubleClick);
   bind(gdwin::eWindowEvent::eMouseWheel);
   bind(gdwin::eWindowEvent::eMouseLeave);
   bind(gdwin::eWindowEvent::eKeyDown);
   bind(gdwin::eWindowEvent::eKeyUp);
   bind(gdwin::eWindowEvent::eChar);
   bind(gdwin::eWindowEvent::eCommand);
   bind(gdwin::eWindowEvent::eContextMenu);
}

/** --------------------------------------------------------------------------- register_class_s
 * @brief Registers the window class used by views.
 *
 * register_window_g does not set a class style or clear the background brush, so views register their
 * own class. CS_DBLCLKS is needed for WM_LBUTTONDBLCLK and friends to be sent at all, and no
 * background brush means that the view paints every pixel itself (no erase flicker).
 */
bool view::register_class_s(HINSTANCE hinstance)
{
   WNDCLASSEXW wndclassex = {};
   if( ::GetClassInfoExW(hinstance, L"gdwin_view", &wndclassex) != FALSE ) return true; // already registered

   wndclassex = {};
   wndclassex.cbSize = sizeof(WNDCLASSEXW);
   wndclassex.style = CS_DBLCLKS;
   wndclassex.lpfnWndProc = reinterpret_cast<WNDPROC>( gdwin::window<>::get_callback_s() );
   wndclassex.hInstance = hinstance;
   wndclassex.hCursor = ::LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));       // IDC_ARROW
   wndclassex.hbrBackground = nullptr;
   wndclassex.lpszClassName = L"gdwin_view";
   return ::RegisterClassExW(&wndclassex) != 0;
}

/** --------------------------------------------------------------------------- create
 * @brief Creates the view as a child window.
 * @param hwndParent Parent window, the frame
 * @param iX position in parent client coordinates
 * @param iY position in parent client coordinates
 * @param iWidth width in pixels
 * @param iHeight height in pixels
 * @param uId child window id
 */
bool view::create(HWND hwndParent, int iX, int iY, int iWidth, int iHeight, uint32_t uId)
{                                                                                                  assert( hwndParent != nullptr );
   if( m_hwnd != nullptr ) return true;

   if( register_class_s(::GetModuleHandleW(nullptr)) == false ) return false;

   const uint32_t uStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
   const uint32_t uStyleEx = ( m_uFlags & eFlagClientEdge ) != 0 ? WS_EX_CLIENTEDGE : 0;

   std::array<std::byte, 256> arrayBuffer;
   gd::argument::arguments argumentsWindow{ arrayBuffer, { { "style", uStyle }, { "styleex", uStyleEx }, { "x", iX }, { "y", iY }, { "width", iWidth }, { "height", iHeight }, { "parent", (void*)hwndParent }, { "menu", (void*)(UINT_PTR)uId } } };
   return gdwin::window<>::create(std::string(get_class_name_s()), argumentsWindow);   // title is not used by a child window
}

/// Move and size the view
void view::move(const RECT& rectView, bool bRepaint)
{
   if( m_hwnd == nullptr ) return;
   ::MoveWindow(m_hwnd, rectView.left, rectView.top, rectView.right - rectView.left, rectView.bottom - rectView.top, bRepaint ? TRUE : FALSE);
}

/// Make the view cover the client area of the parent
void view::fit_to_parent()
{
   if( m_hwnd == nullptr ) return;
   HWND hwndParent = ::GetParent(m_hwnd);
   if( hwndParent == nullptr ) return;

   RECT rectParent;
   ::GetClientRect(hwndParent, &rectParent);
   move(rectParent);
}

RECT view::get_client_rect() const
{
   RECT rectClient = {};
   if( m_hwnd != nullptr ) ::GetClientRect(m_hwnd, &rectClient);
   return rectClient;
}

/** --------------------------------------------------------------------------- on_event
 * @brief Translates events to the virtual hooks of the view.
 *
 * Returns handled only for what the view consumed, everything else goes to default processing.
 * Several native messages share one event, therefore uNativeMessage is checked where it matters.
 */
gdwin::lresult view::on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   switch( eEvent )
   {
   case gdwin::eWindowEvent::eCreate:
   {
      if( uNativeMessage != WM_CREATE ) break;                                 // WM_NCCREATE is also eCreate, returning handled(0) there would cancel creation
      if( on_create() == false ) return handled_s(-1);                         // -1 cancels creation
      return handled_s();
   }

   case gdwin::eWindowEvent::eDestroy:
   {
      if( uNativeMessage == WM_DESTROY ) { on_destroy(); break; }

      if( uNativeMessage == WM_NCDESTROY )                                     // last message, let default processing run and then forget the handle
      {
         call_default(WM_NCDESTROY, 0, 0);
         ::SetWindowLongPtrW(m_hwnd, GWLP_USERDATA, 0);
         m_hwnd = nullptr;
         return handled_s();
      }
   }
   break;

   case gdwin::eWindowEvent::ePaint:
      paint_();
      return handled_s();

   case gdwin::eWindowEvent::eResize:
   {
      if( uNativeMessage != WM_SIZE ) break;                                   // WM_SIZING has a RECT in lParam
      m_iClientWidth = (*pargumentsParam)["width"].as_int();
      m_iClientHeight = (*pargumentsParam)["height"].as_int();
      on_size(m_iClientWidth, m_iClientHeight);
      return handled_s();
   }

   case gdwin::eWindowEvent::eFocusIn:
      on_focus(true);
      break;

   case gdwin::eWindowEvent::eFocusOut:
      on_focus(false);
      break;

   case gdwin::eWindowEvent::eMouseMove:
   {
      int iX = (*pargumentsParam)["x"].as_int();
      int iY = (*pargumentsParam)["y"].as_int();
      uint32_t uButtons = (*pargumentsParam)["buttons"].as_uint();

      if( m_bTrackingMouse == false )                                          // eMouseEnter has no native message, so it is generated here
      {
         TRACKMOUSEEVENT trackmouseevent = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hwnd, 0 };
         if( ::TrackMouseEvent(&trackmouseevent) != FALSE ) { m_bTrackingMouse = true; on_mouse_enter(); }
      }

      if( on_mouse_move(iX, iY, uButtons) == true ) return handled_s();
   }
   break;

   case gdwin::eWindowEvent::eMouseLeave:
      m_bTrackingMouse = false;
      on_mouse_leave();
      return handled_s();

   case gdwin::eWindowEvent::eMouseDown:
   case gdwin::eWindowEvent::eMouseUp:
   case gdwin::eWindowEvent::eMouseDoubleClick:
   {
      int iX = (*pargumentsParam)["x"].as_int();
      int iY = (*pargumentsParam)["y"].as_int();
      uint32_t uButtons = (*pargumentsParam)["buttons"].as_uint();            // buttons that are down
      gdwin::eMouseButton eButton = static_cast<gdwin::eMouseButton>((*pargumentsParam)["button"].as_uint()); // button that caused the event

      bool bHandled = false;
      if( eEvent == gdwin::eWindowEvent::eMouseDown )
      {
         if( ::GetFocus() != m_hwnd ) ::SetFocus(m_hwnd);                      // a click gives the view keyboard focus
         bHandled = on_mouse_down(eButton, iX, iY, uButtons);
      }
      else if( eEvent == gdwin::eWindowEvent::eMouseUp ) { bHandled = on_mouse_up(eButton, iX, iY, uButtons); }
      else { bHandled = on_mouse_double_click(eButton, iX, iY, uButtons); }

      if( bHandled == true ) return handled_s( eButton >= gdwin::eMouseButtonBack ? 1 : 0 ); // WM_XBUTTON* must return TRUE when handled
   }
   break;

   case gdwin::eWindowEvent::eMouseWheel:
   {
      POINT pointClient = { (*pargumentsParam)["x"].as_int(), (*pargumentsParam)["y"].as_int() }; // wheel position is in screen coordinates
      ::ScreenToClient(m_hwnd, &pointClient);
      int iDelta = (*pargumentsParam)["delta"].as_int();
      if( on_mouse_wheel(iDelta, (*pargumentsParam)["horizontal"].as_bool(), pointClient.x, pointClient.y) == true ) return handled_s();
   }
   break;

   case gdwin::eWindowEvent::eKeyDown:
   case gdwin::eWindowEvent::eKeyUp:
   {
      if( uNativeMessage == WM_SYSKEYDOWN || uNativeMessage == WM_SYSKEYUP ) break; // system keys are not for the view, so ALT+key menu shortcuts keep working
      uint32_t uKey = (*pargumentsParam)["key"].as_uint();
      uint32_t uRepeat = (*pargumentsParam)["repeat"].as_uint();
      bool bHandled = eEvent == gdwin::eWindowEvent::eKeyDown ? on_key_down(uKey, uRepeat) : on_key_up(uKey);
      if( bHandled == true ) return handled_s();
   }
   break;

   case gdwin::eWindowEvent::eChar:
   {
      if( uNativeMessage != WM_CHAR ) break;                                   // dead chars and system chars use default processing
      if( on_char((*pargumentsParam)["char"].as_uint(), (*pargumentsParam)["repeat"].as_uint()) == true ) return handled_s();
   }
   break;

   case gdwin::eWindowEvent::eContextMenu:
   {
      int iX = (*pargumentsParam)["x"].as_int();
      int iY = (*pargumentsParam)["y"].as_int();
      if( (*pargumentsParam)["keyboard"].as_bool() == true )                   // keyboard invoked (shift+F10 or menu key), use center of view
      {
         RECT rectClient = get_client_rect();
         POINT pointCenter = { ( rectClient.right - rectClient.left ) / 2, ( rectClient.bottom - rectClient.top ) / 2 };
         ::ClientToScreen(m_hwnd, &pointCenter);
         iX = pointCenter.x;
         iY = pointCenter.y;
      }
      if( on_context_menu(iX, iY) == true ) return handled_s();
   }
   break;

   case gdwin::eWindowEvent::eCommand:
   {
      if( uNativeMessage != WM_COMMAND ) break;                                // eCommand also maps WM_SYSCOMMAND

      uint32_t uId = (*pargumentsParam)["id"].as_uint();
      uint32_t uNotifyCode = (*pargumentsParam)["notify_code"].as_uint();
      if( on_command(uId, uNotifyCode) == true ) return handled_s();

      if( uNotifyCode <= 1 )                                                   // 0 = menu, 1 = accelerator, pass on to the frame. Control notifications stay with the view
      {
         HWND hwndParent = ::GetParent(m_hwnd);
         if( hwndParent != nullptr ) { ::SendMessageW(hwndParent, WM_COMMAND, MAKEWPARAM(uId, uNotifyCode), 0); return handled_s(); }
      }
   }
   break;

   default:
      break;
   }

   return gdwin::lresult();                                                    // not handled, default processing
}

/** --------------------------------------------------------------------------- paint_
 * @brief Handles WM_PAINT. Fills the background and calls on_draw, optionally through a memory bitmap.
 *
 * The view is registered without background brush, so WM_ERASEBKGND does nothing and all pixels
 * are written here. With double buffer the bitmap is only as large as the invalid area.
 */
void view::paint()
{
   PAINTSTRUCT paintstruct;
   HDC hdcPaint = ::BeginPaint(m_hwnd, &paintstruct);
   const RECT rectClip = paintstruct.rcPaint;
   const int iWidth = rectClip.right - rectClip.left;
   const int iHeight = rectClip.bottom - rectClip.top;

   if( iWidth > 0 && iHeight > 0 )
   {
      bool bBuffered = false;
      if( is_double_buffer() == true )
      {
         HDC hdcMemory = ::CreateCompatibleDC(hdcPaint);
         HBITMAP hbitmapMemory = hdcMemory != nullptr ? ::CreateCompatibleBitmap(hdcPaint, iWidth, iHeight) : nullptr;
         if( hbitmapMemory != nullptr )
         {
            HGDIOBJ hbitmapOld = ::SelectObject(hdcMemory, hbitmapMemory);
            ::SetViewportOrgEx(hdcMemory, -rectClip.left, -rectClip.top, nullptr); // on_draw uses client coordinates
            fill_background_(hdcMemory, rectClip);
            on_draw(hdcMemory, rectClip);
            ::BitBlt(hdcPaint, rectClip.left, rectClip.top, iWidth, iHeight, hdcMemory, rectClip.left, rectClip.top, SRCCOPY);
            ::SelectObject(hdcMemory, hbitmapOld);
            ::DeleteObject(hbitmapMemory);
            bBuffered = true;
         }
         if( hdcMemory != nullptr ) ::DeleteDC(hdcMemory);
      }

      if( bBuffered == false )                                                 // no double buffer or out of gdi resources, draw directly
      {
         fill_background_(hdcPaint, rectClip);
         on_draw(hdcPaint, rectClip);
      }
   }

   ::EndPaint(m_hwnd, &paintstruct);
}

/// Fill rectangle with the background color
void view::fill_background(HDC hdc, const RECT& rectFill) const
{
   ::SetDCBrushColor(hdc, m_colorrefBackground);
   ::FillRect(hdc, &rectFill, static_cast<HBRUSH>(::GetStockObject(DC_BRUSH)));
}

_GDWIN_END
