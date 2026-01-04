/** @FILE [tag: document, cache, error, file] [summary: Document stores information for http]
 * \file Document.h
 *
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

// ## GD

#include "gd/gd_uuid.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_database.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_column-buffer.h"

#include "Types.h"

#include "meta/METAQueries.h"

#include "Session.h"

class CApplication;


/** @CLASS [tag: document] [summary: Document class. Facade object for data in cleaner]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class CDocument // @AI [tag: document, data] [llm: core]
{
public:

   constexpr static uint64_t uDefaultReqiuestFlags_c = Types::eRequestItemIp | Types::eRequestItemUserAgent | Types::eRequestItemSession;

public:
   /// tag dispatcher used to state dependet operations
   struct tag_state {};

public:
   /// @brief Type alias for table, which can be either a `gd::table::dto::table` or a `gd::table::arguments::table`.
   using table_t = std::variant< std::unique_ptr< gd::table::dto::table >, std::unique_ptr< gd::table::arguments::table > >;
   /// @brief Type alias for pointer to table, which can be either a pointer to `gd::table::dto::table` or a pointer to `gd::table::arguments::table`.
   using pointer_table_t = std::variant< gd::table::dto::table*, gd::table::arguments::table* >;

   // ## @API [tag: construct] [description: Construct document.]
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

   ~CDocument();
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
   uint64_t GetRequestFlags() const { return m_uRequestFlags;  }
   CApplication* GetApplication() { return m_papplication; }
   const CApplication* GetApplication() const { return m_papplication; }
   gd::variant_view Get(const std::string_view& stringName) const { return m_arguments[stringName].as_variant_view(); }
   void Set(const std::string_view& stringName, const gd::variant_view& variantValue) { m_arguments.set(stringName, variantValue); }

   std::string_view GetId() const { return m_arguments["id"].as_string_view(); }
   void SetId(const std::string_view& stringId) { m_arguments.set("id", stringId); }

   std::string_view GetName() const { return m_arguments["name"].as_string_view(); }
   void SetName(const std::string_view& stringName) { m_arguments.set("name", stringName); }

   void SetDatabase(gd::database::database_i* pdatabase_);
   gd::database::database_i* GetDatabase() { return m_pdatabase; }
//@}

   void Initialize();
   void Initialize( CApplication* papplication, const std::string_view& stringName );


/** \name RESULT
 * Document are able to generate result data based on what the document is storing
 *///@{
   std::pair<bool, std::string> RESULT_Save(const gd::argument::shared::arguments& argumentsResult, const gd::table::dto::table* ptableResult );
//@}

/** \name PROPERTY
 * ## @API [tag: property]
        [description: Property accessors for the documents internal state]
        [detail: Properties are stored in the `m_arguments` member and it can store any kind of data. Each property is identified by a key.]
*///@{
   gd::argument::shared::arguments& PROPERTY_Get() { return m_arguments; }
   const gd::argument::shared::arguments& PROPERTY_Get() const { return m_arguments; }
   gd::variant_view PROPERTY_Get(std::string_view stringName) const { return m_arguments[stringName].as_variant_view(); }
   gd::variant_view PROPERTY_Get(std::string_view stringName, gd::variant_view default_) const { return m_arguments.exists(stringName) ? m_arguments[stringName].as_variant_view() : default_; }
	void PROPERTY_Set(const std::string_view& stringName, const gd::variant_view& variantValue) { m_arguments.set(stringName, variantValue); }
	bool PROPERTY_Exists(const std::string_view& stringName) const { return m_arguments.exists(stringName); }
	/// Updates properties from application properties, e.g. detail level
   void PROPERTY_UpdateFromApplication();
//@}



/** \name CACHE
 * ## @API [tag: table-cache]
        [description: Documents can cache information in tables that are named.]
        [detail: "Each cache is stored in a named table, which is kept in the document's `m_vectorTableCache` member
         Tables may also be marked as temporary, meaning they should be removed as soon as they are no longer needed."]
*
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
   bool CACHE_Get(const std::string_view& stringId, pointer_table_t& ptable_ );
   gd::table::dto::table* CACHE_Get( const std::string_view& stringId, bool bPrepare );
   gd::table::dto::table* CACHE_Get( const std::string_view& stringId ) { return CACHE_Get( stringId, true ); }
   gd::table::arguments::table* CACHE_GetTableArguments( const std::string_view& stringId, bool bPrepare );
   gd::table::arguments::table* CACHE_GetTableArguments(const std::string_view& stringId) { return CACHE_GetTableArguments(stringId, false); }


   /// Sort cached table for specified column
   std::pair<bool, std::string> CACHE_Sort(const std::string_view& stringId, const gd::variant_view& column_, gd::table::dto::table* ptable_ = nullptr );
   /// Filter cached table based on where expression
   std::pair<bool, std::string> CACHE_Where(std::string_view stringId, std::string_view stringWhere, gd::table::dto::table* ptable_ = nullptr);

   /// Return information to generate cache data
   std::pair<bool, std::string> CACHE_GetInformation( const std::string_view& stringId, gd::argument::arguments& argumentsCache );
   gd::argument::arguments CACHE_GetInformation( const std::string_view& stringId );

   /// Clear all cache data
   void CACHE_Clear();
   /// Erase cache with specified id
   void CACHE_Erase( const std::string_view& stringId );
   /// Erase all temporary cache tables
   void CACHE_Erase( gd::types::tag_temporary );
   /// Dump cache data to string
   std::string CACHE_Dump(const std::string_view& stringId);
	/// Check if cache with specified id exists
   bool CACHE_Exists(const std::string_view& stringId) const;
#ifndef NDEBUG
   bool CACHE_Exists_d( const std::string_view& stringId );
#endif // !NDEBUG
//@}


/** \name OPERATION
*///@{
//@}

