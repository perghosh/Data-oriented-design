// @FILE [tag: api, system] [summary: API System command class] [type: source] [name: APISystem.cpp]

#include <filesystem>

#include "gd/gd_arguments.h"
#include "gd/gd_binary.h"
#include "gd/gd_uuid.h"
#include "gd/gd_table_column-buffer.h"


#include "../Router.h"
#include "../Document.h"
#include "../Application.h"

#include "APISystem.h"

std::pair<bool, std::string> ValidateSession_s(const std::string& stringSession);

std::pair<bool, std::string> CAPISystem::Execute()
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   // ## execute database command based on m_vectorCommand and m_argumentsParameter

   if( m_vectorCommand.empty() == true ) return { true, "No commands"};

   std::pair<bool, std::string> result_(true,"");

   CRouter::Encode_s( m_argumentsParameter, { "query" } );

   for( std::size_t uIndex = m_uCommandIndex; uIndex < m_vectorCommand.size(); ++uIndex )
   {
      std::string_view stringCommand = m_vectorCommand[uIndex];

      if( stringCommand == "sys" ) continue;

      if( stringCommand == "file" )
      {
         uIndex++;
         for( ; uIndex < m_vectorCommand.size(); uIndex++ && result_.first == true )
         {
            stringCommand = m_vectorCommand[uIndex];

            if( stringCommand == "delete" )        { result_ = Execute_FileDelete(); }
            else if( stringCommand == "directory" ){ result_ = Execute_FileDirectory(); }
            else if( stringCommand == "exists" )   { result_ = Execute_FileExists(); }
            else break;

            if( m_objects.Empty() == false ) { m_objects["command"] = stringCommand; }
         }
      }
      else if( stringCommand == "meta" )
      {
         uIndex++;
         if( uIndex >= m_vectorCommand.size() ) return { false, "Missing session command" };
         stringCommand = m_vectorCommand[uIndex];

         if(stringCommand == "query" || stringCommand == "sql")               // meta/query|sql
         {
            uIndex++;
            for( ; uIndex < m_vectorCommand.size(); uIndex++ && result_.first == true )
            {
               stringCommand = m_vectorCommand[uIndex];

               if( stringCommand == "add" )        { result_ = Execute_MetadataQueryAdd(); }       // add
               else if( stringCommand == "delete" ){ result_ = Execute_MetadataQueryDelete(); }    // delete
               else if( stringCommand == "exists" ){ result_ = Execute_MetadataQueryExists(); }    // exists
               else break;

               if( m_objects.Empty() == false ) { m_objects["command"] = stringCommand; }
            }
         }
         else if(stringCommand == "db")
         {
            uIndex++;
            for( ; uIndex < m_vectorCommand.size(); uIndex++ && result_.first == true )
            {
               stringCommand = m_vectorCommand[uIndex];

               if( stringCommand == "fields" )     { result_ = Execute_MetadataDBField(); }
               else break;

               if( m_objects.Empty() == false ) { m_objects["command"] = stringCommand; }
            }
         }
      }
      else if( stringCommand == "session" )
      {
         uIndex++;
         for( ; uIndex < m_vectorCommand.size(); uIndex++ && result_.first == true )
         {
            stringCommand = m_vectorCommand[uIndex];

            if( stringCommand.empty() == true ) { break; }
         
            if( stringCommand == "add" )           { result_ = Execute_SessionAdd(); }
            else if( stringCommand == "count" )    { result_ = Execute_SessionCount(); }
            else if( stringCommand == "exists" )   { result_ = Execute_SessionExists(); }
            else if( stringCommand == "delete" )   { result_ = Execute_SessionDelete(); }
            else if( stringCommand == "list" )     { result_ = Execute_SessionList(); }
            else break;

            if( m_objects.Empty() == false ) { m_objects["command"] = stringCommand; }
         }
      }
      else
      {
         return { false, "unknown database command: " + std::string(stringCommand) };
      }

      SetCommandIndex( uIndex );

      if( result_.first == false ) { return result_; }

      if( stringCommand.empty() == true ) { break; }                         // if empty comand that means that we are going back to root
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------
 * Deletes a file
 */
std::pair<bool, std::string> CAPISystem::Execute_FileDelete()
{
   std::string stringPathFound;
   std::string stringPath = m_argumentsParameter["path"].as_string();
   if(stringPath.empty() == false)
   {
      if( std::filesystem::exists(stringPath) && std::filesystem::is_regular_file(stringPath) )
      {
         stringPathFound = stringPath;
      }
   }
   
   if( stringPathFound.empty() == false )
   {
      std::filesystem::remove(stringPathFound);
      gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "path", stringPathFound }, { "deleted", true } } ); // result data
      m_objects.Add( parguments_ );
   }
   
   return { true, "" };
}


