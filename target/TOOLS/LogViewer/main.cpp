// application.cpp : Defines the entry point for the application.
//

          
#include "window.h"

#include "os/OS_Event.h"
#include "ui/FrameMain.h"
#include "application.h"
#include "windowapplication.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   gd_win::event_map_registry registryMaps;
   //registryMaps.append({ "Mouse", std::bitset<gd_win::uMaxMessageId>(0x);

   CFrameMain framemain;
   if(framemain.Create(L"Blank Window", 800, 600) == false) return 0;

   framemain.Show(nCmdShow);
   return CWindow::Run_s();
}


void RegisterWindowMap(gd_win::event_map_registry & registryMaps)
{
   //registryMaps.append({ "main", );
}
