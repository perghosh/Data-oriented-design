#include "HttpServer.h"

#include "command/RouterDatabase.h"

CHttpServer::~CHttpServer()
{
   // ## release all servers
   for ( auto& pserver : m_vectorServer )
   {
      pserver->release();
   }
   m_vectorServer.clear();                                                     // clear vector                                    
}

std::pair<bool, std::string> CHttpServer::Initialize()
{
   // ## Add default routers
   // Routers are like sub servers, they handle a set of commands

   CRouterDatabase* prouterdatabase = new CRouterDatabase("database");         // Create a new router for database
   prouterdatabase->add_reference();                                           // Add reference to router
   // Bind the callback method to this router server (each server can have any number of callbacks)
   prouterdatabase->callback_add( std::bind( &CRouterDatabase::Execute, prouterdatabase, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );

   m_vectorServer.push_back(prouterdatabase);                                  // Keep server router for database


   return { true, "" };
}

std::pair<bool, std::string> CHttpServer::Execute(const std::string_view& stringCommand)
{
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