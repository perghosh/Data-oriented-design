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
#include "gd/gd_utf8.h"

#include "Command.h"
#include "Application.h"

#include "Document.h"

void CDocument::common_construct(const CDocument& o)
{
   m_arguments = o.m_arguments;
   m_vectorError = o.m_vectorError;
}

void CDocument::common_construct(CDocument&& o) noexcept
{
   m_arguments = std::move(o.m_arguments);
   m_vectorError = std::move(o.m_vectorError);
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
std::pair<bool, std::string> CDocument::FILE_Harvest(const gd::argument::shared::arguments& argumentsPath)
{
   CACHE_Prepare("file");
   CACHE_Prepare("file-count");
   auto* ptable_ = CACHE_Get("file");                                                              assert( ptable_ != nullptr );


   auto result_ = FILES_Harvest_g(argumentsPath, ptable_);
   if( result_.first == false ) return result_;

   //auto ptableCount =  std::make_unique<gd::table::dto::table>( gd::table::dto::table( 0u, { {"rstring", 0, "path"}, {"uint64", 0, "count"}, {"uint64", 0, "comment"}, {"uint64", 0, "space"} }, gd::table::tag_prepare{} ) );


   /*
   for( const auto& itRow : *ptable_ )
   {
      auto value_ = itRow.cell_get_variant_view( "path" );
      std::string stringFile = value_.as_string();
      
      auto uRow = ptableCount->get_row_count();
      ptableCount->row_add();

      uint64_t uCount = RowCount(stringFile);

      ptableCount->cell_set(uRow, "path", stringFile);
      ptableCount->cell_set(uRow, "count", uCount);
   }
   //auto stringTable = gd::table::to_string( *ptableCount, gd::table::tag_io_cli{});
   //std::cout << "\n" << stringTable << "\n";

   //CountRowsInFile(*ptable_); // TODO: remove this line, it is only for debug
   */
   return result_;
}

std::pair<bool, std::string> CDocument::FILE_Filter(const std::string_view& stringFilter)
{                                                                                                  assert( stringFilter.empty() == false );
   std::vector<uint64_t> vectorRemoveRow;
   auto vectorPath = gd::utf8::split(stringFilter, ';');
   auto* ptableFile = CACHE_Get("file");                                                           assert(ptableFile != nullptr);

   for( uint64_t uRow = 0, uRowCount = ptableFile->size(); uRow < uRowCount; uRow++ )
   {
      bool bMatched = false;
      auto stringFilename = ptableFile->cell_get_variant_view( uRow, "filename" ).as_string_view();

      // ## match file against wildcards

      // ### go through filters to check for a match
      for( const auto& filter_ : vectorPath )
      {
         bool bMatch = gd::ascii::strcmp( stringFilename, filter_, gd::utf8::tag_wildcard{} );
         if( bMatch == true ) { bMatched = true; break; }
      }

      // ### if no match then add to list for delete
      if( bMatched == false )  { vectorRemoveRow.push_back( uRow ); }
   }

   if( vectorRemoveRow.empty() == false )
   {
      ptableFile->erase(vectorRemoveRow);
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Filters out binary files from the cache table.
 *
 * This method checks each file in the cache table and removes those that are
 * determined to be binary files. It uses a buffer to read the file content and
 * checks if it is text or binary.
 *
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `file` cache table must be prepared and available in the cache.
 * @post The `file` table is updated to remove binary files.
 */
std::pair<bool, std::string> CDocument::FILE_FilterBinaries()
{
   char piBuffer[1024]; // buffer used to check if file is binary or not
   std::vector<uint64_t> vectorRemoveRow;
   auto* ptableFile = CACHE_Get("file");                                                           assert(ptableFile != nullptr);
   for( uint64_t uRow = 0, uRowCount = ptableFile->size(); uRow < uRowCount; uRow++ )
   {
      // ## generate full file path

      auto stringFilename = ptableFile->cell_get_variant_view(uRow, "filename").as_string_view();
      auto stringFolder = ptableFile->cell_get_variant_view(uRow, "folder").as_string_view();
      gd::file::path pathFile(stringFolder);
      pathFile += stringFilename;
    
      std::string stringFile = pathFile.string();
      if( std::filesystem::is_regular_file(stringFile) == false ) { vectorRemoveRow.push_back(uRow); }
      else
      {
         // ## open file and check if it is a text file

         // Open filenn and read 1024 bytes into buffer
         std::ifstream file_(stringFile, std::ios::binary);
         if( file_.is_open() == false ) { vectorRemoveRow.push_back(uRow); continue; }
         file_.read(piBuffer, sizeof(piBuffer));
         auto uSize = file_.gcount();
         file_.close();
         if( uSize == 0 ) { vectorRemoveRow.push_back(uRow); continue; }
         // Check if file is binary
         bool bIsText = gd::utf8::is_text( piBuffer, uSize );
         if( bIsText == false ) { vectorRemoveRow.push_back(uRow); continue; }
      }
   }

   if( vectorRemoveRow.empty() == false )
   {
      ptableFile->erase(vectorRemoveRow);
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Updates row counters for files in the cache.
 *
 * This method synchronizes and updates the `file-count` cache table with row counters
 * for each file listed in the `file` cache table. It ensures that each file in the
 * `file` table has a corresponding entry in the `file-count` table, and calculates
 * the number of rows (lines) in each file.
 *
 * @details
 * - The method iterates through all rows in the `file` cache table.
 * - For each file, it checks if a corresponding entry exists in the `file-count` table
 *   by matching the `key` column in the `file` table with the `file-key` column in the
 *   `file-count` table.
 * - If no matching entry is found, a new row is added to the `file-count` table with
 *   the file's key, filename, and an initial count.
 * - The method constructs the full file path using the `folder` and `filename` columns
 *   from the `file` table.
 * - It calculates the number of rows (lines) in the file using the `COUNT_Row` function
 *   and updates the `count` column in the `file-count` table.
 *
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `file` and `file-count` cache tables must be prepared and available in the cache.
 * @post The `file-count` table is updated with row counters for all files in the `file` table.
 *
 * @note This method assumes that the `COUNT_Row` function is responsible for counting
 *       the rows (lines) in a file and returning the result in the `argumentsResult` object.
 */
std::pair<bool, std::string> CDocument::FILE_UpdateRowCounters()
{
   auto* ptableFile = CACHE_Get("file");                                                           assert( ptableFile != nullptr );
   auto* ptableFileCount = CACHE_Get("file-count");                                                assert( ptableFileCount != nullptr );

   for( const auto& itRowFile : *ptableFile )
   {
      int64_t iRowIndexCount = -1;
      uint64_t uFileKey = itRowFile.cell_get_variant_view("key");
      for( auto itRowCount = ptableFileCount->begin(); itRowCount != ptableFileCount->end(); ++itRowCount )
      {
         uint64_t key_ = itRowCount.cell_get_variant_view("file-key");
         if( key_ != uFileKey ) break;
         iRowIndexCount = (int64_t)itRowCount.get_row();
      }

      if( iRowIndexCount == -1 )
      {
         iRowIndexCount = ptableFileCount->get_row_count();
         ptableFileCount->row_add( gd::table::tag_null{} );
         ptableFileCount->cell_set( iRowIndexCount, "key", uint64_t(iRowIndexCount + 1));
         ptableFileCount->cell_set( iRowIndexCount, "file-key", itRowFile.cell_get_variant_view("key") );
         ptableFileCount->cell_set( iRowIndexCount, "filename", itRowFile.cell_get_variant_view("filename") );
      }

      // ## build full file path from table

      auto string_ = itRowFile.cell_get_variant_view("folder").as_string();
      gd::file::path pathFile(string_);
      string_ = itRowFile.cell_get_variant_view("filename").as_string();
      pathFile += string_;
      std::string stringFile = pathFile.string();
      

      gd::argument::shared::arguments argumentsResult;

      auto result_ = COMMAND_CollectFileStatistics( {{"source", stringFile} }, argumentsResult);
      if( result_.first == false ) { ERROR_Add( result_.second ); }
      
      uint64_t uCount = argumentsResult["count"].as_uint64();
      ptableFileCount->cell_set(iRowIndexCount, "count", uCount);
      if( argumentsResult["code"].is_null() == false )
      {
         ptableFileCount->cell_set(iRowIndexCount, "code", argumentsResult["code"].as_uint64());
         ptableFileCount->cell_set(iRowIndexCount, "characters", argumentsResult["characters"].as_uint64());
         ptableFileCount->cell_set(iRowIndexCount, "comment", argumentsResult["comment"].as_uint64());
         ptableFileCount->cell_set(iRowIndexCount, "string", argumentsResult["string"].as_uint64());
      }
   }

   return { true, "" };
}

std::pair<bool, std::string> CDocument::FILE_UpdatePatternCounters(const std::vector<std::string>& vectorPattern)
{                                                                                                  assert( vectorPattern.empty() == false ); assert( vectorPattern.size() < 64 ); // max 64 patterns
   using namespace gd::table::dto;
   constexpr unsigned uTableStyle = (table::eTableFlagNull64|table::eTableFlagRowStatus);
   // file-count table: key | file-key | path | count
   auto ptable_ = std::make_unique<table>( table( uTableStyle, { {"uint64", 0, "key"}, {"uint64", 0, "file-key"}, {"rstring", 0, "folder"}, {"rstring", 0, "filename"} } ) );

   std::vector<uint64_t> vectorCount; // vector storing results from COMMAND_CollectPatternStatistics

   for( const auto& itPattern : vectorPattern )
   {
      std::string stringPattern = itPattern;
      // ## shorten pattern to 15 characters
      std::string stringName = stringPattern.substr(0, 15);
      
      ptable_->column_add("uint64", 0, stringName, stringPattern);
   }

   auto result_ = ptable_->prepare();                                                              assert( result_.first == true );

   CACHE_Add(std::move(*ptable_), "file-pattern");                            // add it to internal application cache, table is called "file-pattern"

   auto* ptableFilePattern = CACHE_Get("file-pattern", false);                // get it to make sure it is in cache

   auto* ptableFile = CACHE_Get("file");                                                           assert( ptableFile != nullptr );

   for( const auto& itRowFile : *ptableFile )
   {
      // ## generate full file path (folder + filename)
      auto string_ = itRowFile.cell_get_variant_view("folder").as_string();
      gd::file::path pathFile(string_);
      string_ = itRowFile.cell_get_variant_view("filename").as_string();
      pathFile += string_;
      std::string stringFile = pathFile.string();

      auto uRow = ptableFilePattern->get_row_count();
      ptableFilePattern->row_add( gd::table::tag_null{} );
      ptableFilePattern->cell_set( uRow, "key", uint64_t(uRow + 1));
      ptableFilePattern->cell_set( uRow, "file-key", itRowFile.cell_get_variant_view("key") );
      ptableFilePattern->cell_set( uRow, "folder", itRowFile.cell_get_variant_view("folder") );
      ptableFilePattern->cell_set( uRow, "filename", itRowFile.cell_get_variant_view("filename") );

      auto result_ = COMMAND_CollectPatternStatistics( {{"source", stringFile} }, vectorPattern, vectorCount );
      for( unsigned u = 0; u < vectorCount.size(); u++ )
      {
         ptableFilePattern->cell_set(uRow, u + 4, vectorCount[u]);             // set pattern count in table
      }

      vectorCount.resize(vectorPattern.size(), 0);                             // set counters to 0 in vector
   }

   return { true, "" };
}



std::pair<bool, std::string> CDocument::RESULT_Save(const gd::argument::shared::arguments& argumentsResult, const gd::table::dto::table* ptableResult)
{
   std::string stringType = argumentsResult["type"].as_string();               // type of result
   std::string stringOutput = argumentsResult["output"].as_string();           // output file name, could be a database

   if( stringOutput.empty() == true ) { return { false, "No output file specified" }; }

   gd::file::path pathFile(stringOutput);
   std::string stringExtension = pathFile.extension().string();

   // convert string to lowercase
   std::transform(stringExtension.begin(), stringExtension.end(), stringExtension.begin(), ::tolower);

   std::string stringResult;

   if( stringType == "COUNT" )
   {
      if( stringExtension == ".csv" )
      {
         stringResult = gd::table::to_string(*ptableResult, gd::table::tag_io_header{}, gd::table::tag_io_csv{}); // save table to string
      }
      else if( stringExtension == ".sql" )
      {
         std::string stringTableName = argumentsResult["table"].as_string();
         if( stringTableName.empty() == true ) stringTableName = pathFile.stem().string();
         stringResult = gd::table::write_insert_g( stringTableName, *ptableResult, gd::table::tag_io_sql{}); // save table to string
      }
   }

   if( stringOutput.empty() == false && stringResult.empty() == false )
   {
      std::ofstream file_(stringOutput, std::ios::binary);
      if( file_.is_open() == false ) return { false, "Failed to open file: " + stringOutput };
      file_.write(stringResult.data(), stringResult.size());
      file_.close();
   }



   /*
   else if( stringType == "XML" )
   {
      auto result_ = gd::table::to_file(*ptableResult, stringOutput, gd::table::tag_io_xml{});
      if( result_.first == false ) return { false, result_.second };
   }
   else if( stringType == "SQL" )
   {
      auto result_ = gd::table::to_file(*ptableResult, stringOutput, gd::table::tag_io_sql{});
      if( result_.first == false ) return { false, result_.second };
   }
   */
   
   return { true, "" };
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
   constexpr unsigned uTableStyle = (table::eTableFlagNull32|table::eTableFlagRowStatus);

   auto ptableFind = CACHE_Get(stringId, false);
   if( ptableFind != nullptr ) return;                                         // table already exists, exit

   // ## prepare file list
   //    columns: "path, size, date, extension
   if( stringId == "file" )                                                    // file cache, used to store file information
   {
      auto ptable_ = CACHE_Get(stringId, false);
      if( ptable_ == nullptr )
      {
         // file table: key | path | size | date | extension
         auto ptable_ = std::make_unique<table>( table( uTableStyle, { {"uint64", 0, "key"}, {"rstring", 0, "folder"}, {"rstring", 0, "filename"}, {"uint64", 0, "size"}, {"double", 0, "date"}, {"string", 10, "extension"} }, gd::table::tag_prepare{} ) );
         CACHE_Add(std::move(*ptable_), stringId); // add it to internal application cache
      }
   }
   else if( stringId == "file-count" )
   {
      auto ptable_ = CACHE_Get(stringId, false);
      if( ptable_ == nullptr )
      {
         // file-count table: key | file-key | path | count
         auto ptable_ = std::make_unique<table>( table( uTableStyle, 
            { {"uint64", 0, "key"}, {"uint64", 0, "file-key"}, {"rstring", 0, "filename"}, 
              {"uint64", 0, "count"}, {"uint64", 0, "code"}, {"uint64", 0, "characters"}, {"uint64", 0, "comment"}, {"uint64", 0, "string"} }, gd::table::tag_prepare{} )
         );
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

/// @brief Dump cache data to string
std::string CDocument::CACHE_Dump(const std::string_view& stringId) 
{  
   const auto* ptable_ = CACHE_Get(stringId, false);
   if( ptable_ == nullptr ) { return "Cache not found for ID: " + std::string(stringId); }

   std::string stringCliTable = gd::table::to_string(*ptable_, gd::table::tag_io_cli{});

   return stringCliTable;
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

/// generate result from file counting, table has the folder taken from file table and filename from file-count table in cache
/// Result columns: "folder, filename, count
gd::table::dto::table CDocument::RESULT_RowCount()
{
   using namespace gd::table::dto;
   // Define the result table structure
   constexpr unsigned uTableStyle = (table::eTableFlagNull32 | table::eTableFlagRowStatus);
   table tableResult(uTableStyle, {{"rstring", 0, "folder"}, {"rstring", 0, "filename"}, {"uint64", 0, "count"}, {"uint64", 0, "code"}, {"uint64", 0, "characters"}, {"uint64", 0, "comment"}, {"uint64", 0, "string"}}, gd::table::tag_prepare{});

   // Retrieve the file and file-count cache tables
   auto* ptableFile = CACHE_Get("file", false);                                                    assert( ptableFile != nullptr );
   auto* ptableFileCount = CACHE_Get("file-count", false);                                         assert( ptableFileCount != nullptr );

   // ## Iterate through the rows in the file-count table
   for (const auto& itRowCount : *ptableFileCount)
   {
      uint64_t iFileKey = itRowCount.cell_get_variant_view("file-key").as_uint64();
      uint64_t uCount = itRowCount.cell_get_variant_view("count").as_uint64();
      auto stringFilename = itRowCount.cell_get_variant_view("filename").as_string();

      // Find the corresponding row in the file table using the file key
      for (const auto& itRowFile : *ptableFile)
      {
         if (itRowFile.cell_get_variant_view("key").as_uint64() == iFileKey)
         {
            auto stringFolder = itRowFile.cell_get_variant_view("folder").as_string();

            // Add a new row to the result table
            auto uRow = tableResult.get_row_count();
            tableResult.row_add( gd::table::tag_null{} );
            tableResult.cell_set(uRow, "folder", stringFolder );
            tableResult.cell_set(uRow, "filename", stringFilename);
            tableResult.cell_set(uRow, "count", uCount);
            if( itRowCount.cell_get_variant_view("code").is_null() == false )
            {
               tableResult.cell_set(uRow, "code", itRowCount.cell_get_variant_view("code").as_uint64());
               tableResult.cell_set(uRow, "characters", itRowCount.cell_get_variant_view("characters").as_uint64());
               tableResult.cell_set(uRow, "comment", itRowCount.cell_get_variant_view("comment").as_uint64());
               tableResult.cell_set(uRow, "string", itRowCount.cell_get_variant_view("string").as_uint64());
            }

            break; // Exit the loop once the matching row is found
         }
      }
   }

   return tableResult;
}

/** ---------------------------------------------------------------------------
 * @brief Generate a result table with pattern counts.
 * @return A table containing the pattern counts for each file.
 */
gd::table::dto::table CDocument::RESULT_PatternCount()
{
   using namespace gd::table::dto;
   constexpr unsigned FIXED_COLUMN_COUNT = 2; // Number of fixed columns (folder and filename)
   // Define the result table structure
   constexpr unsigned uTableStyle = ( table::eTableFlagNull64 | table::eTableFlagRowStatus );
   table tableResult(uTableStyle, { {"rstring", 0, "folder"}, {"rstring", 0, "filename"} });
   // Retrieve the file-pattern cache table
   auto* ptableFilePattern = CACHE_Get("file-pattern", false);                                     assert(ptableFilePattern != nullptr);
   unsigned uColumnFileName = ptableFilePattern->column_get_index("filename") + 1; // get index for filename column
   for( auto it = ptableFilePattern->column_begin() + uColumnFileName; it != ptableFilePattern->column_end(); ++it ) { tableResult.column_add( *it, *ptableFilePattern ); }
   tableResult.prepare();                                                        // prepare table, this will allocate internal memory for the table                  


   std::vector<gd::variant_view> vectorPatternCount;
   // ## Iterate through the rows in the file-pattern table
   for( const auto& itRowCount : *ptableFilePattern )
   {
      auto stringFilename = itRowCount.cell_get_variant_view("filename").as_string();
      auto stringFolder = itRowCount.cell_get_variant_view("folder").as_string();
      auto uRowSource = itRowCount.get_row();
      ptableFilePattern->row_get_variant_view(uRowSource, uColumnFileName, vectorPatternCount); // get row data from table
      // Add a new row to the result table
      auto uRow = tableResult.get_row_count();
      tableResult.row_add(gd::table::tag_null{});
      tableResult.cell_set(uRow, "folder", stringFolder);
      tableResult.cell_set(uRow, "filename", stringFilename);

      tableResult.row_set(uRow, FIXED_COLUMN_COUNT,  vectorPatternCount);      // set row data to table from column 2, after filename

      vectorPatternCount.clear(); // clear vector for next row
   }
   return tableResult;
}




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
