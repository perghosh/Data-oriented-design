// @FILE [tag: lua, objects] [summary: Header file for LUA objects and utility functions] [type: header] [name: LUAObjects]

#pragma once

#include "lua/sol.hpp"

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_variant.h"
#include "gd/gd_variant_view.h"
#include "gd/gd_com.h"
#include "gd/gd_database.h"
#include "gd/gd_database_sqlite.h"


#ifndef LUA_BEGIN
#  define LUA_BEGIN namespace LUA {
#  define LUA_END }
#endif

class CDocument;
class CApplication;

LUA_BEGIN


/// convert all string values to string in vector
void ConvertTo_g( const sol::table& tableText, std::vector<std::string>& vecorText );
/// convert to vector with gd::variant values
void ConvertTo_g( const sol::table& tableValue, std::vector<gd::variant>& vecorValue );
void ConvertTo_g( const sol::table& tableValue, std::vector<gd::variant_view>& vecorValue );
/// convert table to arguments object and return it
gd::argument::arguments ConvertToArguments_g( const sol::table& tableValue );
/// insert table values to arguments object
void ConvertToArguments_g( const sol::table& tableValue, gd::argument::arguments& argumentsValue );
///
inline void ConvertToArguments_g( const sol::optional<sol::table>& table_, gd::argument::arguments& argumentsValue ) { if( table_.has_value() ) { ConvertToArguments_g( table_.value(), argumentsValue  ); } }
inline gd::argument::arguments ConvertToArguments_g( const sol::optional<sol::table>& table_ ) {
   gd::argument::arguments argumentsValue;
   ConvertToArguments_g( table_, argumentsValue );
   return argumentsValue;
}


std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::variant& value_ );
std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::variant_view& value_ );
gd::variant ConvertFromAny_g( const std::variant<int64_t, std::string, double, bool, sol::lua_nil_t>& any_ );


/** @CLASS [tag: Table. lua, wrapper] [summary: Wrapper for gd::table::table_column_buffer class in lua]
 * \brief Database wrapper in lua
 */
class Table
{
// ## construction -------------------------------------------------------------
public:
   Table() : m_ptable{ nullptr } { }
   Table( const std::string_view& stringColumns );
   Table( uint64_t uCount, const std::string_view& stringColumns );
   Table( void* ptable ) : m_ptable{ (gd::table::table_column_buffer*)ptable } { }
   // copy
   Table( const Table& o ) { common_construct( o ); }
   Table( Table&& o ) { common_construct( std::move( o ) ); }
   // assign
   Table& operator=( const Table& o ) { common_construct( o ); return *this; }
   Table& operator=( Table&& o ) { common_construct( o ); return *this; }

   ~Table();
private:
   // common copy
   void common_construct( const Table& o ) { m_ptable = new gd::table::table_column_buffer( *o.m_ptable ); }
   void common_construct( Table&& o ) { m_ptable = o.m_ptable; o.m_ptable = nullptr; }

// ## operator -----------------------------------------------------------------
public:
   gd::table::table_column_buffer& operator*() const { return *m_ptable; }

/** \name OPERATION
*///@{
   gd::table::table_column_buffer* GetTable() const { return m_ptable; }
   uint64_t GetColumnCount() const { return m_ptable->get_column_count(); }
   uint64_t GetRowCount() const { return m_ptable->get_row_count(); }
   
   /// Set attribute
   void SetColumnAttribute( std::variant<int64_t,std::string_view> column_, const std::string_view& stringAttribute, std::variant<int64_t,std::string_view> value_ );

   /// Return some type of selected column attribute for all columns in table as table
   sol::table GetColumns( const sol::variadic_args& variadicargs );

   std::variant<int64_t,std::string, double, bool, sol::lua_nil_t> GetCellValue( uint64_t uRow, std::variant<int64_t,std::string_view> column_ ) const;
   void SetCellValue( uint64_t uRow, std::variant<int64_t, std::string_view> column_, std::variant<int64_t,std::string, double, bool, sol::lua_nil_t> value_ );
   void SetCellValues( const Table& table, const sol::table& tableArguments );
   void Fill( std::variant<int64_t, std::string_view, sol::table> column_, std::variant<int64_t,std::string, double, bool, sol::lua_nil_t> value_ );

   /// Find value in table
   int64_t Find( std::variant<int64_t, std::string_view> column_, std::variant<int64_t,std::string, double, bool, sol::lua_nil_t> value_ );

   void AddRow( const sol::optional< std::variant<uint64_t,sol::table> > row_ );

   void Read( const std::string_view& stringFileOrData, const std::variant<std::string_view,sol::table>& format_ );
   std::string Write( const sol::table& tableOption );
//@}

// ## attributes ----------------------------------------------------------------
public:
   gd::table::dto::table* m_ptable;
};

/**
 * \brief Database wrapper in lua
 */
