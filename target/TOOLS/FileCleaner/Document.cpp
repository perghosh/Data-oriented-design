/**
* \file Document.cpp
* 
* ### 0TAG0 File navigation, mark and jump to common parts
* - `0TAG0CACHE.Document` - cache methods

*/

#include <filesystem>
#include <format>
#include <fstream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>

#include "pugixml/pugixml.hpp"

#include "gd/gd_file.h"
#include "gd/gd_table_io.h"
#include "gd/gd_utf8.h"
#include "gd/math/gd_math_string.h"
#include "gd/parse/gd_parse_formats.h"

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

   // ## Check for verbose mode
   if( GetApplication()->PROPERTY_Get("verbose").as_bool() == true )
   {
      auto vector_ = argumentsPath.get_argument_all("source", gd::types::tag_view{}); // get all source paths
      if( vector_.size() == 1 )
      {
         std::string stringSource( vector_[0] );
         auto vectorPath = gd::utf8::split(stringSource, ';');   
         for( const auto& s_ : vectorPath ) { MESSAGE_Display(std::format("source: {}", s_)); }
      }
      else if( vector_.size() > 1 )
      {
         for( const auto& s_ : vector_ ) { MESSAGE_Display(std::format("source: {}", s_.as_string_view())); }
      }

      auto uCount = argumentsPath["recursive"].as_uint();
      MESSAGE_Display(std::format("recursive: {}", uCount));
   }

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


/** ---------------------------------------------------------------------------
 * @brief Harvests file information and filters based on the provided arguments.
 *
 * This method combines the functionality of harvesting file information and filtering
 * it based on a specified string filter. It first calls FILE_Harvest to gather file
 * information, and then applies the filter if provided.
 *
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @param stringFilter The filter string to apply to the harvested files.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> CDocument::FILE_Harvest(const gd::argument::shared::arguments& argumentsPath, std::string stringFilter)
{
   auto result_ = FILE_Harvest(argumentsPath);
   if( result_.first == false ) return result_;

   if( stringFilter.empty() == false )
   {
      auto result_ = FILE_Filter(stringFilter);
      if( result_.first == false ) return result_;
   }
   else
   {
      auto result_ = FILE_FilterBinaries();
      if( result_.first == false ) return result_;
   }
   
   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Filters out files from the cache table based on a string filter.
 *
 * This method checks each file in the cache table against the provided string filter.
 * If a file does not match the filter, it is removed from the cache table.
 *
 * @param stringFilter The filter string to apply to the filenames. Not that this is a wildcard filter and may contain multiple filters separated by ';'.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `file` cache table must be prepared and available in the cache.
 * @post The `file` table is updated to remove files that do not match the filter.
 */
