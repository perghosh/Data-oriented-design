#include "FrameMain.h"


CFrameMain::CFrameMain()
{
   bind(gdwin::eWindowEvent::eClose);                                          // bind the close event to this window (WM_CLOSE)
   bind(gdwin::eWindowEvent::eDestroy);                                        // bind the destroy event to this window (WM_DESTROY)
}

gdwin::lresult CFrameMain::on_event(gdwin::eWindowEvent eEvent, gd::argument::arguments* pargumentsParam, uint64_t uNativeMessage)
{
   switch (eEvent)
   {
   case gdwin::eWindowEvent::eClose: { ::DestroyWindow(m_hwnd); } break;
   case gdwin::eWindowEvent::eDestroy: { ::PostQuitMessage(0); } break;

   }
   
   return gdwin::lresult();
}

void CFrameMain::OnMouseDown(UINT, POINT point)
{
   OutputDebugStringA(("Left button down at: (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")\n").c_str());
}

void CFrameMain::OnMouseMove(UINT, POINT point)
{
   OutputDebugStringA(("Left button move at: (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")\n").c_str());
}

