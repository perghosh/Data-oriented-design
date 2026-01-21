// @FILE [tag: database, sqlite] [description: wrapper class for SQLite database] [type: header]

/*
## class database
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | database(), database(sqlite3*), database(void*), database(void*, bool)             | Constructors for creating database objects with or without existing SQLite connections.       |
| Assignment          | operator=(const database&), operator=(database&&)                                 | Copy and move assignment operators for database objects.                                      |
| Connection          | open(stringView, unsigned), open(stringView)                                       | Open/create SQLite database files with optional flags.                                        |
| Status              | is_owner(), is_open()                                                              | Check database ownership and connection status.                                               |
| Execution           | execute(stringView), ask(stringView, variant*), transaction(variant_view)         | Execute SQL statements, query single values, and manage transactions.                       |
| Key Information     | get_insert_key(), get_insert_key(variant&), get_insert_key_raw()                  | Retrieve the last inserted row ID from the database.                                         |
| Change Information  | get_change_count()                                                                 | Get the number of rows affected by the last statement.                                       |
| Resource Management | close(), release()                                                                 | Close the database connection and release ownership.                                        |
| Access              | get_sqlite3(), set_flags(unsigned, unsigned), is_flag(unsigned)                   | Access the underlying SQLite handle and manage flags.                                       |
| Static Utilities    | open_s(stringView, int), execute_s(sqlite3*, stringView), bind_s(...), close_s(...) | Static utility functions for database operations independent of instances.                  |

## class cursor
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | cursor(), cursor(database*)                                                        | Constructors for creating cursor objects, optionally with a database connection.              |
| Assignment          | operator=(const cursor&), operator=(cursor&&)                                     | Copy and move assignment operators for cursor objects.                                        |
| Connection          | open(), open(stringView), open(stringView, function)                               | Execute SQL queries and prepare result sets.                                                  |
| Preparation         | prepare(stringView), prepare(stringView, initializer_list), prepare(stringView, vector) | Prepare SQL statements with optional parameter binding.                                     |
| Parameter Binding   | bind_parameter(int, variant_view), bind_parameter(int, initializer_list), bind_parameter(int, vector) | Bind values to SQL statement parameters.                                                     |
| Execution           | execute()                                                                          | Execute prepared statements.                                                                 |
| Navigation          | next(), reset(), is_valid_row()                                                    | Navigate through result sets and check row validity.                                         |
| Data Access         | operator[](unsigned), operator[](stringView), get_variant(), get_variant_view()   | Access column values as variants or variant views.                                          |
| Metadata            | get_column_count(), get_parameter_count(), get_parameter_name()                    | Retrieve information about result columns and parameters.                                     |
| Record Access       | get_record(), get_record() const, operator const record&()                         | Access the underlying record structure.                                                      |
| Resource Management | close()                                                                            | Close the cursor and release resources.                                                     |
| Index Operations    | get_index(stringView)                                                              | Get column index by name.                                                                    |
| Static Utilities    | get_column_type_s(const char*), get_column_ctype_s(const char*), bind_columns_s(...) | Static utility functions for type detection and column binding.                             |

## class database_i
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | database_i(), database_i(stringView), database_i(stringView, stringView)          | Constructors for creating interface objects with optional name and dialect.                    |
| Assignment          | operator=(database_i&&)                                                            | Move assignment operator for database interface objects.                                     |
| Interface           | query_interface(guid, void**), add_reference(), release()                           | COM-style interface methods for reference counting and queryInterface.                       |
| Connection          | open(stringView), open(arguments)                                                  | Open database connections using connection strings or arguments.                             |
| Execution           | execute(stringView), ask(stringView, variant*), transaction(variant_view)         | Execute SQL statements, query single values, and manage transactions.                         |
| Cursor Creation     | get_cursor(cursor_i**)                                                             | Create and retrieve cursor objects for query execution.                                       |
| Resource Management | close(), erase(), get_pointer()                                                    | Close connections, erase database, and access internal pointer.                               |
| Metadata            | name(), dialect(), set(stringView, variant_view)                                  | Get/set database name, dialect, and configuration properties.                                |
| Key Information     | get_insert_key()                                                                   | Retrieve the last inserted row ID.                                                           |
| Change Information  | get_change_count()                                                                 | Get the number of rows affected by the last statement.                                       |


## class cursor_i
| Area                | Methods (Examples)                                                                 | Description                                                                                   |
|---------------------|------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
| Construction        | cursor_i(), cursor_i(database*)                                                    | Constructors for creating cursor interface objects, optionally with a database connection.    |
| Assignment          | operator=(cursor_i&&)                                                              | Move assignment operator for cursor interface objects.                                        |
| Interface           | query_interface(guid, void**), add_reference(), release()                           | COM-style interface methods for reference counting and queryInterface.                       |
| Connection          | open(), open(stringView)                                                           | Execute SQL queries and prepare result sets.                                                 |
| Preparation         | prepare(stringView), prepare(stringView, vector)                                   | Prepare SQL statements with optional parameter binding.                                      |
| Parameter Binding   | bind(vector), bind(unsigned, vector)                                               | Bind values to SQL statement parameters.                                                     |
| Execution           | execute()                                                                          | Execute prepared statements.                                                                 |
| Navigation          | next()                                                                             | Advance to the next row in the result set.                                                  |
| Data Access         | get_record(record**), get_record(), get_record() const                             | Access the underlying record structure.                                                      |
| Status              | is_open(), is_valid_row()                                                          | Check cursor state and row validity.                                                         |
| Resource Management | close()                                                                            | Close the cursor and release resources.                                                     |
| Metadata            | get_column_count()                                                                 | Get the number of columns in the result set.                                                |




*Sample on how to write vector storing pair double values in to database*
~~~(.cpp)
TEST_CASE( "Write pair with doubles into sqlite database", "[Write]" ) {
   // ## custom code to get a database connection, not relevant to sample
   if( false == ( bool )pdatabase_g ) connect_database();
   gd::database::sqlite::database databaseActive( *pdatabase_g, false );


   // ## create vector with double numbers
   std::vector< std::pair<double, double> > vectorNumberToWrite;
   for( int i = 0; i < 10; i++ ) { vectorNumberToWrite.push_back( { double(i), double(i + 1.0) }); }

   // ## create table storing double values
   gd::table::table_column_buffer table(10);                                   // start with 10 rows
   table.column_add( { {"double", 0 }, {"double", 0 } }, gd::table::tag_type_name{});
   table.prepare();                                                            // prepare table, here buffers are allocated

   // ## copy values from vector to table
   for( auto it : vectorNumberToWrite )
   {
      auto uRow = table.get_row_count();
      table.row_add();                                                         // make sure that row exists

      // ## set values to column 0 and column 1
      table.cell_set( uRow, 0, it.first );
      table.cell_set( uRow, 1, it.second );
   }

   // ## insert values to database

   // ### sql query used to insert
   gd::database::sqlite::cursor  cursorInsert( &databaseActive );
   std::string stringSql = "INSERT INTO TMeasurement(FMean, FVariance, FSquaredError, FNSamples) VALUES (?,?,?,?)"; // test query to insert something to
   auto [bOk, stringError] = cursorInsert.prepare( stringSql );                                    REQUIRE( bOk == true );

   // ### insert table values to table
   for( decltype(table.get_row_count()) uRow = 0; uRow < table.get_row_count(); uRow++ )
   {
      auto vectorAddRow = table.cell_get_variant_view( uRow );

      // ## add two dummy values, need to complete values in TMeasurement table
      vectorAddRow.push_back( double(9999.9) );
      vectorAddRow.push_back( int64_t(888) );

      std::tie( bOk, stringError ) = cursorInsert.bind_parameter( vectorAddRow );// Bind row values to query
      if( bOk == true )
      {
         std::tie( bOk, stringError ) = cursorInsert.execute();                                    REQUIRE( bOk == true );
      }
   }

   cursorInsert.close();
}

~~~

*/

