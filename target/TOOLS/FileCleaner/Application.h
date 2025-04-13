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

   void COMMAND_Read(char** ppbszArgument);

   void COMMAND_Print(const std::string& stringArgument);

   void COMMAND_Count(const std::string& stringArgument);

   std::pair<bool, std::string> DATABASE_Open( const gd::argument::shared::arguments& argumentsOpen );

/** \name DATABASE
*///@{
   /// Connect database, string format for connection is database dependent
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




// ## free functions ------------------------------------------------------------
public:
   static void Prepare_s( gd::cli::options& optionsApplication );


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