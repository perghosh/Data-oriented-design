#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_database.h"

//#include "command/Router.h"
#include "HttpServer.h"

#include "application/ApplicationBasic.h"

class CServer;
namespace gd { namespace cli { class options; } }

/**
* \brief
*
*
* ## Properties (application)
* - file-log : log file name
* - log-console : log console severity
* - log-level : log severity level
* - folder-root : root folder for site
* - system-treadcount : number of threads to use
* - ip : ip address to bind to
*
\code
\endcode
*/
class CApplication : public application::basic::CApplication
{
   // ## construction -------------------------------------------------------------
public:
   CApplication() {}
   // copy
   CApplication(const CApplication& o) { common_construct(o); }
   CApplication(CApplication&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CApplication& operator=(const CApplication& o) { common_construct(o); return *this; }
   CApplication& operator=(CApplication&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CApplication();
private:
   // common copy
   void common_construct(const CApplication& o) {}
   void common_construct(CApplication&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   CServer* GetServer() const { return m_pserverBoost; }
   CHttpServer* GetHttpServer() { return m_phttpserver; }
   const CHttpServer* GetHttpServer() const { return m_phttpserver; }
//@}

/** \name OPERATION
*///@{
   /// Method that can be used to harvest main arguments
   std::pair<bool, std::string> Main( int iArgumentCount, char* ppbszArgument[], std::function<bool ( const std::string_view&, const gd::variant_view& )> process_ );
   /// Initialize application to connect, load needed data and other stuff to make it work
   std::pair<bool, std::string> Initialize();
   /// Use this for clean up
   std::pair<bool, std::string> Exit();

   /// Configure global application arguments
   std::pair< bool, std::string > Configure( const gd::cli::options& optionsApplication );


//@}

/** \name SERVER
*///@{
   std::pair<bool, std::string> SERVER_Start();
//@}

/** \name ROUTER
* Router methods to handle commands, routing to correct command object
*///@{
   /// Get active server, this is the defualt server that is used to route commands
   gd::com::server::server_i* ROUTER_GetActiveServer() { return m_pserver; }
   /// Set active server, this is the defualt server that is used to route commands
   void ROUTER_Set(gd::com::server::server_i* pserver);
//@}



/** \name DATABASE
 * Manage database connections stored in application
 *///@{
   void DATABASE_SetActive( const std::variant<std::size_t, std::string_view>& index_ );
   void DATABASE_SetNull();
   /// Add database connection to list of connected databases
   void DATABASE_Add( gd::database::database_i* pdatabase ) { pdatabase->add_reference(); m_vectorDatabase.push_back( pdatabase ); }
   /// return pointer to database for index
   gd::database::database_i* DATABASE_Get( std::size_t uIndex ) const { assert( m_vectorDatabase.size() > uIndex ); return m_vectorDatabase[uIndex]; }
   /// return pointer to database for name
   gd::database::database_i* DATABASE_Get( const std::string_view& stringDatabaseName ) const;
   /// Check if there is any database connection
   bool DATABASE_Empty() const { return m_vectorDatabase.empty(); }
//@}

/** \name CONFIGURATION
*///@{
    /// Read configuration file, reads settings from xml or json file
   std::pair<bool, std::string> CONFIGURATION_Read( const std::string_view& stringFileName );
//@}



protected:
   /** \name INTERNAL
   *///@{

   //@}

public:
   /** \name DEBUG
   *///@{

   //@}


   // ## attributes ----------------------------------------------------------------
public:
   CServer* m_pserverBoost{};     ///< server object , used to handle incoming data and send response, holds boost objects
   //CRouter m_router;              ///< command router, used to route commands to correct command object
   CHttpServer* m_phttpserver{};  ///< http server object, used to handle http requests
   gd::com::server::server_i* m_pserver{}; ///< active server

   std::mutex m_mutexDatabase;   ///< Handle database locking
   gd::database::database_i* m_pdatabase{}; ///< active database
   std::vector<gd::database::database_i*> m_vectorDatabase; ///< list of databases (for most situations only one database is used)


   // ## free functions ------------------------------------------------------------
public:
   //static std::pair<bool, std::string> Start( CApplication* papplication );
   static int Main_s( int iArgumentCount, char* ppbszArgument[] );

   /// Prepare application options for command line
   void Prepare_s( gd::cli::options& optionsApplication );
   /// Read parsed options and set properties
   std::pair<bool, std::string> Read_s( CApplication* papplication_, gd::cli::options& optionsApplication );

   /// Resolve command string into commands and set position to commands
   static std::vector<std::string_view> ROUTER_Resolv_s(const std::string_view& stringCommand, size_t* puOffset );


};

/// Set active server, this is the defualt server that is used to route commands
inline void CApplication::ROUTER_Set(gd::com::server::server_i* pserver) {
   if ( m_pserver != nullptr ) m_pserver->release();
   m_pserver = pserver;
   if ( m_pserver != nullptr ) m_pserver->add_reference();
}

/// ---------------------------------------------------------------------------
/// Clear database active connection and set pointer to null
inline void CApplication::DATABASE_SetNull() {
   if ( m_pdatabase != nullptr ) {
      m_pdatabase->release();
      m_pdatabase = nullptr;
   }
}


/// ---------------------------------------------------------------------------
/// Return database pointer for database name
/// Each database in list can be named and using that name you can access it
inline gd::database::database_i* CApplication::DATABASE_Get(const std::string_view& stringDatabaseName) const {
   for( auto& it : m_vectorDatabase ) {
      if( it->name() == stringDatabaseName  ) return it;
   }
   return nullptr;
}


/// Global pointer to application object
extern CApplication* papplication_g;
