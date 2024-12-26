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
#include "gd/console/gd_console_print.h"
#include "gd/console/gd_console_style.h"

#include "application/ApplicationBasic.h"

struct Defender
{
   Defender() {}
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

   void Count() { m_iCount++; }

   void BOMB_Add();
   void SHIP_Reset();
   void GAME_Start();
   void GAME_End();
   void Update();

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   int m_iCount = 0;

   std::string m_stringState;
   std::vector<gd::argument::arguments> m_vectorBomb;
   gd::argument::arguments m_argumentsShip;

   gd::console::caret m_caretTopLeft;
   gd::console::device m_deviceGame;

// ## free functions ----------------------------------------------------------

};

                                                                                    