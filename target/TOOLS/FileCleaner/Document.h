/**
 * \file Document.h
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0construct.Document` - Represents a single argument in `arguments`.
 */

#pragma once

// ## STL

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ## GD

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"


class CApplication;


/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CDocument
{
// ## construction -------------------------------------------------------------
public: // 0TAG0construct.Document
   CDocument() {}
   CDocument(CApplication* papplication) : m_papplication(papplication) { m_arguments.append("name", "default"); }
   CDocument( CApplication* papplication, const std::string_view& stringName ): m_papplication(papplication) { m_arguments.append( "name", stringName ); }
   CDocument( const gd::argument::shared::arguments& arguments_ ): m_arguments( arguments_ ) {}
// copy
   CDocument(const CDocument& o) { common_construct(o); }
   CDocument(CDocument&& o) noexcept { common_construct(std::move(o)); }
// assign
   CDocument& operator=(const CDocument& o) { common_construct(o); return *this; }
   CDocument& operator=(CDocument&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~CDocument() {}
private:
// common copy
   void common_construct(const CDocument& o);
   void common_construct(CDocument&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   CApplication* GetApplication() { return m_papplication; }
   const CApplication* GetApplication() const { return m_papplication; }
   gd::variant_view Get(const std::string_view& stringName) const { return m_arguments[stringName].as_variant_view(); }
   void Set(const std::string_view& stringName, const gd::variant_view& variantValue) { m_arguments.set(stringName, variantValue); }

   std::string_view GetName() const { return m_arguments["name"].as_string_view(); }
   void SetName(const std::string_view& stringName) { m_arguments.set("name", stringName); }
   std::string_view Getpath() const { return m_arguments["path"].as_string_view(); }
   void SetPath(const std::string_view& stringName) { m_arguments.set("path", stringName); }
//@}

/** @name FILE
* 
*///@{

   std::pair<bool, std::string> FILE_Harvest( const gd::argument::shared::arguments& argumentsPath );
   std::pair<bool, std::string> FILE_Filter( const std::string_view& stringFilter );
   std::pair<bool, std::string> FILE_FilterBinaries();
   std::pair<bool, std::string> FILE_UpdateRowCounters();
      
//@}

/** \name RESULT
 * Document are able to generate result data based on what the document is storing
 *///@{
   std::pair<bool, std::string> RESULT_Save(const gd::argument::shared::arguments& argumentsResult, const gd::table::dto::table* ptableResult );
//@}

/** \name CACHE
 * Document are able to cache information and it can handle different named caches.
 * Each cache is stored in a named table and that table is then stored in 
 * member vector in document called `m_vectorTableCache`.
 *///@{
   /// Prepare cache information structure
   void CACHE_Prepare( const std::string_view& stringId );
   /// Load cache data
   std::pair<bool, std::string> CACHE_Load( const std::string_view& stringId );
   bool CACHE_Add( gd::table::dto::table&& table, const std::string_view& stringId );
   bool CACHE_Add( gd::table::dto::table&& table ) { return CACHE_Add( std::forward<gd::table::dto::table>( table ), std::string_view{}); }
   /// Return table with cache data
   gd::table::dto::table* CACHE_Get( const std::string_view& stringId, bool bLoad );
   gd::table::dto::table* CACHE_Get( const std::string_view& stringId ) { return CACHE_Get( stringId, true ); }
   /// Return information to generate cache data
   std::pair<bool, std::string> CACHE_GetInformation( const std::string_view& stringId, gd::argument::arguments& argumentsCache );
   gd::argument::arguments CACHE_GetInformation( const std::string_view& stringId );
   void CACHE_Erase( const std::string_view& stringId );
   /// Dump cache data to string
   std::string CACHE_Dump(const std::string_view& stringId);
#ifndef NDEBUG
   bool CACHE_Exists_d( const std::string_view& stringId );
#endif // !NDEBUG
//@}

/** \name RESULT
 * Methods used to generate results data based on what the document is storing
*///@{
   /// @brief Generate result to present row counting in files
   gd::table::dto::table RESULT_RowCount();
//@}

/** \name OPERATION
*///@{
//@}

/** \name ERROR
*///@{
/// Add error to internal list of errors
   void ERROR_Add( const std::string_view& stringError );

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
   CApplication* m_papplication = nullptr;      ///< pointer to application object

   gd::argument::shared::arguments m_arguments; ///< document information (members)

   // ## cache information is stored in dto tables (dto = data transfer object)
   std::shared_mutex m_sharedmutexTableCache;   ///< mutex used as lock for table methods in document
   std::string m_stringCacheConfiguration;      ///< file name for file with cache configuration information 
   /// vector of tables used to cache data
   std::vector< std::unique_ptr< gd::table::dto::table > > m_vectorTableCache;

   // ## Error information. Document are able to collect error information that may be usefull locating problems
   std::shared_mutex m_sharedmutexError;        ///< mutex used to manage errors in threaded environment
   std::vector< gd::argument::arguments > m_vectorError; ///< vector storing internal errors



// ## free functions ------------------------------------------------------------
public:



};