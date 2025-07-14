/**
 * @file Command.cpp
 * 
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0FileExtensions.PrepareState` - Prepares the state for file extensions used to parse.
 * 
 */

 // @TAG #command.refactor

#include <chrono>
#include <format>
#include <iterator>

#include "gd/gd_file.h"
#include "gd/gd_table_io.h"
#include "gd/gd_table_aggregate.h"
#include "gd/gd_utf8.h"
#include "gd/parse/gd_parse_window_line.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "automation/code-analysis/Expression.h"

#include "automation/code-analysis/Run.h"

#include "Command.h"

namespace detail {

   /// add file to table
   void add_file_to_table(const gd::file::path& pathFile, gd::table::dto::table* ptable_, bool bSize = false)
   {
      auto uRow = ptable_->row_add_one();

      ptable_->cell_set(uRow, "key", uRow + 1);
      auto folder_ = pathFile.parent_path().string();
      ptable_->cell_set(uRow, "folder", folder_);
      auto filename_ = pathFile.filename().string();
      ptable_->cell_set(uRow, "filename", filename_);
      ptable_->cell_set(uRow, "extension", pathFile.extension().string());

      // get file size
      if( bSize == true )
      {
         std::error_code errorcode_;                                           // error code for last write time
         auto time_ = std::filesystem::last_write_time(pathFile, errorcode_);
         if( !errorcode_ )
         { 
            auto sctp_ = std::chrono::time_point_cast<std::chrono::system_clock::duration>( time_ - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now() );
            auto now_ = std::chrono::system_clock::now();
            auto diff_ = now_ - sctp_;
            auto days_ = std::chrono::duration_cast<std::chrono::days>(diff_).count();
            ptable_->cell_set(uRow, "date", static_cast<double>(days_), gd::types::tag_convert{});
         }

         std::string stringFilePath = pathFile.string();
         std::ifstream ifstreamFile(stringFilePath.data(), std::ios::binary | std::ios::ate);
         if( ifstreamFile.is_open() == true )
         {
            std::streamsize uSize = ifstreamFile.tellg();
            ptable_->cell_set(uRow, "size", uSize, gd::types::tag_convert{});
         }
         ifstreamFile.close();
      }
   }

   /// add file to table if it matches wildcard filter
   bool add_file_to_table(const gd::file::path& pathFile, const std::string_view& stringWildcard, gd::table::dto::table* ptable_, bool bSize = false)
   {
      auto filename_ = pathFile.filename().string();                                               assert(filename_.empty() == false);
      if( stringWildcard.empty() == false )
      {
         char iSplit = ';';                                                   // separator for wildcards
         auto uPosition = stringWildcard.find_first_of(";,");
         if( uPosition != std::string_view::npos ) { iSplit = stringWildcard[uPosition]; } // use the first separator found

         std::vector<std::string_view> vectorWildcard = gd::utf8::split(stringWildcard, iSplit);
         bool bMatched = false;
         for( const auto& filter_ : vectorWildcard )
         {
            bool bMatch = gd::ascii::strcmp( filename_, filter_, gd::utf8::tag_wildcard{} );
            if( bMatch == true ) { bMatched = true; break; }
         }

         if( bMatched == false ) return false;                                 // no match, return false
      }

      auto uRow = ptable_->row_add_one();
      ptable_->cell_set(uRow, "key", uRow + 1);

      unsigned uColumnPath = ptable_->column_find_index("path");               // get column index for path
      if( uColumnPath != (unsigned)-1 )                                        // found "path" column ?
      {
         ptable_->cell_set(uRow, uColumnPath, pathFile.string());              // set path in table
      }
      else
      {
         auto folder_ = pathFile.parent_path().string();
         ptable_->cell_set(uRow, "folder", folder_);
         ptable_->cell_set(uRow, "filename", filename_);
      }

      ptable_->cell_set(uRow, "extension", pathFile.extension().string());

      // get file size
      if( bSize == true )
      {
         std::error_code errorcode_;                                           // error code for last write time
         auto time_ = std::filesystem::last_write_time(pathFile, errorcode_);
         if( !errorcode_ )
         { 
            auto sctp_ = std::chrono::time_point_cast<std::chrono::system_clock::duration>( time_ - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now() );
            auto now_ = std::chrono::system_clock::now();
            auto diff_ = now_ - sctp_;
            auto days_ = std::chrono::duration_cast<std::chrono::days>(diff_).count();
            ptable_->cell_set(uRow, "date", static_cast<double>(days_), gd::types::tag_convert{});
         }

         std::string stringFilePath = pathFile.string();
         std::ifstream ifstreamFile(stringFilePath.data(), std::ios::binary | std::ios::ate);
         if( ifstreamFile.is_open() == true )
         {
            std::streamsize uSize = ifstreamFile.tellg();
            ptable_->cell_set(uRow, "size", uSize, gd::types::tag_convert{});
         }
         ifstreamFile.close();
      }

      return true;                                                             // match, return true
   }

}


 /** --------------------------------------------------------------------------
  * @brief Harvests files from a specified directory path and populates a table with their details.
  *
  * This method recursively traverses the directory structure starting from the given path, 
  * collecting information about each file and storing it in the provided table. The information 
  * includes the file's folder, filename, extension, and size.
  *
  * @param stringPath The root directory path to start harvesting files from.
  * @param stringWildcard A wildcard pattern to filter files. Only files matching this pattern will be added to the table.
  * @param ptable_ A pointer to the table where the harvested file details will be stored.
  *                The table must be pre-initialized and not null.
  * @param uDepth The maximum depth for recursive traversal. A value of 0 means no recursion.
  * @return A pair containing:
  *         - `bool`: `true` if the harvesting was successful, `false` otherwise.
  *         - `std::string`: An empty string on success, or an error message on failure.
  *
  * @note The method uses `std::filesystem` for directory traversal and file operations.
  *       If an error occurs during traversal (e.g., permission issues), the method will 
  *       return `false` along with the error message.
  *
  * @example
  * @code
  * gd::table::dto::table table;
  * std::string path = "/example/directory";
  * unsigned depth = 2;
  * auto result = FILES_Harvest_g(path, &table, depth);
  * if(result.first) {
  *     std::cout << "Files harvested successfully." << std::endl;
  * } else {
  *     std::cerr << "Error: " << result.second << std::endl;
  * }
  * @endcode
  * 
  * @verbatim
  * 
  * @endverbatim
  */
