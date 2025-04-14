/**
* \file Document.cpp
* 
* ### 0TAG0 File navigation, mark and jump to common parts
* - `0TAG0CACHE` - cache methods
*/

#include <filesystem>
#include <format>
#include <fstream>
#include <memory>

#include "pugixml/pugixml.hpp"

#include "gd/gd_file.h"
#include "gd/gd_table_io.h"

#include "Command.h"
#include "Application.h"

#include "Document.h"

void CDocument::common_construct(const CDocument& o)
{
   m_arguments = o.m_arguments;
   m_vectorData = o.m_vectorData;
}

void CDocument::common_construct(CDocument&& o) noexcept
{
   m_arguments = std::move(o.m_arguments);
   m_vectorData = std::move(o.m_vectorData);
}

/** ---------------------------------------------------------------------------
 * @brief Loads a file into the document from the specified path.
 *
 * This method attempts to open and read the contents of a file specified by the given path into the document's internal data structure (`m_vectorData`).
 * The file is read in binary mode, and its size is determined to pre-allocate memory for efficiency. If successful, the path is stored in the document.
 *
 * @param stringPath A string view representing the path to the file to be loaded.
 * @return A pair containing:
 *         - `bool`: `true` if the file was successfully loaded, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure (e.g., "Failed to open file: <path>").
 */
std::pair<bool, std::string> CDocument::Load(const std::string_view& stringPath)
{
   std::ifstream ifstreamFile(stringPath.data(), std::ios::binary | std::ios::ate);
   if( ifstreamFile.is_open() == false ) { return { false, std::format("Failed to open file: {}", stringPath) }; }

   // ## Get file size from position at end
   std::streamsize uSize = ifstreamFile.tellg();
   ifstreamFile.seekg(0, std::ios::beg);

   m_vectorData.reserve( static_cast<size_t>(uSize) );

   // Read entire file in one go
   m_vectorData.assign( std::istreambuf_iterator<char>(ifstreamFile), std::istreambuf_iterator<char>() );

   Set("path", stringPath );

   return {true, ""};
}


/** ---------------------------------------------------------------------------
 * @brief Saves the document's data to a file at the specified path.
 *
 * This method writes the contents of the document's internal data structure (`m_vectorData`) to a file in binary mode.
 * If the file cannot be opened or written to, an error message is returned.
 *
 * @param stringPath A string view representing the path where the file should be saved.
 * @return A pair containing:
 *         - `bool`: `true` if the file was successfully saved, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure (e.g., "Failed to write to file: <path>").
 */
