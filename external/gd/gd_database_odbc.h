// @FILE [tag: database, odbc] [description: wrapper class for ODBC database] [type: header] [name: gd_database_odbc.h]

#pragma once

#ifdef GD_DATABASE_ODBC_USE

#include <cassert>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#ifndef WIN32
#else
#  include <windows.h>
#endif

#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_arguments.h"
#include "gd_database.h"
#include "gd_database_types.h"
#include "gd_database_record.h"

#include <sql.h>
#include <sqlext.h>
#include <sqlext.h>


#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma clang diagnostic ignored "-Wunused-value"
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wdeprecated-enum-compare"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
   #pragma GCC diagnostic ignored "-Wunused-value"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 26495 26812 )
#endif


// ## forward declare gd code
namespace gd::argument { class arguments; }


#ifndef _GD_DATABASE_ODBC_BEGIN
#define _GD_DATABASE_ODBC_BEGIN namespace gd { namespace database { namespace odbc {
#define _GD_DATABASE_ODBC_END } } }
_GD_DATABASE_ODBC_BEGIN
#else
_GD_DATABASE_ODBC_BEGIN
#endif


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class database
{
public:
   enum enumFlag
   {
//      eFlagOwner                    = 0b0000'0000'0000'0001,      ///< if database owns the internal connection
//      eFlagConnected                = 0b0000'0000'0000'0010,      ///< if object is connected to database
   };

// ## construction -------------------------------------------------------------
public:
   database() {}
   database( SQLHANDLE hEnvironment ): m_uFlags( eDatabaseStateOwner ), m_hEnvironment( hEnvironment ) {}
   database( SQLHANDLE hEnvironment, SQLHANDLE hDatabase ): m_hEnvironment(hEnvironment), m_hDatabase(hDatabase) {}
   // copy
   database( const database& o ) { common_construct( o ); }
   database( database&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   database& operator=( const database& o ) { common_construct( o ); return *this; }
   database& operator=( database&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~database() { deallocate(); }
private:
   // common copy
   void common_construct( const database& o ) {
      m_uFlags = o.m_uFlags; m_hEnvironment = o.m_hEnvironment; m_hDatabase = o.m_hDatabase;
      set_flags( 0, eDatabaseStateOwner );
   }
   // move
   void common_construct( database&& o ) noexcept {
      m_uFlags = o.m_uFlags; m_hEnvironment = o.m_hEnvironment; m_hDatabase = o.m_hDatabase;
      o.m_uFlags = 0; o.m_hEnvironment = nullptr;
   }

// ## operator -----------------------------------------------------------------
public:
   operator SQLHANDLE() const { return m_hDatabase; }


// ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{
   void set_flags( unsigned uSet, unsigned uClear ) { m_uFlags |= uSet; m_uFlags &= ~uClear;  }
   bool is_flag( unsigned uFlag ) const noexcept { return (m_uFlags & uFlag) == uFlag;  }

   //@}

   /** \name OPERATION
   *///@{
   /// Allocate needed resources to open database, if not allocated this is called from `open` methods
   std::pair<bool, std::string> allocate();
   ///@{ open database (open is same as connect to database)
   std::pair<bool, std::string> open( const std::string_view& stringDriverConnect );
   std::pair<bool, std::string> open( const gd::argument::arguments& argumentsConnect );
   ///@}
   /// check if database owns connection
   bool is_owner() const { return ((m_uFlags & eDatabaseStateOwner) == eDatabaseStateOwner); }
   /// check if database is connected
   bool is_open();

   /// Execute sql statement that don't produce a cursor
   std::pair<bool, std::string> execute( const std::string_view& stringStatement );

   /// Ask for single value from database, handy to use without fiddle with cursor
   std::pair<bool, std::string> ask( const std::string_view& stringStatement, gd::variant* pvariantValue );

   /// Execute a transaction operation based on the provided variant view.
   std::pair<bool, std::string> transaction(const gd::variant_view& transaction_);


   void close();
   bool error( std::string& stringError );
   bool error( SQLHANDLE hStatement, std::string& stringError );
   std::string error() { std::string stringError; error( stringError ); return stringError; }
   std::string error( SQLHANDLE hStatement ) { std::string stringError; error( hStatement, stringError ); return stringError; }
   void deallocate();

   /// Release internal odbc connection
   SQLHANDLE release() { SQLHANDLE hDatabase = m_hDatabase; m_hDatabase = nullptr; return m_hDatabase; }


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
   unsigned m_uFlags = 0;                 ///< flags to mark state for database object
   SQLHANDLE m_hEnvironment = nullptr;    ///< odbc environment handle
   SQLHANDLE m_hDatabase = nullptr;       ///< odbc database handle


// ## free functions ------------------------------------------------------------
public:
   /// check if statement is valid
   static bool is_statement_handle_open_s( SQLHANDLE hStatement );


};


/** ===========================================================================
 * \brief Handle the data returned from SQL SELECT queries
 *
 *
 *
 \code
 \endcode
 */
class cursor
{
// ## construction -------------------------------------------------------------
public:
   cursor(): m_uState(0), m_hStatement(nullptr), m_pdatabase(nullptr) {}
   cursor( database* pdatabase ) : m_uState( 0 ), m_hStatement( nullptr ), m_pdatabase( pdatabase ) { assert( pdatabase != nullptr ); }
   // copy
   cursor(const cursor& o) { common_construct(o); }
   cursor(cursor&& o) noexcept { common_construct(o); }
   // assign
   cursor& operator=(const cursor& o) { common_construct(o); return *this; }
   cursor& operator=(cursor&& o) noexcept { common_construct(o); return *this; }

   ~cursor() { close(); }
private:
   // common copy
   void common_construct(const cursor& o) {}
   void common_construct(cursor&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   // ## Index operators, returns variant_view with value from column
   //    Using index to column or column name it is possible to access column value
   //    matching index or name.
   gd::variant_view operator[](unsigned uIndex) const { return get_variant_view(uIndex); }
   gd::variant_view operator[](const std::string_view& stringName) const { return get_variant_view(stringName); }

   /// get internal record that has information about columns
   operator const record& () const { return m_recordRow; }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   const record* get_record() const { return &m_recordRow; }
   record* get_record() { return &m_recordRow; }
   unsigned get_column_count() const { return (unsigned)m_recordRow.size(); }
   unsigned get_parameter_count();
   std::string_view get_parameter_name( unsigned uIndex );
//@}

/** \name OPERATION
*///@{
   bool is_open() const noexcept { return m_hStatement != nullptr; }
   /// Open SQL prepared SELECT query
   std::pair<bool, std::string> open();
   /// Open SQL SELECT query
   std::pair<bool, std::string> open(const std::string_view& stringSql);
   //std::pair<bool, std::string> open(const std::string_view& stringSql, std::function<bool( sqlite3_stmt* pstmt )>);

   /// @name prepare
   /// @brief Prepare sql statement that have parameters that should be bound
   ///@{
   std::pair<bool, std::string> prepare(const std::string_view& stringSql);
   std::pair<bool, std::string> prepare(const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue);
   ///@}

   /// @name bind_parameter
   /// @brief bind parameter values
   ///@{
   std::pair<bool, std::string> bind_parameter( int iIndex, const gd::variant_view& VVValue );
   std::pair<bool, std::string> bind_parameter( int iIndex, const gd::variant& VVValue, tag_variant );
   std::pair<bool, std::string> bind_parameter( int iOffset, std::initializer_list< gd::variant_view > vectorValue );
   std::pair<bool, std::string> bind_parameter( int iOffset, const std::vector< gd::variant_view >& vectorValue );
   std::pair<bool, std::string> bind_parameter( std::initializer_list< gd::variant_view > vectorValue ) { return bind_parameter( 1, vectorValue ); }
   std::pair<bool, std::string> bind_parameter( const std::vector< gd::variant_view >& vectorValue ) { return bind_parameter( 1, vectorValue ); }
   ///@}

   std::pair<bool, std::string> add_columns( SQLHANDLE hStatement, record& recordBindTo, unsigned uCount );

   /// bind buffers to database values
   std::pair<bool, std::string> bind();

   std::pair<bool, std::string> execute();

   std::pair<bool, std::string> next( bool* pbIsOnRow, int iCount );
   bool next() { bool bRow; next( &bRow, get_column_count() ); return bRow; }

   void update() { update(0, (unsigned)m_recordRow.size()); }
   void update( unsigned uFrom, unsigned uTo );
   std::pair<bool, std::string> update_blob();

   /// check if row is valid
   bool is_valid_row() const { return (m_uState & eCursorStateRow) == eCursorStateRow; }



   /// close statement if open
   void close();

   // ## `variant` methods, return value(s) as variants
   std::vector<gd::variant> get_variant() const;
   gd::variant get_variant( unsigned uColumnIndex ) const;
   std::vector<gd::variant_view> get_variant_view() const;
   gd::variant_view get_variant_view( unsigned uColumnIndex ) const;
   gd::variant_view get_variant_view( const std::string_view& stringName ) const;
   std::vector<gd::variant_view> get_variant_view( const std::vector<unsigned>& vectorIndex ) const;
   gd::argument::arguments get_arguments() const;

   int get_index( const std::string_view& stringName ) const;
//@}


// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uState;            ///< cursor state
   SQLHANDLE m_hStatement;
   database* m_pdatabase;        ///< database cursor reads data from 
   record m_recordRow;           ///< buffer used to store data from active row

// ## free functions ------------------------------------------------------------
public:
   static unsigned get_column_type_s( int iSqlType );
   static unsigned get_column_ctype_s( int iSqlType );
   static int get_column_type_s( gd::types::enumTypeNumber eType );
   static int get_column_ctype_s( gd::types::enumTypeNumber eType );
   static std::string_view get_column_ctype_name_s( int iCType );

};

/// bind value to parameter in active statement
inline std::pair<bool, std::string> cursor::bind_parameter( int iOffset, std::initializer_list< gd::variant_view > vectorValue ) {
   int iIndex = iOffset;
   for( const auto& it : vectorValue ) {
      auto result_ = bind_parameter( iIndex, it );
      if( result_.first == false ) return result_;
      iIndex++;
   }
   return { true, "" };
}

/// bind value to parameter in active statement from vector
inline std::pair<bool, std::string> cursor::bind_parameter( int iOffset, const std::vector< gd::variant_view >& vectorValue ) {
   int iIndex = iOffset;                                                                           assert( iIndex != 0 ); // binding parameters are one based in sqlite
   for( const auto& it : vectorValue ) {
      auto result_ = bind_parameter( iIndex, it );
      if( result_.first == false ) return result_;
      iIndex++;
   }
   return { true, "" };
}


/// close cursor if open, open is same as it has one active statement
inline void cursor::close() {
   if(m_hStatement != nullptr) { 
      ::SQLFreeHandle( SQL_HANDLE_STMT, m_hStatement );
      m_hStatement = nullptr; 
      m_uState = 0; 
      m_recordRow.clear();
   }
}




_GD_DATABASE_ODBC_END


_GD_DATABASE_ODBC_BEGIN

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- cursor_i
// ----------------------------------------------------------------------------


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class cursor_i : public gd::database::cursor_i
{
// ## construction -------------------------------------------------------------
public:
   cursor_i() {}
   cursor_i( database* pdatabase ) { m_pcursor = std::make_unique<cursor>( pdatabase ); }
   // copy
   //cursor_i( const cursor_i& o ) { common_construct( o ); }
   cursor_i( cursor_i&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   //cursor_i& operator=( const cursor_i& o ) { common_construct( o ); return *this; }
   cursor_i& operator=( cursor_i&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   virtual ~cursor_i() { 
      if( m_pcursor ) { m_pcursor->close(); }
      m_pcursor = nullptr;
   }
private:
   // common copy
   //void common_construct( const cursor_i& o ) { m_pdatabase = o.m_pdatabase; m_stringName = m_stringName; }
   void common_construct( cursor_i&& o ) noexcept {
      m_pcursor = std::move(o.m_pcursor); m_iReference = o.m_iReference;
   }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   int32_t query_interface(const com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   unsigned get_column_count() override { return m_pcursor->get_column_count(); }
   bool is_valid_row() override { return m_pcursor->is_valid_row(); }
   std::pair<bool, std::string> prepare(const std::string_view& stringSql ) override;
   std::pair<bool, std::string> prepare( const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue ) override;
   std::pair<bool, std::string> bind( const std::vector< gd::variant_view >& vectorValue ) override;
   std::pair<bool, std::string> bind( unsigned uIndex, const std::vector< gd::variant_view >& vectorValue ) override;
   std::pair<bool, std::string> open() override;
   std::pair<bool, std::string> open( const std::string_view& stringStatement ) override;
   std::pair<bool, std::string> next() override;
   bool is_open() override { return m_pcursor->is_open(); }
   std::pair<bool, std::string> execute() override;
   std::pair<bool, std::string> get_record( record** ppRecord ) override;
   record* get_record() override { return m_pcursor->get_record(); }
   const record* get_record() const override { return m_pcursor->get_record(); }
   void close() override;

//@}


public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:


// ## free functions ------------------------------------------------------------
public:
   std::unique_ptr<cursor> m_pcursor;
   int m_iReference = 1;


};

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------- database_i
// ----------------------------------------------------------------------------

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class database_i : public gd::database::database_i
{
// ## construction -------------------------------------------------------------
public:
   database_i() { m_pdatabase = std::make_unique<database>(); }
   database_i( const std::string_view& stringName ): m_stringName(stringName) { m_pdatabase = std::make_unique<database>(); }
   database_i( const std::string_view& stringName, const std::string_view& stringDialect ): m_stringName(stringName), m_stringDialect(stringDialect) { m_pdatabase = std::make_unique<database>(); }
   database_i( database* pdatabase ): m_pdatabase(pdatabase) {}
   // copy
   //database_i( const database_i& o ) { common_construct( o ); }
   database_i( database_i&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   //database_i& operator=( const database_i& o ) { common_construct( o ); return *this; }
   database_i& operator=( database_i&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   virtual ~database_i() { 
      if( m_pdatabase ) { m_pdatabase->close(); }
      m_pdatabase = nullptr;
   }
private:
   // common copy
   //void common_construct( const database_i& o ) { m_pdatabase = o.m_pdatabase; m_stringName = m_stringName; }
   void common_construct( database_i&& o ) noexcept {
      m_pdatabase = std::move(o.m_pdatabase); m_stringName = std::move( m_stringName ); m_iReference = o.m_iReference;
   }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   void set_dialect( const std::string_view& stringDialect ) { m_stringDialect = stringDialect; }
//@}

/** \name OPERATION
*///@{
   int32_t query_interface(const com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   std::string_view name() const override { return m_stringName; }
   std::string_view dialect() const override { return m_stringDialect; }
   void set( const std::string_view& stringName, const gd::variant_view& value_ ) override;
   std::pair<bool, std::string> open( const std::string_view& stringDriverConnect ) override;
   std::pair<bool, std::string> open( const gd::argument::arguments& argumentsConnect ) override;
   std::pair<bool, std::string> execute( const std::string_view& stringStatement ) override;
   std::pair<bool, std::string> ask( const std::string_view& stringStatement, gd::variant* pvariantValue ) override;
   std::pair<bool, std::string> transaction(const gd::variant_view& transaction_) override;
   std::pair<bool, std::string> get_cursor( gd::database::cursor_i** ppCursor ) override;
   void close() override;
   void erase() override;
   void* get_pointer() override { return m_pdatabase.get(); }
   gd::variant get_change_count() override;
   gd::variant get_insert_key() override;

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   std::unique_ptr<database> m_pdatabase;
   std::string m_stringName;     ///< database management system
   std::string m_stringDialect;  ///< sql dialect used for database
   int m_iReference = 1;
   enumServerName m_eServerName = eServerNameUnknown;


// ## free functions ------------------------------------------------------------
public:


};

_GD_DATABASE_ODBC_END



#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif

#endif // GD_DATABASE_ODBC_USE
