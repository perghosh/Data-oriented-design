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

#ifndef NDEBUG
      #  define ASSIGN_D( add_to_, from_ ) (add_to_ = from_)
#else
#  define ASSIGN_D ((void)0)
#endif

// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- device
// ----------------------------------------------------------------------------



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
   struct position
   {
      // ## construction -------------------------------------------------------------

      position() {}
      position( uint8_t* puPosition ): m_puPosition(puPosition) {}
      ~position() {}

      position& operator=( char ch ) { *m_puPosition = ch; return *this; }
      position& operator=( const std::string_view& string_ );

      // ## attributes
      uint8_t* m_puPosition = nullptr;
#ifndef NDEBUG
      const device* m_pdevice_d = nullptr;
      position& operator=( const device* p_ ) { m_pdevice_d = p_; return *this; }
#endif
   };



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

      position operator[]( unsigned uColumn ) {                                                    assert( uColumn < m_uLength );
         position position_(m_puRow + uColumn);  ASSIGN_D( position_, m_pdevice_d ); return position_;
      }
      const position operator[]( unsigned uColumn ) const {                                        assert( uColumn < m_uLength ); 
         position position_(m_puRow + uColumn);  ASSIGN_D( position_, m_pdevice_d ); return position_;
      }

   // ## attributes
      uint8_t* m_puRow;
      unsigned m_uLength;
#ifndef NDEBUG
      const device* m_pdevice_d = nullptr;
      row& operator=( const device* p_ ) { m_pdevice_d = p_; return *this; }
#endif
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
   row operator[]( unsigned uRow ) { row row_( offset( uRow ), m_uColumnCount ); ASSIGN_D( row_, this ); return row_; }


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

/** \name DEVICE manipulation
*///@{
   void scroll_y( int iOffset );
//@}


protected:
/** \name INTERNAL
*///@{
   uint8_t* offset( unsigned uRow ) const;
   uint8_t* offset( unsigned uRow, unsigned uColumn ) const;
   uint8_t* buffer_begin() const { return m_puDrawBuffer; }
   uint8_t* buffer_end() const { return (m_puDrawBuffer + (m_uRowCount * m_uColumnCount)); }
//@}

public:
/** \name DEBUG
*///@{
   /// validate pointer to be within draw buffer for device
   bool validate_position_d( const uint8_t* pposition_ ) const { return ( pposition_ >= buffer_begin() && pposition_ < buffer_end() ); }
//@}


// ## attributes ----------------------------------------------------------------
public:
   uint8_t* m_puRowBuffer = nullptr; 
   uint8_t* m_puDrawBuffer = nullptr;
   uint8_t* m_puColorBuffer = nullptr;
   unsigned m_uRowCount;
   unsigned m_uColumnCount;
   uint8_t m_uFillCharacter = m_uFillCharacter_s;

   static uint8_t m_uFillCharacter_s;

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


// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- caret
// ----------------------------------------------------------------------------

/**
 * \brief
 *
 *
 */
struct caret
{
// ## construction ------------------------------------------------------------
   caret(): m_uRow(0), m_uColumn(0) {}
   caret( unsigned uRow, unsigned uColumn ): m_uRow(uRow), m_uColumn(uColumn) {}

   // copy
   caret(const caret& o) { common_construct(o); }
   caret(caret&& o) noexcept { common_construct(std::move(o)); }
   // assign
   caret& operator=(const caret& o) { common_construct(o); return *this; }
   caret& operator=(caret&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~caret() {}
   // common copy
   void common_construct(const caret& o) { m_uRow = o.m_uRow; m_uColumn = o.m_uColumn; }

// ## methods -----------------------------------------------------------------

   void render( std::string& stringPrint );

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   unsigned m_uRow;
   unsigned m_uColumn;

// ## free functions ----------------------------------------------------------

};


_GD_CONSOLE_END