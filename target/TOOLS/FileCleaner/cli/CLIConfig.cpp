#include <fstream>

#ifdef _WIN32
    #include <windows.h>
    #include <shlobj.h>
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/types.h>
#endif

#include "../Application.h"

#include "CLIConfig.h"


NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> Configuration_g(const gd::cli::options* poptionsConfiguration)
{                                                                                                  assert(poptionsConfiguration != nullptr);
   const gd::cli::options& options_ = *poptionsConfiguration; // get the options from the command line arguments

   if( options_["create"].is_true() == true )
   {
      auto result_ = CreateConfiguration();
      if( result_.first == false ) { return result_; }

      papplication_g->PrintMessage(result_.second, gd::argument::arguments() );
   }

   return {true, ""};
}



/** ---------------------------------------------------------------------------
 * @brief Creates a default configuration file for the application if it does not already exist, handling platform-specific paths and directory creation.
 * 
 * Create the configuration file if it doesn't exist
 * For Windows: C:\Users\<username>\AppData\Local\cleaner\configuration.json
 * For Linux: ~/.local/share/cleaner/configuration.json
 *
 * @return A std::pair where the first element is a boolean indicating success (true if the configuration file exists or was created successfully, false otherwise).
 */
std::pair<bool, std::string> CreateConfiguration()
{
   try {
      std::string stringPath;

#ifdef _WIN32
      // Windows: C:\Users\<username>\AppData\Local\cleaner\configuration.json
      char* piAppData = nullptr;
      size_t uLength = 0;
      if( _dupenv_s(&piAppData, &uLength, "LOCALAPPDATA") == 0 && piAppData != nullptr )
      {
         stringPath = std::string(piAppData) + "\\cleaner";
         free(piAppData);
      }
      else { return { false, "Failed to get LOCALAPPDATA environment variable" }; }
#else
      // Linux: ~/.local/share/cleaner/configuration.json
      const char* piDir = getenv("HOME");
      if( piDir == nullptr ) 
      {
         struct passwd* pw = getpwuid(getuid());
         if( pw == nullptr ) { return { false, "Failed to get home directory" }; }
         piDir = pw->pw_dir;
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
      std::filesystem::path fullConfigPath = pathCleaner / "configuration.json";

      // Check if configuration file already exists
      if( std::filesystem::exists(fullConfigPath) ) { return { true, "Configuration file already exists at: " + fullConfigPath.string() }; }

      // Create default configuration content
      std::string defaultConfig = R"({
"version": "1.0",
"cleaner.color": {
   "line": null,
   "body": null,
   "header": null,
   "footer": null
}
})";

      // Write configuration file
      std::ofstream ofstreamFile(fullConfigPath);
      if( !ofstreamFile.is_open() ) { return { false, "Failed to create configuration file: " + fullConfigPath.string() }; }

      ofstreamFile << defaultConfig;
      ofstreamFile.close();

      if( ofstreamFile.fail() ) { return { false, "Failed to write to configuration file: " + fullConfigPath.string() }; }

      return { true, "Configuration file created successfully at: " + fullConfigPath.string() };

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


NAMESPACE_CLI_END