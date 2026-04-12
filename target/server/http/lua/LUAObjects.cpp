// @FILE [tag: lua, objects] [summary: Implementation file for LUA objects and utility functions] [type: source] [name: LUAObjects.cpp]

#include <filesystem>

#include "../Document.h"
#include "../Application.h"

#include "LUAObjects.h"

LUA_BEGIN

void ConvertTo_g( const sol::table& tableText, std::vector<std::string>& vecorText )
{
   for( auto& it : tableText )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::string ) { vecorText.push_back( value_.as<std::string>() ); }
   }
}


void ConvertTo_g( const sol::table& tableValue, std::vector<gd::variant>& vecorValue )
{
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { vecorValue.push_back( value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { vecorValue.push_back( gd::variant( value_.as<std::string_view>() ) ); }
      else if( type_ == sol::type::boolean ) { vecorValue.push_back( value_.as<bool>() ); }
   }
}

void ConvertTo_g( const sol::table& tableValue, std::vector<gd::variant_view>& vecorValue )
{
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { vecorValue.push_back( value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { vecorValue.push_back( gd::variant_view( value_.as<std::string_view>() ) ); }
      else if( type_ == sol::type::boolean ) { vecorValue.push_back( value_.as<bool>() ); }
   }
}

gd::argument::arguments ConvertToArguments_g( const sol::table& tableValue )
{
   gd::argument::arguments argumentsValue;
   for( auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<std::string_view>()); }
   }

   return argumentsValue; 
}

