#include "conio.h"


#include <format>
#include <random>

#ifndef NDEBUG
#include "gd/gd_debug.h"
#endif

#include "Application.h"

std::pair<bool, std::string> CApplication::Initialize()
{
   ::srand((unsigned)::time(NULL));                                           // init random numbers

   m_deviceGame.create(20, 80);

   BRUSH_Reset();

   return application::basic::CApplication::Initialize();
}
