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


 /** --------------------------------------------------------------------------
 * @brief Harvests files from a specified directory path and populates a table with their details.
 *
 * This method recursively traverses the directory structure starting from the given path, 
 * collecting information about each file and storing it in the provided table. The information 
 * includes the file's folder, filename, extension, and size.
 *
 * @param stringPath The root directory path to start harvesting files from.
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
      if( std::filesystem::is_directory(stringPath) == false )                 // not a directory
      {
         if( std::filesystem::is_regular_file(stringPath) == true )            // is file
         {
            add_(gd::file::path(stringPath));
            return { true, "" };
         }
         else
         {
            return { false, "Path is not a directory or file: " + stringPath };
         }
      }

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
   auto result_ = COMMAND_PrepareState( {{"source",stringFile}}, state_);
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

   // ## Prepare source file
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);

   // ### Open file
   if( std::filesystem::is_regular_file(stringFile) == false ) return { false, "File not found: " + stringFile };
   std::ifstream file_(stringFile, std::ios::binary);
   if(file_.is_open() == false) return { false, "Failed to open file: " + stringFile };

   gd::parse::window::line lineBuffer(48 * 64, 64 * 64, gd::types::tag_create{});  // create line buffer 64 * 64 = 4096 bytes = 64 cache lines

   gd::expression::parse::state state_; // state is used to check what type of code part we are in
   auto result_ = COMMAND_PrepareState( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                                // error in state preparation

   // ## count occurrences of each pattern in the source code
   auto count_ = [&vectorPattern, &vectorCount](const std::string& stringText) // count method that counts occurrences of each pattern in the source code
      {
         // ## Count occurrences of each pattern in text
         for(size_t u = 0; u < vectorPattern.size(); ++u)
         {
            size_t uPosition = 0;
            while((uPosition = stringText.find(vectorPattern[u], uPosition)) != std::string::npos)
            {
               vectorCount[u]++;
               uPosition += vectorPattern[u].length();                         // Move past the current match
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
   while (lineBuffer.eof() == false)
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
            if( state_.deactivate( it, &uLength ) == true ) 
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
               stringText.clear();                                             // clear text for analysis
               
               
               if( uLength > 1 ) it += ( uLength - 1 );                       // skip to end of state marker and if it is more than 1 character, skip to end of state
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

   unsigned uFindInState = eStateCode; // state of the parser

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
   auto result_ = COMMAND_PrepareState( {{"source",stringFile}}, state_);
   if( result_.first == false ) return result_;                               // error in state preparation

   uint64_t uCountNewLine = 0;                                                // counts all new lines in file (all '\n' characters)

   // ## find pattern in code, returns index to found pattern within patternsFind if match, otherwise -1
   auto add_line_to_table_ = [uFileKey,ptable_,&stringFile,&patternsFind](int iPatternIndex, const std::string& stringText, uint64_t uLineRow, uint64_t uColumn = 0u) 
      {
         auto uRow = ptable_->get_row_count();
         ptable_->row_add();
         ptable_->cell_set(uRow, "key", uRow + 1);
         ptable_->cell_set(uRow, "file-key", uFileKey);
         ptable_->cell_set(uRow, "filename", stringFile);
         ptable_->cell_set(uRow, "line", stringText);
         ptable_->cell_set(uRow, "row", uLineRow);
         ptable_->cell_set(uRow, "column", uColumn);

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
   while (lineBuffer.eof() == false)
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
               {
                  uint64_t uColumn;
                  int iPattern = patternsFind.find_pattern(stringSourceCode, &uColumn); // try to find pattern in source code
                  if( iPattern != -1 )                                        // did we find a pattern?
                  {
                     // ## figure ot row and column
                     auto uRow = uCountNewLine; // row number for current buffer
                     auto uPosition = it - first_;
                     uRow -= lineBuffer.count('\n', uPosition);               // subtract number of new lines in buffer from current position to get the right row

                     stringSourceCode = gd::utf8::trim_to_string(stringSourceCode); // trim source code
                     add_line_to_table_(iPattern, stringSourceCode, uCountNewLine, uColumn); // add line to table
                  }
               }
               stringSourceCode.clear();                                      // clear source code
               state_.activate(it);                                           // activate state

               // If multiline and `uRowCharacterCodeCount` is not 0 that means that there are characters in the code section before multiline
               if( uRowCharacterCodeCount > 0 && state_.is_multiline() == false ) 
               { 
                  stringSourceCode.clear();
                  uRowCharacterCodeCount = 0;
               }

               continue;
            }

            
            if( *it == '\n' ) 
            { 
               uint64_t uColumn;
               int iPattern = patternsFind.find_pattern(stringSourceCode, &uColumn); // try to find pattern in source code
               if( iPattern != -1 )                                           // did we find a pattern?
               {
                  // ## figure ot row and column
                  auto uRow = uCountNewLine; // row number for current buffer
                  auto uPosition = it - first_;
                  uRow -= lineBuffer.count('\n', uPosition);                  // subtract number of new lines in buffer from current position to get the right row

                  stringSourceCode = gd::utf8::trim_to_string(stringSourceCode); // trim source code
                  add_line_to_table_(iPattern, stringSourceCode, uRow, uColumn); // add line to table
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
         {
            stringText += *it;                                                // add character to text for analysis
            // ## check if we have found end of state
            unsigned uLength;
            if( state_.deactivate( it, &uLength ) == true ) 
            {
               if( uFindInState & (eStateComment|eStateString) )
               {
                  if( state_.is_comment() == true && (uFindInState & eStateComment) )
                  {
                     //count_(stringText);
                  }
                  else if( state_.is_string() == true && ( uFindInState & eStateString ) )
                  {
                     //count_(stringText);
                  }
               }
               stringText.clear();                                             // clear text for analysis


               if( uLength > 1 ) it += ( uLength - 1 );                       // skip to end of state marker and if it is more than 1 character, skip to end of state
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



   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prepares the state for parsing based on the file extension.
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @param state_ The state object to be prepared.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> COMMAND_PrepareState(const gd::argument::shared::arguments& argumentsPath, gd::expression::parse::state& state_)
{
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);
   
   gd::file::path pathFile(stringFile);
   std::string stringExtension = pathFile.extension().string();

   // convert string to lowercase
   std::transform(stringExtension.begin(), stringExtension.end(), stringExtension.begin(), ::tolower);

   if( stringExtension == ".cpp" || stringExtension == ".c" || stringExtension == ".h" || stringExtension == ".hpp" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "R\"(", ")\"");
   }
   else if( stringExtension == ".cs" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "\"\"\"", "\"\"\"");
   }
   else if( stringExtension == ".java" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   }
   else if( stringExtension == ".js" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("COMMENTBLOCK"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "`", "`");
   }
   else if( stringExtension == ".json" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("COMMENTBLOCK"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".rs" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   }
   else if( stringExtension == ".html" || stringExtension == ".xml" )
   {
      state_.add(std::string_view("BLOCKCOMMENT"), "<!--", "-->");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".css" )
   {
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".py" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".sql" )
   {
      state_.add(std::string_view("LINECOMMENT"), "--", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".json" )
   {
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".txt" || stringExtension == ".md" )
   {
      // No special states for text files
   }
   else
   {
      return { false, "Unknown file type: " + stringFile };
   }
   


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
