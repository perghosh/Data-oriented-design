#ifdef GD_DATABASE_SQLITE_USE

#include <vector>

#include "gd_database_sqlite.h"


_GD_DATABASE_SQLITE_BEGIN


/** -----------------------------------------------------------------------------------------------
 * @brief Open sqlite database from specified file
 * Opens sqlite database file. If file do not exist then a new database is created
 * @param stringFileName database file to open
 * @param vectorFlags vector of flags for open operation, possible values are: "owner" and "connected"
 * @return true if ok, false and error information if failed
 */
std::pair<bool, std::string> database::open(const std::string_view& stringFileName, const std::vector<std::string_view>& vectorFlags )
{
   if( m_psqlite3 != nullptr ) close();

   auto [psqlite3, stringError] = open_s(stringFileName, vectorFlags );
   if( psqlite3 != nullptr )
   {
      m_psqlite3 = psqlite3;
      if( stringError.empty() == true )
      {
         set_flags( eDatabaseStateOwner|eDatabaseStateConnected, 0 );
         return { true, std::string() };        // no error text then ok
      }

      return { false, stringError };
   }
   
   return { false, stringError };
}

/** -----------------------------------------------------------------------------------------------
 * @brief Open sqlite database from specified file
 * Opens sqlite database file. If file do not exist then a new database is created
 * @param stringFileName database file to open
 * @param uFlags flags for open operation, possible values are: eDatabaseStateOwner and eDatabaseStateConnected
 * @return true if ok, false and error information if failed
*/
std::pair<bool,std::string> database::open(const std::string_view& stringFileName, unsigned uFlags)
{
   if( m_psqlite3 != nullptr ) close();

   auto [psqlite3, stringError] = open_s(stringFileName, uFlags );
   if( psqlite3 != nullptr )
   {
      m_psqlite3 = psqlite3;
      if( stringError.empty() == true )
      {
         set_flags( eDatabaseStateOwner|eDatabaseStateConnected, 0 );
         return { true, std::string() };        // no error text then ok
      }

      return { false, stringError };
   }
   
   return { false, stringError };
}

