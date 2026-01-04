// @FILE [tag: api, system] [summary: API System command class] [type: source] [name: APISystem.cpp]

#include "gd/gd_arguments.h"
#include "gd/gd_binary.h"

#include "../Router.h"
#include "../Document.h"
#include "../Application.h"

#include "APISystem.h"

std::pair<bool, std::string> ValidateSession_s(const std::string& stringSession);

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
std::pair<bool, std::string> CAPISystem::Execute()
{                                                                                                  assert( m_vectorCommand.empty() == false && "No commands");
   // ## execute database command based on m_vectorCommand and m_argumentsParameter

   if( m_vectorCommand.empty() == true ) return { true, "No commands"};

   std::pair<bool, std::string> result_(true,"");

   CRouter::Encode_s( m_argumentsParameter, { "query" } );

   for( std::size_t uIndex = 0; uIndex < m_vectorCommand.size(); ++uIndex )
   {
      m_uCommandIndex = static_cast<unsigned>( uIndex );
      std::string_view stringCommand = m_vectorCommand[uIndex];

      if( stringCommand == "sys" ) continue;

      if( stringCommand == "session" )
      {
         uIndex++;
         if( uIndex >= m_vectorCommand.size() ) return { false, "Missing session command" };
         stringCommand = m_vectorCommand[uIndex];
         
         if( stringCommand == "add" )           { result_ = Execute_SessionAdd(); }
         else if( stringCommand == "count" )    { result_ = Execute_SessionCount(); }
         else if( stringCommand == "delete" )   { result_ = Execute_SessionDelete(); }
         else if( stringCommand == "list" )     { result_ = Execute_SessionList(); }
         else { return { false, "unknown session command: " + std::string(stringCommand) }; }
      }
      else
      {
         return { false, "unknown database command: " + std::string(stringCommand) };
      }

      if( result_.first == false ) { return result_; }
   }

   return { true, "" };
}

std::pair<bool, std::string> CAPISystem::Execute_SessionAdd()
{
   std::string stringSession = m_argumentsParameter["session"].as_string();    // get session to add

   if( stringSession.size() < 32 ) { stringSession.append( 32 - stringSession.size(), '0' ); }
   
   auto result_ = ValidateSession_s(stringSession);
   if( result_.first == false ) { return result_; }
   
   // ## copy session

   gd::types::uuid uuid;
   if( stringSession.length() == 32 ) gd::binary_copy_hex_g( uuid,  stringSession);
   else if( stringSession.length() == 36 ) gd::binary_copy_uuid_g( uuid, stringSession );
   
   CDocument* pdocument = GetDocument();
   
   uint64_t uIndex = pdocument->SESSION_Add(uuid);

   // ## return response with index for session added

   gd::argument::arguments* parguments_ = new gd::argument::arguments( { { "index", uIndex } } );
   m_objects.Add( parguments_ );
   
   return { true, "" };
}


std::pair<bool, std::string> CAPISystem::Execute_SessionDelete()
{
   std::string stringSession = m_argumentsParameter["session"].as_string();    // get session to add

   // ## Pad session if less than 32 bytes
   if( stringSession.size() < 32 ) { stringSession.append(32 - stringSession.size(), '0'); }
   
   auto result_ = ValidateSession_s(stringSession);
   if( result_.first == false ) { return result_; }
   
   gd::types::uuid uuid;
   gd::binary_copy_hex_g( uuid,  stringSession);
   
   CDocument* pdocument = GetDocument();
   
   pdocument->SESSION_Delete( uuid );
   
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

std::pair<bool, std::string> CAPISystem::Execute_SessionList()
{
   CDocument* pdocument = GetDocument();

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
