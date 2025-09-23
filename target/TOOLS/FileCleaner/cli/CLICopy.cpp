/**
 * @file CLICopy.cpp
 */

#include <chrono>
#include <format>

#include "gd/gd_uuid.h"
#include "gd/gd_table_aggregate.h"

#include "../Command.h"
#include "../Application.h"

#include "CLICopy.h"


// @TAG #cli #copy


NAMESPACE_CLI_BEGIN

static std::pair<bool, std::string> FILE_PatternFilter_s( const gd::argument::shared::arguments& arguments_, CDocument* pdocument );


// ## Copy operations

std::pair<bool, std::string> Copy_g(const gd::cli::options* poptionsCopy, CDocument* pdocument)
{                                                                                                  assert(poptionsCopy != nullptr );
   const gd::cli::options& options_ = *poptionsCopy;
   std::string stringSource = (*poptionsCopy)["source"].as_string();
   CApplication::PreparePath_s(stringSource);                                 // if source is empty then set it to current path, otherwiss prepare it

   std::string stringIgnore = options_["ignore"].as_string();
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }

   unsigned uRecursive = options_["recursive"].as_uint();
   if(uRecursive == 0 && options_.exists("R") == true) uRecursive = 16;        // set to 16 if R is set, find all files

   pdocument->GetApplication()->UpdateApplicationState();

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter == "*" || stringFilter == "." || stringFilter == "**" ) 
   { 
      stringFilter.clear();                                                   // if filter is set to * then clear it, we want all files
      if( uRecursive == 0 ) uRecursive = 16;                                  // if recursive is not set, set it to 16, find all files
   }

   /*
   if( options_.exists("pattern") == true )                                    // 
   {
      //gd::argument::shared::arguments arguments_( { { "depth", uRecursive }, { "filter", stringFilter }, { "pattern", options_["pattern"].as_string() }, { "segment", options_["segment"].as_string() } });
      //auto result_ = DirPattern_g( stringSource, arguments_, pdocument );
   }
   else if( options_.exists("rpattern") == true )
   {

   }
   */
   if (options_.exists("target") == true)
   {
      gd::argument::shared::arguments arguments_({ { "depth", uRecursive } } );
      arguments_.append(options_.get_arguments(), { "filter", "overwrite", "pattern", "rpattern", "segment", "newer"});

      auto result_ = CopyFiles_g(stringSource, options_["target"].as_string(), arguments_, pdocument);
   }

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
 * @param stringSource      The source directory or file path to copy from.
 * @param stringTargetFolder The target directory path to copy files to.
 * @param arguments_        Options for the copy operation.
 * @param pdocument         Pointer to the document object for storing and displaying results.
 * @return std::pair<bool, std::string> Pair indicating success/failure and an error message if any.
 */