std::pair<bool, std::string> database::execute( const std::string_view& stringQuery, std::function<bool( const gd::argument::arguments* )> callback_ )
{                                                                                                   assert(m_psqlite3 != nullptr);

   // Create context to pass to sqlite3_exec callback
   struct callback_context {
      std::function<bool( const gd::argument::arguments* )>* callback;
      gd::argument::arguments arguments;
   };
   
   callback_context context;
   context.callback = &callback_;
   // SQLite callback that fills arguments and calls user callback
   auto sqlite_callback = [](void* pContext, int iColumnCount, char** ppValues, char** ppColumnNames) -> int {
      callback_context* pCallbackContext = static_cast<callback_context*>(pContext);
      
      // Clear arguments for new row
      pCallbackContext->arguments.clear();
      
      // Fill arguments with column name-value pairs
      for( int i = 0; i < iColumnCount; i++ )
      {
         const char* pbszColumnName = ppColumnNames[i];
         const char* pbszValue = ppValues[i];
         
         if( pbszValue != nullptr )
         {
            // Try to determine type from value
            // SQLite3_exec returns all values as strings, so we store them as strings
            pCallbackContext->arguments.append(pbszColumnName, std::string_view(pbszValue));
         }
         else
         {
            // NULL value
            pCallbackContext->arguments.append(pbszColumnName, nullptr);
         }
      }
      
      // Call user callback
      bool bContinue = (*pCallbackContext->callback)(&pCallbackContext->arguments);
      
      // Return 0 to continue, non-zero to abort
      return bContinue ? 0 : 1;
   };

   char* pbszError = nullptr;
   auto iExecuteResult = ::sqlite3_exec(m_psqlite3, stringQuery.data(), nullptr, nullptr, &pbszError );
   if( iExecuteResult != SQLITE_OK )
   {
      std::string stringError = pbszError;
      ::sqlite3_free( pbszError );                                                                 assert( iExecuteResult != SQLITE_OK );
      return { false, std::move(stringError) };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Ask for single value from sql statement
 * @param stringStatement string to execute that should returan at least one value
 * @param pvariantValue pointer to variant that gets value from statement
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> database::ask( const std::string_view& stringStatement, gd::variant* pvariantValue )
{                                                                                                  assert( m_psqlite3 != nullptr );
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

/**---------------------------------------------------------------------------
 * @brief Executes a database transaction based on the provided variant view.
 *
 * This method processes a transaction command specified either as a string
 * ("begin", "commit", "rollback", or a raw SQL command) or as an integer using
 * predefined transaction enumeration values. It returns a pair containing a
 * boolean indicating success/failure and a string with the result or error message.
 *
 * @param transaction_ The transaction command as a gd::variant_view, which can contain
 *                     either a string or integer representation of the transaction.
 * @return std::pair<bool, std::string> A pair where:
 *         - first: true if the transaction succeeded, false otherwise
 *         - second: result message or error description
 *
 * @note Supported string commands:
 *       - "begin": Starts a new transaction
 *       - "commit": Commits the current transaction
 *       - "rollback": Rolls back the current transaction
 *       - Any other string: Treated as a raw SQL command to execute
 * @note Supported integer commands:
 *       - eTransactionBegin: Begins a transaction
 *       - eTransactionCommit: Commits a transaction
 *       - eTransactionRollback: Rolls back a transaction
 * @note If the input type or value is not supported, returns {false, "not implemented"}.
 */
std::pair<bool, std::string> database::transaction( const gd::variant_view& transaction_ )
{
   if( transaction_.is_string() == true )
   {
      std::string stringTransaction = transaction_.as_string();
      if( stringTransaction == "begin" ) return execute("BEGIN TRANSACTION");
      if( stringTransaction == "commit" ) return execute("COMMIT TRANSACTION");
      if( stringTransaction == "rollback" ) return execute("ROLLBACK TRANSACTION");
      else return execute( stringTransaction );
   }
   else if( transaction_.is_int() == true )
   {
      int iTransaction = transaction_.as_int();
      if( iTransaction == eTransactionBegin ) return execute("BEGIN TRANSACTION");
      if( iTransaction == eTransactionCommit ) return execute("COMMIT TRANSACTION");
      if( iTransaction == eTransactionRollback ) return execute("ROLLBACK TRANSACTION");
      if( iTransaction == eTransactionMerge ) 
      {
         auto iResult = ::sqlite3_wal_checkpoint(m_psqlite3, nullptr);
         if( iResult != SQLITE_OK ) 
         {
            std::string stringError = sqlite3_errmsg(m_psqlite3);
            return { false, stringError };
         } 

         return { true, "" };
      }
   }

   return { false, "not implemented" };
}



/** -----------------------------------------------------------------------------------------------
 * @brief Returns last insert key that was generated using auto increment
 * @return gd::variant last generated key
*/
gd::variant database::get_insert_key() const
{                                                                                                  assert( m_psqlite3 != nullptr );
   int64_t iRowId = ::sqlite3_last_insert_rowid( m_psqlite3 );
   return iRowId;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get last insert key that was generated using auto increment or row id if key is another type
 * @param variantKey reference to variant that gets last generated key
 * @return true if ok, false and error information if failed
*/
std::pair<bool, std::string> database::get_insert_key( gd::variant& variantKey ) const
{                                                                                                  assert( m_psqlite3 != nullptr );
   variantKey = (int64_t)::sqlite3_last_insert_rowid( m_psqlite3 );
   return { true, "" };
}



/** -----------------------------------------------------------------------------------------------
 * @brief Returns number of changed rows
 * @return gd::variant get number of changes in recent call to server
*/
gd::variant database::get_change_count() const
{                                                                                                  assert( m_psqlite3 != nullptr );
   int32_t iCount = ::sqlite3_changes( m_psqlite3 );
   return iCount;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Convert string flags to SQLite integer flags
 * @param flags Vector of flag names (e.g., "read", "write", "create")
 * @return Combined integer flags
*/
constexpr int flags_s(const std::vector<std::string_view>& vectorFlag)
{
    int iFlags = 0;
    
    for(const auto& stringFlag : vectorFlag)
    {
       if(stringFlag == "read" || stringFlag == "readonly") {  iFlags |= SQLITE_OPEN_READONLY; }
       else if(stringFlag == "write" || stringFlag == "readwrite") { iFlags |= SQLITE_OPEN_READWRITE; }
       else if(stringFlag == "create") { iFlags |= SQLITE_OPEN_CREATE; }
       else if(stringFlag == "fullmutex") { iFlags |= SQLITE_OPEN_FULLMUTEX; }
    }
    
    return iFlags;
}

/** -----------------------------------------------------------------------------------------------
 * @brief Open SQLite database with string-based flags
 * @param stringFileName File name representing sqlite database
 * @param flags Vector of flag names (e.g., {"write", "create"})
 * @return Pointer to sqlite database or if error then add error information
*/
std::pair<sqlite3*, std::string> database::open_s( const std::string_view& stringFileName, const std::vector<std::string_view>& vectorFlags)
{
   int iFlags = flags_s(vectorFlags);
   if (iFlags == 0) iFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
    
   return open_s(stringFileName, iFlags);
}

/** -----------------------------------------------------------------------------------------------
 * @brief open sql database for file name, if no database is found then create new database
 * @param stringFileName file name representing sqlite database
 * @return pointer to sqlite database or if error then add error information
*/
std::pair<sqlite3*, std::string> database::open_s(const std::string_view& stringFileName, int iFlags)
{
   if( iFlags == 0 ) iFlags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX;
   
   sqlite3* psqlite3 = nullptr;

   auto iOpenResult = ::sqlite3_open_v2(stringFileName.data(), &psqlite3, iFlags, nullptr );
   if( iOpenResult == SQLITE_OK ) return { psqlite3, std::string() };

   auto pbszError = ::sqlite3_errmsg(psqlite3);
   if( pbszError == nullptr ) pbszError = "unknown error";

   return { psqlite3, pbszError };
}

/** ---------------------------------------------------------------------------
 * @brief Execute SQL query
 * Executes any valid SQL query against sqlite database. Use this to execute 
 * SQL queries that do not return a result where you need a result cursor.
 * @param psqlite pointer to sqlite connection
 * @param stringQuery sql query to execute against connected sqlite database
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> database::execute_s(sqlite3* psqlite, const std::string_view& stringQuery)
{                                                                                                  assert(psqlite != nullptr);
   char* pbszError = nullptr;
   auto iExecuteResult = ::sqlite3_exec(psqlite, stringQuery.data(), nullptr, nullptr, &pbszError );
   if( iExecuteResult != SQLITE_OK )
   {
      std::string stringError = pbszError;
      ::sqlite3_free( pbszError );                                                                 assert( iExecuteResult != SQLITE_OK );
      return { false, std::move(stringError) };
   }

   return { true, std::string() };
}

// TODO: fix binding for insert queries
std::pair<bool, std::string> database::bind_s( sqlite3* psqlite, const std::vector<gd::variant_view>& vectorValue )
{                                                                                                  assert( psqlite != nullptr );
/*
   for( auto it = std::begin( vectorValue ), itEnd = std::end( vectorValue ); it = )
   {

   }
*/

   return std::pair<bool, std::string>();
}

// TODO: add callback logic
/*
std::pair<bool, std::string> database::execute_s(sqlite3* psqlite, const std::string_view& stringQuery)
{                                                                                                  assert(psqlite != nullptr); assert(stringQuery.length() > sizeof("INSERT INTO"));
   char* pbszError = nullptr;
   auto iExecuteResult = ::sqlite3_exec(psqlite, stringQuery.data(), nullptr, nullptr, &pbszError );
   if( iExecuteResult != SQLITE_OK )
   {
      std::string stringError = pbszError;
      ::sqlite3_free( pbszError );
      return { false, std::move(stringError) };
   }

   return { true, std::string() };
}
*/


/** ---------------------------------------------------------------------------
 * @brief Close down sqlite database connection
 * @param psqlite pointer to database connection to close
*/
void database::close_s( sqlite3* psqlite )
{
   if( psqlite != nullptr )
   {
      ::sqlite3_close(psqlite);
   }
}



/** ---------------------------------------------------------------------------
 * @brief Open select statement
 * @return true if ok otherwise false and error information
*/
std::pair<bool, std::string> cursor::open()
{                                                                                                  assert( m_pdatabase != nullptr ); assert( m_pstmt != nullptr );
   // ## if no record buffer is allocated for internal statement then we need to do that before steping in result
   if( m_recordRow.empty() == true )
   {
      bind_columns_s( m_pstmt, m_recordRow );                                  // bind buffers to columns in result
   }
                                                                                                   assert( m_recordRow.empty() == false );
   auto iResult = ::sqlite3_step( m_pstmt );                                   // move to first row in result
   if( iResult == SQLITE_ROW )
   {
      update();                                                                // update buffers for each column
      m_uState |= eCursorStateRow;                                             // valid row state
   }
   else if( iResult == SQLITE_DONE )
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
   }
   else if( iResult == SQLITE_OK )
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
   }
   else if( iResult < SQLITE_ROW )   
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }
   else
   {                                                                                               assert( false );
      m_uState &= ~eCursorStateRow;                                            // no valid row state
      return { false, "" };                                                    // should we get here?
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Opens result set from select string
 * @param stringSql select string to execute against database that opens cursor
 * @return true if ok, false and error information on error
*/
std::pair<bool, std::string> cursor::open(const std::string_view& stringSql)
{                                                                                                  assert( m_pdatabase != nullptr );
   close();                                                                    // close statement if there is one active open statement
   sqlite3_stmt* pstmt;
   const char* pbszTail;
   auto iResult = ::sqlite3_prepare_v2(m_pdatabase->get_sqlite3(), stringSql.data(), (int)stringSql.length(), &pstmt, &pbszTail);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());                               assert( iResult != SQLITE_OK );
      return { false, pbszError };
   }

   bind_columns_s( pstmt, m_recordRow );                                       // bind buffers to columns in result

   m_pstmt = pstmt;

   iResult = ::sqlite3_step( pstmt );                                          // move to first row in result
   if( iResult == SQLITE_ROW )
   {
      update();                                                                // update buffers for each column
      m_uState |= eCursorStateRow;                                             // valid row state
   }
   else if( iResult == SQLITE_DONE )
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
   }
   else if( iResult == SQLITE_OK )
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
   }
   else if( iResult < SQLITE_ROW )   
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }
   else
   {                                                                                               assert( false );
      m_uState &= ~eCursorStateRow;                                            // no valid row state
      return { false, "" };                                                    // should we get here?
   }

   return { true, "" };
}

