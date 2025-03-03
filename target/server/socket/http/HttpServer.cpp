#include "HttpServer.h"

#include "command/RouterDatabase.h"
#include "command/RouterScript.h"
#include "command/RouterApplication.h"

CHttpServer::~CHttpServer()
{
   // ## release all servers
   for ( auto& pserver : m_vectorServer )
   {
      pserver->release();
   }
   m_vectorServer.clear();                                                     // clear vector                                    
}

gd::com::server::server_i* CHttpServer::GetServer(const std::string_view& stringServerName)
{
   for( auto it : m_vectorServer )
   {
      if( it->is_endpoint(stringServerName) == true )
      {
         return it;
      }
   }

   return nullptr;
}


std::pair<bool, std::string> CHttpServer::Initialize()
{
   // ## Add default routers
   // Routers are like sub servers, they handle a set of commands

   CRouterDatabase* prouterdatabase = new CRouterDatabase("database", "db");   // Create a new router for database
   m_vectorServer.push_back(prouterdatabase);                                  // Keep server router for database
   CRouterScript* prouterscript = new CRouterScript("script");                 // Create a new router for script
   m_vectorServer.push_back(prouterscript);                                    // Keep server router for script
   CRouterApplication* prouterapplication = new CRouterApplication("application", "app");// Create a new router for application
   m_vectorServer.push_back(prouterapplication);                               // Keep server router for application


   //prouterdatabase->add_reference();                                           // Add reference to router
   // Bind the callback method to this router server (each server can have any number of callbacks)
   //prouterdatabase->callback_add( std::bind( &CRouterDatabase::Execute, prouterdatabase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );



   return { true, "" };
}

std::pair<bool, std::string> CHttpServer::Execute(const std::string_view& stringCommand, gd::com::server::response_i** ppresponse)
{
   using namespace gd::com::server::router;
   gd::com::pointer< command > pcommand = gd::com::pointer< command >( new command( this ) );   
   std::vector< std::string_view > vectorCommand = static_cast<gd::com::server::router::command*>( pcommand )->add_querystring( stringCommand );

   auto result = Execute(vectorCommand, pcommand.get(), ppresponse );


   return { true, "" };
}

std::pair<bool, std::string> CHttpServer::Execute( gd::com::server::command_i* pcommand, gd::com::server::response_i** ppresponse )
{
   using namespace gd::com::server::router;

   command* pcommandRun = (command*)pcommand;

   for( size_t u = 0; u < pcommandRun->size(); u++ )
   {
      command::arguments& arguments_ = (*pcommandRun)[u];

      std::string_view stringServer = arguments_[0];
      gd::com::server::server_i* pserver = GetServer(stringServer);
      if( pserver == nullptr ) { return { false, std::string("No server found for command: ") + stringServer.data() }; }


      pcommandRun->activate((int)u);
      auto result = pserver->get(pcommand, *ppresponse);
   }

   /*
   std::string_view stringServerName = (*pcommandRun)[(size_t)0];

   server* pserver = (server*)GetServer(stringServerName);
   if( pserver == nullptr ) { return { false, std::string( "No server found for command: " ) + stringServerName.data() }; }

   auto result = pserver->get( pcommand, *ppresponse );
   */

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Executes a command given as a vector of string views.
 * 
 * This method handles the execution of a command already parsed into string views. 
 * It identifies the server by the first string view, processes the rest as the command, 
 * and executes it through the appropriate server instance.
 *
 * @param vectorCommand A vector of string views representing the command and its parameters.
 * @param pcommand Pointer to the command interface to be used for adding command details.
 * @param ppresponse Pointer to a pointer for storing the response object after execution.
 * @return std::pair<bool, std::string> A pair where the bool indicates success/failure, 
 *         and the string provides additional information or an error message.
 * @note Assertion ensures that the command vector is not empty before processing.
 */
std::pair<bool, std::string> CHttpServer::Execute(const std::vector<std::string_view>& vectorCommand,  gd::com::server::command_i* pcommand, gd::com::server::response_i** ppresponse)
{                                                                                                  assert( vectorCommand.empty() == false );
   using namespace gd::com::server::router;

   std::string_view stringServerName = vectorCommand[0];                       // get server name from command
   server* pserver = (server*)GetServer(stringServerName);
   if( pserver == nullptr ) { return { false, std::string( "No server found for command: " ) + stringServerName.data() }; }

   std::vector<std::string_view> vectorCall(vectorCommand.begin() + 1, vectorCommand.end());// remove first element from vector
   ( (command*)pcommand )->add_command(vectorCall);                            // add command sequence to command object

   auto result = pserver->get( pcommand, *ppresponse);

   return { true, "" };
}


bool CHttpServer::is_endpoint(const std::string_view& stringCommand)
{


   return false;
}

std::pair<bool, std::string> CHttpServer::get(const std::string_view* stringCommandList, const gd::argument::arguments* pargumentsParameter, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse)
{

   return { true, "" };
}