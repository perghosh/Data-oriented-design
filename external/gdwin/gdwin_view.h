// @FILE [tag: window, view] [summary: Base class for a view, a child window that fills the client area of a frame] [description: Child window that is hosted by a main frame, child frame, splitter pane or tab page. Handles painting, size, mouse, keyboard and command routing.] [name: gdwin_view.h] [type: header]

/**
 * @file gdwin_view.h
 *
 * @brief Base class for views hosted inside a frame window (draft).
 *
 * A view is a plain child window. The frame owns the layout: it creates the view, resizes it
 * when the frame is resized, forwards focus to it and offers it menu commands first.
 * The view does not know what kind of parent it has, so the same class works in a main frame,
 * an MDI child frame, a splitter pane or a tab page.
 *
 *    register__            register the window class of the view
 *    paint__               painting, optional double buffering
 *    input__               mouse and keyboard hooks
 *    routing__             command routing between view and frame
 */

#pragma once

#include <cstdint>
#include <string_view>
#include <utility>

#include "gdwin_window.h"

#ifndef _GDWIN_BEGIN
#define _GDWIN_BEGIN namespace gdwin {
#define _GDWIN_END }
#endif

_GDWIN_BEGIN

class view : public gdwin::window<>
{
public:
   enum enumFlags : unsigned
   {
      eFlagUnknown      = 0x00, ///< no flags
      eFlagDoubleBuffer = 0x01, ///< paint to memory bitmap and copy to screen, no flicker
      eFlagClientEdge   = 0x02, ///< sunken border like the view in a classic frame window
   };

   // @API [tag: construction]
public:
   view(unsigned uFlags = eFlagDoubleBuffer);
   view(const view&) = delete;
   view& operator=(const view&) = delete;
   ~view() override = default;

   // @API [tag: window]
public:
   gdwin::lresult on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage) override;
   std::pair<bool, std::string_view> get_window_class() const override { return { true, get_class_name_s() }; } // true = registered by view::create

   // @API [tag: operation]
public:
   /// Create the view as a child of hwndParent. uId is the child id (sent as hMenu), 0 if not needed
   bool create(HWND hwndParent, int iX, int iY, int iWidth, int iHeight, uint32_t uId = 0);

   /// Move and size the view, this is what the frame calls from its resize handling
   void move(const RECT& rectView, bool bRepaint = true);
   /// Make the view fill the whole client area of its parent
   void fit_to_parent();

   RECT get_client_rect() const;
   void set_background(COLORREF colorref) { m_colorrefBackground = colorref; if( m_hwnd != nullptr ) invalidate(false); }
   COLORREF get_background() const { return m_colorrefBackground; }
   bool is_double_buffer() const { return ( m_uFlags & eFlagDoubleBuffer ) != 0; }

   void set_focus() { if( m_hwnd != nullptr ) ::SetFocus(m_hwnd); }
   void capture_mouse() { ::SetCapture(m_hwnd); }
   void release_mouse() { if( ::GetCapture() == m_hwnd ) ::ReleaseCapture(); }

   // @API [tag: routing]
public:
   /// Frame calls this to give the view the first chance to handle a menu or accelerator command. Return true if handled.
   virtual bool on_command(uint32_t uId, uint32_t uNotifyCode) { return false; }

   // @API [tag: hooks]
protected:
   /// Called for WM_CREATE, create child controls here. Return false to cancel creation
   virtual bool on_create() { return true; }
   /// Called for WM_DESTROY, window handle is still valid
   virtual void on_destroy() {}
   /// Draw the view. Background is already filled, coordinates are client coordinates, rectClip is the area that needs painting
   virtual void on_draw(HDC hdc, const RECT& rectClip) {}
   /// Client area got a new size. Width and height are 0 when the frame is minimized
   virtual void on_size(int iWidth, int iHeight) {}
   virtual void on_focus(bool bGained) {}

   // Input hooks, return true if handled. uButtons is a bit mask of the buttons that are down, bit position is gdwin::eMouseButton
   virtual void on_mouse_enter() {}
   virtual void on_mouse_leave() {}
   virtual bool on_mouse_move(int iX, int iY, uint32_t uButtons) { return false; }
   virtual bool on_mouse_down(gdwin::eMouseButton eButton, int iX, int iY, uint32_t uButtons) { return false; }
   virtual bool on_mouse_up(gdwin::eMouseButton eButton, int iX, int iY, uint32_t uButtons) { return false; }
   virtual bool on_mouse_double_click(gdwin::eMouseButton eButton, int iX, int iY, uint32_t uButtons) { return false; }
   /// iDelta is a multiple of WHEEL_DELTA, position is client coordinates
   virtual bool on_mouse_wheel(int iDelta, bool bHorizontal, int iX, int iY) { return false; }
   virtual bool on_key_down(uint32_t uKey, uint32_t uRepeat) { return false; }
   virtual bool on_key_up(uint32_t uKey) { return false; }
   virtual bool on_char(uint32_t uChar, uint32_t uRepeat) { return false; }
   /// Position is screen coordinates, if invoked from keyboard it is the center of the view
   virtual bool on_context_menu(int iX, int iY) { return false; }

   // @API [tag: static]
public:
   static const char* get_class_name_s() { return "gdwin_view"; }
   /// Register the window class of the view (double click, no background brush). Safe to call more than once
   static bool register_class_s(HINSTANCE hinstance);

protected:
   static gdwin::lresult handled_s(int iResult = 0) { return gdwin::lresult{ gdwin::lresult::enumState::eHandled, iResult }; }

private:
   void paint();
   void fill_background(HDC hdc, const RECT& rectFill) const;

   // ## attributes ----------------------------------------------------------------
protected:
   unsigned m_uFlags = 0;                          ///< see enumFlags
   COLORREF m_colorrefBackground = 0;              ///< color used to fill the background before on_draw
   int m_iClientWidth = 0;                         ///< last known width of client area
   int m_iClientHeight = 0;                        ///< last known height of client area
   bool m_bTrackingMouse = false;                  ///< true while waiting for WM_MOUSELEAVE
};

_GDWIN_END
