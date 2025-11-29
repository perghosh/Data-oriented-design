// @FILE tag: session] [description: Session management for HTTP server, manage user sessions] [type: header]

#include "Session.h"


void CSession::construct()
{
   using namespace gd::table::arguments;

   const unsigned uTableFlags =  table::eTableFlagNull32 | table::eTableFlagRowStatus | table::eTableFlagArguments;
   m_tableSession = gd::table::arguments::table(uTableFlags, { { "binary", 16, "uuid" }, { "uint32", 0, "rights" } }, gd::table::tag_prepare{});
}

std::pair<bool, std::string> CSession::Create( std::size_t uMaxSessionCount )
{                                                                                                  assert( uMaxSessionCount > 0 && "Can not be 0 creating session");
   m_tableSession.reserve( uMaxSessionCount );
   m_tableSession.set_row_count(uMaxSessionCount);                             // set row count to uMaxSessionCount

   return { true, "" };
}