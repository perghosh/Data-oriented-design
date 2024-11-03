#pragma once

#include <iostream>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>

#include "application/ApplicationBasic.h"

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------- Worm
// ----------------------------------------------------------------------------


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

// ## get/set -----------------------------------------------------------------
   std::pair<unsigned,unsigned> GetHeadPosition() const;
   template<typename VALUE>
   void SetProperty( const std::string_view& stringName, VALUE v_ ) { m_argumentsWorm.set(stringName, v_); }


// ## methods -----------------------------------------------------------------

   /// Create worm, this is needed before working with worm
   std::pair< bool, std::string > Create();
   /// move worm to next position
   void Move();

   std::vector<gd::console::rowcolumn> ToList( const std::string_view& stringType ) const;


// ## attributes --------------------------------------------------------------
   uint64_t m_uMoveCounter = 0;     ///<
   gd::argument::shared::arguments m_argumentsWorm;

// ## free functions ----------------------------------------------------------
   static uint64_t ToBodyPart_s( unsigned uRow, unsigned uColumn );
   static uint64_t Move_s( uint64_t uRowColumn, int32_t iRow, int32_t iColumn );
};

/// return head position for worm
inline std::pair<unsigned, unsigned> Worm::GetHeadPosition() const {
   uint64_t uHead = m_argumentsWorm["head"];
   return { unsigned(uHead >> 32), unsigned(uHead) };
}

/// convert row and column to single 64 bit number storing position
inline uint64_t Worm::ToBodyPart_s(unsigned uRow, unsigned uColumn) {
   uint64_t uBodyPart = ( uint64_t(uRow) << 32 );
   uBodyPart += uColumn;
   return uBodyPart;
}

/// move row-column position 
inline uint64_t Worm::Move_s(uint64_t uRowColumn, int32_t iRow, int32_t iColumn) {
   unsigned uRow = ( uRowColumn >> 32 );
   unsigned uColumn = (unsigned)( uRowColumn & 0xffffffff );

   uRow += iRow;                                                                                   assert( (int)uRow >= 0 );
   uColumn += iColumn;                                                                             assert( (int)uColumn >= 0 );

   uRowColumn = uint64_t(uRow) << 32 | uColumn;
   return uRowColumn;
}

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- Application
// ----------------------------------------------------------------------------


/**
 * \brief Application object to manage the worm game
 *
 * Application acts as a god object for the worm game. Not the best solution but 
 * ok for this sample showing gd-code.
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
   void SetState( const std::string_view& stringState ) { m_stringState = stringState; }
   const std::string& GetState() const { return m_stringState; }

   std::pair<bool, std::string> Initialize() override;

   /// Read inputs to know how worm should move or setting applicatin state
   std::pair<bool, std::string> ReadInput();

   void PrepareFrame();

   /// Draw application
   void Draw();
   void DrawStartUpScreen();
   void DrawFrame();


/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   
   Worm m_worm;
   gd::console::caret m_caretTopLeft;
   gd::console::device m_deviceGame;
   std::string m_stringState = "wait";

// ## free functions ----------------------------------------------------------

};