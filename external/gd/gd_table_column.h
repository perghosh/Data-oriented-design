#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <string_view>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "gd_arguments.h"
#include "gd_table.h"
#include "gd_types.h"
#include "gd_variant_view.h"

#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wreorder-ctor"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
   #pragma clang diagnostic ignored "-Winvalid-offsetof"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 )
#endif



#ifndef _GD_TABLE_DETAIL_BEGIN
#  define _GD_TABLE_DETAIL_BEGIN namespace gd { namespace table { namespace detail {
#  define _GD_TABLE_DETAIL_END } } }
#endif

_GD_TABLE_DETAIL_BEGIN

// ============================================================================
// ===================================================================== column
// ============================================================================

/**
 * \brief column is used to transfer column information between objects
 *
 * With `column` it is possible to collect information from one type of table
 * and create similar type in another type of table.
 * Column holds just the basic column attributes needed for columns in different
 * table objects.
 *
 \code
 \endcode
 */
class column
{
public:
   /** 
    * \brief constant numbers used in table or items used in table
    */
   enum
   {
      // ## column flags marking column states, how column behaves/works
      eColumnStateLength      = 0b0000'0000'0000'0001, ///< column flag marking that value begins with length
      eColumnStateReference   = 0b0000'0000'0000'0010, ///< column flag marking that value is stored in reference object

      eColumnStateAlignMask   = 0b0000'0000'1111'0000, ///< 
      eColumnStateAlignLeft   = 0b0000'0000'0001'0000, ///< 
      eColumnStateAlignCenter = 0b0000'0000'0010'0000, ///< 
      eColumnStateAlignBottom = 0b0000'0000'0100'0000, ///< 
      eColumnStateAlignMiddle = 0b0000'0000'1000'0000, ///< 
   };

// ## construction -------------------------------------------------------------
public:
   column( tag_undefined ) {}
   explicit column( unsigned uType ) : m_uState{}, m_uType{uType}, m_uCType{uType}, m_uPosition{}, m_uSize{}, m_uPrimitiveSize{ gd::types::value_size_g(uType) } {}
   explicit column( unsigned uType, const std::string_view& stringName ) : m_uState{}, m_uType{ uType }, m_uCType{ uType }, m_uPosition{}, m_uSize{}, m_uPrimitiveSize{ gd::types::value_size_g(uType) }, m_stringName{ stringName } {}
   explicit column( unsigned uType, unsigned uSize ) : m_uState{}, m_uType{ uType }, m_uCType{ uType }, m_uPosition{}, m_uSize{ uSize }, m_uPrimitiveSize{ gd::types::value_size_g(uType) } {}
   explicit column( unsigned uType, unsigned uSize, const std::string_view& stringName ) : m_uState{}, m_uType{ uType }, m_uCType{ uType }, m_uPosition{}, m_uSize{ uSize }, m_uPrimitiveSize{ gd::types::value_size_g(uType) }, m_stringName{ stringName } {}
   explicit column( unsigned uType, unsigned uCType, unsigned uSize, unsigned uPrimitiveSize, unsigned uPosition, const std::string_view& stringName, const std::string_view& stringAlias ) 
      : m_uState{}, m_uType{ uType }, m_uCType{ uCType }, m_uSize{ uSize }, m_uPrimitiveSize{ uPrimitiveSize }, m_uPosition{uPosition}, m_stringName{ stringName }, m_stringAlias{ stringAlias } {}
   column( unsigned uState, unsigned uType, unsigned uCType, unsigned uSize, unsigned uPrimitiveSize, unsigned uPosition, const std::string_view& stringName, const std::string_view& stringAlias ) 
      : m_uState{uState}, m_uType{ uType }, m_uCType{ uCType }, m_uSize{ uSize }, m_uPrimitiveSize{ uPrimitiveSize }, m_uPosition{uPosition}, m_stringName{ stringName }, m_stringAlias{ stringAlias } {}
   column() : m_uState{}, m_uType{}, m_uCType{}, m_uPosition{}, m_uSize{}, m_uPrimitiveSize{} {}


