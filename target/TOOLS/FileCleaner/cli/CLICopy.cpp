/**
 * @file CLICopy.cpp
 */

#include <chrono>
#include <format>

#include <boost/regex.hpp>

#include "gd/gd_uuid.h"

#include "../Command.h"
#include "../Application.h"

#include "CLI_Shared.h"

#include "CLICopy.h"


// @TAG #cli #copy


NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> PrepareCopy_s( const std::vector<std::string>& vectorSourceFile, const std::string& stringTargetFolder, const gd::argument::shared::arguments& arguments_, CDocument* pdocument);
std::pair<bool, std::string> DoCopy_s( gd::table::dto::table* ptableCopy, const gd::argument::shared::arguments& arguments_, CDocument* pdocument );

// ## Copy operations

std::pair<bool, std::string> Copy_g(const gd::cli::options* poptionsCopy, CDocument* pdocument)
{                                                                                                  assert(poptionsCopy != nullptr );
   const gd::cli::options& options_ = *poptionsCopy;

   gd::argument::shared::arguments argumentsFileHarvest;
   SHARED_ReadHarvestSetting_g( options_, argumentsFileHarvest, pdocument );


   if (options_.exists("target") == true)
   {
      argumentsFileHarvest.append(options_.get_arguments(), { "filter", "overwrite", "pattern", "rpattern", "segment", "newer", "where", "preview"});

      auto result_ = CopyFiles_g(options_["target"].as_string(), argumentsFileHarvest, pdocument);
      if( result_.first == false ) return result_;
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Copies a single file from source to target location.
 *
 *   - Normalizes the target path based on special cases (., .., directories).
 *   - Ensures the target directory exists, creating it if necessary.
 *   - Copies the file from source to target, overwriting if it already exists.
 *
 * @param stringSource The source file path.
 * @param stringTarget The target file path or directory.
 * @param pdocument    Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> CopySingleFile_s(std::string stringSource, std::string stringTarget, CDocument* pdocument)
{
   assert(pdocument != nullptr);
   std::filesystem::path pathSourceFile(stringSource);

   // ## Normalize target path for traling slash
   while( stringTarget.empty() == false && ( stringTarget.back() == '\\' || stringTarget.back() == '/' ) ) { stringTarget.pop_back(); }
   stringTarget = std::filesystem::path(stringTarget).string(); // Normalize target folder
   std::filesystem::path pathTargetFile(stringTarget);

   // Normalize target path based on special cases
   if( stringTarget == "." || stringTarget == "./" || stringTarget == "" )
   {
      pathTargetFile = std::filesystem::current_path() / pathSourceFile.filename();
   }
   else if( stringTarget == ".." )
   {
      pathTargetFile = std::filesystem::current_path().parent_path() / pathSourceFile.filename();
   }
   else if( stringTarget == "../" )
   {
      pathTargetFile = std::filesystem::current_path().parent_path() / pathSourceFile.filename();
   }
   else if( std::filesystem::is_directory(pathTargetFile) == true )
   {
      pathTargetFile = pathTargetFile / pathSourceFile.filename();
   }
   else if( pathTargetFile.has_extension() == false )
   {
      // Assume it's a directory path and append filename
      pathTargetFile = pathTargetFile / pathSourceFile.filename();
   }
   // If pathTargetFile has extension, treat it as complete file path

   // Ensure target directory exists
   std::error_code errorcode;
   std::filesystem::create_directories(pathTargetFile.parent_path(), errorcode);
   if( errorcode ) { return { false, "Failed to create target directory: " + pathTargetFile.parent_path().string() + " Error: " + errorcode.message() }; }

   // Copy the file
   std::filesystem::copy_file(pathSourceFile, pathTargetFile, std::filesystem::copy_options::overwrite_existing, errorcode);
   if( errorcode ) { return { false, "Failed to copy file: " + pathSourceFile.string() + " to " + pathTargetFile.string() + " Error: " + errorcode.message() }; }

   pdocument->MESSAGE_Display("Copied file: " + pathSourceFile.string() + " to " + pathTargetFile.string());

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief
 *   Copies files from source to target folder while preserving directory structure.
 *
 *   - Harvests files from the given source path using the provided filter and search depth.
 *   - Validates source and target paths to ensure they are different directories.
 *   - For each file found, verifies its existence and adds it to the copy list.
 *   - Copies files to target folder, creating subdirectories as needed while removing the source folder prefix.
 *
 * @param stringTargetFolder The target directory path to copy files to.
 * @param arguments_        Options for the copy operation.
 * @param pdocument         Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> CopyFiles_g( const std::string& stringTargetFolder, const gd::argument::shared::arguments& arguments_, CDocument* pdocument)
{                                                                                               assert( stringTargetFolder != "" ); assert( pdocument != nullptr );
   constexpr std::string_view stringTableId = "file-dir";
   auto ptableDir = pdocument->CACHE_Get(stringTableId, true );
   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_[{"depth", "recursive"}].as_uint();
   std::string stringSource = arguments_["source"].as_string();
   auto result_ = FILES_Harvest_WithWildcard_g( stringSource, stringFilter, ptableDir, uDepth, true); 
   if( result_.first == false ) return result_;

   std::string stringTargetFolder_ = stringTargetFolder;

	pdocument->MESSAGE_Display(std::format("Files found from source/sources '{}'", stringSource));

   // ### Special case, if source is a file then just copy that file
   // Special case: if source is a file, copy that single file
   if( std::filesystem::is_regular_file(stringSource) == true )
   {
      return CopySingleFile_s(stringSource, stringTargetFolder, pdocument);
   }

	// ## determine the source folder to remove from path when copying ...........
   
	std::string stringSourceFolder; // the source folder to remove from path when copying
   if(std::filesystem::is_directory(stringSource) == true) { stringSourceFolder = stringSource; }
   else
   {
      stringSourceFolder = std::filesystem::path(stringSource).parent_path().string();// get parent directory if source is a file
   }
   
	// ## validate stringSource and stringTarget .................................

   if( std::filesystem::exists( stringSourceFolder) == false ) { return { false, "Source folder does not exist: " + stringSourceFolder }; }


	if( std::filesystem::is_directory(stringTargetFolder) == false && std::filesystem::exists(stringTargetFolder) == false) // Check target folder and if not found then create it
	{
		pdocument->MESSAGE_Display("Target folder does not exist, creating: " + stringTargetFolder);
		std::error_code errorcode;
		std::filesystem::create_directories(stringTargetFolder, errorcode);
		if(errorcode) return { false, "Failed to create target directory: " + errorcode.message() };
	}
   else if( std::filesystem::is_regular_file(stringTargetFolder) == true )
   {
      stringTargetFolder_ = std::filesystem::path(stringTargetFolder).parent_path().string(); // if target is a file then get the parent directory
   }

   // ## if where and sort then filter and sort result .......................

   if(arguments_.exists("where") == true)
   {
      auto result_ = pdocument->CACHE_Where(stringTableId, arguments_["where"].as_string_view());   // filter table based on where condition
		if(result_.first == false) return result_;
   }

	// ## Prepare paths .......................................................

   std::filesystem::path pathSource = std::filesystem::canonical(stringSourceFolder);
   std::filesystem::path pathTarget = std::filesystem::canonical(stringTargetFolder_);
   if( pathSource == pathTarget )                                              // Check if source and target are the same folder
   {
      return { false, "Source and target folders cannot be the same" };
   }	                                                                                              LOG_DEBUG_RAW("Source folder: " & pathSource.string().c_str()); LOG_DEBUG_RAW("Target folder: " & pathTarget.string().c_str());
   
	// ## Generate list of files to copy to target folder ......................

   std::vector<std::string> vectorSourceFile;
   gd::table::dto::table* ptableFile = ptableDir;                             // get the table pointer
   for(const auto& itRowFile : *ptableFile)
   {
      std::string stringFile = itRowFile.cell_get_variant_view("path").as_string(); // get the file path
      
      if( std::filesystem::exists(stringFile) == false )                      // check if file exists
      {
         pdocument->ERROR_Add("Missing file: " + stringFile); continue;
      }
      
      vectorSourceFile.push_back(stringFile);                                 // add file to vector
   }
   
   if( vectorSourceFile.empty() == true ) { return { false, "No files found to copy" }; } // no files to copy
                                                                                                    LOG_DEBUG_RAW("Files to copy: " & vectorSourceFile.size());

   bool bPreview = false; // not used yet
   bPreview = arguments_["preview"].as_bool();                                // get preview option

   // ## copy files to target folder, if sub directories then create same file structure in target but remove the source folder

   result_ = PrepareCopy_s(vectorSourceFile, stringTargetFolder_, arguments_, pdocument);
   if( result_.first == false ) return result_;

   auto ptableCopy = pdocument->CACHE_Get("#copy");

   if( bPreview == true )
   {
      std::string stringCliPreview = gd::table::to_string(*ptableCopy, gd::table::tag_io_cli{});
      pdocument->MESSAGE_Display( stringCliPreview );
      pdocument->MESSAGE_Display("Preview mode enabled, no files were copied.");
   }
   else
   {
      auto result_ = DoCopy_s( ptableCopy, arguments_, pdocument );
      if(result_.first == false ) return result_;
   }

   pdocument->CACHE_Erase("#copy"); 

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Filters files in a document based on provided patterns, updating internal tables and returning the operation status.
 * @param arguments_ A collection of arguments containing pattern options and other parameters.
 * @param pdocument Pointer to the document object to be filtered and updated.
 * @return A pair where the first element is a boolean indicating success or failure, and the second element is a string containing an error message or an empty string if successful.
 */
std::pair<bool, std::string> FILE_PatternFilter_s(const gd::argument::shared::arguments& arguments_, CDocument* pdocument)
{
	if(arguments_.exists("pattern") == true) 
   { 
      auto vectorPattern = arguments_.get_all<std::string>("pattern"); // get all patterns from options and put them into vectorPattern

      // remove empty patterns
      vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
      if( vectorPattern.size() == 0 ) return {false, "No patterns provided."}; // if no patterns are provided, return an error

      auto result_ = pdocument->FILE_UpdatePatternList(vectorPattern, arguments_); // Search for patterns in harvested files and place them into the result table
      if( result_.first == false ) return result_;
	}
   else if (arguments_.exists("rpattern") == true)
   {
      auto vectorPattern = arguments_.get_all<std::string>("rpattern"); // get all patterns from options and put them into vectorPattern
      // remove empty patterns
      vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
      if (vectorPattern.size() == 0) return { false, "No patterns provided." }; // if no patterns are provided, return an error

      std::vector<std::pair<boost::regex, std::string>> vectorRegexPattern;

      // ## convert string to regex and put it into vectorRegexPatterns

      for (auto& stringPattern : vectorPattern)
      {
         try
         {
            boost::regex regexPattern(stringPattern);
            vectorRegexPattern.push_back({ regexPattern, stringPattern });
         }
         catch (const boost::regex_error& e)
         {
            std::string stringError = "Invalid regex pattern: '" + stringPattern + "'. Error: " + e.what();
            return { false, stringError };
         }
      }

      auto result_ = pdocument->FILE_UpdatePatternList(vectorRegexPattern, arguments_); // Search for patterns in harvested files and place them into the result table
      if (result_.first == false) return result_;
   }
   else { return { false, "No pattern or rpattern option provided." }; }

   auto ptableLineList = pdocument->CACHE_Get("file-linelist", false);
	pdocument->MESSAGE_Display(ptableLineList, CDocument::tag_state{}); // display the pattern result table
   gd::table::aggregate aggregate_(ptableLineList);
   auto vectorFileKey = aggregate_.unique("file-key");                        // get all unique file keys from the harvested files table

   auto ptableDir = pdocument->CACHE_Get("file-dir", false);                                       assert(ptableDir != nullptr);

   auto vectorRow = ptableDir->find_all("key", vectorFileKey);                // mark all rows as in use that are in the pattern result table
   std::sort(vectorRow.begin(), vectorRow.end());                             // sort the found rows

   // create vector with rows inverted from found rows placed into vectorRow
   std::vector<uint64_t> vectorRowInverted;
   size_t uRowPosition = 0;

   for( auto itRow : vectorRow ) 
   { 
      for( auto u = uRowPosition; u < itRow; u++ ) 
      { 
         vectorRowInverted.push_back(u);  // add to inverted set
      }
      uRowPosition = itRow + 1;
   }

   if( uRowPosition < ptableDir->size() ) 
   {
      for( auto u = uRowPosition; u < ptableDir->size(); u++ ) { vectorRowInverted.push_back(u); } // add to inverted set
   }

   ptableDir->erase(vectorRowInverted);                                         // erase all rows that are not in the pattern result table

   pdocument->MESSAGE_Display(ptableDir, CDocument::tag_state{}); // display the pattern result table

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Prepares the copy operation by parsing arguments and setting up the copy table.
 *
 *   - Parses the "newer" argument to determine time thresholds for file copying.
 *   - Sets up a table to track files to be copied, including source, target, and flags for path creation and copying.
 *
 * @param vectorSourceFile List of source files to be copied.
 * @param stringTargetFolder The target directory where files will be copied.
 * @param arguments_ Options for the copy operation.
 * @param arguments_.overwrite Boolean flag indicating whether to overwrite existing files.
 * @param arguments_.newer Time threshold for copying files based on last write time.
 * @param arguments_.source Source directory for the files to be copied.
 * @param pdocument Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> PrepareCopy_s( const std::vector<std::string>& vectorSourceFile, const std::string& stringTargetFolder, const gd::argument::shared::arguments& arguments_, CDocument* pdocument)
{                                                                                                  assert( pdocument != nullptr ); assert( stringTargetFolder != "" );
   // ## Prepare settings for copy operation .................................

   // Parse and calculate the time threshold for "newer" comparison
   std::chrono::system_clock::time_point timeThreshold;
   int iNewerFilter = 0;  // 0 = no filter, positive = newer than, negative = older than

   bool bOverwrite = arguments_["overwrite"].as_bool();                       // get overwrite option
   std::string stringNewer = arguments_["newer"].as_string();                 // get newer option, format is hh:mm:ss and with priority for hours, then minutes and last seconds if something is missing
   std::string stringSource = arguments_["source"].as_string();

   gd::table::dto::table tableCopy(0, { { "rstring", "source"}, { "rstring", "target"}, { "bool", "create-path"}, { "bool", "copy"} }, gd::table::tag_prepare{}); // create table for copy operation


   if(stringNewer.empty() == false)                                          // if newer is set then parse it
   {
      std::string stringTimeOnly = stringNewer;

      // Check for negative prefix and set filter direction
      if (stringTimeOnly[0] == '-')
      {
         iNewerFilter = -1;                                                   // negative = older files filter
         stringTimeOnly = stringTimeOnly.substr(1);                           // remove the negative sign for parsing
      }
      else
      {
         iNewerFilter = 1;                                                    // positive = newer files filter
      }

      auto vectorTime = CApplication::Split_s(stringTimeOnly, ':');
      if (vectorTime.size() > 3) return { false, "Invalid format for newer option, use [-]hh:mm:ss" };

      unsigned uHours = 0;
      unsigned uMinutes = 0;
      unsigned uSeconds = 0;

      try
      {
         if (vectorTime.size() >= 1) uHours = std::stoi(vectorTime[0]);
         if (vectorTime.size() >= 2) uMinutes = std::stoi(vectorTime[1]);
         if (vectorTime.size() == 3) uSeconds = std::stoi(vectorTime[2]);
      }
      catch (const std::exception& e) { return { false, "Invalid time values in newer option: " + std::string(e.what()) }; }

      if (uHours > 0x10000 || uMinutes > 59 || uSeconds > 59) return { false, "Invalid time value for newer option, use [-]hh:mm:ss" };

      unsigned uTotalSeconds = uHours * 3600 + uMinutes * 60 + uSeconds;      // Calculate total seconds for the time difference   

      auto now = std::chrono::system_clock::now();
      if (iNewerFilter < 0)
      {
         // For negative values, we set threshold to current time + duration (files can be older but within limit)
         timeThreshold = now - std::chrono::seconds(uTotalSeconds);
         pdocument->MESSAGE_Display(std::format( "Using older filter: files can be older but not more than {:02}:{:02}:{:02} old", uHours, uMinutes, uSeconds ) );
      }
      else
      {
         // For positive values, we set threshold to current time - duration (files must be newer than X time ago)
         timeThreshold = now - std::chrono::seconds(uTotalSeconds);
         pdocument->MESSAGE_Display( std::format( "Using newer filter: files must be newer than {:02}:{:02}:{:02} ago", uHours, uMinutes, uSeconds ) );
      }

      bOverwrite = true;                                                      // if newer is set then we must overwrite
   }

   std::string stringSourceFolder; // the source folder to remove from path when copying
   if(std::filesystem::is_directory(stringSource) == true) { stringSourceFolder = stringSource; }
   else { stringSourceFolder = std::filesystem::path(stringSource).parent_path().string(); } // get parent directory if source is a file

   for( const auto& stringSourceFile : vectorSourceFile )
   {
      bool bCopy = true;
      bool bCreatePath = false;
		std::filesystem::path pathSourceFile(stringSourceFile); // get the source file path
		std::filesystem::path pathRelative = std::filesystem::relative(pathSourceFile, stringSourceFolder); // get the relative path to source folder
		std::filesystem::path pathTargetFile = std::filesystem::path(stringTargetFolder) / pathRelative; // create the target file path
      
      // create target directory if it doesn't exist
      std::filesystem::path pathTargetDir = pathTargetFile.parent_path();
      if( std::filesystem::exists(pathTargetDir) == false ) bCreatePath = true;

		bool bTargetExists = std::filesystem::exists(pathTargetFile);           // check if target file exists
      if(bOverwrite == false && bTargetExists == true) { bCopy = false; }     // if not overwrite and file exists then skip

      if( bCopy == true && iNewerFilter != 0 && bTargetExists == true)
      {
         // ### Apply "newer" or "older" filter logic .......................
         try
         {
            // Get source file last write time
            auto source_last_write_ = std::filesystem::last_write_time(pathSourceFile);
            // Convert to system_clock time_point
            auto source_time_ = std::chrono::system_clock::time_point( std::chrono::duration_cast<std::chrono::system_clock::duration>( source_last_write_.time_since_epoch() ) );            

            auto target_last_write_ = std::filesystem::last_write_time(pathTargetFile);
            auto target_time_ = std::chrono::system_clock::time_point( std::chrono::duration_cast<std::chrono::system_clock::duration>( target_last_write_.time_since_epoch() ) );

            if(iNewerFilter < 0) // Older files filter
            {
               // Copy files that are at least X time old (but still older than target)
               // Skip if source is newer than target OR source is not old enough
               if(source_time_ >= target_time_ || source_time_ > timeThreshold) { bCopy = false; }
            }
            else // Newer files filter
            {
               // Copy files modified within the last X time AND newer than target
               // Skip if source is not recent enough OR not newer than target
               if(source_time_ < timeThreshold || source_time_ <= target_time_) { bCopy = false; }
            }

			} // try
         catch (const std::exception& e) { pdocument->ERROR_Add( std::format( "Failed to check file times for: {} - {}", stringSourceFile, e.what() ));  continue; }
      }


      // if file is to be copied then add to table
      auto uRow = tableCopy.row_add_one();
      tableCopy.cell_set(uRow, "source", stringSourceFile);
      tableCopy.cell_set(uRow, "target", pathTargetFile.string());
      tableCopy.cell_set(uRow, "create-path", bCreatePath);
      tableCopy.cell_set(uRow, "copy", bCopy);
   }

   pdocument->CACHE_Add(std::move(tableCopy), "#copy");

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Executes the file copy operation based on the provided copy table.
 *
 *   - Iterates through each row in the copy table.
 *   - Checks if the file is marked for copying.
 *   - Creates target directories if specified.
 *   - Copies files from source to target, handling errors and logging results.
 *
 * @param ptableCopy Pointer to the copy table containing source, target, and flags.
 * @param arguments_ Options for the copy operation.
 * @param pdocument Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> DoCopy_s( gd::table::dto::table* ptableCopy, const gd::argument::shared::arguments& arguments_, CDocument* pdocument )
{                                                                                                  assert( ptableCopy != nullptr ); assert( pdocument != nullptr );
   unsigned uFilesCopied = 0;
   unsigned uFilesSkipped = 0;
   for( const auto& itRow : *ptableCopy )
   {
      bool bCopy = itRow.cell_get_variant_view("copy").as_bool();
      if( bCopy == false ) { uFilesSkipped++; continue; }

      std::string stringSourceFile = itRow.cell_get_variant_view("source").as_string();
      std::string stringTargetFile = itRow.cell_get_variant_view("target").as_string();
      bool bCreatePath = itRow.cell_get_variant_view("create-path").as_bool();
      // create target directory if it doesn't exist
      if( bCreatePath == true )
      {
         std::filesystem::path pathTargetDir = std::filesystem::path(stringTargetFile).parent_path();
         if( std::filesystem::exists(pathTargetDir) == false )
         {
            std::error_code errorcode;
            std::filesystem::create_directories(pathTargetDir, errorcode);
            if( errorcode ) { pdocument->ERROR_Add("Failed to create directory: " + pathTargetDir.string() + " - " + errorcode.message()); continue; }
         }
      }
      
      // copy the file
      std::error_code errorcode;
      std::filesystem::copy_file(stringSourceFile, stringTargetFile, std::filesystem::copy_options::overwrite_existing, errorcode);
      if(errorcode)
      {
         pdocument->ERROR_Add("Failed to copy file: " + stringSourceFile + " to " + stringTargetFile + " - " + errorcode.message());
      }
      else { uFilesCopied++; }
   }
   
   // Display summary
   pdocument->MESSAGE_Display("Copy operation completed");
   pdocument->MESSAGE_Display( std::format( "Files copied: {}", uFilesCopied ) );
   if(uFilesSkipped > 0) pdocument->MESSAGE_Display( std::format( "  Files skipped: {}", uFilesSkipped ) );

   return { true, "" };
}



NAMESPACE_CLI_END