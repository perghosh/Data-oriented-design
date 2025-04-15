/**
 * \file Metadata_Statements.h
 * 
 * \brief Store database statement information, this could be any statement that is executable in database.
 * 
 */

#pragma once

#include <cassert>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"
#include "gd/gd_variant_view.h"

// #include "Metadata_Types.h"



#ifndef APPLICATION_DATABASE_METADATA_BEGIN
#  define APPLICATION_DATABASE_METADATA_BEGIN namespace application { namespace database { namespace metadata {
#  define APPLICATION_DATABASE_METADATA_END } } }
#endif

APPLICATION_DATABASE_METADATA_BEGIN

/*-----------------------------------------*/ /**
 * \brief type of database statement
 *
 */
enum enumStatementType
{
   // ## sql statement type number
   eStatementTypeUnknown         = 0,
   eStatementTypeSelect          = 1,
   eStatementTypeInsert          = 2,
   eStatementTypeUpdate          = 3,
   eStatementTypeDelete          = 4,
};

constexpr enumStatementType GetStatementType_g( const std::string_view& stringType );


// ----------------------------------------------------------------------------
// ----------------------------------------------------------------- CStatement
// ----------------------------------------------------------------------------


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CStatement
{
public:
   enum enumFlags
   {
      eFlagNone =          0,
      eFlagRepeatParse =   0x0001,  ///< repeat parse of statement and convert to complete statement
      eFlagIgnoreParse =   0x0002,  ///< ignore parse of statement
      eFlagCache       =   0x0004,  ///< cache statement result
   };



// ## construction -------------------------------------------------------------
public:
   CStatement(): m_uType(0), m_uuidKey( gd::uuid::new_uuid_s() ) {}
   CStatement( unsigned uType, const std::string_view& stringName, const std::string_view& stringSql ): m_uType(uType), m_uuidKey( gd::uuid::new_uuid_s() ) { SetName( stringName ); SetSql( stringSql ); }
   CStatement( const std::string_view& stringType, const std::string_view& stringName, const std::string_view& stringSql ): m_uType(GetStatementType_g(stringType)), m_uuidKey( gd::uuid::new_uuid_s() ) { SetName( stringName ); SetSql( stringSql ); }
   CStatement( const std::string_view& stringType, const std::string_view& stringName, const std::string_view& stringSql, unsigned uFlags ): m_uType(GetStatementType_g(stringType)), m_uFlags(uFlags), m_uuidKey( gd::uuid::new_uuid_s() ) { SetName( stringName ); SetSql( stringSql ); }
   // copy
   CStatement( const CStatement& o ) { common_construct( o ); }
   CStatement( CStatement&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CStatement& operator=( const CStatement& o ) { common_construct( o ); return *this; }
   CStatement& operator=( CStatement&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CStatement() {}
private:
   // common copy
   void common_construct( const CStatement& o ) { m_uuidKey = o.m_uuidKey; m_uType = o.m_uType; m_arguments = o.m_arguments; m_uFlags = o.m_uFlags; }
   void common_construct( CStatement&& o ) noexcept { m_uuidKey = o.m_uuidKey; m_uType = o.m_uType; m_arguments = std::move( o.m_arguments ); m_uFlags = o.m_uFlags; }

// ## operator -----------------------------------------------------------------
public:
   bool operator==( const gd::uuid& uuidKey ) { return m_uuidKey == uuidKey; }
   bool operator!=( const gd::uuid& uuidKey ) { return m_uuidKey != uuidKey; }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   bool IsRepeatParse() const { return (m_uFlags & eFlagRepeatParse) == eFlagRepeatParse; }

   gd::uuid GetKey() const { return m_uuidKey; }
   unsigned GetType() const { return m_uType; }
   const gd::variant_view GetName() const { return m_arguments["name"].as_variant_view(); }
   void SetName( const std::string_view& stringName ) { m_arguments.set("name", stringName ); }
   const gd::variant_view GetSql() const { return m_arguments["sql"].as_variant_view(); }
   void SetSql( const std::string_view& stringSql ) { m_arguments.set("sql", stringSql ); }
   std::string GetFormat() const { return m_arguments["format"].as_string(); }
   void SetFormat( const std::string_view& stringFormat );
//@}

/** \name OPERATION
*///@{
   void TABLE_Set( const std::string_view& stringTable );
   void TABLE_Add( const std::string_view& stringTable );
   bool TABLE_Exists() const { return m_arguments["table"].empty() == false; }
   unsigned TABLE_GetCount() const;
   std::string TABLE_Get( unsigned uTableIndex ) const;

   std::pair< bool, std::string > GetCompiledText() const;
   std::pair< bool, std::string > GetCompiledText( const gd::argument::arguments& argumentsValue ) const;

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
   gd::uuid m_uuidKey;     ///< unique key for statement
   unsigned m_uType;       ///< type of statement
   unsigned m_uFlags = 0;  ///< flags for statement
   gd::argument::arguments m_arguments;///< statement member data packed in `arguments` object


// ## free functions ------------------------------------------------------------
public:
   static constexpr std::string_view ToString_s(enumFlags eFlag);
   static constexpr enumFlags FromString_s(std::string_view stringFlag);


};

// set format for statement
inline void CStatement::SetFormat( const std::string_view& stringFormat ) { 
   if( stringFormat.empty() == false ) m_arguments.set("format", stringFormat ); 
   else                                m_arguments.remove("format");
}

constexpr std::string_view CStatement::ToString_s( enumFlags eFlag )
{
   switch( eFlag )
   {
   case eFlagNone: return "none";
   case eFlagRepeatParse: return "repeat-parse";
   case eFlagIgnoreParse: return "ignore-parse";
   case eFlagCache: return "cache";
   default: return "Unknown";
   }
}

constexpr CStatement::enumFlags CStatement::FromString_s(std::string_view str)
{
   if (str == "eFlagNone") return eFlagNone;
   if (str == "eFlagRepeatParse") return eFlagRepeatParse;
   if (str == "eFlagIgnoreParse") return eFlagIgnoreParse;
   if (str == "eFlagCache") return eFlagCache;
   return static_cast<enumFlags>(-1); // or throw an exception if you prefer
}




// ----------------------------------------------------------------------------
// ---------------------------------------------------------------- CStatements
// ----------------------------------------------------------------------------

/// tag dispatcher to execute version of method that do not lock (not thread safe)
struct tag_nolock {};

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CStatements
{
// ## construction -------------------------------------------------------------
public:
   CStatements() {}
   // copy
   CStatements( const CStatements& o ) { common_construct( o ); }
   CStatements( CStatements&& o ) noexcept { common_construct( std::move( o ) ); }
   // assign
   CStatements& operator=( const CStatements& o ) { common_construct( o ); return *this; }
   CStatements& operator=( CStatements&& o ) noexcept { common_construct( std::move( o ) ); return *this; }

   ~CStatements() {}
private:
   // common copy
   void common_construct( const CStatements& o ) {}
   void common_construct( CStatements&& o ) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   CStatement* Get( size_t uIndex ) { return &m_vectorStatement[uIndex]; }
   const CStatement* Get( size_t uIndex ) const { return &m_vectorStatement[uIndex]; }
   CStatement* Get( const std::string_view& stringName );
   const CStatement* Get( const std::string_view& stringName ) const noexcept;
   CStatement* Find( const std::string_view& stringName );
   CStatement* Find( const std::string_view& stringName, tag_nolock );
   const CStatement* Find( const std::string_view& stringName ) const noexcept;
   const CStatement* Find( const std::string_view& stringName, tag_nolock ) const noexcept;
   const CStatement* Find( const std::string_view& stringType, const std::string_view& stringName ) const noexcept;

/** \name APPEND
* Appends statement to collection
*///@{
   void Append( const gd::argument::arguments& argumentsStatement );
   void Append( CStatement&& statement );
   void Append( CStatement&& statement, tag_nolock );
   void Append( CStatement&& statement, std::function<void ( CStatement* )> callback_ );
   
//@}

/** \name Various methods, similar to those find for container classes in stl
* 
*///@{
   void Remove( const std::string_view& stringName );
   size_t Size() const { return m_vectorStatement.size(); }
   void Clear();
   bool Empty() const { return m_vectorStatement.empty(); }
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
   std::vector< CStatement > m_vectorStatement;

private:
   mutable std::shared_mutex m_sharedmutex; ///< needed for thread safety, multiple reads but only one that can write (`mutable` is used to allow modification in const methods)


// ## free functions ------------------------------------------------------------
public:



};

/// Get statement pointer for name
inline CStatement* CStatements::Get( const std::string_view& stringName ) {
   auto pstatement = Find( stringName );                                                           assert( pstatement != nullptr );
   return pstatement;
}

/// Get statement pointer for name
inline const CStatement* CStatements::Get( const std::string_view& stringName ) const noexcept {
   auto pstatement = Find( stringName );                                                           assert( pstatement != nullptr );
   return pstatement;
}

constexpr enumStatementType GetStatementType_g( const std::string_view& stringType )
{
   using namespace gd::types::detail;

   enumStatementType eType = eStatementTypeUnknown;
   uint32_t uTypeName = hash_type( stringType ); 

   switch( uTypeName )
   {
   case hash_type("select"): eType = eStatementTypeSelect;  break;             // select
   case hash_type("insert"): eType = eStatementTypeInsert;  break;             // insert
   case hash_type("update"): eType = eStatementTypeUpdate;  break;             // update
   case hash_type("delete"): eType = eStatementTypeDelete;  break;             // delete
   default: assert(false);
   }

   return eType;
}



APPLICATION_DATABASE_METADATA_END