#pragma once

#include <iostream>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "application/ApplicationBasic.h"

/**
* \brief Worm object is used to manage the worm data
*
*
*/
struct Worm
{
   // ## construction ------------------------------------------------------------
   Worm() {}
   // copy
   Worm(const Worm& o) { common_construct(o); }
   Worm(Worm&& o) noexcept { common_construct(std::move(o)); }
   // assign
   Worm& operator=(const Worm& o) { common_construct(o); return *this; }
   Worm& operator=(Worm&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~Worm() {}
   // common copy
   void common_construct(const Worm& o) {}
   void common_construct(Worm&& o) noexcept {}

   // ## methods -----------------------------------------------------------------
   std::pair< bool, std::string > Create();

   std::vector<gd::console::rowcolumn> ToList( const std::string_view& stringType ) const;


   // ## attributes --------------------------------------------------------------
   gd::argument::shared::arguments m_argumentsWorm;

   // ## free functions ----------------------------------------------------------
   static uint64_t ToBodyPart_s( unsigned uRow, unsigned uColumn );
};

/// convert row and column to single 64 bit number storing position
inline uint64_t Worm::ToBodyPart_s(unsigned uRow, unsigned uColumn)
{
   uint64_t uBodyPart = ( uint64_t(uRow) << 32 );
   uBodyPart += uColumn;
   return uBodyPart;
}



/**
 * \brief
 *
 *
 */
struct Application : public application::basic::CApplication
{
   // ## construction ------------------------------------------------------------
   Application() {}
   // copy
   Application(const Application& o) { common_construct(o); }
   Application(Application&& o) noexcept { common_construct(std::move(o)); }
   // assign
   Application& operator=(const Application& o) { common_construct(o); return *this; }
   Application& operator=(Application&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~Application() {}
   // common copy
   void common_construct(const Application& o) {}
   void common_construct(Application&& o) noexcept {}

// ## methods -----------------------------------------------------------------
   std::pair<bool, std::string> Initialize() override;

   /// Draw application
   void Draw();

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   Worm m_worm;
   gd::console::caret m_caretTopLeft;
   gd::console::device m_deviceGame;

// ## free functions ----------------------------------------------------------

};