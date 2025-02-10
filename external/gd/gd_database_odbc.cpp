#ifdef GD_DATABASE_ODBC_USE

#include "gd_utf8_2.h"
#include "gd_arguments.h"
#include "gd_database_odbc.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
#elif defined( _MSC_VER )
   #pragma warning( disable : 6387 ) 
#endif


_GD_DATABASE_ODBC_BEGIN

//#define GD_SIZEOF_SQLLEN sizeof( SQLLEN )


/** ---------------------------------------------------------------------------
 * @brief allocate resources needed to connect to database
 * @return std::pair<bool, std::string> true if ok, on fail return false and error information
*/
std::pair<bool, std::string> database::allocate()
{                                                                                                  assert( m_hDatabase == nullptr );
   if( m_hEnvironment == nullptr )
   {                                                                                               
      if( ::SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnvironment ) == SQL_ERROR )
      {                                                                                            assert( false );
         return { false, "Unable to allocate an environment handle\n" };
      }

      if( ::SQLSetEnvAttr( m_hEnvironment, SQL_ATTR_ODBC_VERSION, ( SQLPOINTER )SQL_OV_ODBC3, 0 ) == SQL_ERROR )
      {                                                                                            assert( false );
         return { false, "ODBC version 3.0 could not be set\n" };
      }
   }

   if( SQLAllocHandle(SQL_HANDLE_DBC, m_hEnvironment, &m_hDatabase ) == SQL_ERROR )
   {                                                                                               assert( false );
      return { false, "Unable to allocate database handle" };
   }

   return { true, std::string() };
}

/** ---------------------------------------------------------------------------
 * @brief set attribute value 
 * @param stringName attribute name
 * @param value_ value for attribute
 */
void database_i::set(const std::string_view& stringName, const gd::variant_view& value_)
{
   if(stringName == "dialect")
   {
      m_stringDialect = value_.as_string();
   }
   else
   {                                                                                               assert( false );
   }
}


/** ---------------------------------------------------------------------------
 * @brief open database using one single string, how string is formated depends on database that is opened
 * @param stringDriverConnect information in odbc connect format to open database
 * @return true if connected, false and error information on error
*/
std::pair<bool, std::string> database::open( const std::string_view& stringDriverConnect )
{
   if( m_hDatabase == nullptr )
   {
      auto result_ = allocate();
      if( result_.first == false ) return result_;
   }

   SQLRETURN iReturn;

   if(stringDriverConnect.find("UID") == std::string_view::npos &&
      stringDriverConnect.find("DSN") == std::string_view::npos)
   {
      auto vectorConnect = gd::utf8::split( stringDriverConnect, ";");
      std::string stringServer( std::move( vectorConnect[0] ));
      std::string stringUser( std::move( vectorConnect[1] ));
      std::string stringPassword( std::move( vectorConnect[2] ));
      iReturn = ::SQLConnect( m_hDatabase, (SQLCHAR*)stringServer.c_str(), (SQLSMALLINT)stringServer.length(), (SQLCHAR*)stringUser.c_str(), (SQLSMALLINT)stringUser.length(), (SQLCHAR*)stringPassword.c_str(), (SQLSMALLINT)stringPassword.length() );
      if(iReturn < 0)
      {
         auto stringError = error();
         return { false, stringError };
      }
   }
   else
   {
      iReturn = ::SQLDriverConnect( m_hDatabase, nullptr, (SQLCHAR*)stringDriverConnect.data(), (unsigned)stringDriverConnect.length(), nullptr, 0, nullptr, 0 );
      if( iReturn < 0 )
      {
         auto stringError = error();
         return { false, stringError };
      }
   }

   set_flags( (eDatabaseStateOwner|eDatabaseStateConnected), 0 );
   return { true, std::string() };
}

/** ---------------------------------------------------------------------------
 * @brief Connect to database using ODBC `SQLConnect` api
 * @param argumentsConnect connect information for database
*/
std::pair<bool, std::string> database::open( const gd::argument::arguments& argumentsConnect )
{
   if( m_hDatabase == nullptr )
   {
      auto result_ = allocate();
      if( result_.first == false ) return result_;
   }

   std::string stringServer = argumentsConnect["server"].as_string();
   std::string stringUser = argumentsConnect["user"].as_string();
   std::string stringPassword = argumentsConnect["password"].as_string();

   SQLRETURN iReturn;
   iReturn = ::SQLConnect( m_hDatabase, (SQLCHAR*)stringServer.c_str(), (SQLSMALLINT)stringServer.length(), (SQLCHAR*)stringUser.c_str(), (SQLSMALLINT)stringUser.length(), (SQLCHAR*)stringPassword.c_str(), (SQLSMALLINT)stringPassword.length() );

   if( iReturn < 0 )
   {
      auto stringError = error();
      return { false, stringError };
   }

   set_flags( (eDatabaseStateOwner|eDatabaseStateConnected), 0 );
   return { true, std::string() };
}

