/**
* \file gd_console_print.h
* 
* \brief 
* 
* Information about console 
* Format switching between different colors
* `\033[<style>;<foreground_color>;<background_color>m`
* 
* \033[38;5;<color_code>m  // Set foreground (text) color
* \033[48;5;<color_code>m  // Set background color
* 
* `tcgetattr` and `tcsetattr` is used to set console behaviour
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

// ## tag dispatchers

struct tag_color{};      ///< logic to work with color

struct tag_format_cli{}; ///< format for cli 

struct tag_height{};    ///< height operations
struct tag_width{};     ///< width operations

// ## helper structs


/** ---------------------------------------------------------------------------
 * \brief manage row and column position in console
 */
struct rowcolumn
{
// ## construction -------------------------------------------------------------
   rowcolumn(): m_uRow(0), m_uColumn(0) {}
   rowcolumn( unsigned uRow, unsigned uColumn ): m_uRow(uRow), m_uColumn(uColumn) {}
   ~rowcolumn() {}

   operator uint64_t() const { 
      uint64_t u_ = uint64_t(m_uRow) << 32 | m_uColumn; 
      return u_;
   }

   unsigned row() const { return m_uRow; }
   unsigned column() const { return m_uColumn; }

// ## attributes
   unsigned m_uRow;				///< Row index
   unsigned m_uColumn;			///< Column index
};



// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- device
// ----------------------------------------------------------------------------