std::pair<bool, std::string> FILES_Harvest_g(const std::string& stringPath, const std::string& stringWildcard, gd::table::dto::table* ptable_, unsigned uDepth, bool bSize )
{                                                                                                  assert( ptable_ != nullptr );
   try
   {
      if( std::filesystem::is_directory(stringPath) == false )                 // not a directory
      {
         if( std::filesystem::is_regular_file(stringPath) == true )            // is file
         {
            detail::add_file_to_table(gd::file::path(stringPath), stringWildcard, ptable_, bSize);
            return { true, "" };
         }
         else
         {
            return { false, "Path is not a directory or file: " + stringPath };
         }
      }

      for( const auto& it : std::filesystem::directory_iterator(stringPath) )
      {
         if( it.is_directory() == true )                                      // is file directory
         {
            if( papplication_g->IsState( CApplication::eApplicationStateCheckIgnoreFolder ) == true )
            {
               auto stringDirectory = it.path().string();
               // convert to forward slashes for consistency
               std::replace(stringDirectory.begin(), stringDirectory.end(), '\\', '/');

               bool bIgnore = papplication_g->IGNORE_Match( stringDirectory );
               if( bIgnore == true ) continue;                                // ignore this directory
            }

            if( uDepth > 0 )
            {
               auto stringChildPath = it.path().string();
               auto [bOk, stringError] = FILES_Harvest_g(stringChildPath, stringWildcard, ptable_, (uDepth - 1), bSize );// recursive call to harvest files in subdirectories
               if( bOk == false ) return { false, stringError };               // error in recursive call
            }
         }
         else
         {
            if( it.is_regular_file() == true )                
            {
               try
               {
                  //std::string string_ = it.path().string();
                  gd::file::path path_(it);
                  if( papplication_g->IsState( CApplication::eApplicationStateCheckIgnoreFile ) == true )
                  {
                     std::string string_ = path_.filename().string();
                     bool bIgnore = papplication_g->IGNORE_MatchFilename( string_ );
                     if( bIgnore == true ) continue;
                  }

                  detail::add_file_to_table(path_, stringWildcard, ptable_, bSize);
               }
               catch( const std::exception& e )
               {
                  papplication_g->ERROR_Add("Error reading file: " + std::string(e.what()));
               }
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
   std::string stringFilter = argumentsPath["filter"].as_string();

   auto vectorPath = gd::utf8::split(stringSource, ';');

   for( auto itPath : vectorPath )
   {
      auto [bOk, stringError] = FILES_Harvest_g(std::string(itPath), stringFilter, ptable_, uRecursive); // harvest (read) files based on source, source can be a file or directory or multiple separated by ;
      if( bOk == false ) return { false, stringError };
   }


#ifndef NDEBUG
   auto stringTable = gd::table::to_string( *ptable_, gd::table::tag_io_cli{});
#endif

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Reads lines from a file starting at a specified row with an offset and count.
 *
 * This method reads lines from the specified file, starting at the given row (with an optional offset),
 * and returns a specified number of lines. The lines are concatenated into a single string.
 *
 * @param stringPath The path to the file to read lines from.
 * @param uRow The starting row (0-based index) from which to begin reading lines.
 * @param iOffset An offset to apply to the starting row. Can be negative to move up or positive to move down.
 * @param iCount The number of lines to read from the file.
 * @param stringLines A reference to a string where the read lines will be concatenated.
 * @param piLeadingLineCount A pointer to an integer where the number of leading lines (lines before the starting row) will be stored. Can be `nullptr` if not needed.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> FILES_ReadLines_g(const std::string& stringPath, uint64_t uRow, int64_t iOffset, uint64_t uCount, std::string& stringLines, int64_t* piLeadingLineCount )
{                                                                                                  assert( stringPath.empty() == false );
   if( std::filesystem::is_regular_file(stringPath) == false ) return { false, "File not found: " + stringPath };

   std::ifstream file_(stringPath, std::ios::binary);
   if( file_.is_open() == false ) return { false, "Failed to open file: " + stringPath };

   // Calculate the starting line (with offset)
   int64_t iStartLine = static_cast<int64_t>(uRow) + static_cast<int64_t>(iOffset);
   if( iStartLine < 0 ) iStartLine = 0;
   uint64_t uStartLine = static_cast<uint64_t>(iStartLine); // ensure we work with unsigned for line counting

   // Calculate the leading line count if requested, this is to be used to know how many lines were before the starting line
   // and to mark what the important line is in the concatenated stringLines
   if( piLeadingLineCount != nullptr ) { *piLeadingLineCount = (uRow - uStartLine) - 1; } // calculate leading lines

   std::string stringLine;
   uint64_t uCurrentLine = 0;
   uint64_t uLinesRead = 0;

   while( std::getline(file_, stringLine) )
   {
      if( uCurrentLine >= uStartLine )
      {
         iOffset++;

         stringLines += stringLine;
         stringLines += '\n';
         uLinesRead++;
         if( uLinesRead >= uCount ) break;
      }
      uCurrentLine++;
   }

   if( uLinesRead == 0 ) return { false, "No lines read from file (check row/offset/count)" };

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Reads a full row from a file and populates the corresponding line in the table.
 *
 * This method reads lines from the specified file starting at the given row offset in table,
 * and populates the corresponding line in the table. It uses a window buffer to read
 * lines efficiently and handles line breaks correctly.
 * 
 * When parsing the file for matching patterns it only reads parts of lines that are relevant and that is not the same as what the user likes to see when results are displayed.
 * Users what to see the full line from source code, so this method reads the full line from the file and places it in the table.
 *
 * @param pifstream A pointer to an input file stream from which to read lines.
 * @param ptable_ A pointer to the table where the lines will be stored.
 * @param uRowStartOffset The starting row offset in the table from which to begin reading lines.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> FILES_ReadFullRow_g(std::ifstream* pifstream, gd::table::dto::table* ptable_, uint64_t uRowStartOffset)
{                                                                                                  assert(ptable_ != nullptr); assert(uRowStartOffset < ptable_->get_row_count() ); assert( pifstream != nullptr );
   unsigned uColumnRow = ptable_->column_get_index("row");                     // get column index for row
   unsigned uColumnLine = ptable_->column_get_index("line");                   // get column index for line

   uint64_t uFileReadLine = 0;                                                 // read line counter, this is used to count lines read from file and match with row in table
   gd::parse::window::line windowLine_(8192 - 512, 8192, gd::types::tag_create{});// create line buffer

   auto uAvailable = windowLine_.available();
   pifstream->read((char*)windowLine_.buffer(), uAvailable);
   auto uReadSize = pifstream->gcount();                                        // get number of valid bytes read
   windowLine_.update(uReadSize);                                              // Update valid size in line buffer

   std::string stringLine;                                                     // string to hold line read from file

   auto uRow = uRowStartOffset;
   uint64_t uReadRow = ptable_->cell_get<uint64_t>(uRow, uColumnRow);          // get row from table

   // ## Scan file and read lines found in table
   while( windowLine_.eof() == false )
   {
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});         // get range of valid data in buffer
      for( auto it = first_; it < last_; it++ ) 
      {
         if( *it == '\n' )                                                     // count lines read from file
         {
            if( uFileReadLine == uReadRow )                                    // is we at the line we want to read
            {
               stringLine = gd::utf8::trim_to_string(stringLine);              // trim line
               ptable_->cell_set(uRow, uColumnLine, stringLine);               // set line in table

               // ## read next row from table and make sure it is larger than the previous row
               auto uReadRowOld = uReadRow;
               do{
                  uRow++;
                  if( uRow >= ptable_->get_row_count() ) { return { true, "" }; } // if we reached end of table, return success
                  uReadRow = ptable_->cell_get<uint64_t>(uRow, uColumnRow);    // get row from table
               } while( uReadRow == uReadRowOld );
            }

            stringLine.clear();
            uFileReadLine++;                                 
         }
         else if( uFileReadLine == uReadRow )                                  // found line to read
         {
            stringLine += *it;
         }
      }

      windowLine_.rotate();                                                   // rotate buffer

      if( uReadSize > 0 )                                                     // was it possible to read data last read, then more data is available
      {
         auto uAvailable = windowLine_.available();                           // get available space in buffer to be filled
         pifstream->read((char*)windowLine_.buffer(), windowLine_.available());// read more data into available space in buffer
         uReadSize = pifstream->gcount();
         windowLine_.update(uReadSize);                                       // update valid size in line buffer
      }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Cleans a file by reading its content and processing it according to the provided arguments.
 *
 * This method reads the content of the specified file, processes it based on the provided arguments,
 * and stores the cleaned content in the provided string buffer. It handles state activation and deactivation
 * for parsing expressions within the file.
 *
 * @param stringPath The path to the file to be cleaned.
 * @param argumentsOption The arguments containing options for cleaning, such as whether to keep newlines.
 * @param stringBuffer A reference to a string where the cleaned content will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> CLEAN_File_g(const std::string& stringPath, const gd::argument::shared::arguments& argumentsOption, std::string& stringBuffer)
{
   enum { eStateCode = 0x01, eStateComment = 0x02, eStateString = 0x04 }; // states for code, comment and string

   unsigned uSegment = 0; // default state is code

   // ## Open file

   if( std::filesystem::is_regular_file(stringPath) == false ) return { false, "File not found: " + stringPath }; // check if file exists

   std::ifstream file_(stringPath, std::ios::binary);
   if( file_.is_open() == false ) return { false, "Failed to open file: " + stringPath };

   // ## check what type of segmentation we want to keep, default is to keep code segment
   if( argumentsOption.exists("segment") == true )
   {
      auto stringSegment = argumentsOption["segment"].as_string();
      if( stringSegment == "comment" ) { uSegment |= eStateComment; } 
      else if( stringSegment == "string" ) { uSegment |= eStateString; } // store state for string segment
      else if( stringSegment == "all" ) { uSegment = eStateCode | eStateComment | eStateString; } // store states for all segments
   }


   bool bKeepNwline = argumentsOption.get_argument<bool>("newline", true);    // keep newlines ?
   gd::parse::window::line windowLine_(8192 - 512, 8192, gd::types::tag_create{});// create line buffer

   std::string stringFile = stringPath;                                                            assert(stringFile.empty() == false);
   gd::expression::parse::state state_; // state for parsing expressions
   auto result_ = CApplication::PrepareState_s( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                                // error in state preparation

   auto uAvailable = windowLine_.available(); // get available space in buffer to be filled
   file_.read((char*)windowLine_.buffer(), uAvailable);                        // read more data into available space in buffer
   auto uSize = file_.gcount(); // get number of valid bytes read
   windowLine_.update(uSize);                                                  // Update valid size in line buffer                  
   // ## Scan file and read lines found in table
   while( windowLine_.eof() == false )
   {
      auto [first_, last_] = windowLine_.range(gd::types::tag_pair{});     // get range of valid data in buffer

      if( uSegment == 0 )
      { 
         for( const auto* it = first_; it < last_; it++ ) 
         {
            if( state_.in_state() == false )                                  // not in a state? that means we are reading source code
            {
               // ## check if we have found state
               if( state_[*it] != 0 && state_.exists( it ) == true )
               {
                  unsigned uLength = (unsigned)state_.activate(it);           // activate state
                  if( uLength > 1 ) it += ( uLength - 1 );                    // skip to end of state marker and if it is more than 1 character, skip to end of state
               }
               else
               {
                  stringBuffer.push_back(*it);                                // add character to buffer
               }
            }
            else
            {
               // ## check if we have found end of state
               unsigned uLength;
               if( state_.deactivate( it, &uLength ) == true ) 
               {
                  if( uLength > 1 ) it += (uLength - 1);                      // skip to end of state marker and if it is more than 1 character, skip to end of state
                  // check for ending linebreak 
                  if( *it == '\n' && bKeepNwline == true ) stringBuffer.push_back(*it);// add newline to buffer if we keep newlines
                  continue;
               }

               if( *it == '\n' && bKeepNwline == true ) stringBuffer.push_back(*it);
            }
         }
      }
      else
      {
         for( const auto* it = first_; it < last_; it++ ) 
         {
            if( state_.in_state() == false )                                  // not in a state? that means we are reading source code
            {
               // ## check if we have found state
               if( state_[*it] != 0 && state_.exists( it ) == true )
               {
                  unsigned uLength = (unsigned)state_.activate(it);           // activate state
                  if( uLength > 1 ) it += ( uLength - 1 );                    // skip to end of state marker and if it is more than 1 character, skip to end of state
               }
               else
               {
                  if( (uSegment & eStateCode) != 0 )                          // if we are in code segment
                  {
                     stringBuffer.push_back(*it);                              // add character to buffer
                  }
                  else
                  {
                     if( *it == '\n' && bKeepNwline == true ) stringBuffer.push_back(*it);
                  }
               }
            }
            else
            {
               // ## check if we have found end of state
               unsigned uLength;
               if( state_.deactivate( it, &uLength ) == true ) 
               {
                  if( uLength > 1 ) it += (uLength - 1);                      // skip to end of state marker and if it is more than 1 character, skip to end of state
                  if( *it == '\n' && bKeepNwline == true ) stringBuffer.push_back(*it);// add newline to buffer if we keep newlines
                  continue;                                                   // move on
               }

               if( (uSegment & eStateComment) != 0 && state_.is_comment() == true ) // in comment state?
               {
                  stringBuffer.push_back(*it);
               }
               else if( (uSegment & eStateString) != 0 && state_.is_string() == true ) // in string state?
               {
                  stringBuffer.push_back(*it);
               }
               else if( *it == '\n' && bKeepNwline == true )                   // if we are not in a selected state and we have a newline
               {
                  stringBuffer.push_back(*it);                                  // add newline to buffer
               }
            }
         }
      }

      windowLine_.rotate();                                                   // rotate buffer

      auto uAvailable = windowLine_.available();                              // get available space in buffer to be filled
      file_.read((char*)windowLine_.buffer(), windowLine_.available());       // read more data into available space in buffer
      uSize = file_.gcount();
      windowLine_.update(uSize);                                              // update valid size in line buffer
   }

   file_.close();

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Counts the number of rows in a file.
 * 
 * This method reads the specified file and counts the number of rows (lines) in it.
 * The input and output are passed through the `argumentsPath` and `argumentsResult` containers.
 * 
 * @param argumentsPath The arguments container containing the input parameters:
 *        - `source` (string): The source file path to count rows in.
 * @param argumentsResult The arguments container to store the result:
 *        - `count` (uint64_t): The number of rows counted in the file.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 * 
 * ### Example Usage:
 * @code
 * gd::argument::shared::arguments argsPath;
 * argsPath.set("source", "example.txt");
 * 
 * gd::argument::shared::arguments argsResult;
 * 
 * auto result = COMMAND_CountRows(argsPath, argsResult);
 * if(result.first) {
 *     std::cout << "Row count: " << argsResult["count"].as_uint64() << std::endl;
 * } else {
 *     std::cerr << "Error: " << result.second << std::endl;
 * }
 * @endcode
 */
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
 * @brief Collects file statistics from the specified source file.
 *
 * This method reads the specified file and collects various statistics, including:
 * - Total number of lines
 * - Number of code lines
 * - Number of code characters
 * - Number of comment sections
 * - Number of string sections
 *
 *
 * ### Steps:
 * 1. **Prepare the source file**:
 *    - Retrieve the file path from the `argumentsPath` container.
 *    - Validate that the file exists and is a regular file.
 *    - Open the file in binary mode for reading.
 * 2. **Initialize the parsing state**:
 *    - Determine the file type based on its extension.
 *    - Configure the parsing state to handle comments, strings, and other relevant constructs.
 * 3. **Read the file into a buffer**:
 *    - Allocate a buffer for reading the file in chunks.
 *    - Read the file data into the buffer and update the buffer state.
 * 4. **Process the file content**:
 *    - Iterate through the buffer to analyze each character.
 *    - Identify and count lines, code characters, comment sections, and string sections.
 *    - Handle transitions between different states (e.g., entering or exiting a comment or string).
 * 5. **Update counters**:
 *    - Increment counters for lines, code lines, code characters, comments, and strings as appropriate.
 * 6. **Store the results**:
 *    - Populate the `argumentsResult` container with the collected statistics.
 *    - Include the total counts for lines, code lines, code characters, comments, and strings.
 * 7. **Return the result**:
 *    - Return a success flag (`true`) and an empty error message on success.
 *    - If an error occurs (e.g., file not found or failed to open), return `false` and an appropriate error message.
 *
 *
 * @param argumentsPath The arguments container containing the input parameters:
 *        - `source` (string): The source file path to collect statistics from.
 * @param argumentsResult The arguments container to store the result:
 *        - `count` (uint64_t): The number of lines counted in the file.
 *        - `code` (uint64_t): The number of code lines counted in the file.
 *        - `characters` (uint64_t): The number of code characters counted in the file.
 *        - `comment` (uint64_t): The number of comment sections counted in the file.
 *        - `string` (uint64_t): The number of string sections counted in the file.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> COMMAND_CollectFileStatistics(const gd::argument::shared::arguments& argumentsPath, gd::argument::shared::arguments& argumentsResult)
{
   // ## prepare source file
   // get file from "source" argument
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);

   // ### Open file
   if( std::filesystem::is_regular_file(stringFile) == false ) return { false, "File not found: " + stringFile };
   std::ifstream file_(stringFile, std::ios::binary);
   if( file_.is_open() == false ) return { false, "Failed to open file: " + stringFile };

   gd::parse::window::line lineBuffer(4096, gd::types::tag_create{});         // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   // ## counters
   //    Lots of counters to count different things in the file, this is not easy to follow so be careful

   uint64_t uCountNewLine = 0;      // counts all new lines in file (all '\n' characters)
   uint64_t uCountCodeLines = 0;    // counts all code lines in file (if there are visible characters in the line)
   uint64_t uCountCodeCharacters = 0; // counts all code characters in file
   uint64_t uCountComment = 0;      // counts all comment sections
   uint64_t uCountString = 0;       // counts all string sections

   uint64_t uRowCharacterCodeCount = 0; // number of characters of code in current row (helper variable)

   std::string stringSourceCode;    // gets source code for analysis

   gd::expression::parse::state state_; //
   auto result_ = CApplication::PrepareState_s( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                                // error in state preparation

   // if no states are defined, count rows in file
   if( state_.empty() == true ) { return COMMAND_CountRows(argumentsPath, argumentsResult); } 

   // ## Read the file into the buffer
   auto uAvailable = lineBuffer.available();
   file_.read((char*)lineBuffer.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   // ## Process the file
   while(lineBuffer.eof() == false)
   {
      uCountNewLine += lineBuffer.count('\n');                                // count all new lines in buffer

      auto [first_, last_] = lineBuffer.range(gd::types::tag_pair{});

      for(auto it = first_; it < last_; it++ ) 
      {
         if( state_.in_state() == false )                                     // not in a state? that means we are reading source code
         {
            // ## check if we have found state
            if( state_[*it] != 0 && state_.exists( it ) == true )
            {
               stringSourceCode.clear();                                      // clear source code
               state_.activate(it);                                           // activate state
               
               // If multiline and `uRowCharacterCodeCount` is not 0 that means that there are characters in the code section before multiline
               if( uRowCharacterCodeCount > 0 && state_.is_multiline() == false ) 
               { 
                  uCountCodeLines++; 
                  uRowCharacterCodeCount = 0;
               }

               // ## check type of state that was activated
#ifndef NDEBUG
               std::string_view stringState_d = gd::expression::parse::state::get_string_s( state_.get_state() );
#endif // !NDEBUG

               if( state_.is_comment() == true ) uCountComment++;             // count comment sections
               else if( state_.is_string() == true ) uCountString++;          // count string sections


               continue;
            }

            stringSourceCode += *it;                                          // add character to source code
            if( *it == '\n' ) 
            { 
               if( uRowCharacterCodeCount ) uCountCodeLines++;                // count code lines if there are characters in the line
               uRowCharacterCodeCount = 0;                                    // reset code character count for next line
            }
            else if( gd::expression::is_code_g( *it ) != 0 ) 
            { 
               uRowCharacterCodeCount++;                                      // count all code characters in line
               uCountCodeCharacters++;                                        // count all code characters in file
            }
         }
         else
         {
            // ## check if we have found end of state
            unsigned uLength;
            if( state_.deactivate( it, &uLength ) == true ) 
            {
               if( uLength > 1 ) it += (uLength - 1);                         // skip to end of state marker and if it is more than 1 character, skip to end of state
               continue;
            }
         }
      }

      lineBuffer.rotate();                                                    // rotate buffer

      if( uReadSize > 0 )                                                     // was it possible to read data last read, then more data is available
      {
         auto uAvailable = lineBuffer.available();                            // get available space in buffer to be filled
         file_.read((char*)lineBuffer.buffer(), lineBuffer.available());      // read more data into available space in buffer
         uReadSize = file_.gcount();
         lineBuffer.update(uReadSize);                                        // update valid size in line buffer
      }

   }

   argumentsResult.set("count", uCountNewLine);                               // set count of new lines in result
   argumentsResult.set("code", uCountCodeLines);                              // set count of code lines in result
   argumentsResult.set("characters", uCountCodeCharacters);                   // set count of code characters in result
   argumentsResult.set("comment", uCountComment);                             // set count of comment sections in result
   argumentsResult.set("string", uCountString);                               // set count of string sections in result

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Collects pattern statistics from the specified source file.
 *
 * This method reads the specified file and counts occurrences of each pattern
 * in the source code. The patterns are provided in the `vectorPattern` parameter.
 * 
  * ### Steps:
 * 1. **Prepare the source file**:
 *    - Validate the file path and ensure it exists.
 *    - Open the file in binary mode for reading.
 * 2. **Initialize the state**:
 *    - Prepare the parsing state based on the file extension.
 *    - Set up patterns to count occurrences in the source code.
 * 3. **Read and process the file**:
 *    - Read the file into a buffer.
 *    - Iterate through the buffer to identify and count patterns.
 *    - Handle different states (e.g., code, comments, strings) appropriately.
 * 4. **Count occurrences**:
 *    - For each pattern, count its occurrences in the relevant sections of the file.
 *    - Store the counts in the `vectorCount` parameter.
 * 5. **Return the result**:
 *    - Return a success flag and an error message (if any).
 *
 * @param argumentsPath The arguments container containing the input parameters:
 *        - `source` (string): The source file path to collect statistics from.
 * @param vectorPattern A vector of strings representing the patterns to count.
 * @param vectorCount A reference to a vector where the counts of each pattern will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> COMMAND_CollectPatternStatistics(const gd::argument::shared::arguments& argumentsPath, const std::vector<std::string>& vectorPattern, std::vector<uint64_t>& vectorCount)
{
   enum { eStateCode = 0x01, eStateComment = 0x02, eStateString = 0x04 }; // states for code, comment and string

   unsigned uFindInState = eStateCode; // state of the parser

   // ## if state is sent than try to figure out from what state to find patterns
   if( argumentsPath.exists("state") == true )
   {
      std::string stringState = argumentsPath["state"].as_string();
      if( stringState == "comment" ) uFindInState = eStateComment;
      else if( stringState == "string" ) uFindInState = eStateString;
      else if( stringState == "code" ) uFindInState = eStateCode;
      else if( stringState == "all" ) uFindInState = ( eStateComment | eStateString | eStateCode );
   }


   gd::parse::patterns patternsFind(vectorPattern); // patterns to find in source code
   patternsFind.sort();                                                       // Sort patterns by length, longest first

   // ## Prepare source file
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);

   // ### Open file
   if( std::filesystem::is_regular_file(stringFile) == false ) return { false, "File not found: " + stringFile };
   std::ifstream file_(stringFile, std::ios::binary);
   if(file_.is_open() == false) return { false, "Failed to open file: " + stringFile };

   gd::parse::window::line lineBuffer(48 * 64, 64 * 64, gd::types::tag_create{});  // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   gd::expression::parse::state state_; // state is used to check what type of code part we are in
   auto result_ = CApplication::PrepareState_s( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                                // error in state preparation

   // ## count occurrences of each pattern in the source code
   auto count_ = [&patternsFind, &vectorPattern, &vectorCount](const std::string& stringText) // count method that counts occurrences of each pattern in the source code
      {
         // ## Count occurrences of each pattern in text

         const char* piPosition = stringText.c_str();
         const char* piEnd = piPosition + stringText.length();
         uint64_t uOffset = 0;
         int64_t iPattern = 0;
         while( (iPattern = patternsFind.find_pattern(piPosition, piEnd, &uOffset)) != -1 ) // find pattern in text
         {
            piPosition += uOffset;                                             // Move to position
            const std::string_view stringPattern = patternsFind.get_pattern(iPattern);  // get pattern from patternsFind

            ///## find pattern in vector
            for( auto it = vectorPattern.begin(); it != vectorPattern.end(); it++ ) // find pattern in vector
            {
               if( *it == stringPattern )
               {
                  vectorCount[std::distance( vectorPattern.begin(), it )]++;
                  piPosition += stringPattern.length();                        // Move past the current match
                  break;
               }
            }
         }
      };


   // ## Initialize pattern counts
   vectorCount.clear();                                                        // clear any previous counts
   vectorCount.resize(vectorPattern.size(), 0);

   std::string stringSourceCode; // gets source code for analysis
   std::string stringText;       // gets text for analysis
   uint64_t uRowCharacterCodeCount = 0; // number of characters of code in current row (helper variable)

   // ## Read the file into the buffer
   auto uAvailable = lineBuffer.available();
   file_.read((char*)lineBuffer.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   // ## Process the file
   while(lineBuffer.eof() == false)
   {
      auto [first_, last_] = lineBuffer.range(gd::types::tag_pair{});

      for(auto it = first_; it < last_; it++ ) 
      {
         if( state_.in_state() == false )                                     // not in a state? that means we are reading source code
         {
            // ## check if we have found state
            if( state_[*it] != 0 && state_.exists( it ) == true )
            {
               if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) ) count_(stringSourceCode); // count patterns in source code
               stringSourceCode.clear();                                      // clear source code
               state_.activate(it);                                           // activate state

               // If multiline and `uRowCharacterCodeCount` is not 0 that means that there are characters in the code section before multiline
               if( uRowCharacterCodeCount > 0 && state_.is_multiline() == false ) 
               { 
                  if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) ) count_(stringSourceCode); // count patterns in source code
                  stringSourceCode.clear();
                  uRowCharacterCodeCount = 0;
               }

               continue;
            }

            stringSourceCode += *it;                                          // add character to source code
            if( *it == '\n' ) 
            { 
               if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) ) count_(stringSourceCode); // count patterns in source code
               stringSourceCode.clear();
               uRowCharacterCodeCount = 0;                                    // reset code character count for next line
            }
            else if( gd::expression::is_code_g( *it ) != 0 ) 
            { 
               uRowCharacterCodeCount++;                                      // count all code characters in line
            }
         }
         else
         {
            stringText += *it;                                                // add character to text for analysis
            // ## check if we have found end of state
            unsigned uLength;
            if( state_.deactivate( it, &uLength, gd::expression::parse::state::tag_manual{}) == true ) 
            {
               if( uFindInState & (eStateComment|eStateString) )
               {
                  if( state_.is_comment() == true && (uFindInState & eStateComment) )
                  {
                     count_(stringText);
                  }
                  else if( state_.is_string() == true && ( uFindInState & eStateString ) )
                  {
                     count_(stringText);
                  }
               }
               state_.clear_state();                                           // clear state
               stringText.clear();                                             // clear text for analysis
               
               if( uLength > 1 ) it += ( uLength - 1 );                       // skip to end of state marker and if it is more than 1 character, skip to end of state
               continue;
            }
         }
      }

      lineBuffer.rotate();                                                    // rotate buffer

      if( uReadSize > 0 )                                                     // was it possible to read data last read, then more data is available
      {
         file_.read((char*)lineBuffer.buffer(), lineBuffer.available());      // read more data into available space in buffer
         uReadSize = file_.gcount();
         lineBuffer.update(uReadSize);                                        // update valid size in line buffer
      }
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Lists lines in a file that match specified patterns.
 *
 * This method reads the specified file and identifies lines that match any of the given patterns.
 * The matching lines, along with their details (e.g., line number, column, and file information),
 * are stored in the provided table.
 *
 * ### Steps:
 * 1. **Prepare the source file**:
 *    - Retrieve the file path from the `argumentsPath` container.
 *    - Validate that the file exists and is a regular file.
 *    - Open the file in binary mode for reading.
 * 2. **Initialize the parsing state**:
 *    - Determine the file type based on its extension.
 *    - Configure the parsing state to handle comments, strings, and other relevant constructs.
 * 3. **Read and process the file**:
 *    - Read the file into a buffer in chunks.
 *    - Iterate through the buffer to analyze each line and character.
 *    - Identify and handle different states (e.g., code, comments, strings).
 * 4. **Match patterns**:
 *    - For each line, check if it matches any of the specified patterns.
 *    - If a match is found, record the line, its row, column, and other details in the table.
 * 5. **Store results**:
 *    - Populate the `ptable_` with details of all matching lines.
 * 6. **Return the result**:
 *    - Return a success flag (`true`) and an empty error message on success.
 *    - If an error occurs (e.g., file not found or failed to open), return `false` and an appropriate error message.
 *
 * @param argumentsPath The arguments container containing the input parameters:
 *        - `source` (string): The source file path to search for patterns.
 *        - `file-key` (uint64_t): A unique key identifying the file in the main table.
 *        - `state` (string): The state to search for patterns in (e.g., "comment", "string", "code", "all").
 * @param patternsFind The patterns to search for in the file.
 * @param ptable_ A pointer to the table where the matching lines will be stored.
 *        Each row in the table contains:
 *        - `key`: A unique identifier for the row.
 *        - `file-key`: The unique key identifying the file.
 *        - `filename`: The name of the file being processed.
 *        - `line`: The content of the matching line.
 *        - `row`: The line number of the match.
 *        - `column`: The column number where the match starts.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> COMMAND_ListLinesWithPattern(const gd::argument::shared::arguments& argumentsPath, const gd::parse::patterns& patternsFind, gd::table::dto::table* ptable_)
{
   enum { eStateCode = 0x01, eStateComment = 0x02, eStateString = 0x04 }; // states for code, comment and string

   uint64_t uSaveRowCount = ptable_->get_row_count(); // save number of rows to fill in the full row for found rows at the end

   unsigned uFindInState = eStateCode; // state of the parser

   // ## if state is sent than try to figure out from what state to find patterns
   if( argumentsPath.exists("state") == true )
   {
      std::string stringState = argumentsPath["state"].as_string();
      if( stringState == "comment" ) uFindInState = eStateComment;
      else if( stringState == "string" ) uFindInState = eStateString;
      else if( stringState == "code" ) uFindInState = eStateCode;
      else if( stringState == "all" ) uFindInState = ( eStateComment | eStateString | eStateCode );
   }

   uint64_t uFileKey = argumentsPath["file-key"]; // key to file for main table holding activ files

   // ## prepare source file

   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);
   if( std::filesystem::is_regular_file(stringFile) == false ) return { false, "File not found: " + stringFile };
   gd::file::path pathFile(stringFile);
   std::string stringExtension = pathFile.extension().string();

   // ### Open file

   std::ifstream file_(stringFile, std::ios::binary);
   if(file_.is_open() == false) return { false, "Failed to open file: " + stringFile };

   gd::parse::window::line lineBuffer(48 * 64, 64 * 64, gd::types::tag_create{});  // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   gd::expression::parse::state state_; // state is used to check what type of code part we are in
   auto result_ = CApplication::PrepareState_s( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                               // error in state preparation


   uint64_t uCountNewLine = 0;                                                // counts all new lines in file (all '\n' characters)

   // ## Function to add line to table
   auto add_line_to_table_ = [uFileKey,ptable_,&stringFile,&patternsFind](int iPatternIndex, std::string& stringText, uint64_t uLineRow, uint64_t uColumn, const std::string_view& stringPattern ) 
      {  
         stringText = gd::utf8::trim_to_string(stringText);                   // trim

         // ## adds line with information about found pattern to table holding matches
         auto uRow = ptable_->row_add_one();
         ptable_->cell_set(uRow, "key", uRow + 1);
         ptable_->cell_set(uRow, "file-key", uFileKey);
         ptable_->cell_set(uRow, "filename", stringFile);
         ptable_->cell_set(uRow, "line", stringText);
         ptable_->cell_set(uRow, "row", uLineRow);
         ptable_->cell_set(uRow, "column", uColumn);
         ptable_->cell_set(uRow, "pattern", stringPattern, gd::types::tag_adjust{});
      };


   std::string stringSourceCode; // gets source code for analysis
   std::string stringText;       // gets text for analysis
   uint64_t uRowCharacterCodeCount = 0; // number of characters of code in current row (helper variable)


   // ## Read the file into the buffer
   auto uAvailable = lineBuffer.available();
   file_.read((char*)lineBuffer.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   // ## Process the file
   while(lineBuffer.eof() == false)
   {
      uCountNewLine += lineBuffer.count('\n');                                // count all new lines in buffer

      auto [first_, last_] = lineBuffer.range(gd::types::tag_pair{});         // get first and last position of buffer
#ifndef NDEBUG
      auto iState_d = state_.m_iActive;
      std::string_view stringText_d( (const char*)first_, last_ - first_ ); // for debug purposes
#endif // NDEBUG

      for(auto it = first_; it < last_; it++ ) 
      {
         if( state_.in_state() == false )                                     // not in a state? that means we are reading source code
         {
            // ## SOURCE CODE
            //    check if we have found state
            if( state_[*it] != 0 && state_.exists(it) == true )               // switch to state ?
            {
               if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) )
               {
                  uint64_t uColumn;
                  int iPattern = patternsFind.find_pattern(stringSourceCode, &uColumn); // try to find pattern in source code
                  if( iPattern != -1 )                                        // did we find a pattern?
                  {
                     // ## figure ot row and column
                     auto uRow = uCountNewLine; // row number for current buffer
                     auto uPosition = it - first_;
                     uRow -= lineBuffer.count('\n', uPosition);               // subtract number of new lines in buffer from current position to get the right row

                     std::string_view stringPattern = patternsFind.get_pattern(iPattern);
                     add_line_to_table_(iPattern, stringSourceCode, uRow, uColumn, stringPattern); // add line to table
                  }
               }
               stringSourceCode.clear();                                      // clear source code
               uRowCharacterCodeCount = 0;
               state_.activate(it);                                           // activate state
               continue;
            }

            
            if( *it == '\n' ) 
            { 
               if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) )
               {
                  uint64_t uColumn;
                  int iPattern = patternsFind.find_pattern(stringSourceCode, &uColumn); // try to find pattern in source code
                  if( iPattern != -1 )                                         // did we find a pattern?
                  {
                     // ## figure ot row and column
                     auto uRow = uCountNewLine; // row number for current buffer
                     auto uPosition = it - first_;
                     uRow -= lineBuffer.count('\n', uPosition);                // subtract number of new lines in buffer from current position to get the right row

                     std::string_view stringPattern = patternsFind.get_pattern(iPattern);
                     add_line_to_table_(iPattern, stringSourceCode, uRow, uColumn, stringPattern); // add line to table
                  }
               }

               stringSourceCode.clear();
               uRowCharacterCodeCount = 0;                                    // reset code character count for next line
               continue;
            }
            else if( gd::expression::is_code_g( *it ) != 0 ) 
            { 
               uRowCharacterCodeCount++;                                      // count all code characters in line
            }

            stringSourceCode += *it;                                          // add character to source code
         }
         else
         {  // Handle state that is outside code
            stringText += *it;                                                // add character to text for analysis
            // ## check if we have found end of state
            unsigned uLength;
            if( state_.deactivate(it, &uLength, gd::expression::parse::state::tag_manual{}) == true ) // NOTE: this needs manual reset of internal state
            {
               if( uFindInState & ( eStateComment | eStateString ) && stringText.empty() == false )        // if find text in comment or string
               {
                  if( ( state_.is_comment() == true && ( uFindInState & eStateComment ) ) ||
                      ( state_.is_string() == true && ( uFindInState & eStateString ) ) )
                  {
                     uint64_t uColumn;
                     int iPattern = patternsFind.find_pattern(stringText, &uColumn); // try to find pattern in string
                     if( iPattern != -1 )                                           // did we find a pattern?
                     {
                        // ## figure ot row and column
                        auto uRow = uCountNewLine; // row number for current buffer
                        auto uPosition = it - first_;
                        uRow -= lineBuffer.count('\n', uPosition);         // subtract number of new lines in buffer from current position to get the right row

                        std::string_view stringPattern = patternsFind.get_pattern(iPattern);
                        add_line_to_table_(iPattern, stringText, uRow, uColumn, stringPattern); // add line to table
                     }
                  }
               } // if( uFindInState & ( eStateComment | eStateString ) && stringText.empty() == false )

               state_.clear_state();                                           // clear state
               stringText.clear();                                             // clear text for analysis


               if( uLength > 1 ) it += ( uLength - 1 );                        // skip to end of state marker and if it is more than 1 character, skip to end of state
               continue;
            }

            if( *it == '\n' )                                                 // end of line ?
            {
               // ## if we are in comment or string state, then we need to check if we have found pattern in text.

               if( (uFindInState & ( eStateComment | eStateString )) && stringText.empty() == false )
               {
                  if( ( state_.is_comment() == true && ( uFindInState & eStateComment ) ) ||
                      ( state_.is_string() == true && ( uFindInState & eStateString ) ) )
                  {
                     uint64_t uColumn;
                     int iPattern = patternsFind.find_pattern(stringText, &uColumn); // try to find pattern in string
                     if( iPattern != -1 )                                      // did we find a pattern?
                     {
                        // ## figure ot row and column
                        auto uRow = uCountNewLine;                             // row number for current buffer
                        auto uPosition = it - first_;
                        uRow -= lineBuffer.count('\n', uPosition);             // subtract number of new lines in buffer from current position to get the right row

                        std::string_view stringPattern = patternsFind.get_pattern(iPattern);
                        add_line_to_table_(iPattern, stringText, uRow, uColumn, stringPattern); // add line to table
                     }
                  }
               }

               stringText.clear();                                             // clear text for analysis
            }
         }
      }

      lineBuffer.rotate();                                                    // rotate buffer

      if( uReadSize > 0 )                                                     // was it possible to read data last read, then more data is available
      {
         auto uAvailable = lineBuffer.available();                            // get available space in buffer to be filled
         file_.read((char*)lineBuffer.buffer(), lineBuffer.available());      // read more data into available space in buffer
         uReadSize = file_.gcount();
         lineBuffer.update(uReadSize);                                        // update valid size in line buffer
      }
   }

   // ## Check if rows have been added, if so then read the full line into "line" field as a preview
   if( ptable_->size() > uSaveRowCount )
   {
      // move file to the beginning
      file_.clear(); // clear EOF and fail bits
      file_.seekg(0, std::ios::beg); // move to the beginning of the file
      FILES_ReadFullRow_g( &file_, ptable_, uSaveRowCount ); // read full row into "line" field as a preview for all rows that were added to the table
   }

   return { true, "" };
}



/** ----------------------------------------------------------------------------
 * @brief Lists lines in a file that match any of the provided regular expression patterns.
 *
 * This function reads the specified file and searches for lines that match any of the given regular expressions.
 * It supports searching within code, comment, string, or all states, as specified by the "state" argument.
 * For each matching line, it records details such as the file key, filename, line content, row, column, and matched pattern in the provided table.
 *
 * @param argumentsPath Arguments container with the following keys:
 *   - "source" (string): Path to the source file to search.
 *   - "file-key" (uint64_t): Unique key identifying the file.
 *   - "state" (string, optional): State to search in ("code", "comment", "string", or "all"). Defaults to "code".
 * @param vectorRegexPatterns Vector of pairs, each containing a std::regex and its string representation, to search for in the file.
 * @param ptable_ Pointer to the table where matching line details will be stored.
 * @return std::pair<bool, std::string> Returns {true, ""} on success, or {false, error message} on failure.
 *
 * Each matching line added to the table will include:
 *   - "key": Unique row identifier.
 *   - "file-key": The file key from argumentsPath.
 *   - "filename": The name of the file being processed.
 *   - "line": The content of the matching line.
 *   - "row": The line number of the match.
 *   - "column": The column number where the match starts.
 *   - "pattern": The matched pattern as a string.
 */
std::pair<bool, std::string> COMMAND_ListLinesWithPattern(const gd::argument::shared::arguments& argumentsPath, const std::vector< std::pair<boost::regex, std::string> >& vectorRegexPatterns, gd::table::dto::table* ptable_)
{
   enum { eStateCode = 0x01, eStateComment = 0x02, eStateString = 0x04 }; // states for code, comment and string

   unsigned uFindInState = eStateCode; // state of the parser

   uint64_t uSaveRowCount = ptable_->get_row_count(); // save number of rows to fill in the full row for found rows at the end

   // ## if state is sent than try to figure out from what state to find patterns
   if( argumentsPath.exists("state") == true )
   {
      std::string stringState = argumentsPath["state"].as_string();
      if     ( stringState == "comment" ) uFindInState = eStateComment;
      else if( stringState == "string"  ) uFindInState = eStateString;
      else if( stringState == "code"    ) uFindInState = eStateCode;
      else if( stringState == "all"     ) uFindInState = ( eStateComment | eStateString | eStateCode );
   }

   uint64_t uFileKey = argumentsPath["file-key"]; // key to file for main table holding activ files

   // ## prepare source file

   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);
   if( std::filesystem::is_regular_file(stringFile) == false ) return { false, "File not found: " + stringFile };
   gd::file::path pathFile(stringFile);
   std::string stringExtension = pathFile.extension().string();

   // ### Open file

   std::ifstream file_(stringFile, std::ios::binary);
   if(file_.is_open() == false) return { false, "Failed to open file: " + stringFile };

   gd::parse::window::line lineBuffer(48 * 64, 64 * 64, gd::types::tag_create{});  // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   gd::expression::parse::state state_; // state is used to check what type of code part we are in
   auto result_ = CApplication::PrepareState_s( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                               // error in state preparation


   uint64_t uCountNewLine = 0;                                                // counts all new lines in file (all '\n' characters)

   // ## find pattern in code using regex, returns index to matched regex in regexPatterns if match, otherwise -1
   auto find_pattern_ = [&vectorRegexPatterns](const std::string& stringText, uint64_t* puColumn) -> int {
      for(size_t u = 0; u < vectorRegexPatterns.size(); ++u)
      {
         boost::smatch smatch_;
         if(boost::regex_search(stringText, smatch_, vectorRegexPatterns[u].first)) 
         {
            if(puColumn) *puColumn = smatch_.position( size_t(0) );
            return static_cast<int>(u);
         }
      }
      return -1;
   };

   // ## find pattern in code, returns index to found pattern within patternsFind if match, otherwise -1
   auto add_line_to_table_ = [uFileKey,ptable_,&stringFile,&vectorRegexPatterns](int iPatternIndex, std::string& stringText, uint64_t uLineRow, uint64_t uColumn, const std::string_view& stringPattern ) 
      {  
         stringText = gd::utf8::trim_to_string(stringText);                   // trim

         // ## adds line with information about found pattern to table holding matches
         auto uRow = ptable_->row_add_one();
         ptable_->cell_set(uRow, "key", uRow + 1);
         ptable_->cell_set(uRow, "file-key", uFileKey);
         ptable_->cell_set(uRow, "filename", stringFile);
         ptable_->cell_set(uRow, "line", stringText);
         ptable_->cell_set(uRow, "row", uLineRow);
         ptable_->cell_set(uRow, "column", uColumn);
         ptable_->cell_set(uRow, "pattern", stringPattern, gd::types::tag_adjust{});
      };


   std::string stringSourceCode; // gets source code for analysis
   std::string stringText;       // gets text for analysis
   uint64_t uRowCharacterCodeCount = 0; // number of characters of code in current row (helper variable)


   // ## Read the file into the buffer
   auto uAvailable = lineBuffer.available();
   file_.read((char*)lineBuffer.buffer(), uAvailable);
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   // ## Process the file
   while(lineBuffer.eof() == false)
   {
      uCountNewLine += lineBuffer.count('\n');                                // count all new lines in buffer

      auto [first_, last_] = lineBuffer.range(gd::types::tag_pair{});         // get first and last position of buffer

      for(auto it = first_; it < last_; it++ ) 
      {
         if( state_.in_state() == false )                                     // not in a state? that means we are reading source code
         {
            // ## check if we have found state
            if( state_[*it] != 0 && state_.exists( it ) == true )
            {
               if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) )
               {                                                                                   assert( stringSourceCode.empty() == false );
                  stringSourceCode = gd::utf8::trim_right_to_string(stringSourceCode);// trim right because we are in code state and changes state in the middle of the line
                  uint64_t uColumn;
                  int iPattern = find_pattern_(stringSourceCode, &uColumn);    // try to find pattern in source code
                  if( iPattern != -1 )                                         // did we find a pattern?
                  {
                     // ## figure ot row and column
                     auto uRow = uCountNewLine; // row number for current buffer
                     auto uPosition = it - first_;
                     uRow -= lineBuffer.count('\n', uPosition);               // subtract number of new lines in buffer from current position to get the right row

                     add_line_to_table_(iPattern, stringSourceCode, uRow, uColumn, vectorRegexPatterns[iPattern].second); // add line to table
                  }
               }
               stringSourceCode.clear();                                      // clear source code
               uRowCharacterCodeCount = 0;
               auto uSize = state_.activate(it);                              // activate state
               it += (uSize - 1);                                             // skip to end of state marker
               continue;
            }

            
            if( *it == '\n' ) 
            { 
               if( (uRowCharacterCodeCount > 0) && (uFindInState & eStateCode) )
               {                                                                                   assert( stringSourceCode.empty() == false );
                  uint64_t uColumn;
                  int iPattern = find_pattern_(stringSourceCode, &uColumn);    // try to find pattern in source code
                  if( iPattern != -1 )                                         // did we find a pattern?
                  {
                     // ## figure ot row and column
                     auto uRow = uCountNewLine; // row number for current buffer
                     auto uPosition = it - first_;
                     uRow -= lineBuffer.count('\n', uPosition);                // subtract number of new lines in buffer from current position to get the right row

                     add_line_to_table_(iPattern, stringSourceCode, uRow, uColumn, vectorRegexPatterns[iPattern].second); // add line to table
                  }
               }

               stringSourceCode.clear();
               uRowCharacterCodeCount = 0;                                    // reset code character count for next line
               continue;
            }
            else if( gd::expression::is_code_g( *it ) != 0 ) 
            { 
               uRowCharacterCodeCount++;                                      // count all code characters in line
            }

            stringSourceCode += *it;                                          // add character to source code
         }
         else
         {  // Handle state that is outside code
            stringText += *it;                                                // add character to text for analysis
            // ## check if we have found end of state
            unsigned uLength;
            if( state_.deactivate(it, &uLength, gd::expression::parse::state::tag_manual{}) == true ) // NOTE: this needs manual reset of internal state
            {
               if( uFindInState != eStateCode && stringText.empty() == false )
               {
                  if( uFindInState & ( eStateComment | eStateString ) )        // if find text in comment or string
                  {
                     if( ( state_.is_comment() == true && ( uFindInState & eStateComment ) ) ||
                         ( state_.is_string() == true && ( uFindInState & eStateString ) ) )
                     {
                        uint64_t uColumn;
                        int iPattern = find_pattern_(stringText, &uColumn);    // try to find pattern in source code
                        if( iPattern != -1 )                                           // did we find a pattern?
                        {
                           // ## figure ot row and column
                           auto uRow = uCountNewLine; // row number for current buffer
                           auto uPosition = it - first_;
                           uRow -= lineBuffer.count('\n', uPosition);         // subtract number of new lines in buffer from current position to get the right row

                           add_line_to_table_(iPattern, stringText, uRow, uColumn, vectorRegexPatterns[iPattern].second); // add line to table
                        }
                     }
                  } // if( uFindInState & ( eStateComment | eStateString ) )
               } // if( stringText.empty() == false )

               state_.clear_state();                                           // clear state
               stringText.clear();                                             // clear text for analysis


               if( uLength > 1 ) it += ( uLength - 1 );                        // skip to end of state marker and if it is more than 1 character, skip to end of state
               continue;
            }

            if( *it == '\n' )
            {
               if( (uFindInState & ( eStateComment | eStateString )) && stringText.empty() == false )
               {
                  if( ( state_.is_comment() == true && ( uFindInState & eStateComment ) ) ||
                      ( state_.is_string() == true && ( uFindInState & eStateString ) ) )
                  {
                     uint64_t uColumn;
                     int iPattern = find_pattern_(stringSourceCode, &uColumn);    // try to find pattern in source code
                     if( iPattern != -1 )                                      // did we find a pattern?
                     {
                        // ## figure ot row and column
                        auto uRow = uCountNewLine;                             // row number for current buffer
                        auto uPosition = it - first_;
                        uRow -= lineBuffer.count('\n', uPosition);             // subtract number of new lines in buffer from current position to get the right row

                        add_line_to_table_(iPattern, stringSourceCode, uRow, uColumn, vectorRegexPatterns[iPattern].second); // add line to table
                     }
                  }
               }

               stringText.clear();                                             // clear text for analysis
            }
         }
      }

      lineBuffer.rotate();                                                    // rotate buffer

      if( uReadSize > 0 )                                                     // was it possible to read data last read, then more data is available
      {
         file_.read((char*)lineBuffer.buffer(), lineBuffer.available());      // read more data into available space in buffer
         uReadSize = file_.gcount();
         lineBuffer.update(uReadSize);                                        // update valid size in line buffer
      }
   }

   // ## Check if rows have been added, if so then read the full line into "line" field as a preview
   if( ptable_->size() > uSaveRowCount )
   {
      // move file to the beginning
      file_.clear(); // clear EOF and fail bits
      file_.seekg(0, std::ios::beg); // move to the beginning of the file
      FILES_ReadFullRow_g( &file_, ptable_, uSaveRowCount ); // read full row into "line" field as a preview for all rows that were added to the table
   }


   return { true, "" };
}

/** ----------------------------------------------------------------------------
 * @brief Finds patterns in a string and stores the results in a table.
 *
 * This function searches for specified patterns in the provided string code.
 * It identifies matches, records their line numbers, and stores the results in a table.
 * The function supports regular expressions.
 *
 * @param stringCode The source code as a string to search for patterns.
 * @param vectorPatterns A vector of patterns to search for. It holds string that represents the regex pattern and regex object used to search for.
 * @param argumentsFind Arguments containing arguments how to find and data to generate result.
 * @param ptable_ Pointer to the table where matching lines will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> COMMAND_FindPattern_g(const std::string& stringCode, const std::vector<std::string>& vectorPatterns, const gd::argument::shared::arguments& argumentsFind, gd::table::dto::table* ptable_)
{
   uint64_t uFileKey = argumentsFind["file-key"]; // key to file for main table holding activ files
   std::string stringFile = argumentsFind["source"].as_string();                                   assert(stringFile.empty() == false);
   std::vector<std::pair<uint64_t, std::string>> vectorRow; // vector to hold matched rows and the pattern that matched

   try 
   {
      // ## Function to add line to table
      auto add_line_to_table_ = [uFileKey,ptable_,&stringFile]( uint64_t uLineRow, const std::string_view& stringPattern ) 
         {  
            // ## adds line with information about found pattern to table holding matches
            auto uRow = ptable_->row_add_one();
            ptable_->cell_set(uRow, "key", uRow + 1);
            ptable_->cell_set(uRow, "file-key", uFileKey);
            ptable_->cell_set(uRow, "filename", stringFile);
            ptable_->cell_set(uRow, "row", uLineRow);
            ptable_->cell_set(uRow, "pattern", stringPattern, gd::types::tag_adjust{});
         };

      // ## Function to count newlines from start to get row number
      auto count_newline_ = [&stringCode](size_t pos) -> uint64_t 
      {
         return static_cast<uint64_t>(std::count(stringCode.begin(), stringCode.begin() + pos, '\n'));
      };

      // ## Search for each regex pattern in the entire buffer
      for(const auto& stringPattern : vectorPatterns) 
      {
         // ## Search for the string pattern in `stringCode` and count all matches
         size_t uPosition = 0;
         while((uPosition = stringCode.find(stringPattern, uPosition)) != std::string::npos) 
         {
            uint64_t uRow = count_newline_(uPosition); // Determine which row this match starts in
            vectorRow.emplace_back(uRow, stringPattern); // Store the match with its row number and pattern identifier
            uPosition += stringPattern.length(); // Move past the current match
         }
      }

      // Sort results by row number for easier processing
      std::sort(vectorRow.begin(), vectorRow.end(), 
         [](const std::pair<uint64_t, std::string>& a, const std::pair<uint64_t, std::string>& b) { return a.first < b.first; });

      // ## Store results in the table for each match found
      if( vectorRow.empty() == false && ptable_ != nullptr )                  // found matches and table is provided ? 
      {
         std::string stringFile = argumentsFind["source"].as_string(); // Get the source file name

         // Open the file to read full lines later
         std::ifstream file_(stringFile, std::ios::binary);
         if( file_.is_open() == false ) { return {false, "Failed to open file: " + stringFile}; }

         for( const auto& it : vectorRow ) 
         {
            uint64_t uRow = it.first; // Get the row number
            add_line_to_table_(uRow, it.second); // Add the row to the table
         }


         file_.clear(); // clear EOF and fail bits
         file_.seekg(0, std::ios::beg); // move to the beginning of the file
         auto result_ = FILES_ReadFullRow_g( &file_, ptable_, ptable_->size() - vectorRow.size() ); // read full line for each row into "line" field as a preview for all rows that were added to the table
         if( result_.first == false ) { return result_; }                     // If reading full lines failed, return error
      }

      // Return success with info about matches found
      uint64_t totalLines = static_cast<uint64_t>(std::count(stringCode.begin(), stringCode.end(), '\n')) + 1;
      std::string result = "Found " + std::to_string(vectorRow.size()) + " matches across " + std::to_string(totalLines) + " lines";
      return {true, result};
   }
   catch (const std::exception& e) 
   {
      return {false, std::string("Error processing patterns: ") + e.what()};
   }

   return { true, "" };                                                       // Return success if no matches found
}


/** ----------------------------------------------------------------------------
 * @brief Finds patterns in a string and stores the results in a table.
 *
 * This function searches for specified regular expression patterns in the provided string code.
 * It identifies matches, records their line numbers, and stores the results in a table.
 *
 * @param stringCode The source code as a string to search for patterns.
 * @param vectorRegexPatterns A vector of pairs, each containing a boost::regex and its string representation, to search for in the code.
 * @param argumentsFind Arguments containing arguments how to find and data to generate result.
 * @param ptable_ Pointer to the table where matching lines will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> COMMAND_FindPattern_g(const std::string& stringCode, const std::vector<std::pair<boost::regex, std::string>>& vectorRegexPatterns, const gd::argument::shared::arguments& argumentsFind, gd::table::dto::table* ptable_)
{
   uint64_t uFileKey = argumentsFind["file-key"]; // key to file for main table holding activ files
   std::string stringFile = argumentsFind["source"].as_string();                                   assert(stringFile.empty() == false);
   std::vector<std::pair<uint64_t, std::string>> vectorRow; // vector to hold matched rows and the pattern that matched

   try 
   {
      // ## Function to add line to table
      auto add_line_to_table_ = [uFileKey,ptable_,&stringFile]( uint64_t uLineRow, const std::string_view& stringPattern ) 
         {  
            // ## adds line with information about found pattern to table holding matches
            auto uRow = ptable_->row_add_one();
            ptable_->cell_set(uRow, "key", uRow + 1);
            ptable_->cell_set(uRow, "file-key", uFileKey);
            ptable_->cell_set(uRow, "filename", stringFile);
            ptable_->cell_set(uRow, "row", uLineRow);
            ptable_->cell_set(uRow, "pattern", stringPattern, gd::types::tag_adjust{});
         };

      // ## Function to count newlines from start to get row number
      auto count_newline_ = [&stringCode](size_t pos) -> uint64_t 
      {
         return static_cast<uint64_t>(std::count(stringCode.begin(), stringCode.begin() + pos, '\n'));
      };

      // ## Search for each regex pattern in the entire buffer
      for(const auto& pattern : vectorRegexPatterns) 
      {
         boost::sregex_iterator itRegex(stringCode.begin(),stringCode.end(), pattern.first);
         boost::sregex_iterator itEnd;

         // ### Find all matches for this pattern
         while(itRegex != itEnd) 
         {
            const boost::smatch& match_ = *itRegex;
            size_t uPosition = static_cast<size_t>( match_.position() );       // Get the position of the match in the string
            uint64_t uRow = count_newline_(uPosition);                         // Determine which row this match starts in
            vectorRow.emplace_back(uRow, pattern.second);                      // Store the match with its row number and pattern identifier
            ++itRegex;
         }
      }

      // Sort results by row number for easier processing
      std::sort(vectorRow.begin(), vectorRow.end(), 
         [](const std::pair<uint64_t, std::string>& a, const std::pair<uint64_t, std::string>& b) { return a.first < b.first; });

      // ## Store results in the table if needed
      if( vectorRow.empty() == false && ptable_ != nullptr)                   // found matches and table is provided ? 
      {
         std::string stringFile = argumentsFind["source"].as_string(); // Get the source file name

         // Open the file to read full lines later
         std::ifstream file_(stringFile, std::ios::binary);
         if( file_.is_open() == false ) { return {false, "Failed to open file: " + stringFile}; }

         for( const auto& it : vectorRow ) 
         {
            uint64_t uRow = it.first; // Get the row number
            add_line_to_table_(uRow, it.second); // Add the row to the table
         }


         file_.clear(); // clear EOF and fail bits
         file_.seekg(0, std::ios::beg); // move to the beginning of the file
         auto result_ = FILES_ReadFullRow_g( &file_, ptable_, ptable_->size() - vectorRow.size() ); // read full line for each row into "line" field as a preview for all rows that were added to the table
         if( result_.first == false ) { return result_; }                     // If reading full lines failed, return error
      }

      // Return success with info about matches found
      uint64_t totalLines = static_cast<uint64_t>(std::count(stringCode.begin(), stringCode.end(), '\n')) + 1;
      std::string result = "Found " + std::to_string(vectorRow.size()) + " matches across " + std::to_string(totalLines) + " lines";
      return {true, result};
   }
   catch (const std::exception& e) 
   {
      return {false, std::string("Error processing patterns: ") + e.what()};
   }

   return { true, "" };                                                       // Return success if no matches found
}


std::pair<bool, std::string> COMMAND_ReadSnippet_g( const std::string& stringCode, const gd::argument::shared::arguments& argumentsCode, const gd::table::dto::table* ptableLineList, gd::table::dto::table* ptableSnippet)
{

   return RunExpression_g(stringCode, argumentsCode, ptableLineList, ptableSnippet);
   /*
   using namespace gd::expression;

   std::vector<gd::expression::token> vectorToken;
   std::pair<bool, std::string> result = gd::expression::token::parse_s(stringCode, vectorToken, gd::expression::tag_formula{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   // ## compile tokens and that means to convert tokens to postfix, place them in correct order to be processed
   std::vector<gd::expression::token> vectorPostfix;
   result = gd::expression::token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});
   if( result.first == false ) { throw std::invalid_argument(result.second); }

   gd::expression::runtime runtime_;
 
   runtime_.add( { uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str")});
   //runtime_.add( { uMethodSelectSize_g, pmethodSelect_g, std::string("source")});

   //ExpressionSource

   */
   return {true, ""};
}
// 0TAG0FileExtensions.PrepareState



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
 * if(result_.first) { .. ommitted .. }
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

/** ---------------------------------------------------------------------------
 * @brief Removes rows from a table where all specified columns have a value of zero.
 * 
 * This function iterates through each row in the table and checks the specified columns.
 * If all values in the specified columns for a row are zero, that row is marked for removal.
 * After checking all rows, the marked rows are removed from the table.
 * 
 * @param ptable_ Pointer to the table from which rows will be removed.
 * @param vectorColumnIndex A vector of column indices to check for zero values.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> TABLE_RemoveZeroRow(gd::table::dto::table* ptable_, const std::vector<unsigned>& vectorColumnIndex)
{
   std::vector<uint64_t> vectorRemoveRow; // vector to hold rows to be removed

   auto uRowCount = ptable_->get_row_count(); 
   for( unsigned uRow = 0; uRow < uRowCount; ++uRow )
   {
      bool bZero = true;
      for( unsigned uColumnIndex : vectorColumnIndex )
      {                                                                                               assert( uColumnIndex < ptable_->get_column_count() );
         auto uValue = ptable_->cell_get<uint64_t>(uRow, uColumnIndex);
         if( uValue != 0 ) { bZero = false; break; }
      }
      if( bZero == true ) vectorRemoveRow.push_back(uRow);                    // if all values in row are zero, add row to remove vector
   }

   if( vectorRemoveRow.empty() == false )
   {
      ptable_->erase(vectorRemoveRow); // remove rows from table
   }

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Filters rows in a table based on expressions evaluated against a specified column.
 * 
 * This function evaluates each expression against the value in the specified column of each row.
 * If the expression evaluates to false, the row is removed from the table.
 * 
 * @param ptable_ Pointer to the table to filter.
 * @param uColumn The index of the column to evaluate.
 * @param vectorExpression A vector of strings representing expressions to evaluate.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> EXPRESSION_FilterOnColumn_g( gd::table::dto::table* ptable_, unsigned uColumn, const std::vector<std::string> vectorExpression )
{                                                                                                  assert(ptable_ != nullptr); assert(uColumn < ptable_->get_column_count());
   using namespace gd::expression;

   // ## Prepare runtime for expressions, adds methods for default and string
   gd::expression::runtime runtime_;
   runtime_.set_debug();                                                      // enable debug mode for runtime, needed becuase user are able to set methods
   runtime_.add( { (unsigned)uMethodDefaultSize_g, gd::expression::pmethodDefault_g, ""});
   runtime_.add( { (unsigned)uMethodStringSize_g, gd::expression::pmethodString_g, std::string("str") }); // string methods, set namespace "str" for string methods


   std::vector<uint64_t> vectorRemoveRow; // vector to hold rows to be removed

   std::vector< std::vector<gd::expression::token> > vectorExpressionToken;   // vector to hold expression tokens for each expression

   for( const auto& stringExpression : vectorExpression )                     // Convert each expression string to tokens and put them into vectorExpressionToken
   {
      // ## generate expression tokens from string expression

      std::vector<token> vectorToken;
      auto result_ = token::parse_s(stringExpression, vectorToken, gd::expression::tag_formula{});

      // ## convert to postfix notation

      std::vector<token> vectorPostfix;
      result_ = token::compile_s(vectorToken, vectorPostfix, gd::expression::tag_postfix{});  if( result_.first == false ) { return result_; }

      vectorExpressionToken.emplace_back(std::move(vectorPostfix));           // store parsed tokens
   }

   auto uRowCount = ptable_->get_row_count(); 
   for( unsigned uRow = 0; uRow < uRowCount; ++uRow )
   {
      auto stringValue = ptable_->cell_get_variant_view(uRow, uColumn).as_string();
      runtime_.set_variable( "line", stringValue );

      for( auto & vectorPostfix : vectorExpressionToken )
      {
         // ## calculate expression for current row

         gd::expression::value valueResult;
         auto result_ = token::calculate_s(vectorPostfix, &valueResult, runtime_);
         if( result_.first == false ) { return result_; }                     // error in calculation ?

         if( valueResult.is_bool() == true )                                  // if expression is false, then remove row
         {
            bool bResult = valueResult.as_bool();                             // get boolean result
            if( bResult == false ) { vectorRemoveRow.push_back(uRow); break; } // if expression is false, then remove row and break out of loop
         }
         else
         {
            // ## Generate error information
            //    Harvest expressions to return information about them
            std::string stringError;
            for( const auto& it : vectorExpression )
            {
               stringError += std::format("   {}\n", it);
            }

            return { false, std::format( "Expression is not returning boolean value:\n   {}", stringError) };
         }
      }
   }


   if( vectorRemoveRow.empty() == false )
   {
      ptable_->erase(vectorRemoveRow); // remove rows from table
   }

   return { true, "" };
}


/** --------------------------------------------------------------------------- @TAG #clipboard #os
 * @brief Reads the clipboard content and returns it as a UTF-8 encoded string.
 * 
 * This function attempts to read the clipboard content, first trying to retrieve
 * Unicode text. If that fails, it falls back to ANSI text. The result is returned
 * as a UTF-8 encoded string.
 * 
 * @param stringClipboard Reference to a string where the clipboard content will be stored.
 * @return A pair containing:
 *         - `bool`: `true` if reading was successful, `false` otherwise.
 *         - `std::string`: An error message if reading failed, or an empty string on success.
 */
std::pair<bool, std::string> OS_ReadClipboard_g(std::string& stringClipboard)
{
   std::string stringResult;

#ifdef _WIN32
   if( ::OpenClipboard(nullptr) == FALSE ) { return { false, "Failed to open clipboard" }; }

   // Get handle of clipboard object for Unicode text
   HANDLE hClipboard = GetClipboardData(CF_UNICODETEXT);
   if( hClipboard != nullptr) 
   {
      wchar_t* piText = static_cast<wchar_t*>(GlobalLock(hClipboard));         // Lock the handle to get the actual text pointer
      if(piText != nullptr) 
      {
         int iBufferSize = WideCharToMultiByte(CP_UTF8, 0, piText, -1, nullptr, 0, nullptr, nullptr);// Convert wide string to narrow string (UTF-8)
         if(iBufferSize > 0) 
         {
            char* piBuffer = new char[iBufferSize];
            WideCharToMultiByte(CP_UTF8, 0, piText, -1, piBuffer, iBufferSize, nullptr, nullptr);
            stringResult = piBuffer;
            delete[] piBuffer;
         }
         
         ::GlobalUnlock(hClipboard);                                          // Release the lock
      }
   }
   else 
   {
      // Fallback to ANSI text if Unicode is not available
      hClipboard = ::GetClipboardData(CF_TEXT); // use hClipboard as previously declared
      if (hClipboard != nullptr) {
         char* pszText = static_cast<char*>(GlobalLock(hClipboard));
         if (pszText != nullptr) {
            stringResult = pszText;
            GlobalUnlock(hClipboard);
         }
      }
   }

   ::CloseClipboard();                                                        // Release the clipboard
#else
   std::string stringOS = papplication_g->PROPERTY_Get("os").as_string();     // Get OS type
   if( stringOS == "linux")
   {
      FILE* pfile = popen("xclip -selection clipboard -o", "r");
      if( pfile == nullptr ) { return { false, "Failed to open clipboard" }; }
      
      char piBuffer[1024];
      while( fgets(piBuffer, sizeof(piBuffer), pfile) ) 
      {
         stringResult += piBuffer;
      }
      pclose(pfile);                                                              // Close the pipe to the clipboard command
   }
   else if( stringOS == "wsl" )
   {
      FILE* pfile = popen("pwsh.exe -NoProfile -Command \"Get-Clipboard -Raw\"", "r");
      if( pfile == nullptr ) { return { false, "Failed to open clipboard" }; }

      char piBuffer[1024];
      while( fgets(piBuffer, sizeof(piBuffer), pfile) )
      {
         stringResult += piBuffer;
      }
      pclose(pfile);                                                              // Close the pipe to the clipboard command
   }
   else
   {
      return { false, "Unsupported OS for clipboard reading" };
   }
#endif 
   stringResult = gd::utf8::trim_to_string(stringResult); // Trim the result to remove any leading/trailing whitespace
   stringClipboard = std::move(stringResult);                                  // Move the result to the output string
   return { true, "" };
}
