#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_database.h"
#include "gd/gd_table_arguments.h"

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
   std::pair<bool, std::string> SERVER_Start( unsigned uIndex = 0 );
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

/** \name SITE
*///@{

   /// Add site entry
   void SITE_Add( std::string_view stringIp, uint32_t uPort, std::string_view stringFolder );
   /// Get site folder for row
   std::string_view SITE_GetFolder( uint64_t uRow ) const;
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
   CHttpServer* m_phttpserver{};  ///< http server object, used to handle http requests
   gd::com::server::server_i* m_pserver{}; ///< active server

   std::mutex m_mutexDatabase;   ///< Handle database locking
   gd::database::database_i* m_pdatabase{}; ///< active database
   std::vector<gd::database::database_i*> m_vectorDatabase; ///< list of databases (for most situations only one database is used)

   std::unique_ptr<gd::table::arguments::table> m_ptableSite;  ///< table holding site information like ip and root folder, port etc.


   // ## free functions ------------------------------------------------------------
public:

   /// Prepare application options for command line
   void Prepare_s( gd::cli::options& optionsApplication );
   /// Read parsed options and set properties
   std::pair<bool, std::string> Read_s( CApplication* papplication_, gd::cli::options& optionsApplication );



};

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
