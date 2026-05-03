// @FILE [tag: api, base] [summary: Implementation of base class for API commands] [type: source] [name: API_Base.cpp]

#include <memory>
#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/parse/gd_parse_json.h"
#include "gd/gd_database_sqlite.h"
#include "gd/gd_file.h"
#include "gd/database/gd_database_io.h"

#include "gd/gd_sql_query.h"
#include "gd/gd_sql_query_builder.h"

#include "../lua/LUAObjects.h"

#include "../Application.h"

#include "../render/RENDERSql.h"

#include "API_Scripting.h"

#include "API_Base.h"

std::string CAPIContext::GetIpAddress()
{
   if( ( m_uFlags & eFlagSession ) != 0 && m_psession != nullptr )
   {
      return m_psession->as_string("ip");
   }
   return {};
}

std::string CAPIContext::GetSessionId()
{
   if( ( m_uFlags & eFlagSession ) != 0 && m_psession != nullptr )
   {
      return m_psession->as_string("session");
   }
   return {};
}

void CAPIContext::ResetResults()
{
   m_objects.Clear();
   m_argumentsGlobal.clear();
   m_stringLastError.clear();
   ClearFlag( eFlagHasResult );
   ClearFlag( eFlagHasError );
}

void CAPIContext::ResetDatabase() 
{ 
   if( ( m_uFlags & eFlagDatabaseOwner ) != 0 && m_pdatabase != nullptr )
   {
      m_pdatabase->close();
      m_pdatabase->release();
   }

   m_pdatabase = nullptr; 
   ClearFlag( eFlagDatabaseOwner ); 
}


void CAPIContext::Reset()
{
   m_papplication = nullptr;
   m_pdocument    = nullptr;
   m_psession     = nullptr;
   m_uFlags       = eFlagNone;
   ResetResults();
}


// ## implementation -----------------------------------------------------------

/** -------------------------------------------------------------------------- CAPI_Base::common_construct
 * @brief Move constructor implementation for CAPI_Base.
 *
 * Execution-scoped members are moved directly.
 * Context ownership requires care: if the source was using its locally owned
 * context (m_pcontext == &o.m_contextOwned) we move that context into our own
 * m_contextOwned and point m_pcontext at it.  If the source held an external
 * context pointer we simply copy the pointer — the external lifetime is managed
 * by the caller (typically CRouter).
 */
void CAPI_Base::common_construct( CAPI_Base&& o ) noexcept
{
   // -- execution-scoped members --------------------------------------------
   m_stringCommand          = o.m_stringCommand;
   m_vectorCommand          = std::move( o.m_vectorCommand );
   m_uCommandIndex          = std::exchange( o.m_uCommandIndex, 0 );
   m_argumentsQS            = std::move( o.m_argumentsQS );
   m_stringBody             = o.m_stringBody;
   m_arrayBufferCounter     = std::move( o.m_arrayBufferCounter );    // NOTE: was self-moving in original — fixed
   m_argumentsArgumentCount = std::move( o.m_argumentsArgumentCount );

   // -- context ownership ---------------------------------------------------
   const bool bSourceOwnsContext = ( o.m_pcontext == &o.m_contextOwned );
   if( bSourceOwnsContext )
   {
      // Move the locally owned context into ours and point at it
      m_contextOwned = std::move( o.m_contextOwned );
      m_pcontext     = &m_contextOwned;
   }
   else
   {
      // External context: copy the pointer, leave our m_contextOwned default
      m_pcontext = o.m_pcontext;
   }

   // Leave the source in a valid but empty state
   o.m_pcontext = &o.m_contextOwned;
}

/** -------------------------------------------------------------------------- CAPI_Base::GetDocument
 * @brief Retrieves the document associated with the current API instance.
 *
 * If the context already has a document bound, it is returned immediately.
 * Otherwise the document name is resolved from the request parameters
 * ("document" or "doc", defaulting to "default") and looked up via the
 * application.  The resolved pointer is stored back into the context so
 * subsequent calls within the same request are free.
 *
 * @return Pointer to CDocument, or nullptr if not found (last error is set).
 */
