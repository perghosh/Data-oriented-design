/**
 * \file gd_console_console.h
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include "../gd_types.h"
#include "../gd_math.h"


#ifndef _GD_CONSOLE_BEGIN
#define _GD_CONSOLE_BEGIN namespace gd { namespace console {
#define _GD_CONSOLE_END } }
#endif

_GD_CONSOLE_BEGIN


/**
 * \brief
 *
 *
 */
struct progress
{
// ## construction ------------------------------------------------------------
   progress() : m_uRow(0), m_uColumn(0), m_uWidth(0), m_uMax(0), m_uValue(0) {}
   progress(unsigned uRow, unsigned uColumn) : m_uRow(uRow), m_uColumn(uColumn), m_uWidth(0), m_uMax(0), m_uValue(0) {}
   progress( const std::pair<unsigned, unsigned>& pairRC ): m_uRow(pairRC.first), m_uColumn(pairRC.second), m_uWidth(0), m_uMax(0), m_uValue(0) {}
   progress(const std::pair<unsigned, unsigned>& pairRC, unsigned uWidth) : m_uRow(pairRC.first), m_uColumn(pairRC.second), m_uWidth(0), m_uMax(0), m_uValue(0) { set_width( uWidth ); }
   progress(unsigned uRow, unsigned uColumn, unsigned uWidth): m_uRow(uRow), m_uColumn(uColumn), m_uWidth(0), m_uMax(0), m_uValue(0) { set_width( uWidth ); }
   progress(unsigned uRow, unsigned uColumn, unsigned uWidth, unsigned uMax): m_uRow(uRow), m_uColumn(uColumn), m_uWidth(uWidth), m_uMax(uMax), m_uValue(0) { }
   // copy
   progress(const progress& o) { common_construct(o); }
   // assign
   progress& operator=(const progress& o) { common_construct(o); return *this; }

   ~progress() {}
// common copy
   void common_construct(const progress& o) {
      m_uRow = o.m_uRow;
      m_uColumn = o.m_uColumn;
      m_uWidth = o.m_uWidth;
      m_uMax = o.m_uMax;
      m_uValue = o.m_uValue;
   }

// ## methods -----------------------------------------------------------------
   void set_position(unsigned uRow, unsigned uColumn) { m_uRow = uRow; m_uColumn = uColumn; }
   void set_position(const std::pair<unsigned, unsigned>& pairRC) { m_uRow = pairRC.first; m_uColumn = pairRC.second; }

   void set_width(unsigned uWidth);
   void set_max(unsigned uMax) { assert(uMax > 0); m_uMax = uMax; }

   unsigned get_row() const { return m_uRow; }
   unsigned row() const { return m_uRow; }
   unsigned get_column() const { return m_uColumn; }
   unsigned column() const { return m_uColumn; }

   unsigned get_width() const { return m_uWidth; }

   gd::math::algebra::point<unsigned> first() const { return { m_uColumn, m_uRow }; }
   gd::math::algebra::point<unsigned> position() const { return { m_uValue, m_uRow }; }
   gd::math::algebra::point<unsigned> last() const { return { m_uMax, m_uRow }; }

   void reset() { m_uValue = 0; }
   void update(unsigned uValue) { assert(uValue <= m_uMax); m_uValue = uValue; }
   void update( unsigned uValue, gd::types::tag_percent);
   void complete() { m_uValue = m_uMax; }

   void print_to(const std::string& stringLeft, const std::string& stringFill, const std::string& stringPointer, const std::string& stringRight, std::string& stringBar) const;

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   unsigned m_uRow = 0; ///< Row in the console where the progress bar is displayed
   unsigned m_uColumn = 0; ///< Column in the console where the progress bar is displayed
   unsigned m_uWidth = 0; ///< Width of the progress bar
   unsigned m_uMax = 0; ///< Maximum value for the progress bar
   unsigned m_uValue = 0; ///< Current value of the progress bar


// ## free functions ----------------------------------------------------------

};

/// Set progress bar width and adjust max if necessary
inline void progress::set_width(unsigned uWidth) { 
   m_uWidth = uWidth; 
   m_uMax = m_uMax < m_uWidth ? uWidth : m_uMax; 
}

