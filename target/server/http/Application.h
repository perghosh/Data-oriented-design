#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_database.h"
#include "gd/gd_table_table.h"
#include "gd/gd_table_arguments.h"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_define.h"

#include "HttpServer.h"

#include "application/ApplicationBasic.h"

#include "Document.h"

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
   CDocument* GetDocument() { return m_pdocumentActive; }
   const CDocument* GetDocument() const { return m_pdocumentActive; }
   /// Get server pointer
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

   /// Execute command from command line
   std::pair< bool, std::string > Execute( gd::cli::options& optionsCommand );


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
   ///
   std::pair<bool, std::string> DATABASE_Connect( const gd::argument::arguments& argumentsConnect );
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

// ## @API [tag: configuration] [description: Configuration management in application]

   std::pair<bool, std::string> CONFIG_Load(); /// Load configuration from default file
   /// Load configuration from specified file
   std::pair<bool, std::string> CONFIG_Load(const std::string_view& stringFileName );

   gd::variant_view CONFIG_Get( std::string_view stringGroup, std::string_view stringName ) const;
   gd::variant_view CONFIG_Get( std::string_view stringGroup, const std::initializer_list<std::string_view> listName ) const;

   void CONFIG_Set( std::string_view stringGroup, std::string_view stringName, const gd::variant_view& value_ );

   /// Checks if configuration is not loaded, if pointer to configuration is null than it has not been loaded
   bool CONFIG_Empty() const { return m_ptableConfig == nullptr; }
   bool CONFIG_Exists( std::string_view stringGroup, std::string_view stringName ) const;

   /// Handle array values
   void CONFIG_HandleArray( std::string_view stringGroup, std::string_view stringName, const gd::argument::shared::arguments& arguments_ );

	// @FILE [tag: document] [description: document management in application]

   // ## Add documents
   CDocument* DOCUMENT_Add(const std::string_view& stringName);
   CDocument* DOCUMENT_Add(const gd::argument::shared::arguments& arguments_);

   // ## Get documents
   const CDocument* DOCUMENT_Get(const std::string_view& stringName) const;
   CDocument* DOCUMENT_Get(const std::string_view& stringName);
   CDocument* DOCUMENT_Get(const std::string_view& stringName, bool bCreate);

   // ## Remove documents
   void DOCUMENT_Remove(const std::string_view& stringName);

   // ## Document statistics and general operations
   size_t DOCUMENT_Size() const;
   bool DOCUMENT_Empty() const;
   void DOCUMENT_Clear();

   // ## iterator methods for documents
   std::vector<std::unique_ptr<CDocument>>::iterator DOCUMENT_Begin();
   std::vector<std::unique_ptr<CDocument>>::iterator DOCUMENT_End();
   std::vector<std::unique_ptr<CDocument>>::const_iterator DOCUMENT_Begin() const;
   std::vector<std::unique_ptr<CDocument>>::const_iterator DOCUMENT_End() const;


// @API [tag: command] [description: broadcast commands]


   std::pair<bool, std::string> SERVER_Execute(const std::string_view& stringCommand, const gd::argument::arguments& argumentsVariable, gd::variant* pvariantResult );


// @API [tag: message] [description: Application are able to print messages, both normal messages, progress and error messages]

   std::pair<bool, std::string> PrintMessage(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   std::pair<bool, std::string> PrintMessage(const std::string_view& stringMessage);
   /// Print progress message to user
   std::pair<bool, std::string> PrintProgress(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   /// Print error message to user
   std::pair<bool, std::string> PrintError(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   ///
   void Print( std::string_view stringColor, gd::types::tag_background );



// @API [tag: error] [description: Application are able to collect error information, for example doing a larger operation where some tasks fail bit it isn't fatal, then store error in application for later display]

/// Add error to internal list of errors
   void ERROR_Add( const std::string_view& stringError );
   void ERROR_AddWarning( const std::string_view& stringError );
   std::string ERROR_Report() const;


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

   std::mutex m_mutex;           ///< general mutex for application

   std::shared_mutex m_sharedmutex;                ///< mutex used for application command that may be used by different threads

   std::mutex m_mutexDatabase;   ///< Handle database locking
   gd::database::database_i* m_pdatabase{}; ///< active database
   std::vector<gd::database::database_i*> m_vectorDatabase; ///< list of databases (for most situations only one database is used)

   std::mutex m_mutexDocument;   ///< Handle document locking
   CDocument* m_pdocumentActive{};  ///< active document
   std::vector< std::unique_ptr<CDocument> > m_vectorDocument;///< list of connected documents, if used in multidocument environment

   std::unique_ptr<gd::table::arguments::table> m_ptableSite;  ///< table holding site information like ip and root folder, port etc.

   std::mutex m_mutexError;           ///< mutex used to manage errors in threaded environment
   std::vector< gd::argument::arguments > m_vectorError; ///< vector storing internal errors

   std::unique_ptr<gd::table::table> m_ptableConfig; ///< Table used to store configuration data


   // ## free functions ------------------------------------------------------------
public:

   /// Prepare application options for command line
   static void Prepare_s( gd::cli::options& optionsApplication );
   /// Read parsed options and set properties
   static std::pair<bool, std::string> Read_s( CApplication* papplication_, gd::cli::options& optionsApplication );
	///  Open database and put connection into pointer reference
	static std::pair<bool, std::string> OpenDatabase_s( const gd::argument::arguments& argumentsOpen, gd::database::database_i*& pdatabase_);

   static std::string FOLDER_GetRoot_s( const std::string_view& stringSubfolder );



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

/// return iterator to the beginning of the document list
inline std::vector<std::unique_ptr<CDocument>>::iterator CApplication::DOCUMENT_Begin() {
   return m_vectorDocument.begin();
}

/// return iterator to the end of the document list
inline std::vector<std::unique_ptr<CDocument>>::iterator CApplication::DOCUMENT_End() {
   return m_vectorDocument.end();
}

/// return const iterator to the beginning of the document list
inline std::vector<std::unique_ptr<CDocument>>::const_iterator CApplication::DOCUMENT_Begin() const {
   return m_vectorDocument.cbegin();
}

/// return const iterator to the end of the document list
inline std::vector<std::unique_ptr<CDocument>>::const_iterator CApplication::DOCUMENT_End() const {
   return m_vectorDocument.cend();
}



/// Global pointer to application object
extern CApplication* papplication_g;
