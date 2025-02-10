#include "HttpServer.h"

#include "../command/RouterDatabase.h"

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

   CRouterDatabase* prouterdatabase = new CRouterDatabase("database");
   prouterdatabase->add_reference();
   m_vectorServer.push_back(prouterdatabase);


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