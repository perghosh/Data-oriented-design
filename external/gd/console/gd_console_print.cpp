#include "../gd_debug.h"
#include "gd_console_print.h"

#ifdef _WIN32
#  include <windows.h>
#  undef min
#  undef max
#else
#  include <sys/ioctl.h>
#  include <unistd.h>
#endif


_GD_CONSOLE_BEGIN

namespace {
   void order(unsigned& u1, unsigned& u2 )
   {
      if(u1 > u2)
      {
         auto u_ = u1;
         u1 = u2;
         u2 = u_;
      }
   }
}

// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- device
// ----------------------------------------------------------------------------


uint8_t device::m_uFillCharacter_s = ' ';

/// copy device data from one device to another (this)
void device::common_construct(const device& o)
{
   m_uFlags = o.m_uFlags;
   m_uColumnCount = o.m_uColumnCount;
   m_uRowCount = o.m_uRowCount;
   m_iFillCharacter = o.m_iFillCharacter;

   create_buffers( false );

   uint64_t uDeviceSize = calculate_device_size_s( o );
   memcpy( m_puDrawBuffer, o.m_puDrawBuffer, uDeviceSize );
   memcpy( m_puColorBuffer, o.m_puColorBuffer, uDeviceSize );

   uint64_t uRowBufferSize = calculate_row_buffer_size_s( o.m_uColumnCount );
   memcpy( m_puRowBuffer, o.m_puRowBuffer, uRowBufferSize );
}

/// moves data from one device to another (this)
void device::common_construct(device&& o) noexcept
{
   m_uFlags = o.m_uFlags; o.m_uFlags = 0;
   m_uColumnCount = o.m_uColumnCount; o.m_uColumnCount = 0;
   m_uRowCount = o.m_uRowCount; o.m_uRowCount = 0;
   m_iFillCharacter = o.m_iFillCharacter; o.m_iFillCharacter = 0;

   m_puDrawBuffer = o.m_puDrawBuffer; o.m_puDrawBuffer = nullptr;
   m_puColorBuffer = o.m_puColorBuffer; o.m_puColorBuffer = nullptr;
   m_puRowBuffer = o.m_puRowBuffer; o.m_puRowBuffer = nullptr;
}

/// create internal buffers for used by device
void device::create_buffers( bool bInitialize )
{
   auto uDeviceSize = calculate_device_size_s( *this );
   m_puDrawBuffer = new uint8_t[ uDeviceSize ];
   m_puColorBuffer = new uint8_t[ uDeviceSize ];

   auto uRowBufferSize = calculate_row_buffer_size_s( m_uColumnCount );
   m_puRowBuffer = new uint8_t[ uRowBufferSize ];                              // temporary row used to produce output

   if( bInitialize == true )
   {
      memset( m_puDrawBuffer, m_iFillCharacter, uDeviceSize );
      memset( m_puColorBuffer, 0, uDeviceSize );
   }
}

/** ---------------------------------------------------------------------------
 * @brief prepare and create device for printing
 * @return true if ok, false and error information if error
 */