/** -----------------------------------------------------------------------------------------------
 * @brief Open select statement
~~~~~~~~~~~~~{.c++}
   std::string stringSelect = std::string{R"SQL(SELECT n FROM registries WHERE idc = ? and idc = ?)SQL"};
   gd::database::sqlite::database databaseTest;

   gd::database::sqlite::cursor cursorRegisteres( &databaseTest );

   auto result_ = cursorRegisteres.open( stringSelect, []( auto pstmt ) -> bool {
      auto count_ = ::sqlite3_bind_parameter_count( pstmt );
      auto name_ = ::sqlite3_bind_parameter_name(pstmt, 0);
      name_ = ::sqlite3_bind_parameter_name(pstmt, 1);
      ::sqlite3_bind_int( pstmt, 0, 1 );
      ::sqlite3_bind_int( pstmt, 0, 2 );
      return true;
   });
~~~~~~~~~~~~~
 * @param stringSql SELECT query cursor will handle
 * @param callbackPrepare callback to enable processing preparing query
 * @return true if ok otherwise false and error information
*/
std::pair<bool, std::string> cursor::open( const std::string_view& stringSql, std::function<bool( sqlite3_stmt* pstmt )> callbackPrepare )
{
   close();                                                                    // close statement if there is one active open statement
   sqlite3_stmt* pstmt;
   const char* pbszTail;
   auto iResult = ::sqlite3_prepare_v2(m_pdatabase->get_sqlite3(), stringSql.data(), (int)stringSql.length(), &pstmt, &pbszTail);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());                               assert( iResult != SQLITE_OK );
      return { false, pbszError };
   }

   if( (bool)callbackPrepare == true )
   {
      bool bOk = callbackPrepare( pstmt );

      if( bOk == false ) return { bOk, "callback error" };
   }

   bind_columns_s( pstmt, m_recordRow );                                       // bind buffers to columns in result

   m_pstmt = pstmt;

   iResult = ::sqlite3_step( pstmt );                                          // move to first row in result
   if( iResult == SQLITE_ROW )
   {
      update();                                                                // update buffers for each column
      m_uState |= eCursorStateRow;                                             // valid row state
   }
   else if( iResult == SQLITE_DONE )
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
   }
   else if( iResult == SQLITE_OK )
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
   }
   else if( iResult < SQLITE_ROW )   
   {
      m_uState &= ~eCursorStateRow;                                            // no valid row state
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }
   else
   {                                                                                               assert( false );
      m_uState &= ~eCursorStateRow;                                            // no valid row state
      return { false, "" };                                                    // should we get here?
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prepare sql into statement
 * @param stringSql sql command to be prepared
 * @return true if ok, false and error information if fail
*/
std::pair<bool, std::string> cursor::prepare( const std::string_view& stringSql )
{
   close();                                                                    // close statement if there is one active open statement
   sqlite3_stmt* pstmt;
   const char* pbszTail;
   auto iResult = ::sqlite3_prepare_v2(m_pdatabase->get_sqlite3(), stringSql.data(), (int)stringSql.length(), &pstmt, &pbszTail);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());                               assert( iResult != SQLITE_OK );
      return { false, pbszError };
   }

   m_pstmt = pstmt;

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief binds value to parameter in sql query
 * @param iIndex parameter index to bind to
 * @param VVValue value parameter gets
 * @return true if ok, false and error information if error
