#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_database.h"

#include "../command/Router.h"

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
   CRouter* GetRouter() { return &m_router; }
   const CRouter* GetRouter() const { return &m_router; }
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

/** \name SERVER
*///@{
   std::pair<bool, std::string> SERVER_Start();
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
   CRouter m_router;             ///< command router

   std::mutex m_mutexDatabase;   ///< Handle database locking
   gd::database::database_i* m_pdatabase; ///< active database
   std::vector<gd::database::database_i*> m_vectorDatabase; ///< list of databases (for most situations only one database is used)


   // ## free functions ------------------------------------------------------------
public:
   //static std::pair<bool, std::string> Start( CApplication* papplication );
   static int Main_s( int iArgumentCount, char* ppbszArgument[] );


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
