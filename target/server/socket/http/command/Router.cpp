#include <iostream>
#include <ranges>

#include "gd/gd_arguments_shared.h"

#include "Router.h"


/// CRouter destructor, clean attached servers
CRouter::~CRouter()
{
   // release all servers
   for( auto it : m_vectorServer )
   {
      it->release();
   }
}

/** ---------------------------------------------------------------------------
 * @brief Connect server to internal server list
 * You can connect multiple servers to router and they may be called something with server or router
 * and they need to implement server_i interface.
 * @param pserver pointer to server that is connected
 * @return true if ok, false and error information on error
 */
std::pair<bool, std::string> CRouter::Connect(gd::com::server::server_i* pserver)
{
   pserver->add_reference();

   m_vectorServer.push_back( pserver );

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Get(const std::string_view& stringArgument)
{

   return { true, "" };
}

std::pair<bool, std::string> CRouter::Get(std::vector<std::string_view>& vectorCommand)
{
   if( vectorCommand.empty() == true ) return { false, "No command" };

   // ## Find server that can handle command
   for ( auto it : m_vectorServer )
   {
      if( it->is_endpoint(vectorCommand.front()) == true )
      {

      }
   }


   return { true, "" };
}

gd::com::server::server_i* CRouter::GetServer(const std::string_view& stringServer)
{
   for( auto it : m_vectorServer )
   {
      if( it->is_endpoint(stringServer) == true )
      {
         return it;
      }
   }

   return nullptr;
}