CDocument* CAPI_Base::GetDocument()
{                                                                                                  assert( m_pcontext->GetApplication() != nullptr );
   if( m_pcontext->GetDocument() != nullptr ) return m_pcontext->GetDocument();

   CApplication* papplication = m_pcontext->GetApplication();

   std::string stringDocument = m_argumentsQS[{ {"document"}, {"doc"} }].as_string();
   if( stringDocument.empty() == true ) stringDocument = "default";

   CDocument* pdocument = papplication->DOCUMENT_Get( stringDocument );

   if( pdocument == nullptr )
   {
      m_pcontext->SetError( "document not found: " + stringDocument );
   }
   else
   {
      m_pcontext->SetDocument( pdocument );                             // cache in context for subsequent calls
   }

   return pdocument;
}

/** -------------------------------------------------------------------------- CAPI_Base::GetNextArgument
 * @brief Returns the next occurrence of a named request argument and advances its internal counter.
 *
 * Uses `GetArgumentIndex( stringName )` to determine which occurrence to read:
 * - `0`  -> first lookup by name only (`GetArgument( stringName )`)
 * - `>0` -> indexed lookup via `operator[]` (for repeated keys like `xml`, `xml[2]`, ...)
 *
 * If a non-null value is found, `IncrementArgumentCounter( stringName )` is called so the
 * next invocation returns the following occurrence for the same argument name.
 *
 * @param stringName Argument key to read from `m_argumentsParameter`.
 * @return gd::variant_view View of the resolved value; null view when no occurrence exists.
 */
gd::variant_view CAPI_Base::GetNextArgument( std::string_view stringName )
{
   gd::variant_view value_; // value for this occurrence of the argument

   size_t uIndex = GetArgumentIndex( stringName );                            // get the current use-count for this argument name
   if( uIndex == 0 ) value_ = GetArgument( stringName );                      // first occurrence: look up by name only
   else              value_ = ( *this )[{stringName, uIndex}];                // subsequent occurrence: look up by name and index (e.g. "xml", "xml[2]", etc.)

   if( value_.is_null() == false )
   {
      IncrementArgumentCounter( stringName );                                 // advance the counter for this argument name so that the next call picks up the next occurrence
   }

   return value_;
}

/** -------------------------------------------------------------------------- CAPI_Base::GetArgumentIndex
 * @brief Returns the current use-count for a named argument (single-name variant).
 *
 * The counter reflects how many times IncrementArgumentCounter() has been
 * called for this name within the current execution, allowing chained
 * operations to select the correct occurrence of a repeated argument key.
 *
 * @param stringName  Argument name to look up.
 * @return            Current count (0 if never incremented).
 */
size_t CAPI_Base::GetArgumentIndex( const std::string_view& stringName ) const
{
   if( m_argumentsArgumentCount.empty() ) return 0;

   if( m_argumentsArgumentCount.exists( stringName ) == true )
   {
      return m_argumentsArgumentCount[stringName].as_uint();
   }

   return 0;
}

/** -------------------------------------------------------------------------- CAPI_Base::GetArgumentIndex
 * @brief Counts paired command occurrences up to the active command index (deprecated).
 *
 * @param stringFirst   First segment name.
 * @param stringSecond  Second segment name.
 * @return              Number of matching consecutive pairs found before m_uCommandIndex.
 */
size_t CAPI_Base::GetArgumentIndex( const std::string_view& stringFirst,
                                    const std::string_view& stringSecond ) const
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands" );
   size_t uCount = 0;
   for( unsigned uIndex = 0; ( uIndex + 1 ) < m_uCommandIndex; ++uIndex )
   {
      std::string_view stringCommandFirst  = m_vectorCommand[uIndex];
      std::string_view stringCommandSecond = m_vectorCommand[uIndex + 1];

      if( stringCommandFirst == stringFirst || stringCommandSecond == stringSecond ) { ++uCount; }
   }
   return uCount;
}

/** -------------------------------------------------------------------------- CAPI_Base::IncrementArgumentCounter
 * @brief Increment the internal use-count for a named argument.
 *
 * Called by Execute_* methods after consuming one occurrence of a repeated
 * argument (e.g. "xml", "query", "record") so that the next operation in the
 * same request picks up the next occurrence.
 *
 * @param stringName  Name of the argument whose counter should advance.
 */
void CAPI_Base::IncrementArgumentCounter( std::string_view stringName )
{                                                                                                  assert( stringName.length() < 20 );
   if( m_argumentsArgumentCount.exists( stringName ) == false )
   {
      uint16_t uCount = 1;
      m_argumentsArgumentCount.append( stringName, uCount );
   }
   else
   {
      uint16_t uCount = m_argumentsArgumentCount[stringName];
      ++uCount;
      m_argumentsArgumentCount.set( stringName, uCount );
   }
}

