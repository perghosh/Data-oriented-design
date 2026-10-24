#include "FrameMain.h"


CFrameMain::CFrameMain()
{
   bind(gdwin::eWindowEvent::eMouseDown);                                      // bind the mouse down event to this window (WM_LBUTTONDOWN)
   bind(gdwin::eWindowEvent::eMouseMove);                                      // bind the mouse move event to this window (WM_MOUSEMOVE)
   bind(gdwin::eWindowEvent::eClose);                                          // bind the close event to this window (WM_CLOSE)
   bind(gdwin::eWindowEvent::eDestroy);                                        // bind the destroy event to this window (WM_DESTROY)
}

gdwin::lresult CFrameMain::on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   switch (eEvent)
   {
   case gdwin::eWindowEvent::eMouseMove: { OnMouseMove(*pargumentsParam); } break;
   case gdwin::eWindowEvent::eMouseDown: { OnMouseDown( *pargumentsParam ); } break;
   case gdwin::eWindowEvent::eKeyDown: { OnKeyDown( *pargumentsParam ); } break;
   case gdwin::eWindowEvent::eClose: { ::DestroyWindow(m_hwnd); } break;
   case gdwin::eWindowEvent::eDestroy: { ::PostQuitMessage(0); } break;
   }
   
   return gdwin::lresult();
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

void CFrameMain::Create()
{
   enum enumMenu {
      /* file */
      eOpen,
      eSave,
      eSaveAs,
      eExit,

      /* view */
      eRefresh,
      eClear,
      eShowLineNumbers,

      /* search */
      eFind,

      /* tools */
      eFilter,

      /* help */
      eAbout,
      eHelp
   };

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
   AppendMenu(hMenuFile, MF_STRING, eRefresh, "Refresh");
   AppendMenu(hMenuFile, MF_STRING, eClear, "Clear");
   AppendMenu(hMenuFile, MF_CHECKED, eShowLineNumbers, "Show line numbers");
   AppendMenu(hMenu, MF_POPUP, ( UINT_PTR )hMenuFile, "View");

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