class Database
{
// ## construction -------------------------------------------------------------
public:
   Database() : m_pdatabase{ nullptr } { }
   Database( void* pdatabase ) : m_pdatabase{ (gd::database::database_i*)pdatabase } { }
   Database( sol::table tableOpen );
   // copy
   Database( const Database& o ) { common_construct( o ); }
   // assign
   Database& operator=( const Database& o ) { common_construct( o ); return *this; }

   ~Database();
private:
   // common copy
   void common_construct( const Database& o ) { m_pdatabase = o.m_pdatabase; }

// ## operator -----------------------------------------------------------------
public:

/** \name OPERATION
*///@{
   bool IsOpen() const { return (m_pdatabase != nullptr); }
   void Open( const std::variant<std::string_view,sol::table>& connect_ );
   void Execute( const std::string_view& stringSql );
   std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Ask( const std::string_view& stringSql, std::optional<std::string> type_ );
   void Close();
//@}

// ## attributes ----------------------------------------------------------------
public:
   bool m_bOwner = false;                 ///< if database object is owned
   gd::database::database_i* m_pdatabase; ///< pointer to database object
};

/**
 * \brief Database wrapper in lua
 */
class Cursor
{
// ## construction -------------------------------------------------------------
public:
   Cursor() {}
   Cursor( const Database* pdatabase ) {
      pdatabase->m_pdatabase->get_cursor( &m_pcursor );
   }
   // copy
   Cursor( const Cursor& o ) { common_construct( o ); }
   // assign
   Cursor& operator=( const Cursor& o ) { common_construct( o ); return *this; }

   ~Cursor() {}
private:
   // common copy
   void common_construct( const Cursor& o ) { m_pcursor = o.m_pcursor; }

// ## operator -----------------------------------------------------------------
public:

/** \name OPERATION
*///@{
   bool IsOpen() const { return m_pcursor->is_open(); }
   void Open( const std::string_view& stringSql, const sol::optional<sol::table>& params_ );
   void Next();
   bool IsValidRow() const;
   void Close();

   std::variant<int64_t,std::string, double, bool, sol::lua_nil_t> GetValue( const std::variant<int64_t,std::string_view>& column_ );

   Table GetTable();

   //std::variant<int64_t,std::string, double, bool, sol::lua_nil_t>
//@}

// ## attributes ----------------------------------------------------------------
public:
   gd::com::pointer<gd::database::cursor_i> m_pcursor;
};

/** @CLASS [tag: Document. lua, wrapper] [summary: Wrapper for CDocument class in lua]
 * \brief Document wrapper in lua
 */
class Document
{
// ## construction -------------------------------------------------------------
public:
   Document() : m_pdocument{ nullptr } { }
   Document( void* pdocument ) : m_pdocument{ (CDocument*)pdocument } { }
   // copy
   Document( const Document& o ) { common_construct( o ); }
   // assign
   Document& operator=( const Document& o ) { common_construct( o ); return *this; }

   ~Document() {}
private:
   // common copy
   void common_construct( const Document& o ) { m_pdocument = o.m_pdocument; }

// ## operator -----------------------------------------------------------------
public:

/** \name OPERATION
*///@{
   //Table GetCacheTable( const std::string_view& stringCacheId );
   //MetaTables GetMetaTables();
   //MetaFields GetMetaFields();
//@}

// ## attributes ----------------------------------------------------------------
public:
   CDocument* m_pdocument;
};


/** @CLASS [tag: Application. lua, wrapper] [summary: Wrapper for CApplication class in lua]
 * \brief Application wrapper in lua
 */
class Application
{
   // ## construction -------------------------------------------------------------
public:
   Application();
   Application( CApplication* papplication ) { m_papplication = papplication; }
   // copy
   Application( const Application& o ) { common_construct( o ); }
   // assign
   Application& operator=( const Application& o ) { common_construct( o ); return *this; }

   ~Application();
private:
   // common copy
   void common_construct( const Application& o ) {}

   // ## operator -----------------------------------------------------------------
public:

   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{
   Document GetDocument();

   /// Return total number of properties in application
   uint64_t GetPropertyCount();
   /// Get property value for name or index
   std::variant<int64_t,std::string, double, bool> GetProperty( const std::variant<int64_t,std::string>& index_ );
   /// Get property name for index
   std::string GetPropertyName( uint64_t uIndex ) const;
   /// Set property value
   void SetProperty( const std::string_view& stringName, std::variant<int64_t,std::string, double> value_ );
   /// Set logging level
   void SetLogLevel( const std::string_view& stringLevel );
   //@}

   /** \name OPERATION
   *///@{
   /// Initialize application object if script is free from web server
   void Initialize( std::optional<std::string> connect_database_ );
   /// Print message text to output
   void Message( const std::string_view& stringTypeOrMessage, std::optional<std::string> message_ );




   // ## attributes ----------------------------------------------------------------
public:
   bool m_bOwner = false;  ///< if application class owns `CApplication` object
   CApplication* m_papplication;
};

LUA_END