#include "gd_console_print.h"

_GD_CONSOLE_BEGIN

// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- device
// ----------------------------------------------------------------------------


uint8_t device::m_uFillCharacter = ' ';

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

