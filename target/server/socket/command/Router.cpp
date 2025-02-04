#include <iostream>
#include <ranges>

#include "gd/gd_arguments_shared.h"

#include "Router.h"


/// CRouter destructor, clean attached servers
CRouter::~CRouter()
{
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
   /*
   gd::argument::shared::arguments arguments_;
   std::vector<int> vector_ = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

   arguments_ += vector_;
   arguments_ += "vector_";

   arguments_ += {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};;

   arguments_ += std::make_pair( "name", 20 );
   arguments_ += std::make_pair( "name", gd::variant_view(20) );

   std::cout << arguments_.print() << " ";

   // Filter even numbers using a lambda expression
   auto even_numbers = vector_| std::views::filter([](int n) { return n % 2 == 0; });

   // Print the filtered numbers
   for (int n : even_numbers) {
      std::cout << n << " ";
   }
   std::cout << std::endl;
   */

   return { true, "" };
}


