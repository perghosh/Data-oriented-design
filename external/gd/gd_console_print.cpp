#include "gd_console_print.h"

_GD_CONSOLE_BEGIN

// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- device
// ----------------------------------------------------------------------------


uint8_t device::m_uFillCharacter_s = ' ';

std::pair<bool, std::string> device::create()
{
   clear();

   auto uDeviceSize = calculate_device_size_s( *this );

   m_puRowBuffer = new uint8_t[ m_uColumnCount * 5 + 1 ];                      // temporary row used to produce output

   m_puDrawBuffer = new uint8_t[ uDeviceSize ];
   m_puColorBuffer = new uint8_t[ uDeviceSize ];

   memset( m_puDrawBuffer, m_uFillCharacter, uDeviceSize );
   memset( m_puColorBuffer, 0, uDeviceSize );


   return { true, "" };
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


std::pair<bool, std::string> device::render(std::string& stringPrint)
{
   decltype( m_puRowBuffer ) puRow;
   std::string stringPrint_;

   for( unsigned uRow = 0; uRow < m_uRowCount; uRow++ )
   {
      puRow = m_puRowBuffer;
      for( unsigned uColumn = 0; uColumn < m_uColumnCount; uColumn++ )
      {
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
      memset( puClear, m_uFillCharacter, uRowCountToMove * m_uColumnCount );   // fill the space where data has been moved from
   };

   move_and_clear_( m_puDrawBuffer, iOffsetRow, m_uFillCharacter );
   move_and_clear_( m_puColorBuffer, iOffsetRow, 0 );

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
void caret::render(std::string& stringPrint)
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


_GD_CONSOLE_END