std::pair<bool, std::string> device::create()
{
   clear();

   create_buffers( true );

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief create device to handle printing, buffers storing data are allocated
 * @code
gd::console::device deviceTest;
deviceTest.create( 20, 100 );
gd::console::draw::line line_( 0, 0,  5, 90 );
line_.print( &deviceTest, '*' );
stringOut = deviceTest.render( gd::console::tag_format_cli{} );
std::cout << stringOut;
 * @endcode
 * @param uRowCount number of rows
 * @param uColumnCount number of columns
 * @return true if ok, false and error information if failing
 */
std::pair<bool, std::string> device::create(unsigned uRowCount, unsigned uColumnCount)
{
   m_uRowCount = uRowCount;
   m_uColumnCount = uColumnCount;

   return create();
}

/** ---------------------------------------------------------------------------
 * @brief Print text with color at position
 * @param uRow row position
 * @param uColumn column position
 * @param stringText text to print
 * @param uColor color for text
 */
void device::print( unsigned uRow, unsigned uColumn, const std::string_view& stringText, unsigned uColor )
{                                                                                                  assert( uRow < m_uRowCount ); assert( uColumn < m_uColumnCount );
   auto pposition_ = offset( uRow, uColumn );                                                      assert( (pposition_+ stringText.length()) <= buffer_end() );
   memcpy( pposition_, stringText.data(), stringText.length() );
   memset( offset_color( uRow, uColumn ),(uint8_t)uColor, stringText.length() );
}


/** ---------------------------------------------------------------------------
 * @brief print character at positions in vector
 * @param vectorRC vector with positions where to print character on device
 * @param ch_ character to print
 */
void device::print(const std::vector<rowcolumn>& vectorRC, char iCharacter )
{
   for(auto it : vectorRC)
   {
      print( it, iCharacter );
   }
}

/** ---------------------------------------------------------------------------
 * @brief clear internal resources for device
 */
void device::clear()
{
   delete [] m_puRowBuffer;
   delete [] m_puDrawBuffer;
   delete [] m_puColorBuffer;

   m_puRowBuffer  = nullptr;
   m_puDrawBuffer = nullptr;
   m_puColorBuffer = nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief render device to print
 * @param stringPrint string getting information to print
 * @return true if ok, false and error information on error
 * 
 * ```bash
 # ## Example of rendering device to print

 # 256-color foreground (e.g., orange, color 208)
 echo -e "\033[38;5;208mHello, World!\033[0m"

 # 256-color background (e.g., light gray, color 248)
 echo -e "\033[48;5;248mHello, World!\033[0m"

 # True color foreground (e.g., RGB orange)
 echo -e "\033[38;2;255;128;0mHello, World!\033[0m" 
 * ```
 */
std::pair<bool, std::string> device::render(std::string& stringPrint) const
{
   uint8_t uActiveColor = 0;
   decltype( m_puRowBuffer ) puRow;
   std::string stringPrint_;
   std::string stringTemp;

   stringPrint_.reserve( calculate_device_size_s( *this ) );
   stringPrint_ = "\033[0m";

   for( unsigned uRow = 0; uRow < m_uRowCount; uRow++ )
   {
#ifndef NDEBUG
      const char* pbszBuffer_d = (const char*)m_puRowBuffer;
#endif
      puRow = m_puRowBuffer;
      for( unsigned uColumn = 0; uColumn < m_uColumnCount; uColumn++ )
      {
         auto uColor = at( uRow, uColumn, tag_color{});
         if(uColor != 0 && uColor != uActiveColor)
         {
            stringTemp = "\033[38;5;";
            stringTemp += std::to_string( uColor );
            stringTemp += 'm';
            memcpy( puRow, stringTemp.data(), stringTemp.length() );
            puRow += stringTemp.length();
            uActiveColor = uColor;
         }

         *puRow = at( uRow, uColumn );
         puRow++;
      }
      *puRow = '\n';
      puRow++;

      size_t uLength = puRow - m_puRowBuffer;
      stringPrint_.append( (const char*)m_puRowBuffer, uLength );
   }

   if(stringPrint.empty() == true) { stringPrint = std::move( stringPrint_ ); }
   else
   {
      stringPrint += stringPrint_;
   }

   return { true, "" };
}

/// simplified method to work with rendering
std::string device::render(tag_format_cli) const
{
   std::string stringPrint;
   auto [bOk, stringError] = render( stringPrint );                                                assert( bOk );

   return stringPrint;
}

void device::fill( unsigned uRow, unsigned uColumn, unsigned uHeight, unsigned uWidth, uint8_t uCharacter )
{                                                                                                  assert( (uRow + uHeight) <= m_uRowCount ); assert( (uColumn + uWidth) <= m_uColumnCount ); assert( m_puDrawBuffer != nullptr );
   for(unsigned uRowPosition = uRow, uRowMax = uRow + uHeight; uRowPosition < uRowMax; uRowPosition++)
   {
      for(unsigned uColumnPosition = uColumn, uColumnMax = uColumn + uWidth; uColumnPosition < uColumnMax; uColumnPosition++)
      {
         uint8_t* puBuffer = offset( uRowPosition, uColumnPosition );
         *puBuffer = uCharacter;
      }
   }

   if(m_iColor != -1)
   {
      for(unsigned uRowPosition = uRow, uRowMax = uRow + uHeight; uRowPosition < uRowMax; uRowPosition++)
      {
         for(unsigned uColumnPosition = uColumn, uColumnMax = uColumn + uWidth; uColumnPosition < uColumnMax; uColumnPosition++)
         {
            uint8_t* puBuffer = offset_color( uRowPosition, uColumnPosition );
            *puBuffer = (uint8_t)m_iColor;
         }
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Scroll device up or down
 * Scolling up or down is same as moving data in device number of rows x columns within
 * buffers. draw buffer and color buffer.
 * @param iOffsetRow number of rows to scroll up or down (negative = down)
 */
void device::scroll_y(int32_t iOffsetRow)
{                                                                                                  assert( iOffsetRow );
   unsigned uRowCountToMove = iOffsetRow > 0 ? iOffsetRow : iOffsetRow * -1;                       assert( uRowCountToMove < m_uRowCount); // no meaning to scroll if everything is scrolled
   unsigned uMoveOffset = uRowCountToMove * m_uColumnCount;
   unsigned uDeltaToFrom = (m_uRowCount - uRowCountToMove) * m_uColumnCount;   // number of characters that are over written

   // ## move data within buffers that is used to create output
   // @param puBuffer pointer to buffer where data is moved to simulate scrolling
   // @param iOffsetRow how many rows to scroll
   // @param uClear character used to fill the empty space after movement of data
   auto move_and_clear_ = [this](uint8_t* puBuffer, int32_t iOffsetRow, uint8_t uClear ) -> void {
      unsigned uRowCountToMove = iOffsetRow > 0 ? iOffsetRow : iOffsetRow * -1;                    assert( uRowCountToMove < m_uRowCount); // no meaning to scroll if everything is scrolled
      unsigned uMoveOffset = uRowCountToMove * m_uColumnCount;
      unsigned uDeltaToFrom = (m_uRowCount - uRowCountToMove) * m_uColumnCount;// number of characters that are over written
      
      uint8_t* puTo = puBuffer;
      uint8_t* puFrom = puTo + uMoveOffset;
      uint8_t* puClear = puTo + uDeltaToFrom;
      if(iOffsetRow < 0)                                                       // reverse if moving "down" (scroll up)
      {
         puTo = puBuffer + uMoveOffset;
         puFrom = puBuffer;
         puClear = puFrom;
      }

      unsigned uMoveCount = (m_uRowCount - uRowCountToMove) * m_uColumnCount;  // number of characters in block to move
      memmove( puTo, puFrom, uMoveCount );                                     // move data within buffer
      memset( puClear, uClear, uRowCountToMove * m_uColumnCount );             // fill the space where data has been moved from
   };

   move_and_clear_( m_puDrawBuffer, iOffsetRow, (uint8_t)m_iFillCharacter );
   move_and_clear_( m_puColorBuffer, iOffsetRow, 0 );

}

/// Return console (terminal) size
rowcolumn device::terminal_get_size_s()
{
#  ifdef _WIN32
   CONSOLE_SCREEN_BUFFER_INFO CSBI;
   if( ::GetConsoleScreenBufferInfo( ::GetStdHandle( STD_OUTPUT_HANDLE ), &CSBI ) == TRUE )
   {
      unsigned uRow = CSBI.srWindow.Bottom - CSBI.srWindow.Top + 1;
      unsigned uColumn = CSBI.srWindow.Right - CSBI.srWindow.Left + 1;
      rowcolumn rowcolumn_( uRow, uColumn );
      return rowcolumn_;
   }
#  else
   struct winsize winsize_;
   if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize_) == 0) 
   {
      unsigned uRow = winsize_.ws_row;
      unsigned uColumn = winsize_.ws_col;
      rowcolumn rowcolumn_( uRow, uColumn );
      return rowcolumn_;
   }
#  endif
                                                                                                   assert( false );
   return rowcolumn();
}

device::position& device::position::operator=(const std::string_view& string_)
{                                                                                                  assert( m_pdevice_d->validate_position_d(m_puPosition + string_.length()) == true );
   memcpy( m_puPosition, string_.data(), string_.length() );
   return *this;
}


// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- caret
// ----------------------------------------------------------------------------

/// Generate string to position where caret should be placed
void caret::render(std::string& stringPrint) const
{
   std::string stringPrint_;

   stringPrint_ += std::string_view{ "\033[" };
   stringPrint_ += std::to_string( m_uRow );
   stringPrint_ += std::string_view{ ";" };
   stringPrint_ += std::to_string( m_uColumn );
   stringPrint_ += std::string_view{ "H" };

   if(stringPrint.empty() == true) { stringPrint = std::move( stringPrint_ ); }
   else
   {
      stringPrint += stringPrint_;
   }
}

/// simplified method to work with rendering
std::string caret::render(tag_format_cli) const
{
   std::string stringPrint;
   render( stringPrint ); 

   return stringPrint;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------- line
// ----------------------------------------------------------------------------

namespace draw {

/** ---------------------------------------------------------------------------
 * @brief Draw line on selected device
 * @code
 gd::console::device deviceTest( 5, 100 );      // 5 lines and 100 columns
 deviceTest.create();                           // create device (allocates internal memory)
 gd::console::draw::line line_( 0, 5,  0, 95 ); // generate line object
 line_.print( &deviceTest, '*' );               // print line on device
 auto stringOut = deviceTest.render( gd::console::tag_format_cli{} );  // render device to string
 std::cout << stringOut;                        // print string to console
 * @endcode
 * @param pdevice pointer to device line is drawn on
 * @param iCharacter character to draw
 */
void line::print(device* pdevice, char iCharacter) const
{
   unsigned uR1 = m_uRow1, uR2 = m_uRow2, uC1 = m_uColumn1, uC2 = m_uColumn2;

   order(uR1, uR2);
   order(uC1, uC2);

   unsigned uDeltaRow = uR2 - uR1;
   unsigned uDeltaColumn = uC2 - uC1;

   int iNext = uDeltaColumn - uDeltaRow;

   while(uR1 != uR2 || uC1 != uC2)
   {
      pdevice->print( uR1, uC1, iCharacter );

      int iNext2 = 2 * iNext;
      if( iNext2 > -(int)uDeltaRow ) { iNext -= uDeltaRow; uC1 += 1; }
      if( iNext2 < (int)uDeltaColumn ) { iNext += uDeltaColumn; uR1 += 1; }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Draw line on selected device with specified color
 * @code
 gd::console::device deviceTest( 5, 100 );      // 5 lines and 100 columns 
 deviceTest.create();                           // create device (allocates internal memory)
 gd::console::draw::line line_( 0, 5,  0, 95 ); // generate line object
 // ## print three lines with different colors
 line_.print( &deviceTest, '*', gd::console::color_g("cyan1") );
 line_.move_down();
 line_.print( &deviceTest, '*', gd::console::color_g("gold1") );
 line_.move_down();
 line_.print( &deviceTest, '*', gd::console::color_g("grey35") );
 auto stringOut = deviceTest.render( gd::console::tag_format_cli{} ); // render device to string
 std::cout << stringOut;                        // print string to console
 * @endcode
 * @param pdevice pointer to device line is drawn on
 * @param iCharacter character to draw
 * @param uColor color for line
 */
void line::print(device* pdevice, char iCharacter, uint8_t uColor) const
{
   unsigned uR1 = m_uRow1, uR2 = m_uRow2, uC1 = m_uColumn1, uC2 = m_uColumn2;

   order(uR1, uR2);
   order(uC1, uC2);

   unsigned uDeltaRow = uR2 - uR1;
   unsigned uDeltaColumn = uC2 - uC1;

   int iNext = uDeltaColumn - uDeltaRow;

   while(uR1 != uR2 || uC1 != uC2)
   {
      pdevice->print( uR1, uC1, iCharacter, uColor );

      int iNext2 = 2 * iNext;
      if( iNext2 > -(int)uDeltaRow ) { iNext -= uDeltaRow; uC1 += 1; }
      if( iNext2 < (int)uDeltaColumn ) { iNext += uDeltaColumn; uR1 += 1; }
   }
}


/** ---------------------------------------------------------------------------
 * @brief Draw line on selected device
 * @param pdevice pointer to device line is drawn on
 * @param iBegin first character to draw in line
 * @param iMiddle characters for middle part of line 
 * @param iEnd last character to draw in line
 */
unsigned line::print(device* pdevice, char iBegin, char iMiddle, char iEnd) const
{
   char piBuffer[] = {iBegin, iMiddle, iEnd};
   const char* piCharacter = piBuffer;
   unsigned uR1 = m_uRow1, uR2 = m_uRow2, uC1 = m_uColumn1, uC2 = m_uColumn2;

   order(uR1, uR2);
   order(uC1, uC2);

   unsigned uDeltaRow = uR2 - uR1;
   unsigned uDeltaColumn = uC2 - uC1;

   int iNext = uDeltaColumn - uDeltaRow;
   unsigned uCount = 0;

   unsigned uRSave;
   unsigned uCSave;


   while(uR1 != uR2 || uC1 != uC2)
   {
      pdevice->print( uR1, uC1, *piCharacter );
      uRSave = uR1;
      uCSave = uC1;

      int iNext2 = 2 * iNext;
      if( iNext2 > -(int)uDeltaRow ) { iNext -= uDeltaRow; uC1 += 1; }
      if( iNext2 < (int)uDeltaColumn ) { iNext += uDeltaColumn; uR1 += 1; }

      if( uCount == 0 ) piCharacter++;
      uCount++;
   }

   // if more than two characters then place the last character in end position
   if( uCount > 1 ) { pdevice->print( uRSave, uCSave, *(piCharacter + 1) ); }

   return uCount;
}

} // namespace draw {


_GD_CONSOLE_END

