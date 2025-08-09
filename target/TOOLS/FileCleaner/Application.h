/**                                                                            @TAG #application #document #config #ignore
 * @file Application.h
 * 
 * @brief Main application class for FileCleaner tool. Acts as the entry point for the application, handling initialization, command line arguments, and application state.
 * 
 * CApplication is the main class for the FileCleaner tool, responsible for managing the application state.
 * Through the application it is possible to acces all other parts of the application. CApplications is
 * like a facade object that provides access to the application functionality.
 *
 * Parts in application:
 * - DOCUMENT - manages and accesses documents, documents are used to handle data.
 * - IGNORE - manages ignore patterns for files and folders. This is global for the application.
 * - CONFIG - Configuration management, used to store and retrieve application settings.
 *
 * Application is also responsible to manage starting and stopping the application. What happens when
 * the application is started or stopped is within the application responsibility.
 * 
 * ## Free functions
 * There are a lot of free functions that acts as some type of utility functions for the application.
 * Header file is included in the application to make it possible to use these functions.
 *
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0construct.Application` - Represents a single argument in `arguments`.
 */

#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

//#include "gd/gd_database.h"
//#include "gd/gd_database_odbc.h"
//#include "gd/gd_database_sqlite.h"
#include "gd/gd_table_table.h"
#include "gd/gd_arguments_shared.h"
#include "gd/expression/gd_expression_parse_state.h"
#include "gd/parse/gd_parse_match_pattern.h"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_define.h"

#include "gd/console/gd_console_console.h"

//#include "jsoncons/json.hpp"


#include "application/database/Metadata_Statements.h"
#include "Document.h"


#include "application/ApplicationBasic.h"

//namespace jsoncons { class json; }
namespace gd { namespace cli { class options; } }
namespace CONFIGURATION { class CSettings; }

/// @brief Forward declaration using jsoncons library, this to avoid including the full jsoncons header here.
namespace jsoncons {
   struct sorted_policy;
   template<typename CharT, typename Policy, typename Allocator>
   class basic_json;