std::pair<bool, std::string> CDocument::FILE_Filter(const std::string_view& stringFilter)
{                                                                                                  assert( stringFilter.empty() == false );
   std::vector<uint64_t> vectorRemoveRow;

   char iSplit = ';';                                                   // separator for wildcards
   auto uPosition = stringFilter.find_first_of(";,");
   if( uPosition != std::string_view::npos ) { iSplit = stringFilter[uPosition]; } // use the first separator found

   auto vectorWildcard = gd::utf8::split(stringFilter, iSplit);
   auto* ptableFile = CACHE_Get("file");                                                           assert(ptableFile != nullptr);

   for( uint64_t uRow = 0, uRowCount = ptableFile->size(); uRow < uRowCount; uRow++ )
   {
      bool bMatched = false;
      auto stringFilename = ptableFile->cell_get_variant_view( uRow, "filename" ).as_string_view();

      // ## match file against wildcards

      // ### go through filters to check for a match
      for( const auto& filter_ : vectorWildcard )
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

   uint64_t uFileIndex = 0; // index for file table
   auto uFileCount = ptableFile->get_row_count(); // get current row count in file-count table

   for( uint64_t uRow = 0, uRowCount = ptableFile->size(); uRow < uRowCount; uRow++ )
   {
      uFileIndex++;                                                            // increment file index for each file, used for progress message

      // ## calculate percentage for progress message

      if( uFileIndex % 10 == 0 ) // show progress message every 10 files
      {
         uint64_t uPercent = (uFileIndex * 100) / uFileCount;                 // calculate percentage of files processed
         MESSAGE_Progress("", { {"percent", uPercent}, {"label", "Scan files"}, {"sticky", true} });
      }


      // ## generate full file path

      auto stringFilename = ptableFile->cell_get_variant_view(uRow, "filename").as_string_view();
      auto stringFolder = ptableFile->cell_get_variant_view(uRow, "folder").as_string_view();
      gd::file::path pathFile(stringFolder);
      pathFile += stringFilename;

      
      std::string stringExtension = pathFile.extension().string();

      bool bIsText = CApplication::IsTextFile_s(stringExtension); // Check if file is text or binary based on extension
    
      if( bIsText == false )
      {
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
            bIsText = gd::utf8::is_text( piBuffer, uSize );
            if( bIsText == false ) { vectorRemoveRow.push_back(uRow); continue; }
         }
      }
   }

   if( vectorRemoveRow.empty() == false )
   {
      ptableFile->erase(vectorRemoveRow);
   }

   MESSAGE_Progress("", {{"clear", true}});

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
std::pair<bool, std::string> CDocument::FILE_UpdateRowCounters( int iThreadCount )
{
   auto* ptableFile = CACHE_Get("file");                                                           assert( ptableFile != nullptr );
   auto* ptableFileCount = CACHE_Get("file-count");                                                assert( ptableFileCount != nullptr );

   auto uFileCount = ptableFile->get_row_count();                             // Total number of files to process

   std::atomic<uint64_t> uAtomicFileIndex(0);  // Current file being processed
   std::atomic<uint64_t> uAtomicProcessedCount(0); // Count of processed files
   std::mutex mutexTableCount; // Mutex to protect access to file-count table
   std::mutex mutexProgress; // Mutex to protect progress updates
   std::vector<std::string> vectorError; // Collect errors from all threads
   std::mutex mutexErrors;  // Mutex to protect access to vectorError

   // ## Worker function to process files in parallel .........................
   auto process_ = [&](int iThreadId) -> void
   {
      while( true )
      {
         // Get next file index to process
         uint64_t uRowIndex = uAtomicFileIndex.fetch_add(1);                  // get thread safe current index and increment it
         if(uRowIndex >= uFileCount) { break; }
         
         try
         {
            // STEP 1: Get file info (no mutex needed - ptableFile is read-only)
            uint64_t uFileKey = ptableFile->cell_get_variant_view(uRowIndex, "key"); // Key for file
            auto stringFolder = ptableFile->cell_get_variant_view(uRowIndex, "folder").as_string();
            auto stringFilename = ptableFile->cell_get_variant_view(uRowIndex, "filename").as_string();

            // STEP 2: Build full file path (no mutex needed)
            gd::file::path pathFile(stringFolder);
            pathFile += stringFilename;
            std::string stringFile = pathFile.string();

            // STEP 3: Process file statistics (SLOW operation - no mutex needed, thread-safe)
            gd::argument::shared::arguments argumentsResult;
            auto result_ = COMMAND_CollectFileStatistics({{"source", stringFile}}, argumentsResult);

            if(result_.first == false)
            {
               std::lock_guard<std::mutex> lockErrors(mutexErrors);
               vectorError.push_back("File: " + stringFile + " - " + result_.second);

               // Update progress even on failure
               uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
               if(uProcessed % 10 == 0)
               {
                  std::lock_guard<std::mutex> lockProgress(mutexProgress);
                  uint64_t uPercent = (uProcessed * 100) / uFileCount;
                  MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Scan files"}, {"sticky", true}});
               }
               continue;                                                       // Skip to next file on error
            }

            // STEP 4: Update table (FAST operation - mutex needed due to potential reallocation)
            {
               std::lock_guard<std::mutex> lockFileCount(mutexTableCount);    // ✅ FIXED: Minimal mutex scope

               int64_t iRowIndexCount = -1;                                   // Index for file-count table, -1 means not found

               // Check if file key exists in file-count table
               for(auto itRowCount = ptableFileCount->begin(); itRowCount != ptableFileCount->end(); ++itRowCount)
               {
                  uint64_t uFKFileKey = itRowCount.cell_get_variant_view("file-key");
                  if(uFKFileKey == uFileKey) 
                  { 
                     iRowIndexCount = itRowCount.get_row(); 
                     break;                                                   // Found matching key
                  }
               }

               // Add new row if doesn't exist
               if(iRowIndexCount == -1)
               {
                  iRowIndexCount = ptableFileCount->get_row_count();
                  ptableFileCount->row_add(gd::table::tag_null{});           // Can cause reallocation
                  ptableFileCount->cell_set(iRowIndexCount, "key", uint64_t(iRowIndexCount + 1));
                  ptableFileCount->cell_set(iRowIndexCount, "file-key", uFileKey);
                  ptableFileCount->cell_set(iRowIndexCount, "filename", stringFilename);
               }

               // Update the statistics in the file-count table
               uint64_t uCount = argumentsResult["count"].as_uint64();
               ptableFileCount->cell_set(iRowIndexCount, "count", uCount);

               if(argumentsResult["code"].is_null() == false)                  // Update additional statistics if available
               {
                  ptableFileCount->cell_set(iRowIndexCount, "code", argumentsResult["code"].as_uint64());
                  ptableFileCount->cell_set(iRowIndexCount, "characters", argumentsResult["characters"].as_uint64());
                  ptableFileCount->cell_set(iRowIndexCount, "comment", argumentsResult["comment"].as_uint64());
                  ptableFileCount->cell_set(iRowIndexCount, "string", argumentsResult["string"].as_uint64());
               }
            }

            // Update progress (thread-safe)
            uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
            if(uProcessed % 10 == 0)                                           // Show progress every 10 files
            {
               std::lock_guard<std::mutex> lockProgress(mutexProgress);
               uint64_t uPercent = (uProcessed * 100) / uFileCount;
               MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Scan files"}, {"sticky", true}});
            }
         }
         catch(const std::exception& exception_)
         {
            std::lock_guard<std::mutex> lockErrors(mutexErrors);
            vectorError.push_back(std::string("Thread ") + std::to_string(iThreadId) + " error: " + exception_.what());
         }      
      }
   };

   // ## Prepare and run threads ..............................................

   if( iThreadCount <= 0 ) { iThreadCount = std::thread::hardware_concurrency(); } // Use hardware concurrency if no thread count is specified
   if(iThreadCount <= 0) { iThreadCount = 1; }                                 // Fallback to single thread if hardware_concurrency returns 0
   if( iThreadCount > 8 ) { iThreadCount = 8; }                                // Limit to 8 threads for performance and resource management

   // Create and launch worker threads
   std::vector<std::thread> vectorCountThread;
   vectorCountThread.reserve(iThreadCount);

   for(int i = 0; i < iThreadCount; ++i) { vectorCountThread.emplace_back(process_, i); }

   // Wait for all threads to complete
   for(auto& threadWorker : vectorCountThread) { threadWorker.join(); }
   
   MESSAGE_Progress("", {{"clear", true}});                                   // Clear progress message

   // ### Handle any collected errors
   if(!vectorError.empty())
   {
      for(const auto& stringError : vectorError)
      {
         ERROR_Add(stringError);
      }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Updates pattern counters for files in the cache using multithreading.
 *
 * This method processes a list of patterns and updates the "file-pattern" cache table
 * with the count of occurrences of each pattern in the files listed in the "file" cache table.
 * The method uses multithreading to process files in parallel.
 *
 * @param argumentsPattern Arguments for pattern collection (e.g., segment specification)
 * @param vectorPattern A vector of strings representing the patterns to search for.
 *                      The vector must not be empty and can contain a maximum of 64 patterns.
 * @param iThreadCount Number of threads to use (0 = auto-detect)
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `vectorPattern` must not be empty and must contain fewer than 64 patterns.
 * @post The "file-pattern" cache table is updated with the pattern counts for each file.
 *
 * @note COMMAND_CollectPatternStatistics must be thread-safe.
 */
std::pair<bool, std::string> CDocument::FILE_UpdatePatternCounters(const gd::argument::shared::arguments& argumentsPattern, const std::vector<std::string>& vectorPattern, int iThreadCount)
{                                                                                                  assert( vectorPattern.empty() == false ); assert( vectorPattern.size() < 64 ); // max 64 patterns
   using namespace gd::table::dto;
   constexpr unsigned uTableStyle = (table::eTableFlagNull64|table::eTableFlagRowStatus);
   // file-pattern table: key | file-key | folder | filename | pattern1 | pattern2 | ...
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

   auto uFileCount = ptableFile->get_row_count();                             // Total number of files to process

   // Pre-allocate all rows in file-pattern table to avoid reallocation during threading
   for(uint64_t u = 0; u < uFileCount; u++ )
   {
      ptableFilePattern->row_add( gd::table::tag_null{} );
      ptableFilePattern->cell_set( u, "key", uint64_t(u + 1));

      // Copy file info from file table to pattern table
      ptableFilePattern->cell_set( u, "file-key", ptableFile->cell_get_variant_view(u, "key") );
      ptableFilePattern->cell_set( u, "folder", ptableFile->cell_get_variant_view(u, "folder") );
      ptableFilePattern->cell_set( u, "filename", ptableFile->cell_get_variant_view(u, "filename") );
   }

   std::atomic<uint64_t> uAtomicFileIndex(0);                                 // Current file being processed
   std::atomic<uint64_t> uAtomicProcessedCount(0);                            // Count of processed files
   std::mutex mutexProgress;                                                  // Mutex to protect progress updates
   std::vector<std::string> vectorError;                                      // Collect errors from all threads
   std::mutex mutexErrors;                                                    // Mutex to protect access to vectorError

   // ## Worker function to process files in parallel .........................
   auto process_ = [&](int iThreadId) -> void
   {
      std::vector<uint64_t> vectorCount;                                      // Thread-local vector for pattern counts
      
      while( true )
      {
         // Get next file index to process
         uint64_t uRowIndex = uAtomicFileIndex.fetch_add(1);                  // get thread safe current index and increment it
         if(uRowIndex >= uFileCount) { break; }
         
         try
         {
            // STEP 1: Get file info (no mutex needed - ptableFile is read-only)
            auto stringFolder = ptableFile->cell_get_variant_view(uRowIndex, "folder").as_string();
            auto stringFilename = ptableFile->cell_get_variant_view(uRowIndex, "filename").as_string();

            // STEP 2: Build full file path (no mutex needed)
            gd::file::path pathFile(stringFolder);
            pathFile += stringFilename;
            std::string stringFile = pathFile.string();

            // STEP 3: Process pattern statistics (SLOW operation - no mutex needed, thread-safe)
            gd::argument::shared::arguments argumentsPattern_({ {"source", stringFile} });
            if( argumentsPattern.exists("segment") == true ) { argumentsPattern_.set("segment", argumentsPattern["segment"].as_string_view()); } // set the segment (code, comment, string) to search in
            
            auto result_ = COMMAND_CollectPatternStatistics( argumentsPattern_, vectorPattern, vectorCount );

            if(result_.first == false)
            {
               std::lock_guard<std::mutex> lockErrors(mutexErrors);
               vectorError.push_back("File: " + stringFile + " - " + result_.second);

               // Update progress even on failure
               uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
               if(uProcessed % 10 == 0)
               {
                  std::lock_guard<std::mutex> lockProgress(mutexProgress);
                  uint64_t uPercent = (uProcessed * 100) / uFileCount;
                  MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Pattern scan"}, {"sticky", true}});
               }
               continue;                                                       // Skip to next file on error
            }

            // STEP 4: Update table (FAST operation - no mutex needed, pre-allocated memory)
            for( unsigned u = 0; u < vectorCount.size(); u++ )
            {
               ptableFilePattern->cell_set(uRowIndex, u + 4, vectorCount[u]); // set pattern count in table (columns 0-3 are key, file-key, folder, filename)
            }

            // Clear counts for next iteration
            std::fill(vectorCount.begin(), vectorCount.end(), 0);

            // Update progress (thread-safe)
            uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
            if(uProcessed % 10 == 0)                                           // Show progress every 10 files
            {
               std::lock_guard<std::mutex> lockProgress(mutexProgress);
               uint64_t uPercent = (uProcessed * 100) / uFileCount;
               MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Pattern scan"}, {"sticky", true}});
            }
         }
         catch(const std::exception& exception_)
         {
            std::lock_guard<std::mutex> lockErrors(mutexErrors);
            vectorError.push_back(std::string("Thread ") + std::to_string(iThreadId) + " error: " + exception_.what());
         }      
      }
   };

   // ## Prepare and run threads ..............................................

   if( iThreadCount <= 0 ) { iThreadCount = std::thread::hardware_concurrency(); } // Use hardware concurrency if no thread count is specified
   if(iThreadCount <= 0) { iThreadCount = 1; }                                 // Fallback to single thread if hardware_concurrency returns 0
   if( iThreadCount > 8 ) { iThreadCount = 8; }                                // Limit to 8 threads for performance and resource management

   // Create and launch worker threads
   std::vector<std::thread> vectorPatternThread;
   vectorPatternThread.reserve(iThreadCount);

   for(int i = 0; i < iThreadCount; ++i) { vectorPatternThread.emplace_back(process_, i); }

   // Wait for all threads to complete
   for(auto& threadWorker : vectorPatternThread) { threadWorker.join(); }

   MESSAGE_Progress("", {{"clear", true}});                                   // Clear progress message

   // ### Handle any collected errors
   if(!vectorError.empty())
   {
      for(const auto& stringError : vectorError)
      {
         ERROR_Add(stringError);
      }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Updates pattern counters for files in the cache using multithreading.
 *
 * This method processes all files in the `file` cache table and counts occurrences
 * of specified regex patterns in each file. Results are stored in a new `file-pattern`
 * cache table. The method uses multithreading to process files in parallel.
 *
 * @param argumentsPattern Arguments for pattern collection (e.g., segment specification)
 * @param vectorRegexPatterns Vector of regex patterns to search for in files
 * @param iThreadCount Number of threads to use (0 = auto-detect)
 *
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `file` cache table must be prepared and available in the cache.
 * @post The `file-pattern` table is created and populated with pattern counters.
 *
 * @note COMMAND_CollectPatternStatistics must be thread-safe.
 */
std::pair<bool, std::string> CDocument::FILE_UpdatePatternCounters(const gd::argument::shared::arguments& argumentsPattern, const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPatterns, int iThreadCount)
{                                                                                                  assert( vectorRegexPatterns.empty() == false ); assert( vectorRegexPatterns.size() < 64 ); // max 64 patterns
   using namespace gd::table::dto;
   constexpr unsigned uTableStyle = (table::eTableFlagNull64|table::eTableFlagRowStatus);
   // file-count table: key | file-key | path | count
   auto ptable_ = std::make_unique<table>( table( uTableStyle, { {"uint64", 0, "key"}, {"uint64", 0, "file-key"}, {"rstring", 0, "folder"}, {"rstring", 0, "filename"} } ) );

   std::vector<uint64_t> vectorCount; // vector storing results from COMMAND_CollectPatternStatistics

   for( const auto& itPattern : vectorRegexPatterns )
   {
      std::string stringPattern = itPattern.second;
      std::string stringName = stringPattern;                    
      if( stringName.length() > 15 ) { stringName = stringName.substr(15) +  "..."; } // shorten pattern to 15 characters
      ptable_->column_add("uint64", 0, stringName, stringPattern);
   }

   auto result_ = ptable_->prepare();                                                              assert( result_.first == true );

   CACHE_Add(std::move(*ptable_), "file-pattern");                            // add it to internal application cache, table is called "file-pattern"
   auto* ptableFilePattern = CACHE_Get("file-pattern", false);                // get it to make sure it is in cache
   auto* ptableFile = CACHE_Get("file");                                                           assert( ptableFile != nullptr );

   auto uFileCount = ptableFile->get_row_count();                             // Total number of files to process

   // Pre-allocate all rows in file-pattern table to avoid reallocation during threading
   for(uint64_t u = 0; u < uFileCount; u++ )
   {
      ptableFilePattern->row_add( gd::table::tag_null{} );
      ptableFilePattern->cell_set( u, "key", uint64_t(u + 1));

      // Copy file info from file table to pattern table
      ptableFilePattern->cell_set( u, "file-key", ptableFile->cell_get_variant_view(u, "key") );
      ptableFilePattern->cell_set( u, "folder", ptableFile->cell_get_variant_view(u, "folder") );
      ptableFilePattern->cell_set( u, "filename", ptableFile->cell_get_variant_view(u, "filename") );
   }

   std::atomic<uint64_t> uAtomicFileIndex(0);                                 // Current file being processed
   std::atomic<uint64_t> uAtomicProcessedCount(0);                           // Count of processed files
   //std::mutex mutexTablePattern;                                              // Mutex to protect access to file-pattern table
   std::mutex mutexProgress;                                                  // Mutex to protect progress updates
   std::vector<std::string> vectorError;                                     // Collect errors from all threads
   std::mutex mutexErrors;                                                    // Mutex to protect access to vectorError

   // ## Worker function to process files in parallel .........................
   auto process_ = [&](int iThreadId) -> void
   {
      std::vector<uint64_t> vectorCount;                                      // Thread-local vector for pattern counts
      
      while( true )
      {
         // Get next file index to process
         uint64_t uRowIndex = uAtomicFileIndex.fetch_add(1);                  // get thread safe current index and increment it
         if(uRowIndex >= uFileCount) { break; }
         
         try
         {
            // STEP 1: Get file info (no mutex needed - ptableFile is read-only)
            auto stringFolder = ptableFile->cell_get_variant_view(uRowIndex, "folder").as_string();
            auto stringFilename = ptableFile->cell_get_variant_view(uRowIndex, "filename").as_string();

            // STEP 2: Build full file path (no mutex needed)
            gd::file::path pathFile(stringFolder);
            pathFile += stringFilename;
            std::string stringFile = pathFile.string();

            // STEP 3: Process pattern statistics (SLOW operation - no mutex needed, thread-safe)
            gd::argument::shared::arguments argumentsPattern_(argumentsPattern);  // Copy base arguments
            argumentsPattern_.set("source", stringFile);
            
            auto result_ = COMMAND_CollectPatternStatistics( argumentsPattern_, vectorRegexPatterns, vectorCount );

            if(result_.first == false)
            {
               std::lock_guard<std::mutex> lockErrors(mutexErrors);
               vectorError.push_back("File: " + stringFile + " - " + result_.second);

               // Update progress even on failure
               uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
               if(uProcessed % 10 == 0)
               {
                  std::lock_guard<std::mutex> lockProgress(mutexProgress);
                  uint64_t uPercent = (uProcessed * 100) / uFileCount;
                  MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Pattern scan"}, {"sticky", true}});
               }
               continue;                                                       // Skip to next file on error
            }

            // STEP 4: Update table (FAST operation - mutex needed for thread safety)
            {
               //std::lock_guard<std::mutex> lockFilePattern(mutexTablePattern); // ✅ Minimal mutex scope

               // Update pattern counters in the file-pattern table
               for( unsigned u = 0; u < vectorCount.size(); u++ )
               {
                  ptableFilePattern->cell_set(uRowIndex, u + 4, vectorCount[u]); // set pattern count in table (columns 0-3 are key, file-key, folder, filename)
               }
            }

            // Clear counts for next iteration
            std::fill(vectorCount.begin(), vectorCount.end(), 0);

            // Update progress (thread-safe)
            uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
            if(uProcessed % 10 == 0)                                           // Show progress every 10 files
            {
               std::lock_guard<std::mutex> lockProgress(mutexProgress);
               uint64_t uPercent = (uProcessed * 100) / uFileCount;
               MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Pattern scan"}, {"sticky", true}});
            }
         }
         catch(const std::exception& exception_)
         {
            std::lock_guard<std::mutex> lockErrors(mutexErrors);
            vectorError.push_back(std::string("Thread ") + std::to_string(iThreadId) + " error: " + exception_.what());
         }      
      }
   };

   // ## Prepare and run threads ..............................................

   if( iThreadCount <= 0 ) { iThreadCount = std::thread::hardware_concurrency(); } // Use hardware concurrency if no thread count is specified
   if(iThreadCount <= 0) { iThreadCount = 1; }                                 // Fallback to single thread if hardware_concurrency returns 0
   if( iThreadCount > 8 ) { iThreadCount = 8; }                                // Limit to 8 threads for performance and resource management

   // Create and launch worker threads
   std::vector<std::thread> vectorPatternThread;
   vectorPatternThread.reserve(iThreadCount);

   for(int i = 0; i < iThreadCount; ++i) { vectorPatternThread.emplace_back(process_, i); }

   // Wait for all threads to complete
   for(auto& threadWorker : vectorPatternThread) { threadWorker.join(); }

   MESSAGE_Progress("", {{"clear", true}});                                   // Clear progress message

   // ### Handle any collected errors
   if(!vectorError.empty())
   {
      for(const auto& stringError : vectorError)
      {
         ERROR_Add(stringError);
      }
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Updates the pattern list for files in the cache using multithreading.
 *
 * This method processes a list of patterns and applies them to the files stored in the cache.
 * It generates a list of lines in each file where the patterns are found and stores the results
 * in the "file-linelist" cache table. The method uses multithreading to process files in parallel.
 *
 * @param vectorPattern A vector of strings representing the patterns to search for.
 *                      The vector must not be empty and can contain a maximum of 64 patterns.
 * @param argumentsList Arguments for pattern processing (e.g., segment specification, max lines)
 * @param iThreadCount Number of threads to use (0 = auto-detect)
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `vectorPattern` must not be empty and must contain fewer than 64 patterns.
 * @post The "file-linelist" cache table is updated with the lines where the patterns are found.
 *
 * @note COMMAND_ListLinesWithPattern must be thread-safe.
 */
std::pair<bool, std::string> CDocument::FILE_UpdatePatternList(const std::vector<std::string>& vectorPattern, const gd::argument::shared::arguments& argumentsList, int iThreadCount)
{                                                                                                  assert(vectorPattern.empty() == false); // Ensure the pattern list is not empty
                                                                                                   assert(vectorPattern.size() < 64);      // Ensure the pattern list contains fewer than 64 patterns
   using namespace gd::table;
   gd::parse::patterns patternsFind(vectorPattern);
   patternsFind.sort();                                                       // Sort patterns by length, longest first (important for pattern matching)

   auto* ptableFile = CACHE_Get("file");                                      // Retrieve the "file" cache table
   auto* ptableLineList = CACHE_Get("file-linelist", true);                   // Ensure the "file-linelist" table is in cache
                                                                                                   assert(ptableFile != nullptr); assert(ptableLineList != nullptr);

   std::string_view stringSegment;
   if( argumentsList.exists("segment") == true ) { stringSegment = argumentsList["segment"].as_string_view(); } // Get the segment (code, comment, string) to search in

   uint64_t uMax = argumentsList["max"].as_uint64();                          // Get the maximum number of lines to process
   auto uFileCount = ptableFile->get_row_count();                             // Total number of files to process

   // ## Thread synchronization variables

   std::atomic<uint64_t> uAtomicFileIndex(0);                                 // Current file being processed
   std::atomic<uint64_t> uAtomicProcessedCount(0);                            // Count of processed files
   std::atomic<uint64_t> uAtomicTotalLines(0);                                // Total lines found across all threads
   std::mutex mutexProgress;                                                  // Mutex to protect progress updates
   std::mutex mutexLineList;                                                  // Mutex to protect ptableLineList access
   std::vector<std::string> vectorError;                                      // Collect errors from all threads
   std::mutex mutexErrors;                                                    // Mutex to protect access to vectorError

   // ## Prepare columns for line list table 
   detail::columns* pcolumnsThread = new detail::columns{};
   ptableLineList->to_columns( *pcolumnsThread );

   // ## Worker function to process files in parallel .........................
   auto process_ = [&](int iThreadId) -> void
   {
      // Create thread-local table for collecting results
      std::unique_ptr<table> ptableLineListLocal = std::make_unique<table>(pcolumnsThread, 10, ptableLineList->get_flags(), 10); // Create local table with 10 rows pre-allocated
      
      while( true )
      {
         // Get next file index to process
         uint64_t uRowIndex = uAtomicFileIndex.fetch_add(1);                  // get thread safe current index and increment it
         if(uRowIndex >= uFileCount) { break; }
         
         try
         {
            // STEP 1: Get file info (ptableFile is read-only so no mutex needed)
            auto stringFolder = ptableFile->cell_get_variant_view(uRowIndex, "folder").as_string();
            auto stringFilename = ptableFile->cell_get_variant_view(uRowIndex, "filename").as_string();

            // STEP 2: Build full file path
            gd::file::path pathFile(stringFolder);
            pathFile += stringFilename;
            std::string stringFile = pathFile.string();

            auto uKey = ptableFile->cell_get_variant_view(uRowIndex, "key").as_uint64();

            // STEP 3: Find lines with patterns 
            gd::argument::shared::arguments arguments_({{"source", stringFile}, {"file-key", uKey}});
            if( stringSegment.empty() == false ) arguments_.set("segment", stringSegment.data()); // Set the segment (code, comment, string) to search in
            
            auto result_ = COMMAND_ListLinesWithPattern(arguments_, patternsFind, ptableLineListLocal.get()); // Find lines with patterns and update the local table, ptableLineListLocal is thread-local

            if(result_.first == false)
            {
               std::lock_guard<std::mutex> lockErrors(mutexErrors);
               vectorError.push_back("File: " + stringFile + " - " + result_.second);

               // Update progress even on failure
               uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
               if(uProcessed % 10 == 0)
               {
                  std::lock_guard<std::mutex> lockProgress(mutexProgress);
                  uint64_t uPercent = (uProcessed * 100) / uFileCount;
                  MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Find in files"}, {"sticky", true}});
               }
               continue;                                                       // Skip to next file on error
            }

            // STEP 4: Append results to main table (FAST operation - mutex needed for thread safety)
            {
               std::lock_guard<std::mutex> lockLineList(mutexLineList);
               ptableLineList->append(ptableLineListLocal.get());              // Append the results from the local table to the main table

               // Update total line count and check if we've exceeded the maximum
               uint64_t uCurrentLines = uAtomicTotalLines.fetch_add(ptableLineListLocal->get_row_count()) + ptableLineListLocal->get_row_count();
               if( uCurrentLines > uMax ) 
               {
                  uAtomicFileIndex.store(uFileCount);                          // Signal other threads to stop by setting file index to max
               }
            }
            
            ptableLineListLocal->row_clear();                                  // Clear local table rows for next iteration

            // Update progress (thread-safe)
            uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;
            if(uProcessed % 10 == 0)                                           // Show progress every 10 files
            {
               std::lock_guard<std::mutex> lockProgress(mutexProgress);
               uint64_t uPercent = (uProcessed * 100) / uFileCount;
               MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Find in files"}, {"sticky", true}});
            }
         }
         catch(const std::exception& exception_)
         {
            std::lock_guard<std::mutex> lockErrors(mutexErrors);
            vectorError.push_back(std::string("Thread ") + std::to_string(iThreadId) + " error: " + exception_.what());
         }      
      }
   };

   // ## Prepare and run threads ..............................................

   if( iThreadCount <= 0 ) { iThreadCount = std::thread::hardware_concurrency(); } // Use hardware concurrency if no thread count is specified
   if(iThreadCount <= 0) { iThreadCount = 1; }                                 // Fallback to single thread if hardware_concurrency returns 0
   if( iThreadCount > 8 ) { iThreadCount = 8; }                                // Limit to 8 threads for performance and resource management
   if( ptableFile->size() < iThreadCount ) { iThreadCount = (int)ptableFile->size(); } // Limit threads to number of files

   // Create and launch worker threads
   std::vector<std::thread> vectorPatternThread;
   vectorPatternThread.reserve(iThreadCount);

   for(int i = 0; i < iThreadCount; ++i) { vectorPatternThread.emplace_back(process_, i); }

   // Wait for all threads to complete
   for(auto& threadWorker : vectorPatternThread) { threadWorker.join(); }

   MESSAGE_Progress("", {{"clear", true}});                                   // Clear progress message

   // ### Handle any collected errors
   if(!vectorError.empty())
   {
      for(const auto& stringError : vectorError) { ERROR_Add(stringError); }
   }

   return {true, ""};
}


/** ---------------------------------------------------------------------------
 * @brief Updates the pattern list for files in the cache using regex patterns and multithreading.
 *
 * This method processes a list of regex patterns and applies them to the files stored in the cache.
 * It generates a list of lines in each file where the patterns are found and stores the results
 * in the "file-linelist" cache table. The method uses multithreading to process files in parallel.
 *
 * @param vectorRegexPatterns A vector of pairs containing regex patterns and their names.
 *                            The vector must not be empty.
 * @param argumentsList Arguments for pattern processing (e.g., segment specification, max lines)
 * @param iThreadCount Number of threads to use (0 = auto-detect)
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `vectorRegexPatterns` must not be empty.
 * @post The "file-linelist" cache table is updated with the lines where the patterns are found.
 *
 * @note COMMAND_ListLinesWithPattern must be thread-safe.
 */
std::pair<bool, std::string> CDocument::FILE_UpdatePatternList(const std::vector<std::pair<boost::regex, std::string>>& vectorRegexPatterns, const gd::argument::shared::arguments& argumentsList, int iThreadCount)
{                                                                                                  assert(vectorRegexPatterns.empty() == false); // Ensure the regex pattern list is not empty
   using namespace gd::table;
   
   auto* ptableFile = CACHE_Get("file");                                      // Retrieve the "file" cache table
   auto* ptableLineList = CACHE_Get("file-linelist", true);                   // Ensure the "file-linelist" table is in cache
                                                                                                   assert(ptableFile != nullptr); assert(ptableLineList != nullptr);

   std::string_view stringSegment;
   if( argumentsList.exists("segment") == true ) { stringSegment = argumentsList["segment"].as_string_view(); } // Get the segment (code, comment, string) to search in

   uint64_t uMax = argumentsList["max"].as_uint64();                          // Get the maximum number of lines to be printed
   auto uFileCount = ptableFile->get_row_count();                             // Total number of files to process

   // ## Thread synchronization variables

   std::atomic<uint64_t> uAtomicFileIndex(0);                                 // Current file being processed
   std::atomic<uint64_t> uAtomicProcessedCount(0);                            // Count of processed files
   std::atomic<uint64_t> uAtomicTotalLines(0);                                // Total lines found across all threads
   //std::mutex mutexProgress;                                                  // Mutex to protect progress updates
   std::mutex mutexLineList;                                                  // Mutex to protect ptableLineList access
   std::vector<std::string> vectorError;                                      // Collect errors from all threads
   std::mutex mutexErrors;                                                    // Mutex to protect access to vectorError

   // ## Prepare columns for line list table 
   detail::columns* pcolumnsThread = new detail::columns{};
   ptableLineList->to_columns( *pcolumnsThread );


   // ## Worker function to process files in parallel .........................
   auto process_ = [&](int iThreadId) -> void
   {
      // Create thread-local table for collecting results
      std::unique_ptr<table> ptableLineListLocal = std::make_unique<table>(pcolumnsThread, 10, ptableLineList->get_flags(), 10); // Create local table with 10 rows pre-allocated
      
      while( true )
      {
         uint64_t uRowIndex = uAtomicFileIndex.fetch_add(1);                  // get thread safe current index and increment it
         if(uRowIndex >= uFileCount) { break; }
         
         try
         {
            // STEP 1: Get file info (ptableFile is read-only so no mutex needed)
            auto stringFolder = ptableFile->cell_get_variant_view(uRowIndex, "folder").as_string();
            auto stringFilename = ptableFile->cell_get_variant_view(uRowIndex, "filename").as_string();

            // STEP 2: Build full file path
            gd::file::path pathFile(stringFolder);
            pathFile += stringFilename;
            std::string stringFile = pathFile.string();

            auto uKey = ptableFile->cell_get_variant_view(uRowIndex, "key").as_uint64();

            // STEP 3: Find lines with regex patterns 
            gd::argument::shared::arguments arguments_({{"source", stringFile}, {"file-key", uKey}});
            if( stringSegment.empty() == false ) arguments_.set("segment", stringSegment.data()); // Set the segment (code, comment, string) to search in
            
            auto result_ = COMMAND_ListLinesWithPattern(arguments_, vectorRegexPatterns, ptableLineListLocal.get()); // Find lines with regex patterns and update the local table

            if(result_.first == false)
            {
               std::lock_guard<std::mutex> lockErrors(mutexErrors);
               vectorError.push_back("File: " + stringFile + " - " + result_.second);

               uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;   // Update progress even on failure
               if(uProcessed % 10 == 0)
               {
                  //std::lock_guard<std::mutex> lockProgress(mutexProgress);
                  uint64_t uPercent = (uProcessed * 100) / uFileCount;
                  MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Find in files"}, {"sticky", true}});
               }
               continue;                                                       // Skip to next file on error
            }

            // STEP 4: Append results to main table (FAST operation - mutex needed for thread safety)
            {
               std::lock_guard<std::mutex> lockLineList(mutexLineList);
               ptableLineList->append(ptableLineListLocal.get());              // Append the results from the local table to the main table

               // Update total line count and check if we've exceeded the maximum
               uint64_t uCurrentLines = uAtomicTotalLines.fetch_add(ptableLineListLocal->get_row_count()) + ptableLineListLocal->get_row_count();
               if( uCurrentLines > uMax ) 
               {
                  uAtomicFileIndex.store(uFileCount);                          // Signal other threads to stop by setting file index to max
               }
            }
            
            ptableLineListLocal->row_clear();                                  // Clear local table rows for next iteration

            uint64_t uProcessed = uAtomicProcessedCount.fetch_add(1) + 1;      // Update progress (thread-safe)
            if(uProcessed % 10 == 0)                                           // Show progress every 10 files
            {
               //std::lock_guard<std::mutex> lockProgress(mutexProgress);
               uint64_t uPercent = (uProcessed * 100) / uFileCount;
               MESSAGE_Progress("", {{"percent", uPercent}, {"label", "Find in files"}, {"sticky", true}});
            }
         }
         catch(const std::exception& exception_)
         {
            std::lock_guard<std::mutex> lockErrors(mutexErrors);
            vectorError.push_back(std::string("Thread ") + std::to_string(iThreadId) + " error: " + exception_.what());
         }      
      }
   };

   // ## Prepare and run threads ..............................................

   if( iThreadCount <= 0 ) { iThreadCount = std::thread::hardware_concurrency(); } // Use hardware concurrency if no thread count is specified
   if(iThreadCount <= 0) { iThreadCount = 1; }                                 // Fallback to single thread if hardware_concurrency returns 0
   if( iThreadCount > 8 ) { iThreadCount = 8; }                                // Limit to 8 threads for performance and resource management
   if( ptableFile->size() < iThreadCount ) { iThreadCount = (int)ptableFile->size(); } // Limit threads to number of files

   // Create and launch worker threads
   std::vector<std::thread> vectorPatternThread;
   vectorPatternThread.reserve(iThreadCount);

   for(int i = 0; i < iThreadCount; ++i) { vectorPatternThread.emplace_back(process_, i); }

   // Wait for all threads to complete
   for(auto& threadWorker : vectorPatternThread) { threadWorker.join(); }

   MESSAGE_Progress("", {{"clear", true}});                                   // Clear progress message

   // ### Handle any collected errors
   if(!vectorError.empty())
   {
      for(const auto& stringError : vectorError) { ERROR_Add(stringError); }
   }

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Updates the pattern find results for files in the cache.
 *
 * This method processes a list of patterns and applies them to the files stored in the cache.
 * It generates a list of lines in each file where the patterns are found and stores the results
 * in the "file-linelist" cache table.
 *
 * @param vectorPattern A vector of strings representing the patterns to search for.
 *                      The vector must not be empty and can contain a maximum of 64 patterns.
 * @param pargumentsFind The arguments containing additional parameters such as segment and max hits.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @pre The `vectorPattern` must not be empty and must contain fewer than 64 patterns.
 * @post The "file-linelist" cache table is updated with the lines where the patterns are found.
 *
 * @details
 * - The method ensures that the "file-linelist" cache table is prepared and available.
 * - For each file in the "file" cache table, it generates the full file path by combining
 *   the "folder" and "filename" columns.
 * - It then calls the `COMMAND_FindPattern_g` function to find the lines in the file
 *   that match the patterns and updates the "file-linelist" table with the results.
 * - If an error occurs during the process, it is added to the internal error list.
 */
std::pair<bool, std::string> CDocument::FILE_UpdatePatternFind( const std::vector< std::string >& vectorPattern, const gd::argument::shared::arguments* pargumentsFind )
{                                                                                                  assert( pargumentsFind != nullptr );
   auto* ptableLineList = CACHE_Get("file-linelist", true);                   // Ensure the "file-linelist" table is in cache
   auto* ptableFile = CACHE_Get("file");                                      // Retrieve the "file" cache table

   uint64_t uFileIndex = 0; // index for file table
   auto uFileCount = ptableFile->get_row_count(); // get current row count in file-count table
   uint64_t uMax = 500u; // Default maximum number of hits, can be overridden by options_
   if( pargumentsFind != nullptr ) uMax = pargumentsFind->get_argument<uint64_t>("max", 500u ); // @@TODO: Change solution to take default value for number of hits from applicaton property

   std::string stringFileBuffer;
   stringFileBuffer.reserve( 64 * 64 );

   // ## Prepare key value if used
   std::vector<gd::argument::arguments> vectorKeyValue;
   if( pargumentsFind->exists("keys") == true || pargumentsFind->exists("kv") == true )                                  // Check if key-value pairs are provided 
   {
      std::string stringKVFormat;
      if( pargumentsFind->exists("kv-format") == true )
      {
         stringKVFormat = ( *pargumentsFind )["kv-format"].as_string();        // Get the key-value format, if key-value pairs are scoped like {key=value}, [key:value] etc. "{}=" or "[]:" are used to read sample scope
      }

      if( pargumentsFind->exists("keys") == true )
      {
         auto vector_ = pargumentsFind->get_all<std::string>("keys");
         
         if( vector_.size() == 1 ) 
         { 
            char iSplit = ';';                                                // default separator for keys
            std::string& stringKeys = vector_[0];
            auto uPosition = stringKeys.find_first_of(",");   
            if( uPosition != std::string::npos ) { iSplit = ','; }            // use the first separator found
            vector_ = CApplication::Split_s(vector_[0], iSplit);              // split it by iSplit character to get multiple keys
            // remove all empty keys
            vector_.erase(std::remove_if(vector_.begin(), vector_.end(), [](const std::string& key) { return key.empty(); }), vector_.end());
         } 

         for( const auto& key_ : vector_ )
         {
            gd::argument::arguments argumentsRule({ {"key", key_ } });
            if( stringKVFormat.empty() == false ) { argumentsRule.append("scope", stringKVFormat); } // Add scope rules for key value items if provided
            vectorKeyValue.push_back( argumentsRule );
         }
      }
   }


   // ## Prepare the patterns for finding and searching in files

   for(const auto& itRowFile : *ptableFile)
   {
      // ## calculate percentage for progress message

      uFileIndex++;                                                            // increment file index for each file, used for progress message
      if( uFileIndex % 10 == 0 ) // show progress message every 10 files
      {
         uint64_t uPercent = (uFileIndex * 100) / uFileCount;                 // calculate percentage of files processed
         MESSAGE_Progress( "", {{"percent", uPercent}, {"label", "Find in files"}, {"sticky", true} });
      }

      // ## Generate the full file path (folder + filename)
      auto string_ = itRowFile.cell_get_variant_view("folder").as_string();
      gd::file::path pathFile(string_);
      string_ = itRowFile.cell_get_variant_view("filename").as_string();
      pathFile += string_;
      std::string stringFile = pathFile.string();

      auto uKey = itRowFile.cell_get_variant_view("key").as_uint64();

      // Find lines with patterns and update the "file-linelist" table
      gd::argument::shared::arguments arguments_({{"source", stringFile}, {"file-key", uKey}});
      if( pargumentsFind->exists("segment") == true ) arguments_.append("segment", (*pargumentsFind)["segment"].as_string() ); // Add segment if it exists in the arguments
      stringFileBuffer.clear();                                               // Clear the blob vector to reuse it for the next file

      // ## Prepare file for finding patterns, file is loaded into stringFileBuffer

      auto result_ = CLEAN_File_g(stringFile, arguments_, stringFileBuffer);  // Load file into memory as a blob
      if( result_.first == false)
      {
         ERROR_Add(result_.second);                                           // Add error to the internal error list
         continue;                                                            // Skip to the next file if there was an error
      }

      if( stringFileBuffer.empty() == true ) continue;                        // Skip empty files

      // ## Find patterns in the stringFileBuffer

      uint64_t uRowOffset = ptableLineList->size();                           // Get the current row count in the "file-linelist" table
      result_ = COMMAND_FindPattern_g(stringFileBuffer, vectorPattern, arguments_, ptableLineList ); // Find lines with patterns in the file blob
      if( result_.first == false )
      {
         ERROR_Add(result_.second); // Add error to the internal error list
         continue; // Skip to the next file if there was an error
      }

      if( uRowOffset == ptableLineList->size() ) continue;                    // Skip if no patterns were found in the file 

      // ## Update key-value pairs if provided, checks for any arguments associated with key value logic

      if( pargumentsFind->exists("keys") == true || pargumentsFind->exists("kv") == true )
      {
         // Extract rows where to look for key-value pairs
         std::vector<uint64_t> vectorRow;
         gd::table::dto::table tableRow(0u, { {"uint64", 0, "row"}, {"uint64", 0, "linelist-key"} }, gd::table::tag_prepare{}); // Prepare table for key-value pairs

         for( auto itRow = uRowOffset; itRow < ptableLineList->size(); itRow++ ) 
         { 
            uint64_t uRow = ptableLineList->cell_get_variant_view(itRow, "row");
            uint64_t uKey = ptableLineList->cell_get_variant_view(itRow, "key");
            vectorRow.push_back(uRow);                                       // add row number in file

            auto r_ = tableRow.row_add_one(); // Add a new row to the key-value table
            tableRow.cell_set(r_, "row", uRow);                        // Set the row number in the key-value table
            tableRow.cell_set(r_, "linelist-key", uKey);               // Set the line list key in the key-value table
         } 

         BUFFER_UpdateKeyValue(arguments_, stringFileBuffer, tableRow, vectorKeyValue);   // Update table from key-value pairs @TAG #kv
      }

      if( ptableLineList->size() > uMax ) { break; }                          // Stop if the maximum number of lines is reached
   }

   MESSAGE_Progress( "", {{"percent", 100}, {"label", "Find in files"}, {"sticky", true} });

   return { true, "" };

}

std::pair<bool, std::string> CDocument::FILE_UpdatePatternFind( const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPatterns, const gd::argument::shared::arguments* pargumentsFind )
{
   auto* ptableLineList = CACHE_Get("file-linelist", true);                   // Ensure the "file-linelist" table is in cache
   auto* ptableFile = CACHE_Get("file");                                      // Retrieve the "file" cache table

   auto default_ = gd::argument::shared::arguments();
   gd::argument::shared::arguments& options_ = default_;
   if( pargumentsFind != nullptr ) options_ = *pargumentsFind ;

   uint64_t uFileIndex = 0; // index for file table
   auto uFileCount = ptableFile->get_row_count(); // get current row count in file-count table
   uint64_t uMax = options_.get_argument<uint64_t>("max", 500u );             // @@TODO: Change solution to take default value for number of hits from applicaton property

   std::string stringFileBuffer; // Buffer to store the file content
   stringFileBuffer.reserve( 64 * 64 );


   for(const auto& itRowFile : *ptableFile)
   {
      // ## calculate percentage for progress message

      uFileIndex++;                                                            // increment file index for each file, used for progress message
      if( uFileIndex % 10 == 0 ) // show progress message every 10 files
      {
         uint64_t uPercent = (uFileIndex * 100) / uFileCount;                 // calculate percentage of files processed
         MESSAGE_Progress( "", {{"percent", uPercent}, {"label", "Find in files"}, {"sticky", true} });
      }

      // ## Generate the full file path (folder + filename)
      auto string_ = itRowFile.cell_get_variant_view("folder").as_string();
      gd::file::path pathFile(string_);
      string_ = itRowFile.cell_get_variant_view("filename").as_string();
      pathFile += string_;
      std::string stringFile = pathFile.string();

      auto uKey = itRowFile.cell_get_variant_view("key").as_uint64();

      // Find lines with patterns and update the "file-linelist" table
      gd::argument::shared::arguments arguments_({{"source", stringFile}, {"file-key", uKey}});
      if( pargumentsFind->exists("segment") == true ) arguments_.append("segment", (*pargumentsFind)["segment"].as_string() ); // Add segment if it exists in the arguments
      stringFileBuffer.clear();                                               // Clear the blob vector to reuse it for the next file
      auto result_ = CLEAN_File_g(stringFile, arguments_, stringFileBuffer);  // Load file into memory as a blob
      if( result_.first == false)
      {
         ERROR_Add(result_.second);                                           // Add error to the internal error list
         continue;                                                            // Skip to the next file if there was an error
      }

      if( stringFileBuffer.empty() == true ) continue;                        // Skip empty files

      // ## Find patterns in the file blob
      result_ = COMMAND_FindPattern_g(stringFileBuffer, vectorRegexPatterns, arguments_, ptableLineList ); // Find lines with patterns in the file blob
      if( result_.first == false )
      {
         ERROR_Add(result_.second); // Add error to the internal error list
         continue; // Skip to the next file if there was an error
      }

      if( ptableLineList->size() > uMax ) { break; }                          // Stop if the maximum number of lines is reached
   }

   MESSAGE_Progress( "", {{"percent", 100}, {"label", "Find in files"}, {"sticky", true} });

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Updates key-value pairs in the cache keyvalue table from a file buffer.
 *
 * This method processes a file buffer and extracts key-value pairs based on the provided rules.
 * It updates the "keyvalue" cache table with the extracted key-value pairs.
 *
 * @param argumentsFile The arguments containing the file key and source string.
 * @param stringFileBuffer The file buffer containing the content of the file. From this buffer key-value pairs are extracted.
 * @param tableRow The table containing rows to process with information where to find key-value extraction.
 * @param vectorRule A vector of rules defining how to extract key-value pairs.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> CDocument::BUFFER_UpdateKeyValue(const gd::argument::shared::arguments& argumentsFile, std::string_view stringFileBuffer, gd::table::dto::table& tableRow, const std::vector<gd::argument::arguments>& vectorRule)
{
   uint64_t uFileKey = argumentsFile["file-key"].as_uint64();                                      assert( uFileKey > 0 );
   std::string_view stringFile = argumentsFile["source"].as_string_view();                         assert( stringFile.empty() == false );
   auto ptableKeyValue = CACHE_GetTableArguments("keyvalue", true); // Ensure the "keyvalue" table is in cache

   //for( const auto& row_ : tableRow )
   for( auto itRow = tableRow.begin(); itRow != tableRow.end(); ++itRow ) // Iterate through the rows in the table
   {
      uint64_t uKeyValueRow = std::numeric_limits<uint64_t>::max();
      uint64_t uRow = itRow.cell_get_variant_view("row").as_uint64();

      std::string_view stringFrom = gd::math::string::select_from_line(stringFileBuffer, uRow); // Get the line from the file buffer
      std::string_view stringContent = gd::math::string::select_content_lines(stringFrom); // Get the content of the line
      if( stringContent.empty() == true ) continue; // Skip empty lines

      // ## extract key-value pairs from string based on rules in vectorKeyValue
      for(const auto& rule_ : vectorRule) 
      {
         std::string stringValue; // Variable to store the extracted value
         // ## Logic to extract key-value pairs from stringContent based on keyValue
         std::vector<std::string> vectorKey = rule_.get_all< std::string>("key"); // Get all keys from the rule, can be multiple keys
         std::string stringKeys = rule_["keys"].as_string();                  // Get the keys from the rule, this can be a single key or multiple keys separated by ,
         std::string stringScope = rule_["scope"].as_string();                // scope information is how key-value pairs are scoped, like {key=value}, [key:value] etc.

         const gd::parse::code codeKeyValue(stringScope);

         if( stringKeys.empty() == false ) { gd::utf8::split(stringKeys, ',', vectorKey); } // If multiple keys are provided, split them by comma and add to the vector

         // ## Read values for each key in vectorKey
         //    Values read are placed in `keyvalue` table, table are able to store extra columns on each row

         for( const auto& stringKey : vectorKey )
         {
            const char* piResult = gd::parse::strstr(stringContent, stringKey, codeKeyValue, codeKeyValue.is_bracket(), gd::types::tag_name{});
            if( piResult == nullptr ) continue;                                   // Skip if the key is not found in the content
            stringValue = codeKeyValue.read_value(piResult, stringContent.data() + stringContent.size(), gd::types::tag_view{});
            if( stringValue.empty() == false )
            {
               if( uKeyValueRow == std::numeric_limits<uint64_t>::max() )
               {
                  uKeyValueRow = ptableKeyValue->row_add_one();                   // Add a new row to the key-value table

                  ptableKeyValue->cell_set(uKeyValueRow, "key", uint64_t(uKeyValueRow + 1)); // Set the key in the key-value table
                  ptableKeyValue->cell_set( uKeyValueRow, "file-key", uFileKey );// set foreign key to file table
                  ptableKeyValue->cell_set(uKeyValueRow, "filename", stringFile);// Set the file name in the key-value table
                  ptableKeyValue->cell_set(uKeyValueRow, "row", uRow);         // Set the key in the key-value table
                  uint64_t uLineListKey = itRow.cell_get_variant_view("linelist-key").as_uint64(); assert(uLineListKey > 0);
                  ptableKeyValue->cell_set(uKeyValueRow, "file-linelist-key", uLineListKey);
               }
               // ## Add key-value pair to the table
               ptableKeyValue->cell_add_argument(uKeyValueRow, stringKey, stringValue);
            }
         }
      }
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

   if( stringType == "COUNT" || stringType == "LIST" )
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
      else
      {
         stringResult = gd::table::to_string(*ptableResult, { {"verbose", true} }, gd::table::tag_io_cli{});
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


// 0TAG0CACHE.Document 

/** --------------------------------------------------------------------------- @TAG #cache 
 * @brief Prepares a cache table for the specified identifier.  
 *  
 * This method initializes and prepares a table for caching data associated with the given `stringId`.  
 * If the `stringId` matches specific identifiers like "file", "file-count", or "file-linelist",  
 * it creates tables with predefined columns tailored for their respective purposes:  
 *  
 * - **file**: Stores file information such as folder, filename, size, date, and extension.  
 * - **file-count**: Tracks row counters for files, including counts for code, characters, comments, and strings.  
 * - **file-linelist**: Lists lines where patterns are found, including details like row, column, and matched pattern.  
 *  
 * The table is then added to the internal application cache.  
 *  
 * @param stringId A string view representing the identifier for the cache table.  
 *  
 * @details  
 * - The method first checks if a cache table with the given `stringId` already exists using `CACHE_Get`.  
 * - If the table does not exist, it creates a new `table` object with predefined columns.  
 * - The table is wrapped in a `std::unique_ptr` and added to the cache using `CACHE_Add`.  
 *  
 * @note This method assumes that the `CACHE_Add` function handles the ownership of the table.  
 */  
void CDocument::CACHE_Prepare(const std::string_view& stringId, std::unique_ptr<gd::table::dto::table>* ptable) // @TAG #data.cache
{
   using namespace gd::table::dto;
   constexpr unsigned uTableStyle = ( table::eTableFlagNull32 | table::eTableFlagRowStatus );

   if( ptable == nullptr )
   {
      auto ptableFind = CACHE_Get(stringId, false);
      if( ptableFind != nullptr ) return;                                      // table already exists, exit  
   }

   std::unique_ptr<gd::table::dto::table> ptable_;

   // ## prepare file list  
   //    columns: "path, size, date, extension  
   if( stringId == "file" )                                                    // file cache, used to store file information  
   {
      auto p_ = CACHE_Get(stringId, false);
      if( p_ == nullptr )
      {
         // file table: key | path | size | date | extension  
         ptable_ = std::make_unique<table>(table(uTableStyle, { {"uint64", 0, "key"}, {"rstring", 0, "folder"}, {"rstring", 0, "filename"}, {"uint64", 0, "size"}, {"double", 0, "date"}, {"string", 20, "extension"} }, gd::table::tag_prepare{}));
         ptable_->property_set("id", stringId);                                // set id for table, used to identify table in cache
      }
   }
   else if( stringId == "file-dir" )                                           // file cache, used to store file information  
   {
      auto p_ = CACHE_Get(stringId, false);
      if( p_ == nullptr )
      {
         // file table: key | path | size | date | extension  
         ptable_ = std::make_unique<table>(table(uTableStyle, { {"uint64", 0, "key"}, {"rstring", 0, "path"}, {"uint64", 0, "size"}, {"double", 0, "date"}, {"string", 20, "extension"} }, gd::table::tag_prepare{}));
         ptable_->property_set("id", stringId);                                // set id for table, used to identify table in cache
      }
   }
   else if( stringId == "file-count" )                                         // row counter table  
   {
      auto p_ = CACHE_Get(stringId, false);
      if( p_ == nullptr )
      {
         // file-count table: key | file-key | filename  
         //           count | code | characters | comment | string  
         ptable_ = std::make_unique<table>(table(uTableStyle,
            { {"uint64", 0, "key"}, {"uint64", 0, "file-key"}, {"rstring", 0, "filename"},
              {"uint64", 0, "count"}, {"uint64", 0, "code"}, {"uint64", 0, "characters"}, {"uint64", 0, "comment"}, {"uint64", 0, "string"} }, gd::table::tag_prepare{})
         );
         ptable_->property_set("id", stringId);                                // set id for table, used to identify table in cache
      }
   }
   else if( stringId == "file-linelist" )                                      // lists line where pattern was found  
   {
      auto p_ = CACHE_Get(stringId, false);
      if( p_ == nullptr )
      {
         // file-linelist table: key | file-key | filename  
         //                      line | row | column | pattern, segment  
         //                      line = the row in text where pattern was found  
         ptable_ = std::make_unique<table>(table(uTableStyle,
            { {"uint64", 0, "key"}, {"uint64", 0, "file-key"}, {"rstring", 0, "filename"},
              {"rstring", 0, "line"}, {"uint64", 0, "row"}, {"uint64", 0, "column"}, {"string", 32, "pattern"}, {"string", 10, "segment"} }, gd::table::tag_prepare{})
         );
         ptable_->property_set("id", stringId);                                // set id for table, used to identify table in cache
      }
   }
   else if( stringId == "file-snippet" )
   { 
      ptable_ = std::make_unique<table>(table(uTableStyle,
         { {"uint64", 0, "key"}, {"uint64", 0, "file-key"}, {"rstring", 0, "filename"},
         {"string", 10, "format"}, {"uint64", 0, "row"}, {"rstring", 0, "snippet"} }, gd::table::tag_prepare{})
      );
      ptable_->property_set("id", stringId);                                  // set id for table, used to identify table in cache
   }
   else if( stringId == "keyvalue" )
   {
      auto ptableKeys = std::make_unique<gd::table::arguments::table>( gd::table::tag_full_meta{} );
      ptableKeys->column_prepare();
      ptableKeys->column_add("uint64", 0, "key");                             // add key column
      ptableKeys->column_add("uint64", 0, "file-key");                        // foreign key to file table
      ptableKeys->column_add("uint64", 0, "file-linelist-key");               // foreign key to file-linelist table
      ptableKeys->column_add("rstring", 0, "filename");                       // name of file
      ptableKeys->column_add("uint64", 0, "row");                             // row number in file
      ptableKeys->prepare();                                                  // prepare table
      ptableKeys->property_set("id", stringId);                               // set id for table, used to identify table in cache
      CACHE_Add( std::move(ptableKeys) );                                     // add table to cache
      return;                                                                 // exit, table is already added to cache
   }
   else if( stringId == "history" )
   {
      auto p_ = CACHE_Get(stringId, false);
      if( p_ == nullptr )
      {
         // file table: index | date | command | line 
         ptable_ = std::make_unique<table>(table(uTableStyle, { {"uint64", 0, "key"}, {"int32", 0, "index"}, {"rstring", 0, "date"}, {"rstring", 0, "command"}, {"rstring", 0, "line"} }, gd::table::tag_prepare{}));
         ptable_->property_set("id", stringId);                                // set id for table, used to identify table in cache
      }
   }
   else { assert(false); } // unknown cache table

   if( ptable != nullptr )
   {
      *ptable = std::move(ptable_);                                            // move table to caller
   }
   else
   {
      CACHE_Add(std::move(*ptable_)); // add it to internal application cache  
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
      // Use std::visit to check if the table has the same id
      bool bFound = std::visit([&stringTableId](const auto& ptable_)
      {
         auto argumentId = ptable_->property_get("id");
         return argumentId.is_string() && stringTableId == static_cast<const char*>(argumentId);
      }, *it); // closing the visit lambda   

      if(bFound) {  assert(false);  return false; }
   }

   /// Create unique_ptr with table and move table data to this table
   std::unique_ptr<gd::table::dto::table> ptable = std::make_unique<gd::table::dto::table>( std::move( table ) );

   if( stringId.empty() == false )
   {
      ptable->property_set( { "id", stringTableId } );
   }
   m_vectorTableCache.push_back( std::move( ptable ) );                        // insert table to vector

   return true;
}

void CDocument::CACHE_Add( std::unique_ptr< gd::table::arguments::table > ptable_ )
{
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );       // locks `m_vectorTableCache`

   m_vectorTableCache.push_back( std::move( ptable_ ) );                        // insert table to vector
}


std::string CDocument::CACHE_Add( gd::table::dto::table&& table, const std::string_view& stringId, gd::types::tag_temporary )
{
   std::string stringTableId( stringId );
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );       // locks `m_vectorTableCache`

   table.property_set({ "temporary", true }); // mark table as temporary

   if( stringTableId.empty() == true ) 
   { 
      stringTableId = gd::uuid(gd::uuid::tag_random{}).to_string();
      table.property_set({ "id", stringTableId }); // set id to table
   }
   
#ifndef NDEBUG
   if( stringId.empty() == false )
   {
      // ## There is a tiny chance table was added before this method was called, we need to check with exclusive lock
      for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
      {
         // Use std::visit to check if the table has the same id
         bool bFound = std::visit([&stringTableId](const auto& ptable_)
         {
            auto argumentId = ptable_->property_get("id");
            return argumentId.is_string() && stringTableId == static_cast<const char*>(argumentId);
         }, *it); // closing the visit lambda   

         if( bFound == true ) 
         { 
            assert(false); return "Table with id already exists in cache"; // found table, exit
         }
      }
   }
#endif // NDEBUG

   /// Create unique_ptr with table and move table data to this table
   std::unique_ptr<gd::table::dto::table> ptable = std::make_unique<gd::table::dto::table>( std::move( table ) );
   m_vectorTableCache.push_back( std::move( ptable ) );            // insert table to vector

   return stringTableId;
}

void CDocument::CACHE_Add( std::unique_ptr< gd::table::dto::table > ptableAdd )
{
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );       // locks `m_vectorTableCache`
   m_vectorTableCache.push_back( std::move( ptableAdd ) );                     // insert table to vector
}

/** ---------------------------------------------------------------------------
 * @brief Get pointer to table with specified id
 * @param stringId id to table that is returned
 * @return pointer to table with id
 */
gd::table::dto::table* CDocument::CACHE_Get( const std::string_view& stringId, bool bPrepare )
{
   {
      std::shared_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );

      for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
      {
         if( std::holds_alternative< std::unique_ptr< gd::table::dto::table > >(*it) == true ) 
         {
            auto& ptable_ = std::get< std::unique_ptr< gd::table::dto::table > >(*it);
            auto argumentId = ptable_->property_get("id");
            if( argumentId.is_string() && stringId == ( const char* )argumentId ) return ptable_.get();
         }
      }
   }

   if( bPrepare == true )
   {
      CACHE_Prepare( stringId );
      auto* ptable_ = CACHE_Get( stringId, false );                                                assert( ptable_ != nullptr );
      return ptable_;
   }

   return nullptr;
}

gd::table::arguments::table* CDocument::CACHE_GetTableArguments( const std::string_view& stringId, bool bPrepare )
{
   {
      std::shared_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );

      for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
      {
         if( std::holds_alternative< std::unique_ptr< gd::table::arguments::table > >(*it) == true ) 
         {
            auto& ptable_ = std::get< std::unique_ptr< gd::table::arguments::table > >(*it);
            auto argumentId = ptable_->property_get("id");
            if( argumentId.is_string() && stringId == ( const char* )argumentId ) return ptable_.get();
         }
      }
   }

   if( bPrepare == true )
   {
      CACHE_Prepare( stringId );
      return CACHE_GetTableArguments(stringId, false);                        // Get the table after preparing it
   }

   return nullptr;

}

// @TASK #user.per [name: sort] [brief: Find sort column by first match exact, then check for index and last try to find part of column name] [state: open] [date: 2025-08-13]

/** ---------------------------------------------------------------------------
* @brief Sorts a cached table by a specified column.
*
* This method sorts the rows of a cache table identified by `stringId` based on the values
* in the specified column. The column can be identified either by its name (string) or
* by its index (integer). Sorting can be performed in ascending or descending order.
*
* @param stringId The identifier of the cache table to be sorted.
* @param column_ The column to sort by. This can be:
*                - A string (e.g., "columnName") to specify the column by name.
*                  If the string starts with a '-', the sort order will be descending.
*                - An integer to specify the column by index. A negative value indicates
*                  descending order.
* @param ptable_ A pointer to the cache table. If `nullptr`, the method will attempt
*                to retrieve the table from the cache.
*
* @return A pair containing:
*         - `bool`: `true` if the sorting was successful, `false` otherwise.
*         - `std::string`: An empty string on success, or an error message on failure.
*
* @details
* - If the column is specified as a string, the method checks if the column exists in the
*   table. If the column name starts with a '-', it is treated as a descending sort.
* - If the column is specified as an integer, the method validates the column index.
*   A negative index indicates descending order.
* - The method uses the `sort_null` function of the `gd::table::dto::table` class to
*   perform the sorting.
*
* @pre The cache table identified by `stringId` must exist.
* @post The rows in the cache table are sorted based on the specified column.
*
*/
std::pair<bool, std::string> CDocument::CACHE_Sort(const std::string_view& stringId, const gd::variant_view& column_, gd::table::dto::table* ptable_)  // @TAG #data.cache #command.sort
{
   bool bAscending = true;
   int iColumn = -1;
   if( ptable_ == nullptr ) { ptable_ = CACHE_Get(stringId, false); }
                                                                                                   assert(ptable_ != nullptr);
   if( column_.is_string() )
   {
      std::string stringColumn = column_.as_string();
      if( stringColumn[0] == '-' ) { bAscending = false; stringColumn.erase(0, 1); }
      iColumn = ptable_->column_find_index(stringColumn);
      if( iColumn == -1 ) 
      { 
         bool bError = true;
         // check if column is a number, if so, convert it to index
         if( stringColumn.find_first_not_of("0123456789") == std::string::npos )
         {
            iColumn = std::stoi(stringColumn);
            if( iColumn < 0 ) { bAscending = false; iColumn = -iColumn; }

            // check if column index is valid and not above column count
            if( (unsigned)iColumn < ptable_->get_column_count() ) { bError = false; } // column index is valid
         }


         if( bError == true ) 
         { 
            return { false, "Column not found: " + stringColumn }; 
         }
      }
   }
   else if( column_.is_integer() )
   {
      iColumn = column_.as_int();
      if( iColumn < 0 )
      {
         bAscending = false;
         iColumn = -iColumn;
      }

      if( (unsigned)iColumn >= ptable_->get_column_count() ) { return { false, "Column not found: " + std::to_string(iColumn) }; }
   }
                                                                                                   assert( iColumn >= 0 && (unsigned)iColumn < ptable_->get_column_count() );
   ptable_->sort_null(iColumn, bAscending);

   return { true, "" };
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
 * @brief Erase all temporary cache tables.
 *
 * This method removes all tables from the cache that are marked as temporary.
 * A table is considered temporary if its "temporary" property is set to true.
 *
 * @param tag_temporary Tag dispatch to select this overload.
 */
void CDocument::CACHE_Erase(gd::types::tag_temporary)
{
   std::unique_lock<std::shared_mutex> lock_(m_sharedmutexTableCache);

   // ## Remove all tables with property "temporary" == true

   auto itTable = m_vectorTableCache.begin();
   while( itTable != m_vectorTableCache.end())
   {
      bool bTemporary = std::visit([](const auto& ptable_)                     // Use std::visit to access the table's property
      {
         auto argumentTemporary = ptable_->property_get("temporary");
         return argumentTemporary.is_bool() && argumentTemporary.as_bool();
      }, *itTable);

      if( bTemporary == true ) 
      {
         itTable = m_vectorTableCache.erase(itTable); // Erase and get next iterator
      } 
      else 
      {
         ++itTable; // Move to next table
      }
   }
}


/** ---------------------------------------------------------------------------
 * @brief Erase table cache
 * @param stringId id for cache to delete
 */
void CDocument::CACHE_Erase( const std::string_view& stringId ) 
{
   std::shared_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );

   if(CACHE_Exists(stringId) == true)
   {
      for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
      {
         // Use std::visit to get argument id from table
         auto argumentId = std::visit([](const auto& ptable_) {
            return ptable_->property_get("id");
         }, *it);

         if( argumentId.is_string() && stringId == ( const char* )argumentId )
         {
            it = m_vectorTableCache.erase( it );
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

bool CDocument::CACHE_Exists(const std::string_view& stringId) const
{
   for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
   {
      // Use std::visit to check if the table has the same id
      auto argumentId = std::visit([](const auto& ptable_) {
         return ptable_->property_get("id");
      }, *it);
      if( argumentId.is_string() && stringId == ( const char* )argumentId ) return true;
   }
   return false;
}


#ifndef NDEBUG
/// For debug, check if cache with id exists
bool CDocument::CACHE_Exists_d( const std::string_view& stringId )
{
   for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
   {
      auto argumentId = std::visit([](const auto& ptable_) {
         return ptable_->property_get("id");
      }, *it);
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

   std::vector< std::tuple< std::string_view, unsigned, std::string_view > > 
      vectorColumn( {{"rstring", 0, "folder"}, {"rstring", 0, "filename"}, {"uint64", 0, "count"}, {"uint64", 0, "code"}, {"uint64", 0, "characters"}, {"uint64", 0, "comment"}, {"uint64", 0, "string"}} );

   // ## check for modes where user wants to see code
   //    in this case we remove the folder column and only show filename and count to make it compact and easy to open file in editor

   auto eMode = GetApplication()->GetMode(); // get application mode
   if( eMode == CApplication::eModeReview || eMode == CApplication::eModeSearch )
   { 
      // remove folder and only show filename and count
      vectorColumn.erase(vectorColumn.begin());                               // remove first column, folder
      eMode = CApplication::eModeReview;                                      // set mode to review to simplify code to adapt to this mode
   }

   table tableResult(uTableStyle, vectorColumn, gd::table::tag_prepare{});

   // Retrieve the file and file-count cache tables
   auto* ptableFile = CACHE_Get("file", false);                                                    assert( ptableFile != nullptr );
   auto* ptableFileCount = CACHE_Get("file-count", false);                                         assert( ptableFileCount != nullptr );

   // ## Iterate through the rows in the file-count table
   for(const auto& itRowCount : *ptableFileCount)
   {
      uint64_t iFileKey = itRowCount.cell_get_variant_view("file-key").as_uint64();
      uint64_t uCount = itRowCount.cell_get_variant_view("count").as_uint64();
      auto stringFilename = itRowCount.cell_get_variant_view("filename").as_string();

      // Find the corresponding row in the file table using the file key
      for(const auto& itRowFile : *ptableFile)
      {
         if(itRowFile.cell_get_variant_view("key").as_uint64() == iFileKey)
         {
            auto stringFolder = itRowFile.cell_get_variant_view("folder").as_string();

            // Add a new row to the result table
            auto uRow = tableResult.get_row_count();
            tableResult.row_add( gd::table::tag_null{} );

            if( eMode == CApplication::eModeReview )
            {
               // add full path to file
               gd::file::path pathFile( stringFolder );
               pathFile += stringFilename;
               tableResult.cell_set(uRow, "filename", pathFile.string());
            }
            else
            {
               tableResult.cell_set(uRow, "folder", stringFolder );
               tableResult.cell_set(uRow, "filename", stringFilename);
            }
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
   constexpr unsigned FIXED_COLUMN_COUNT = 1; // Number of fixed columns (folder and filename)
   // Define the result table structure
   constexpr unsigned uTableStyle = ( table::eTableFlagNull64 | table::eTableFlagRowStatus );
   table tableResult(uTableStyle, { {"rstring", 0, "filename"} });
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

      gd::file::path pathFile(stringFolder);
      pathFile += stringFilename;
      std::string stringFile = pathFile.string();


      auto uRowSource = itRowCount.get_row();
      ptableFilePattern->row_get_variant_view(uRowSource, uColumnFileName, vectorPatternCount); // get row data from table
      // Add a new row to the result table
      auto uRow = tableResult.get_row_count();
      tableResult.row_add(gd::table::tag_null{});
      tableResult.cell_set(uRow, "filename", stringFile);

      tableResult.row_set(uRow, FIXED_COLUMN_COUNT,  vectorPatternCount);      // set row data to table from column 2, after filename

      vectorPatternCount.clear(); // clear vector for next row
   }
   return tableResult;
}

/** ---------------------------------------------------------------------------
 * @brief Generate a result table with pattern line list.  
 *  
 * This method generates a result table containing all lines in files where patterns were found.  
 * The result includes the file path, line number, column number, matched pattern, and a snippet of the line.  
 *   
 * @details  
 * - The method retrieves the "file" and "file-linelist" cache tables.  
 * - For each row in the "file-linelist" table, it finds the corresponding file in the "file" table using the file key.  
 * - It constructs the full file path and formats the result string based on the editor type (e.g., Visual Studio or VSCode).  
 * - The result string includes the file path, line and column numbers, matched pattern, and a snippet of the line.  
 * - The snippet is truncated to 120 characters if it exceeds this length.  
 *  
 * @param 
 * @return A table containing the pattern line list for each file. Columns in table are line, file, and context.
 *  
 * @pre The "file" and "file-linelist" cache tables must be prepared and available in the cache.  
 * @post The result table is generated with the pattern line list.  
 *  
 * @note The editor type (e.g., Visual Studio or VSCode) is currently hardcoded but can be retrieved from application settings in the future.  
 */  
gd::table::dto::table CDocument::RESULT_PatternLineList( const gd::argument::arguments& argumentsOption )
{
   // @brief Enum for editor types
   enum enumEditor { eVisualStudio, eVSCode, eSublime };
   // @brief Enum constant for column positions in table
   enum enumColumn { eColumnLine = 0, eColumnFile, eColumnContext, eColumnRow, eColumnRowLeading };

   using namespace gd::table::dto;
   enumEditor eEditor = eVisualStudio; // TODO: get editor from application settings  
   auto stringEditor = m_papplication->PROPERTY_Get("editor").as_string();
   if( stringEditor == "vscode" ) eEditor = eVSCode;
   else if( stringEditor == "sublime" ) eEditor = eSublime;

   int64_t iContextOffset = 0, iContextCount = 0; // variables used to bring context to found code

   unsigned uPatternCount = argumentsOption.get_argument( "pattern-count", 1u );
   if( argumentsOption.exists("offset") == true )
   {
      iContextOffset = argumentsOption.get_argument( "offset", (int64_t)0 ) - 1;
      iContextCount = argumentsOption.get_argument( "count", (int64_t)0 );
   }

   // Define the result table structure  
   constexpr unsigned uTableStyle = ( table::eTableFlagNull64 | table::eTableFlagRowStatus );
   table tableResult(uTableStyle, { {"rstring", 0, "line"}, {"rstring", 0, "file"}, {"rstring", 0, "context"}, {"uint64", 0, "row"}, {"uint64", 0, "row-leading"} }, gd::table::tag_prepare{});

   // Retrieve the file-pattern cache table  
   auto* ptableFile = CACHE_Get("file");                                                           assert(ptableFile != nullptr);
   auto* ptableLineList = CACHE_Get("file-linelist", false);                                       assert(ptableLineList != nullptr);

   unsigned uKeyColumnInFile = ptableFile->column_get_index("key"); // get index for key column  

   std::string string_;

   for( uint64_t uRow = 0, uRowCount = ptableLineList->size(); uRow < uRowCount; uRow++ )
   {
      auto uFileKey = ptableLineList->cell_get_variant_view(uRow, "file-key").as_uint64();

      int64_t iFileRow = ptableFile->find(uKeyColumnInFile, true, gd::variant_view(uFileKey)); // find row in file table with file-key  
      if( iFileRow == -1 ) { assert(iFileRow != -1); continue; }

      auto stringFolder = ptableFile->cell_get_variant_view(iFileRow, "folder").as_string();
      auto stringFilename = ptableFile->cell_get_variant_view(iFileRow, "filename").as_string();
      gd::file::path pathFile(stringFolder);
      pathFile += stringFilename;
      std::string stringFile = pathFile.string();

      // ## Add a new row to the result table

      auto uNewRow = tableResult.row_add_one();
      tableResult.cell_set(uNewRow, "file", stringFile);

      // ### Prepare the format to match the editor

      uint64_t uLineinSource = ptableLineList->cell_get_variant_view(uRow, "row");// get row data from table
      uLineinSource++;                                                        // line number in source file is 1-based, in table it is 0-based
      //uint64_t uColumninSource = ptableLineList->cell_get_variant_view(uRow, "column");// get row data from table


      // #### Build the result string for the file where pattern was found  
      if( eEditor == eVisualStudio )
      {
         stringFile += "(";
         stringFile += std::to_string(uLineinSource);
         //stringFile += ",";
         //stringFile += std::to_string(uColumninSource);
         stringFile += "):  [";
      }
      else if( eEditor == eVSCode )
      {
         stringFile += ":";
         stringFile += std::to_string(uLineinSource);
         //stringFile += ":";
         //stringFile += std::to_string(uColumninSource);
         stringFile += " - [";
      }
      else if( eEditor == eSublime )
      {
         stringFile += ":";
         stringFile += std::to_string(uLineinSource);
         stringFile += " - [";
      }

      // ## If more than one pattern that is searched for then add pattern to string
      if( uPatternCount > 1 )
      {
         stringFile += ptableLineList->cell_get_variant_view(uRow, "pattern").as_string();
         stringFile += "] - [";
      }

      std::string stringLine = ptableLineList->cell_get_variant_view(uRow, "line").as_string();
      stringLine = gd::math::string::trim_repeated_chars(stringLine);          // trim line from characters that are repeated more than 2 times
      if( stringLine.length() > 120 ) stringLine = stringLine.substr(0, 120) + "..."; // limit line length to 120 characters  
      stringLine += "]";                                                       // close line with ]

      stringFile += stringLine;

      tableResult.cell_set(uNewRow, 0, stringFile);                            // add clickable string to table

      if( iContextCount != 0 )
      {
         string_.clear();
         int64_t iLeadingLines = 0;
         FILES_ReadLines_g(pathFile.string(), uLineinSource, iContextOffset, iContextCount, string_, &iLeadingLines);
         tableResult.cell_set(uNewRow, 2, string_);
         tableResult.cell_set(uNewRow, eColumnRow, uLineinSource, gd::types::tag_convert{});
         tableResult.cell_set(uNewRow, eColumnRowLeading, iLeadingLines, gd::types::tag_convert{});
      }
   }

   return tableResult;
}



void CDocument::MESSAGE_Display(const std::string_view& stringMessage)
{
   m_papplication->PrintMessage(stringMessage, gd::argument::arguments() );    // display message in application window
}

void CDocument::MESSAGE_Display(const std::string_view& stringMessage, const gd::argument::arguments& argumentsMessage )
{
   m_papplication->PrintMessage(stringMessage, argumentsMessage );             // display message in application window
}

/// Restore the message display to original state
void CDocument::MESSAGE_Display()
{
   m_papplication->Print( "", gd::types::tag_background{});
}

void CDocument::MESSAGE_Background()
{
   m_papplication->Print( "background", gd::types::tag_background{} );        // restore background message
}

void CDocument::MESSAGE_Progress(const std::string_view& stringMessage)
{
   m_papplication->PrintProgress(stringMessage, gd::argument::arguments() );  // display progress message in application
}

void CDocument::MESSAGE_Progress(const std::string_view& stringMessage, const gd::argument::arguments& argumentsMessage )
{
   m_papplication->PrintProgress(stringMessage, argumentsMessage );           // display progress message in application
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

void CDocument::ERROR_Print() 
{
   std::shared_lock<std::shared_mutex> lock_( m_sharedmutexError );            // locks `m_vectorError`
   if( m_vectorError.empty() == true ) return;                                 // no errors, exit

   for( const auto& itError : m_vectorError )
   {
      std::string stringError = itError["text"].as_string();
      if( stringError.empty() == false ) { m_papplication->PrintError(stringError, gd::argument::arguments() ); } // print error message
   }

}

void CDocument::RESULT_VisualStudio_s( gd::table::dto::table& table_, std::string& stringResult )
{
   unsigned uColumnCount = table_.get_column_count(); // get number of columns

   for( const auto& itRow : table_ )
   {
      // combine all columns into one row
      std::string stringRow = itRow.cell_get_variant_view("line").as_string();
      /*
      std::string stringRow;
      for( unsigned uColumn = 0; uColumn < (uColumnCount - 3); uColumn++ )
      {
         if( uColumn != 0 ) stringRow += "\t"; // add tab between columns
         auto stringColumn = itRow.cell_get_variant_view(uColumn).as_string();
         if( stringColumn.empty() == false ) stringRow += stringColumn;
      }
      */
      stringRow += "\n";
      stringResult += stringRow;
   }
}