#pragma once

#ifdef GD_DATABASE_SQLITE_USE

#include <cassert>
#include <cstring>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_arguments.h"
#include "gd_database.h"
#include "gd_database_types.h"
#include "gd_database_record.h"

#include "sqlite/sqlite3.h"



#ifndef _GD_DATABASE_SQLITE_BEGIN
#define _GD_DATABASE_SQLITE_BEGIN namespace gd { namespace database { namespace sqlite {
#define _GD_DATABASE_SQLITE_END } } }
_GD_DATABASE_SQLITE_BEGIN
#else
_GD_DATABASE_SQLITE_BEGIN
#endif


/** ===========================================================================
 * \brief Wrapper class for SQLite database
 *
 *
 *
 \code
 \endcode
 */
class database
{
// ## construction -------------------------------------------------------------
public:
   database() : m_uFlags( 0 ), m_psqlite3{ nullptr } {}
   database( sqlite3* psqlite3 ) : m_uFlags( eDatabaseStateOwner ), m_psqlite3{ psqlite3 } {}
   database( void* psqlite3 ) : m_uFlags( eDatabaseStateOwner ), m_psqlite3{ (sqlite3*)psqlite3 } {}
   database( void* psqlite3, bool bOwner ) : m_uFlags( bOwner == true ? eDatabaseStateOwner : 0 ), m_psqlite3{ (sqlite3*)psqlite3 } {}