   using json = basic_json<char, sorted_policy, std::allocator<char>>;  // Recreate the alias locally (must match exactly)   
}

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CApplication : public application::basic::CApplication
{
public:
   // type of mode for the application, Available modes: `review`, `stats`, `search`, `changes`, `audit`, `document`
   enum enumMode 
   {
      eModeUnknown = 0, ///< Unknown or unspecified mode
      eModeReview,      ///< Review mode, used for reviewing files
      eModeStats,       ///< Statistics mode, used for gathering statistics
      eModeSearch,      ///< Search mode, used for searching files
      eModeChanges,     ///< Changes mode, used for tracking changes in files
      eModeAudit,       ///< Audit mode, used for auditing files
      eModeDocument     ///< Document mode, used for managing documents
   };


   /**
    * \enum enumUIType
    * \brief Represents the type of user interface for the application.
    *
    * - eUITypeUnknown:   Unknown or unspecified UI type.
    * - eUITypeConsole:  Console-based UI.
    * - eUITypeWeb:      Web-based UI.
    * - eUITypeWIMP:     WIMP (Windows, Icons, Menus, Pointer) desktop UI.
    * - eUITypeVSCode:   Visual Studio Code extension UI.
    * - eUITypeVS:       Visual Studio extension UI.
    * - eUITypeSublime:  Sublime Text extension UI.
    */
   enum enumUIType
   {
      eUITypeUnknown = 0, ///< Unknown or unspecified UI type
      eUITypeConsole,     ///< Console-based UI
      eUITypeWeb,         ///< Web-based UI
      eUITypeWIMP,        ///< WIMP (Windows, Icons, Menus, Pointer) desktop UI
      eUITypeVSCode,      ///< Visual Studio Code extension UI
      eUITypeVS,          ///< Visual Studio extension UI
      eUITypeSublime,     ///< Sublime Text extension UI

      eUIFile,            ///< Output to file
   };

   /**
    * \enum enumApplicationState
    * \brief Represents the state of the application.
    *
    * - eApplicationStateUnknown:  Unknown or unspecified application state.
    * - eApplicationStateInitialized:  Application has been initialized.
    * - eApplicationStateWork:  Application is in work state, threads are running.
    * - eApplicationStateIdle:  Application is in pause state, threads are paused.
    * - eApplicationStateExit:  Application is in exit state, threads are exiting.
    */
   enum enumApplicationState
   {
      eApplicationStateUnknown           = 0,    ///< Unknown or unspecified application state
      eApplicationStateInitialized       = 0x01, ///< Application has been initialized
      eApplicationStateWork              = 0x02, ///< Application is in work state, threads are running
      eApplicationStateIdle              = 0x04, ///< Application is in pause state, threads are paused
      eApplicationStateExit              = 0x08, ///< Application is in exit state, threads are exiting

      eApplicationStateCheckIgnoreFolder = 0x10, ///< Application is checking ignore patterns for folders, used to avoid processing folders that are ignored
      eApplicationStateCheckIgnoreFile   = 0x20, ///< Application is checking ignore patterns for files, used to avoid processing files that are ignored
   };


   /**
    * \brief ignore information for folders or files to ignore
    * 
    * Holds information about folders or files to ignore during processing.
    * 
    * Note that rules in ignore is appliced from the project root folder downwards.
    */
   struct ignore
   {
      enum enumType
      {
         eTypeRoot = 0x0001, ///< Ignore type is root ignore, only the first matching folder is ignored
         eTypeFolder = 0x0002, ///< Ignore type is folder ignore, all matching folders are ignored
         eTypeFile = 0x0004, ///< Ignore type is file ignore, all matching files are ignored
         eTypeWildcard = 0x0008, ///< Ignore type is wildcard ignore, all matching files and folders are ignored
      };

      ignore() {}
      ignore(const unsigned uType, const std::string_view& stringIgnore): m_uType(uType), m_stringIgnore(stringIgnore) {}
      ignore(const ignore& o) : m_uType(o.m_uType), m_stringIgnore(o.m_stringIgnore) {}
      ~ignore() {}

      operator std::string_view() const { return m_stringIgnore; } ///< Convert ignore to string view

      bool is_root() const { return ( m_uType & eTypeRoot ) != 0; } ///< Check if ignore is root ignore
      bool is_folder() const { return ( m_uType & eTypeFolder ) != 0; } ///< Check if ignore is folder ignore
      bool is_file() const { return ( m_uType & eTypeFile ) != 0; } ///< Check if ignore is file ignore
      bool is_wildcard() const { return ( m_uType & eTypeWildcard ) != 0; } ///< Check if ignore is wildcard ignore

      // ## attributes
      unsigned m_uType = 0; ///< Type of ignore, e.g. folder, file, and how to apply it
      std::string m_stringIgnore; ///< String with ignore pattern
   };



// ## construction -------------------------------------------------------------
public:  // 0TAG0construct.Application
   CApplication();
   // copy
   CApplication(const CApplication& o);
   CApplication(CApplication&& o) noexcept;
   // assign
   CApplication& operator=(const CApplication& o);
   CApplication& operator=(CApplication&& o) noexcept;

   ~CApplication();
private:
   // common copy
   void common_construct(const CApplication& o);
   void common_construct(CApplication&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:
   /// interface operator to database
   //operator gd::database::database_i*() const { assert( m_pdatabase != nullptr ); return m_pdatabase;  }
   /// pointer to statements object
   //operator application::database::metadata::CStatements*() const { assert(m_pstatements != nullptr); return m_pstatements.get(); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   enumMode GetMode() const { return m_eMode; }
   void SetMode(enumMode eMode) { m_eMode = eMode; }
   void SetMode(const std::string_view& stringMode); 
   std::string GetModeAsString() const;

   enumUIType GetUIType() const { return m_eUIType; }
   void SetUIType(enumUIType eUIType) { m_eUIType = eUIType; }
   std::string GetUITypeAsString() const;

   // ## application state checks
   
   bool IsInitialized() const { return ( m_uApplicationState & eApplicationStateInitialized ) != 0; }
   bool IsWork() const { return ( m_uApplicationState & eApplicationStateWork ) != 0; }
   bool IsIdle() const { return ( m_uApplicationState & eApplicationStateIdle ) != 0; }
   bool IsExit() const { return ( m_uApplicationState & eApplicationStateExit ) != 0; }

   bool IsState(unsigned uState) const { return ( m_uApplicationState & uState ) != 0; }

   void SetState(unsigned uSet, unsigned uClear) { m_uApplicationState = ( m_uApplicationState & ~uClear ) | uSet; }
//@}

/** \name INTERFACE
*///@{
   std::pair<bool, std::string> Main( int iArgumentCount, char* ppbszArgument[], std::function<bool ( const std::string_view&, const gd::variant_view& )> process_ ) override;
   std::pair<bool, std::string> Initialize() override;
   std::pair<bool, std::string> Exit() override;
//@}


/** \name OPERATION
*///@{
   std::pair<bool, std::string> Initialize( gd::cli::options& optionsApplication );
   /// Update application state
   void UpdateApplicationState();
   /// Create application specific directory if it does not exist
   std::pair<bool, std::string> CreateDirectory();
   /// Print message to user
   std::pair<bool, std::string> PrintMessage(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   /// Print progress message to user
   std::pair<bool, std::string> PrintProgress(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   /// Print error message to user
   std::pair<bool, std::string> PrintError(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   ///
   void Print( std::string_view stringColor, gd::types::tag_background );

//@}

   std::pair<bool, std::string> STATEMENTS_Load(const std::string_view& stringFileName);

/** \name DOCUMENT operations
* Documents are used to store information for each file that is beeing processed
*///@{
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

   // ## Add ignore pattern to list of ignored folders

   void IGNORE_Add( unsigned uType, const std::string_view& stringIgnore ) { m_vectorIgnore.push_back( { uType, std::string( stringIgnore ) } ); }
   void IGNORE_Add( const std::vector<ignore>& vectorIgnore ) { m_vectorIgnore.insert( m_vectorIgnore.end(), vectorIgnore.begin(), vectorIgnore.end() ); }
   void IGNORE_Add( const std::vector<std::string> vectorIgnore );

   /// Ccheck if paths is to be ignored
   bool IGNORE_Match( const std::string_view& stringPath, const std::string_view& stringRoot ) const;
   bool IGNORE_Match( const std::string_view& stringPath ) const { return IGNORE_Match( stringPath, std::string_view() ); }
   bool IGNORE_MatchFilename(const std::string_view& stringFileName ) const;

   /// Return number of ignore patterns
   size_t IGNORE_Size() const { return m_vectorIgnore.size(); }
   /// Check if ignore list is empty
   bool IGNORE_Empty() const { return m_vectorIgnore.empty(); }
   /// Clear ignore list
   void IGNORE_Clear() { m_vectorIgnore.clear(); }

//@}


/** \name DATABASE
*///@{
   /*
   std::pair<bool, std::string> DATABASE_Open( const gd::argument::shared::arguments& argumentsOpen );
   std::pair<bool, std::string> DATABASE_Update();
   std::pair<bool, std::string> DATABASE_Upgrade( uint64_t uVersion );
   void DATABASE_Append( gd::database::database_i* pdatabase, bool bActivate );
   std::pair<bool, std::string> DATABASE_Connect( const std::string_view& stringConnect );
   void DATABASE_CloseActive();
   */
//@}

/** \name DATABASE
*///@{
   std::pair<bool, std::string> CONFIG_Load(); /// Load configuration from default file
   /// Load configuration from specified file
   std::pair<bool, std::string> CONFIG_Load(const std::string_view& stringFileName );

   gd::variant_view CONFIG_Get( std::string_view stringGroup, std::string_view stringName ) const;
   gd::variant_view CONFIG_Get( std::string_view stringGroup, const std::initializer_list<std::string_view> listName ) const;

   /// Checks if configuration is not loaded, if pointer to configuration is null than it has not been loaded
   bool CONFIG_Empty() const { return m_ptableConfig == nullptr; }

//@}

/** \name ERROR
*///@{
/// Add error to internal list of errors
   void ERROR_Add( const std::string_view& stringError );
   std::string ERROR_Report() const;

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
   enumMode m_eMode = eModeUnknown;                ///< Mode of the application, e.g. review, stats, search, changes, audit, document
   enumUIType m_eUIType = eUITypeUnknown;          ///< Type of user interface
   unsigned m_uApplicationState = eApplicationStateUnknown; ///< State of the application

   std::shared_mutex m_sharedmutex;                ///< mutex used for application command that may be used by different threads
   /// List of documents
   std::vector<std::unique_ptr<CDocument>> m_vectorDocument;

   std::vector<ignore> m_vectorIgnore;             ///< Strings with patterns for folders to ignore

   //std::vector< gd::database::database_i* > m_vectorDatabase; ///< list of connected databases
   //gd::database::database_i* m_pdatabase = nullptr;///< active database connection

   std::unique_ptr<application::database::metadata::CStatements> m_pstatements;   ///< pointer to statement object

   std::shared_mutex m_sharedmutexError;           ///< mutex used to manage errors in threaded environment
   std::vector< gd::argument::arguments > m_vectorError; ///< vector storing internal errors

   CONFIGURATION::CSettings* m_psettings = nullptr;

   gd::console::console m_console;                 ///< console used for printing formated messages to user

   std::unique_ptr<jsoncons::json> m_pjsonConfig;  ///< JSON configuration object
   std::unique_ptr<gd::table::table> m_ptableConfig; ///< Table used to store configuration data

// ## free functions ------------------------------------------------------------
public:
   // ## Constants

   static enumUIType GetUITypeFromString_s(const std::string_view& stringUIType);

   // ## Prepare Application 

   /// Prepare options for application, options are used to parse command-line arguments
   static void Prepare_s( gd::cli::options& optionsApplication );
   /// Prepare console for application, console is used to print messages to user
   static std::pair<bool, std::string> Prepare_s( gd::console::console* pconsole );
   /// Prepare logging for application.
   static void PrepareLogging_s();
   /// Prepare state used to investigate source files
   static std::pair<bool, std::string> PrepareState_s(const gd::argument::shared::arguments& argumentsPath, gd::expression::parse::state& state_);

   // ## Path operations
   static unsigned PreparePath_s( std::string& stringPath );
   static unsigned PreparePath_s( std::string& stringPath, char iSplitCharacter );

   // ## Folder operations
   static std::pair<bool, std::string> FolderGetHome_s(std::string& stringHomePath);

   // ## Configuration read functions

   /// Read folders to ignore from ignore file if found, otherwise return empty vector
   static std::pair<bool, std::string> ReadIgnoreFile_s( const std::string_view& stringForderOrFile, std::vector<ignore>& vectorIgnorePattern );

   static std::pair<bool, std::string> SettingsRead_s(const std::string_view stringFile, gd::types::tag_xml );

   // ## Read data from database

   //static void Read_s(const gd::database::record* precord, gd::table::table_column_buffer* ptablecolumnbuffer );
   //static void Read_s( gd::database::cursor_i* pcursorSelect, gd::table::table_column_buffer* ptablecolumnbuffer );

   // ## Utility functions

   // ## History functions

   /// Save last command to history
   static std::pair<bool, std::string> HistorySaveArguments_s(const std::string_view& stringArguments);
   /// Print history to console
   static std::pair<bool, std::string> HistoryPrint_s();
   /// Get history location
   static std::pair<bool, std::string> HistoryLocation_s(std::filesystem::path& pathLocation);
   /// Save history table to file
   static std::pair<bool, std::string> HistorySave_s(const std::string_view& stringFileName, const gd::table::dto::table* ptable);

   /// Split string into vector of strings, delimitier is ; or ,. It first tries to find ;, if not found then it tries to find ,
   static std::vector<std::string> Split_s(const std::string& stringText, char iCharacter = 0);
   /// Splits a string into vector of number string, it tries to figure out characters used to split numbers.
   static std::vector<std::string> SplitNumber_s(const std::string& stringText);

   static std::pair<bool, std::string> ParseKeyValueRule_s( const std::string_view stringRule, gd::argument::arguments* pargumentsKVRule );


   static bool IsTextFile_s(const std::string_view& stringExtension);
   
#ifdef _WIN32
   // ## windows specific functions

   /// Prepare for windows OS specific functionality, things like initialize COM
   static std::pair<bool, std::string> PrepareWindows_s();
   /// Exit windows specific functionality, things like uninitialize COM
   static std::pair<bool, std::string> ExitWindows_s();
#endif


};

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

inline std::pair<bool, std::string> CApplication::CONFIG_Load() {
   return CONFIG_Load( std::string_view{} );
}

extern CApplication* papplication_g; ///< global pointer to application object