   // copy
   column( const column& o ) { common_construct( o ); }
   column( column&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   column& operator=( const column& o ) { common_construct( o ); return *this; }
   column& operator=( column&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~column() {}
private:
   // common copy
   void common_construct( const column& o ) { memcpy(static_cast<void*>(this), &o, offsetof( column, m_stringName ) ); m_stringName = o.m_stringName; m_stringAlias = o.m_stringAlias; m_arguments = o.m_arguments; }
   void common_construct( column&& o ) noexcept { memcpy(static_cast<void*>(this), &o, offsetof( column, m_stringName ) ); m_stringName = std::move( o.m_stringName ); m_stringAlias = std::move( o.m_stringAlias ); m_arguments = std::move( o.m_arguments ); }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
   [[nodiscard]] unsigned state() const noexcept { return m_uState; }
   void state( unsigned uState ) { m_uState = uState; }
   void type( unsigned uType ) { m_uType = uType; }
   [[nodiscard]] unsigned type() const noexcept { return m_uType; }
   void ctype( unsigned uCType ) { m_uCType = uCType; }
   [[nodiscard]] unsigned ctype() const noexcept { return m_uCType; }
   /// extract the number type part from ctype
   [[nodiscard]] unsigned type_number() const noexcept { return m_uType & 0x0000'00ff; }
   [[nodiscard]] unsigned type_group() const noexcept { return m_uType & 0x0000'ff00; }
   /// extract the number type part from ctype
   [[nodiscard]] unsigned ctype_number() const noexcept { return m_uCType & 0x0000'00ff; }
   [[nodiscard]] unsigned ctype_group() const noexcept { return m_uCType & 0x0000'ff00; }
   void position( unsigned uPosition ) { m_uPosition = uPosition; }
   [[nodiscard]] unsigned position() const noexcept { return m_uPosition; }
   void size( unsigned uSize ) { m_uSize = uSize; }
   [[nodiscard]] unsigned size() const noexcept { return m_uSize; }
   void primitive_size( unsigned uSize ) { m_uPrimitiveSize = uSize; }
   [[nodiscard]] unsigned primitive_size() const noexcept { return m_uPrimitiveSize; }
   [[nodiscard]] std::string_view name() const { return std::string_view{ m_stringName }; }
   void name( const std::string& stringName ) { m_stringName = stringName; }
   void name( const std::string_view& stringName ) { m_stringName = stringName; }
   [[nodiscard]] std::string_view alias() const { return std::string_view{ m_stringAlias }; }
   void alias( const std::string_view& stringAlias ) { m_stringAlias = stringAlias; }

   std::string_view ctype_name() const { return gd::types::type_name_g( m_uCType ); }
   std::string_view type_name() const { return gd::types::type_name_g( m_uType ); }

   /// no size or reference value in buffer for value returns true, if size buffer (uint32_t) value is not fixed
   bool is_fixed() const noexcept { return (m_uState & (eColumnStateLength | eColumnStateReference)) == 0; }
   /// if value holds value length as prefix in column buffer
   bool is_length() const noexcept { return (m_uState & eColumnStateLength); }
   /// if column store value in as reference value
   bool is_reference() const noexcept { return (m_uState & eColumnStateReference); }
   /// is column aligned (default is top/right)
   bool is_aligned() const noexcept { return (m_uState & eColumnStateAlignMask); }
   /// is column a primitive type that can be directly stored in column buffer
   bool is_primitive() const noexcept { return gd::types::is_primitive_g( m_uCType ); }

   // ## Transfer methods

   /// collect column information into `argument::column` object
   void get( argument::column& column_ ) const;



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
   unsigned m_uState;   ///< column state, like length, align
   unsigned m_uType;    ///< native value type
   unsigned m_uCType;   ///< c value type (lower byte has the number for type)
   unsigned m_uPosition;///< position where value starts
   unsigned m_uSize;    ///< max column size (also the internal buffer size), for fixed types this is 0
   unsigned m_uPrimitiveSize;///< size in bytes for each C++ primitive type or some special types like uuid
   std::string m_stringName;
   std::string m_stringAlias;
   gd::argument::arguments m_arguments;

   // ## free functions ------------------------------------------------------------
public:

};

// ============================================================================
// ==================================================================== columns
// ============================================================================

/**                                                                                                @API [tag: class, class, table] [description: columns class holding column information]
 * \brief Store information about columns in table
 *
 * This class holds all information about columns used a table. The reason behind
 * storing columns in a separate class is that it is possible to create tables without
 * the overhead of also create information about columns for each table. Columns
 * are shared between tables and can be used to create similar tables with.
 *
 \code
 \endcode
 */
class columns
{
public: 
   using value_type = column;
   using const_value_type = const column;
   using iterator = std::vector<column>::iterator;
   using const_iterator = std::vector<column>::const_iterator;
   typedef std::random_access_iterator_tag iterator_category;

   // ## construction -------------------------------------------------------------
public:
   columns() {}
   // copy
   columns( const columns& o ) { common_construct( o ); }
   columns( columns&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   columns& operator=( const columns& o ) { common_construct( o ); return *this; }
   columns& operator=( columns&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~columns() {}
private:
   // common copy
   void common_construct( const columns& o ) { m_vectorColumn = o.m_vectorColumn; }
   void common_construct( columns&& o ) noexcept { m_vectorColumn = std::move( o.m_vectorColumn ); }

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{

   //@}

   /// @name add
   /// Add columns to table, this is typically done before adding values to table. Remember to call @see prepare before adding data
   /// Parameters:
   /// - `uColumnType` type of column to add @see: gd::types::enumType
   /// - `stringType` type of columns as string name, will be converted to the type number
   /// - `uSize` if type do not have a fixed size then size will have the maximum length for text
   /// - `columnToAdd` has all column properties for column to add
   /// - `stringName` name for column
   /// - `stringAlias` alias name for column (column can have both name and alias)
   ///@{
   columns& add( const column& columnToAdd ) { m_vectorColumn.push_back( columnToAdd ); return *this; }
   columns& add( const column&& columnToAdd ) { m_vectorColumn.emplace_back( std::move( columnToAdd ) ); return *this; }
   columns& add( unsigned uColumnType, const std::string_view& stringName ) { return add( uColumnType, 0, stringName ); }
   columns& add( unsigned uColumnType, unsigned uSize );
   columns& add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias );
   columns& add( unsigned uColumnType, unsigned uSize, const std::string_view& stringName ) { return add( uColumnType, uSize, stringName, std::string_view{} ); }
   columns& add( unsigned uColumnType, unsigned uSize, const std::string_view& stringAlias, tag_alias ) { return add( uColumnType, uSize, std::string_view{}, stringAlias ); }
   columns& add( const std::vector< std::tuple< unsigned, unsigned, std::string_view > >& vectorColumn );
   columns& add( const std::string_view& stringType ) { return add( column( (unsigned)gd::types::type_g( stringType ) ) ); }
   columns& add( const std::string_view& stringType, const std::string_view& stringName ) { return add( (unsigned)gd::types::type_g( stringType ), 0, stringName, std::string_view{}); }
   columns& add( const std::string_view& stringType, unsigned uSize ) { return add( (unsigned)gd::types::type_g( stringType ), uSize ); }
   columns& add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringName ) { return add( (unsigned)gd::types::type_g( stringType ), uSize, stringName, std::string_view{}); }
   columns& add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringAlias, tag_alias ) { return add( (unsigned)gd::types::type_g( stringType ), uSize, std::string_view{}, stringAlias); }
   columns& add( const std::string_view& stringType, unsigned uSize, const std::string_view& stringName, const std::string_view& stringAlias ) { return add( (unsigned)gd::types::type_g( stringType ), uSize, stringName, stringAlias); }

   columns& add( const std::vector< std::pair< std::string_view, unsigned > >& vectorType, tag_type_name );
   columns& add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view > >& vectorType, tag_type_name );
   columns& add( const std::vector< std::tuple< std::string_view, unsigned, std::string_view, std::string_view > >& vectorType, tag_type_name );
   columns& add( const std::initializer_list< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name );
   columns& add( const std::vector< std::pair< std::string_view, std::string_view > >& vectorType, tag_type_name );
   columns& add( const std::vector< std::pair< unsigned, unsigned > >& vectorType, tag_type_constant );
   columns& add( const columns* p_ );
   ///@}


   /** \name OPERATION
   *///@{
   column* get( unsigned uIndex ) { assert( uIndex < size() ); return &m_vectorColumn[uIndex]; }
   const column* get( unsigned uIndex ) const { assert( uIndex < size() ); return &m_vectorColumn[uIndex]; }
#if defined(GD_TABLE_64BIT)
   column* get( size_t uIndex ) { assert( uIndex < size() ); return &m_vectorColumn[uIndex]; }
   const column* get( size_t uIndex ) const { assert( uIndex < size() ); return &m_vectorColumn[uIndex]; }
#endif
   int find_index( std::string_view stringName ) const noexcept;
   int find_index( const gd::variant_view& column_ ) const noexcept;

   unsigned state( unsigned uIndex ) const { return get( uIndex )->state(); }
   unsigned ctype( unsigned uIndex ) const { return get( uIndex )->ctype(); }
   unsigned ctype_number( unsigned uIndex ) const { return get( uIndex )->ctype_number(); }
   std::string_view ctype_name( unsigned uIndex ) const { return get( uIndex )->ctype_name(); }
   unsigned type( unsigned uIndex ) const { return get( uIndex )->type(); }
   unsigned type_number( unsigned uIndex ) const { return get( uIndex )->type_number(); }
   unsigned size( unsigned uIndex ) const { return get( uIndex )->size(); }
   unsigned primitive_size( unsigned uIndex ) const { return get( uIndex )->primitive_size(); }
   std::string_view type_name( unsigned uIndex ) const { return get( uIndex )->type_name(); }
   std::string_view name( unsigned uIndex ) const { return get( uIndex )->name(); }
   std::string_view alias( unsigned uIndex ) const { return get( uIndex )->alias(); }
   
   bool is_primitive( unsigned uIndex ) const { return get( uIndex )->is_primitive(); }

   iterator begin() { return m_vectorColumn.begin(); }
   iterator end() { return m_vectorColumn.end(); }
   const_iterator begin() const { return m_vectorColumn.begin(); }
   const_iterator end() const { return m_vectorColumn.end(); }
   const_iterator cbegin() const { return m_vectorColumn.cbegin(); }
   const_iterator cend() const { return m_vectorColumn.cend(); }

   size_t size() const noexcept { return m_vectorColumn.size(); }
   bool empty() const  noexcept { return m_vectorColumn.empty(); }
   void clear() { m_vectorColumn.clear(); }

   int get_reference() const { return m_iReference; }
   void add_reference() { m_iReference++; }
   void release();


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
   int m_iReference = 0;
   std::vector<column> m_vectorColumn;

   // ## free functions ------------------------------------------------------------
public:



};

/// decrease reference counter delete if no more references
inline void columns::release() {                                                                   assert( m_iReference > 0 );
   m_iReference--;
   if( m_iReference == 0 ) delete this; 
}

_GD_TABLE_DETAIL_END

#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