*/
std::pair<bool, std::string> cursor::bind_parameter( int iIndex, const gd::variant_view& VVValue )
{                                                                                                  assert( m_pstmt != nullptr );
   int iResult;
   using namespace gd::variant_type;

#ifdef _DEBUG
   auto uGroup_d = VVValue.get_type_group();
#endif // _DEBUG


   switch( VVValue.get_type_group() & ~(eGroupSigned | eGroupNumber) )         // clear types not needed in switch
   {
      case 0:
      {                                                                                            assert( VVValue.type_number() == eTypeNumberUnknown );
         iResult = ::sqlite3_bind_null( m_pstmt, iIndex );
      }
      break;

      case eGroupBoolean:
      case eGroupInteger:
      {
         if( VVValue.type_number() == eTypeNumberInt64 )
         {
            iResult = ::sqlite3_bind_int64( m_pstmt, iIndex, (int64_t)VVValue );
         }
         else
         {
            auto value_ = VVValue.as_int64();
            iResult = ::sqlite3_bind_int64( m_pstmt, iIndex, value_ );
         }
      }
      break;

      case eGroupDecimal:
      {
         if( VVValue.type_number() == eTypeNumberDouble )
         {
            iResult = ::sqlite3_bind_double( m_pstmt, iIndex, VVValue );
         }
         else
         {
            auto value_ = VVValue.as_double();
            iResult = ::sqlite3_bind_double( m_pstmt, iIndex, value_ );
         }
      }
      break;

      case eGroupString:
      case eGroupDate:
      {
         if( VVValue.is_char_string() )
         {
            iResult = ::sqlite3_bind_text( m_pstmt, iIndex, VVValue.c_str(), VVValue.length(), nullptr);
         }
         else
         {
            auto value_ = VVValue.as_string();
            iResult = ::sqlite3_bind_text( m_pstmt, iIndex, value_.c_str(), (int)value_.size(), nullptr );
         }
      }
      break;

      case eGroupBinary:
      {
         if( VVValue.is_binary() == true )
         {
            iResult = ::sqlite3_bind_blob( m_pstmt, iIndex, (const void*)(const unsigned char*)VVValue, VVValue.length(), nullptr );
         }
         else
         {                                                                                         assert( false );
               
         }
      }
      break;


      default:
      {
         return { false, "Missmatch using grouped types, developer error" };
      }

   }

   if( iResult == SQLITE_OK ) return { true, "" };

   auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
   return { false, pbszError };
}

