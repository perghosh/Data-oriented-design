// application.cpp : Defines the entry point for the application.
//

          
#include "window.h"

#include "os/OS_Event.h"
#include "ui/FrameMain.h"
#include "application.h"
#include "windowapplication.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   using namespace gdwin;

   gdwin::event_registry registryMaps;
   //registryMaps.append({ "Mouse", std::bitset<gd_win::uMaxMessageId>(0x);

   CFrameMain framemain;
   if (framemain.create("Blank Window", { {"style", "overlappedwindow"}, {"width", 800}, {"height", 600},{ "callback", gdwin::window<>::get_callback_s() }}) == false) return 0;

   framemain.Show(nCmdShow);
   return run_s();
}


void RegisterWindowMap(gd_win::event_map_registry & registryMaps)
{
   //registryMaps.append({ "main", );
}
