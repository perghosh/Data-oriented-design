/**
 * \file Application.h
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0construct.Application` - Represents a single argument in `arguments`.
 */

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_arguments_shared.h"

#include "Document.h"

#include "application/ApplicationBasic.h"

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


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name INTERFACE
*///@{
   std::pair<bool, std::string> Initialize() override;
   std::pair<bool, std::string> Exit() override;
//@}


/** \name OPERATION
*///@{

//@}

/** \name DOCUMENT operations
* Documents are used to store information for each file that is beeing processed
*///@{
   // ## Add documents
   void DOCUMENT_Add(const std::string_view& stringName);
   void DOCUMENT_Add(const gd::argument::shared::arguments& arguments_);

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



// ## free functions ------------------------------------------------------------
public:



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