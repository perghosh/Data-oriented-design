#pragma once

#include <iostream>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_console_print.h"
#include "gd/gd_console_style.h"

#include "application/ApplicationBasic.h"

struct Defender
{
   Defender() {}

};

struct Application
{
   Application() {}
   Application(std::pair<unsigned, unsigned> pairPoint) : m_pairPoint(pairPoint) {}

   void Initialize();

   void Draw();
   
   std::pair<unsigned, unsigned> m_pairPoint;

   int m_iMove = 0;
   bool bAllowedMove = true;

   gd::console::device m_deviceGame;
};