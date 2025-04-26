/**
 * @file Command.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * 
 */

#include "gd/gd_file.h"
#include "gd/gd_table_io.h"
#include "gd/gd_table_aggregate.h"
#include "gd/gd_utf8.h"
#include "gd/parse/gd_parse_window_line.h"

#include "Command.h"

/*
int CountRowsInFile(const gd::table::dto::table& table_)
{

   int iCount = 0;

   gd::table::dto::table tableTemp = table_;

   for( const auto& itRow : tableTemp )
   {
      auto value_ = itRow.cell_get_variant_view( "path" );
      std::string stringFile = value_.as_string();
      if( std::filesystem::is_regular_file(stringFile) == true )
      {
         std::ifstream ifstreamFile(stringFile);
         std::string stringText;

         while( std::getline(ifstreamFile, stringText) )
         {
            iCount++;
         }

         ifstreamFile.close();
      }
   }

   return iCount;
}



int RowCount( const std::string& stringFile )
{
   if( std::filesystem::is_regular_file(stringFile) == true )
   {
      std::ifstream ifstreamFile(stringFile);
      std::string stringText;
      int iCount = 0;

      while( std::getline(ifstreamFile, stringText) )
      {
         iCount++;
      }

      ifstreamFile.close();

      return iCount;
   }

   return 0;
}
*/

std::pair<bool, std::string> FILES_Harvest_g(const std::string& stringPath, gd::table::dto::table* ptable_, unsigned uDepth )
{                                                                                                  assert( ptable_ != nullptr );
   // ## add file to table
   auto add_ = [ptable_](const gd::file::path& pathFile)
   {
      auto uRow = ptable_->get_row_count();
      ptable_->row_add();

      ptable_->cell_set(uRow, "key", uRow + 1);
      auto folder_ = pathFile.parent_path().string();
      ptable_->cell_set(uRow, "folder", folder_);
      auto filename_ = pathFile.filename().string();
      ptable_->cell_set(uRow, "filename", filename_);
      ptable_->cell_set(uRow, "extension", pathFile.extension().string());

      // get file size
      std::string stringFilePath = pathFile.string();
      std::ifstream ifstreamFile(stringFilePath.data(), std::ios::binary | std::ios::ate);
      if( ifstreamFile.is_open() == true )
      {
         std::streamsize uSize = ifstreamFile.tellg();
         ptable_->cell_set(uRow, "size", uSize, gd::types::tag_convert{});
      }
      ifstreamFile.close();
   };

   try
   {
      for( const auto& it : std::filesystem::directory_iterator(stringPath) )
      {
         if( it.is_directory() == true )                                       // is file directory
         {
            if( uDepth > 0 )
            {
               auto stringChildPath = it.path().string();
               auto [bOk, stringError] = FILES_Harvest_g(stringChildPath, ptable_, (uDepth - 1) );// recursive call to harvest files in subdirectories
               if( bOk == false ) return { false, stringError };               // error in recursive call
            }
         }
         else
         {
            if( it.is_regular_file() == true )                
            {
               add_(gd::file::path(it.path()));
            }
         }
      }
   }
   catch( const std::filesystem::filesystem_error& e )
   {
      std::string stringError = e.what();
      return { false, stringError };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Harvests files from the specified path and populates a table with their details.
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @param ptable_ A pointer to the table where the harvested file details will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the harvesting was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> FILES_Harvest_g(const gd::argument::shared::arguments& argumentsPath, gd::table::dto::table* ptable_)
{                                                                                                  assert( ptable_ != nullptr );

   unsigned uRecursive = argumentsPath["recursive"].as_uint();
   std::string stringSource = argumentsPath["source"].as_string();
   auto vectorPath = gd::utf8::split(stringSource, ';');

   for( auto itPath : vectorPath )
   {
      auto [bOk, stringError] = FILES_Harvest_g(std::string(itPath), ptable_, uRecursive); // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if( bOk == false ) return { false, stringError };
   }


#ifndef NDEBUG
   auto stringTable = gd::table::to_string( *ptable_, gd::table::tag_io_cli{});
#endif

   return { true, "" };
}

std::pair<bool, std::string> COMMAND_CountRows(const gd::argument::shared::arguments& argumentsPath, gd::argument::shared::arguments& argumentsResult )
{
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);

   // ## Open file
   std::ifstream file_(stringFile, std::ios::binary);
   if( file_.is_open() == false ) return { false, "Failed to open file: " + stringFile };

   gd::parse::window::line line_(64 * 64, gd::types::tag_create{});           // 64 * 64 = 4096 bytes = 64 cache lines

   // Read the file into the buffer
   auto uAvailable = line_.available();
   file_.read((char*)line_.buffer(), uAvailable);  
   auto uSize = file_.gcount();
   line_.update(uSize);

   uint64_t uCountNewLine = 0;

   // ## Process the file
   while(line_.eof() == false)
   {
      uCountNewLine += line_.count('\n');                                      // count new lines in buffer

      // ## Rotate the buffer and read more data
      line_.rotate();                                                          // "rotate" data, move data from end of buffer to start of buffer
      file_.read((char*)line_.buffer(), line_.available());                    // fill buffer with data from file
      uSize = file_.gcount();
      line_.update(uSize);                                                     // update used size of internal buffer
   }

   argumentsResult.set("count", uCountNewLine);                                // set count of new lines in result

   
   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Adds a summary row to the specified table by calculating the sum of specified columns.
 * @param ptable_ A pointer to the table where the summary row will be added.
 * @param vectorColumnIndex A vector of column indices for which the sum will be calculated.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 * 
 * @code
 * // Example usage:
 * gd::table::dto::table table_;
 * ... populate the table with data ...
 * std::vector<unsigned> vectorColumnIndex = { 1, 2, 3 };
 * auto result_ = TABLE_AddSumRow(&table_, vectorColumnIndex); // add sum row and values for columns 1, 2, and 3
 * if (result_.first) { .. ommitted .. }
 * @endcode
 */
std::pair<bool, std::string> TABLE_AddSumRow(gd::table::dto::table* ptable_, const std::vector<unsigned>& vectorColumnIndex)
{                                                                                                  assert(ptable_ != nullptr);
   auto uRow = ptable_->get_row_count(); 
   ptable_->row_add( gd::table::tag_null{} );
   for( unsigned uColumnIndex : vectorColumnIndex )
   {                                                                                               assert( uColumnIndex < ptable_->get_column_count() );
      auto uSum = gd::table::sum<uint64_t>(*ptable_, uColumnIndex, 0, uRow);
      ptable_->cell_set(uRow, uColumnIndex, uSum, gd::table::tag_convert{});
   }

   return { true, "" };
}
