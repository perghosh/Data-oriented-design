#include "RouterApplication.h"



std::pair<bool, std::string> CRouterApplication::get(gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse)
{
   gd::com::server::router::command* pcommand_ = (gd::com::server::router::command*)pcommand;

   if( pcommand_->empty() == false )
   {
      auto* parguments = pcommand_->get_command(0);
      auto stringCommand = (*parguments)[0];
      if( stringCommand == std::string_view{"application"} ) { stringCommand = (*parguments)[1]; }
      auto result_ = Execute( stringCommand, pcommand_, presponse );
   }

   return {true, ""};
}


std::pair<bool, std::string> CRouterApplication::Execute(const std::string_view& stringCommand, gd::com::server::command_i* pcommand, gd::com::server::response_i* presponse)
{
   gd::com::server::router::command* pcommand_ = (gd::com::server::router::command*)pcommand;

   if( stringCommand == "property" )
   {
   }


   return {true, ""};
}