/** \name MESSAGE
* @API [tag: message]
       [description: Pass information to the active output, message is just text that in some way is to be displayed to user]
*///@{
   /// Display message to user
   void MESSAGE_Display( const std::string_view& stringMessage );
   void MESSAGE_Display( const std::string_view& stringMessage, const gd::argument::arguments& arguments_ );
   void MESSAGE_Display( const gd::table::dto::table* ptable_, tag_state);
   /// Reset message display, i.e. colors are restored to default
   void MESSAGE_Display();
   void MESSAGE_Background();
   /// Display message to user with progress information
   void MESSAGE_Progress( const std::string_view& stringMessage );
   void MESSAGE_Progress( const std::string_view& stringMessage, const gd::argument::arguments& arguments_ );

//@}

// ## @API [tag: sessions] [description: Session management for document]
   uint64_t SESSION_Add( const gd::types::uuid& uuidSession );
   uint64_t SESSION_Add( const gd::types::uuid& uuidSession, gd::types::tag_unsafe );
   void SESSION_Add( const std::vector<std::string>& vectorUuid );
   void SESSION_Delete( const gd::types::uuid& uuidSession );
   uint64_t SESSION_Count() const;
   bool SESSION_Empty() const { return m_psessions != nullptr && m_psessions->Empty() == false; }
   CSessions* SESSION_Get() { return m_psessions.get(); }
   const CSessions* SESSION_Get() const { return m_psessions.get(); }
   std::pair<bool, std::string> SESSION_Initialize( size_t uMaxCount);

// ## @API [tag: queries] [description:  ]

   META::CQueries* QUERIES_Get() { return m_pqueries.get(); }
   const META::CQueries* QUERIES_Get() const { return m_pqueries.get(); }
   bool QUERIES_Empty() const { return m_pqueries != nullptr && m_pqueries->Empty() == false; }
   std::pair<bool, std::string> QUERIES_Initialize( const gd::argument::arguments& arguments_ );
   std::pair<bool, std::string> QUERIES_Initialize() { return QUERIES_Initialize( gd::argument::arguments() ); }

/** \name ERROR
* ## @API [tag: error]
       [description: Document are able to collect error information, for example doing a larger operation where some tasks fail bit it isn't fatal, then store error in document for later display]
*///@{
/// Add error to internal list of errors
   void ERROR_Add( const std::string_view& stringError );
   void ERROR_AddWarning( const std::string_view& stringError );
   bool ERROR_Empty() const { return m_vectorError.empty(); }
   size_t ERROR_Size() const { return m_vectorError.size(); }
   void ERROR_Print( bool bClear = false );

//@}

protected:

public:


// ## attributes ----------------------------------------------------------------
public:
   uint64_t m_uRequestFlags = uDefaultReqiuestFlags_c; ///< document flags
   CApplication* m_papplication = nullptr;      ///< pointer to application object

   gd::argument::shared::arguments m_arguments; ///< document information (members)

	gd::database::database_i* m_pdatabase{};     ///< document database connection if any

   std::unique_ptr<CSessions> m_psessions;      ///< session manager for document if any

   std::unique_ptr<META::CQueries> m_pqueries;  ///< pointer to query information if any

   // ## cache information is stored in dto tables (dto = data transfer object)
   std::shared_mutex m_sharedmutexTableCache;   ///< mutex used as lock for table methods in document
   std::string m_stringCacheConfiguration;      ///< file name for file with cache configuration information

   /// Mutex lock used when methods within document work on table data
   std::mutex m_mutexCache;
   /// vector of tables used to cache data
   std::vector< table_t > m_vectorTableCache;

   // ## Error information. Document are able to collect error information that may be usefull locating problems
   std::shared_mutex m_sharedmutexError;        ///< mutex used to manage errors in threaded environment
   std::vector< gd::argument::arguments > m_vectorError; ///< vector storing internal errors



// ## free functions ---------------------------------------------------------
public:

};

/// @brief Set database connection for document
inline void CDocument::SetDatabase(gd::database::database_i* pdatabase_)
{
   if(m_pdatabase != nullptr) { m_pdatabase->release(); m_pdatabase = nullptr; }

	if(pdatabase_ != nullptr) pdatabase_->add_reference();
   m_pdatabase = pdatabase_;
}

/// @brief Prepare cache information structure
inline void CDocument::CACHE_Prepare( const std::string_view& stringId )
{
   CACHE_Prepare(stringId, nullptr);
}

inline void CDocument::CACHE_Clear()
{
   std::unique_lock lock( m_mutexCache );
   m_vectorTableCache.clear();
}