   // copy
   database(const database& o) { common_construct(o); }
   database(database&& o) noexcept { common_construct( std::move(o) ); }
   // assign
   database& operator=(const database& o) { common_construct(o); return *this; }
   database& operator=(database&& o) noexcept { common_construct(o); return *this; }

   ~database() { close(); }
private:
// common copy
   void common_construct(const database& o) {
      m_psqlite3 = o.m_psqlite3; m_uFlags = o.m_uFlags;
   }
   void common_construct(database&& o) noexcept {
      m_psqlite3 = o.m_psqlite3; m_uFlags = o.m_uFlags;
      o.m_psqlite3 = nullptr;
   }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   void set_flags( unsigned uSet, unsigned uClear ) { m_uFlags |= uSet; m_uFlags &= ~uClear;  }
   bool is_flag( unsigned uFlag ) const noexcept { return (m_uFlags & uFlag) == uFlag;  }

   sqlite3* get_sqlite3() const { return m_psqlite3; }
//@}

   // @API [tag: open, connect] [description: conenct to or create database using open]

   std::pair<bool, std::string> open(const std::string_view& stringFileName, const std::vector<std::string_view>& vectorFlags );
   std::pair<bool, std::string> open(const std::string_view& stringFileName, unsigned uFlags );
   std::pair<bool, std::string> open(const std::string_view& stringFileName) { return open( stringFileName, 0 ); }

   /// check if database owns connection
   bool is_owner() const { return ((m_uFlags & eDatabaseStateOwner) == eDatabaseStateOwner); }

   /// check if database is connected
   bool is_open() { return ((m_uFlags & eDatabaseStateConnected) == eDatabaseStateConnected); }

   /// Execute sql, any sql
   std::pair<bool, std::string> execute(const std::string_view& stringQuery);

   // @API [tag: execute] [description: execute statements against database]

   /// Execute sql, any sql and pick upp result values for statements executed
   std::pair<bool, std::string> execute(const std::string_view& stringQuery, std::function<bool( const gd::argument::arguments* )> callback_ );

   /// Ask for single value from database, handy to use without fiddle with cursor
   std::pair<bool, std::string> ask( const std::string_view& stringStatement, gd::variant* pvariantValue );

   /// execution operation related to transaction logic
   std::pair<bool, std::string> transaction( const gd::variant_view& transaction_ );

   /// get last inserted key
   gd::variant get_insert_key() const;
   std::pair<bool, std::string> get_insert_key( gd::variant& variantKey ) const;
   int64_t get_insert_key_raw() const;

   /// get number of changes in last call to database
   gd::variant get_change_count() const;

   /// close sqlite connection if open
   void close() { if( is_owner() == true ) { close_s(m_psqlite3); } m_psqlite3 = nullptr; }

   /// Release internal pointer to sqlite database
   sqlite3* release() { sqlite3* psqlite3 = m_psqlite3; m_psqlite3 = nullptr; return psqlite3; }

//@}


// ## attributes ----------------------------------------------------------------
public:
   unsigned m_uFlags = 0;              ///< flags to mark state for database object
   sqlite3* m_psqlite3 = nullptr;      ///< pointer to sqlite database connection


// ## free functions ------------------------------------------------------------
public:

   static std::pair<sqlite3*, std::string> open_s( const std::string_view& stringFileName, const std::vector<std::string_view>& vectorFlags);
   static std::pair<sqlite3*, std::string> open_s(const std::string_view& stringFileName, int iFlags);
   static std::pair<sqlite3*, std::string> open_s( const std::string_view& stringFileName ) { return open_s( stringFileName, 0 ); }
   static std::pair<bool, std::string> execute_s(sqlite3* psqlite, const std::string_view& stringQuery);

