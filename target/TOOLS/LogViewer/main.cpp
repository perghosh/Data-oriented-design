// application.cpp : Defines the entry point for the application.
//

          
#include "window.h"

#include "os/OS_Event.h"
#include "application.h"
#include "windowapplication.h"




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
   CDrawWindow window;
   if(window.Create(L"Blank Window", 800, 600) == false) return 0;

   window.Show(nCmdShow);
   return win::CWindow::Run_s();
}