/** ---------------------------------------------------------------------------
 * @brief Check if connection to database is open
 * @return true if connection is open, false if not
*/
bool database::is_open()
{
   if( is_flag( eDatabaseStateConnected ) == true ) return true;

   if( m_hDatabase != nullptr )
   {
      SQLRETURN iReturn;
      SQLINTEGER iConnectionStatus = 0;
      SQLINTEGER iLength = 0;
      iReturn = ::SQLGetConnectAttr( m_hDatabase, SQL_ATTR_CONNECTION_DEAD, &iConnectionStatus, 0, &iLength ); assert( iReturn < 0 );
      if( iConnectionStatus == SQL_CD_FALSE ) return true;
   }
   
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief Execute SQL statement
 * @param stringStatement sql statement to execute
 * @return true if ok, false and error information on error
*/
std::pair<bool, std::string> database::execute( const std::string_view& stringStatement )
{                                                                                                  assert( is_open() );
   SQLRETURN iReturn;
   SQLHANDLE hStatement = nullptr;

   if( SQLAllocHandle(SQL_HANDLE_STMT, m_hDatabase, &hStatement ) == SQL_ERROR ) { assert( false ); return { false, "Unable to allocate statement handle" }; }

   iReturn = ::SQLExecDirect( hStatement, (SQLCHAR*)stringStatement.data(), (SQLINTEGER)stringStatement.length() );
   if( !SQL_SUCCEEDED(iReturn) )
   {
      auto stringError = error( hStatement );
      ::SQLFreeHandle( SQL_HANDLE_STMT, hStatement );
      return { false, stringError };
   }

   if( hStatement != nullptr ) ::SQLFreeHandle( SQL_HANDLE_STMT, hStatement );

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Ask for single value from sql statement
 * @param stringStatement string to execute that should returan at least one value
 * @param pvariantValue pointer to variant that gets value from statement
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> database::ask( const std::string_view& stringStatement, gd::variant* pvariantValue )
{                                                                                                  assert( is_open() );
   cursor cursorAsk( this );
   auto [ bOk, stringError ] = cursorAsk.prepare( stringStatement );                               assert( bOk == true );
   if( bOk == false ) return { false, stringError };

   auto result_ = cursorAsk.open();
   if( result_.first == false ) return result_;

   if( cursorAsk.is_valid_row() && pvariantValue != nullptr )
   {
      *pvariantValue = cursorAsk.get_record()->get_variant( 0 );
   }

   return { true, "" };

}

/** ---------------------------------------------------------------------------
 * @brief close database connection
*/
void database::close()
{
   if( is_owner() == true )
   {
      if( m_hDatabase != nullptr && is_open() == true )
      {
         ::SQLDisconnect( m_hDatabase );
         m_hDatabase = nullptr;
      }
   }
   else
   {
      m_hDatabase = nullptr;
   }

   set_flags( 0, eDatabaseStateConnected );                                // clear connection flag
}

/** ---------------------------------------------------------------------------
 * @brief pick up latest error from odbc database connection
 * @param stringError error string error is read to
 * @return true if error was read, false if not
*/
bool database::error( std::string& stringError )
{                                                                                                  assert( m_hEnvironment != nullptr );
   SQLRETURN iReturn;
   SQLINTEGER iErrorCode;
   char pbszState[10];
   char pbszBuffer[512];
   SQLSMALLINT iLength;
   iReturn = ::SQLError(m_hEnvironment, m_hDatabase, nullptr, (SQLCHAR*)pbszState, &iErrorCode, (SQLCHAR*)pbszBuffer, 512, &iLength);
   if( iReturn >= 0 )
   {
      stringError += "[";
      stringError += pbszBuffer;
      stringError += "] - [";
      stringError += pbszState;
      stringError += "]";

      return true;
   }

   return false;
}

bool database::error( SQLHANDLE hStatement, std::string& stringError )
{                                                                                                  assert( is_open() ); assert( hStatement != nullptr );
   SQLRETURN iReturn;
   SQLINTEGER iErrorCode;
   char pbszState[10];
   char pbszBuffer[512];
   SQLSMALLINT iLength;
   iReturn = ::SQLError(m_hEnvironment, m_hDatabase, hStatement, (SQLCHAR*)pbszState, &iErrorCode, (SQLCHAR*)pbszBuffer, 512, &iLength);
   if( SQL_SUCCEEDED( iReturn ) )
   {
      stringError += "[";
      stringError += pbszBuffer;
      stringError += "] - [";
      stringError += pbszState;
      stringError += "]";

      return true;
   }

   return false;
}

void database::deallocate()
{
   if( is_owner() == true ) return;

   if( is_open() == true )
   {
      close();
   }

   if( m_hDatabase != nullptr )
   {
      ::SQLFreeHandle( SQL_HANDLE_DBC, m_hDatabase );
      m_hDatabase = nullptr;
      set_flags( 0, eDatabaseStateOwner );
   }

   if( m_hEnvironment != nullptr )
   {
      ::SQLFreeHandle( SQL_HANDLE_ENV, m_hEnvironment );
      m_hEnvironment = nullptr;
   }
}

/// is statement handle is valid
bool database::is_statement_handle_open_s( SQLHANDLE hStatement )
{                                                                                                  assert( hStatement != nullptr );
   SQLSMALLINT iCursorType;
   SQLRETURN iReturn = ::SQLGetStmtAttr( hStatement, SQL_ATTR_CURSOR_TYPE, &iCursorType, sizeof(iCursorType), nullptr );
   if( SQL_SUCCEEDED( iReturn ) ) return true;

   return false;
}

/** ---------------------------------------------------------------------------
 * @brief Prepare sql into statement
 * @param stringSql sql command to be prepared
 * @return true if ok, false and error information if fail
*/
std::pair<bool, std::string> cursor::prepare( const std::string_view& stringSql )
{
   close();                                                                    // close statement if there is one active open statement

   SQLHANDLE hStatement; // statement handle for query that is used to work with result
   SQLRETURN iReturn;    // return value from odbc calls

   // ## allocate statement handle
   iReturn = ::SQLAllocHandle( SQL_HANDLE_STMT, *m_pdatabase, &hStatement );
   if( iReturn != SQL_SUCCESS ) return { false, m_pdatabase->error() };

   m_hStatement = hStatement;                                                  // assign handle to member
   iReturn = ::SQLPrepare( hStatement, (SQLCHAR*)stringSql.data(), (SQLINTEGER)stringSql.length());
   if( !SQL_SUCCEEDED( iReturn ) ) { return { false, m_pdatabase->error( hStatement ) }; }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Prepare sql into statement
 * @param stringSql sql command to be prepared
 * @param vectorValue bind these values to parameters in query, same order as parameters in query
 * @return true if ok, false and error information if fail
*/
std::pair<bool, std::string> cursor::prepare( const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue )
{
   auto result_ = prepare( stringSql );
   if( std::get<0>( result_ ) == true )
   {
      int iIndex = 1;
      for( auto it : vectorValue )
      {
         result_ = bind_parameter( iIndex, it );
         if( result_.first == false ) return result_;

         iIndex++;
      }
   }
   else
   {
      return result_;
   }

   return { true, "" };
}

std::pair<bool, std::string> cursor::open()
{                                                                                                  assert( m_hStatement != nullptr );
   SQLRETURN iReturn = ::SQLExecute( m_hStatement );
   if( SQL_SUCCEEDED( iReturn ) == true ) return { true, std::string() };

   return { false, m_pdatabase->error( m_hStatement ) };
}

/** ---------------------------------------------------------------------------
 * @brief Opens result set from select string
 * @code
gd::database::odbc::cursor cursorUser( &database );
auto result_ = cursorUser.open( "SELECT * FROM T_TestColumnTypes" );
if( result_.first == false ) return result_;
std::string stringResult;
while( cursorUser.is_valid_row() == true )
{ 
   stringResult += cursorUser[0].as_string();
   for( auto u = 1; u < cursorUser.get_column_count(); u++ )
   {
      stringResult += ", ";
      auto stringValue = cursorUser[u].as_string();
      stringResult += stringValue;
   }
   stringResult += "\n";

   cursorUser.next();
}
 * @endcode
 * @param stringSql select string to execute against database that opens cursor
 * @return true if ok, false and error information on error
*/
std::pair<bool, std::string> cursor::open(const std::string_view& stringSql)
{                                                                                                  assert( m_pdatabase != nullptr );
   close();                                                                    // close statement if there is one active open statement
   SQLHANDLE hStatement; // statement handle for query that is used to work with result
   SQLRETURN iReturn;    // return value from odbc calls

   // ## allocate statement handle
   iReturn = ::SQLAllocHandle( SQL_HANDLE_STMT, *m_pdatabase, &hStatement );
   if( iReturn != SQL_SUCCESS ) return { false, m_pdatabase->error() };

   m_hStatement = hStatement;                                                  // assign handle to member

   // ## Set cursor type

   SQLPOINTER pCursorType = SQL_CURSOR_FORWARD_ONLY;
   iReturn = ::SQLSetStmtAttr( hStatement, SQL_CURSOR_TYPE, pCursorType, 0 );
   if( !SQL_SUCCEEDED( iReturn ) ) { return { false, m_pdatabase->error( hStatement ) }; }

   // ## Execute query
   iReturn = ::SQLExecDirect( hStatement, (SQLCHAR*)stringSql.data(), (SQLINTEGER)stringSql.length() );
   if( !SQL_SUCCEEDED( iReturn ) ) { return { false, m_pdatabase->error( hStatement ) }; }

   SQLSMALLINT iColumnCount;
   iReturn = ::SQLNumResultCols( hStatement, &iColumnCount );                                      assert( iColumnCount > 0 );
   if( !SQL_SUCCEEDED( iReturn ) ) { assert( false ); return { false, m_pdatabase->error( hStatement ) }; }


   auto result_ = add_columns( hStatement, m_recordRow, iColumnCount );
   if( result_.first == false ) return result_;
   result_ = bind();
   if( result_.first == false ) return result_;

   bool bIsOnRow = false;
   result_ = next( &bIsOnRow, 0 );

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief binds value to parameter in sql query
 * @param iIndex parameter index to bind to
 * @param VVValue value parameter gets
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> cursor::bind_parameter( int iIndex, const gd::variant_view& VVValue )
{                                                                                                  assert( m_hStatement != nullptr ); assert( iIndex > 0 );
   SQLULEN uLength = VVValue.length();
   SQLSMALLINT iType = (SQLSMALLINT)get_column_type_s( (gd::types::enumTypeNumber)VVValue.type_number() );
   SQLSMALLINT iCType = (SQLSMALLINT)get_column_ctype_s( (gd::types::enumTypeNumber)VVValue.type_number() );
   SQLRETURN iReturn = ::SQLBindParameter( m_hStatement, iIndex, SQL_PARAM_INPUT, iCType, iType, uLength, 0, (SQLPOINTER)VVValue.get_value_buffer(), 0, nullptr );
   if( SQL_SUCCEEDED( iReturn ) == true ) return { true, std::string() };

   return { false, m_pdatabase->error( m_hStatement ) };
}

/** ---------------------------------------------------------------------------
 * @brief binds value to parameter in sql query
 * @param iIndex parameter index to bind to
 * @param VValue value parameter gets
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> cursor::bind_parameter( int iIndex, const gd::variant& VValue, tag_variant )
{                                                                                                  assert( m_hStatement != nullptr ); assert( iIndex > 0 );
   SQLULEN uLength = VValue.length();
   SQLSMALLINT iType = (SQLSMALLINT)get_column_type_s( (gd::types::enumTypeNumber)VValue.type_number() );
   SQLSMALLINT iCType = (SQLSMALLINT)get_column_ctype_s( (gd::types::enumTypeNumber)VValue.type_number() );
   SQLRETURN iReturn = ::SQLBindParameter( m_hStatement, iIndex, SQL_PARAM_INPUT, iCType, iType, uLength, 0, (SQLPOINTER)VValue.get_value_buffer(), 0, nullptr );
   if( SQL_SUCCEEDED( iReturn ) == true ) return { true, std::string() };

   return { false, m_pdatabase->error( m_hStatement ) };
}


std::pair<bool, std::string> cursor::add_columns( SQLHANDLE hStatement, record& recordAddTo, unsigned uCount )
{
   SQLSMALLINT iFieldNameLength;       // Gets field name length 
   SQLSMALLINT iNullable;              // if field is nullable
   SQLULEN     uFieldSize;             // Length of field if value is stored in array like text or binary
   SQLCHAR     pszFieldName[256];      // name of field


   for( unsigned u = 0; u < uCount; u++ )
   {
      
      SQLSMALLINT iColumnNameLength, iSqlType, iDecimalDigits, iNullable;
      
      uFieldSize = 0;
      SQLRETURN iReturn = ::SQLDescribeCol( hStatement, (SQLUSMALLINT) (u + 1), pszFieldName, sizeof(pszFieldName ),  &iFieldNameLength, &iSqlType, &uFieldSize, &iDecimalDigits, &iNullable );
      if( SQL_SUCCEEDED( iReturn ) )
      {
         unsigned uState = 0;
         unsigned uStartBufferSize = 0;                                        // Initial buffer size to store values, this can be increased if field data is larger
         unsigned uType = get_column_type_s( iSqlType );
#ifndef NDEBUG
         auto stringType_d = gd::types::type_name_g( uType );                                      assert( stringType_d.empty() == false );
#endif

         unsigned uSize = value_size_g( uType );                               // get size for type, or zero or number of max type values if buffer
         
         // ## if size is 0 it means that this is not a primitive type and buffer may grow
         if( uSize == 0 )
         {
            // ### check size and if size is larger then add checks to increase buffer when values are fetched

            if( gd::database::value_group_type_g( uType ) == eColumnTypeGroupString ) 
            { 
               uSize = 0; 
               uStartBufferSize = (uFieldSize > 0 && uFieldSize < 256 ? uFieldSize : 128); 
            }
            else if( gd::database::value_group_type_g( uType ) == eColumnTypeGroupBinary ) 
            { 
               uSize = 0;
               uStartBufferSize = (uFieldSize > 0 && uFieldSize < 256 ? uFieldSize : 32); 
            }

            if( uStartBufferSize != uFieldSize ) 
            { 
               m_uState |= eCursorStateMemory;                                 // mark that cursor contain fields that have blob value
               uState |= eColumnValueStateBlob;                                // state for column that it is a blob value
            }
         }

         unsigned uCType = get_column_ctype_s( iSqlType );

         recordAddTo.add( uType, uCType, uSize, uStartBufferSize, (const char*)pszFieldName, "", uState );// binds column buffer in result
#ifndef NDEBUG
         const auto* pcolumn_d = recordAddTo.get_column( (unsigned)recordAddTo.size() - 1 );
         std::string stringColumn = gd::database::debug::print( *pcolumn_d );
#endif // !NDEBUG

      }
      else
      {

      }

      /*
      fields::column* pcolumn = m_fields.initialize_column( i, szColumnName, convert_type( iSqlType ) );
      column_extra* pColumnExtra = get_extra_data( pcolumn );
      pColumnExtra->iSqlCType = get_sql_c_type( pcolumn->m_eColumnType );
      */
   }

   // bind buffers

   return { true, "" };
}


std::pair<bool, std::string> cursor::bind()
{
   SQLRETURN iReturn;
   for( const auto& itField : m_recordRow )
   {
      unsigned uIndex = (unsigned)itField.index();
      SQLSMALLINT iSqlType = (SQLSMALLINT)itField.ctype();                                         assert( iSqlType != 0 ); // no sql type for zero
      SQLLEN uBufferLength = itField.size_buffer();
      void* pValueBuffer = m_recordRow.buffer_get( uIndex );

#if defined(_WIN64) || (SIZEOF_LONG_INT == 8)
      {
         auto* piSize = itField.size_pointer();
         iReturn = ::SQLBindCol( m_hStatement, uIndex + 1, iSqlType, pValueBuffer, uBufferLength, piSize);
      }
#else
      {
         auto* piSize = itField.size_pointer32();
         iReturn = ::SQLBindCol( m_hStatement, uIndex + 1, iSqlType, pValueBuffer, uBufferLength, piSize);
      }
#endif

      if( !SQL_SUCCEEDED( iReturn ) ) return { false, m_pdatabase->error( m_hStatement ) };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Execute a prepared statement, useful for queries with parameters
 * @return true if ok, false and error information if not
*/
std::pair<bool, std::string> cursor::execute()
{                                                                             assert( m_hStatement != nullptr );
   SQLRETURN iReturn;    // return value from odbc calls

   // ## Execute query
   iReturn = ::SQLExecute( m_hStatement );
   if( !SQL_SUCCEEDED( iReturn ) ) { return { false, m_pdatabase->error( m_hStatement ) }; }

   SQLSMALLINT iColumnCount;
   iReturn = ::SQLNumResultCols( m_hStatement, &iColumnCount );                                      assert( iColumnCount > 0 );
   if( !SQL_SUCCEEDED( iReturn ) ) { assert( false ); return { false, m_pdatabase->error( m_hStatement ) }; }


   auto result_ = add_columns( m_hStatement, m_recordRow, iColumnCount );
   if( result_.first == false ) return result_;
   result_ = bind();
   if( result_.first == false ) return result_;

   bool bIsOnRow = false;
   result_ = next( &bIsOnRow, 0 );

   return { true, "" };
}



/** -----------------------------------------------------------------------------------------------
 * @brief 
 * @param pbIsOnRow 
 * @param iCount 
 * @return 
*/
std::pair<bool, std::string> cursor::next( bool* pbIsOnRow, int iCount )
{                                                                                                  assert( m_hStatement != nullptr );
   SQLRETURN iReturn = ::SQLFetch( m_hStatement );
   if( iReturn == SQL_NO_DATA )
   {
      m_uState &= ~eCursorStateRow; 
      if( pbIsOnRow != NULL ) *pbIsOnRow = false;
      return { true, "" };
   }

   if( !SQL_SUCCEEDED(iReturn) )
   {  
      m_uState &= ~eCursorStateRow; 
      if( pbIsOnRow != NULL ) *pbIsOnRow = false;
      return { false, m_pdatabase->error( m_hStatement ) };
   }

   m_uState |= eCursorStateRow; 

   if( m_uState & eCursorStateMemory )
   {
      update_blob();
   }

   /*
   if( iCount < 0 ) iCount = m_recordRow.get_column_count();

   for( it = std::begin( m_recordRow ), itEnd = it + iCount; it != itEnd; it++ )
   {
      if( it->is_null_size() != SQL_NULL_DATA )
      {
         if( it->is_fixed() == true )
         {

         }

      }
   }
   */

   /*
   for( int iColumnIndex = 0; iColumnIndex < iUpdateTo; iColumnIndex++ )
   {
      m_recordRow.get_column(  )


      fields::column* pcolumn = m_fields.get_column( iColumnIndex );
      column_extra* pColumnExtra = get_extra_data( pcolumn );

      if( pColumnExtra->iLenOrInd == SQL_NULL_DATA ) pcolumn->set_null_value();
      else
      {
         pcolumn->clear_null_value();

         // Update variable length data size only
         if( pcolumn->is_fixed_size() == false )
         {
            if( (size_t)pColumnExtra->iLenOrInd >= pcolumn->m_uMaxDataSizeInBytes )
            {
               pcolumn->m_uDataSizeInBytes = pcolumn->m_uMaxDataSizeInBytes;

               // Don't count the null terminator if it's a string
               if( pcolumn->m_eColumnType & eColumnTypeGroupString )
               {
                  pcolumn->m_uDataSizeInBytes -= pcolumn->m_eColumnType == eColumnTypeCompleteWString ? sizeof(wchar_t) : sizeof(char);
               }
            }
            else pcolumn->m_uDataSizeInBytes = pColumnExtra->iLenOrInd;
         }
      }
   }
   */

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief if result has blob data or long text values then use this to check if buffer storing data needs to grow
 * @return true if ok, false and error information on fail
*/
std::pair<bool, std::string> cursor::update_blob()
{
   SQLRETURN iReturn;

   for( auto it = std::begin( m_recordRow ), itEnd = std::end( m_recordRow ); it != itEnd; it++ )
   {
#ifndef NDEBUG
      const auto* pcolumn_d = &(*it);
      auto blob_d_ = it->is_blob();
      auto size_d_ = it->isize();
      auto size_buffer_d_ = it->size_buffer();
#endif // !NDEBUG

      // check if data size loaded is same as buffer size, that means that it could be more data to load
      if( it->is_blob() == true && it->isize() >= (int64_t)it->isize_buffer() )
      {
         unsigned uBufferIndex = it->value();
         unsigned uNewSize = it->size();
         const uint8_t* puSizeAndBuffer = m_recordRow.resize( uBufferIndex, uNewSize );
         unsigned uNewBufferSize = buffers::buffer_size_s( puSizeAndBuffer );  // get allocated buffer size
         it->size_buffer( uNewBufferSize );                                    // set size for new allocated buffer

         uint8_t* pbBuffer = m_recordRow.buffer_get( it->index() );
#ifndef NDEBUG
         const char* pbszView_d = (const char*)pbBuffer;
         uint64_t uValueSize_d = it->size();
#endif // !NDEBUG

         auto uBufferSize = it->size_buffer();
         SQLUSMALLINT uIndex = (SQLUSMALLINT)(it->index() + 1);

#if defined(_WIN64) || (SIZEOF_LONG_INT == 8)
         int64_t* piSize = it->size_pointer();
#else
         int32_t* piSize = it->size_pointer32();
#endif 


         iReturn = ::SQLGetData( m_hStatement, uIndex, (SQLSMALLINT)it->ctype(), pbBuffer, uBufferSize, piSize );
         if( !SQL_SUCCEEDED( iReturn ) ) return { false, m_pdatabase->error( m_hStatement ) };

      }
   }

   return { true, "" };
}




/** ---------------------------------------------------------------------------
 * @brief Update column buffers
 * @param uFrom start on column to update buffer
 * @param uTo end before this column updating buffers
*/
void cursor::update( unsigned uFrom, unsigned uTo )
{                                                                              assert( m_hStatement != nullptr );
   enumColumnTypeComplete eType; // column type
   uint8_t* pbBuffer;            // pointer to active field in sqlite stmt (statement)
   int iValueSize = 0;           // gets value size for field that do not have a max limit for size

   for( auto u = uFrom; u < uTo; u++ )
   {
      /*
      int iType = ::sqlite3_column_type( m_pstmt, u );
      gd::database::record::column* pcolumn = m_recordRow.get_column( u );
      if (iType != SQLITE_NULL)
      {
      }
      else
      {
         pcolumn->set_null(true);
      }
      */
   }
}




/** -----------------------------------------------------------------------------------------------
 * @brief Collects values from active row and place them in returned vector with `variant`
 * @return std::vector<gd::variant> get all values from current row
*/
std::vector<gd::variant> cursor::get_variant() const
{
   std::vector<gd::variant> vectorValue;
   for( auto u = 0u, uTo = get_column_count(); u < uTo; u++ )
   { 
      vectorValue.push_back( get_variant( u ) );
   }
   return vectorValue;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in specified column
 * @param uColumnIndex Index to column where retured value i value
 * @return gd::variant value is placed and returned in variant
*/
gd::variant cursor::get_variant( unsigned uColumnIndex ) const
{                                                                             assert( uColumnIndex < get_column_count() );
   const gd::database::record::column* pcolumn = m_recordRow.get_column( uColumnIndex );

   if (pcolumn->is_null() == false)
   {
      unsigned uType = pcolumn->type();
      uint8_t* pbBuffer = m_recordRow.buffer_get( uColumnIndex );              // get buffer to column
      switch( uType )
      {
      case eColumnTypeCompleteInt16: return gd::variant( *(int16_t*)pbBuffer );
      case eColumnTypeCompleteInt32: return gd::variant( (int32_t)*(int64_t*)pbBuffer);
      case eColumnTypeCompleteInt64: return gd::variant( *(int64_t*)pbBuffer );
      case eColumnTypeCompleteCFloat: return gd::variant( *(float*)pbBuffer );
      case eColumnTypeCompleteCDouble: return gd::variant( *(double*)pbBuffer );
      case eColumnTypeCompleteDecimal: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteString: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteUtf8String: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteBinary: return gd::variant( (unsigned char*)pbBuffer, (size_t)pcolumn->size() );
         
      default: assert(false);
      }
   }

   return gd::variant();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Collects values from active row and place them in returned vector with `variant_view`
 * @return std::vector<gd::variant_view> get all values from current row
*/
std::vector<gd::variant_view> cursor::get_variant_view() const
{
   std::vector<gd::variant_view> vectorValue;
   vectorValue.reserve( get_column_count() );
   for( auto u = 0u, uTo = get_column_count(); u < uTo; u++ )
   { 
      vectorValue.push_back( get_variant_view( u ) );
   }
   return vectorValue;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in specified column
 * @param uColumnIndex Index to column where retured value i value
 * @return gd::variant_view value is placed and returned in variant_view
*/
gd::variant_view cursor::get_variant_view( unsigned uColumnIndex ) const
{                                                                             assert( uColumnIndex < get_column_count() );
   const gd::database::record::column* pcolumn = m_recordRow.get_column( uColumnIndex );

   if (pcolumn->is_null() == false)
   {
      unsigned uType = pcolumn->type();
      uint8_t* pbBuffer = m_recordRow.buffer_get( uColumnIndex );              // get buffer to column
      switch( uType )
      {
      case eColumnTypeCompleteUnknown: return gd::variant_view();
      case eColumnTypeCompleteInt16: return gd::variant_view( *(int16_t*)pbBuffer);
      case eColumnTypeCompleteInt32: return gd::variant_view( *(int32_t*)pbBuffer);
      case eColumnTypeCompleteInt64: return gd::variant_view( *(int64_t*)pbBuffer );
      case eColumnTypeCompleteCFloat: return gd::variant_view( *(float*)pbBuffer );
      case eColumnTypeCompleteCDouble: return gd::variant_view( *(double*)pbBuffer );
      case eColumnTypeCompleteDecimal: return gd::variant_view( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteString: return gd::variant_view( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteUtf8String: return gd::variant_view( gd::variant_type::utf8( (const char*)pbBuffer, (size_t)pcolumn->size()) );
      case eColumnTypeCompleteBinary: return gd::variant_view( (unsigned char*)pbBuffer, (size_t)pcolumn->size() );
         
      default: assert(false);
      }
   }

   return gd::variant_view();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in named column
 * @param stringName column name for returned value
 * @return gd::variant_view value is placed and returned in variant_view
*/
gd::variant_view cursor::get_variant_view( const std::string_view& stringName ) const
{
   int iColumnIndex = get_index( stringName );
   if( iColumnIndex != -1 ) return get_variant_view( iColumnIndex );

   return gd::variant_view();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Collects values from active row from specified columns and place them in returned vector with `variant_view`
 * @param vectorIndex column index where values are read and inserted to vector
 * @return std::vector<gd::variant_view> vector with values
*/
std::vector<gd::variant_view> cursor::get_variant_view( const std::vector<unsigned>& vectorIndex ) const
{
   std::vector<gd::variant_view> vectorValue;
   vectorValue.reserve( vectorIndex.size() );
   for( auto it : vectorIndex )
   {                                                                                               assert( it < get_column_count() );
      vectorValue.push_back( get_variant_view( it ) );
   }
   return vectorValue;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Return row values in arguments object
 * @return gd::argument::arguments arguments with values from current row
*/
gd::argument::arguments cursor::get_arguments() const
{
   gd::argument::arguments argumentsRow;

   const gd::database::record& record = *this;
   for( unsigned uColumn = 0, uColumnCount = get_column_count(); uColumn < uColumnCount; uColumn++ )
   {
      auto stringName = record.name_get( uColumn );
      gd::variant_view variantviewValue = (*this)[uColumn];
      argumentsRow.append_argument( stringName, variantviewValue );
   }

   return argumentsRow;
}

/** ---------------------------------------------------------------------------
 * @brief return index to column based field name
 * @param stringName name column index is returned for
 * @return int index to column if found, otherwise -1
*/
int cursor::get_index( const std::string_view& stringName ) const
{
   return m_recordRow.get_column_index_for_name( stringName );
}

/// return internal gd type for odbc value type
unsigned cursor::get_column_type_s( int iSqlType )
{
   switch( iSqlType )
   {
   case SQL_CHAR:             return eColumnTypeCompleteString;
   case SQL_VARCHAR:          return eColumnTypeCompleteString;
   case SQL_LONGVARCHAR:      return eColumnTypeCompleteString;
   case SQL_WCHAR:            return eColumnTypeCompleteWString;
   case SQL_WVARCHAR:         return eColumnTypeCompleteWString;
   case SQL_WLONGVARCHAR:     return eColumnTypeCompleteWString;
   case SQL_DECIMAL:          return eColumnTypeCompleteDecimal;
   case SQL_NUMERIC:          return eColumnTypeCompleteNumeric;   
   case SQL_SMALLINT:         return eColumnTypeCompleteInt16;
   case SQL_INTEGER:          return eColumnTypeCompleteInt32;
   case SQL_REAL:             return eColumnTypeCompleteCFloat;
   case SQL_FLOAT:            return eColumnTypeCompleteCDouble;
   case SQL_DOUBLE:           return eColumnTypeCompleteCDouble;
   case SQL_BIT:              return eColumnTypeCompleteBit;
   case SQL_TINYINT:          return eColumnTypeCompleteInt8;
   case SQL_BIGINT:           return eColumnTypeCompleteInt64;
   case SQL_BINARY:           return eColumnTypeCompleteBinary;
   case SQL_VARBINARY:        return eColumnTypeCompleteBinary;
   case SQL_LONGVARBINARY:    return eColumnTypeCompleteBinary;
   case SQL_DATE:             return eColumnTypeCompleteString;  
   case SQL_TIME:             return eColumnTypeCompleteString;  
   case SQL_TIMESTAMP:        return eColumnTypeCompleteString;  
   case SQL_TYPE_DATE:        return eColumnTypeCompleteString;   //TODO: Fix type
   case SQL_TYPE_TIME:        return eColumnTypeCompleteString;   //TODO: Fix type
   case SQL_TYPE_TIMESTAMP:   return eColumnTypeCompleteString;   //TODO: Fix type
   case SQL_GUID:             return eColumnTypeCompleteGuid;
   default:                   return eColumnTypeCompleteString;
   }
                                                                                                   assert( false );
   return 0;
}

/// get c type for odbc database type value
unsigned cursor::get_column_ctype_s( int iSqlType )
{
   switch( iSqlType )
   {
   case SQL_CHAR:             return SQL_C_CHAR;
   case SQL_VARCHAR:          return SQL_C_CHAR;
   case SQL_LONGVARCHAR:      return SQL_C_CHAR;
   case SQL_WCHAR:            return SQL_C_WCHAR;
   case SQL_WVARCHAR:         return SQL_C_WCHAR;
   case SQL_DECIMAL:          return SQL_C_CHAR;
   case SQL_NUMERIC:          return SQL_C_NUMERIC;   
   case SQL_SMALLINT:         return SQL_C_SHORT;
   case SQL_INTEGER:          return SQL_C_LONG;
   case SQL_REAL:             return SQL_C_FLOAT;
   case SQL_FLOAT:            return SQL_C_FLOAT;
   case SQL_DOUBLE:           return SQL_C_DOUBLE;
   case SQL_BIT:              return SQL_C_BIT;
   case SQL_TINYINT:          return SQL_C_TINYINT;
   case SQL_BIGINT:           return (unsigned)SQL_C_SBIGINT;
   case SQL_BINARY:           return SQL_C_BINARY;
   case SQL_VARBINARY:        return SQL_C_BINARY;
   case SQL_LONGVARBINARY:    return SQL_C_BINARY;
   case SQL_DATE:             return SQL_C_DATE;  
   case SQL_TIME:             return SQL_C_TIME;  
   case SQL_TIMESTAMP:        return SQL_C_TIMESTAMP; 
   case SQL_TYPE_DATE:        return SQL_C_CHAR;
   case SQL_TYPE_TIME:        return SQL_C_CHAR;
   case SQL_TYPE_TIMESTAMP:   return SQL_C_CHAR;
   //case SQL_GUID:             return eColumnTypeCompleteGuid;
   // default:                   return eColumnTypeCompleteString;
   }
                                                                                                   assert( false );
   return 0;
}

int cursor::get_column_type_s( gd::types::enumTypeNumber eType )
{
   using namespace gd::types;

   switch( eType )
   {
   case eTypeNumberInt8: return SQL_TINYINT;
   case eTypeNumberInt16: return SQL_SMALLINT;
   case eTypeNumberInt32: return SQL_INTEGER;
   case eTypeNumberUInt32: return SQL_INTEGER;
   case eTypeNumberInt64: return SQL_BIGINT;
   case eTypeNumberUInt64: return SQL_BIGINT;
   case eTypeNumberFloat: return SQL_FLOAT;
   case eTypeNumberDouble: return SQL_DOUBLE;
   case eTypeNumberGuid: return SQL_GUID;
   case eTypeNumberString: return SQL_VARCHAR;
   case eTypeNumberUtf8String: return SQL_VARCHAR;
   case eTypeNumberBinary: return SQL_BINARY;
   default: assert( false );
   }

   return SQL_UNKNOWN_TYPE;
}


int cursor::get_column_ctype_s( gd::types::enumTypeNumber eType )
{
   using namespace gd::types;

   switch( eType )
   {
   case eTypeNumberInt8: return SQL_C_TINYINT;
   case eTypeNumberInt16: return SQL_C_SHORT;
   case eTypeNumberInt32: return SQL_C_LONG;
   case eTypeNumberUInt32: return SQL_C_ULONG;
   case eTypeNumberInt64: return SQL_C_SBIGINT;
   case eTypeNumberUInt64: return SQL_C_UBIGINT;
   case eTypeNumberFloat: return SQL_C_FLOAT;
   case eTypeNumberDouble: return SQL_C_DOUBLE;
   case eTypeNumberGuid: return SQL_C_GUID;
   case eTypeNumberString: return SQL_C_CHAR;
   case eTypeNumberUtf8String: return SQL_C_CHAR;
   case eTypeNumberBinary: return SQL_C_BINARY;
   default: assert( false );
   }

   return SQL_UNKNOWN_TYPE;
}

/// return name for odbc c type for value
std::string_view cursor::get_column_ctype_name_s( int iCType )
{
   switch( iCType )
   {
   case SQL_C_BIT: return "bit";
   case SQL_C_SHORT: return "short";
   case SQL_C_TINYINT: return "tinyint";
   case SQL_C_LONG: return "long";
   case SQL_C_SLONG: return "long";
   case SQL_C_ULONG: return "ulong";
   case SQL_C_SBIGINT: return "bigint";
   case SQL_C_UBIGINT: return "ubigint";
   case SQL_C_FLOAT: return "float";
   case SQL_C_DOUBLE: return "double";
   case SQL_C_CHAR: return "char";
   case SQL_C_WCHAR: return "wchar";
   case SQL_C_NUMERIC: return "numeric";
   case SQL_C_BINARY: return "binary";
   case SQL_C_DATE: return "date";
   case SQL_C_TIME: return "time";
   case SQL_C_TIMESTAMP: return "timestamp";
   // case SQL_C_GUID: "guid";
   default: assert(false);
   }

   return "";
}



_GD_DATABASE_ODBC_END

_GD_DATABASE_ODBC_BEGIN

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- cursor_i
// ----------------------------------------------------------------------------

int32_t cursor_i::query_interface( const com::guid& guidId, void** ppObject )
{
   return com::S_Ok;
}

unsigned cursor_i::release() 
{                                                                                                  assert( m_iReference > 0 );
   m_iReference--; 
   if( m_iReference == 0 )
   {
      delete this;
      return 0;
   }
   
   return (unsigned)m_iReference; 
};

std::pair<bool, std::string> cursor_i::prepare( const std::string_view& stringSql )
{
   return m_pcursor->prepare( stringSql );
}

std::pair<bool, std::string> cursor_i::prepare( const std::string_view& stringSql, const std::vector< gd::variant_view >& vectorValue )
{
   return m_pcursor->prepare( stringSql, vectorValue );
}

std::pair<bool, std::string> cursor_i::bind( const std::vector< gd::variant_view >& vectorValue )
{
   return m_pcursor->bind_parameter( 1, vectorValue );
}

std::pair<bool, std::string> cursor_i::bind( unsigned uIndex, const std::vector< gd::variant_view >& vectorValue )
{
   return m_pcursor->bind_parameter( uIndex, vectorValue );
}

std::pair<bool, std::string> cursor_i::open()
{
   return m_pcursor->open();
}

std::pair<bool, std::string> cursor_i::open( const std::string_view& stringStatement )
{
   return m_pcursor->open( stringStatement );
}

std::pair<bool, std::string> cursor_i::next()
{                                                                                                  assert( m_pcursor->is_valid_row() == true );
   bool bIsOnRow;
   unsigned uUpdateCount = m_pcursor->get_column_count();
   return m_pcursor->next( &bIsOnRow, uUpdateCount );
}

std::pair<bool, std::string> cursor_i::execute()
{
   return m_pcursor->execute();
}


std::pair<bool, std::string> cursor_i::get_record( record** ppRecord )
{                                                                                                  assert( ppRecord != nullptr );
   *ppRecord = m_pcursor->get_record();
   return { true, "" };
}

void cursor_i::close()
{
   m_pcursor->close();
}



// ----------------------------------------------------------------------------
// ----------------------------------------------------------------- database_i
// ----------------------------------------------------------------------------

int32_t database_i::query_interface( const com::guid& guidId, void** ppObject )
{
   if( guidId == COMPONENT_CURSOR )                                            // cursor interface ?
   {
      cursor_i* pcursor = new cursor_i( m_pdatabase.get() );
      *ppObject = pcursor;
      return com::S_Ok;
   }
   
   return com::E_NoInterface;
}

unsigned database_i::release() 
{                                                                                                  assert( m_iReference > 0 );
   m_iReference--; 
   if( m_iReference == 0 )
   {
      delete this;
      return 0;
   }
   
   return (unsigned)m_iReference; 
};


std::pair<bool, std::string> database_i::open( const std::string_view& stringDriverConnect )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->open( stringDriverConnect );
}

std::pair<bool, std::string> database_i::open( const gd::argument::arguments& argumentsConnect )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->open( argumentsConnect );
}

std::pair<bool, std::string> database_i::execute( const std::string_view& stringStatement )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->execute( stringStatement );
}

std::pair<bool, std::string> database_i::ask( const std::string_view& stringStatement, gd::variant* pvariantValue )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->ask( stringStatement, pvariantValue );
}

std::pair<bool, std::string> database_i::get_cursor( gd::database::cursor_i** ppCursor )
{                                                                                                  assert( ppCursor != nullptr );
   cursor_i* pcursor = new cursor_i( m_pdatabase.get() );

   *ppCursor = pcursor;

   return { true, "" };
}

void database_i::close()
{                            
   if( m_pdatabase->is_open() == true ) 
   { 
      m_pdatabase->close(); 
   }
}

void database_i::erase()
{                            
   close();
   delete this;
}

gd::variant database_i::get_change_count()
{
   // TODO: This need logic to return number of changes
   return int64_t( 1 );
}

gd::variant database_i::get_insert_key()
{
   // TODO: This need logic to return last primary key generated for insert
   return int64_t( 1 );
}




_GD_DATABASE_ODBC_END

#endif // GD_DATABASE_ODBC_USE
