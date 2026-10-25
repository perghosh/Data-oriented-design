
#include "DialogAbout.h"
#include "DialogHelp.h"

#include "FrameMain.h"
#include <commctrl.h>

#pragma comment(lib, "Comctl32.lib")


enum enumMenu {
   /* file */
   eOpen = 1000,
   eSave,
   eSaveAs,
   eExit,

   /* view */
   eRefresh = 2000,
   eClear,
   eShowLineNumbers,

   /* search */
   eFind = 3000,

   /* tools */
   eFilter = 4000,

   /* help */
   eAbout = 5000,
   eHelp
};


CFrameMain::CFrameMain()
{
   bind(gdwin::eWindowEvent::eCommand);
   bind(gdwin::eWindowEvent::eResize);
   bind(gdwin::eWindowEvent::eMouseDown);                                      // bind the mouse down event to this window (WM_LBUTTONDOWN)
   //bind(gdwin::eWindowEvent::eMouseMove);                                      // bind the mouse move event to this window (WM_MOUSEMOVE)
   bind(gdwin::eWindowEvent::eClose);                                          // bind the close event to this window (WM_CLOSE)
   bind(gdwin::eWindowEvent::eDestroy);                                        // bind the destroy event to this window (WM_DESTROY)
}

CFrameMain::~CFrameMain()
{
   if( m_hMenu != nullptr )
   {
      if( m_hwnd != nullptr && ::GetMenu(m_hwnd) == m_hMenu ) { ::SetMenu(m_hwnd, nullptr); }
      ::DestroyMenu(m_hMenu);
      m_hMenu = nullptr;
   }
}

gdwin::lresult CFrameMain::on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   switch (eEvent)
   {
   case gdwin::eWindowEvent::eMouseMove: { OnMouseMove(*pargumentsParam); } break;
   case gdwin::eWindowEvent::eMouseDown: { OnMouseDown( *pargumentsParam ); } break;
   case gdwin::eWindowEvent::eKeyDown: { OnKeyDown( *pargumentsParam ); } break;
   case gdwin::eWindowEvent::eCommand: { OnCommand( *pargumentsParam ); } break;
   case gdwin::eWindowEvent::eResize:
      if( m_hwndStatusBar != nullptr ) { ::SendMessageW(m_hwndStatusBar, WM_SIZE, 0, 0); }
      break;
   case gdwin::eWindowEvent::eClose: { ::DestroyWindow(m_hwnd); } break;
   case gdwin::eWindowEvent::eDestroy: { m_hMenu = nullptr; m_hwndStatusBar = nullptr; ::PostQuitMessage(0); } break;
   }
   
   return gdwin::lresult();
}

void CFrameMain::Create()
{

   HMENU hMenu;
   HMENU hSubMenu;

   hMenu = ::CreateMenu();

   /* Create file menu */
   hSubMenu = ::CreateMenu();
   AppendMenu(hSubMenu, MF_STRING, eOpen, "Open");
   AppendMenu(hSubMenu, MF_STRING, eSave, "Save");
   AppendMenu(hSubMenu, MF_STRING, eSaveAs, "Save as");
   AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
   AppendMenu(hSubMenu, MF_STRING, eExit, "Exit");
   AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSubMenu, "File");

   /* Create view menu */
   hSubMenu = ::CreateMenu();
   AppendMenu(hSubMenu, MF_STRING, eRefresh, "Refresh");
   AppendMenu(hSubMenu, MF_STRING, eClear, "Clear");
   AppendMenu(hSubMenu, MF_CHECKED, eShowLineNumbers, "Show line numbers");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hSubMenu, "View");

   /* Create search menu */
   hSubMenu = ::CreateMenu();
   AppendMenu(hSubMenu, MF_STRING, eFind, "Find");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hSubMenu, "Search");

   /* Create tools menu */
   hSubMenu = ::CreateMenu();
   AppendMenu(hSubMenu, MF_STRING, eFilter, "Filter");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hSubMenu, "Tools");

   /* Create help menu */
   hSubMenu = ::CreateMenu();
   AppendMenu(hSubMenu, MF_STRING, eAbout, "About");
   AppendMenu(hSubMenu, MF_STRING, eHelp, "Help");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hSubMenu, "Help");

   m_hMenu = hMenu;

   INITCOMMONCONTROLSEX initCommonControls_{};
   initCommonControls_.dwSize = sizeof(initCommonControls_);
   initCommonControls_.dwICC = ICC_BAR_CLASSES;
   if( ::InitCommonControlsEx(&initCommonControls_) == FALSE )
   {
      OutputDebugStringA("Could not initialize common controls for the status bar.\n");
      return;
   }

   bool bCreated = gdwin::window<>::create("Blank Window", { {"style", "overlappedwindow"}, {"width", 800}, {"height", 600},  { "callback", gdwin::window<>::get_callback_s() }, { "menu", m_hMenu }});
   if( bCreated == false )
   {
      OutputDebugStringA("Could not create the main window.\n");
      return;
   }

   m_hwndStatusBar = ::CreateWindowExW(0, STATUSCLASSNAMEW, L"Ready",
      WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
      m_hwnd, nullptr, ::GetModuleHandleW(nullptr), nullptr);
   if( m_hwndStatusBar == nullptr )
   {
      OutputDebugStringA("Could not create the status bar.\n");
      return;
   }

   SetStatusText("Ready");
   ::SendMessageA(m_hwndStatusBar, WM_SIZE, 0, 0);
 }

 void CFrameMain::SetStatusText(const char* pszText)
 {
   if( m_hwndStatusBar != nullptr )
      ::SendMessageA(m_hwndStatusBar, SB_SETTEXTA, 0, reinterpret_cast<LPARAM>(pszText));
 }

 void CFrameMain::OnMouseMove(gd::argument::arguments& arguments)
{
   OutputDebugStringA(("Left button move at: (" + arguments["x"].as_string() + ", " + arguments["y"].as_string() + ")\n").c_str());
}


void CFrameMain::OnMouseDown(gd::argument::arguments& arguments)
{
   OutputDebugStringA(("Left button down at: (" + arguments["x"].as_string() + ", " + arguments["y"].as_string() + ")\n").c_str());
}

void CFrameMain::OnKeyDown(gd::argument::arguments& arguments)
{
   OutputDebugStringA(("Key down: " + arguments["key"].as_string() + "\n").c_str());
}


void CFrameMain::OnCommand(gd::argument::arguments& arguments)
{
   uint32_t uCommandId = arguments["id"].as_uint();
   char pszMenuText[128] = {};
   if( m_hMenu != nullptr && ::GetMenuStringA(m_hMenu, uCommandId, pszMenuText, 128, MF_BYCOMMAND) > 0 )
   {
      SetStatusText(pszMenuText);                                             // text of the menu item matching the command id
   }
   if( uCommandId == eAbout )
   {
      CDialogAbout dialogAbout;
      if( dialogAbout.do_modal(m_hwnd) == -1 )
      {
         OutputDebugStringA("About dialog failed to create.\n");
      }
   }
   else if( uCommandId == eHelp )
   {
      CDialogHelp dialogHelp;
      if( dialogHelp.do_modal(m_hwnd) == -1 )
      {
         OutputDebugStringA("Help dialog failed to create.\n");
      }
   }

   OutputDebugStringA(("Command: " + arguments["id"].as_string() + "\n").c_str());
}  