void ConvertToArguments_g( const sol::table& tableValue, gd::argument::arguments& argumentsValue )
{
   for( const auto& it : tableValue )
   {
      const auto& value_ = it.second;
      sol::type type_ = value_.get_type();
      if( type_ == sol::type::number ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<int64_t>()); }
      else if( type_ == sol::type::string ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<std::string_view>()); }
      else if( type_ == sol::type::table ) 
      {
         const auto& name_ = it.first.as<std::string_view>();
         const auto& tableInner = value_.as<sol::table>();
         for( const auto& itInner : tableInner )
         {
            const auto& valueInner = itInner.second;
            sol::type iType = valueInner.get_type();
            if( iType == sol::type::number ) { argumentsValue.append( name_, valueInner.as<int64_t>()); }
            else if( iType == sol::type::string ) { argumentsValue.append( name_, valueInner.as<std::string_view>()); }
         }
      }
      else if( type_ == sol::type::boolean ) { argumentsValue.append( it.first.as<std::string_view>(), value_.as<bool>()); }
   }
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::variant& value_ )
{
   if( value_.is_integer() == true )      { return value_.as_int64(); }
   else if( value_.is_string() == true )  { return value_.as_string(); }
   else if( value_.is_decimal() == true ) { return value_.as_double(); }
   return sol::lua_nil;
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> ConvertToAny_g( const gd::variant_view& value_ )
{
   if( value_.is_integer() == true )      { return value_.as_int64(); }
   else if( value_.is_string() == true )  { return value_.as_string(); }
   else if( value_.is_decimal() == true ) { return value_.as_double(); }
   return sol::lua_nil;
}

gd::variant ConvertFromAny_g( const std::variant<int64_t, std::string, double, bool, sol::lua_nil_t>& any_ )
{
   auto uIndex = any_.index();
   if( uIndex == 0 ) return std::get<0>( any_ );
   else if( uIndex == 1 ) return std::get<1>( any_ );
   else if( uIndex == 2 ) return std::get<2>( any_ );
   else if( uIndex == 3 ) return std::get<3>( any_ );

   return gd::variant();
}

void ConvertFromAny_g( const std::variant<int64_t, std::string, double, bool, sol::lua_nil_t>& any_, gd::variant_view& variantviewValue )
{
   auto uIndex = any_.index();
   if( uIndex == 0 ) variantviewValue = std::get<0>( any_ );
   else if( uIndex == 1 ) variantviewValue = std::get<1>( any_ );
   else if( uIndex == 2 ) variantviewValue = std::get<2>( any_ );
   else if( uIndex == 3 ) variantviewValue = std::get<3>( any_ );
   else
   {
      variantviewValue.clear();
   }
}

// ----------------------------------------------------------------------------
// ---------------------------------------------------------------------- Table
// ----------------------------------------------------------------------------

namespace {
   /// add columns by parsing string to extract information about columns to adds
   void add_column_s(gd::table::dto::table*& ptable, const std::string_view& stringColumn)
   {
      auto result_ =  ptable->column_add( stringColumn, gd::table::tag_parse{});
      if(result_.first == false)
      {
         delete ptable;
         ptable = nullptr;
         std::string stringError("Invalid format: ");
         stringError += result_.second;
         throw sol::error( stringError );
      }

      result_ = ptable->prepare();
      if(result_.first == false)
      {
         delete ptable;
         ptable = nullptr;
         std::string stringError("Invalid format: ");
         stringError += result_.second;
         throw sol::error( stringError );
      }
   }
}

/// Construct table with full meta support and start with 10 number of allocated rows (grows dynamically)
/// Use parse logic in table to generate columns, read internal dto table docs to find how that works
Table::Table(const std::string_view& stringColumn )
{
   m_ptable = new gd::table::dto::table( 10, gd::table::tag_full_meta{});
   add_column_s( m_ptable, stringColumn );
}

/// Construct table with full meta support and start with specified number of allocated rows (grows dynamically)
/// Use parse logic in table to generate columns, read internal dto table docs to find how that works
Table::Table( uint64_t uRowCount, const std::string_view& stringColumn )
{
   m_ptable = new gd::table::dto::table( (unsigned)uRowCount, gd::table::tag_full_meta{});
   add_column_s( m_ptable, stringColumn );
}


Table::~Table() 
{ 
   delete m_ptable; 
}


// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- Database
// ----------------------------------------------------------------------------

Database::Database( sol::table tableOpen )
{
   m_pdatabase = nullptr;
   Open( tableOpen );
}

Database::~Database()
{
   if( m_pdatabase != nullptr && m_bOwner == true )
   {
      m_pdatabase->release();
   }
}


/** ----------------------------------------------------------------------------
 * @brief Open database (creates a database connection)
 * @param connect_ information needed to connect database
 */
void Database::Open( const std::variant<std::string_view, sol::table>& connect_ )
{
   Close();

   auto open_sqlite_( [] ( const std::string_view& stringOpen ) -> gd::database::database_i* {
      if( std::filesystem::exists( stringOpen ) == false ) throw sol::error( std::string( "Database file not found: ") + stringOpen.data() );

      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();  // create database interface
      auto [bOk, stringError] = pdatabase->open( stringOpen );                  // open database, for now this is hardcoded to open sqlite
      if( bOk == false )
      {
         pdatabase->release();
         throw sol::error( stringError );
      }
      return pdatabase;
   });

   if( connect_.index() == 0 )
   {
      std::string_view stringOpen = std::get<0>( connect_ );

      m_pdatabase = open_sqlite_( stringOpen );
      if( m_pdatabase != nullptr ) { m_bOwner = true; }

   }
   else
   {
      gd::argument::arguments argumentsOpen;
      ConvertToArguments_g( std::get<1>( connect_ ), argumentsOpen );

      if( argumentsOpen["file"].is_true() )
      {
         std::string stringOpen = argumentsOpen["file"].as_string();
         m_pdatabase = open_sqlite_( stringOpen );
         if( m_pdatabase != nullptr ) { m_bOwner = true; }
      }
   }
}

void Database::Execute( const std::string_view& stringSql )
{
   auto [bOk, stringError] = m_pdatabase->execute( stringSql );
   if( bOk == false ) { throw sol::error( stringError ); }
}

/** ---------------------------------------------------------------------------
 * @brief Ask for value in database, this method will only take first value found from query
 * @param stringSql sql string used to ask for value
 * @param type_ optional type, if return type is to converted to some specific type
 * @return std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> value from query
*/
std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Database::Ask( const std::string_view& stringSql, std::optional<std::string> type_ )
{
   gd::com::pointer<gd::database::cursor_i> pcursor;
   m_pdatabase->get_cursor( &pcursor );

   auto [bOk, stringError] = pcursor->open( stringSql );
   if( bOk == false ) { throw sol::error( stringError ); }

   gd::variant variantValue;

   if( pcursor->is_valid_row() )
   {
      variantValue = pcursor->get_record()->get_variant( 0 );

      if( type_.has_value() )
      {
         variantValue.convert( type_.value() );
      }
   }
   
   return ConvertToAny_g( variantValue );
}

/// Close connection if open
void Database::Close()
{
   if( m_pdatabase != nullptr && m_bOwner == true )
   {
      m_bOwner = false;
      m_pdatabase->release();
   }
   m_pdatabase = nullptr;
}


// ----------------------------------------------------------------------------
// --------------------------------------------------------------------- Cursor
// ----------------------------------------------------------------------------

void Cursor::Open( const std::string_view& stringSql, const sol::optional<sol::table>& params_ )
{
   if( params_.has_value() == false )
   {
      auto [bOk, stringError] = m_pcursor->open( stringSql );
      if( bOk == false ) { throw sol::error( stringError ); }
   }
   else
   {
      std::vector<gd::variant_view> vectorValue;
      ConvertTo_g( params_.value(), vectorValue );

      auto [ bOk, stringError ] = m_pcursor->prepare( stringSql, vectorValue );                    assert( bOk == true );
      if( bOk == false ) { throw sol::error( stringError ); }
      std::tie( bOk, stringError ) = m_pcursor->open();                                            assert( bOk == true );
      if( bOk == false ) { throw sol::error( stringError ); }
   }
}

void Cursor::Next()
{
   if( m_pcursor->is_open() == true )
   {
      m_pcursor->next();
   }
}

bool Cursor::IsValidRow() const
{
   if( m_pcursor->is_open() ) return m_pcursor->is_valid_row();
   return false;
}


void Cursor::Close()
{
   m_pcursor->close();
}

std::variant<int64_t, std::string, double, bool, sol::lua_nil_t> Cursor::GetValue( const std::variant<int64_t,std::string_view>& column_ )
{
   int iColumn;
   gd::variant_view value_;

   if( column_.index() == 1 ) { iColumn = m_pcursor->get_record()->get_column_index_for_name( std::get<1>( column_ ) ); }
   else                       { iColumn = (int)std::get<0>( column_ ); }

   value_ = m_pcursor->get_record()->get_variant_view( unsigned(iColumn) );

   return ConvertToAny_g( value_ );
}

/** ----------------------------------------------------------------------------
 * @brief Fill table with result from cursor and return it

*sample lua code*
~~~.c
function ReadCodes( iGroup )
   local database = Database.new( {file=app:GetProperty("database")} )
   local sSelectCode = "SELECT CodeK, FName FROM TCode WHERE GroupK = " .. iGroup

   local cursor = Cursor.new( database )
   cursor:Open( sSelectCode )

   local tableCode = cursor:GetTable()
   return tableCode
end

function main()
   local tableCode = ReadCodes( 4 );
   print( tableCode:Write( { format="cli" } ) )
end

main()
~~~
 * @return Table table with result from cursor
 */
Table Cursor::GetTable()
{
   std::unique_ptr< gd::table::table_column_buffer > ptable = std::unique_ptr< gd::table::table_column_buffer >( new gd::table::table_column_buffer( 10 ) );

   if( m_pcursor->is_open() == true )
   {
      assert(false);
      //application::database::TABLE_Append_g( ptable.get(), m_pcursor );
   }
   else
   {
      throw sol::error( "cursor isn't open, no statement to process" );
   }
   return Table( ptable.release() );
}




// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- Application
// ----------------------------------------------------------------------------

Application::Application() 
{ 
   m_papplication = papplication_g; 
}

Application::~Application()
{
   if( m_bOwner == true ) delete m_papplication;
}

Document Application::GetDocument()
{                                                                                                  assert( m_papplication != nullptr );
   return Document( (void*)m_papplication->GetDocument() );
}

/// return number of properties in application
uint64_t Application::GetPropertyCount()
{                                                                                                  assert( m_papplication != nullptr );
   return (uint64_t)m_papplication->PROPERTY_Size();
}

/// Read application property
std::variant<int64_t,std::string, double, bool> Application::GetProperty( const std::variant<int64_t,std::string>& index_ )
{                                                                                                  assert( m_papplication != nullptr );
   decltype( m_papplication->PROPERTY_Get("")) value_;

   if( index_.index() == 1 ) value_ = m_papplication->PROPERTY_Get( std::get<1>( index_ ) );
   else                      value_ = m_papplication->PROPERTY_Get( std::get<0>( index_ ) );

   if( value_.is_string() == true ) return value_.as_string();
   else if( value_.is_integer() == true ) return value_.as_int64();
   else if( value_.is_decimal() == true ) return value_.as_double();
   else { return value_.as_bool(); }
                                                                                                   assert( false );
   return false;
}

/// Return property name for property index
std::string Application::GetPropertyName( uint64_t uIndex ) const
{                                                                                                  assert( uIndex < m_papplication->PROPERTY_Size() );
   return m_papplication->PROPERTY_GetName( uIndex );
}

/// Set application property
void Application::SetProperty( const std::string_view& stringName, std::variant<int64_t,std::string, double> value_ )
{
   m_papplication->PROPERTY_Set( stringName, gd::to_variant_view_g( value_, gd::variant_type::tag_std_variant{}));
}

/// Set log level
void Application::SetLogLevel( const std::string_view& stringLevel )
{
   /*
   char chLevel = stringLevel[0];
   if( chLevel >= 'a' ) chLevel = chLevel - ( 'a' - 'A' );
   m_papplication->LOG_SetLevel_s( chLevel );
   */
}

/// Initialize application object.
/// May be used if you do not have any prepared application object 
void Application::Initialize( std::optional<std::string> connect_database_ )
{
   if( m_papplication == nullptr )
   {
      m_papplication = new CApplication();
      m_bOwner = true;
   }
}

/** ---------------------------------------------------------------------------
 * @brief prints message using application message logic
 * 
 * Sample lua code using the `Message` method
 @code
 function main()
 app:Message("=== Execute main")
 app:Message("ERROR", "--- This is not an error")
 app:Message("DEBUG", "--- Debug message")
 end

 main()
 @endcode
 * @param stringTypeOrMessage If only one parameter this is the message, more than one this is the type
 * @param message_ If type for message is set then this has the printed message
 */
void Application::Message(const std::string_view & stringTypeOrMessage, std::optional<std::string> message_)
{
   std::string stringType;
   std::string_view stringMessage;
   if( message_.has_value() )
   {
      stringMessage = message_.value();
      stringType = stringTypeOrMessage;
      //m_papplication->MESSAGE_Print( stringMessage, stringType );
   }
   else
   {
      //m_papplication->MESSAGE_Print( stringTypeOrMessage );
   }
}



LUA_END