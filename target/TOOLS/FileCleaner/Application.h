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

#include "gd/gd_log_logger.h"
#include "gd/gd_log_logger_printer.h"
#include "gd/gd_log_logger_define.h"


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
   std::pair<bool, std::string> STATEMENTS_Load(const std::string_view& stringFileName);

//@}

/** \name DOCUMENT operations
* Documents are used to store information for each file that is beeing processed
*///@{
   // ## Add documents
   CDocument* DOCUMENT_Add(const std::string_view& stringName);
   CDocument* DOCUMENT_Add(const gd::argument::shared::arguments& arguments_);

   // ## Get documents
   const CDocument* DOCUMENT_Get(const std::string_view& stringName) const;
   CDocument* DOCUMENT_Get(const std::string_view& stringName);

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
   /// List of documents
   std::vector<std::unique_ptr<CDocument>> m_vectorDocument;

   std::vector< gd::database::database_i* > m_vectorDatabase; ///< list of connected databases
   gd::database::database_i* m_pdatabase = nullptr;  ///< active database connection

   std::unique_ptr<application::database::metadata::CStatements> m_pstatements;   ///< pointer to statement object



// ## free functions ------------------------------------------------------------
public:
   // ## Prepare Application 

   static void Prepare_s( gd::cli::options& optionsApplication );
   static void PrepareLogging_s();

   // ## Read data from database

   static void Read_s(const gd::database::record* precord, gd::table::table_column_buffer* ptablecolumnbuffer );
   static void Read_s( gd::database::cursor_i* pcursorSelect, gd::table::table_column_buffer* ptablecolumnbuffer );

   // ## Utility functions

   // ## History functions
   // static std::string HistorySaveArguments_s(const std::string_view& stringArguments);

   // Split string into vector of strings, delimitier is ; or ,. It first tries to find ;, if not found then it tries to find ,
   static std::vector<std::string> Split_s(const std::string& stringText, char iCharacter = 0);


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
