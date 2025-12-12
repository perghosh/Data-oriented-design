

#include "gd/gd_file.h"
#include "gd/gd_table.h"
#include "gd/gd_table_io.h"
#include "gd/gd_utf8.h"
#include "gd/math/gd_math_string.h"
#include "gd/parse/gd_parse_formats.h"

#include "pugixml/pugixml.hpp"
#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"

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


/** --------------------------------------------------------------------------- @API [tag: cache, table ] [description: Generate tables for selected cache identifiers]
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
void CDocument::CACHE_Prepare(const std::string_view& stringId, std::unique_ptr<gd::table::dto::table>* ptable)
{
   using namespace gd::table::dto;
   constexpr unsigned uTableStyle = ( table::eTableFlagNull32 | table::eTableFlagRowStatus );
}

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
 * @brief Check if cache table with id exists
 * @param stringId id to check for
 * @return true if table with id exists
 */ 
bool CDocument::CACHE_Get(const std::string_view& stringId, pointer_table_t& ptable_ )
{
   std::shared_lock<std::shared_mutex> lock_( m_sharedmutexTableCache );

   for( auto it = std::begin( m_vectorTableCache ), itEnd = std::end( m_vectorTableCache ); it != itEnd; it++ )
   {
      // Use std::visit to check if the table has the same id
      bool bFound = std::visit([&stringId, &ptable_](const auto& ptable__)
      {
         auto argumentId = ptable__->property_get("id");
         if( argumentId.is_string() && stringId == static_cast<const char*>(argumentId) ) 
         { 
            ptable_ = ptable__.get(); 
            return true; 
         }
         return false;
      }, *it); // closing the visit lambda   
      if(bFound) { return true; }
   }
   return false;
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
            auto argumentId = ptable_->property_get("id");                                         assert( argumentId.is_string() == true );
            if( argumentId.is_string() && stringId == argumentId.as_string_view() )
            {
#ifndef NDEBUG
            auto stringId_d = argumentId.as_string_view();
               //LOG_DEBUG_RAW( "Table found in cache: " & stringId & " number of rows: " & ptable_->size());
#endif // NDEBUG
               return ptable_.get();
            }
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

/** ---------------------------------------------------------------------------
 * @brief Get pointer to arguments table with specified id
 * @param stringId id to table that is returned
 * @return pointer to table with id
 */
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
            if( argumentId.is_string() && stringId == ( const char* )argumentId )            
            {                                                                                      
#ifndef NDEBUG
               //LOG_DEBUG_RAW( "Table found in cache: " & stringId & " number of rows: " & ptable_->size());
#endif // NDEBUG
               return ptable_.get();
            }
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

/** ---------------------------------------------------------------------------  @CODE [tag: sort] [description: Sorts a cached table by a specified column]
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
std::pair<bool, std::string> CDocument::CACHE_Sort(const std::string_view& stringId, const gd::variant_view& column_, gd::table::dto::table* ptable_)  
{
   bool bAscending = true;
   int iColumn = -1;
   if( ptable_ == nullptr ) { ptable_ = CACHE_Get(stringId, false); }
                                                                                                   assert(ptable_ != nullptr);
   if(ptable_->size() == 0) { return { true, "" }; } // empty table, nothing to sort

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


void CDocument::MESSAGE_Display(const std::string_view& stringMessage)
{
   m_papplication->PrintMessage(stringMessage, gd::argument::arguments() );   // display message in application window
}

void CDocument::MESSAGE_Display(const std::string_view& stringMessage, const gd::argument::arguments& argumentsMessage )
{
   m_papplication->PrintMessage(stringMessage, argumentsMessage );            // display message in application window
}

void CDocument::MESSAGE_Display(const gd::table::dto::table* ptable_, tag_state)
{
   std::string stringTable = gd::table::to_string(*ptable_, gd::table::tag_io_cli{});
   m_papplication->PrintMessage(stringTable, gd::argument::arguments() );     // display message in application window
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
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexError );           // locks `m_vectorError`
   gd::argument::arguments argumentsError( { {"text", stringError}, {"type", "error"} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsError) );
}

/** ---------------------------------------------------------------------------
 * @brief Add warning to internal list of problems
 * @param stringWarning warning information
 */
void CDocument::ERROR_AddWarning( const std::string_view& stringWarning )
{
   if( GetApplication()->PROPERTY_Exists( "verbose" ) == false || GetApplication()->PROPERTY_Get( "verbose" ).as_bool() == false ) return; // only add warning in verbose mode
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexError );           // locks `m_vectorError`
   gd::argument::arguments argumentsWarning( { {"text", stringWarning}, {"type", "warning"} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsWarning) );
}


void CDocument::ERROR_Print( bool bClear ) 
{
   std::shared_lock<std::shared_mutex> lock_( m_sharedmutexError );           // locks `m_vectorError`
   if( m_vectorError.empty() == true ) return;                                // no errors, exit

   for( const auto& itError : m_vectorError )
   {
      std::string stringError = itError["text"].as_string();
      if( stringError.empty() == false ) { m_papplication->PrintError(stringError, gd::argument::arguments() ); } // print error message
   }

   if (bClear == true) m_vectorError.clear();                                 // clear error list

}


