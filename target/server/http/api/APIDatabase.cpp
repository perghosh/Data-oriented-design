#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/parse/gd_parse_json.h"
#include "gd/gd_database_sqlite.h"
#include "gd/gd_file.h"
#include "gd/database/gd_database_io.h"

#include "gd/gd_sql_query.h"
#include "gd/gd_sql_query_builder.h"

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

#include "../service/SERVICE_SqlBuilder.h"
#include "../render/RENDERSql.h"

#include "../Router.h"
#include "../Document.h"
#include "../Application.h"

#include "APIDatabase.h"

using namespace SERVICE;


// common_construct methods are now implemented in CAPI_Base

// GetArgumentIndex is now implemented in CAPI_Base

/** --------------------------------------------------------------------------
 * @brief Executes the database command based on the command vector and parameters.
 *
 * This method processes the database command stored in m_vectorCommand and uses
 * the parameters in m_argumentsParameter to perform the requested operation.
 *
 * The method supports the following commands:
 * - "db create": Creates a new database (currently only SQLite is supported)
 *   Requires parameters:
 *     - "type": Database type (e.g., "sqlite")
 *     - "name": Database name/path
 * - "db delete": Deletes a database (not yet implemented)
 *
 * @return std::pair<bool, std::string> A pair containing:
 *         - bool: Success status (true if operation succeeded, false otherwise)
 *         - std::string: Error message if operation failed, empty string if succeeded
 *
 * @note The command vector must not be empty. The method asserts this condition.
 *
 * Example usage:
 * @code
 * // Create a new SQLite database
 * CAPIDatabase dbCmd({"db", "create"}, {{"type", "sqlite"}, {"name", "mydatabase"}});
 * auto result = dbCmd.Execute();
 * if(result.first) {
 *     // Database created successfully
 * } else {
 *     // Error occurred: result.second contains the error message
 * }
 * @endcode
 */
std::pair<bool, std::string> CAPIDatabase::Execute()
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   // ## execute database command based on m_vectorCommand and m_argumentsParameter

   if( m_vectorCommand.empty() == true ) return { true, "No commands"};

   std::pair<bool, std::string> result_(true,"");

   //CRouter::Encode_s( m_argumentsParameter, { "query", "values" } );
   std::size_t uIndex = m_uCommandIndex;
   for( ; uIndex < m_vectorCommand.size(); ++uIndex )
   {
      m_uCommandIndex = static_cast<unsigned>( uIndex );
      std::string_view stringCommand = m_vectorCommand[uIndex];

      if( stringCommand == "db" ) continue;                                   // skip db

      SetCommand( stringCommand );                                            // set current command being processed, this is the command at m_uCommandIndex in m_vectorCommand

      if( stringCommand == "execute" ) { result_ = Execute_Execute(); }       // endpoint db/execute
      else if( stringCommand == "create" ) { result_ = Execute_Create(); }    // endpoint db/create
      else if( stringCommand == "open" ) { result_ = Execute_Open(); }        // endpoint db/open
      else if( stringCommand == "query" ) { result_ = Execute_Query(); }      // endpoint db/query
      else if( stringCommand == "select" ) { result_ = Execute_Select(); }    // endpoint db/select
      else if( stringCommand == "ask" ) { result_ = Execute_Ask(); }          // endpoint db/select
      else if( stringCommand == "insert" ) { result_ = Execute_Insert(); }    // endpoint db/insert
      else if( stringCommand == "update" ) { result_ = Execute_Update(); }    // endpoint db/update
      else if( stringCommand == "delete" ) { result_ = Execute_Delete(); }    // endpoint db/delete
      else if( stringCommand == "drop" )
      {
      }
      else
      {
         return { false, "unknown database command: " + std::string(stringCommand) };
      }

      if( result_.first == false ) 
      {                                                                                           LOG_ERROR( "DB error: " & result_.second );
         return result_; 
      }

#ifndef NDEBUG
      auto uObjectCount_d = Objects().Size();
#endif // NDEBUG
      
      if( Objects().Empty() == false ) { Objects()["command"] = stringCommand; }
   }

   SetCommandIndex( static_cast<unsigned>( uIndex ) );


   return { true, "" };
}

