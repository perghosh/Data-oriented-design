// application.cpp : Defines the entry point for the application.
//

          
#include "window.h"

#include "os/OS_Event.h"
#include "application.h"
#include "windowapplication.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   gd_win::registry registryMaps;
   registryMaps.append({ "Mouse", std::bitset<gd_win::registry::m_uMaxEventId_s>(0x));

   CDrawWindow window;
   if(window.Create(L"Blank Window", 800, 600) == false) return 0;

   window.Show(nCmdShow);
   return CWindow::Run_s();
}