std::pair<bool, std::string> CDocument::Save(const std::string_view& stringPath)
{
   std::ofstream ofstreamFile(stringPath.data(), std::ios::binary);
   if(!ofstreamFile) { return {false, std::format("Failed to open file: {}", stringPath )}; }

   ofstreamFile.write(reinterpret_cast<const char*>(m_vectorData.data()), m_vectorData.size());
   if( !ofstreamFile ) { return {false, std::format("Failed to write to file: {}", stringPath ) }; }

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Counts the occurrences of a specific character in the document's data.
 *
 * This method scans the document's internal data (`m_vectorData`) and returns the number of times a given character appears.
 *
 * @param uCharacter The 8-bit unsigned integer representing the character to count.
 * @return The number of occurrences of the specified character in the document's data.
 */
size_t CDocument::Count(uint8_t uCharacter) const
{
   auto uCount = std::count(m_vectorData.begin(), m_vectorData.end(), uCharacter);
   return uCount;
}

/** ---------------------------------------------------------------------------
 * @brief Harvests file information based on the provided arguments.
 *
 * This method retrieves file information (such as path, size, date, and extension) based on the specified arguments.
 * It uses a cache table to store the harvested data.
 *
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @return A pair containing:
 *         - `bool`: `true` if the harvesting was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */ 
std::pair<bool, std::string> CDocument::HarvestFile(const gd::argument::shared::arguments& argumentsPath)
{
   CACHE_Prepare("files");
   auto* ptable_ = CACHE_Get("files");                                                             assert( ptable_ != nullptr );

   auto result_ = HarvestFile_g(argumentsPath, ptable_);
   return result_;
}


// 0TAG0CACHE

/** ---------------------------------------------------------------------------
 * @brief Prepares a cache table for the specified identifier.
 *
 * This method initializes and prepares a table for caching data associated with the given `stringId`.
 * If the `stringId` matches "files", it creates a table with predefined columns ("path", "size", "date", "extension").
 * The table is then added to the internal application cache.
 *
 * @param stringId A string view representing the identifier for the cache table.
 *                 If the identifier is "files", a table with specific columns is prepared.
 *
 * @details
 * - The method first checks if a cache table with the given `stringId` already exists using `CACHE_Get`.
 * - If the table does not exist, it creates a new `table` object with predefined columns.
 * - The table is wrapped in a `std::unique_ptr` and added to the cache using `CACHE_Add`.
 *
 * @note This method assumes that the `CACHE_Add` function handles the ownership of the table.
 */
void CDocument::CACHE_Prepare(const std::string_view& stringId)
{
   using namespace gd::table::dto;

   // ## prepare file list
   //    columns: "path, size, date, extension
   if( stringId == "files" )
   {
      auto ptable_ = CACHE_Get(stringId, false);
      if( ptable_ == nullptr )
      {
         table* ptable_ = new table( 0u, { {"rstring", 0, "path"}, {"uint64", 0, "size"}, {"double", 0, "date"}, {"string", 10, "extension"} }, gd::table::tag_prepare{} );
         std::unique_ptr<table> ptable(ptable_);
         CACHE_Add(std::move(*ptable_), stringId); // add it to internal application cache
      }
   }
}

/** ---------------------------------------------------------------------------
 * @brief Load cache 
 * @param stringId id for cached table, only one id for cache is able to exist
 * @return true if cache was loaded, fals and error information on error
 */
std::pair<bool, std::string> CDocument::CACHE_Load( const std::string_view& stringId )
{                                                                                                  assert( m_papplication != nullptr ); assert( CACHE_Exists_d( stringId ) == false );
   /*
   gd::argument::arguments argumentsCache = CACHE_GetInformation( stringId );
   if( argumentsCache.empty() == true )
   {                                                                                               // LOG_WARNING( "No cache information for " <<  stringId );
      return { false, "" };
   }

   auto argumentLanguage = argumentsCache["language"];

   if( argumentLanguage == "sql" )                                             // create cache from sql select query, structure for cached table is generated from executed sql
   {
      gd::database::database_i* pdatabase = m_papplication->GetDatabaseMain(); 
      gd::table::dto::table tableCache;
      auto stringSelect = argumentsCache["value"].as_string();
      auto [bOk, stringError] = application::database::SQL_SelectToTable_g( pdatabase, stringSelect, &tableCache );// run query and add result to table
      if( bOk == false ) return { false, stringError };
      CACHE_Add( std::move( tableCache ), stringId );                          // add it to internal application cache
   }
   else if( argumentLanguage == "table" )
   {
      std::string stringColumnDesign = argumentsCache["value"].as_string();
      gd::table::dto::table tableCache( 10, gd::table::tag_full_meta{} );
      auto result_ = tableCache.column_add( stringColumnDesign, gd::table::tag_parse{});
      if( result_.first == false )
      {                                                                                            assert( false );
         return { false, fmt::format( "Error in string used to configure table columns - {}", stringColumnDesign ) };
      }
      tableCache.prepare();
      CACHE_Add( std::move( tableCache ), stringId );                          // add it to internal application cache
   }
   */
   return { true, "" };
}

// 0TAG0CACHE
/** ---------------------------------------------------------------------------
 * @brief Add table to document, table may be used as a sort of cache for data stored as table
 * @param table 
 * @param stringId 
 * @return true if added, false if table with id was found 
 */
bool CDocument::CACHE_Add( gd::table::dto::table&& table, const std::string_view& stringId )
{
   std::string_view stringTableId( stringId );
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );       // locks `m_vectorTableCache`

   if( stringTableId.empty() == true ) { stringTableId = ( const char* )table.property_get( "id" ); }

   // ## There is a tiny chance table was added before this method was called, we need to check with exclusive lock
   for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
   {
      auto argumentId = (*it)->property_get( "id" );
      if( argumentId.is_string() && stringTableId == (const char *)argumentId ) return false; // found table, exit
   }

   /// Create unique_ptr with table and move table data to this table
   std::unique_ptr<gd::table::dto::table> ptable = std::make_unique<gd::table::dto::table>( std::move( table ) );

   if( stringId.empty() == false )
   {                                                                                               assert( ptable->property_get( "id" ).is_null() );
      ptable->property_set( { "id", stringTableId } );
   }
   m_vectorTableCache.push_back( std::move( ptable ) );                        // insert table to vector

   return true;
}

/** ---------------------------------------------------------------------------
 * @brief Get pointer to table with specified id
 * @param stringId id to table that is returned
 * @return pointer to table with id
 */
gd::table::dto::table* CDocument::CACHE_Get( const std::string_view& stringId, bool bLoad )
{
   {
      std::shared_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );

      for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
      {
         auto argumentId = (*it)->property_get( "id" );
         if( argumentId.is_string() && stringId == ( const char* )argumentId ) return it->get();
      }
   }

   if( bLoad == true )
   {
      auto [bOk, stringError] = CACHE_Load( stringId );                                            // LOG_WARNING_IF( bOk == false, "Failed to find script: " << stringId );

      if( bOk == true ) return CACHE_Get( stringId, false );
      else
      {                                                                        // store internal error
         ERROR_Add( stringError );
      }
   }

   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Get information about cache to be able to generate data for it
 * 
 * @code