/** --------------------------------------------------------------------------
 * Execute File Directory Command
 * 
 * The sys/file/directory command is used to perform actions on file directories.
 * 
 * @param stringAction The action to perform on the file directory
 */
std::pair<bool, std::string> CAPISystem::Execute_FileDirectory()
{
   std::string stringAction;

   if( m_argumentsParameter.exists( "action" ) == true ) { stringAction = m_argumentsParameter["action"].as_string(); }

   if( stringAction == "get" )
   {
      std::string stringType = m_argumentsParameter["type"].as_string();      // type of folder, "root", "application" etc
      std::string stringFolderType("folder-");
      stringFolderType += stringType;

      std::string stringDirectory = papplication_g->PROPERTY_Get( stringFolderType ).as_string();

      gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "directory", stringDirectory } } ); // result data
      m_objects.Add( parguments_ );
   }
   else if( stringAction == "set" )
   {
      std::string stringType = m_argumentsParameter["type"].as_string();
      std::string stringFolderType("folder-");
      stringFolderType += stringType;
      
      std::string stringValue = m_argumentsParameter.get_argument({"name", "value"}).as_string();
      papplication_g->PROPERTY_Set( stringFolderType, stringValue );          // Note that setting folder is not thread safe and should only be done in development mode or local on prem solutions
   }
   else
   {
      return { false, "invalid action: " + stringAction };
   }

   return { true, "" };
}

/**
 * Checks if a file exists
 */
std::pair<bool, std::string> CAPISystem::Execute_FileExists()
{
   std::string stringPathFound;
   std::string stringPath = m_argumentsParameter["path"].as_string();
   if(stringPath.empty() == false)
   {
      if( std::filesystem::exists(stringPath) )
      {
         stringPathFound = stringPath;
      }
   }
   
   if( stringPathFound.empty() == false )
   {
      gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "path", stringPathFound }, { "exists", true } } ); // result data
      m_objects.Add( parguments_ );
   }
   
   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Add
 * @return 
 */
std::pair<bool, std::string> CAPISystem::Execute_MetadataQueryAdd()
{
   std::string stringName = Get("name").as_string();      // Name for query
   std::string stringType = Get("type").as_string();      // type of query
   std::string stringFormat = Get("format").as_string();  // query format, type of format information is stored in like text, xml, json etc
   std::string stringQuery = Get("query").as_string();    // query

   CDocument* pdocument = GetDocument();
   META::CQueries* pqueries = pdocument->QUERIES_Get();                                            assert( pqueries );

   auto result_ = pqueries->Add( stringName, stringType, stringFormat, stringQuery );

   if( result_.first == true ) 
   {
      gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "id", result_.second } } ); // result data
      m_objects.Add( parguments_ );
   }

   return result_;
}

std::pair<bool, std::string> CAPISystem::Execute_MetadataQueryDelete()
{
   std::string stringId = Get("id").as_string();
   std::string stringKey = Get("key").as_string();

   CDocument* pdocument = GetDocument();
   META::CQueries* pqueries = pdocument->QUERIES_Get();                                            assert( pqueries );

   auto result_ = pqueries->Delete( {stringId, stringKey} );
   return result_;
}

