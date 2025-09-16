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
#include <variant>
#include <vector>

#include <boost/regex.hpp>

// ## GD

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_arguments.h"
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
public:
   /// @brief Type alias for table, which can be either a `gd::table::dto::table` or a `gd::table::arguments::table`.
   using table_t = std::variant< std::unique_ptr< gd::table::dto::table >, std::unique_ptr< gd::table::arguments::table > >;

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
* File operations work on tables stored in cache and ech table is identified by a string id.
*///@{

   /// @brief Harvest file information and store it in a table (harvest = collect)
   std::pair<bool, std::string> FILE_Harvest( const gd::argument::shared::arguments& argumentsPath );
   std::pair<bool, std::string> FILE_Harvest( const gd::argument::shared::arguments& argumentsPath, std::string stringFilter );
   std::pair<bool, std::string> FILE_Filter( const std::string_view& stringFilter );
   std::pair<bool, std::string> FILE_FilterBinaries();
   std::pair<bool, std::string> FILE_UpdateRowCounters( int iThreadCount = 0 );
   std::pair<bool, std::string> FILE_UpdatePatternCounters( const gd::argument::shared::arguments& argumentsPattern, const std::vector<std::string>& vectorPattern, int iThreadCount = 0 );
   std::pair<bool, std::string> FILE_UpdatePatternCounters( const gd::argument::shared::arguments& argumentsPattern, const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPatterns, int iThreadCount = 0 );
   std::pair<bool, std::string> FILE_UpdatePatternList( const std::vector<std::string>& vectorPattern, const gd::argument::shared::arguments& argumentsList, int iThreadCount = 0 );
   std::pair<bool, std::string> FILE_UpdatePatternList( const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPatterns, const gd::argument::shared::arguments& argumentsList, int iThreadCount = 0 );

   std::pair<bool, std::string> FILE_UpdatePatternFind( const std::vector< std::string >& vectorRegexPatterns, const gd::argument::shared::arguments* pargumentsFind, int iThreadCount = 0 );
   std::pair<bool, std::string> FILE_UpdatePatternFind_old( const std::vector< std::string >& vectorRegexPatterns, const gd::argument::shared::arguments* pargumentsFind );
   std::pair<bool, std::string> FILE_UpdatePatternFind( const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPatterns, const gd::argument::shared::arguments* pargumentsList, int iThreadCount = 0 );

   std::pair<bool, std::string> BUFFER_UpdateKeyValue( const gd::argument::shared::arguments& argumentsFile, std::string_view stringFileBuffer, gd::table::dto::table& tableRow, const std::vector<gd::argument::arguments>& vectorRule);
      
//@}

/** \name RESULT
 * Document are able to generate result data based on what the document is storing
 *///@{
   std::pair<bool, std::string> RESULT_Save(const gd::argument::shared::arguments& argumentsResult, const gd::table::dto::table* ptableResult );
//@}