   static std::pair<bool, std::string> bind_s(sqlite3* psqlite, const std::vector<gd::variant_view>&);

   static void close_s(sqlite3* psqlite);
};

inline std::pair<bool, std::string> database::execute(const std::string_view& stringQuery) { return database::execute_s(m_psqlite3, stringQuery); }

inline int64_t database::get_insert_key_raw() const {                                              assert( m_psqlite3 != nullptr );
   return ::sqlite3_last_insert_rowid( m_psqlite3 );
}



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
   cursor(): m_uState(0), m_pstmt(nullptr), m_pdatabase(nullptr) {}
   cursor( database* pdatabase ) : m_uState( 0 ), m_pstmt( nullptr ), m_pdatabase( pdatabase ) { assert( pdatabase != nullptr ); assert( pdatabase->get_sqlite3() != nullptr ); }
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
   bool is_open() const noexcept { return m_pstmt != nullptr; }
   /// Open SQL SELECT query
   std::pair<bool, std::string> open();
   std::pair<bool, std::string> open(const std::string_view& stringSql);
   std::pair<bool, std::string> open(const std::string_view& stringSql, std::function<bool( sqlite3_stmt* pstmt )>);


   /// @name prepare
   /// @brief Prepare sql statement that have parameters that should be bound
   ///@{
   std::pair<bool, std::string> prepare(const std::string_view& stringSql);
   std::pair<bool, std::string> prepare(const std::string_view& stringSql, std::initializer_list< gd::variant_view > vectorValue );
   std::pair<bool, std::string> prepare(const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue );
   ///@}

   std::pair<bool, std::string> bind_parameter( int iIndex, const gd::variant_view& VVValue );
   std::pair<bool, std::string> bind_parameter( int iOffset, std::initializer_list< gd::variant_view > vectorValue );
   std::pair<bool, std::string> bind_parameter( int iOffset, const std::vector< gd::variant_view >& vectorValue );
   std::pair<bool, std::string> bind_parameter( std::initializer_list< gd::variant_view > vectorValue ) { return bind_parameter( 1, vectorValue ); }
   std::pair<bool, std::string> bind_parameter( const std::vector< gd::variant_view >& vectorValue ) { return bind_parameter( 1, vectorValue ); }

   std::pair<bool, std::string> execute();

   void update( sqlite3_stmt* pstmt = nullptr ) { update(0, (unsigned)m_recordRow.size(), pstmt); }
   void update( unsigned uFrom, unsigned uTo, sqlite3_stmt* pstmt = nullptr );

   /// go to next row
   std::pair<bool, std::string> next();
   /// check if row is valid
   bool is_valid_row() const { return (m_uState & eCursorStateRow) == eCursorStateRow; }

   /// clear bindings but do not close statement, useful for select cursors
   std::pair<bool, std::string> reset();

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
   unsigned m_uState;            ///< cursor state
   sqlite3_stmt* m_pstmt;        ///< sqlite statement for active result
   database* m_pdatabase;        ///< database cursor reads data from
   record m_recordRow;           ///< buffer used to store data from active row


// ## free functions ------------------------------------------------------------
public:
   static unsigned get_column_type_s( const char* pbszColumnType );
   static unsigned get_column_ctype_s( const char* pbszColumnType );
   static void bind_columns_s( sqlite3_stmt* pstmt, record& recordBindTo);

};

/// get number of sql parameters for active statement
inline unsigned cursor::get_parameter_count() {                                                    assert( m_pstmt != nullptr );
   return ::sqlite3_bind_parameter_count( m_pstmt );
}

/// get number of sql parameters for active statement
inline std::string_view cursor::get_parameter_name( unsigned uIndex ) {                            assert( m_pstmt != nullptr ); assert( uIndex > 0 && uIndex < 10000 );
   return ::sqlite3_bind_parameter_name( m_pstmt, uIndex );
}



/// wrapper to prepare and bind parameters
inline std::pair<bool, std::string> cursor::prepare( const std::string_view& stringSql, std::initializer_list< gd::variant_view > vectorValue ) {
   auto result_ = prepare( stringSql );
   if( result_.first == true ) {
      result_ = bind_parameter( 1, vectorValue );
   }

   return result_;
}

