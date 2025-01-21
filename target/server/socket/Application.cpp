#include "Application.h"
#include "Application.h"


CApplication::~CApplication() 
{
}


std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{

   return application::basic::CApplication::Main( iArgumentCount, ppbszArgument, nullptr );
}

std::pair<bool, std::string> CApplication::Initialize()
{

   return application::basic::CApplication::Initialize();
}

/** ---------------------------------------------------------------------------
* @brief call this before application is exited, place last cleanup in this
* @return true if ok, false and error information on error
*/
std::pair<bool, std::string> CApplication::Exit()
{
   return application::basic::CApplication::Exit();
}

/// Return database by index
gd::database::database_i* CApplication::DATABASE_Get( size_t uIndex ) const
{                                                                                                  assert( uIndex < m_vectorDatabase.size() );
   return m_vectorDatabase.at( uIndex );
}

/// Return database by name
gd::database::database_i* CApplication::DATABASE_Get(const std::string_view& stringName) const
{
   for( auto& pdatabase : m_vectorDatabase )
   {
      if( pdatabase->name() == stringName ) { return pdatabase; }
   }
   return nullptr;
}

