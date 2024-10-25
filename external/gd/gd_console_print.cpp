#include "gd_debug.h"
#include "gd_console_print.h"

_GD_CONSOLE_BEGIN

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
   m_uFillCharacter = o.m_uFillCharacter;

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
   m_uFillCharacter = o.m_uFillCharacter; o.m_uFillCharacter = 0;

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
      memset( m_puDrawBuffer, m_uFillCharacter, uDeviceSize );
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
      //m_puRowBuffer[uLength] = '\0';
      //stringPrint_ += (const char*)m_puRowBuffer;
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
   uint8_t* puBuffer = m_puDrawBuffer;
   for(unsigned uRowPosition = uRow, uRowMax = uRow + uHeight; uRowPosition < uRowMax; uRowPosition++)
   {
      for(unsigned uColumnPosition = uColumn, uColumnMax = uColumn + uWidth; uColumnPosition < uColumnMax; uColumnPosition++)
      {
         puBuffer = offset( uRowPosition, uColumnPosition );
         *puBuffer = uCharacter;
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



_GD_CONSOLE_END