/** --------------------------------------------------------------------------
 * @brief Checks if a metadata query with the specified name exists and stores the result.
 * 
 * @param "name" query name to check if it exists
 * 
 * @return A pair where the first element is true if the operation was successful, false otherwise. The second element is an error message if the operation failed
 */
std::pair<bool, std::string> CAPISystem::Execute_MetadataQueryExists()
{
   std::string stringName = Get("name").as_string();

   if( stringName.empty() == true ) { return { false, "Missing parameter 'name'" }; }

   CDocument* pdocument = GetDocument();
   META::CQueries* pqueries = pdocument->QUERIES_Get();                                            assert( pqueries );
   
   gd::argument::arguments arguments_( { { "name", stringName } } );
   int64_t iRow = pqueries->Find( arguments_ );

   gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "exists", iRow } } ); // result data
   if( iRow >= 0 )
   {
      auto uuid_ = pqueries->GetQueryId( iRow );
      std::string stringUuid;
      gd::binary_to_hex_g( uuid_, 16, stringUuid );
      parguments_->append( "id", stringUuid );
   }

   m_objects.Add( parguments_ );

   return { true, "" };
}

std::pair<bool, std::string> CAPISystem::Execute_MetadataDBField()
{
   using namespace gd::table::dto;
   std::string stringTable = Get( "table" ).as_string();                      // Table name field belongs to
   std::string stringField = Get( "field" ).as_string();                      // Return information about field or fields if comma separated list

   auto vectorField = gd::utf8::split( std::string_view( stringField ), ',' );

   if( stringTable.empty() == true || stringField.empty() == true ) return { false, "Missing parameter 'table' or 'field'" };

   // ## Build information for fields ........................................

   CDocument* pdocument = GetDocument();
   uint64_t uMaxTextLength = 0;
   const META::CDatabase* pdatabase_ =  pdocument->DATABASE_Get();
   auto result_ = pdatabase_->ComputeTextLength( stringTable, vectorField, &uMaxTextLength );
   if( result_.first == false ) { return result_; }

   // Columns for each field is table, name, alias, type, js type, description
   uMaxTextLength++; // add one for extra space
   table* ptable_ = new table( (table::eTableFlagNull32), { { "string", uMaxTextLength, "table"}, { "string", uMaxTextLength, "column"}, { "string", uMaxTextLength, "alias"}, { "uint32", 0, "type" }, { "uint32", 0, "size" } }, gd::table::tag_prepare{} );

   result_ = pdatabase_->ReadColumnMetadata( stringTable, vectorField, ptable_ );

#ifndef NDEBUG
   std::string stringTable_d = gd::table::debug::print( *ptable_ );
#endif // NDEBUG

   m_objects.Add( ptable_ );

   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Adds a session key
 * 
 * Add either by using a provided session string or by generating a new session, 
 * and returns the result of the operation.
 * 
 * @return A pair consisting of a boolean indicating success  or failure
 */
std::pair<bool, std::string> CAPISystem::Execute_SessionAdd()
{
   gd::types::uuid uuid; // uuid for session
   std::string stringSession; // session string read from request

   if( m_argumentsParameter.exists("new") == false )
   {
      stringSession = m_argumentsParameter["session"].as_string();    // get session to add

      if( stringSession.size() < 32 ) { stringSession.append( 32 - stringSession.size(), '0' ); } // Pad session if less than 32 bytes
   
      auto result_ = ValidateSession_s(stringSession);
      if( result_.first == false ) { return result_; }

      if( stringSession.length() == 32 ) gd::binary_copy_hex_g( uuid,  stringSession);
      else if( stringSession.length() == 36 ) gd::binary_copy_uuid_g( uuid, stringSession );
   }
   else
   { 
      uuid = gd::uuid( gd::types::tag_command_random{} );
      stringSession = gd::binary_to_hex_g( uuid.data(), 16, false );
   }
   
   CDocument* pdocument = GetDocument();
   
   uint64_t uIndex = pdocument->SESSION_Add(uuid);

   // ## return response with index for session added

   gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "index", uIndex } } );
   if( m_argumentsParameter.exists("new") == true ) { parguments_->append("session", stringSession); }
   m_objects.Add( parguments_ );
   
   return { true, "" };
}