/// Updates the progress value based on a percentage
inline void progress::update( unsigned uValue, gd::types::tag_percent) {                           assert(uValue <= 100);
   m_uValue = (m_uMax * uValue) / 100; // Convert percentage to value based on max
}



/**
 * @class console
 * @brief Manages console metrics, positioning, and color operations.
 *
 * The console class provides an interface for manipulating and querying
 * console properties such as size, cursor position, buffer size, and text colors.
 * It supports setting and retrieving the cursor position, changing the console's
 * foreground and background colors using ANSI escape codes, and reading or clearing
 * lines of text from the console.
 *
 * Example usage:
 * @code
 * gd::console::console con;
 * con.set_size(80, 25);
 * con.set_xy(0, 0);
 * con.set_foreground_color(255, 0, 0); // Set text color to red
 * con.set_background_color(0, 0, 0);   // Set background to black
 * @endcode
 *
 * @note This class is designed to be platform-agnostic, but some features may
 *       depend on the underlying terminal or operating system.
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
   std::pair<int, int> yx() const { return { m_iCursorY, m_iCursorX }; }
   std::pair<unsigned, unsigned> yx( gd::types::tag_type_unsigned ) const { return { (unsigned)m_iCursorY, (unsigned)m_iCursorX }; }
   
   int get_width() const { return m_iWidth; }
   int get_height() const { return m_iHeight; }

   void set_size(int iWidth, int iHeight);
   void set_xy(int iX, int iY);
   void set_row_column(int iRow, int iColumn) { set_xy(iColumn, iRow); }
   void set_buffer_size(int iBufferWidth, int iBufferHeight);

//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> initialize();

   /// set cursor position in console
   std::pair<bool, std::string> move_to(int iRow, int iColumn);
   std::pair<bool, std::string> move_to(auto row_, auto column_, gd::types::tag_row_column) { return move_to( (int)row_, (int)column_ ); }
   std::pair<bool, std::string> move_to(auto column_, auto row_, gd::types::tag_column_row) { return move_to( (int)row_, (int)column_ ); }

   void print( const gd::math::algebra::point<unsigned>& point_, std::string_view stringText );
   /// Prints text using stringstream
   void print( const std::string& stringText );

   std::pair<bool, std::tuple<int, int, int>> query_foreground_color() const { return query_foreground_color_s(); }
   std::pair<bool, std::tuple<int, int, int>> query_background_color() const { return query_background_color_s(); }

   /// Set text color using ANSI escape codes
   void set_foreground_color(int iRed, int iGreen, int iBlue);
   void set_foreground_color( std::tuple<int, int, int> color ) { set_foreground_color(std::get<0>(color), std::get<1>(color), std::get<2>(color)); }
   /// Set background color using ANSI escape codes
   void set_background_color(int iRed, int iGreen, int iBlue);
   void set_background_color(std::tuple<int, int, int> color) { set_background_color(std::get<0>(color), std::get<1>(color), std::get<2>(color)); }

   /// Clear the entire screen
   //std::pair<bool, std::string> clear_screen();
   std::pair<bool, std::string> clear_line() { return clear_line_s(); }

   /// read curcsor position in console
   std::pair<bool, std::string> read_cursor_position() { return read_console_cursor_position_s(this); }

   /// Read text from the console at the specified position and length
   std::pair<bool, std::string> read_text(int iStartX, int iStartY, int iLength) { return read_text_s(iStartX, iStartY, iLength); }

   /// check if the console is empty (width or height <= 0)
   bool empty() const { return m_iWidth <= 0 || m_iHeight <= 0; }


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
   static std::pair<bool, std::string> read_console_cursor_position_s( console* pconsole );
   static std::pair<bool, std::tuple<int, int, int>> query_foreground_color_s();
   static std::pair<bool, std::tuple<int, int, int>> query_background_color_s();

   static std::pair<bool, std::string> clear_line_s();

   static std::pair<bool, std::string> read_text_s(int iStartX, int iStartY, int iLength);

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