/** ---------------------------------------------------------------------------
 * @brief Execute a prepared statement, useful for queries with parameters
 * @return true if ok, false and error information if not
*/
std::pair<bool, std::string> cursor::execute()
{                                                                             assert( m_pstmt != nullptr );

   int iResult = sqlite3_step(m_pstmt);
   if( iResult != SQLITE_DONE && iResult != SQLITE_ROW )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }


   iResult = sqlite3_clear_bindings(m_pstmt);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }

   iResult = sqlite3_reset(m_pstmt);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Update column buffers
 * @param uFrom start on column to update buffer
 * @param uTo end before this column updating buffers
*/
void cursor::update( unsigned uFrom, unsigned uTo )
{                                                                              assert( m_pstmt != nullptr );
   enumColumnTypeComplete eType; // column type
   uint8_t* pbBuffer;            // pointer to active field in sqlite stmt (statement)
   int iValueSize = 0;           // gets value size for field that do not have a max limit for size

   for( auto u = uFrom; u < uTo; u++ )
   {
      int iType = ::sqlite3_column_type( m_pstmt, u );
      gd::database::record::column* pcolumn = m_recordRow.get_column( u );
      if (iType != SQLITE_NULL)
      {
         pcolumn->set_null(false);
         eType = (enumColumnTypeComplete)pcolumn->type();
         if( pcolumn->is_fixed() == true )
         {
            pbBuffer = m_recordRow.buffer_get( u );                            assert( pbBuffer != nullptr ); // get buffer to column
         }
         else
         {
            // ## Compare buffer size with value size, if buffer size is smaller than
            //    needed value size, then increase buffer size.
            //    Remember that first 8 bytes in buffer has length as unsigned and type ans unsigned.
            pbBuffer = m_recordRow.buffer_get_detached( pcolumn->value() ); 
            unsigned uBufferSize = buffers::buffer_size_s( pbBuffer );
            iValueSize = ::sqlite3_column_bytes(m_pstmt, u);              
            if( (iValueSize + 1) > ( int )uBufferSize )                        // compare size and add one for the zero terminator
            {
               pbBuffer = m_recordRow.resize( pcolumn->value(), iValueSize + 1 );// increase buffer size, add one for zero termination
            }
            pbBuffer = buffers::buffer_get_value_from_root_s( pbBuffer );
#ifdef _DEBUG
            auto buffer_length_d = *( unsigned* )buffers::buffer_get_root_from_value_s( pbBuffer );
            auto value_length_d = (unsigned)::sqlite3_column_bytes( m_pstmt, u );
            assert( buffer_length_d >= value_length_d );
#endif // _DEBUG

         }

         switch( eType )
         {
         case eColumnTypeCompleteInt64:
            *(int64_t*)pbBuffer = ::sqlite3_column_int64( m_pstmt, u );
            break;
         case eColumnTypeCompleteCDouble:
            *(double*)pbBuffer = ::sqlite3_column_double( m_pstmt, u );
            break;
         case eColumnTypeCompleteUtf8String: 
         {
            const unsigned char* pbValue = ::sqlite3_column_text(m_pstmt, u);  // get text value from field

            if( iValueSize == 0 )                                              // if text value length hasn't been retrieved then this must be a fixed buffer
            {
               // ## Copy text from buffer in sqlite db
               int iSize = ::sqlite3_column_bytes( m_pstmt, u );
               memcpy( pbBuffer, pbValue, iSize );
               pbBuffer[iSize] = '\0';
               pcolumn->size( iSize );
            }
            else
            {
               int iSize = iValueSize;
               memcpy( pbBuffer, pbValue, iSize );
               pbBuffer[iSize] = '\0';
               pcolumn->size( iSize );
               iValueSize = 0;                                                 // reset value size
            }
         }
         break;
         case eColumnTypeCompleteBinary: 
         {
            const uint8_t* pbValue = (const uint8_t*)::sqlite3_column_blob(m_pstmt, u);  // get binary value from field

            if( iValueSize == 0 )                                              // if binary value length hasn't been retrieved then this must be a fixed buffer
            {
               // ## Copy text from buffer in sqlite db
               int iSize = ::sqlite3_column_bytes( m_pstmt, u );
               memcpy( pbBuffer, pbValue, iSize );
               pcolumn->size( iSize );
            }
            else
            {
               int iSize = iValueSize;
               memcpy( pbBuffer, pbValue, iSize );
               iValueSize = 0;                                                 // reset value size
            }
         }
         break;


         default:
            assert( false );
         }
      }
      else
      {
         pcolumn->set_null(true);
      }
   }
}