/** -------------------------------------------------------------------------- CAPI_Base::Exists
 * @brief Check whether a named argument exists in the per-request query string.
 *
 * @param stringName  Argument name to test.
 * @return            True if present in m_argumentsQS.
 */
bool CAPI_Base::Exists( const std::string_view& stringName ) const
{
   return m_argumentsQS.exists( stringName );
}


/** -------------------------------------------------------------------------- CAPI_Base::PrepareStatement
 * @brief Resolve, preprocess, and prepare a SQL statement, then append it to `stringSelectAddTo`.
 *
 * Accepts `statement_id_` as either a numeric statement row (`size_t`) or a query
 * identifier (`std::string_view`) that is resolved through `Statement_Find`.
 *
 * Workflow:
 * - resolve `uStatementRow` from `statement_id_`
 * - execute Lua-side setup with `Lua_Execute`
 * - load statement template using `Statement_GetQuery`
 * - initialize `CRENDERSql` and apply optional request arguments:
 *   - `columns` via `AddColumns` (JSON format)
 *   - `values` via `AddValues` (JSON format)
 * - preprocess template (`Preprocess`) and keep transformed text alive if changed
 * - finalize render state (`Prepare`) and build SQL (`ToSqlFromTemplate`)
 * - write SQL into `stringSelectAddTo` (assign if empty, otherwise append)
 *
 * @param statement_id_ Statement selector: row index or named query identifier.
 * @param stringSelectAddTo Output SQL buffer; receives generated SQL by assign/append.
 * @return std::pair<bool, std::string> `first` is success; `second` is error text on failure.
 */
std::pair<bool, std::string> CAPI_Base::PrepareStatement( std::variant<size_t, std::string_view> statement_id_, std::string& stringSelectAddTo )
{
   std::string stringQuery;
   std::string stringSelect;
   uint64_t uStatementRow;
   CDocument* pdocument = GetDocument();                                                           assert( pdocument != nullptr );

   // ## Get statement row index ............................................

   if( std::holds_alternative<size_t>( statement_id_ ) == true ) { uStatementRow = std::get<size_t>( statement_id_ ); } // sanity check for row index)
   else
   {
      stringQuery = std::get<std::string_view>( statement_id_ );

      if( stringQuery.empty() == false )
      {
         int64_t iRow = Statement_Find( stringQuery );
         if( iRow == -1 ) { return { false, std::string( "query statement not found for: " ) + std::string( stringQuery ) }; }

         uStatementRow = static_cast<uint64_t>( iRow );
      }
      else { return { false, "statement identifier is empty" }; }
   }
                                                                                                   assert( uStatementRow < 10000 );
   // ## Harvest values from request arguments for Lua setup code (if any) ..
   CRENDERSql sql_( m_pcontext, uStatementRow );
   sql_.Initialize();

   if( Exists( "columns" ) == true )                                          // read "columns"
   {
      auto stringColumns = GetNextArgument( "columns" ).as_string();
      if( stringColumns.empty() == false ) { sql_.ColumnsAdd( stringColumns, gd::types::tag_json{}); }
   }

   if( Exists( "values" ) == true )                                           // read "values" 
   {
      auto stringValues = GetNextArgument( "values" ).as_string();
      if( stringValues.empty() == false ) { sql_.ColumnAddValues( stringValues, gd::types::tag_json{}); }
   }

   // ## Execute Lua setup code if any .......................................

   auto result_ = Lua_Execute( uStatementRow, pdocument, &sql_ );
   if( result_.first == false ) { return result_; }

   std::string_view stringSelectTemplate = Statement_GetQuery( uStatementRow );
   if( stringSelectTemplate.empty() == true ) { return { false, "query statement is empty for: " + std::string( stringQuery ) }; }

   std::string stringTemporary; // If preprocessing and it have modified query then we need to store it.

   result_ = sql_.Preprocess( stringSelectTemplate );
   if( result_.first == false ) { return result_; }
   else if( result_.second.empty() == false ) 
   { 
      stringTemporary = std::move( result_.second ); 
      stringSelectTemplate = stringTemporary;                                 // set to preprocessed query
   }

   result_ = sql_.Prepare();                                                                       if( result_.first == false ) { return result_; }

   result_ = sql_.ValidateColumnValues();                                                          if( result_.first == false ) { return result_; }

   stringSelect.clear();
   result_ = sql_.ToSqlFromTemplate( stringSelectTemplate, stringSelect );                         if( result_.first == false ) { return result_; }

   if( stringSelectAddTo.empty() == true ) { stringSelectAddTo = std::move( stringSelect ); }
   else { stringSelectAddTo += stringSelect; }

   return { true, "" };
}

