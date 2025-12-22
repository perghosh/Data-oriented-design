#include <filesystem>

#include "gd/gd_database_sqlite.h"
#include "gd/gd_file.h"
#include "gd/database/gd_database_io.h"

#include "../Router.h"
#include "../Document.h"
#include "../Application.h"

#include "APIDatabase.h"


void CAPIDatabase::common_construct( const CAPIDatabase& o )
{
    m_vectorCommand = o.m_vectorCommand;
    m_argumentsParameter = o.m_argumentsParameter;
}

void CAPIDatabase::common_construct( CAPIDatabase&& o ) noexcept
{
    m_vectorCommand = std::move( o.m_vectorCommand );
    m_argumentsParameter = std::move( o.m_argumentsParameter );
}

/** --------------------------------------------------------------------------
 * @brief Returns the number of times a given argument name appears in the command list.
 * 
 * Note that it is possible to have multiple occurrences of the same command and in order to match
 * arguments correctly this method counts how many times the specified argument name appears from
 * the active command index.
 * 
 * @param stringName The name of the argument to search for.
 * @return The number of occurrences of the specified argument name in the command list.
 */
size_t CAPIDatabase::GetArgumentIndex( const std::string_view& stringName ) const 
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   size_t uCount = 0;
   for( unsigned uIndex = 0; uIndex < m_uCommandIndex; ++uIndex )
   {
      std::string_view stringCommand = m_vectorCommand[uIndex];
      if( stringCommand == stringName ) { ++uCount; }
   }
   return uCount;
}

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

   CRouter::Encode_s( m_argumentsParameter, { "query" } );

   for( std::size_t uIndex = 0; uIndex < m_vectorCommand.size(); ++uIndex )
   {
      m_uCommandIndex = static_cast<unsigned>( uIndex );
      std::string_view stringCommand = m_vectorCommand[uIndex];

      if( stringCommand == "db" ) continue;

      if( stringCommand == "create" )
      {
         result_ = Execute_Create();
      }
      else if( stringCommand == "open" )
      {
         result_ = Execute_Open();
      }
      else if( stringCommand == "query" )
      {
         result_ = Execute_Query();
      }
      else if( stringCommand == "select" )
      {
         result_ = Execute_Select();
      }
      else if( stringCommand == "insert" )
      {
         result_ = Execute_Insert();
      }
      else if( stringCommand == "delete" )
      {
      }
      else if( stringCommand == "drop" )
      {
      }
      else
      {
         return { false, "unknown database command: " + std::string(stringCommand) };
      }

      if( result_.first == false ) { return result_; }
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
	CDocument* pdocument = m_pApplication->DOCUMENT_Get(stringDocument, true);
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
 * CAPIDatabase dbCmd({}, {{"document", "mydoc"}, {"query", "SELECT * FROM mytable"}});
 * auto result = dbCmd.Execute_Select();
 * if(result.first) { ... }
 * @endcode
 */
std::pair<bool, std::string> CAPIDatabase::Execute_Select()
{
   gd::database::database_i* pdatabaseOpen = nullptr;

   CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   auto uIndex = GetArgumentIndex( "select" );
   std::string stringQuery = ( *this )[{"query", uIndex}].as_string();
   if( stringQuery.empty() == true ) { return { false, "no query specified to execute" }; }

   gd::com::pointer<gd::database::cursor_i> pcursor;
   pdatabase->get_cursor( &pcursor );

   std::pair< bool, std::string > pairReturn;   
   pairReturn = pcursor->open( stringQuery );

   // ## create table to hold select result

   if( pairReturn.first == true )
   {
      auto ptable_ = new gd::table::dto::table( gd::table::tag_full_meta{} );
      gd::database::to_table( pcursor.get(), ptable_ );
      m_objects.Add( ptable_ );
   }

   return pairReturn;
}

std::pair<bool, std::string> CAPIDatabase::Execute_Insert()
{
   gd::database::database_i* pdatabaseOpen = nullptr;

   CDocument* pdocument = GetDocument();
   if( pdocument == nullptr ) { return { false, GetLastError() }; }

   auto* pdatabase = pdocument->GetDatabase();
   if( pdatabase == nullptr ) return { false, "no database connection in document: " + std::string( pdocument->GetName() ) };

   std::string stringQuery = m_argumentsParameter["query"].as_string();
   if( stringQuery.empty() == true ) { return { false, "no query specified to execute" }; }

   auto result_ = pdatabase->execute( stringQuery );
   if( result_.first == false ) { return result_; }

   auto variantInsertKey = pdatabase->get_insert_key();


   return { true, "" };
}

/// @brief Retrieves the document associated with the current API database instance.
/// @return Pointer to the CDocument object.
/// @note Note that document returned need a name for document or it will return the "default" document.
CDocument* CAPIDatabase::GetDocument()
{                                                                                                  assert( m_pApplication != nullptr );
   std::string stringDocument = m_argumentsParameter[{ {"document"}, {"doc"} }].as_string();
   if( stringDocument.empty() == true ) stringDocument = "default";

   CDocument* pdocument = m_pApplication->DOCUMENT_Get( stringDocument );

   if( pdocument == nullptr ) { m_stringLastError = "document not found: " + stringDocument; } // generate error if document do not exists

   return pdocument;
}