/** -----------------------------------------------------------------------------------------------
 * @brief Go to next row
 * Move cursor to next row in result and update internal buffers with data from result row
 * @return true if ok, false and error information if eror
*/
std::pair<bool, std::string> cursor::next()
{
   int iResult = ::sqlite3_step(m_pstmt);
   if(iResult == SQLITE_ROW) 
   { 
      m_uState |= eCursorStateRow; 
      update();
   }
   else if( iResult == SQLITE_DONE )
   {
      m_uState &= ~eCursorStateRow; 
   }
   else
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }

   return { true, std::string() };
}

/** -----------------------------------------------------------------------------------------------
 * @brief resets active statement, clears bindings 
 * @return true if ok, false and error information if eror
*/
std::pair<bool, std::string> cursor::reset()
{                                                                             assert( m_pstmt != nullptr );
   int iResult = sqlite3_clear_bindings(m_pstmt);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }

   iResult = sqlite3_reset(m_pstmt);
   if( iResult != SQLITE_OK )
   {
      auto pbszError = ::sqlite3_errmsg(m_pdatabase->get_sqlite3());
      return { false, pbszError };
   }

   return { true, "" };
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
      case eColumnTypeCompleteInt32: return gd::variant( (int32_t)*(int64_t*)pbBuffer);
      case eColumnTypeCompleteInt64: return gd::variant( *(int64_t*)pbBuffer );
      case eColumnTypeCompleteCDouble: return gd::variant( *(double*)pbBuffer );
      case eColumnTypeCompleteString: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteUtf8String: return gd::variant( (const char*)pbBuffer, (size_t)pcolumn->size() );
         
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
      case eColumnTypeCompleteInt32: return gd::variant_view( (int32_t)*(int64_t*)pbBuffer);
      case eColumnTypeCompleteInt64: return gd::variant_view( *(int64_t*)pbBuffer );
      case eColumnTypeCompleteCDouble: return gd::variant_view( *(double*)pbBuffer );
      case eColumnTypeCompleteString: return gd::variant_view( (const char*)pbBuffer, (size_t)pcolumn->size() );
      case eColumnTypeCompleteUtf8String: return gd::variant_view( gd::variant_type::utf8( (const char*)pbBuffer, (size_t)pcolumn->size()) );
         
      default: assert(false);
      }
   }

   return gd::variant_view();
}