std::pair<bool, std::string> CAPIDatabase::Execute_Execute()
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsOptions(buffer_);

   std::string stringName = GetParameterArguments()["name"].as_string();
   std::string stringFormat = GetParameterArguments()["format"].as_string();
   CDocument* pdocument = GetDocument();

   if( Exists( "xml" ) )
   {
      std::string stringTable = GetParameterArguments()["table"].as_string();
      if( stringTable.empty() == false ) { argumentsOptions["table"] = stringTable; }

      argumentsOptions["form"] = "attribute";
      gd::argument::arguments argumentsReturn;
      argumentsReturn.reserve( 64 );
      pugi::xml_document* pxmldocument = GetParameterArguments()["xml"].get_pointer<pugi::xml_document>(); // get pointer to xml pointer that is prepared
      XML_BulkInsert( argumentsOptions, pxmldocument, pdocument, &argumentsReturn );
      Objects().Add( argumentsReturn );
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Creates a new database based on the parameters in m_argumentsParameter.
 *
 * This method creates a new SQLite database with the name specified in the
 * "name" parameter. The database type can be specified with the "type" parameter,
 * but currently only "sqlite" type is supported.
 *
 * If the database name doesn't have an extension, ".sqlite" is automatically added.
 * If the path is not absolute, it's converted to an absolute path.
 * The method checks if the database file already exists and returns an error if it does.
 *
 * @return std::pair<bool, std::string> A pair containing:
 *         - bool: Success status (true if database was created successfully, false otherwise)
 *         - std::string: Error message if creation failed, empty string if succeeded
 *
 * Example usage:
 * @code
 * // Create a new SQLite database
 * CAPIDatabase dbCmd({}, {{"type", "sqlite"}, {"name", "mydatabase"}});
 * auto result = dbCmd.Execute_Create();
 * if(result.first) {
 *     // Database created successfully
 * } else {
 *     // Error occurred: result.second contains the error message
 * }
 * @endcode
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Create()
{
   std::string stringType = m_argumentsParameter["type"].as_string();
   std::string stringName = m_argumentsParameter["name"].as_string();

   if( stringType.empty() == true || stringType == "sqlite" )
   {
      // ## create sqlite database with name

      // ### Check the file name for sqlite database so it do not exists
      std::filesystem::path pathFile( stringName );
      if( pathFile.has_extension() == false ) { pathFile += ".sqlite"; }

      auto result_ = gd::file::file_absolute_g( pathFile.string(), stringName );
		if(result_.first == false) { return { false, "failed to get absolute path for database file: " + result_.second }; }

      if(std::filesystem::exists(stringName) == true) { return { false, "database file already exists: " + stringName }; }

		// ### Create sqlite database

      gd::database::sqlite::database databaseCreate;
		result_ = databaseCreate.open(stringName, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX);
		if(result_.first == false) { return { false, "failed to create sqlite database: " + result_.second }; }

		databaseCreate.close();
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Connects to an existing database.
 *
 * This method is intended to establish a connection to an existing database.
 * Currently, this method is not implemented and always returns a success status.
 *
 * @return std::pair<bool, std::string> A pair containing:
 *         - bool: Success status (currently always returns true)
 *         - std::string: Error message (currently always empty)
 *
 * @note This method is not yet implemented.
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Open()
{
   gd::database::database_i* pdatabaseOpen = nullptr;
   std::string stringType = m_argumentsParameter["type"].as_string();
   std::string stringName = m_argumentsParameter["name"].as_string();

   std::string stringDocument = m_argumentsParameter[{ {"document"}, {"doc"} }].as_string();

	if(stringDocument.empty() == true) stringDocument = "default";

   // ## Check if document already is connected to database
   CDocument* pdocument = GetApplication()->DOCUMENT_Get(stringDocument, true);
   if( pdocument->IsDatabaseOpen() == true ) { return { false, "document already connected to database" }; }

   if(stringType.empty() == true || stringType == "sqlite")
   {
      std::filesystem::path pathFile(stringName);
      if(pathFile.has_extension() == false) { pathFile += ".sqlite"; }

      auto result_ = gd::file::file_absolute_g(pathFile.string(), stringName);
      if(result_.first == false) { return { false, "failed to get absolute path for database file: " + result_.second }; }

		gd::argument::arguments argumentsOpen;
		argumentsOpen.push_back({ "name", stringName });
      argumentsOpen.push_back({ "type", std::string_view("sqlite") });


		result_ = CApplication::OpenDatabase_s(argumentsOpen, pdatabaseOpen);
      if(result_.first == false) { return result_; }
                                                                                                   LOG_INFORMATION_RAW( "Opened database: " & stringName );
   }
                                                                                                   assert(pdatabaseOpen != nullptr);
	pdocument->SetDatabase(pdatabaseOpen);

   pdatabaseOpen->release();

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Executes a SQL query on the specified database.
 *
 * This method retrieves the database connection from the specified document
 * and executes the SQL query provided in the "query" parameter of m_argumentsParameter.
 *
 * @return std::pair<bool, std::string>  A pair where the bool indicates success/failure
 *
 * Example usage:
 * @code
 * // Execute a SQL query
 * CAPIDatabase dbCmd({}, {{"document", "mydoc"}, {"query", "SELECT * FROM mytable"}});
 * auto result = dbCmd.Execute_Query();
 * if(result.first) { ... }
 * @endcode
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Query()
{
   gd::database::database_i* pdatabaseOpen = nullptr;

   CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();                                // get database from document, this connection has to be opened before
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   std::string stringQuery = m_argumentsParameter["query"].as_string();       // get query to execute
   if( stringQuery.empty() == true ) { return { false, "no query specified to execute" }; }

   auto result_ = pdatabase->execute( stringQuery );                          // execute query on database
   if( result_.first == false ) { return result_; }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Executes a SELECT SQL query and retrieves the results.
 *
 * This method retrieves the database connection from the specified document
 * and executes the SELECT SQL query provided in the "query" parameter of m_argumentsParameter.
 * The results of the query are converted into a table format.
 *
 * @return std::pair<bool, std::string> A pair where the bool indicates success/failure
 *
 * Example usage:
 * @code
 * // Execute a SELECT SQL query
 * CAPIDatabase dbCmd({}, {{"document", "mydoc"}, {"select", "SELECT * FROM mytable"}});
 * auto result = dbCmd.Execute_Select();
 * if(result.first) { ... }
 * @endcode
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Select()
{
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsOptional( buffer_ );
   gd::database::database_i* pdatabaseOpen = nullptr;

   CDocument* pdocument = GetDocument();                                                           if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   // ## Prepare SQL statement ................................................
   std::string stringSelect;
   auto result_ = Sql_Prepare(stringSelect, argumentsOptional);
   if( result_.first == false ) { return result_; }

   gd::com::pointer<gd::database::cursor_i> pcursor;
   pdatabase->get_cursor( &pcursor );

   std::pair< bool, std::string > pairReturn;   
   pairReturn = pcursor->open( stringSelect );

   // ## create table to hold select result

   if( pairReturn.first == true )
   {
      auto ptable_ = new gd::table::dto::table( gd::table::tag_full_meta{} );
      gd::database::to_table( pcursor.get(), ptable_ );
#ifndef NDEBUG
      // std::string stringTable_d = gd::table::debug::print( *ptable_ );
      // std::string stringTableHex_d = gd::binary_to_hex_g( stringTable_d );
#endif // NDEBUG
      Objects().Add( ptable_ );

      if( argumentsOptional["name"].is_true() == true )
      {                                                                                            assert( argumentsOptional["name"].is_string() == true && "name argument must be a string" );
         auto stringName = argumentsOptional["name"].as_string();
         Objects()["name"] = stringName;

         // ## Check for ui information
         META::CQueries* pqueries = pdocument->QUERIES_Get();
         auto iRow = pqueries->GetQueryRow( stringName );                    assert( iRow != -1 && "query name not found in document queries" );
         if( iRow != -1 )
         {
            auto parguments_ = pqueries->GetQueryArguments( static_cast<uint64_t>( iRow ) );
            if( parguments_ != nullptr )
            {
               for( auto [key_, value_] : parguments_->named() ) 
               { 
                  Objects()[key_] = value_.as_string();         
               }
            }
         }
      }
   }

   return pairReturn;
}

std::pair<bool, std::string> CAPIDatabase::Execute_Ask()
{
   gd::database::database_i* pdatabaseOpen = nullptr;

   CDocument* pdocument = GetDocument();                                                           if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   // ## Prepare SQL statement ................................................
   std::string stringSelect;
   auto result_ = Sql_Prepare(stringSelect);
   if( result_.first == false ) { return result_; }

   gd::com::pointer<gd::database::cursor_i> pcursor;
   pdatabase->get_cursor( &pcursor );

   std::pair< bool, std::string > pairReturn;   
   pairReturn = pcursor->open( stringSelect );

   // ## create table to hold select result

   if( pairReturn.first == true )
   {
      gd::table::dto::table table_( gd::table::tag_full_meta{} );
      gd::database::to_table( pcursor.get(), &table_ );

      gd::argument::arguments* parguments_ = new gd::argument::arguments();

      if( table_.size() > 0 )
      {
         table_.row_get_arguments( 0u, *parguments_ );
      }

      Objects().Add( parguments_ );
   }

   return pairReturn;
}

/** ---------------------------------------------------------------------------
 * @brief Execute an insert query.
 * 
 * @param "id" Id for query template
 * @param "values" Values for query template, these are inserted into the query template.
 * @param "query" Query template to execute
 * @return A pair containing a boolean indicating success and a string containing the error message if any.
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Insert()
{
   gd::database::database_i* pdatabaseOpen = nullptr;

   CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   // ## Prepare SQL statement ................................................
   std::string stringExecute;
   auto result_ = Sql_Prepare(stringExecute);
   if( result_.first == false ) { return result_; }
                                                                                                   //LOG_DEBUG_RAW( "SQL-INSERT: " & stringExecute.substr( 0, 128 ) );
   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsKey( buffer_ );
   result_ = pdatabase->execute( stringExecute, [&argumentsKey]( const auto* parguments_ ){ argumentsKey = *parguments_; return true; });
   if( result_.first == false ) { return result_; }

   gd::variant variantInsertKey;
   if( argumentsKey.empty() == false ) { variantInsertKey = argumentsKey[0u].as_variant(); }
   else { variantInsertKey = pdatabase->get_insert_key(); }

   // ## if not the last command in endpoint sequence then add to arguments as
   if( IsLastCommand() == false ) { SetGlobal( "key", variantInsertKey.as_variant_view() ); } // "key" is default name for geneated key values if not specified. maybe this need to be changed

   if( argumentsKey.empty() == false )
   {
      gd::argument::arguments* parguments_ = new gd::argument::arguments( argumentsKey );
      Objects().Add( parguments_ );
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Execute an update query.
 * 
 * @param "id" Id for query template
 * @param "values" Values for query template, these are inserted into the query template.
 * @param "query" Query template to execute
 * @return A pair containing a boolean indicating success and a string containing the error message if any.
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Update()
{
   CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   // ## Prepare SQL statement ................................................
   std::string stringExecute;
   auto result_ = Sql_Prepare(stringExecute);
   if( result_.first == false ) { return result_; }

   result_ = pdatabase->execute( stringExecute );
   if( result_.first == false ) { return result_; }

   // ## Find out number of rows affected ....................................
   gd::variant vChangeCount = pdatabase->get_change_count();
   
   gd::argument::arguments* parguments_ = new gd::argument::arguments( {{ "count", vChangeCount.as_int64() }} );
   Objects().Add( parguments_ );


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Execute a delete query.
 * 
 * @param "id" Id for query template
 * @param "values" Values for query template, these are inserted into the query template.
 * @param "query" Query template to execute
 * @return A pair containing a boolean indicating success and a string containing the error message if any.
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Delete()
{
   CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   // ## Prepare SQL statement ................................................
   std::string stringExecute;
   auto result_ = Sql_Prepare(stringExecute);
   if( result_.first == false ) { return result_; }

   result_ = pdatabase->execute( stringExecute );
   if( result_.first == false ) { return result_; }
   
   // ## Find out number of rows affected ....................................
   gd::variant vChangeCount = pdatabase->get_change_count();
   
   gd::argument::arguments* parguments_ = new gd::argument::arguments( {{ "count", vChangeCount.as_int64() }} );
   Objects().Add( parguments_ );
   

   return { true, "" };
}

/** --------------------------------------------------------------------------
 * Prepare SQL statement for execution.
 * @param stringSql SQL statement to prepare.
 * @param "values" Values for query template, these are inserted into the query template.
 * @param "query" Query template to execute
 * @return A pair containing a boolean indicating success and a string containing the error message if any.
 */
std::pair<bool, std::string> CAPIDatabase::Sql_Prepare(std::string& stringSql, gd::argument::arguments& argumentsOptional )
{
   std::pair<bool, std::string> result_( true, "" );
   CDocument* pdocument = GetDocument();                                                           assert( pdocument != nullptr );
   CSqlBuilder sqlbuilder;
   std::string stringQueryTemplate;
   auto uDialect = pdocument->DATABASE_Get()->GetDialect(); // get database dialect to use for sql building

   if( Exists( "xml" ) == true )
   {
      auto stringCommand = GetCommand();                                      // get current command being processed, this is the command at m_uCommandIndex and should match sql statements like select, insert, update or delete
      CRENDERSql sql_( pdocument, gd::sql::enumSqlDialect(uDialect) );
      sql_.Initialize();
      pugi::xml_document* pdocument = reinterpret_cast<pugi::xml_document*>( GetArgument("xml").as_void() );

      // ## Query values in pdocument xml using xpath ........................
      pugi::xpath_node_set xpathnodesetValues = pdocument->select_nodes("//values");
      auto uIndex = GetArgumentIndex( "xml" );
      
      // ### Get values at index
      if( uIndex < xpathnodesetValues.size() )
      {
         // #### values element with elements used to build sql statement
         pugi::xml_node xmlnodeValues = xpathnodesetValues[uIndex].node();
         if( xmlnodeValues.first_child() ) 
         { 
            result_ = sql_.Add( xmlnodeValues );                                                   if( result_.first == false ) { return result_; } // add values
         }
         else if( xmlnodeValues.type() == pugi::node_cdata )
         {
            // #### Get CDATA values if no child nodes
            gd::argument::shared::arguments argumentsValues;
            argumentsValues.reserve( 128 );
            std::string stringValues = xmlnodeValues.child_value();
            auto result_ = gd::parse::json::parse_shallow_object_g( stringValues, argumentsValues, false );
            if( result_.first == false ) return result_;
         }

         result_ = sql_.Prepare();                                                                 if( result_.first == false ) { return result_; }

         // #### Prepare SQL statement based on command
         std::string stringSql;
         stringSql.reserve( 128 );
         result_ = sql_.ToSql( stringCommand, stringSql );                                         if( result_.first == false ) { return result_; }

         sqlbuilder = stringSql;
         sqlbuilder.SetType( stringCommand );
      }

      IncrementArgumentCounter( "xml" );                                     // increamet index for xml argument to support multiple xml arguments 
   }
   else if( Exists( "json" ) == true )
   {

   }
   else if( Exists("values") == true )
   {
      std::string stringValues = GetArgument("values").as_string();
      gd::argument::shared::arguments argumentsValues;
      argumentsValues.reserve( 128 );
      auto result_ = gd::parse::json::parse_shallow_object_g( stringValues, argumentsValues, false );
      if( result_.first == false ) return result_;

      if( IsGlobalEmpty() == false )
      {
         for( const auto [key_, value_] : GetGlobalArguments().named() )
         {
            std::string stringKey("::");
            stringKey += key_;
            argumentsValues.append_argument( stringKey, value_, gd::types::tag_view{});
         }
      }

      sqlbuilder = argumentsValues;
   }
   else if( Exists( "record" ) == true )
   {
      std::string stringRecord = GetArgument("record").as_string();
      IncrementArgumentCounter( "record" );                                   // @TODO: this is a bit hacky way to support multiple query arguments but it works for now, can be improved (refactor to one single method)

      if( GetCommand() == "insert" )
      {
         sqlbuilder.SetType( CSqlBuilder::eTypeInsert );
         CRENDERSql sql_( pdocument, gd::sql::enumSqlDialect(uDialect) );
         sql_.Initialize();
         result_ = sql_.AddRecord( stringRecord, gd::types::tag_json{});      // add record formated as json
         if( result_.first == false ) { return result_; }

         result_ = sql_.Prepare();
         if( result_.first == false ) { return result_; }

         std::string stringQuery;
         result_ = sql_.ToSqlInsert( stringQuery );
         if( result_.first == false ) { return result_; }

         sqlbuilder = stringQuery;
      }
      else if( GetCommand() == "update" )
      {
         sqlbuilder.SetType( CSqlBuilder::eTypeUpdate );
         CRENDERSql sql_( pdocument, gd::sql::enumSqlDialect(uDialect) );
         sql_.Initialize();
         result_ = sql_.AddRecord( stringRecord, gd::types::tag_json{} );     // add record formated as json
         if( result_.first == false ) { return result_; }

         if( sql_.CountPartType( CRENDERSql::ePartTypeWhere ) == 0 ) { return { false, "Invalid update: missing WHERE clause" }; }

         result_ = sql_.Prepare();

         std::string stringQuery;
         result_ = sql_.ToSqlUpdate( stringQuery );
         if( result_.first == false ) { return result_; }
         sqlbuilder = stringQuery;
      }
      else if( GetCommand() == "delete" )
      {
         sqlbuilder.SetType( CSqlBuilder::eTypeDelete );
         CRENDERSql sql_( pdocument, gd::sql::enumSqlDialect(uDialect) );

         sql_.Initialize();
         result_ = sql_.AddRecord( stringRecord, gd::types::tag_json{} );     // add record formated as json
         if( result_.first == false ) { return result_; }

         if( sql_.CountPartType( CRENDERSql::ePartTypeWhere ) == 0 ) { return { false, "Invalid update: missing WHERE clause" }; }

         result_ = sql_.Prepare();

         std::string stringQuery;
         result_ = sql_.ToSqlDelete( stringQuery );
         if( result_.first == false ) { return result_; }
         sqlbuilder = stringQuery;
      }
      else
      {
         return { false, "record parameter only supported for insert and update commands" };
      }
   }
   
   // ## if sqlbuilder is not ready then try to build it from query template and values
   if( sqlbuilder.IsSqlReady() == false )
   {
      auto uIndex = GetArgumentIndex( "query" );
      if( uIndex == 0 ) stringQueryTemplate = GetArgument("query").as_string();
      else { stringQueryTemplate = (*this)[{"query", uIndex}].as_string(); }
      IncrementArgumentCounter( "query" );                                    // @TODO: this is a bit hacky way to support multiple query arguments but it works for now, can be improved (refactor to one single method)
   
      if( stringQueryTemplate.empty() == true ) { return { false, "no query specified to execute" }; }

      if( stringQueryTemplate[0] == '#' )
      {
         // ## Get query based on name .........................................
         std::string_view stringQueryName( stringQueryTemplate.c_str() + 1, stringQueryTemplate.length() - 1 );
         argumentsOptional.push_back( { "name", stringQueryName } );          // add query name used to access query

         META::CQueries* pqueries = pdocument->QUERIES_Get();

         result_ = pqueries->GetQuery( stringQueryName, stringQueryTemplate );
         if( result_.first == false ) { return result_; }
      }

      sqlbuilder = stringQueryTemplate;                                       // assign to template
   }
   
   std::string stringExecute;

   if( sqlbuilder.IsSqlReady() == false )
   {
      result_ = sqlbuilder.Build( stringExecute );                            // @CRITICAL: [tag: sql] [description: Build SQL statement from template and values]
      if( result_.first == false ) { return result_; }
   }
   else
   {
      stringExecute =std::move( sqlbuilder.GetSql() );
   }

#if (TARGET_COMPILE_MODE_ & 1)
   if( gd::utf8::validate_ascii( stringExecute ).first == true ) { 
      LOG_VERBOSE_RAW( "Prepared SQL: " & gd::utf8::substr(stringExecute, 120) & "\n");
   }
#endif


   stringSql = std::move(stringExecute);

   return { true, "" };
}


std::pair<bool, std::string> CAPIDatabase::XML_BulkInsert( const gd::argument::arguments& argumentsOptions, pugi::xml_document* pxmldocument, CDocument* pdocument, gd::argument::arguments* pargumentsReturn )
{
   using namespace gd::sql;
   std::array<char, 128> buffer_; // buffer to avoid allocate memory
   uint64_t uInsertCount = 0;

   META::CDatabase* pdatabase_ = pdocument->DATABASE_Get();
   auto uDialect = pdatabase_->GetDialect();

   std::string stringForm = argumentsOptions["form"].as_string(); // layout is required and should be string
   std::string stringContainer = argumentsOptions["container"].as_string(); // container is required and should be string

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };


   if( stringContainer.empty() == true ) { stringContainer = "//values"; }

   if( stringForm == "attribute" )
   {
      // ## xml form is like <values column1="value1" column2="value2" />

      std::string stringTable = argumentsOptions["table"].as_string(); // table is required and should be string  
      if( stringTable.empty() == true ) { return { false, "table name is required for attribute form" }; }

      // ### Loop elements in container
      pugi::xpath_node_set xpathnodesetValues = pxmldocument->select_nodes(stringContainer.c_str());
      for( auto& xpathnode_ : xpathnodesetValues )
      {
         query queryInsert{ enumSqlDialect( uDialect ) };

         queryInsert << table_g( stringTable, buffer_ );                     // set table for insert query

         pugi::xml_node xmlnodeValue = xpathnode_.node();
         for( auto& xmlattribute_ : xmlnodeValue.attributes() )              // loop attributes in element and add attribute name and values to query
         {
            std::string_view stringName = xmlattribute_.name();
            std::string_view stringValue = xmlattribute_.value();

            gd::argument::arguments argumentsFind( buffer_ );
            argumentsFind.append( { {std::string_view("table"), gd::variant_view(stringTable)}, {std::string_view("column"), gd::variant_view(stringName)} }, gd::types::tag_view{});
            int64_t iRow = pdatabase_->Column_FindRow( argumentsFind ); 
            if( iRow == -1 ) { return { false, "column not found in database: " + std::string(stringName) }; }

            auto uType = pdatabase_->Column_GetType( iRow );
            queryInsert << field_g( stringName, buffer_ ).value( stringValue ).type( uType );
         }

         std::string stringInsertSql = queryInsert.sql_get( eSqlInsert );
         auto result_ = pdatabase->execute( stringInsertSql );
         if( result_.first == false ) { return result_; }
         uInsertCount++;
      }
   }
   else
   {
      // ## xml form is like <values><value element="name" value="value" /></values> 
      std::string stringElement = argumentsOptions["element"].as_string();   // element name for columns
      std::string stringValue = argumentsOptions["value"].as_string();       // value attribute name
      std::string stringTable = argumentsOptions["table"].as_string();       // table name for insert

      if( stringTable.empty() == true ) { return { false, "table name is required for element form" }; }
      if( stringElement.empty() == true ) { stringElement = "element"; }     // default element attribute
      if( stringValue.empty() == true ) { stringValue = "value"; }           // default value attribute

      // ### Loop container elements (each represents one row)
      pugi::xpath_node_set xpathnodesetValues = pxmldocument->select_nodes(stringContainer.c_str());
      for( auto& xpathnode_ : xpathnodesetValues )
      {
         query queryInsert{ enumSqlDialect( uDialect ) };
         queryInsert << table_g( stringTable, buffer_ );                     // set table for insert query

         pugi::xml_node xmlnodeValues = xpathnode_.node();
         for( auto& xmlnodeValue_ : xmlnodeValues.children() )               // loop child elements (each is a column)
         {
            if( xmlnodeValue_.name() == nullptr || stringElement != xmlnodeValue_.name() ) continue; // skip if no element name

            std::string_view stringName = xmlnodeValue_.attribute(stringElement.c_str()).value();
            std::string_view stringFieldValue = xmlnodeValue_.attribute(stringValue.c_str()).value();

            if( stringName.empty() == true ) continue;                       // skip if no element name

            gd::argument::arguments argumentsFind( buffer_ );
            argumentsFind.append( { {std::string_view("table"), gd::variant_view(stringTable)}, {std::string_view("column"), gd::variant_view(stringName)} }, gd::types::tag_view{});
            int64_t iRow = pdatabase_->Column_FindRow( argumentsFind );
            if( iRow == -1 ) { return { false, "column not found in database: " + std::string(stringName) }; }

            auto uType = pdatabase_->Column_GetType( iRow );
            queryInsert << field_g( stringName, buffer_ ).value( stringFieldValue ).type( uType );
         }

         std::string stringInsertSql = queryInsert.sql_get( eSqlInsert );
         auto result_ = pdatabase->execute( stringInsertSql );
         if( result_.first == false ) { return result_; }
         uInsertCount++;
      }
   }

   if( pargumentsReturn != nullptr ) { pargumentsReturn->push_back( { "count", gd::variant_view(uInsertCount) } ); }

   return { true, "" };
}

// GetDocument is now implemented in CAPI_Base