/// @brief remove session by value or by index
std::pair<bool, std::string> CAPISystem::Execute_SessionDelete()
{
   if( Exists("session") == true )
   {
      std::string stringSession = m_argumentsParameter["session"].as_string(); // get session to delete
      
      // ## Pad session if less than 32 bytes and validate ...................
      if( stringSession.size() < 32 ) { stringSession.append(32 - stringSession.size(), '0'); }
      auto result_ = ValidateSession_s(stringSession);
      if( result_.first == false ) { return result_; }
      
      gd::types::uuid uuid;
      gd::binary_copy_hex_g( uuid, stringSession);
      
      CDocument* pdocument = GetDocument();
      
      pdocument->SESSION_Delete( uuid );
   }
   else if( Exists("index") == true )
   {
      uint64_t uIndex = m_argumentsParameter["index"].as_uint64();
      
      CDocument* pdocument = GetDocument();
      
      pdocument->SESSION_Delete( uIndex );
   }
   else { return { false, "Missing parameter 'session' or 'index'" }; }
   
   return { true, "" };
}

/** --------------------------------------------------------------------------
 * @brief Count number of active sessions
 * @return pair <bool, std::string>  A pair where the bool indicates success/failure
 */
std::pair<bool, std::string> CAPISystem::Execute_SessionCount()
{
   CDocument* pdocument = GetDocument();
   
   uint64_t uCount = pdocument->SESSION_Count();

   gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "count", uCount } } );
   m_objects.Add( parguments_ );
   
   return { true, "" };
}

/// @brief Check if session is found
std::pair<bool, std::string> CAPISystem::Execute_SessionExists()
{
   CDocument* pdocument = GetDocument();
   auto* psessions = pdocument->SESSION_Get();

   std::string stringSession = m_argumentsParameter["session"].as_string();   // session to check for

   // ## Pad session if less than 32 bytes
   if( stringSession.size() < 32 ) { stringSession.append(32 - stringSession.size(), '0'); }

   auto result_ = ValidateSession_s(stringSession);
   if( result_.first == false ) { return result_; }

   gd::types::uuid uuid;
   gd::binary_copy_hex_g( uuid, stringSession);

   int64_t iPosition = psessions->Find( uuid );

   gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "index", iPosition } } );
   m_objects.Add( parguments_ );

   return { true, "" };
}

/// @brief List all active sessions
std::pair<bool, std::string> CAPISystem::Execute_SessionList()
{
   CDocument* pdocument = GetDocument();
   auto* psessions = pdocument->SESSION_Get();

   gd::table::dto::table* ptable_ = new gd::table::dto::table();
   psessions->Copy( *ptable_ );
   m_objects.Add( ptable_ );

   // ## get list of sessions and place in in table object
   
   return { true, "" };
}


/// Validate session string
std::pair<bool, std::string> ValidateSession_s(const std::string& stringSession) 
{
   if( stringSession.size() == 32 ) 
   { 
      auto result_ = gd::binary_validate_hex_g(stringSession);                                     if( result_.first == false ) { return { false, "invalid session: " + stringSession }; }
   }
   else if( stringSession.size() == 36 )
   {
      auto result_ = gd::binary_validate_uuid_g(stringSession);                                     if( result_.first == false ) { return { false, "invalid session: " + stringSession }; }
   }
   else
   {
      return { false, "invalid session: " + stringSession };
   }
   
   return {true, ""};
}
