/**
 * @file CLIConfig.cpp
 * @brief Implementation file for CLI configuration operations.
 */


#include <fstream>
#include <filesystem>

#include "gd/gd_file.h"

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/types.h>
#endif

#include "../Application.h"

#include "CLI_Shared.h"

#include "CLIConfig.h"


NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> Configuration_g(const gd::cli::options* poptionsConfiguration)
{                                                                                                  assert(poptionsConfiguration != nullptr);
   const gd::cli::options& options_ = *poptionsConfiguration; // get the options from the command line arguments

   if( options_["create"].is_true() == true )
   {
      auto result_ = ConfigurationCreate_g();
      if( result_.first == false ) { return result_; }

      papplication_g->PrintMessage(result_.second, gd::argument::arguments() );
   }

   if( options_["edit"].is_true() == true )
   {
      // Open the configuration file in the default text editor
      auto result_ = ConfigurationEdit_g();
      if( result_.first == false ) { return result_; }

   }


   if( options_["print"].is_true() == true )
   {

   }

   return {true, ""};
}



/** ---------------------------------------------------------------------------
 * @brief Creates a default configuration file for the application if it does not already exist, handling platform-specific paths and directory creation.
 * 
 * Create the configuration file if it doesn't exist
 * For Windows: C:\Users\<username>\AppData\Local\cleaner\cleaner-configuration.json
 * For Linux: ~/.local/share/cleaner/cleaner-configuration.json
 *
 * @return A std::pair where the first element is a boolean indicating success (true if the configuration file exists or was created successfully, false otherwise).
 */
std::pair<bool, std::string> ConfigurationCreate_g()
{
   try {
      std::string stringPath;

#ifdef _WIN32
      // Windows: C:\Users\<username>\AppData\Local\cleaner\cleaner-configuration.json
      char* piAppData = nullptr;
      size_t uLength = 0;
      if( _dupenv_s(&piAppData, &uLength, "LOCALAPPDATA") == 0 && piAppData != nullptr )
      {
         stringPath = std::string(piAppData) + "\\cleaner";
         free(piAppData);
      }
      else { return { false, "Failed to get LOCALAPPDATA environment variable" }; }
#else
      // Linux: ~/.local/share/cleaner/cleaner-configuration.json
      const char* piDir = getenv("HOME");
      if( piDir == nullptr ) 
      {
         piDir = getenv("XDG_DATA_HOME");
         if(piDir != nullptr) 
         {
            piDir = getenv("USERPROFILE"); // Windows compatibility
            if(piDir == nullptr) { return { false, "Failed to get home directory" };}
         }
      }
      stringPath = std::string(piDir) + "/.local/share/cleaner";
#endif

      // Create directory if it doesn't exist
      std::filesystem::path pathCleaner(stringPath);
      if( !std::filesystem::exists(pathCleaner) )
      {
         if( !std::filesystem::create_directories(pathCleaner) ) { return { false, "Failed to create configuration directory: " + stringPath }; }
      }

      // Full path to configuration file
      std::filesystem::path pathFull = pathCleaner / "cleaner-configuration.json";

      // Check if configuration file already exists
      if( std::filesystem::exists(pathFull) ) { return { true, "Configuration file already exists at: " + pathFull.string() }; }

      // ## Create default configuration content
      //    cleaner.color = colors for different elements
      //    cleaner.format = format for key-value pairs
      //    cleaner.logging = logging settings
      //    cleaner.ignore = ignore patterns for folders and files
      std::string defaultConfig = R"({
"version": "1.0",
"cleaner.color": {
   "background": null,
   "body": null,
   "border": null,
   "default": null,
   "disabled": null,
   "error": null,
   "even": null,
   "footer": null,
   "header": null,
   "highlight": null,
   "info": null,
   "line": null,
   "odd": null,
   "success": null,
   "warning": null
"cleaner.format": {
   "keyvalue": null,
   "header-line": null,
},
"cleaner.logging": {
   "severity": null
},
"cleaner.ignore": {
   "folder": null,
   "file": null
}

})";

      // Write configuration file
      std::ofstream ofstreamFile(pathFull);
      if( !ofstreamFile.is_open() ) { return { false, "Failed to create configuration file: " + pathFull.string() }; }

      ofstreamFile << defaultConfig;
      ofstreamFile.close();

      if( ofstreamFile.fail() ) { return { false, "Failed to write to configuration file: " + pathFull.string() }; }

      return { true, "Configuration file created successfully at: " + pathFull.string() };

   }
   catch( const std::filesystem::filesystem_error& e )
   {
      return { false, "Filesystem error: " + std::string(e.what()) };
   }
   catch( const std::exception& e )
   {
      return { false, "Error creating configuration: " + std::string(e.what()) };
   }
}


std::pair<bool,std::string> ConfigurationEdit_g()
{
   std::string stringHomePath = papplication_g->PROPERTY_Get("folder-home").as_string();

   if( stringHomePath.empty() == true ) return { false, "Home path is not set in the application properties." };

   gd::file::path pathConfigFile(stringHomePath + "/cleaner-configuration.json");
                                                                      
   if( std::filesystem::exists( pathConfigFile ) == false ) return { false, "Configuration file does not exist: " + pathConfigFile.string() };

   return SHARED_OpenFile_g(pathConfigFile);
}


NAMESPACE_CLI_END