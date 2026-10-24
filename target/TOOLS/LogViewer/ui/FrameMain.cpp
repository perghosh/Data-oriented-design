#include "FrameMain.h"


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
   case gdwin::eWindowEvent::eClose: { ::DestroyWindow(m_hwnd); } break;
   case gdwin::eWindowEvent::eDestroy: { m_hMenu = nullptr; ::PostQuitMessage(0); } break;
   }
   
   return gdwin::lresult();
}

void CFrameMain::Create()
{

   HMENU hMenu;
   HMENU hMenuFile;
   HMENU hMenuView;
   HMENU hMenuSearch;
   HMENU hMenuTools;
   HMENU hMenuHelp;

   hMenu = ::CreateMenu();

   /* Create file menu */
   hMenuFile = ::CreateMenu();
   AppendMenu(hMenuFile, MF_STRING, eOpen, "Open");
   AppendMenu(hMenuFile, MF_STRING, eSave, "Save");
   AppendMenu(hMenuFile, MF_STRING, eSaveAs, "Save as");
   AppendMenu(hMenuFile, MF_SEPARATOR, 0, NULL);
   AppendMenu(hMenuFile, MF_STRING, eExit, "Exit");
   AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMenuFile, "File");

   /* Create view menu */
   hMenuView = ::CreateMenu();
   AppendMenu(hMenuView, MF_STRING, eRefresh, "Refresh");
   AppendMenu(hMenuView, MF_STRING, eClear, "Clear");
   AppendMenu(hMenuView, MF_CHECKED, eShowLineNumbers, "Show line numbers");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hMenuView, "View");

   /* Create search menu */
   hMenuSearch = ::CreateMenu();
   AppendMenu(hMenuSearch, MF_STRING, eFind, "Find");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hMenuSearch, "Search");

   /* Create tools menu */
   hMenuTools = ::CreateMenu();
   AppendMenu(hMenuTools, MF_STRING, eFilter, "Filter");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hMenuTools, "Tools");

   /* Create help menu */
   hMenuHelp = ::CreateMenu();
   AppendMenu(hMenuHelp, MF_STRING, eAbout, "About");
   AppendMenu(hMenuHelp, MF_STRING, eHelp, "Help");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hMenuHelp, "Help");

   m_hMenu = hMenu;

   gdwin::window<>::create("Blank Window", { {"style", "overlappedwindow"}, {"width", 800}, {"height", 600},  { "callback", gdwin::window<>::get_callback_s() }, { "menu", m_hMenu }});

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
   OutputDebugStringA(("Command: " + arguments["id"].as_string() + "\n").c_str());
}  
