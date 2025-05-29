/**
 * \file Application.h
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

#include "gd/gd_database.h"
#include "gd/gd_database_odbc.h"
#include "gd/gd_database_sqlite.h"
#include "gd/gd_arguments_shared.h"
#include "gd/expression/gd_expression_parse_state.h"
#include "gd/parse/gd_parse_match_pattern.h"

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_define.h"

#include "gd/console/gd_console_console.h"


#include "application/database/Metadata_Statements.h"
#include "Document.h"

#include "application/ApplicationBasic.h"

namespace gd { namespace cli { class options; } }

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
      eApplicationStateUnknown = 0, ///< Unknown or unspecified application state
      eApplicationStateInitialized = 0x01, ///< Application has been initialized
      eApplicationStateWork = 0x02, ///< Application is in work state, threads are running
      eApplicationStateIdle = 0x04, ///< Application is in pause state, threads are paused
      eApplicationStateExit = 0x08, ///< Application is in exit state, threads are exiting
   };

// ## construction -------------------------------------------------------------
public:  // 0TAG0construct.Application
   CApplication() {}
   // copy
   CApplication(const CApplication& o) { common_construct(o); }
   CApplication(CApplication&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CApplication& operator=(const CApplication& o) { common_construct(o); return *this; }
   CApplication& operator=(CApplication&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CApplication() {}
private:
   // common copy
   void common_construct(const CApplication& o);
   void common_construct(CApplication&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:
   /// interface operator to database
   operator gd::database::database_i*() const { assert( m_pdatabase != nullptr ); return m_pdatabase;  }
   /// pointer to statements object
   operator application::database::metadata::CStatements*() const { assert(m_pstatements != nullptr); return m_pstatements.get(); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   enumUIType GetUIType() const { return m_eUIType; }
   void SetUIType(enumUIType eUIType) { m_eUIType = eUIType; }
   std::string GetUITypeAsString() const;

   // ## application state checks
   
   bool IsInitialized() const { return ( m_uApplicationState & eApplicationStateInitialized ) != 0; }
   bool IsWork() const { return ( m_uApplicationState & eApplicationStateWork ) != 0; }
   bool IsIdle() const { return ( m_uApplicationState & eApplicationStateIdle ) != 0; }
   bool IsExit() const { return ( m_uApplicationState & eApplicationStateExit ) != 0; }

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
   /// Create application specific directory if it does not exist
   std::pair<bool, std::string> CreateDirectory();
   /// Print message to user
   std::pair<bool, std::string> PrintMessage(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);
   /// Print progress message to user
   std::pair<bool, std::string> PrintProgress(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat);

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
//@}


/** \name DATABASE
*///@{
   std::pair<bool, std::string> DATABASE_Open( const gd::argument::shared::arguments& argumentsOpen );
   std::pair<bool, std::string> DATABASE_Update();
   std::pair<bool, std::string> DATABASE_Upgrade( uint64_t uVersion );
   void DATABASE_Append( gd::database::database_i* pdatabase, bool bActivate );
   std::pair<bool, std::string> DATABASE_Connect( const std::string_view& stringConnect );
   void DATABASE_CloseActive();
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
   enumUIType m_eUIType = eUITypeUnknown; ///< Type of user interface
   unsigned m_uApplicationState = eApplicationStateUnknown; ///< State of the application

   std::shared_mutex m_sharedmutex;  ///< mutex used for application command that may be used by different threads
   /// List of documents
   std::vector<std::unique_ptr<CDocument>> m_vectorDocument;

   std::vector< gd::database::database_i* > m_vectorDatabase; ///< list of connected databases
   gd::database::database_i* m_pdatabase = nullptr;  ///< active database connection

   std::unique_ptr<application::database::metadata::CStatements> m_pstatements;   ///< pointer to statement object

   std::shared_mutex m_sharedmutexError;        ///< mutex used to manage errors in threaded environment
   std::vector< gd::argument::arguments > m_vectorError; ///< vector storing internal errors

   gd::console::console m_console; ///< console used for printing formated messages to user

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
   static void PathPrepare_s( std::string& stringPath );

   // ## Read data from database

   static void Read_s(const gd::database::record* precord, gd::table::table_column_buffer* ptablecolumnbuffer );
   static void Read_s( gd::database::cursor_i* pcursorSelect, gd::table::table_column_buffer* ptablecolumnbuffer );

   // ## Utility functions

   // ## History functions

   /// Save last command to history
   static std::pair<bool, std::string> HistorySaveArguments_s(const std::string_view& stringArguments);
   /// Print history to console
   static std::pair<bool, std::string> HistoryPrint_s();

   // Split string into vector of strings, delimitier is ; or ,. It first tries to find ;, if not found then it tries to find ,
   static std::vector<std::string> Split_s(const std::string& stringText, char iCharacter = 0);
   
#ifdef _WIN32
   // ## windows specific functions

   /// Prepare for windows specific functionality, things like initialize COM
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

extern CApplication* papplication_g; ///< global pointer to application object