auto ptableAtoms = pdocument->CACHE_Get( "atoms" );
if( ptableAtoms == nullptr )
{
	gd::table::dto::table table;

   gd::argument::arguments argumentsCache = pdocument->CACHE_GetInformation( "atoms" );
   std::string stringSelect = argumentsCache["value"].as_string();
	auto [bOk, stringError] = application::database::SQL_SelectToTable_g( &databaseRead, stringSelect, &table );

	pdocument->CACHE_Add( std::move( table ), "atoms" );
}

ptableAtoms = pdocument->CACHE_Get( "atoms" );
 * @endcode
 * 
 * xml format with cache information
 * @verbatim
<document>
   <tables>
      <table id="atoms" language="sql" operation="select"><![CDATA[
SELECT FName AS "name", FMass AS "mass", FRadius AS "radius" FROM TAtom
      ]]></table>
   </tables>
</document>
 * @endverbatim
 * 
 * @param stringId id to cache information
 * @param argumentsCache arguments items where cache information is placed
 * @return true if information was found, false and error information if not found
 */
std::pair<bool, std::string> CDocument::CACHE_GetInformation( const std::string_view& stringId, gd::argument::arguments& argumentsCache )
{                                                                                                  assert( std::filesystem::exists( m_stringCacheConfiguration ) == true );
   pugi::xml_document xmldocument;           // read cache information from xml
   pugi::xml_parse_result xmlparseresult = xmldocument.load_file(m_stringCacheConfiguration.c_str()); // loads information about the table structure that is stored in cache, file may be named to `cache.xml`
   if( true == xmlparseresult )
   {
      std::string stringXpathTable = std::format("//table[@id='{}']", stringId );
      pugi::xml_node xmlnode = xmldocument.select_node( stringXpathTable.c_str() ).node();// find cache in xml
      
      if( xmlnode.empty() == false )                                           // found cache in xml
      {
         argumentsCache.append( "id", stringId );
         auto pbszLanguage = xmlnode.attribute("language").value();
         if( *pbszLanguage != '\0' ) argumentsCache.append( "language", pbszLanguage );
         auto pbszOperation = xmlnode.attribute("operation").value();
         if( *pbszOperation != '\0' ) argumentsCache.append( "pbszOperation", pbszOperation );
         argumentsCache.append( "value", xmlnode.first_child().value() );
      }
      else
      {
         auto stringError = std::format( "failed to find cache information for \"{}\"", stringId );
         return {false, stringError };
      }
   }
   else
   {
      std::string stringError = xmlparseresult.description();
      stringError += " [";
      stringError += m_stringCacheConfiguration;
      stringError += "]";
      return { false, stringError };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Get information about cache to be able to generate data for it
 * @param stringId id to cache information
 * @return gd::argument::arguments information about how to generate cache data
 */
gd::argument::arguments CDocument::CACHE_GetInformation( const std::string_view& stringId )
{
   gd::argument::arguments argumentsCache;   // collect cache information in arguments
   auto [bOk, stringError] = CACHE_GetInformation( stringId, argumentsCache );
   if( bOk == false )
   {
      ERROR_Add( stringError );
      // throw std::runtime_error( stringError ); TDOD: error logic
   }

   return argumentsCache;
}

/** ---------------------------------------------------------------------------
 * @brief Erase table cache
 * @param stringId id for cache to delete
 */
void CDocument::CACHE_Erase( const std::string_view& stringId ) 
{
   if(CACHE_Get(stringId, false) != nullptr)
   {
      std::shared_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );
      for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
      {
         auto argumentId = (*it)->property_get( "id" );
         if( argumentId.is_string() && stringId == ( const char* )argumentId )
         {
            m_vectorTableCache.erase( it );
            break;
         }
      }
   }
}


#ifndef NDEBUG
/// For debug, check if chache with id exists
bool CDocument::CACHE_Exists_d( const std::string_view& stringId )
{
   for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
   {
      auto argumentId = (*it)->property_get( "id" );
      if( argumentId.is_string() && stringId == ( const char* )argumentId ) return true;
   }
   return false;
}
#endif // !NDEBUG


/** ---------------------------------------------------------------------------
 * @brief Add error to internal list of errors
 * @param stringError error information
 */
void CDocument::ERROR_Add( const std::string_view& stringError )
{
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexError );            // locks `m_vectorError`
   gd::argument::arguments argumentsError( { {"text", stringError} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsError) );
}