/**  ---------------------------------------------------------------------- Statement_Find
 * @brief Find the row index for a named statement query.
 * 
 * Expects `stringQuery` to reference a query identifier prefixed with `#`.
 * The prefix is stripped before the lookup is forwarded to the document query
 * metadata.
 * 
 * @param stringQuery Query identifier to resolve, for example `#SelectUsers`.
 * @return int64_t Zero-based query row index, or `-1` if the document is
 *         missing, the identifier format is invalid, or the query is not found.
 * 
 * @TODO [tag: statement] [description: add logic to try to find query by key (uuid) if format match]
 */
int64_t CAPI_Base::Statement_Find( std::string_view stringQuery ) const
{                                                                                                  assert( stringQuery.empty() == false );
   const CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return -1; }

   if( stringQuery[0u] == '#' ) { stringQuery.remove_prefix(1); }             // remove leading #

   const META::CQueries* pqueries = pdocument->QUERIES_Get();
   int64_t iRow = pqueries->GetQueryRow( stringQuery );

   if( iRow != -1 ) { return static_cast<int64_t>( iRow ); }
   return -1;
}

/// @brief Get the SQL query template for a given statement row index.
std::string_view CAPI_Base::Statement_GetQuery( uint64_t uStatementRow ) const
{
   const CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return {}; }

   const META::CQueries* pqueries = pdocument->QUERIES_Get();                                     assert( uStatementRow < pqueries->Size() );
   return pqueries->GetQuery( uStatementRow );
}

/**  --------------------------------------------------------------------------- Lua_Execute
 * @brief Execute Lua pre-processing for a statement row if code exists.
 * 
 * Reads query argument `code` for `uStatementRow` and executes it in the current
 * request context. If no `code` is found, the method returns success without doing
 * any work.
 * 
 * During Lua execution, this method optionally forwards request argument `values`
 * to the SQL renderer as JSON input (`AddValues`) so Lua scripts can build complex
 * SQL payloads.
 * 
 * @param uStatementRow Zero-based statement row index in query metadata.
 * @param pdocument Active document that provides query metadata.
 * @return std::pair<bool, std::string>
 * - `{ true, "" }` when execution succeeds (or no code exists).
 * - `{ true, "abort" }` when context status is set to abort.
 * - `{ false, "<error>" }` when Lua execution or JSON value injection fails.
 * 
 * @NOTE [tag: lua,sql,statement] [summary: pre_sql_lua_hook]
 * This is a **pre-SQL hook** used when statement preparation requires logic that
 * cannot be expressed by the regular SQL template builder.
 */
std::pair<bool, std::string> CAPI_Base::Lua_Execute( uint64_t uStatementRow, CDocument* pdocument, CRENDERSql* psql_ )
{
   // ## Process code if any
   META::CQueries* pqueries = pdocument->QUERIES_Get();

   auto vectorCode = pqueries->GetArgumentsValues( uStatementRow, "code" );  // get code for insert
   if( vectorCode.empty() == true ) return { true, "" };

   // ## Execute lua code to prepare sql statement, this allows to use code to prepare complex sql statements that can not be prepared with the current sql builder

   auto result_ = SCRIPT::LuaRequestExecute( vectorCode, GetContext(), psql_, [&](sol::state* pstateLua, CAPIContext* pcontext_) -> std::pair<bool, std::string> {
   /*
      if( Exists( "values" ) == true )
      {
         auto* prequest_ = (*pstateLua)["request"].get<LUA::Request*>();
         auto* psql_ = prequest_->GetRenderSql();
         std::array<std::byte, 128> buffer_;
         gd::argument::arguments argumentsValues(buffer_);
         std::string stringValues = GetArgument("values").as_string();
         auto result_ = psql_->AddValues( stringValues, gd::types::tag_json{} );
         if( result_.first == false ) { return { false, "failed to add values for code execution: " + result_.second }; }
      }
      */
      return { true, "" };
   });

   if( result_.first == false ) { return result_; }

   if( GetContext()->IsStatusAbort() == true ) { return { true, "abort" }; } // check for aborting ?

   return { true, "" };
}