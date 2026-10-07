#include "FrameMain.h"


CFrameMain::CFrameMain()
{
   Bind(gd_win::eWindowEvent::eMouseDown);
   Bind(gd_win::eWindowEvent::eMouseMove);
}

void CFrameMain::OnMouseDown(UINT, POINT point)
{
   OutputDebugStringA(("Left button down at: (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")\n").c_str());
}

void CFrameMain::OnMouseMove(UINT, POINT point)
{
   OutputDebugStringA(("Left button move at: (" + std::to_string(point.x) + ", " + std::to_string(point.y) + ")\n").c_str());
}

