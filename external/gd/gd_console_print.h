/**
* \file gd_console_print.h
* 
* \brief 
* 
*/



#pragma once
#include <cassert>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <type_traits>

#ifndef _GD_CONSOLE_BEGIN
   #define _GD_CONSOLE_BEGIN namespace gd { namespace console {
   #define _GD_CONSOLE_END } }
   _GD_CONSOLE_BEGIN
#else
   _GD_CONSOLE_BEGIN
#endif


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class device
{
public:

   /**
    * \brief
    *
    *
    */
   struct row
   {
   // ## construction -------------------------------------------------------------
      row(): m_puRow(nullptr) {}
      row( uint8_t* puRow ): m_puRow(puRow) {}
      row( uint8_t* puRow, unsigned uLength ): m_puRow(puRow), m_uLength(uLength) {}
      ~row() {}

      uint8_t& operator[]( unsigned uColumn ) { assert( uColumn < m_uLength ); return *(m_puRow + uColumn); }
      uint8_t operator[]( unsigned uColumn ) const { assert( uColumn < m_uLength ); return *(m_puRow + uColumn); }

   // ## attributes
      uint8_t* m_puRow;
      unsigned m_uLength;
   };




// ## construction -------------------------------------------------------------
public:
   device(): m_uRowCount(0), m_uColumnCount(0) {}
   device( unsigned uRowCount, unsigned uColumnCount ): m_uRowCount(uRowCount), m_uColumnCount(uColumnCount) {}
   // copy
   device(const device& o) { common_construct(o); }
   device(device&& o) noexcept { common_construct(std::move(o)); }
   // assign
   device& operator=(const device& o) { common_construct(o); return *this; }
   device& operator=(device&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~device() {}
private:
   // common copy
   void common_construct(const device& o) {}
   void common_construct(device&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   row operator[]( unsigned uRow ) { return row( offset( uRow ), m_uColumnCount ); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   unsigned get_row_count() const { return m_uRowCount; }
   unsigned get_column_count() const { return m_uColumnCount; }
   uint8_t at( unsigned uRow, unsigned uColumn );
//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> create();

   void clear();

   std::pair<bool, std::string> render( std::string& stringPrint );

//@}

protected:
/** \name INTERNAL
*///@{
   uint8_t* offset( unsigned uRow ) const;
   uint8_t* offset( unsigned uRow, unsigned uColumn ) const;
//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   uint8_t* m_puRowBuffer = nullptr; 
   uint8_t* m_puDrawBuffer = nullptr;
   uint8_t* m_puColorBuffer = nullptr;
   unsigned m_uRowCount;
   unsigned m_uColumnCount;

   static uint8_t m_uFillCharacter;

// ## free functions ------------------------------------------------------------
public:
   static uint64_t calculate_device_size_s( unsigned uRowCount, unsigned uColumnCount );
   static uint64_t calculate_device_size_s( const device& device_ );

};

inline uint8_t device::at(unsigned uRow, unsigned uColumn) {                                       assert( m_puDrawBuffer != nullptr );
   auto uPosition = uRow * m_uColumnCount + uColumn;
   return m_puDrawBuffer[ uPosition ];
}

/// calculate position in device buffer and return pointer
inline uint8_t* device::offset( unsigned uRow ) const {                                            assert( m_puDrawBuffer != nullptr ); assert( uRow < m_uRowCount );
   return m_puDrawBuffer + (uRow * m_uColumnCount); 
}

/// calculate position in device buffer and return pointer
inline uint8_t* device::offset(unsigned uRow, unsigned uColumn) const {                            assert( m_puDrawBuffer != nullptr ); assert( uRow < m_uRowCount ); assert( uColumn < m_uColumnCount );
   return m_puDrawBuffer + ( uRow * m_uColumnCount ) + uColumn; 
}

//
inline uint64_t device::calculate_device_size_s( unsigned uRowCount, unsigned uColumnCount ) {
   return uRowCount * uColumnCount;
}

inline uint64_t device::calculate_device_size_s( const device& device_ ) {
   return calculate_device_size_s( device_.get_row_count(), device_.get_column_count() );
}

_GD_CONSOLE_END