/**
 * \brief device is like a drawing plate. It holds core memory buffers to draw on
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

   /** ------------------------------------------------------------------------
    * \brief internal helper object to simplify working with characters on device.
    * `row` is used for, among other things seting values like this `device[1][1] = 'X';`
    */
   struct row
   {
   // ## construction -------------------------------------------------------------
      row(): m_puRow(nullptr), m_uLength(0) {}
      row( uint8_t* puRow ): m_puRow(puRow) {}
      row( uint8_t* puRow, unsigned uLength ): m_puRow(puRow), m_uLength(uLength) {}
      ~row() {}

      // ## position operators for double index operator like `[][] = ?`
      position operator[]( unsigned uColumn ) {                                                    assert( uColumn < m_uLength ); assert( m_pdevice_d->validate_position_d( m_puRow + uColumn ) );
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
   device( const rowcolumn& rowcolumn_ ): m_uRowCount(rowcolumn_.row()), m_uColumnCount(rowcolumn_.column()) {}
   // copy
   device(const device& o) { common_construct(o); }
   device(device&& o) noexcept { common_construct(std::move(o)); }
   // assign
   device& operator=(const device& o) { common_construct(o); return *this; }
   device& operator=(device&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~device() { clear(); }
private:
   // common copy
   void common_construct(const device& o);
   void common_construct(device&& o) noexcept;
   void create_buffers( bool bInitialize );

// ## operator -----------------------------------------------------------------
public:
   row operator[]( unsigned uRow ) { row row_( offset( uRow ), m_uColumnCount ); ASSIGN_D( row_, this ); return row_; }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   unsigned get_row_count() const { return m_uRowCount; }
   unsigned get_column_count() const { return m_uColumnCount; }
   uint8_t at( unsigned uRow, unsigned uColumn ) const;
   uint8_t at( unsigned uRow, unsigned uColumn, tag_color ) const;
   void set_color( unsigned uRow, unsigned uColumn, uint8_t uColor );

   unsigned height() const { return m_uRowCount; }
   unsigned width() const { return m_uColumnCount; }
//@}

/** \name OPERATION
*///@{
   std::pair<bool, std::string> create();

   void clear();

   std::pair<bool, std::string> render( std::string& stringPrint ) const;
   std::string render( tag_format_cli ) const;

//@}

/** \name DEVICE manipulation
*///@{

   // ## fill area with chars and/or color

   void fill( unsigned uRow, unsigned uColumn, unsigned uHeight, unsigned uWidth, uint8_t uCharacter );
   void fill( unsigned uRow, unsigned uColumn, unsigned uHeight, unsigned uWidth, char iCharacter ) { fill( uRow, uColumn, uHeight, uWidth, (uint8_t)iCharacter ); }
   void fill( uint8_t uCharacter ) { fill( 0, 0, m_uRowCount, m_uColumnCount, uCharacter ); }
   void fill( char iCharacter ) { fill( 0, 0, m_uRowCount, m_uColumnCount, (uint8_t)iCharacter ); };

   // ## scroll

   /// Scroll device up or down number of rows
   void scroll_y( int32_t iOffsetRow );
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
   unsigned m_uFlags = 0;
   unsigned m_uRowCount;                     ///< number of rows in device
   unsigned m_uColumnCount;                  ///< number of columns in device
   uint8_t* m_puRowBuffer = nullptr;         ///< temporary buffer used when to render device
   uint8_t* m_puDrawBuffer = nullptr;        ///< device buffer used to store characters to draw
   uint8_t* m_puColorBuffer = nullptr;       ///< device buffer storing color
   uint8_t* m_puBackgroundBuffer = nullptr;  ///< device buffer storing background color
   uint8_t m_uFillCharacter = m_uFillCharacter_s;

   static uint8_t m_uFillCharacter_s;

// ## free functions ------------------------------------------------------------
public:
   static uint64_t calculate_device_size_s( unsigned uRowCount, unsigned uColumnCount );
   static uint64_t calculate_device_size_s( const device& device_ );
   static uint64_t calculate_row_buffer_size_s( unsigned uColumnCount );

   // ## terminal methods
   static rowcolumn terminal_get_size_s();

};

/// get character at row and column position
inline uint8_t device::at(unsigned uRow, unsigned uColumn) const {                                 assert( m_puDrawBuffer != nullptr );
   auto uPosition = uRow * m_uColumnCount + uColumn;
   return m_puDrawBuffer[ uPosition ];
}

/// get color at row and column position
inline uint8_t device::at(unsigned uRow, unsigned uColumn, tag_color) const {                      assert( m_puDrawBuffer != nullptr );
   auto uPosition = uRow * m_uColumnCount + uColumn;
   return m_puColorBuffer[ uPosition ];
}


/// Set color at index
inline void device::set_color(unsigned uRow, unsigned uColumn, uint8_t uColor) {                   assert( m_puColorBuffer != nullptr ); assert( uRow < m_uRowCount ); assert( uColumn < m_uColumnCount );
   auto uPosition = (uRow * m_uColumnCount) + uColumn;                                             assert( uPosition < calculate_device_size_s( *this ) );
   *(m_puColorBuffer + uPosition) = uColor;
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

inline uint64_t device::calculate_row_buffer_size_s(unsigned uColumnCount) {
   return (uColumnCount * 12) + 1;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------- view
// ----------------------------------------------------------------------------

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class view
{
// ## construction -------------------------------------------------------------
public:
   view(): m_uRow(0), m_uColumn(0), m_uHeight(0), m_uWidth(0) {}
   view( const device& device_ ): m_uRow(0), m_uColumn(0), m_uHeight(device_.height()), m_uWidth(device_.width()) {}
   // copy
   view(const view& o) { common_construct(o); }
   view(view&& o) noexcept { common_construct(std::move(o)); }
   // assign
   view& operator=(const view& o) { common_construct(o); return *this; }
   view& operator=(view&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~view() {}
private:
   // common copy
   void common_construct(const view& o) {}
   void common_construct(view&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   void move( int iRowOffset, int iColumnOffset );

//@}

protected:
/** \name INTERNAL
*///@{

//@}

// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uRow;                     ///< number of rows in device
   unsigned m_uColumn;                  ///< number of columns in device
   unsigned m_uHeight;
   unsigned m_uWidth;

};

inline void view::move(int iRowOffset, int iColumnOffset) {

}


// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- caret
// ----------------------------------------------------------------------------

/**
 * \brief caret is for positioning in console
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

   void render( std::string& stringPrint ) const;
   std::string render( tag_format_cli ) const;

/** \name DEBUG
*///@{

//@}

// ## attributes --------------------------------------------------------------
   unsigned m_uRow;
   unsigned m_uColumn;

// ## free functions ----------------------------------------------------------

};


_GD_CONSOLE_END