/** -----------------------------------------------------------------------------------------------
 * @brief Get value in named column
 * @param stringName column name where retured value i value
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

/**
 * @brief return index to column based field name
 * @param stringName name column index is returned for
 * @return int index to column if found, otherwise -1
*/
int cursor::get_index( const std::string_view& stringName ) const
{
   return m_recordRow.get_column_index_for_name( stringName );
}


/*----------------------------------------------------------------------------- get_column_type */ /**
 * Returns the type for this column. It will first try to check the declared type which was used
 * when the table was created. The reason is that SQLite allows any type to be stored in any column.
 * If that fails, the column's real type is returned. If all else fails, utf-8 type is returned.
 * \param iColumn Column number, 0-based
 * \return The column's type
 */
unsigned cursor::get_column_type_s( const char* pbszColumnType )
{
   switch( *pbszColumnType )
   {
   case 'B' : return pbszColumnType[2] == 'N' ? eColumnTypeCompleteBinary : eColumnTypeCompleteInt64;// BINARY|BIT|BIGINT
   case 'D' : return pbszColumnType[1] == 'E' ? eColumnTypeCompleteCDouble : eColumnTypeCompleteUtf8String;// DECIMAL | DATE
   case 'F' : return eColumnTypeCompleteCDouble;                                                   // FLOAT
   case 'G' : return eColumnTypeCompleteBinary;                                                    // GUID
   case 'I' : return eColumnTypeCompleteInt64;                                                     // INTEGER
   case 'N' : return pbszColumnType[1] == 'V' ? eColumnTypeCompleteUtf8String : eColumnTypeCompleteCDouble; // NVARCHAR | NUMERIC
   case 'R' : return eColumnTypeCompleteCDouble;                                                   // REAL
   case 'S' : return eColumnTypeCompleteInt64;                                                     // SMALLINT
   case 'T' : return pbszColumnType[1] == 'E' ? eColumnTypeCompleteUtf8String : eColumnTypeCompleteUtf8String; // TEXT | TIME
   case 'U' : return eColumnTypeCompleteUtf8String;                                                     // UTCTIME | UTCDATETIME
   case 'V' : return pbszColumnType[3] == 'C' ? eColumnTypeCompleteUtf8String : eColumnTypeCompleteBinary; // VARCHAR | VARBIN
   }

   return eColumnTypeCompleteUtf8String;
}

unsigned cursor::get_column_ctype_s( const char* pbszColumnType )
{
   switch( *pbszColumnType )
   {
   case 'B' : {
      if( pbszColumnType[2] == 'T' ) return eColumnTypeCompleteBit;                                // BIT
      if( pbszColumnType[2] == 'G' ) return eColumnTypeCompleteInt64;                              // BIGINT
      return eColumnTypeCompleteBinary;                                                            // BINARY 
   }
   case 'D' : return pbszColumnType[1] == 'E' ? eColumnTypeCompleteCDouble : eColumnTypeCompleteDateTime;// DECIMAL | DATE
   case 'F' : return eColumnTypeCompleteCDouble;                                                   // FLOAT
   case 'G' : return eColumnTypeCompleteGuid;                                                      // GUID
   case 'I' : return eColumnTypeCompleteInt32;                                                     // INTEGER
   case 'N' : return pbszColumnType[1] == 'V' ? eColumnTypeCompleteUtf8String : eColumnTypeCompleteCDouble; // NVARCHAR | NUMERIC
   case 'R' : return eColumnTypeCompleteCDouble;                                                   // REAL
   case 'S' : return eColumnTypeCompleteInt16;                                                     // SMALLINT
   case 'T' : {
      if( pbszColumnType[2] == 'M' ) return eColumnTypeCompleteTime;                               // TIME
      if( pbszColumnType[2] == 'N' ) return eColumnTypeCompleteInt8;                               // TINYINT
      return eColumnTypeCompleteUtf8String;                                                        // TEXT 
   }
   case 'U' : return eColumnTypeCompleteInt64;                                                     // UTCTIME | UTCDATETIME
   case 'V' : return pbszColumnType[3] == 'C' ? eColumnTypeCompleteUtf8String : eColumnTypeCompleteBinary; // VARCHAR | VARBIN
   }

   return eColumnTypeCompleteUtf8String;
}

