/**                                                                           @FILE [tag: cli, configuration] [description: Implementation file for CLI configuration operations] [type: source]
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

/// Get configuration data as a string view, this is the default configuration data
static std::string_view GetDefaultConfigData_s();
/// Create configuration file and write default configuration data to it.
static std::pair<bool, std::string> ConfigurationCreateFile_g( std::string_view stringFileName, std::string_view stringConfig );


/** ---------------------------------------------------------------------------
 * @brief Main function to get to process configuration operations
 * @param poptionsConfiguration Pointer to the command line options
 * @param poptionsConfiguration.create If true, create configuration file
 * @param poptionsConfiguration.local if true work on local configuration file
 * @param poptionsConfiguration.edit If true, edit configuration file
 *
 * @return A pair where the first element is a boolean indicating success (true) or failure (false), and the second element is a string message describing the result.
 */
std::pair<bool, std::string> Configuration_g(const gd::cli::options* poptionsConfiguration)
{                                                                                                  assert(poptionsConfiguration != nullptr);
   std::pair<bool, std::string> result_; // result to return, this is used to return success or failure of the operation
   const gd::cli::options& options_ = *poptionsConfiguration; // get the options from the command line arguments

   if( options_["create"].is_true() == true )
   {
      if( options_["local"].is_true() == true )                               // Create configuration file in the current working directory
      {
         result_ = ConfigurationCreateWorking_g();
         if( result_.first == false ) { return result_; }
      }
      else                                                                    // Create configuration file in the user's home directory
      {
         result_ = ConfigurationCreate_g();
         if( result_.first == false ) { return result_; }
      }

      papplication_g->PrintMessage(result_.second, gd::argument::arguments() );
   }

   if( options_["edit"].is_true() == true )
   {
      // Open the configuration file in the default text editor
      result_ = ConfigurationEdit_g();
      if( result_.first == false ) { return result_; }

   }


   if( options_["print"].is_true() == true )
   {

   }

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Creates a default configuration file in the current working directory if it does not already exist.
 * @return A pair where the first element is a boolean indicating success (true if the configuration file was created or already exists, false if an error occurred), and the second element is a string message describing the result.
 */
std::pair<bool, std::string>  ConfigurationCreateWorking_g()
{
   constexpr std::string_view stringConfigurationFileName( ".cleaner-configuration.json" );
   auto stringPath = papplication_g->PROPERTY_Get("folder-current").as_string();                   assert(stringPath.empty() == false);
   std::string stringFilePath; // contains the full path to the configuration file

   std::filesystem::path pathCurrent(stringPath);

   try
   {
      // Get the current working directory
      std::filesystem::path pathCurrent = std::filesystem::current_path();
      std::filesystem::path pathConfigFile = pathCurrent / stringConfigurationFileName;
      // Check if the configuration file already exists
      if( std::filesystem::exists(pathConfigFile) ) {
         return { true, "Configuration file already exists at: " + pathConfigFile.string() };
      }

      // Create the configuration file with default content
      std::string stringConfig(GetDefaultConfigData_s());
      auto result_ = ConfigurationCreateFile_g(pathConfigFile.string(), stringConfig);
      if( result_.first == false ) { return result_; }
      stringFilePath = pathConfigFile.string();
   }
   catch( const std::filesystem::filesystem_error& e )
   {
      return { false, "Filesystem error: " + std::string(e.what()) };
   }
   catch( const std::exception& e )
   {
      return { false, "Error creating configuration: " + std::string(e.what()) };
   }

   return { true, "Configuration file created: " + stringFilePath };
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
      std::string stringConfig( GetDefaultConfigData_s() );

      // Write configuration file
      auto result_ = ConfigurationCreateFile_g(pathFull.string(), stringConfig);
      if( !result_.first ) { return result_; }

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


/** ---------------------------------------------------------------------------
 * @brief Opens the configuration file in the system's default editor for editing.
 *
 * This function first attempts to find a local configuration file by searching up the directory tree.
 * If no local configuration file is found, it looks for the configuration file in the user's home directory.
 * Once the configuration file is located, it opens the file using the system's default editor.
 *
 * @return A std::pair where the first element is a boolean indicating success (true if the file was opened successfully, false otherwise),
 *         and the second element is a string message describing the result or any error that occurred.
 */
std::pair<bool,std::string> ConfigurationEdit_g()
{
   // ## Try to find local configuration file
   std::filesystem::path pathLocation;
   gd::file::path pathConfigFile;
   uint32_t uDirectoryLevels = 2;                                             // Default to 2 directory levels up
   auto result_ = CApplication::ConfigurationFindFile_s(pathLocation, uDirectoryLevels);

   if( pathLocation.empty() == true )
   {
      std::string stringHomePath = papplication_g->PROPERTY_Get("folder-home").as_string();
      if( stringHomePath.empty() == true ) return { false, "Unable to find configuration." };

      gd::file::path path_(stringHomePath + "/cleaner-configuration.json");
      if( std::filesystem::exists( path_ ) == false ) return { false, "Configuration file does not exist: " + path_.string() };
      pathConfigFile = path_;
   }
   else
   {
      pathConfigFile = pathLocation;
   }

   papplication_g->PrintMessage("Opening configuration file: " + pathConfigFile.string(), gd::argument::arguments());

   return SHARED_OpenFile_g(pathConfigFile);
}


/** ---------------------------------------------------------------------------
 * @brief Returns the default configuration data as a string_view.
 *
 * This function provides a JSON formatted string that represents the default configuration
 * for the application. The configuration includes color settings, format settings, logging
 * settings, and ignore patterns for folders and files.
 *
 * @return A std::string_view containing the default configuration data.
 */
static std::string_view GetDefaultConfigData_s()
{
   std::string_view stringConfig = R"({
"version": "1.0",
// color settings for different operations printed to the console
"cleaner.color": {
   "background": null,
   "body": "#F5DEB3",
   "brief": "#DEB887",
   "default": "#D4C5A9",
   "disabled": "#556655",
   "error": "#DC143C",
   "even": "#929F92",
   "footer": "#CD853F",
   "header": "#DAA520",
   "highlight": "#90EE90",
   "info": "#B8860B",
   "line": "#CB8553",
   "odd": "#5D6A5D",
   "success": "#228B22",
   "warning": "#DC143C"
},
"cleaner.format": {
   "kv": "[]:", // set how cleaner will find key-value separator formating
   "header-line": "0xDAC4BF", // ascii in hex format for header line
   "footer-line": "0xC0C4D9", // ascii in hex format for footer line
   "brief": ">>> " // ascii brief prefix
},
// default result settings
"cleaner.result": {
   "max-lines": 500
},
// logging settings
"cleaner.logging": {
   "severity": null
},
// ignore patterns for folders and files, place folders and files in arrays
"cleaner.ignore": {
   "folder": null,
   "file": null
},
// shortcuts for directories - format: "name":"path"
"cleaner.directory": {
}

})";
   return stringConfig;
}

/** ---------------------------------------------------------------------------
 * @brief Creates a configuration file with the specified name and writes the provided configuration data to it.
 *
 * This function attempts to create a file at the specified path and write the given configuration data into it.
 * If the file creation or writing fails, it returns an error message.
 *
 * @param stringFileName The name of the file to create.
 * @param stringConfig The configuration data to write into the file.
 * @return A std::pair where the first element is a boolean indicating success (true) or failure (false),
 *         and the second element is a string containing an error message if applicable.
 */
static std::pair<bool, std::string> ConfigurationCreateFile_g( std::string_view stringFileName, std::string_view stringConfig )
{
   std::ofstream ofstreamFile(stringFileName.data());
   if( ofstreamFile.is_open() == false ) { return { false, "Failed to create configuration file: " + std::string(stringFileName) }; }

   ofstreamFile << stringConfig;
   ofstreamFile.close();

   if( ofstreamFile.fail() ) { return { false, "Failed to write to configuration file: " + std::string(stringFileName) }; }

   return { true, "Configuration file created successfully at: " + std::string(stringFileName) };
}

NAMESPACE_CLI_END