/** \name CACHE
 * Documents can cache information and support multiple named caches.
 * Each cache is stored in a named table, which is kept in the document's `m_vectorTableCache` member.
 * Each table also stores a string ID that identifies it within the cache.
 * Tables may also be marked as temporary, meaning they should be removed as soon as they are no longer needed.
 *    CDocument is then used as a data source for moving data between operations.
 *///@{
   /// Prepare cache information structure
   void CACHE_Prepare( const std::string_view& stringId );
   void CACHE_Prepare( const std::string_view& stringId, std::unique_ptr<gd::table::dto::table>* ptable );
   /// Load cache data
   std::pair<bool, std::string> CACHE_Load( const std::string_view& stringId );
   bool CACHE_Add( gd::table::dto::table&& table, const std::string_view& stringId );
   void CACHE_Add( std::unique_ptr< gd::table::arguments::table > ptable_ );
   std::string CACHE_Add( gd::table::dto::table&& table, const std::string_view& stringId, gd::types::tag_temporary );
   bool CACHE_Add( gd::table::dto::table&& table ) { return CACHE_Add( std::forward<gd::table::dto::table>( table ), std::string_view{}); }
   void CACHE_Add( std::unique_ptr< gd::table::dto::table > ptableAdd );
   /// Return table with cache data
   gd::table::dto::table* CACHE_Get( const std::string_view& stringId, bool bPrepare );
   gd::table::dto::table* CACHE_Get( const std::string_view& stringId ) { return CACHE_Get( stringId, true ); }
   gd::table::arguments::table* CACHE_GetTableArguments( const std::string_view& stringId, bool bPrepare );
   gd::table::arguments::table* CACHE_GetTableArguments(const std::string_view& stringId) { return CACHE_GetTableArguments(stringId, false); }


   /// Sort cached table for specified column
   std::pair<bool, std::string> CACHE_Sort(const std::string_view& stringId, const gd::variant_view& column_, gd::table::dto::table* ptable_ = nullptr );

   /// Return information to generate cache data
   std::pair<bool, std::string> CACHE_GetInformation( const std::string_view& stringId, gd::argument::arguments& argumentsCache );
   gd::argument::arguments CACHE_GetInformation( const std::string_view& stringId );
   void CACHE_Erase( const std::string_view& stringId );
   /// Erase all temporary cache tables
   void CACHE_Erase( gd::types::tag_temporary );
   /// Dump cache data to string
   std::string CACHE_Dump(const std::string_view& stringId);
   bool CACHE_Exists(const std::string_view& stringId) const;
#ifndef NDEBUG
   bool CACHE_Exists_d( const std::string_view& stringId );
#endif // !NDEBUG
//@}

/** \name RESULT
 * Methods used to generate results data based on what the document is storing
*///@{
   /// @brief Generate result to present row counting in files
   gd::table::dto::table RESULT_RowCount();
   /// @brief Generate result to present pattern count in files
   gd::table::dto::table RESULT_PatternCount();
   /// @brief Generate result to present pattern line list, i.e. all lines in files where pattern was found
   gd::table::dto::table RESULT_PatternLineList( const gd::argument::arguments& argumentsOption );
//@}

/** \name OPERATION
*///@{
//@}

/** \name MESSAGE
*///@{
   /// Display message to user
   void MESSAGE_Display( const std::string_view& stringMessage );
   void MESSAGE_Display( const std::string_view& stringMessage, const gd::argument::arguments& arguments_ );
   /// Reset message display, i.e. colors are restored to default
   void MESSAGE_Display();
   void MESSAGE_Background();
   /// Display message to user with progress information
   void MESSAGE_Progress( const std::string_view& stringMessage );
   void MESSAGE_Progress( const std::string_view& stringMessage, const gd::argument::arguments& arguments_ );

   //void MESSAGE

//@}


/** \name ERROR
*///@{
/// Add error to internal list of errors
   void ERROR_Add( const std::string_view& stringError );
   bool ERROR_Empty() const { return m_vectorError.empty(); }
   size_t ERROR_Size() const { return m_vectorError.size(); }
   void ERROR_Print();

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
   //std::vector< std::unique_ptr< gd::table::dto::table > > m_vectorTableCache;
   std::vector< table_t > m_vectorTableCache;

   // ## Error information. Document are able to collect error information that may be usefull locating problems
   std::shared_mutex m_sharedmutexError;        ///< mutex used to manage errors in threaded environment
   std::vector< gd::argument::arguments > m_vectorError; ///< vector storing internal errors



// ## free functions ------------------------------------------------------------
public:
   /// Generate result from table where rows in table just are listed top to bottom
   static std::string RESULT_VisualStudio_s( gd::table::dto::table& table_ );
   static void RESULT_VisualStudio_s( gd::table::dto::table& table_, std::string& stringResult );


};

/// \brief Prepare cache information structure
inline void CDocument::CACHE_Prepare( const std::string_view& stringId )
{
   CACHE_Prepare(stringId, nullptr);
}

inline std::string CDocument::RESULT_VisualStudio_s(gd::table::dto::table& table_) {
   std::string stringResult;
   RESULT_VisualStudio_s( table_, stringResult );
   return stringResult;
}