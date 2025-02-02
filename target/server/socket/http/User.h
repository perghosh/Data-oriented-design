#pragma once

#include "gd/gd_uuid.h"

#include "Application.h"

/**
 * \brief Manage information for one user connection. 
 * 
 * `CUser` collect information for each request, and holds pointer to access
 * information in webserver.
 * 
 * CUser works together with `CApplication`, when it is created ith will add itself
 * into the application object. And when it is destroyed it will remove itself from
 * list of application users.
 * 
 * Each user works in its own thread.
 * 
 * TODO:
 * - Router object with valid rutes for user and where pointer to user is passed.
 *   - start with command to add database connection.
 *   - Execute sql command where sql is passed as argument.
 *   - Return result from select statement where sql is passed as argument.
 *   - Store SQL statement in database.
 *
 *
 \code
 \endcode
 */
class CUser
{
   // ## construction -------------------------------------------------------------
public:
   CUser() : m_uuidKey( gd::uuid::new_uuid_s() ) {}
   CUser( CApplication* papplication ) : m_papplication( papplication ), m_uuidKey( gd::uuid::new_uuid_s() ) {}
   // copy
   CUser(const CUser& o) { common_construct(o); }
   CUser(CUser&& o) noexcept { common_construct(std::move(o)); }
   // assign
   CUser& operator=(const CUser& o) { common_construct(o); return *this; }
   CUser& operator=(CUser&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CUser() {}
private:
   // common copy
   void common_construct(const CUser& o) {}
   void common_construct(CUser&& o) noexcept {}

   // ## operator -----------------------------------------------------------------
public:


   // ## methods ------------------------------------------------------------------
public:
   /** \name GET/SET
   *///@{

   //@}

   /** \name OPERATION
   *///@{

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
   CApplication* m_papplication; ///< pointer to application object
   gd::uuid::uuid m_uuidKey; ///< user unique identifier for user session

   /*
   TODO:
   Variables that will be needed is request object and respons object.
   */


   // ## free functions ------------------------------------------------------------
public:



};