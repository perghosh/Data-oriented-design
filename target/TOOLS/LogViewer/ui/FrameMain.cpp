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
      eOpen,
      eSave,
      eExit
   };

   HMENU hMenuBar;
   HMENU hMenuFile;

   hMenuBar = ::CreateMenu();

   /* Create file menu */
   hMenuFile = ::CreateMenu();
   AppendMenu(hMenuFile, MF_STRING, eOpen, "Open");
   AppendMenu(hMenuFile, MF_STRING, eSave, "Save");
   AppendMenu(hMenuFile, MF_SEPARATOR, 0, NULL);
   AppendMenu(hMenuFile, MF_STRING, eExit, "Exit");
   AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuFile, "File");

   gdwin::window<>::create("Blank Window", { {"style", "overlappedwindow"}, {"width", 800}, {"height", 600},{ "callback", gdwin::window<>::get_callback_s() }});
}

