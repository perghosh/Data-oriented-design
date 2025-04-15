/**
 * @file Command.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * 
 */

#include "gd/gd_file.h"
#include "gd/gd_table_io.h"

#include "Command.h"

int CountRowsInFile(const gd::table::dto::table& table_)
{
   int iCount = 0;

   for( int i = 0; i < table_.get_row_count(); i++ )
   {
      auto argument_ = table_.cell_get(i, "path");

   }

   return 0;
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

/** ---------------------------------------------------------------------------
 * @brief Harvests files from the specified path and populates a table with their details.
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @param ptable_ A pointer to the table where the harvested file details will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the harvesting was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> HarvestFile_g(const gd::argument::shared::arguments& argumentsPath, gd::table::dto::table* ptable_)
{                                                                                                  assert( ptable_ != nullptr );
   std::string stringSource = argumentsPath["source"].as_string();

   auto add_ = [ptable_](const gd::file::path& pathFile)
   {
      auto uRow = ptable_->get_row_count();
      ptable_->row_add();

      std::string stringFilePath = pathFile.string();
      ptable_->cell_set(uRow, "path", stringFilePath);
      ptable_->cell_set(uRow, "extension", pathFile.extension().string());
      // get file size
      std::ifstream ifstreamFile(stringFilePath.data(), std::ios::binary | std::ios::ate);
      if( ifstreamFile.is_open() == true )
      {
         std::streamsize uSize = ifstreamFile.tellg();
         ptable_->cell_set(uRow, "size", uSize, gd::types::tag_convert{});
      }
      ifstreamFile.close();
   };

   // ## check if source is a file or directory

   if( std::filesystem::is_regular_file(stringSource) )                          // single file
   {
      add_(gd::file::path(stringSource));
   }
   else if( std::filesystem::is_directory(stringSource) )                       // is file directory
   {
      for( const auto& it : std::filesystem::directory_iterator(stringSource) )
      {
         add_(gd::file::path(it.path()));
      }
   }

#ifndef NDEBUG
   auto stringTable = gd::table::to_string( *ptable_, gd::table::tag_io_cli{});
   std::cout << "\n" << stringTable << "\n";
#endif

   return { true, "" };
}

