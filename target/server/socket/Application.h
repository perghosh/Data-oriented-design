#pragma once

#include <utility>
#include <string>
#include <string_view>
#include <vector>
#include <memory>

#include "gd/gd_database.h"
#include "gd/gd_database_odbc.h"
#include "gd/gd_database_sqlite.h"

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

   //@}

   /** \name OPERATION
   *///@{
   /// Method that can be used to harvest main arguments
   std::pair<bool, std::string> Main( int iArgumentCount, char* ppbszArgument[], std::function<bool ( const std::string_view&, const gd::variant_view& )> process_ );
   /// Initialize application to connect, load needed data and other stuff to make it work
   std::pair<bool, std::string> Initialize();
   /// Use this for clean up
   std::pair<bool, std::string> Exit();
   //@}

/** \name DOCUMENT
*///@{
   /*
   void DOCUMENT_Add( CDocument* pdocument );
   void DOCUMENT_Add( std::unique_ptr<CDocument>&& pdocument );
   void DOCUMENT_Add( std::unique_ptr<CDocument>&& pdocument, bool bActivate );
   void DOCUMENT_Remove( std::size_t uIndex );
   void DOCUMENT_Clear();
   void DOCUMENT_SetFirstActive() { m_pdocument = m_vectorDocument.front().get(); }
   bool DOCUMENT_SetActive( std::function<bool( const CDocument& document )> set_active_ );
   std::vector< std::unique_ptr<CDocument> >::iterator DOCUMENT_Begin() noexcept { return m_vectorDocument.begin(); }
   std::vector< std::unique_ptr<CDocument> >::iterator DOCUMENT_End() noexcept { return m_vectorDocument.end(); }
   std::vector< std::unique_ptr<CDocument> >::const_iterator DOCUMENT_Begin() const noexcept { return m_vectorDocument.cbegin(); }
   std::vector< std::unique_ptr<CDocument> >::const_iterator DOCUMENT_End() const noexcept { return m_vectorDocument.cend(); }
   */
//@}

/** \name DATABASE
*///@{
   void DATABASE_Add( gd::database::database_i* pdatabase );
   gd::database::database_i* DATABASE_Get( size_t uIndex ) const;
   gd::database::database_i* DATABASE_Get( const std::string_view& stringName ) const;
   std::size_t DATABASE_Size() const { return m_vectorDatabase.size(); }
   bool DATABASE_Empty() const { return m_vectorDatabase.empty(); }
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
   gd::database::database_i* m_pdatabase; ///< database main connection
   std::vector< gd::database::database_i* > m_vectorDatabase; ///< list of connected databases


   // ## free functions ------------------------------------------------------------
public:
   //static std::pair<bool, std::string> Start( CApplication* papplication );
   //static int Main();


};

/// Add database to application
inline void CApplication::DATABASE_Add(gd::database::database_i* pdatabase) {
   pdatabase->add_reference();
   m_vectorDatabase.push_back(pdatabase);
}


/// Global pointer to application object
extern CApplication* papplication_g;