/// wrapper to prepare and bind parameters
inline std::pair<bool, std::string> cursor::prepare( const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue ) {
   auto result_ = prepare( stringSql );
   if( result_.first == true ) {
      result_ = bind_parameter( 1, vectorValue );
   }

   return result_;
}

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
   if(m_pstmt != nullptr) {
      ::sqlite3_finalize(m_pstmt); m_pstmt = nullptr;
      m_pstmt = nullptr;
      m_uState = 0;
      m_recordRow.clear();
   }
}




_GD_DATABASE_SQLITE_END

_GD_DATABASE_SQLITE_BEGIN

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- cursor_i
// ----------------------------------------------------------------------------


/** --------------------------------------------------------------------------
 * \brief Cursor interface to handle data returned from SQL SELECT queries
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
   cursor_i( cursor_i&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   cursor_i& operator=( cursor_i&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   virtual ~cursor_i() {
      if( m_pcursor ) { m_pcursor->close(); }
      m_pcursor = nullptr;
   }
private:
   // common copy                                                                        is_open
   //void common_construct( const cursor_i& o ) { m_pdatabase = o.m_pdatabase; m_stringName = m_stringName; }
   void common_construct( cursor_i&& o ) noexcept {
      m_pcursor = std::move(o.m_pcursor); m_iReference = o.m_iReference;
   }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:

   // @API [tag: interface] [description: default interface to work with cursor]

   int32_t query_interface(const com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   unsigned get_column_count() override { return m_pcursor->get_column_count(); }
   bool is_valid_row() override { return m_pcursor->is_valid_row(); }
   std::pair<bool, std::string> prepare(const std::string_view& stringSql ) override;
   std::pair<bool, std::string> prepare(const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue ) override;
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

   // @API [tag: utility] [description: methods used to simplify work with cursor]

   void attach( cursor* pcursor ) { m_pcursor.reset( pcursor ); }
   cursor* detach() { return m_pcursor.release(); }

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


/** --------------------------------------------------------------------------
 * \brief Database interface to handle database connection
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

   ~database_i() {
      if( m_pdatabase ) { m_pdatabase->close(); }
      m_pdatabase = nullptr;
   }
private:
   // common copy
   //void common_construct( const database_i& o ) { m_pdatabase = o.m_pdatabase; m_stringName = m_stringName; }
   void common_construct( database_i&& o ) noexcept {
      m_pdatabase = std::move( o.m_pdatabase ); m_stringName = std::move( o.m_stringName );
   }

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   void set_dialect( const std::string_view& stringDialect ) { m_stringDialect = stringDialect; }
//@}

  // @API [tag: interface] [description: default interface to work with database]

   int32_t query_interface(const com::guid& guidId, void** ppObject) override;
   unsigned add_reference() override { m_iReference++; return (unsigned)m_iReference; }
   unsigned release() override;

   std::string_view name() const override { return m_stringName; }
   std::string_view dialect() const override { return m_stringDialect; }
   void set( const std::string_view& stringName, const gd::variant_view& value_ ) override;
   std::pair<bool, std::string> open( const std::string_view& stringDriverConnect ) override;
   std::pair<bool, std::string> open( const gd::argument::arguments& argumentsConnect ) override;
   std::pair<bool, std::string> execute( const std::string_view& stringStatement ) override;
   std::pair<bool, std::string> execute( const std::string_view& stringStatement, std::function<bool( const gd::argument::arguments* )> callback_ ) override;
   std::pair<bool, std::string> ask( const std::string_view& stringStatement, gd::variant* pvariantValue ) override;
   std::pair<bool, std::string> transaction(const gd::variant_view& transaction_) override;

   std::pair<bool, std::string> get_cursor( gd::database::cursor_i** ppCursor ) override;

   void close() override;
   void erase() override;

   void* get_pointer() override { return m_pdatabase.get(); }

   gd::variant get_change_count() override;
   gd::variant get_insert_key() override;

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


// ## free functions ------------------------------------------------------------
public:
   std::unique_ptr<database> m_pdatabase;
   std::string m_stringName;     ///< database management system
   std::string m_stringDialect;  ///< sql dialect used for database
   int m_iReference = 1;


};

_GD_DATABASE_SQLITE_END

#endif // GD_DATABASE_SQLITE_USE