/** ---------------------------------------------------------------------------
 * @brief bind buffers to column values in result
 * @param pstmt result statement buffers binds to
 * @param recordBindTo record object with buffer logic
*/
void cursor::bind_columns_s( sqlite3_stmt* pstmt, record& recordBindTo )
{
   char pb4Bytes[4];

   // ## Collect information about columns in result for buffers storing result data
   auto iColumnCount = ::sqlite3_column_count( pstmt );
   for( auto i = 0; i < iColumnCount; i++ )
   {
      unsigned uColumnType = eColumnTypeCompleteUnknown;
      const char* pbszColumnType = ::sqlite3_column_decltype(pstmt, i);
      if( pbszColumnType != nullptr )                                          // if declared type is valid then try to get type from type name
      {
         if( *pbszColumnType < 'a' ) uColumnType = get_column_type_s( pbszColumnType );
         else
         {
            // ## Convert first four characters to uppercase and get type from it.
            unsigned uIndex = 0;
            while( uIndex < 4 && pbszColumnType[uIndex] != '\0' )
            {
               pb4Bytes[uIndex] = pbszColumnType[uIndex] >= 'a' ? pbszColumnType[uIndex] - ('a' - 'A') : pbszColumnType[uIndex];
               uIndex++;
            }
            uColumnType = get_column_type_s( pb4Bytes );
         }
      }
      else
      {
         // ## Unable to get declared type for column, we use sqlite native type and
         //    convert it to our type.
         int iType = ::sqlite3_column_type(pstmt, i);                          // get internal sqlite type
         if( iType == SQLITE_TEXT )       uColumnType = eColumnTypeCompleteUtf8String;
         else if( iType == SQLITE_BLOB )  uColumnType = eColumnTypeCompleteBinary;
         else if( iType == SQLITE_FLOAT ) uColumnType = eColumnTypeCompleteCDouble;
         else if( iType == SQLITE_NULL )  uColumnType = eColumnTypeCompleteUtf8String;
         else                             uColumnType = eColumnTypeCompleteInt64;
      }
      unsigned uStartBufferSize = 0;
      auto pbszName = ::sqlite3_column_name( pstmt, i );
      auto uSize = value_size_g( uColumnType );
      if( uSize == 0 && pbszColumnType != nullptr ) uSize = value_size_g( pbszColumnType );
      if( uSize == 0 )
      {
         if( gd::database::value_group_type_g( uColumnType ) == eColumnTypeGroupString ) { uSize = 0; uStartBufferSize = 256; }
         else if( gd::database::value_group_type_g( uColumnType ) == eColumnTypeGroupBinary ) { uSize = 0; uStartBufferSize = 32; }
      }

      recordBindTo.add( uColumnType, uSize, uStartBufferSize, pbszName );                        // binds column buffer in result
   }

}

_GD_DATABASE_SQLITE_END

_GD_DATABASE_SQLITE_BEGIN

// ----------------------------------------------------------------------------
// ------------------------------------------------------------------- cursor_i
// ----------------------------------------------------------------------------


int32_t cursor_i::query_interface( const com::guid& guidId, void** ppObject )
{
   return com::E_NoInterface;
}

unsigned cursor_i::release() 
{                                                                                                  assert( m_iReference > 0 );
   m_iReference--; 
   if( m_iReference == 0 )
   {
      m_pcursor->close();                                                      // make sure to close statement handle
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
   return m_pcursor->bind_parameter( vectorValue );
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
   return m_pcursor->next();
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
      m_pdatabase->close();
      delete this;
      return 0;
   }
   
   return (unsigned)m_iReference; 
};

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

std::pair<bool, std::string> database_i::open( const std::string_view& stringDriverConnect )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->open( stringDriverConnect );
}

std::pair<bool, std::string> database_i::open( const gd::argument::arguments& argumentsConnect )
{                                                                                                  assert( m_pdatabase != nullptr );
   unsigned uFlags = (SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX);
   std::string stringFile = argumentsConnect["file"].as_string();

   if( argumentsConnect["create"].is_true() == true ) { uFlags |= SQLITE_OPEN_CREATE; }

   return m_pdatabase->open( stringFile, uFlags );
}

std::pair<bool, std::string> database_i::execute( const std::string_view& stringStatement )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->execute( stringStatement );
}

std::pair<bool, std::string> database_i::ask( const std::string_view& stringStatement, gd::variant* pvariantValue )
{                                                                                                  assert( m_pdatabase != nullptr );
   return m_pdatabase->ask( stringStatement, pvariantValue );
}

std::pair<bool, std::string> database_i::transaction(const gd::variant_view& transaction_)
{                                                                                                  assert(m_pdatabase != nullptr);
   return m_pdatabase->transaction(transaction_);
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
   return m_pdatabase->get_change_count();
}

gd::variant database_i::get_insert_key()
{
   return m_pdatabase->get_insert_key_raw();
}




_GD_DATABASE_SQLITE_END


#endif // GD_DATABASE_SQLITE_USE
