#include <filesystem>

#include "gd/parse/gd_parse_json.h"
#include "gd/gd_database_sqlite.h"
#include "gd/gd_file.h"
#include "gd/database/gd_database_io.h"

#include "../service/SERVICE_SqlBuilder.h"

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

      if( stringCommand == "create" ) { result_ = Execute_Create(); }         // endpoint db/create
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

      if( result_.first == false ) { return result_; }

#ifndef NDEBUG
      auto uObjectCount_d = m_objects.Size();
#endif // NDEBUG
      
      if( m_objects.Empty() == false ) { m_objects["command"] = stringCommand; }
   }

   SetCommandIndex( static_cast<unsigned>( uIndex ) );


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
   CDocument* pdocument = m_papplication->DOCUMENT_Get(stringDocument, true);
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
      auto ptable_ = new gd::table::dto::table( gd::table::tag_full_meta{} );
      gd::database::to_table( pcursor.get(), ptable_ );
      m_objects.Add( ptable_ );
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

      m_objects.Add( parguments_ );
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

   std::array<std::byte, 128> buffer_;
   gd::argument::arguments argumentsKey( buffer_ );
   result_ = pdatabase->execute( stringExecute, [&argumentsKey]( const auto* parguments_ ){ argumentsKey = *parguments_; return true; });
   if( result_.first == false ) { return result_; }

   gd::variant variantInsertKey;
   if( argumentsKey.empty() == false ) { variantInsertKey = argumentsKey[0u].as_variant(); }
   else { variantInsertKey = pdatabase->get_insert_key(); }

   // ## if not the last command in endpoint sequence then add to arguments as
   if( IsLastCommand() == false )
   {
      m_argumentsGlobal.set( "key", variantInsertKey.as_variant_view() );
   }

   if( argumentsKey.empty() == false )
   {
      gd::argument::arguments* parguments_ = new gd::argument::arguments( argumentsKey );
      m_objects.Add( parguments_ );
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
   m_objects.Add( parguments_ );


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
   m_objects.Add( parguments_ );
   

   return { true, "" };
}

/** --------------------------------------------------------------------------
 * Prepare SQL statement for execution.
 * @param stringSql SQL statement to prepare.
 * @param "values" Values for query template, these are inserted into the query template.
 * @param "query" Query template to execute
 * @return A pair containing a boolean indicating success and a string containing the error message if any.
 */
std::pair<bool, std::string> CAPIDatabase::Sql_Prepare(std::string& stringSql)
{
   CDocument* pdocument = GetDocument();                                                           assert( pdocument != nullptr );
   CSqlBuilder sqlbuilder;
   std::string stringQueryTemplate;
   
   if( Exists("values") == true )
   {
      std::string stringValues = GetArgument("values").as_string();
      gd::argument::shared::arguments argumentsValues;
      argumentsValues.reserve( 128 );
      auto result_ = gd::parse::json::parse_shallow_object_g( stringValues, argumentsValues );
      if( result_.first == false ) return result_;

      if( m_argumentsGlobal.empty() == false )
      {
         for( const auto [key_, value_] : m_argumentsGlobal.named() )
         {
            std::string stringKey("::");
            stringKey += key_;
            argumentsValues.append_argument( stringKey, value_, gd::types::tag_view{});
         }
      }

      sqlbuilder = argumentsValues;
   }
   
   auto uIndex = GetArgumentIndex( "query" );
   if( uIndex == 0 ) stringQueryTemplate = GetArgument("query").as_string();
   else
   {
      stringQueryTemplate = (*this)[{"query", uIndex}].as_string();
   }
   
   if( stringQueryTemplate.empty() == true ) { return { false, "no query specified to execute" }; }

   if( stringQueryTemplate[0] == '#' )
   {
      // ## Get query based on name .........................................
      std::string_view stringQueryName( stringQueryTemplate.c_str() + 1, stringQueryTemplate.length() - 1 );
      META::CQueries* pqueries = pdocument->QUERIES_Get();

      auto result_ = pqueries->GetQuery( stringQueryName, stringQueryTemplate );
      if( result_.first == false ) { return result_; }
   }

   sqlbuilder = stringQueryTemplate;                                         // assign to template
   
   std::string stringExecute;
   auto result_ = sqlbuilder.Build( stringExecute );
   if( result_.first == false ) { return result_; }

   IncrementArgumentCounter( "query" );
   
   stringSql = std::move(stringExecute);

   return { true, "" };
}

// GetDocument is now implemented in CAPI_Base
