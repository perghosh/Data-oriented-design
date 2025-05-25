#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>


#ifndef _GD_CONSOLE_BEGIN
#define _GD_CONSOLE_BEGIN namespace gd { namespace console {
#define _GD_CONSOLE_END } }
#endif

_GD_CONSOLE_BEGIN


/**
 * \brief object used to manage console metrics and positioning
 *
 *
 *
 \code
 \endcode
 */
class console
{
// ## construction -------------------------------------------------------------
public:
   console() {}
   
   // copy
   console(const console& o) { common_construct(o); }
   console(console&& o) noexcept { common_construct(std::move(o)); }
   // assign
   console& operator=(const console& o) { common_construct(o); return *this; }
   console& operator=(console&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~console() {}
private:
   // common copy
   void common_construct(const console& o);
   void common_construct(console&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   std::pair<int, int> xy() const { return { m_iCursorX, m_iCursorY }; }

   void set_size(int iWidth, int iHeight);
   void set_xy(int iX, int iY);
   void set_buffer_size(int iBufferWidth, int iBufferHeight);

//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> initialize();

   /// set cursor position in console
   std::pair<bool, std::string> move_to(int iX, int iY);

   /// set console text color
   void set_foreground_color(int iRed, int iGreen, int iBlue);
   void set_background_color(int iRed, int iGreen, int iBlue);

//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   int m_iWidth = 0;
   int m_iHeight = 0;
   int m_iCursorX = 0;
   int m_iCursorY = 0;
   int m_iBufferWidth = 0;
   int m_iBufferHeight = 0;



// ## free functions ------------------------------------------------------------
public:
   static std::pair<bool, std::string> read_console_information_s( console* pconsole );

};

inline void console::common_construct(const console& o) {
   m_iWidth        = o.m_iWidth;
   m_iHeight       = o.m_iHeight;
   m_iCursorX      = o.m_iCursorX;
   m_iCursorY      = o.m_iCursorY;
   m_iBufferWidth  = o.m_iBufferWidth;
   m_iBufferHeight = o.m_iBufferHeight;
}

inline void console::common_construct(console&& o) noexcept {
   m_iWidth        = o.m_iWidth;
   m_iHeight       = o.m_iHeight;
   m_iCursorX      = o.m_iCursorX;
   m_iCursorY      = o.m_iCursorY;
   m_iBufferWidth  = o.m_iBufferWidth;
   m_iBufferHeight = o.m_iBufferHeight;

   // Reset moved object
   o.m_iWidth = 0; o.m_iHeight = 0; o.m_iCursorX = 0; o.m_iCursorY = 0; o.m_iBufferWidth = 0; o.m_iBufferHeight = 0;
}

/// Sets the console size
inline void console::set_size(int iWidth, int iHeight) { m_iWidth = iWidth; m_iHeight = iHeight; }

/// Sets the cursor position
inline void console::set_xy(int iX, int iY) {
   m_iCursorX = iX;
   m_iCursorY = iY;
}

/// Sets the buffer size
inline void console::set_buffer_size(int iBufferWidth, int iBufferHeight) {
   m_iBufferWidth = iBufferWidth;
   m_iBufferHeight = iBufferHeight;
}

_GD_CONSOLE_END