#include "FrameMain.h"


CFrameMain::CFrameMain()
{
   // message -> method, the method signature is checked against the message at compile time
   //Bind<WM_KEYDOWN>(this, &CFrameMain::OnKeyDown);
   Bind<gd_win::eWindowEvent::eMouseDown>(this, &CFrameMain::OnMouseDown);
   Bind<gd_win::eWindowEvent::eMouseMove>(this, &CFrameMain::OnMouseMove);
   //Bind<WM_PAINT>(this, &CFrameMain::OnPaint);
   //Bind<WM_DESTROY>(this, &CFrameMain::OnDestroy);
}

void CFrameMain::OnMouseDown(UINT, POINT point)
{
   OutputDebugStringA(("Left button down at: (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")\n").c_str());
}

void CFrameMain::OnMouseMove(UINT, POINT point)
{
   OutputDebugStringA(("Left button move at: (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")\n").c_str());
}

