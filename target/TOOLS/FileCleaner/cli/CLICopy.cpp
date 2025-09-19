/**
 * @file CLICopy.cpp
 */

#include <format>

#include "gd/gd_uuid.h"

#include "../Command.h"
#include "../Application.h"

#include "CLICopy.h"


// @TAG #cli #copy


NAMESPACE_CLI_BEGIN

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
      gd::argument::shared::arguments arguments_({ { "depth", uRecursive }, { "filter", stringFilter }, { "pattern", options_["pattern"].as_string() }, { "segment", options_["segment"].as_string() } });
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
{                                                                                                  assert( stringSource != "" );
   auto ptableDir = pdocument->CACHE_Get( "file-dir", true );
   auto stringFilter = arguments_["filter"].as_string();
   unsigned uDepth = arguments_["depth"].as_uint();
   auto result_ = FILES_Harvest_g( stringSource, stringFilter, ptableDir, uDepth, true);        if( result_.first == false ) return result_;
   
   std::string stringSourceFolder;
   if(std::filesystem::is_directory(stringSource) == true) { stringSourceFolder = stringSource; }
   else
   {
      stringSourceFolder = std::filesystem::path(stringSource).parent_path().string();          // get parent directory if source is a file
   }
   
   // ## validate stringSource and stringTarget
   std::filesystem::path pathSource = std::filesystem::canonical(stringSourceFolder);
   std::filesystem::path pathTarget = std::filesystem::canonical(stringTargetFolder);
   if( pathSource == pathTarget )                                              // Check if source and target are the same folder
   {
      return { false, "Source and target folders cannot be the same" };
   }
   
   if( std::filesystem::exists(stringTargetFolder) == false )                 // ensure target folder exists
   {
      std::error_code errorcode;
      std::filesystem::create_directories(stringTargetFolder, errorcode);
      if(errorcode) return { false, "Failed to create target directory: " + errorcode.message() };
   }
   
   // ## Generate list of files to copy to target folder
   std::vector<std::string> vectorSourceFile;
   gd::table::dto::table* ptableFile = ptableDir;                             // get the table pointer
   for(const auto& itRowFile : *ptableFile)
   {
      std::string stringFile = itRowFile.cell_get_variant_view("path").as_string(); // get the file path
      
      if( std::filesystem::exists(stringFile) == false )                      // check if file exists
      {
         pdocument->ERROR_Add("File does not exist: " + stringFile); continue;
      }
      
      vectorSourceFile.push_back(stringFile);                                 // add file to vector
   }
   
   if( vectorSourceFile.empty() == true ) { return { false, "No files found to copy" }; } // no files to copy
   
   // ## copy files to target folder, if sub directories then create same file structure in target but remove the source folder
   for( const auto& stringSourceFile : vectorSourceFile )
   {
      std::filesystem::path pathSourceFile(stringSourceFile);
      std::filesystem::path pathRelative = std::filesystem::relative(pathSourceFile, stringSourceFolder);
      std::filesystem::path pathTargetFile = std::filesystem::path(stringTargetFolder) / pathRelative;
      
      // create target directory if it doesn't exist
      std::filesystem::path pathTargetDir = pathTargetFile.parent_path();
      if( std::filesystem::exists(pathTargetDir) == false )
      {
         std::error_code ec;
         std::filesystem::create_directories(pathTargetDir, ec);
         if( ec ) 
         {
            pdocument->ERROR_Add("Failed to create directory: " + pathTargetDir.string() + " - " + ec.message());
            continue;
         }
      }
      
      // copy the file
      std::error_code ec;
      std::filesystem::copy_file(pathSourceFile, pathTargetFile, std::filesystem::copy_options::overwrite_existing, ec);
      if( ec )
      {
         pdocument->ERROR_Add("Failed to copy file: " + stringSourceFile + " to " + pathTargetFile.string() + " - " + ec.message());
      }
   }
   
   return { true, "" };
}



NAMESPACE_CLI_END