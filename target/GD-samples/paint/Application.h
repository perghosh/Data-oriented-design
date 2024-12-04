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

struct Paint
{
   Paint() {}
};

/**
 * \brief
 *
 *
 */
struct CApplication : public application::basic::CApplication
{
   // ## construction ------------------------------------------------------------
   CApplication() {}
   // copy
   CApplication(const CApplication& o) { common_construct(o); }
   CApplication(CApplication&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CApplication& operator=(const CApplication& o) { common_construct(o); return *this; }
   CApplication& operator=(CApplication&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CApplication() {}
   // common copy
   void common_construct(const CApplication& o) {}
   void common_construct(CApplication&& o) noexcept {}

   // ## methods -----------------------------------------------------------------

   const std::string& GetState() const { return m_stringState; }

   std::pair<bool, std::string> Input_Update();

   /// Initialize game objects
   std::pair<bool, std::string> Initialize() override;

   void Move();

   void Draw();

   void PAINT_Add();
   void BRUSH_Reset();

   void Update();

   /** \name DEBUG
   *///@{

   //@}

   // ## attributes --------------------------------------------------------------
   std::string m_stringState;

   bool m_bPaintState = false;

   uint8_t m_uInputCount = 0;

   std::vector<gd::argument::arguments> m_vectorPaint;
   gd::argument::arguments m_argumentsBrush;

   gd::console::caret m_caretTopLeft;
   gd::console::device m_deviceGame;

   // ## free functions ----------------------------------------------------------

};

