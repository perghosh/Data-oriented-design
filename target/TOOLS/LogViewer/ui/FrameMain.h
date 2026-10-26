// @FILE [tag: application, window] [summary: Application window] [name: FrameMain.h] [type: header]

#pragma once

#include <memory>


//#include "../window.h"

#include "gdwin/control/gdwin_control_menu.h"

#include "gdwin/gdwin_window.h"

/** @CLASS [name: CFrameMain] [description:  ]
 * \brief
 *
 *
 \code
 CFrameMain framemain;
 if(framemain.Create() == false) { return -1; }
 RECT rectContent = framemain.GetContentRect();                               // area left for the main view
 \endcode
 */
class CFrameMain : public gdwin::window<>
{
   // @API [tag: construction]
public:
   CFrameMain();
   // copy
   CFrameMain(const CFrameMain& o) { common_construct(o); }
   CFrameMain(CFrameMain&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CFrameMain& operator=(const CFrameMain& o) { common_construct(o); return *this; }
   CFrameMain& operator=(CFrameMain&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CFrameMain() override;
private:
   // common copy
   void common_construct(const CFrameMain& o) {}
   void common_construct(CFrameMain&& o) noexcept {}

// @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]
   gdwin::lresult on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage) override;
   std::pair<bool, std::string_view> get_window_class() const override { return { false, std::string_view("CFrameMain")}; }

   /// Create menu, window, rebar and status bar. Returns false if anything failed, no window is left behind
   bool Create();

   /// Position rebar and status bar and calculate the content rectangle, called on resize and when the rebar height changes
   void Layout();

protected:
// @API [tag: internal]
   bool InitializeControls();                                                  // common controls classes needed for rebar and status bar
   bool CreateMenuBar();
   bool CreateMainWindow();
   bool CreateRebar();
   bool CreateStatusBar();

   /// Create a child control in the frame, size and position is set by Layout or by the parent control
   HWND CreateChild(const wchar_t* pwszClass, const wchar_t* pwszText, DWORD uStyle, int iId, HWND hwndParent = nullptr);
   /// Add a band with a child window to the rebar
   bool InsertRebarBand(HWND hwndChild, const wchar_t* pwszTitle, int iMinWidth, int iMinHeight, int iWidth);


   void OnMouseMove(gd::argument::arguments& arguments);
   void OnMouseDown(gd::argument::arguments& arguments);
   void OnKeyDown(gd::argument::arguments& arguments);
   void OnCommand(gd::argument::arguments& arguments);
   void OnNotify(gd::argument::arguments& arguments);
   
   void SetStatusText(const char* pszText);
   void ToggleMenuCheck(uint32_t uCommandId);                                  // flip check mark for menu item
   void LogError(const char* pszMessage) const;                                // write message and last error to debug output


 public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
  HMENU m_hMenu = nullptr; ///< menu handle
  HWND m_hwndRebar = nullptr; ///< rebar window handle
  HWND m_hwndStatusBar = nullptr; ///< status bar window handle
  RECT m_rectClient = {};  ///< client area between rebar and status bar


// @API [tag: free-functions]
public:



};