std::pair<bool, std::string> CopyFiles_g(const std::string& stringSource, const std::string& stringTargetFolder, const gd::argument::shared::arguments& arguments_, CDocument* pdocument)
{                                                                                               assert( stringSource != "" ); assert( stringTargetFolder != "" ); assert( pdocument != nullptr );
   auto ptableDir = pdocument->CACHE_Get( "file-dir", true );
   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();
   auto result_ = FILES_Harvest_WithWildcard_g( stringSource, stringFilter, ptableDir, uDepth, true); if( result_.first == false ) return result_;

   std::string stringTargetFolder_ = stringTargetFolder;

	pdocument->MESSAGE_Display(std::format("Files found from source/sources '{}'", stringSource));

   // ### Special case, if source is a file then just copy that file
   // Special case: if source is a file, copy that single file
   if( std::filesystem::is_regular_file(stringSource) == true )
   {
      std::filesystem::path pathSourceFile(stringSource);
      std::filesystem::path pathTargetFile(stringTargetFolder_);

      // Normalize target path based on special cases
      if( stringTargetFolder_ == "." || stringTargetFolder_ == "./" || stringTargetFolder_ == "" ) // current folder
      {
         pathTargetFile = std::filesystem::current_path() / pathSourceFile.filename();
      }
      else if( stringTargetFolder_ == ".." || stringTargetFolder_ == "../" )  // parent folder
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

      // Ensure target directory exists
      std::error_code errorcode;
      std::filesystem::create_directories(pathTargetFile.parent_path(), errorcode);
      if(errorcode) { return { false, "Failed to create target directory: " + pathTargetFile.parent_path().string() + " Error: " + errorcode.message() }; }

      // Copy the file
      std::filesystem::copy_file(pathSourceFile, pathTargetFile, std::filesystem::copy_options::overwrite_existing, errorcode);
      if(errorcode) { return { false, "Failed to copy file: " + pathSourceFile.string() + " to " + pathTargetFile.string() + " Error: " + errorcode.message() }; }

      pdocument->MESSAGE_Display("Copied file: " + pathSourceFile.string() + " to " + pathTargetFile.string());
      return { true, "" };
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

	// ## Prepare paths .......................................................

   std::filesystem::path pathSource = std::filesystem::canonical(stringSourceFolder);
   std::filesystem::path pathTarget = std::filesystem::canonical(stringTargetFolder_);
   if( pathSource == pathTarget )                                              // Check if source and target are the same folder
   {
      return { false, "Source and target folders cannot be the same" };
   }	                                                                                              LOG_DEBUG_RAW("Source folder: " & pathSource.string().c_str()); LOG_DEBUG_RAW("Target folder: " & pathTarget.string().c_str());

   // ## Apply pattern filter if set ..........................................

   if( arguments_.exists("pattern") == true )
   {
      gd::argument::shared::arguments argumentsFilter( arguments_ );
      argumentsFilter += { { "files", "file-dir" } };
      auto result_ = FILE_PatternFilter_s(argumentsFilter, pdocument); if( result_.first == false ) return result_;
   }

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

   // ## Prepare settings for copy operation .................................

	bool bOverwrite = arguments_["overwrite"].as_bool();                       // get overwrite option
	std::string stringNewer = arguments_["newer"].as_string();                 // get newer option, format is hh:mm:ss and with priority for hours, then minutes and last seconds if something is missing

   // Parse and calculate the time threshold for "newer" comparison
   std::chrono::system_clock::time_point timeThreshold;
   int iNewerFilter = 0;  // 0 = no filter, positive = newer than, negative = older than

   if (stringNewer.empty() == false)                                         // if newer is set then parse it
   {
      std::string stringTimeOnly = stringNewer;

      // Check for negative prefix and set filter direction
      if (stringTimeOnly[0] == '-')
      {
         iNewerFilter = -1;                                                    // negative = older files filter
         stringTimeOnly = stringTimeOnly.substr(1);                           // remove the negative sign for parsing
      }
      else
      {
         iNewerFilter = 1;                                                     // positive = newer files filter
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
         timeThreshold = now + std::chrono::seconds(uTotalSeconds);
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

   // ## copy files to target folder, if sub directories then create same file structure in target but remove the source folder

   unsigned uFilesCopied = 0;
   unsigned uFilesSkippedDueToAge = 0;
   unsigned uFilesSkippedDueToOverwrite = 0;

   for( const auto& stringSourceFile : vectorSourceFile )
   {
		std::filesystem::path pathSourceFile(stringSourceFile); // get the source file path
		std::filesystem::path pathRelative = std::filesystem::relative(pathSourceFile, stringSourceFolder); // get the relative path to source folder
		std::filesystem::path pathTargetFile = std::filesystem::path(stringTargetFolder) / pathRelative; // create the target file path
      
      // create target directory if it doesn't exist
      std::filesystem::path pathTargetDir = pathTargetFile.parent_path();
      if( std::filesystem::exists(pathTargetDir) == false )
      {
         std::error_code errorcode;
         std::filesystem::create_directories(pathTargetDir, errorcode);
         if( errorcode ) { pdocument->ERROR_Add("Failed to create directory: " + pathTargetDir.string() + " - " + errorcode.message()); continue; }
      }

		bool bTargetExists = std::filesystem::exists(pathTargetFile); // check if target file exists
      if(bOverwrite == false && bTargetExists == true) { uFilesSkippedDueToOverwrite++; continue; } // if not overwrite and file exists then skip

      if(iNewerFilter != 0 && bTargetExists == true)
      {
         // ### Apply "newer" or "older" filter logic .......................
         try
         {
				auto source_last_write_ = std::filesystem::last_write_time(pathSourceFile); // get last write time of source file
				auto source_time_ = std::chrono::system_clock::time_point(source_last_write_.time_since_epoch()); // convert to system clock time point

				if(iNewerFilter < 0)                                              // negative = older files filter
            {
               // For negative newer values: allow files that are older but not too old
					auto target_last_write_ = std::filesystem::last_write_time(pathTargetFile); // get last write time of target file
					auto target_time_ = std::chrono::system_clock::time_point(target_last_write_.time_since_epoch()); // convert to system clock time point  

               // Skip if source file is newer than target (we want older files in this mode)
               // But also skip if source is too old (beyond the time limit)
               if(source_time_ > target_time_ || source_time_ < timeThreshold) { uFilesSkippedDueToAge++; continue; }
            }
            else
            {
               // For positive newer values: standard newer logic
					auto target_last_write_ = std::filesystem::last_write_time(pathTargetFile); // get last write time of target file
					auto target_time_ = std::chrono::system_clock::time_point(target_last_write_.time_since_epoch()); // convert to system clock time point

               // Skip if source file is not newer than target file
               if (source_time_ <= target_time_) { uFilesSkippedDueToAge++; continue; }
				} // if(iNewerFilter < 0) ... else ... 
			} // try
         catch (const std::exception& e) { pdocument->ERROR_Add( std::format( "Failed to check file times for: {} - {}", stringSourceFile, e.what() ));  continue; }
      }

      // copy the file
      std::error_code errorcode;
      std::filesystem::copy_file(pathSourceFile, pathTargetFile, std::filesystem::copy_options::overwrite_existing, errorcode);
      if(errorcode)
      {
         pdocument->ERROR_Add("Failed to copy file: " + stringSourceFile + " to " + pathTargetFile.string() + " - " + errorcode.message());
      }
      else { uFilesCopied++; }
   }

   // Display summary
   pdocument->MESSAGE_Display("Copy operation completed");
   pdocument->MESSAGE_Display( std::format( "Files copied: {}", uFilesCopied ) );
   if(uFilesSkippedDueToOverwrite > 0) pdocument->MESSAGE_Display( std::format( "  Files skipped (overwrite disabled): {}", uFilesSkippedDueToOverwrite ) );
   if(uFilesSkippedDueToAge > 0)  pdocument->MESSAGE_Display( std::format( "  Files skipped (not newer): {}", uFilesSkippedDueToAge ) );

   return { true, "" };
}

std::pair<bool, std::string> FILE_PatternFilter_s(const gd::argument::shared::arguments& arguments_, CDocument* pdocument)
{
   auto vectorPattern = arguments_.get_all<std::string>("pattern"); // get all patterns from options and put them into vectorPattern

   // remove empty patterns
   vectorPattern.erase(std::remove_if(vectorPattern.begin(), vectorPattern.end(), [](const std::string& str) { return str.empty(); }), vectorPattern.end());
   if( vectorPattern.size() == 0 ) return {false, "No patterns provided."}; // if no patterns are provided, return an error

   auto result_ = pdocument->FILE_UpdatePatternList(vectorPattern, arguments_); // Search for patterns in harvested files and place them into the result table
   if( result_.first == false ) return result_;

   auto ptableLineList = pdocument->CACHE_Get("file-linelist", false);
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

   return { true, "" };
}



NAMESPACE_CLI_END