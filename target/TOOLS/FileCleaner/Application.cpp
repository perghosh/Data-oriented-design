/**
 * @file Application.cpp
 *
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0Initialize.Application` - Initialize the application from command line
 * - `0TAG0RUN.Application` - run commands, there are a number of commands that can be run
 * - `0TAG0Options.Application` - prepare command line options
 * - `0TAG0Settings.Application` - settings operations
 *
 */


#include <filesystem>
#include <format>
#include <set>
#include <thread>

#include "pugixml/pugixml.hpp"

#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_table_io.h"
#include "gd/gd_file.h"
#include "gd/parse/gd_parse_window_line.h"
#include "gd/math/gd_math_string.h"


#ifdef _WIN32
#  include <windows.h>
#  include <shlobj_core.h> // For SHGetFolderPathW
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <pwd.h>
#  include <cstdlib>
#endif

#ifdef _WIN32
#include <shlwapi.h>
bool os_fnmatch(const char* piPattern, const char* piPath) {
   return PathMatchSpecA(piPath, piPattern) == TRUE;
}
#else
#include <fnmatch.h>
bool os_fnmatch(const char* piPattern, const char* piPath) {
   return fnmatch(piPattern, piPath, FNM_PATHNAME) == 0;
}
#endif

#include "jsoncons/json.hpp"
#include "jsoncons_ext/jsonpath/jsonpath.hpp"


#ifdef _WIN32
#  include "win/VS_Command.h"
#endif

#include "configuration/Settings.h"

#include "cli/CLIConfig.h"
#include "cli/CLICount.h"
#include "cli/CLICopy.h"
#include "cli/CLIDir.h"
#include "cli/CLIFind.h"
#include "cli/CLIHistory.h"
#include "cli/CLIKeyValue.h"
#include "cli/CLIList.h"
#include "cli/CLIPaste.h"
#include "cli/CLIRun.h"


#include "Command.h"

#include "Application.h"

#if defined( __clang__ )
   #pragma clang diagnostic ignored "-Wunused-variable"
   #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic ignored "-Wunused-variable"
   #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#elif defined( _MSC_VER )
#endif


/// Global pointer to application object
CApplication* papplication_g = nullptr;


CApplication::CApplication()
{
   m_pjsonConfig = std::make_unique<jsoncons::json>();
}

// Copy constructor
CApplication::CApplication(const CApplication& o)
{
   common_construct(o);
}

// Move constructor
CApplication::CApplication(CApplication&& o) noexcept
{
   common_construct(std::move(o));
}

// Copy assignment operator
CApplication& CApplication::operator=(const CApplication& o)
{
   if( this != &o )
   {
      common_construct(o);
   }
   return *this;
}

// Move assignment operator
CApplication& CApplication::operator=(CApplication&& o) noexcept
{
   if( this != &o )
   {
      common_construct(std::move(o));
   }
   return *this;
}


CApplication::~CApplication()
{
   m_vectorDocument.clear();                                                  // Clear the document vector
   m_vectorIgnore.clear();                                                    // Clear the ignore vector
   m_vectorProperty.clear();                                                  // Clear the property vector
   m_argumentsFolder.clear();                                                 // Clear the arguments folder
   m_argumentsVersion.clear();                                                // Clear the arguments version
   // Reset the global pointer
   papplication_g = nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Common construction logic for copy constructor and copy assignment operator.
 *
 * @param o The source object to copy from.
 */
void CApplication::common_construct(const CApplication& o)
{
   // Copy the document vector
   m_vectorDocument.clear();
   for( const auto& document_ : o.m_vectorDocument )
   {
      m_vectorDocument.push_back(std::make_unique<CDocument>(*document_));
   }
}

/** ---------------------------------------------------------------------------
 * @brief Common construction logic for move constructor and move assignment operator.
 *
 * @param o The source object to move from.
 */
void CApplication::common_construct(CApplication&& o) noexcept
{
   // Move the document vector
   m_vectorDocument = std::move(o.m_vectorDocument);
}

/** --------------------------------------------------------------------------- @TAG #type
 * @brief Returns the string representation of the current mode.
 */
std::string CApplication::GetModeAsString() const
{
   switch (m_eMode)
   {
   case eModeReview:   return "review";
   case eModeStats:    return "stats";
   case eModeSearch:   return "search";
   case eModeChanges:  return "changes";
   case eModeAudit:    return "audit";
   case eModeDocument: return "document";
   case eModeUnknown:
   default:            return "unknown";
   }
}


/** ---------------------------------------------------------------------------
 * @brief Returns the string representation of the UI type.
 *
 * This method converts the enum value of the UI type to its corresponding string representation.
 *
 * @return std::string The string representation of the UI type.
 */
std::string CApplication::GetUITypeAsString() const
{
   switch( m_eUIType )
   {
   case    eUITypeUnknown: return "unknown";
   case    eUITypeConsole: return "console";
   case    eUITypeWeb    : return "web";
   case    eUITypeWIMP   : return "wimp";
   case    eUITypeVSCode : return "vscode";
   case    eUITypeVS     : return "vs";
   case    eUITypeSublime: return "sublime";
   }
   return  "unknown";
}

/** --------------------------------------------------------------------------- @CODE [tag: mode, application, configuration] [summary: mode constants for application] [description: cleaners global modes and these modes describe a global state for how cleaner should behave]
 * @brief Sets the mode of the application based on the provided string.
 *
 * This method updates the application's mode based on the input string. If the string does not match any known modes,
 * it sets the mode to `eModeUnknown`.
 *
 * @param stringMode The string representation of the mode to set.
 */
void CApplication::SetMode(const std::string_view& stringMode)
{
   if     (stringMode == "review"  ) m_eMode = eModeReview;
   else if(stringMode == "stats"   ) m_eMode = eModeStats;
   else if(stringMode == "search"  ) m_eMode = eModeSearch;
   else if(stringMode == "changes" ) m_eMode = eModeChanges;
   else if(stringMode == "audit"   ) m_eMode = eModeAudit;
   else if(stringMode == "document") m_eMode = eModeDocument;
   else                              m_eMode = eModeUnknown;
}

/** ---------------------------------------------------------------------------
 * @brief Returns the string representation of the current detail level.
 */
std::string CApplication::GetDetailAsString() const
{
   switch (m_eDetail)
   {
   case eDetailUnknown:  return "unknown";
   case eDetailBasic:    return "basic";
   case eDetailStandard: return "standard";
   case eDetailExtended: return "extended";
   case eDetailFull:     return "full";
   }
   return "unknown";
}


/** ---------------------------------------------------------------------------
 * @brief Sets the detail level of the application based on the provided string.
 *
 * This method updates the application's detail level based on the input string. If the string does not match any known detail levels,
 * it sets the detail level to `eDetailUnknown`.
 *
 * @param stringDetail The string representation of the detail level to set.
 */
void CApplication::SetDetail(const std::string_view& stringDetail)
{
   // convert to lowercase
   std::string stringDetailLower( stringDetail );
   std::transform(stringDetailLower.begin(), stringDetailLower.end(), stringDetailLower.begin(), ::tolower);

   if     (stringDetailLower == "basic"   ) m_eDetail = eDetailBasic;
   else if(stringDetailLower == "standard") m_eDetail = eDetailStandard;
   else if(stringDetailLower == "extended") m_eDetail = eDetailExtended;
   else if(stringDetailLower == "full"    ) m_eDetail = eDetailFull;
   else                                     m_eDetail = eDetailUnknown;
}



/*
@TASK #user.kevin #area.options[name:save options to history][user: kevin]
[description:save the options to history file, so that user can see what options were used last time or select command from history
"store options in argument string, call historysave function for saving argument to file"]
[idea:add a command to save options to history file][state:open]
*/

/*
@TASK #user.kevin #area.options[name:save history no document][user: kevin]
[description:"
-check if document exists
-if document doesn't exist, create it
-CACHE_Prepare history key
-take arguments from command line
-add arguments to history table
-save history file
"]
[state:closed]
*/


/** --------------------------------------------------------------------------- // @CODE [tag: main, application, log] [description: Application Main method, core startup logic is placed or called from this method, think that is needed to get cleaner to work ]
 * @brief Prepares the application by setting up command-line options.
 *
 * Main in application is similar to main in application, but it is used to prepare
 * based on command line arguments. Here tha actual work is done.
 *
 * @param iArgumentCount The number of command-line arguments.
 * @param ppbszArgument The command-line arguments.
 * @param process_ A function to process the command-line arguments.
 */
std::pair<bool, std::string> CApplication::Main(int iArgumentCount, char* ppbszArgument[], std::function<bool(const std::string_view&, const gd::variant_view&)> process_)
{
   // ## Set OS-specific settings
#ifdef _WIN32
   papplication_g->PROPERTY_Add("os", "windows");                              // set OS to windows
#elif defined(__APPLE__)
   papplication_g->PROPERTY_Add("os", "macos");                                // set OS to macOS/Darwin
#else
   std::ifstream file_("/proc/version"); // open the file with os information to read the first line
   if( file_.is_open() == false ) { papplication_g->PROPERTY_Add("os", "linux"); }
   else
   {
      std::string stringLine; // string to hold the first line of the file
      std::getline(file_, stringLine);                                         // read first line
      file_.close();
      std::transform(stringLine.begin(), stringLine.end(), stringLine.begin(), ::tolower);// convert to lowercase
      if( stringLine.find("wsl") != std::string::npos || stringLine.find("") != std::string::npos )
      {
         papplication_g->PROPERTY_Add("os", "wsl");                            // set OS to WSL
      }
   }
#endif

   PrepareLogging_s();

   gd::argument::arguments argumentsHistory; // if history is enabled, this will hold the history arguments that is saved to history file when all is done

   auto result_ = Initialize();
   if( result_.first == false ) { return result_; }

   if( iArgumentCount > 1 )
   {
      PROPERTY_Set("threads", true);                                           // activate threading
      PROPERTY_Set("history-levels", uint64_t(3));                             // set history levels to 2

      std::string stringArgument = gd::cli::options::to_string_s(iArgumentCount, ppbszArgument, 1);
#ifndef NDEBUG
      // Debug: parse the arguments into a vector of strings to check if parsing works correctly
      auto vectorArgument_d = gd::cli::options::parse_s(stringArgument);                           assert( vectorArgument_d.size() == (iArgumentCount - 1) );
#endif


      PROPERTY_Add("arguments", stringArgument);

      gd::cli::options optionsApplication;
      CApplication::Prepare_s(optionsApplication);                             // prepare command-line options

      // ## Parse the command-line arguments

      auto [bOk, stringError] = optionsApplication.parse(iArgumentCount, ppbszArgument);// @CODE [tag: parse] [description: parse command line arguments]
      if( bOk == false )
      {
         std::string stringHelp;
         const gd::cli::options* poptionsActive = optionsApplication.find_active();
         if( poptionsActive != nullptr )
         {
            std::string stringDocumentation;
            HELP_PrintDocumentation( poptionsActive, stringDocumentation );
            PrintMessage( stringDocumentation, gd::argument::arguments() );

            stringError += "\n\n" + stringHelp;
         }

         return { false, stringError };
      }

      optionsApplication.set_argument_count( iArgumentCount );

		// ### Set print state if print flag is found ..........................

      if(optionsApplication.exists("print", gd::types::tag_state_active{}) == true) papplication_g->SetState(eApplicationStatePrint, 0);

		// ### Check if saving to history ......................................

      if( optionsApplication.exists("history", gd::types::tag_state_active{}) == true || optionsApplication.exists("add-to-history", gd::types::tag_state_active{}) == true )
      {
         // If command should be saved to history, we copy the arguments that is saved to history when application is done
         // This is done to make sure that any changes to arguments during application run is not saved to history
         argumentsHistory = optionsApplication.find_active()->get_arguments(); // get the arguments from the active options
      }

		// ### Prompt user for options .........................................

      if( optionsApplication.exists("prompt", gd::types::tag_state_active{}) == true )
      {
         auto result_ = CliPrompt_s( &optionsApplication );                    // prompt user for options
         if( result_.first == false ) { return result_; }
      }

      // ## Logging ........................................................... ## @CODE [tag: logging] [summary: setup logging] [description: if debug mode logging is activated, set logging severity to debug, otherwise set to info or warning based on configuration ]
#ifdef GD_LOG_SIMPLE
      bool bSetLogging = CliLogging_s( &optionsApplication );
#endif // GD_LOG_SIMPLE

      // ## Load configuration ................................................

      if( optionsApplication.exists("config", gd::types::tag_state_active{}) == true )// if config file is set
      {
         std::string stringConfigFile = optionsApplication.get_variant_view("config", gd::types::tag_state_active{}).as_string();
         if( stringConfigFile.empty() == false )
         {
            auto result_ = CONFIG_Load(stringConfigFile);                     // load configuration file
            if( result_.first == false ) { PrintError( result_.second, gd::argument::arguments()); }
         }
      }
      else
      {
         // ### Load the default configuration file ............................
         //     - try to find the configuration file in the current directory or parent directories
         //     - if not found, try to find the configuration file in the home directory

         std::filesystem::path pathConfigLocation; // path to the configuration file
         result_ = ConfigurationFindFile_s(pathConfigLocation, 2);            // try to find the configuration file in the current directory or parent directories
         if( result_.first == true && pathConfigLocation.empty() == false )
         {
            if( std::filesystem::exists(pathConfigLocation) == true )         // if configuration file exists
            {
               result_ = CONFIG_Load(pathConfigLocation.string());                                LOG_WARNING_RAW_IF(result_.first == false, result_.second);
                                                                                                  LOG_DEBUG_RAW_IF(result_.first == true, "== Loaded configuration file: " & pathConfigLocation.string());
            }
            else { LOG_DEBUG_RAW("Configuration file not found in current directory or parent directories."); }
         }
         else
         {
            result_ = CONFIG_Load();                                                               LOG_DEBUG_RAW_IF(result_.first == false, result_.second);
         }
      }

#ifdef GD_LOG_SIMPLE
      if( bSetLogging == false )                                              // if logging is not set, check for logging set in configuration
      {
         std::string stringSeverity = CONFIG_Get("logging", {"severity"}).as_string();
         if( stringSeverity.empty() == false )
         {
            auto eSeverityNumber = gd::log::severity_get_type_number_g(stringSeverity);
            if( eSeverityNumber != gd::log::enumSeverityNumber::eSeverityNumberNone )
            {
               gd::log::logger<0>* plogger = gd::log::get_s();
               plogger->set_severity( eSeverityNumber );                                           LOG_INFORMATION_RAW("== Set logging severity to: " & stringSeverity);
            }
         }
      }
#endif // GD_LOG_SIMPLE


      // ## Configure hardware ................................................

      {
         auto uThreadCount = std::thread::hardware_concurrency();
         if( uThreadCount > 0 )
         {
            PROPERTY_Add("threads", uThreadCount);                                                 LOG_INFORMATION_RAW("== Hardware concurrency: " & std::to_string(uThreadCount) & " threads");
         }
         else
         {
            PROPERTY_Add("threads", 1);                                                            LOG_INFORMATION_RAW("== Hardware concurrency: unknown, set to 1 thread");
         }
      }


      // ## Process the command-line arguments
      std::tie(bOk, stringError) = Initialize(optionsApplication);
      if( bOk == false ) { return { false, stringError }; }

      // @TASK #user.kevin [name: options history][summary: if history option is set then save last command line to history][user: kevin][created: 2025-08-11]
      if( optionsApplication.exists("history", gd::types::tag_state_active{}) == true || optionsApplication.exists("add-to-history", gd::types::tag_state_active{}) == true )
      {
         std::string stringHistory;
         std::filesystem::path pathHistory;
         auto result_ = HistoryFindActive_s(pathHistory);                       // Try to find history file, finds local or home history file but will first try local history file
         if( result_.first == true ) { stringHistory = pathHistory.string(); }

         gd::cli::options* poptionsActive = optionsApplication.find_active();
         if( poptionsActive->exists("add-to-history") == true ) { argumentsHistory.append("alias", (*poptionsActive)["add-to-history"].as_string_view()); }
         result_ = CLI::HistoryAppend_g(stringHistory, poptionsActive->name(), &argumentsHistory, "");  // Append the command to history
         if( result_.first == false ) { return result_; }
      }
   }
   else
   {
      gd::cli::options optionsApplication;
      CApplication::Prepare_s(optionsApplication);                             // prepare command-line options

      std::string stringHelp;
      optionsApplication.print_documentation(stringHelp, gd::cli::options::tag_documentation_table{});// print if no arguments
      PrintMessage( stringHelp, gd::argument::arguments() );
   }

   /*std::filesystem::path pathHistoryLocation;
   HistoryLocation_s(pathHistoryLocation); // Get the history location

   if( std::filesystem::exists(pathHistoryLocation) == true )
   {
      HISTORY_SaveCommand(pathHistoryLocation.string());
   }*/

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Initializes the application.
 *
 * This method performs the initial setup of the application, including setting up paths,
 * loading configuration, and preparing the application state.
 *
 * ## Steps:
 * 1. Set up OS-specific settings.
 * 2. Configure paths used by the application.
 * 3. Load configuration.
 * 4. Read ignore information if found.
 *
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Initialize()
{
   std::pair<bool, std::string> result_;
#ifdef _WIN32
   // Initialize Windows-specific functionality, such as COM
   result_ = PrepareWindows_s();
   if( result_.first == false ) return result_;
#endif

   // ## Configure current paths

   // ### Get the current working directory
   std::filesystem::path pathCurrent = std::filesystem::current_path();        // Get the current working directory
   std::string stringCurrentPath = pathCurrent.string();
   PROPERTY_Add("folder-current", stringCurrentPath );

   /// ## Set user home directory for cleaner
   std::string stringHomePath;
   result_ = FolderGetHome_s(stringHomePath);
   if( result_.first == false ) { LOG_DEBUG_RAW( result_.second ); }
   else { PROPERTY_Add("folder-home", stringHomePath); }                      // Add home folder to properties

   // ## Try to find ignore information

   std::vector<ignore> vectorIgnore;
   result_ = ReadIgnoreFile_s( stringCurrentPath, vectorIgnore );
   if( result_.first == false ) return result_;

   // Add ignore paths
   if( vectorIgnore.empty() == false )
   {                                                                                               LOG_INFORMATION_RAW("== Read: " & vectorIgnore.size() & " ignore patterns");
      IGNORE_Add( vectorIgnore );
   }



   // Perform initialization tasks here
   // For example, you might want to initialize documents or other resources

   // Example: Initialize documents
   // DOCUMENT_Add("example_document");

   // If initialization is successful
   return {true, ""};

   // If initialization fails, return an appropriate error message
   // return {false, "Initialization failed: <error details>"};
}

std::pair<bool, std::string> CApplication::Exit()
{
   // Perform cleanup tasks here
   // For example, you might want to clear documents or release resources

   // Example: Clear documents
   DOCUMENT_Clear();

   std::string stringArguments = PROPERTY_Get("arguments").as_string();

   //HistorySaveArguments_s(stringArguments);

#ifdef _WIN32
   ExitWindows_s();
#endif


   // If cleanup is successful
   return {true, ""};

   // If cleanup fails, return an appropriate error message
   // return {false, "Exit failed: <error details>"};
}

// 0TAG0Initialize.Application

/** --------------------------------------------------------------------------- @CODE [tag: application, option] [description: Initialize application from command line, go here if you want to check whats going to be executed]
 * @brief Initializes the application based on the provided command-line options.
 *
 * This method processes the command-line options and performs initialization tasks
 * based on the active subcommand. It supports various commands such as `count`, `db`,
 * `history`, `list`, `help`, and `version`.
 *
 * ### Steps:
 * 1. Retrieve the active subcommand from the provided options.
 * 2. Perform initialization tasks based on the subcommand:
 *    - **count**: Harvest files, apply filters, count rows, and optionally save or print results.
 *    - **db**: Open or create a database and update its schema.
 *    - **history**: Print the command history.
 *    - **list**: Harvest files, apply patterns, and list matching rows.
 *    - **help**: Display help information for the application.
 *    - **version**: Display the application version.
 * 3. Handle errors and return appropriate success or failure messages.
 *
 * @param optionsApplication The parsed command-line options.
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CApplication::Initialize( gd::cli::options& optionsApplication )
{
   gd::cli::options* poptionsActive = optionsApplication.find_active();
   if( poptionsActive == nullptr ) { return { false, "No active options found" }; }

   // Set argument count to found active options command if set to main options object, that might affect behaviour based on implementation
   if(optionsApplication.get_argument_count() != -1) { poptionsActive->set_argument_count(optionsApplication.get_argument_count()); }

   if( poptionsActive->exists("help") == true )
   {
      // @TODO #user.per [name: options] [description: improve format for help information, wrap lines, set indentation and site line width] [idea: add callback to format output ] [state: open]
      std::string stringDocumentation;

      HELP_PrintDocumentation( poptionsActive, stringDocumentation );

      PrintMessage( stringDocumentation, gd::argument::arguments() );
      return { true, "" };
   }

   if( optionsApplication.exists("mode", gd::types::tag_state_active{}) == true )// if mode the set application mode
   {
      auto stringMode = optionsApplication.get_variant_view("mode", gd::types::tag_state_active{} ).as_string_view();
      SetMode(stringMode);                                                    // set application mode
   }

   if(optionsApplication.exists("detail", gd::types::tag_state_active{}) == true)// if detail the set application detail
   {
      auto stringDetail = optionsApplication.get_variant_view("detail", gd::types::tag_state_active{}).as_string_view();
      if( stringDetail.empty() == false )
      {
         if( isdigit( stringDetail[0] ) )
         {
            // convert string to number
            int iDetail = atoi(stringDetail.data());
            switch( iDetail )
            {
               case 1: SetDetail("basic"   ); break;
               case 2: SetDetail("standard"); break;
               case 3: SetDetail("extended"); break;
               case 4: SetDetail("full"    ); break;
            }
         }
         else
         SetDetail(stringDetail);                                              // set application detail
      }
   }


   /// ## prepare command

#ifndef NDEBUG
   auto stringName_d = poptionsActive->name();
#endif // !NDEBUG

   // ## set editor
   std::string stringEditor = ( *poptionsActive )["editor"].as_string();
   PROPERTY_Set("editor", stringEditor);

   // ## check for verbatim mode (this writes extensive information to the console for user to understand whats going on)
   if( poptionsActive->exists("verbose") == true )
   {                                                                           // set verbatim mode
      PROPERTY_Set("verbose", true);                                                               LOG_INFORMATION_RAW("== Verbose mode enabled");
   }

   // ## set command name
   std::string stringCommandName = poptionsActive->name();
   PROPERTY_Set("command", stringCommandName);                                                     LOG_INFORMATION_RAW("== Command: " & stringCommandName);

   bool bUseThreads = PROPERTY_Get("threads").as_bool(); // get number of threads


   // ## Lambda to execute CLI functions in separate threads
   //    Note that `eApplicationStateWork` is set and should be checked for in application to delay exit
   auto execute_ = [&stringCommandName](auto call_, const gd::cli::options& options_, auto* pdocument_) -> std::pair<bool, std::string> {
      std::thread thread_([call_, options_, pdocument_, &stringCommandName]() {
         try
         {
            pdocument_->GetApplication()->SetState(eApplicationStateWork, eApplicationStateIdle); // set work state
            auto result_ = call_(&options_, pdocument_);
            if( result_.first == false )
            {
               pdocument_->ERROR_Add(result_.second);                          // Add error to the document's error list
               pdocument_->ERROR_Print();                                      // Print errors to the console
            }
         }
         catch(const std::exception& e)
         {
            std::string stringError = std::format("Error in {} thread: {}", stringCommandName, e.what());// generate error message
            pdocument_->ERROR_Add(stringError);                               // Add error to the document's error list
         }
         catch(...)
         {
            std::string stringError = std::format("Unknown error in {} thread", stringCommandName);// generate error message
            pdocument_->ERROR_Add(stringError);                               // Add error to the document's error list
         }

         pdocument_->GetApplication()->SetState(eApplicationStateIdle, eApplicationStateWork); // Set idle state
      });

      // ## Detach thread to run independently
      pdocument_->GetApplication()->SetState(eApplicationStateWork, eApplicationStateIdle); // set work state
      thread_.detach();

      return { true, "" };
   };

   // ## Lambda to execute CLI functions with options that may be modified in separate threads
   //    Note that `eApplicationStateWork` is set and should be checked for in application to delay exit
   auto execute_edit_ = [&stringCommandName](auto call_, gd::cli::options&& options_, auto* pdocument_) -> std::pair<bool, std::string> {
      std::thread thread_([call_, options_, pdocument_, &stringCommandName]() {
         try
         {
            pdocument_->GetApplication()->SetState(eApplicationStateWork, eApplicationStateIdle); // set work state
            auto result_ = call_((gd::cli::options*)&options_, pdocument_);
            if( result_.first == false )
            {
               pdocument_->ERROR_Add(result_.second);                          // Add error to the document's error list
               pdocument_->ERROR_Print();                                      // Print errors to the console
            }
         }
         catch(const std::exception& e)
         {
            std::string stringError = std::format("Error in {} thread: {}", stringCommandName, e.what());// generate error message
            pdocument_->ERROR_Add(stringError);                               // Add error to the document's error list
         }
         catch(...)
         {
            std::string stringError = std::format("Unknown error in {} thread", stringCommandName);// generate error message
            pdocument_->ERROR_Add(stringError);                               // Add error to the document's error list
         }

         pdocument_->GetApplication()->SetState(eApplicationStateIdle, eApplicationStateWork); // Set idle state
      });

      // ## Detach thread to run independently
      pdocument_->GetApplication()->SetState(eApplicationStateWork, eApplicationStateIdle); // set work state
      thread_.detach();

      return { true, "" };
   };


   // return executeInThread(CLI::Count_g, poptionsActive, pdocument, "Count");

   // ## check for sql statements
   if( poptionsActive->exists("statements") == true )
   {
      std::string stringFileName = ( *poptionsActive )["statements"].as_string();
      auto result_ = STATEMENTS_Load(stringFileName);
      if( result_.first == false ) return result_;
   }

   if( stringCommandName == "config" )                                         // command = "config"
   {
      return CLI::Configuration_g(poptionsActive);                             // manage configuration
   }
   else if (stringCommandName == "copy")                                       // copy
   {
      auto* pdocument = DOCUMENT_Get("copy", true);
      auto result_ = CLI::Copy_g(poptionsActive, pdocument);
      if (result_.first == false) return result_;
   }
   else if( stringCommandName == "count" )                                     // command = "count"
   {
      // Add a document for the "count" command
      auto* pdocument = DOCUMENT_Get("count", true);
      if( bUseThreads == true ) { return execute_(CLI::Count_g, poptionsActive->clone_arguments(), pdocument); } // count lines in file or directory in its own thread
      else                      { return CLI::Count_g(poptionsActive, pdocument); }// count lines in file or directory
   }
   else if( stringCommandName == "dir" )
   {
      auto* pdocument = DOCUMENT_Get("dir", true );
      auto result_ = CLI::Dir_g(poptionsActive, pdocument);
      if( result_.first == false ) return result_;
   }
   else if( stringCommandName == "find" )
   {
      // prepare options for find command, this is for usability so that user can use find command without specifying file or pattern
      // Check for only one argument, then this should search for that element
      if(poptionsActive->get_argument_count() == 3)                           // 3 is 3 subtracting program name and command name
      {                                                                                            assert(poptionsActive->exists("R") == false);
         // move from filter to pattern
		   auto stringPattern = (*poptionsActive)["filter"].as_string_view();
         poptionsActive->add_value("pattern", stringPattern);
         poptionsActive->set_value("filter", "**");
      }

      auto* pdocument = DOCUMENT_Get("find", true );
      auto result_ = CLI::Find_g( poptionsActive, pdocument );
      if( result_.first == false ) return result_;
   }
   else if( stringCommandName == "history" )
   {
      auto* pdocument = DOCUMENT_Get("history", true );
      auto result_ = CLI::History_g( poptionsActive, &optionsApplication, pdocument );
      if( result_.first == false ) return result_;
   }
   else if( stringCommandName == "kv" )
   {
      auto* pdocument = DOCUMENT_Get("keyvalue", true );
      auto result_ = CLI::KeyValue_g( poptionsActive, pdocument );
      if( result_.first == false ) return result_;
   }
   else if( stringCommandName == "list" )
   {
		// prepare options for list command, this is for usability so that user can use list command without specifying file or pattern
      // Check for only one argument, then this should search for that element
		if(poptionsActive->get_argument_count() == 3)                           // 3 is 3 subtracting program name and command name
      {                                                                                            assert(poptionsActive->exists("R") == false);
         // move from filter to pattern
		   auto stringPattern = (*poptionsActive)["filter"].as_string_view();
         poptionsActive->add_value("pattern", stringPattern);
         poptionsActive->set_value("filter", "**");
      }

      // Add a document for the "count" command
      auto* pdocument = DOCUMENT_Get("list", true);
      if( bUseThreads == true )
      {
         auto options_ = poptionsActive->clone();
         return execute_edit_(CLI::List_g, std::move(options_), pdocument);    // list lines in file or directory with the matched pattern in its own thread
      }
      else                      { return CLI::List_g(poptionsActive, pdocument); }// list lines in file or directory with the matched pattern
      if( pdocument->ERROR_Empty() == false ) { pdocument->ERROR_Print(); }
   }
   else if( stringCommandName == "paste" )
   {
      return CLI::Paste_g( poptionsActive, &optionsApplication );
   }
   else if( stringCommandName == "run" )
   {
      std::string stringCommand = ( *poptionsActive )["command"].as_string();

      return CLI::Run_g(stringCommand, this);

      /*
      if( bUseThreads == true ) { return execute_(CLI::Run_g, stringCommand, this); } // list lines in file or directory with the matched pattern in its own thread
      else                      { return CLI::Run_g(poptionsActivestringCommand, this); }// list lines in file or directory with the matched pattern
      */
   }
   else if( stringCommandName == "help" )                                      // command = "help"
   {
      using namespace gd::cli; // use namespace for options
      std::string stringDocumentation, stringFlags;

      stringDocumentation += "\n\n";
      stringDocumentation += gd::console::rgb::print(CONFIG_Get("color", { "disabled", "default" }).as_string(), gd::types::tag_color{});
      std::string stringTemp =  "Requested help for commands";
      stringDocumentation += gd::math::string::format_header_line(stringTemp, 80); // format header line for command name
      stringDocumentation += "\n\n";

      HELP_PrintDocumentation( &optionsApplication, stringDocumentation );

      std::cout << stringDocumentation << "\n";
   }
   else if( stringCommandName == "version" )
   {
      std::cout << "version 1.1.0" << "\n";
   }
   else
   {
      return { false, "Unknown command: " + stringCommandName };
   }

   return { true, "" };
}

/** --------------------------------------------------------------------------- @TAG #option #internal #application
 * @brief Internal initialization method for executing commands programmatically.
 *
 * This method is designed for internal use, particularly by the history command
 * to execute other commands. It processes command-line options without threading,
 * verbose output, or external setup tasks.
 *
 * ### Features:
 * - Excludes history command to prevent recursion.
 * - Direct command execution without UI elements.
 * - Supports the following subcommands:
 *   - **config**: Manages configuration files, including creation, editing, and backup.
 *   - **copy**: Copies files from source to target, with support for filters, patterns, and backup options.
 *   - **count**: Counts lines or patterns in files or directories, supports recursive and filtered operations.
 *   - **dir**: Lists files in directories, with filtering, pattern matching, and sorting capabilities.
 *   - **find**: Searches for patterns in files, supports multiline and key-value extraction.
 *   - **kv**: Extracts key-value pairs from files based on specified rules and formats.
 *   - **list**: Lists lines in files matching specified patterns, supports context and scripting.
 *   - **paste**: Reads text from clipboard or input files for further processing.
 *   - **run**: Executes commands from loaded templates or history.
 *   - **version**: Returns the current application version.
 *   - **help**: Displays help information for commands and options.
 * - Each subcommand is executed without threading, verbose output, or external setup, ensuring fast and direct execution.
 *
 * @param optionsApplication The parsed command-line options.
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CApplication::InitializeInternal( gd::cli::options& optionsApplication )
{
   gd::cli::options* poptionsActive = optionsApplication.find_active();
   if( poptionsActive == nullptr ) { return { false, "No active options found" }; }

   bool bSetLogging = CliLogging_s(&optionsApplication);

   // Get command name
	std::string stringCommandName = poptionsActive->name();                                         LOG_DEBUG_RAW("== Internal command: " & stringCommandName);

   // Set basic properties without verbose logging
   PROPERTY_Set("command", stringCommandName);

   if( optionsApplication.exists("prompt", gd::types::tag_state_active{}) == true )
   {
      auto result_ = CliPrompt_s( &optionsApplication );                      // prompt user for options
      if( result_.first == false ) { return result_; }
   }

   // Execute commands (excluding history to prevent recursion)
   if( stringCommandName == "config" )
   {
      return CLI::Configuration_g(poptionsActive);
   }
   else if (stringCommandName == "copy")
   {
      auto* pdocument = DOCUMENT_Get("copy", true);
      return CLI::Copy_g(poptionsActive, pdocument);
   }
   else if( stringCommandName == "count" )
   {
      auto* pdocument = DOCUMENT_Get("count", true);
      return CLI::Count_g(poptionsActive, pdocument);
   }
   else if( stringCommandName == "dir" )
   {
      auto* pdocument = DOCUMENT_Get("dir", true);
      return CLI::Dir_g(poptionsActive, pdocument);
   }
   else if( stringCommandName == "find" )
   {
      auto* pdocument = DOCUMENT_Get("find", true);
      return CLI::Find_g(poptionsActive, pdocument);
   }
   else if( stringCommandName == "kv" )
   {
      auto* pdocument = DOCUMENT_Get("keyvalue", true);
      return CLI::KeyValue_g(poptionsActive, pdocument);
   }
   else if( stringCommandName == "list" )
   {
      auto* pdocument = DOCUMENT_Get("list", true);
      auto result_ = CLI::List_g(poptionsActive, pdocument);
      if( pdocument->ERROR_Empty() == false ) { pdocument->ERROR_Print(); }
      return result_;
   }
   else if( stringCommandName == "paste" )
   {
      return CLI::Paste_g(poptionsActive, &optionsApplication);
   }
   else if( stringCommandName == "run" )
   {
      std::string stringCommand = ( *poptionsActive )["command"].as_string();
      return CLI::Run_g(stringCommand, this);
   }
   else if( stringCommandName == "version" )
   {
      // For internal use, return version info without console output
      return { true, "version 1.0.8" };
   }
   else if( stringCommandName == "history" )
   {
      // Prevent recursion - history command should not call itself internally
      return { false, "History command cannot be executed internally to prevent recursion" };
   }
   else
   {
      return { false, "Unknown command: " + stringCommandName };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Updates the application state based on information from application data.
 *
 * This function is responsible for updating the internal state of the application
 * based on the current configuration and options. Application state are information
 * that are global, it affects everything in the application.
 */
void CApplication::UpdateApplicationState()
{
   // ## Update ignore state based on ignore information
   //    This state is used to optimize file operations and avoid unnecessary checks
   unsigned uIgnore = 0;
   SetState( 0, (eApplicationStateCheckIgnoreFolder|eApplicationStateCheckIgnoreFile) );
   for( const auto& it : m_vectorIgnore )
   {
      if( it.is_file() == true )
      {
         uIgnore |= eApplicationStateCheckIgnoreFile; // set ignore file state
      }
      else if( it.is_folder() == true )
      {
         uIgnore |= eApplicationStateCheckIgnoreFolder; // set ignore folder state
      }
   }

   SetState( uIgnore, 0 );
}

/** --------------------------------------------------------------------------- @TAG #directory
 * @brief Creates application specific directory where files used for cleaner are stored
 *
 * Cleaner can be configured with configuration file and it also handles history
 * On Windows this is stored in %APPDATA%/tools/cleaner and on Linux in ~/.config/cleaner
 *
 * @return std::pair<bool, std::string> - (success, error message)
 *         - success: true if directory was created or already exists, false on failure
 *         - error message: empty string on success, descriptive error on failure
 */
std::pair<bool, std::string> CApplication::CreateDirectory()
{
   std::filesystem::path pathTarget;

#ifdef _WIN32
   // ## Get %APPDATA% path using SHGetFolderPathW

   wchar_t puAppdataPath[MAX_PATH];
   HRESULT iResult = ::SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, puAppdataPath);
   if(FAILED(iResult)) { return { false, "Failed to retrieve APPDATA path" }; }

   // Convert wide string to UTF-8 string
   char utf8_path[MAX_PATH * 4]; // Buffer for UTF-8 conversion
   int iConverted = ::WideCharToMultiByte(CP_UTF8, 0, puAppdataPath, -1, utf8_path, sizeof(utf8_path), NULL, NULL);
   if(iConverted == 0) { return { false, "Failed to convert APPDATA path to UTF-8" }; }

   // Construct full path: %APPDATA%/tools/cleaner
   pathTarget = std::filesystem::path(utf8_path) / "tools" / "cleaner";

#else // Linux
   // ## Check for $XDG_CONFIG_HOME, fallback to ~/.config

   const char* pbszXDG_CONFIG_HOME = std::getenv("XDG_CONFIG_HOME");
   std::string stringConfigurationBase;
   if( pbszXDG_CONFIG_HOME != nullptr && *pbszXDG_CONFIG_HOME != '\0' )
   {
      stringConfigurationBase = pbszXDG_CONFIG_HOME;
   }
   else
   {
      const char* pbszHome = std::getenv("HOME");                             // Get home directory
      if( pbszHome != nullptr )
      {
         /* TODO: Uncomment if you want to use getpwuid as fallback, do no work with static linking
         struct passwd* pw = getpwuid(getuid());                              // Fallback to getpwuid if $HOME is not set
         if(!pw || !pw->pw_dir) { return { false, "Failed to retrieve home directory" }; }
         pbszHome = pw->pw_dir;
         */
         return { false, "Failed to retrieve home directory" }; // If HOME is not set, return error
      }
      stringConfigurationBase = std::string(pbszHome) + "/.config";
   }

   pathTarget = std::filesystem::path(stringConfigurationBase) / "cleaner";    // Construct full path: ~/.config/cleaner
#endif

   try
   {
      if(!std::filesystem::exists(pathTarget) == false )                       // Create directory if it doesn't exist (recursive creation for 'tools' on Windows)
      {
         if( std::filesystem::create_directories(pathTarget) == false )
         {
            return { false, "Failed to create directory: " + pathTarget.string() };
         }
      }
   }
   catch (const std::filesystem::filesystem_error& e)
   {
      return { false, "Failed to create directory: " + std::string(e.what()) };
   }
   catch (const std::exception& e)
   {
      return { false, "Unexpected error: " + std::string(e.what()) };
   }

   return { true, "" };
}


/** --------------------------------------------------------------------------- @TAG #print
 * @brief Prints a message to the console or other output based on the UI type.
 *
 * This method handles printing messages to different output targets based on the application's UI type.
 * It supports console, web, WIMP (Windows, macOS, Linux), Visual Studio Code, and Sublime Text.
 *
 * @param stringMessage The message to print.
 * @param argumentsFormat Optional formatting arguments for the message.
 * @return std::pair<bool, std::string> A pair indicating success or failure and an error message if applicable.
 */
std::pair<bool, std::string> CApplication::PrintMessage(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat)
{
   // Optionally extract formatting options from argumentsFormat
   // For example: severity, color, output target, etc.
   // Example: std::string_view severity = argumentsFormat["severity"].as_string_view();

   std::unique_lock<std::shared_mutex> lock_( m_sharedmutex );

   enumUIType eUIType = m_eUIType; // Get the UI type from the application instance

   if( argumentsFormat.exists("ui") == true )
   {
      std::string stringUIType = argumentsFormat["ui"].as_string();
      eUIType = GetUITypeFromString_s( stringUIType );
   }

   switch(eUIType)
   {
   case eUITypeUnknown:
   case eUITypeConsole:
      {
         if( argumentsFormat.exists("color") == true )
         {
            std::string stringColor = argumentsFormat["color"].as_string();
            if( stringColor.empty() == false ) { stringColor = CONFIG_Get("color", stringColor).as_string(); }
            if( stringColor.empty() == false )
            {
               stringColor = gd::console::rgb::print(stringColor, gd::types::tag_color{});
               std::cout << stringColor;                                      // Print the color code before the message
            }
         }
         std::cout << stringMessage << std::endl;
      }
      break;
   case eUITypeWeb:
      // Implement web output logic here
      // e.g., send message to web UI log
      break;
   case eUITypeWIMP:
      // Implement desktop UI message box or log
      break;
   case eUITypeVSCode:
#ifdef _WIN32
   case eUITypeVS:
      {
         auto result_ = VS::CVisualStudio::Print_s( stringMessage, VS::tag_vs_output{});
         if( result_.first == false )
         {
            std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
            std::cerr << stringError << "\n";
            return result_;
         }
      }
      break;
#endif // _WIN32
   case eUITypeSublime:
      // Implement extension output logic here
      break;
   default:
      // Fallback to console
      std::cout << stringMessage << std::endl;
      break;
   }

   return {true, ""};
}

/**
 * @brief Prints a progress message to the console or other output based on the UI type.
 */
std::pair<bool, std::string> CApplication::PrintProgress(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat) // @TAG #progress.Application
{
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutex );


   constexpr size_t uMaxLength = 100; // Maximum length for the message
   constexpr size_t uMIN_LENGTH_PROGRESS = 60; // Minimum length for the progress bar
   constexpr size_t uMAX_LENGTH_PROGRESS = 120; // Maximum length for the progress bar

   enumUIType eUIType = m_eUIType; // Get the UI type from the application instance
   std::string stringPrint( stringMessage );

   if( stringPrint.length() < uMaxLength )
   {
      stringPrint += std::string(uMaxLength - stringPrint.length(), ' ');      // Pad with spaces to fill the line
   }
   else if( stringPrint.length() > uMaxLength )
   {
      stringPrint = stringPrint.substr(0, uMaxLength);
   }


   if( argumentsFormat.exists("ui") == true )
   {
      std::string stringUIType = argumentsFormat["ui"].as_string();
      eUIType = GetUITypeFromString_s( stringUIType );
   }

   if( eUIType == eUITypeUnknown ) { eUIType = eUITypeConsole; } // Fallback to console if UI type is unknown

   switch(eUIType)
   {
   case eUITypeConsole:
   {
      if( argumentsFormat.empty() == false )
      {
         if( m_console.empty() == true )
         {
            auto result_ = m_console.initialize();                             // Initialize console
            if( result_.first == false ) return result_;
         }

         // ## Print progress to console
         if( argumentsFormat.exists("percent") == true )                       // print progress with percentage
         {
            std::string stringProgress;

            if( argumentsFormat.exists("label") == true )            // If "label" argument is present, use it
            {
               stringProgress = argumentsFormat["label"].as_string() + ": ";
            }

            unsigned uPercent = argumentsFormat["percent"].as_uint();
            stringProgress += std::format("[{:3d}%] ", uPercent);


            if( m_console.get_width() > uMIN_LENGTH_PROGRESS )
            {
               unsigned uWidth = 80; // Default width of the progress bar
               if( m_console.get_width() < ( uWidth + stringProgress.length() - 5 ) ) uWidth = (unsigned)( m_console.get_width() - stringProgress.length() - 5 ); // Adjust width based on console size
               if( (unsigned)m_console.get_width() < uWidth ) uWidth = (unsigned)m_console.get_width(); // Adjust width to console size

               gd::console::progress progressBar( m_console.yx( gd::types::tag_type_unsigned{}), uWidth );

               progressBar.update(uPercent, gd::types::tag_percent{});
               progressBar.print_to( "[ ", "=", ">", " ]", stringProgress );
            }

            m_console.print( stringProgress );

            if( argumentsFormat.exists("sticky") == true ) { std::cout << "\r"; } // If "sticky" argument is present, keep the cursor on the same line
         }
         else if( argumentsFormat.exists("clear") == true )
         {
            m_console.clear_line();                                            // Clear the current line in the console
         }

      }
      else
      {
         std::cout << stringPrint;
      }
   }
   break;
   case eUITypeWeb:
      // Implement web output logic here
      // e.g., send message to web UI log
      break;
   case eUITypeWIMP:
      // Implement desktop UI message box or log
      break;
   case eUITypeVSCode:
#ifdef _WIN32
   case eUITypeVS:
   {
      auto result_ = VS::CVisualStudio::Print_s( stringMessage, VS::tag_vs_output{});
      if( result_.first == false )
      {
         std::string stringError = std::format("Failed to print to Visual Studio: {}", result_.second);
         std::cerr << stringError << "\n";
         return result_;
      }
   }
   break;
#endif // _WIN32
   case eUITypeSublime:
      // Implement extension output logic here
      break;
   default:
   {
      // Fallback to console
      if( argumentsFormat.exists("clear") == true )
      {
         stringPrint = "";                                                     // Clear the line if "clear" argument is present
         stringPrint.append(uMaxLength, ' ');                                  // Fill the line with spaces
         stringPrint += "\r";                                                  // Move the cursor to the beginning of the line
      }

      std::cout << "\033[A\033[2K\r" << stringPrint << std::flush;
   }
   break;
   }

   return {true, ""};
}

std::pair<bool, std::string> CApplication::PrintError(const std::string_view& stringMessage, const gd::argument::arguments& argumentsFormat)
{
   std::cout << "\n##\n## ERROR \n## ------\n" << stringMessage << std::flush;
   return {true, ""};
}

void CApplication::Print( std::string_view stringColor,  gd::types::tag_background )
{
   if( !(m_eUIType == eUITypeUnknown || m_eUIType == eUITypeConsole) ) return;

   std::string stringColorCode = CONFIG_Get("color", stringColor).as_string();
   if( stringColorCode.empty() == false )
   {
      stringColorCode = gd::console::rgb::print(stringColorCode, gd::types::tag_background{});
      std::cout << stringColorCode;                                           // Print the color code before the message
      std::cout << "\033[2J";
      std::cout << "\033[H";
   }
   else
   {
      // ## Reset all attributes and clear the screen to return to the default state
      std::cout << "\033[0m";
      //std::cout << "\033[2J";
      //std::cout << "\033[H";
   }
}


std::pair<bool, std::string> CApplication::STATEMENTS_Load(const std::string_view& stringFileName)
{
   using namespace pugi;
   xml_document xmldocument;   // xml document used to load xml from file

   if( std::filesystem::exists( stringFileName ) == false ) return { true, std::string("statements file not found ") + stringFileName.data() };

   m_pstatements = std::make_unique<application::database::metadata::CStatements>();

   // ## Load xml data
   xml_parse_result xmlparseresult = xmldocument.load_file(stringFileName.data()); // load xml file
   if( (bool)xmlparseresult == false ) return { false, xmlparseresult.description() };

   {
      // ## get all statements
      auto xmlnodeStatements = xmldocument.document_element().child( "statements" );
      while( xmlnodeStatements.empty() == false )
      {
         // ## get statement
         pugi::xml_node xmlnode = xmlnodeStatements.first_child();
         while( xmlnode.empty() == false )                                     // if child
         {
            if( std::string_view(xmlnode.name()) == std::string_view{ "statement" } )
            {
               std::string_view stringName = xmlnode.attribute("name").value();
               std::string_view stringType = xmlnode.attribute("type").value();
               std::string_view stringStatement = xmlnode.child_value();
               if( stringName.empty() == false && stringStatement.empty() == false )
               {
                  if( stringType.empty() == true ) { stringType = "select"; }
                  m_pstatements->Append( gd::argument::arguments( { { "name", stringName }, { "type", stringType }, { "sql", stringStatement } }, gd::types::tag_view{}) ); // add statement to list
               }
            }
            xmlnode = xmlnode.next_sibling();                                  // get next statement
         }

         xmlnodeStatements = xmlnodeStatements.next_sibling("statements");     // get next statements
      }
   }


   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document with the specified name.
 *
 * @param stringName The name of the document to add.
 */
CDocument* CApplication::DOCUMENT_Add(const std::string_view& stringName)
{
   auto pdocument = std::make_unique<CDocument>( this, stringName );
   m_vectorDocument.push_back(std::move(pdocument));
   return m_vectorDocument.back().get();
}

/** ---------------------------------------------------------------------------
 * @brief Adds a new document based on the provided arguments.
 *
 * @param arguments_ The arguments used to create the document.
 */
CDocument* CApplication::DOCUMENT_Add(const gd::argument::shared::arguments& arguments_)
{
   auto pdocument = std::make_unique<CDocument>( arguments_ );
   // Assuming CDocument has a method to initialize from arguments
   // doc->Initialize(arguments_);
   m_vectorDocument.push_back(std::move(pdocument));
   return m_vectorDocument.back().get();
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name.
 *
 * @param stringName The name of the document to retrieve.
 * @return CDocument* Pointer to the document if found, otherwise nullptr.
 */
const CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName) const
{
   for( const auto& pdocument : m_vectorDocument )
   {
      if(pdocument->GetName() == stringName)
      {
         return pdocument.get();
      }
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name.
 *
 * @param stringName The name of the document to retrieve.
 * @return CDocument* Pointer to the document if found, otherwise nullptr.
 */
CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName)
{
   for( const auto& pdocument : m_vectorDocument )
   {
#ifndef NDEBUG
      auto stringName_d = pdocument->GetName();
#endif // !NDEBUG

      if(pdocument->GetName() == stringName)
      {
         return pdocument.get();
      }
   }
   return nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Retrieves a document by its name, creating it if it doesn't exist.
 *
 * @param stringName The name of the document to retrieve or create.
 * @param bCreate Whether to create the document if it doesn't exist and bCreate is true.
 * @return CDocument* Pointer to the document if found or created, otherwise nullptr.
 */
CDocument* CApplication::DOCUMENT_Get(const std::string_view& stringName, bool bCreate)
{
   auto pdocument = DOCUMENT_Get(stringName);
   if( pdocument == nullptr && bCreate == true )
   {
      pdocument = DOCUMENT_Add(stringName);
   }
   return pdocument;
}


/** ---------------------------------------------------------------------------
 * @brief Removes a document by its name.
 *
 * @param stringName The name of the document to remove.
 */
void CApplication::DOCUMENT_Remove(const std::string_view& stringName)
{
   m_vectorDocument.erase(
      std::remove_if(m_vectorDocument.begin(), m_vectorDocument.end(),
         [&stringName](const std::unique_ptr<CDocument>& doc) {
            return doc->GetName() == stringName;
         }),
      m_vectorDocument.end());
}

/** ---------------------------------------------------------------------------
 * @brief Gets the number of documents.
 *
 * @return size_t The number of documents.
 */
size_t CApplication::DOCUMENT_Size() const
{
   return m_vectorDocument.size();
}

/** ---------------------------------------------------------------------------
 * @brief Checks if there are no documents.
 *
 * @return bool True if there are no documents, otherwise false.
 */
bool CApplication::DOCUMENT_Empty() const
{
   return m_vectorDocument.empty();
}

/** -
 * @brief Clears all documents.
 */
void CApplication::DOCUMENT_Clear()
{
   m_vectorDocument.clear();
}

void CApplication::IGNORE_Add(const std::vector<std::string> vectorIgnore)     // @TAG #ignore.Application
{
   for( const auto& stringIgnore : vectorIgnore )
   {
      std::string stringValue = stringIgnore; // copy the ignore pattern
      unsigned uType = 0;                                          // type of ignore
      if( stringIgnore[0] == '/' )                                  // if starts with / then it is a folder
      {
         uType = unsigned(ignore::eTypeRoot|ignore::eTypeFolder);
         stringValue = stringValue.substr(1);                      // remove the first character
      }
      else if( stringValue.back() == '/' )
      {
         uType = unsigned(ignore::eTypeFolder);
         stringValue = stringValue.substr(0, stringValue.length() - 1); // remove the last character
      }
      else if( stringValue.find_first_of("*?") != std::string::npos )
      {
         uType = unsigned(ignore::eTypeFile|ignore::eTypeWildcard);
      }

      if( uType != 0 )
      {
         if( stringValue.find_first_of("*?") != std::string_view::npos ) { uType |= unsigned(ignore::eTypeWildcard); } // if we have a wildcard then set the type to wildcard
         std::string string_( stringValue );
         std::replace(string_.begin(), string_.end(), '\\', '/');
         m_vectorIgnore.push_back( { uType, string_ } );
      }
   }
}

/// ---------------------------------------------------------------------------
/// Checks if the given file path matches any ignore pattern in m_vectorIgnore.
/// Normalizes the path to use forward slashes. Uses os_fnmatch for pattern matching.
/// Returns true if the path should be ignored, false otherwise.
bool CApplication::IGNORE_Match(const std::string_view& stringPath, const std::string_view& stringRoot) const
{                                                                                                  assert(stringPath.empty() == false); // Ensure the path is not empty
   auto normalize_ = [](std::string s_) -> std::string {
      std::replace(s_.begin(), s_.end(), '\\', '/');
      return s_;
      };

   std::string_view stringRoot_ = stringRoot;

   if( stringRoot_.empty() == true )
   {
      stringRoot_ = PROPERTY_Get("folder-current").as_string_view(); // if no root is given, use current folder
   }

   /// ## Generate paht that works like the project path
   //     This is used to match against ignore patterns
   std::string stringProjectPath;
   auto uRootLength = stringRoot_.length();
   if( stringRoot_.back() != '/' && stringRoot_.back() != '\\' ) uRootLength++;
   if( uRootLength < stringPath.length() )
   {
      // covert to lowercase ant match beginning of the path with the root
      std::string stringPathLower( stringPath );
      // convert root to lowercase and match beginning of the path with the root
      std::transform(stringPathLower.begin(), stringPathLower.end(), stringPathLower.begin(), ::tolower);

      std::string stringRootLower(stringRoot_);
      std::transform(stringRootLower.begin(), stringRootLower.end(), stringRootLower.begin(), ::tolower);

      if( stringPathLower.find(stringRootLower) == 0 )
      {
         stringProjectPath = stringPath.substr(uRootLength);                  // remove root from path
      }
      else
      {
         stringProjectPath = stringPath;                                      // if root is not found in path, use the whole path
      }
   }
   else
   {
      stringProjectPath = stringPath;                                         // if root is longer than path, use the whole path
   }

   stringProjectPath = normalize_(stringProjectPath);                         // normalize path to use forward slashes
   auto vectorFolder = gd::utf8::split(stringProjectPath, '/'); // split path into parts

   for( const auto& ignore_ : m_vectorIgnore )
   {
      std::string_view stringMatch = ignore_;

      if( ignore_.is_folder() == true )
      {
         if( ignore_.is_root() == true )
         {
            if( vectorFolder[0] == stringMatch ) return true;                  // if root is matched, ignore the path
         }
         else
         {  // match any folder in the path
            for( const auto& folder_ : vectorFolder )
            {
               if( folder_ == stringMatch ) return true;                       // if any part matches, ignore the path
            }
         }
      }
   }
   return false;
}

/** ---------------------------------------------------------------------------
 * @brief Checks if the given file name matches any ignore pattern in m_vectorIgnore.
 * Normalizes the file name to use forward slashes. Uses gd::ascii::strcmp for wildcard matching.
 * Returns true if the file name should be ignored, false otherwise.
 */
bool CApplication::IGNORE_MatchFilename(const std::string_view& stringFileName) const
{
   for( const auto& ignore_ : m_vectorIgnore )
   {
      if( ignore_.is_file() == true )
      {
         std::string_view stringMatch = ignore_;
         bool bMatch = gd::ascii::strcmp( stringFileName, stringMatch, gd::utf8::tag_wildcard{});
         if( bMatch == true ) return true;                                  // if file name matches the ignore pattern, ignore the path      }
      }
   }

   return false;
}



// 0TAG0Database.Application

#if 0
/** ---------------------------------------------------------------------------
 * @brief Open a database connection
 *
 * @param argumentsOpen The arguments used to open the database.
 * @param argumentsOpen.file The file name of the database.
 * @param argumentsOpen.create Whether to create the database if it doesn't exist (default: true).
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Open(const gd::argument::shared::arguments& argumentsOpen)
{                                                                                                  assert( argumentsOpen.empty() == false ); // Ensure the arguments are not empty
   DATABASE_CloseActive();

   bool bConnected = false;
   bool bCreate = true;
   if( argumentsOpen.exists("create") == true ) { bCreate = argumentsOpen["create"].as_bool(); }

   std::string stringPath = argumentsOpen["file"].as_string();
   if( stringPath.empty() == false )
   {
      gd::argument::arguments argumentsCreate({ {"file", stringPath}, { "create", bCreate} } );

      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();     // create database interface
      pdatabase->add_reference();
      auto result_ = pdatabase->open(argumentsCreate);
      if( result_.first == false )
      {
         pdatabase->release();
         return { false, result_.second };
      }
      m_pdatabase = pdatabase;
      bConnected = true;

   }
   return { bConnected, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Check if database needs to be updated and needed then update the database to the latest version
 *
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Update()
{                                                                                                  assert(m_pdatabase != nullptr); // Ensure the active database connection is valid
   // Check if the table "TVersion" exists
   uint64_t uVersion;
   gd::variant value_;
   auto result_ = m_pdatabase->ask("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='TVersion';", &value_ );
   if( result_.first == false ) { return { false, result_.second }; }
   uVersion = value_.as_uint64();

   if( uVersion < 1u )
   {
      result_ = DATABASE_Upgrade( uVersion );
      if( result_.first == false ) { return { false, result_.second }; }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Upgrade the database to the latest version
 *
 * @param uVersion The current version of the database
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Upgrade(uint64_t uVersion)
{
   std::string_view stringSql;

   if( uVersion == 0 )
   {
      stringSql = R"sql(
CREATE TABLE TVersion( VersionK INTEGER PRIMARY KEY, FVersion INTEGER, FMajor INTEGER, FMinor INTEGER, FBuild INTEGER, FRevision INTEGER );

CREATE TABLE TProject( ProjectK INTEGER PRIMARY KEY, TypeC INTEGER, StateC INTEGER, FName TEXT, FFolder VARCHAR(260), FDescription TEXT, FVersion INTEGER );

CREATE TABLE TFile( FileK INTEGER PRIMARY KEY, ProjectK INTEGER, TypeC INTEGER, FName TEXT, FSize INTEGER, FDescription TEXT );
CREATE TABLE TFileProperty(
   FilePropertyK INTEGER PRIMARY KEY, FileK INTEGER, ValueType INTEGER, FName TEXT, FValue TEXT, FDate REAL,
   FOREIGN KEY( FileK ) REFERENCES TFile( FileK ) ON DELETE CASCADE
);

CREATE TABLE TCodeGroup( CodeGroupK INTEGER PRIMARY KEY, FName TEXT, FDescription TEXT );
CREATE TABLE TCode(
   CodeK INTEGER PRIMARY KEY, CodeGroupK INTEGER, FName VARCHAR(100), FDescription TEXT,
   FOREIGN KEY( CodeGroupK ) REFERENCES TCodeGroup( CodeGroupK )
);

INSERT INTO TVersion( FVersion, FMajor, FMinor, FBuild, FRevision ) VALUES ( 1, 0, 0, 0, 0 );
INSERT INTO TProject( ProjectK, FName, FDescription, FVersion ) VALUES ( 1, 'demo', 'demo project', 1 );
)sql";
      auto result_ = m_pdatabase->execute(stringSql);
      if( result_.first == false ) { return { false, result_.second }; }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Connect to database
 * Connect to database based on string that holds information on how to connect
 *
 * @param stringConnect connection string
 * @return true if ok, false and error information if failed
 */
std::pair<bool, std::string> CApplication::DATABASE_Connect( const std::string_view& stringConnect )
{
   std::string stringConnectionName;
   std::string stringConnectionName2;

   DATABASE_CloseActive();

   if( stringConnect.empty() == true )
   {
      stringConnectionName = PROPERTY_Get("database").as_string();             assert( stringConnectionName.empty() == false );
   }
   else
   {
      stringConnectionName = stringConnect;
   }

   // ## connect main database
   bool bOk = false;
   std::string stringError;
   gd::database::database_i* pdatabaseMain = nullptr;


   if(true)
   {
#ifdef GD_DATABASE_ODBC_USE
      gd::database::odbc::database_i* pdatabase = new gd::database::odbc::database_i();  // create database interface
      pdatabase->add_reference();
      std::tie(bOk, stringError) = pdatabase->open( stringConnectionName );
      pdatabaseMain = pdatabase;
#else
      bOk = false;
      stringError = "Only file connection is enabled, make sure to connect to file database (sqlite)";
#endif
   }
   else
   {
      gd::database::sqlite::database_i* pdatabase = new gd::database::sqlite::database_i();  // create database interface
      pdatabase->add_reference();
      std::tie(bOk, stringError) = pdatabase->open( stringConnectionName );
      pdatabaseMain = pdatabase;
   }

   if( bOk == true )
   {
      m_pdatabase = pdatabaseMain;
   }
   else
   {
      if( pdatabaseMain != nullptr ) pdatabaseMain->release();
      if( stringError.empty() == true ) stringError = "Failed to collect information about database connection";
      return { false, stringError };
   }

   return { true, "" };
}

void CApplication::DATABASE_Append( gd::database::database_i* pdatabase, bool bActivate )
{                                                                                                  assert( pdatabase != nullptr );
   pdatabase->add_reference();
   m_vectorDatabase.push_back( pdatabase );
   if( bActivate == true )
   {
      if( m_pdatabase != nullptr ) m_pdatabase->release();
      m_pdatabase = pdatabase;
      if( m_pdatabase != nullptr ) m_pdatabase->add_reference();
   }
}


/** ---------------------------------------------------------------------------
 * @brief Close active database
*/
void CApplication::DATABASE_CloseActive()
{
   if( m_pdatabase != nullptr )
   {
      m_pdatabase->release();
      m_pdatabase = nullptr;
   }
}
#endif // 0

// 0TAG0Settings.Application @TAG #settings.Application

/*
@TASK #configuration.load #user.per [name: config]
--
[description: "## Load application configuration file into table used to store configuration in application table used for this." ]
[priority: high] [state: open] [owner: per]
[idea: "Method for loading configuration is called `CONFIG_Load'."]
*/


std::pair<bool, std::string> CApplication::CONFIG_Load(const std::string_view& stringFileName)
{                                                                                                 assert( (bool)m_ptableConfig == false );
   using namespace jsoncons;
   using namespace gd::table;

   if( m_ptableConfig != nullptr ) return { true, "" }; // If config table is already set, return success

   constexpr std::string_view stringConfigurationFileName = "cleaner-configuration.json"; // Default configuration file name

   std::string stringFolder( stringFileName );


   // ## Try to find local configuration file


   if( stringFolder.empty() == true )
   {
      stringFolder = PROPERTY_Get("folder-home").as_string();                 // Get home folder from properties
   }

   if( stringFolder.empty() == true ) return { false, "No home folder set" }; // If no home folder is set, return error

   // ## Prepare configuration path
   gd::file::path pathConfiguration(stringFolder);
   if( pathConfiguration.has_extension() == false ) { pathConfiguration += stringConfigurationFileName; } // Add filename if not provided

   if( std::filesystem::exists(pathConfiguration) == false ) { return { false, std::format("configuration file not found: {}", pathConfiguration.string()) }; } // Check if configuration file exists

   // ## Load settings from file
   try
   {
      // Create a new config table with the path to the configuration file
      m_ptableConfig = std::make_unique<table>( table( table::eTableFlagNull32, {{"rstring", 0, "group"}, {"rstring", 0, "name"}, {"rstring", 0, "value"}, {"string", 6, "type"} }, tag_prepare{} ) );
      // Open the JSON file
      std::ifstream ifstreamJson(pathConfiguration);
      if( ifstreamJson.is_open() == false ) { return { false, std::format("Failed to open configuration file: {}", pathConfiguration.string()) }; } // Check if file opened successfully

      // Parse JSON data from the file into a json object
      json jsonDocument = json::parse( ifstreamJson );

      // ## Iterate through the JSON object and populate the config table
      for( const auto& keyvalueRoot : jsonDocument.object_range() )
      {
         if( keyvalueRoot.value().is_object() == false ) continue;            // Skip if value is not an object

         std::string stringKey = keyvalueRoot.key();

         // ## split string between group and name, splitting by '.' and set stringGroup and stringName

         auto vectorSplit = gd::utf8::split(stringKey, '.');                  // Split the string by '.'
         if( vectorSplit.size() < 2 ) continue;                               // Skip if less than 2 parts after splitting

         auto stringCleaner = vectorSplit[0];                                 // First part is the 'cleaner'  marker, all config values belongs to key objects where first part is "cleaner"

         if( stringCleaner != "cleaner" ) continue;                           // Skip if group is not "cleaner"

         auto stringGroup = vectorSplit[1];                                   // Second part is the group name
         for( const auto& value_ : keyvalueRoot.value().object_range() )
         {
            if( value_.value().is_null() ) continue;                          // Skip if value is null
            auto stringName = value_.key();
            auto stringValue = value_.value().as_string();                    // Convert JSON value to string
            auto uRow = m_ptableConfig->row_add_one();
            m_ptableConfig->row_set(uRow, { stringGroup, stringName, stringValue }); // Set value, each value is store as string and belongs to group and name
         }
      }

      // Use the json object (e.g., print it)
      //std::cout << pretty_print(j) << std::endl;
   }
   catch (const std::exception& e)
   {
      std::string stringError = std::format("Error: {}", e.what());
      return { false, stringError };
   }

   return { true, "" }; // Placeholder for settings loading logic
}

/** ---------------------------------------------------------------------------
 * @brief Get configuration value from the config table
 *
 * @param stringGroup The group name of the configuration
 * @param stringName The name of the configuration
 * @return gd::variant_view The value of the configuration, or an empty variant view if not found
 */
gd::variant_view CApplication::CONFIG_Get(std::string_view stringGroup, std::string_view stringName) const
{
   if( m_ptableConfig == nullptr ) return gd::variant_view(); // If no config table is set, return empty variant view

   auto iRow = m_ptableConfig->find({ {"group", stringGroup }, {"name", stringName} }); // Find the row with the specified group and name

   if( iRow != -1 )                                                           // If row is found
   {
      auto value_ = m_ptableConfig->cell_get_variant_view(iRow, "value");     // Return the value from the found row
      return value_;
   }

   return gd::variant_view();
}

/** ---------------------------------------------------------------------------
 * @brief Check if configuration exists in the config table
 *
 * @param stringGroup The group name of the configuration
 * @param stringName The name of the configuration
 * @return bool True if the configuration exists, false otherwise
 */
bool CApplication::CONFIG_Exists( std::string_view stringGroup, std::string_view stringName ) const
{
   if( m_ptableConfig == nullptr ) return false;

   auto iRow = m_ptableConfig->find({ {"group", stringGroup }, {"name", stringName} }); // Find the row with the specified group and name
   if( iRow != -1 ) { return true; }

   return false;
}

/** --------------------------------------------------------------------------- @TAG #help.Application
 * @brief Print documentation for command-line options
 *
 * This function generates documentation for the provided command-line options
 * and appends it to the given string.
 *
 * @param poptions Pointer to the command-line options object
 * @param stringDocumentation Reference to the string where documentation will be appended
 * @return std::pair<bool, std::string> True if successful, false and error message if failed
 */
void CApplication::HELP_PrintDocumentation( const gd::cli::options* poptions, std::string& stringDocumentation)
{                                                                                                  assert( poptions != nullptr );
   using namespace gd::cli; // use namespace for options
   std::string stringFlags; // temporary string to hold flags

   if( poptions->exists( "commands", gd::types::tag_state_active{}) == true)
   {
      stringDocumentation += std::format("{:<12} {}\n", "COMMAND", "DESCRIPTION");
      stringDocumentation += std::format("{:<12} {}\n", "-------", "-----------");
      poptions->print_documentation([this, &stringDocumentation, &stringFlags](auto uType, auto stringName, auto stringDescription, const auto* poption_, const auto* poptions_) -> void {
         if (uType == options::eOptionTypeCommand)
         {
            if (stringName.empty() == true) { return; }                          // skip empty command names

            stringDocumentation += gd::console::rgb::print(CONFIG_Get("color", { "header", "default" }).as_string(), gd::types::tag_color{});
				stringDocumentation += std::format("{:<12} {}", stringName, stringDescription);
            stringDocumentation += "\n";
         }
      });

		return;                                                                     // if we have commands then we do not print options and flags
   }

   poptions->print_documentation([this,&stringDocumentation, &stringFlags](auto uType, auto stringName, auto stringDescription, const auto* poption_, const auto* poptions_) -> void {
      if( poptions_->get_parent() == nullptr ) { return; }                      // skip globals

      if( uType == options::eOptionTypeCommand )
      {
         if( stringName.empty() == true ) { return; }                          // skip empty command names

         stringDocumentation += gd::console::rgb::print( CONFIG_Get("color", { "header", "default" }).as_string(), gd::types::tag_color{});
         stringDocumentation += "\n\n"; // add newline to description
         stringDocumentation += gd::math::string::format_header_line(stringName, 80); // format header line for command name
         stringDocumentation += "\n";
         stringDocumentation += gd::math::string::format_indent(stringDescription, 2, true); // indent description
         stringDocumentation += "\n\n";
      }
      else if( (uType & options::eOptionTypeOption) == options::eOptionTypeOption )
      {
         // pad to 18 characters
         stringDocumentation += gd::console::rgb::print( CONFIG_Get("color", { "body", "default" }).as_string(), gd::types::tag_color{});
         std::string string_ = std::format("- {:.<16}: ", stringName );
         stringDocumentation += string_;
         string_ = stringDescription;
         if( ( uType & options::eOptionTypeFlag ) == options::eOptionTypeFlag ) { string_ += " (flag)"; } // if flag then add to description
         string_ = gd::math::string::format_text_width( string_, 60 );
         string_ = gd::math::string::format_indent( string_, 20, false );
         stringDocumentation += string_;
         stringDocumentation += "\n";
      }
      else if( uType == options::eOptionTypeFlag )
      {
         // pad to 18 characters
         stringFlags += gd::console::rgb::print( CONFIG_Get("color", { "body", "default" }).as_string(), gd::types::tag_color{});
         std::string string_ = std::format("- {:.<16}: ", stringName );
         stringFlags += string_;
         string_ = gd::math::string::format_text_width( stringDescription, 60 );
         string_ = gd::math::string::format_indent( string_, 20, false );
         stringFlags += string_;
         stringFlags += "\n";
      }
      else if( uType == 0 )
      {
         if( stringFlags.empty() == true ) return; // if no flags then skip

         stringDocumentation += "\nFlags\n";
         stringDocumentation += stringFlags;
         stringFlags.clear();
      }
   });

   // ## globals
   poptions->print_documentation([this,&stringDocumentation, &stringFlags](auto uType, auto stringName, auto stringDescription, const auto* poption_, const auto* poptions_) -> void {
      if( poptions_->get_parent() != nullptr ) { return; }                      // skip subcommands
      if( poptions_->name().empty() == false ) { return; }                       // skip if name

      if( uType == options::eOptionTypeCommand )
      {
         if( stringName.empty() == false ) { return; }                          // skip command names

         stringDocumentation += gd::console::rgb::print( CONFIG_Get("color", { "header", "default" }).as_string(), gd::types::tag_color{});
         stringDocumentation += "\n\n"; // add newline to description
         stringDocumentation += gd::math::string::format_header_line("GLOBALS", 80, '#', '=', '#'); // format header line
         stringDocumentation += "\n\n";
      }
      else if( (uType & options::eOptionTypeOption) == options::eOptionTypeOption )
      {
         // pad to 18 characters
         stringDocumentation += gd::console::rgb::print( CONFIG_Get("color", { "body", "default" }).as_string(), gd::types::tag_color{});
         std::string string_ = std::format("- {:.<16}: ", stringName );
         stringDocumentation += string_;
         string_ = stringDescription;
         if( ( uType & options::eOptionTypeFlag ) == options::eOptionTypeFlag ) { string_ += " (flag)"; } // if flag then add to description
         string_ = gd::math::string::format_text_width( string_, 60 );
         string_ = gd::math::string::format_indent( string_, 20, false );
         stringDocumentation += string_;
         stringDocumentation += "\n";
      }
      else if( uType == options::eOptionTypeFlag )
      {
         // pad to 18 characters
         stringFlags += gd::console::rgb::print( CONFIG_Get("color", { "body", "default" }).as_string(), gd::types::tag_color{});
         std::string string_ = std::format("- {:.<16}: ", stringName );
         stringFlags += string_;
         string_ = gd::math::string::format_text_width( stringDescription, 60 );
         string_ = gd::math::string::format_indent( string_, 20, false );
         stringFlags += string_;
         stringFlags += "\n";
      }
      else if( uType == 0 )
      {
         if( stringFlags.empty() == true ) return; // if no flags then skip

         stringDocumentation += "\nFlags\n";
         stringDocumentation += stringFlags;
         stringFlags.clear();
      }
   });
}


/** ---------------------------------------------------------------------------
 * @brief Get configuration value from the config table using a list of names
 *
 * This function searches for the specified configuration names within the given group
 * and returns the value of the first found configuration.
 *
 * @param stringGroup The group name of the configuration
 * @param listName A list of names to search for in the configuration
 * @return gd::variant_view The value of the configuration, or an empty variant view if not found
 *
 * @code
 * auto value_ = application_.CONFIG_Get("color", { "header", "default" });
 * @endcode
 */
gd::variant_view CApplication::CONFIG_Get( std::string_view stringGroup, const std::initializer_list<std::string_view> listName ) const
{
   if( m_ptableConfig == nullptr ) return gd::variant_view(); // If no config table is set, return empty variant view

   for( const auto& stringName : listName )
   {
      auto iRow = m_ptableConfig->find({ {"group", stringGroup }, {"name", stringName} }); // Find the row with the specified group and name
      if( iRow != -1 )                                                           // If row is found
      {
         auto value_ = m_ptableConfig->cell_get_variant_view(iRow, "value");     // Return the value from the found row
         return value_;
      }
   }
   return gd::variant_view();
}


/** ---------------------------------------------------------------------------
* @brief Add error to internal list of errors
* @param stringError error information
*/
void CApplication::ERROR_Add( const std::string_view& stringError )
{
   std::unique_lock<std::shared_mutex> lock_( m_sharedmutexError ); // locks `m_vectorError`
   gd::argument::arguments argumentsError( { {"text", stringError} }, gd::argument::arguments::tag_view{});
   m_vectorError.push_back( std::move(argumentsError) );
}

/** ---------------------------------------------------------------------------
 * @brief Get error information
 *
 * If no errors then empty string is returned
 * @return std::string error information
 */
std::string CApplication::ERROR_Report() const
{
   if( m_vectorError.empty() == false )
   {
      std::string stringError;
      for( const auto& error_ : m_vectorError )
      {
         stringError += error_.print();
         stringError += "\n";
      }
      return stringError;
   }

   return std::string();
}


/** --------------------------------------------------------------------------
 * @brief Converts a string representation of the UI type to the corresponding enum value.
 *
 * This static method takes a string view representing the UI type and returns the corresponding
 * enumUIType value. If the string does not match any known UI type, it returns eUITypeUnknown.
 *
 * @param stringUIType The string representation of the UI type (e.g., "console", "web", "vs").
 * @return enumUIType The corresponding enum value for the UI type.
 */
CApplication::enumUIType CApplication::GetUITypeFromString_s(const std::string_view& stringUIType)
{
   if( stringUIType == "console" )   return eUITypeConsole;
   if( stringUIType == "web" )       return eUITypeWeb;
   if( stringUIType == "wimp" )      return eUITypeWIMP;
   if( stringUIType == "vscode" )    return eUITypeVSCode;
   if( stringUIType == "vs" )        return eUITypeVS;
   if( stringUIType == "sublime" )   return eUITypeSublime;

   return eUITypeUnknown; // Default to none if no match found
}

bool CApplication::IsDetailLevel_s(uint32_t uDetailLevel, const std::string_view& stringDetailLevel)
{                                                                                                  assert(stringDetailLevel.empty() == false);
   if(stringDetailLevel[0] == 'B' || stringDetailLevel == "1") 		         // "BASIC"
   {
      if( uDetailLevel <= 1 ) return true;
   }
	else if(stringDetailLevel[0] == 'S' || stringDetailLevel == "2") 		      // "STANDARD"
   {
      if( uDetailLevel <= 2 ) return true;
   }
	else if(stringDetailLevel[0] == 'E' || stringDetailLevel == "3") 		      // "EXTENDED"
   {
      if( uDetailLevel <= 3 ) return true;
   }
   else if( stringDetailLevel[0] == 'F' || stringDetailLevel == "4")          // "FULL"
   {
      if( uDetailLevel <= 4 ) return true;
   }
   else
   {
      try
      {
         auto uValue = std::stoul(stringDetailLevel.data());
         if( uValue == uDetailLevel ) return true;
      }
      catch(...) {}
   }
   return false;
}

// 0TAG0Options.Application


/** ---------------------------------------------------------------------------  @API [tag: cli, application, options ] [summary: prepare application options]
 * @brief Prepares the application options for command-line usage.
 *
 * This method sets up the available command-line options for the application,
 * including global options and subcommands. Each subcommand is configured
 * with its specific options and descriptions.
 *
 * @param optionsApplication A reference to the `gd::cli::options` object
 *                           where the options and subcommands will be added.
 *
 * ### Global Options
 * - `editor`        : For editor specific configuration. vs, vscode or sublime is currently supported.
 * - `logging`       : Enables logging.
 * - `logging-csv`   : Adds a CSV logger for log messages.
 * - `print`         : Prints results from commands.
 * - `recursive`     : Specifies recursive operations with a depth value.
 * - `output`        : Saves output to a specified file.
 * - `database`      : Sets the folder for log files.
 * - `statements`    : Specifies a file containing SQL statements.
 */
void CApplication::Prepare_s(gd::cli::options& optionsApplication)
{
   optionsApplication.add_flag({"logging", "Turn on logging"});
   optionsApplication.add_flag({"logging-csv", "Add csv logger, prints log information using the csv format"});
   optionsApplication.add_flag({ "help", "Prints help information about command" });
   optionsApplication.add_flag({ "history", "Add active command to history" });
   optionsApplication.add_flag({ "print", "Results from command should be printed" });
   optionsApplication.add_flag({ "verbose", "Write information about operations that might be useful for user" });
   optionsApplication.add_flag({ "icase", "Ignore case when matching patterns" });
   optionsApplication.add_flag({ "word", "Match whole words only when patterns are used" });
   optionsApplication.add({ "config", "specify configuration file to use configuring cleaner" });
   optionsApplication.add({ "editor", "type of editor, vs or vscode is currently supported" });
   optionsApplication.add({ "add-to-history", "Add to history with alias name" });
   optionsApplication.add({ "logging-severity", "Set the logging severity level. Available levels: `verbose`, `debug`, `info`, `warning`, `error`, `fatal`."});
   optionsApplication.add({ "mode", "Specifies the operational mode of the tool, adapting its behavior for different code analysis purposes. Available modes: `review`, `stats`, `search`, `changes`, `audit`, `document`" });
   optionsApplication.add({ "recursive", "Operation should be recursive, by settng number decide the depth" });
   optionsApplication.add({ "output", "Save output to the specified file. Overwrites the file if it exists. Defaults to stdout if not set."});
   optionsApplication.add({ "prompt", "Prompts for values that is typed before execute expression, these values will be asked for"});
   optionsApplication.add_flag_or_option({ "detail", "Set detail level on information presented to user. levels are basic, standard, extended, full or 0,1,2,3. If detail set as flag then standard is used." });

   {  // ## `count` command, copies file from source to target
      gd::cli::options optionsCommand( 0, "count", "Count patterns or lines and segments in selected files" );
      optionsCommand.add({ "filter", "Filter to apply (wildcard file name matching). If empty, all found text files are counted" });
      optionsCommand.add({ "pattern", 'p', "patterns to search for, multiple values are separated by , or ;"});
      optionsCommand.add({ "source", 's', "File(s) or folder(s) to count lines in"});
      optionsCommand.add({ "rpattern", "Use a **regular expression pattern** to search for more complex text matches within file content."});
      optionsCommand.add({ "ignore", "Folder(s) to ignore searching for files"});
      optionsCommand.add({ "segment", "type of segment in code to search in"});
      optionsCommand.add({ "page", "Index for page to print and if page-size is not set then default page-size is 10" });
      optionsCommand.add({ "page-size", "Max number of rows in each page" });
      optionsCommand.add({ "sort", "Sorts result on selected column name" });
      optionsCommand.add({ "stats", "Add statistics to generated output" });
      optionsCommand.add({ "table", "Table is used based on options set, for example generating sql insert queries will use table name to insort to" });
      optionsCommand.add({ "where", "Specify conditions for filtering file names in result." });
      optionsCommand.add_flag( {"R", "Set recursive to 16, simple to scan all subfolders"} );
#ifdef _WIN32
      optionsCommand.add_flag( {"vs", "Adapt to visual studio output window format, make files clickable"} );
      optionsCommand.add_flag( {"win", "Windows specific functionality, logic might be using some special for adapting to features used for windows"} );
#endif
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `config` command, manage configuration file
      gd::cli::options optionsCommand( 0, "config", "Manage configuration" );
      optionsCommand.add_flag({"create", "Create configuration file if it doesn't exist"});
      optionsCommand.add_flag({"edit", "Edit configuration file if it exists"});
      optionsCommand.add_flag({"local", "Create configuration file in current directory"});
      optionsCommand.add_flag({"backup", "Create a backup copy of the configuration file"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   {  // ## `copy` command, count number of lines in file
      gd::cli::options optionsCommand( 0, "copy", "Copy file or selected files from source to target" );
      optionsCommand.add({ "source", 's', "File or files to copy, if many files then a tip is to set filter with --filter and folders in source" });
      optionsCommand.add({ "target", 't', "Destination, where file is copied to" });
      optionsCommand.add({ "filter", "Specify a **wildcard filter** (e.g., `*.txt`, `database.*`) to match file names. Multiple filters can be separated with semicolons (`;`). If no filter is provided, all files in the directory are listed." });
      optionsCommand.add({ "pattern", 'p', "Provide one or more **patterns to search for** within file content. Separate multiple patterns with semicolons (`;`)." });
      optionsCommand.add({ "rpattern", "Use a **regular expression pattern** to search for more complex text matches within file content." });
      optionsCommand.add({ "ignore", "Provide one or more **folder names to exclude** from the listing. Multiple folder names can be separated with semicolons (`;`). This helps exclude irrelevant directories." });
      optionsCommand.add({ "backup", "If destination file exits then make a backup"});
      optionsCommand.add({ "newer", "Only copy files that are newer if target file is found" });
      optionsCommand.add({ "segment", "type of segment in code to search in"});
      optionsCommand.add({ "where", "Specify conditions for filtering file names in result." });
      optionsCommand.add_flag({ "R", "Set recursive to 16, simple to scan all subfolders" });
      optionsCommand.add_flag({ "overwrite", 'o', "Overwrite files existing files"});
      optionsCommand.add_flag({ "preview","Show preview of changes without applying them"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   /*
   {  // ## `db` command, use database and configure settings for that
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "db", "Configure database" );
      optionsCommand.add({"file", 'f', "Where to place database file (used for sqlite databases)"});
      optionsCommand.add({"settings", "Where to write configuration file"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }
   */


   { // ## 'dir' command, list files
      gd::cli::options optionsCommand( 0, "dir", "List selected files, lots of filtering options to select what to list." );
      optionsCommand.add({ "filter", "Specify a **wildcard filter** (e.g., `*.txt`, `database.*`) to match file names. Multiple filters can be separated with semicolons (`;`). If no filter is provided, all files in the directory are listed." });
      optionsCommand.add({ "pattern", 'p', "Provide one or more **patterns to search for** within file content. Separate multiple patterns with semicolons (`;`)."});
      optionsCommand.add({ "source", 's', "Specify the **directory to begin searching** for files. This is the starting point for all file operations. Multiple directories are separated with semicolons (`;`)" });
      optionsCommand.add({ "rpattern", "Use a **regular expression pattern** to search for more complex text matches within file content."});
      optionsCommand.add({ "ignore", "Provide one or more **folder names to exclude** from the listing. Multiple folder names can be separated with semicolons (`;`). This helps exclude irrelevant directories." });
      optionsCommand.add({ "segment", "Limit the search to specific **types of code segments**, such as functions, classes, or comments. This refines your search to relevant code blocks. Valid segments are `code`, `string` or `comment`."});
      optionsCommand.add({ "script", "Execute an **external script file** for advanced processing of the listed files. Useful for custom formatting or filtering." });
      optionsCommand.add({ "sort", "Sort the listed files based on a **specified column name** (e.g., name, size, date). This organizes the output for easier analysis." });
      optionsCommand.add({ "where", "Specify conditions for filtering file names in result." });
      optionsCommand.add_flag_or_option( {"parents", "Adds parent folders to file name when listing."} );
      optionsCommand.add_flag_or_option( {"R", "Enable **recursive listing** of files in subfolders. Sets the recursion depth to 16, ensuring all subdirectories are scanned."} );
      optionsCommand.add_flag({ "compact", "View results in compact format, similar to ls on linux" });
#ifdef _WIN32
      optionsCommand.add_flag( {"vs", "Format the output to be compatible with the **Visual Studio Output window**, enabling seamless integration with the IDE."} );
      optionsCommand.add_flag( {"win", "Enable **Windows-specific functionality**, adapting the listing behavior to leverage Windows operating system features."} );
#endif
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add(std::move(optionsCommand));
   }

   { // ## 'find' command, list files
      gd::cli::options optionsCommand( 0, "find", "Search patterns in files and all filecontent is searched in, this enables multiline patterns" );
      optionsCommand.add({ "filter", "Specify a **wildcard filter** (e.g., `*.txt`, `*.cpp`) to apply when searching for files. Multiple filters are separated with ;. If no filter is provided, all found text files will be searched for patterns." });
      optionsCommand.add({ "pattern", 'p', "Provide one or more **patterns to search for** within file content. Separate multiple patterns with semicolons (`;`)."});
      optionsCommand.add({ "source", 's', "Specify the **directory to begin searching** for files. This is the starting point for all file operations. Multiple directories are separated with semicolons (`;`)" });
      optionsCommand.add({ "rpattern", "Use a **regular expression pattern** to search for more complex text matches within file content."});
      optionsCommand.add({ "kv", "A **shortcut** to define both the keys to search for and the rules for how to find their corresponding values. Use this for quick key-value pair extraction."});
      optionsCommand.add({ "keys", "Specify individual **keys to search for** when collecting associated values. Useful when you need to extract specific data points."});
      optionsCommand.add({ "header", "select columns or keys to include in the output as header."});
      optionsCommand.add({ "footer", "select columns or keys to include in the output as footer."});
      optionsCommand.add({ "brief", "Enable brief output format for key-value pairs. Based on output format this varies but generally shows a condensed view to simplify understanding."});
      optionsCommand.add({ "kv-format", "Define the **scoping format** for how key-value pairs are identified and extracted. This helps the tool understand the structure of your key-value data."});
      optionsCommand.add({ "kv-where", "Specify conditions for filtering key-value pairs. This allows for more precise extraction based on specific criteria." });
      optionsCommand.add({ "context", "Display **surrounding code or text** to provide context for each search result. This helps you understand where the match occurred."});
      optionsCommand.add({ "ignore", "Provide one or more **folder names to exclude** from the search. This helps narrow down your search and improve performance."});
      optionsCommand.add({ "segment", "Limit the search to specific **types of code segments**, such as functions, classes, or comments. This refines your search to relevant code blocks. Valid segments are `code`, `string` or `comment`."});
      optionsCommand.add({ "rule", "Define **rules for what actions to perform** on found matches. This could include formatting, outputting, or further processing."});
      optionsCommand.add({ "script", "Execute an **external script file** for advanced and custom processing of search results. Ideal for complex automation." });
      optionsCommand.add({ "max", "Set the **maximum number of results** to return. Use this to limit output and improve performance for large searches."});
      optionsCommand.add({ "width", "Width for output" });
      optionsCommand.add_flag({ "R", "Enable **recursive search** in subfolders. Sets the recursion depth to 16, ensuring a thorough scan of all subdirectories." });
      optionsCommand.add_flag( {"match-all", "Require **all specified patterns to match** within the same line or row for a result to be considered valid."} );
      optionsCommand.add_flag({ "clip", "Investigate clipboard for related information (file path or search value)" });
#ifdef _WIN32
      optionsCommand.add_flag( {"vs", "Format the output to be compatible with the **Visual Studio Output window**, making it easier to navigate results within the IDE."} );
      optionsCommand.add_flag( {"win", "Activate **Windows-specific functionality**, adapting the tool's behavior and features to the Windows operating system."} );
#endif
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add(std::move(optionsCommand));
   }


   // ## 'history' handle history
   {
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "history", "Handle command history" );
      optionsCommand.add({ "run", "Run history entry, this will run the command from history" });
      optionsCommand.add({ "index", "Specify history entry by index" });
      optionsCommand.add({ "set-alias", "Set alias for history entry" });
      optionsCommand.add_flag_or_option({ "list", "Lists all history entries" });
      optionsCommand.add({ "remove", "Remove history entries" });
      optionsCommand.add({ "width", "Width for output" });
      optionsCommand.add_flag( {"create", "Initialize history logic, creates folders and files needed to manage history, this also enables configuration settings"} );
      optionsCommand.add_flag({ "delete", "Delete history file with saved commands" });
      optionsCommand.add_flag({ "print", "Print history, this will print all of the history entries" });
      optionsCommand.add_flag({ "edit", "Edit history file if it exists" });
      optionsCommand.add_flag({ "local", "Create history file in current directory" });
      optionsCommand.add_flag({ "home", "Create history file in user home directory" });
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent ), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add(std::move(optionsCommand));
   }

   { // ## 'list' command, list rows with specified patterns @TAG #options.list
      gd::cli::options optionsCommand( 0, "list", "Search files and list lines matching specified patterns. Searches are performed line-by-line within files." );
      optionsCommand.add({ "filter", "Filter files by name using **wildcard patterns** (e.g., `*.cpp`, `test*`). Multiple patterns can be separated by semicolons (`;`). If omitted, all files are processed." });
      optionsCommand.add({ "pattern", 'p', "**Search patterns** to find in file content. Multiple patterns can be separated by commas (`,`) or semicolons (`;`). Each line is checked for matches." });
      optionsCommand.add({ "source", 's', "Specify the **file(s) or folder(s)** to search for matching rows. This is the starting point for the search operation. Multiple sources are split with (`;`)." });
      optionsCommand.add({ "ignore", "Provide one or more **folder names to exclude** from the search. Multiple folder names can be separated with semicolons (`;`). This helps narrow down the search scope." });
      optionsCommand.add({ "rpattern", "**Regular expression pattern** for advanced text matching. Supports complex pattern matching using regex syntax." });
      optionsCommand.add({ "context", "Show **context lines** around matches. Displays surrounding code/text to help understand the match location within the file. One or two integer numbers." });
      optionsCommand.add({ "expression", 'e', "Provide an **inline script expression** for advanced customization of search results. This enables non-standard functionality and complex processing." });
      optionsCommand.add({ "script", "Execute an **external script file** for advanced and custom processing of matched rows. Ideal for complex automation tasks." });
      optionsCommand.add({ "max", "**Maximum results** to return. Limits the number of matching lines output to improve performance in large searches." });
      optionsCommand.add({ "segment", "Limit the search to specific **types of code segments**, such as `code`, `comment`, `string`, or `all`. This refines the search to relevant parts of the file." });
      optionsCommand.add_flag( {"R", "Enable **recursive scanning** of all subfolders. Sets the recursion depth to 16, ensuring a thorough search of subdirectories."} );
      optionsCommand.add_flag( {"match-all", "Require **all specified patterns** to match within the same row for it to be included in the results."} );
      optionsCommand.add_flag({ "clip", "Investigate clipboard for related information (file path or search value)" });
#ifdef _WIN32
      optionsCommand.add_flag( {"vs", "Format the output to be compatible with the **Visual Studio Output window**, making file references clickable for easy navigation in the IDE."} );
      optionsCommand.add_flag( {"win", "Enable **Windows-specific functionality**, adapting the tool's behavior to leverage Windows operating system features."} );
#endif
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add(std::move(optionsCommand));
   }

   /*
   {  // ## `join` command, joins two or more files
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "join", "join two or more files" );
      optionsCommand.add({"source", 's', "Files to join"});
      optionsCommand.add({"destination", 'd', "Destination, joined files result"});
      optionsCommand.add({"backup", 'b', "If destination file exits then make a backup"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }
   */

   {  // ## `paste` checks the clipboard for text or input file reading arguments
      gd::cli::options optionsCommand( gd::cli::options::eFlagUnchecked, "paste", "Paste text from clipboard or read from input file" );
      optionsCommand.add({"source", 's', "Files to join"});
      optionsCommand.set_flag( (gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0 );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `help` print help about @TAG #options.help
      gd::cli::options optionsCommand( "help", "Print command line help" );
      optionsCommand.add_flag({ "commands", "List all available commands without detailed descriptions" });
      optionsCommand.set_flag((gd::cli::options::eFlagSingleDash | gd::cli::options::eFlagParent), 0);
      optionsCommand.parent(&optionsApplication);
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   {  // ## `run` print current version
      gd::cli::options optionsCommand( "run", "Run command from loaded command templates" );
      optionsCommand.add({"name", "Name or index for command to execute"});
      optionsCommand.add({"list", "List command found in loaded settings"});
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }


   {  // ## `version` print current version
      gd::cli::options optionsCommand( "version", "Print version" );
      optionsApplication.sub_add( std::move( optionsCommand ) );
   }

   // ## Prepar aliases for commands
   optionsApplication.alias_add("ls", { { "command", "dir" }, { "compact", true } } );
   optionsApplication.alias_add("cp", { { "command", "copy" } } );
   optionsApplication.alias_add("hi", { { "command", "history" } } );
}

// ----------------------------------------------------------------------------
/// @brief Prepare console for command line usage
std::pair<bool, std::string> CApplication::Prepare_s(gd::console::console* pconsole)
{
   auto result_ = pconsole->initialize();
   return result_;
}

// ----------------------------------------------------------------------------
/// @brief Prepare logging
void CApplication::PrepareLogging_s()
{
#ifdef GD_LOG_SIMPLE
   using namespace gd::log;
   gd::log::logger<0>* plogger = gd::log::get_s();                            // get pointer to logger 0

   plogger->append( std::make_unique<gd::log::printer_console>() );           // append printer to logger, this prints to console

   // ## set margin for log messages, this to make it easier to read. a bit hacky
   auto* pprinter_console = (gd::log::printer_console*)plogger->get( 0 );
   // ## color console messages in debug mode
   pprinter_console->set_margin( 8 );                                         // set log margin
   pprinter_console->set_margin_color( eColorBrightBlack );

   unsigned uSeverity = unsigned(eSeverityError);
   plogger->set_severity( uSeverity );

#ifndef NDEBUG
   uSeverity = unsigned(eSeverityDebug);                                      // set debug severity
   plogger->set_severity(uSeverity);                                          // set debug severity in debug mode
#endif

#endif // GD_LOG_SIMPLE
}



/** --------------------------------------------------------------------------- @API [tag: "code, config, rule" ] [description: "Language rules, what is what in different languages, things like string, comment and code"]
 * @brief Prepares the state for parsing based on the file extension.
 * @param argumentsPath The arguments containing the source path for harvesting files.
 * @param state_ The state object to be prepared.
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> CApplication::PrepareState_s(const gd::argument::shared::arguments& argumentsPath, gd::expression::parse::state& state_)
{
   std::string stringFile = argumentsPath["source"].as_string();                                   assert(stringFile.empty() == false);

   gd::file::path pathFile(stringFile);
   std::string stringExtension = pathFile.extension().string();

   // convert string to lowercase
   std::transform(stringExtension.begin(), stringExtension.end(), stringExtension.begin(), ::tolower);

   if( stringExtension.length() < 2 ) return { false, "File extension is too short: " + stringExtension };

   if( stringExtension[1] == 'c' || stringExtension[1] == 'h' || stringExtension[1] == 'i' )
   {
      if( stringExtension == ".cpp" || stringExtension == ".c" || stringExtension == ".cc" || stringExtension == ".cxx" || stringExtension == ".h" || stringExtension == ".hpp" || stringExtension == ".hxx" || stringExtension == ".ipp" )
      {
         state_.add(std::string_view("LINECOMMENT"), "//", "\n");
         state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
         state_.add(std::string_view("STRING"), "\"", "\"", "\\");
         state_.add(std::string_view("RAWSTRING"), "R\"(", ")\"");
         return { true, "" };
      }
   }

   if( stringExtension == ".cs" || stringExtension == ".fs" || stringExtension == ".kt" || stringExtension == ".swift" )
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
   else if( stringExtension == ".js" || stringExtension == ".ts" || stringExtension == ".tsx" || stringExtension == ".jsx" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
      state_.add(std::string_view("RAWSTRING"), "`", "`");

      if( stringExtension == ".jsx" || stringExtension == ".tsx" ) { state_.add(std::string_view("BLOCKCOMMENT"), "{/*", "*/}"); }
   }
   else if( stringExtension == ".go" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\"); // Double-quoted
      state_.add(std::string_view("RAWSTRING"), "`", "`");      // Raw string (no escaping)
   }
   else if( stringExtension == ".rs" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "r\"", "\"");
      state_.add(std::string_view("RAWSTRING"), "r#\"", "\"#");
      state_.add(std::string_view("RAWSTRING"), "r##\"", "\"##");
   }
   else if( stringExtension == ".html" || stringExtension == ".htm" || stringExtension == ".xml" )
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
      state_.add(std::string_view("BLOCKCOMMENT"), "\"\"\"", "\"\"\"");
      state_.add(std::string_view("BLOCKCOMMENT"), "\'\'\'", "\'\'\'");
      state_.add(std::string_view("STRING"), "\"", "\"");
      state_.add(std::string_view("STRING"), "'", "'");
   }
   else if( stringExtension == ".sql" )
   {
      state_.add(std::string_view("LINECOMMENT"), "--", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".php" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".lua" )
   {
      state_.add(std::string_view("LINECOMMENT"), "--", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "--[[", "]]");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
      state_.add(std::string_view("RAWSTRING"), "[[", "]]");
   }
   else if( stringExtension == ".rb" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "=begin", "=end");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".json" )
   {
      state_.add(std::string_view("STRING"), "\"", "\"");
   }
   else if( stringExtension == ".pl" || stringExtension == ".pm" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\\");
   }
   else if( stringExtension == ".sh" || stringExtension == ".bash" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".yaml" || stringExtension == ".yml" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".toml" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".dart" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "r\"", "\"");
   }
   else if( stringExtension == ".clj" )
   {
      state_.add(std::string_view("LINECOMMENT"), ";", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
   }
   else if( stringExtension == ".vim" )
   {
      state_.add(std::string_view("LINECOMMENT"), "\"", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'", "\'");
   }
   else if( stringExtension == ".bat" || stringExtension == ".cmd" )
   {
      state_.add(std::string_view("LINECOMMENT"), "REM", "\n");
      state_.add(std::string_view("LINECOMMENT"), "::", "\n");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'");
   }
   else if( stringExtension == ".ps1" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "<#", "#>");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("STRING"), "\'", "\'");
      state_.add(std::string_view("RAWSTRING"), "@\"", "\n\"@");
   }
   else if( stringExtension == ".mak" || stringExtension == ".makefile" || stringExtension == ".ninja" )
   {
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
   }
   else if( stringExtension == ".ini" )
   {
      state_.add(std::string_view("LINECOMMENT"), ";", "\n");
      state_.add(std::string_view("LINECOMMENT"), "#", "\n");
   }
   else if( stringExtension == ".txt" || stringExtension == ".md" )
   {
      // No special states for text files
   }
   else if( stringExtension == ".zig" )
   {
      state_.add(std::string_view("LINECOMMENT"), "//", "\n");
      state_.add(std::string_view("BLOCKCOMMENT"), "/*", "*/");
      state_.add(std::string_view("STRING"), "\"", "\"", "\\");
      state_.add(std::string_view("RAWSTRING"), "\\\\", "\\\\");
   }
   else
   {
      return { false, "Unknown file type: " + stringFile };
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Ensures the provided path is absolute. Supports wildcard patterns.
 *
 * @note In sample there are some ** / characters, please remove the space when using it. This to make the documentation parser happy.
 *
 * Enhanced version that handles wildcard patterns with multiple scanning modes:
 * - "test*"     → scans current directory for matching folders
 * - "** /test*"  → recursively scans current directory and subdirectories
 * - "/path/test*" → scans specified path for matching folders
 *
 * ### Behavior:
 * - If the path is empty, it assigns the current working directory.
 * - If the path contains wildcards (* or ?), it expands to matching folders.
 * - If the pattern starts with "** /" it does recursive scanning.
 * - If the path is relative, it converts it to an absolute path.
 * - If the path contains multiple entries separated by `;` or `,`, it processes each entry.
 *
 * @param stringPath A reference to the path string to process. Modified in place.
 *                   If wildcards match multiple paths, they are joined with `;`
 *
 * ### Example Usage:
 * ```cpp
 * // Scan current directory only
 * std::string path1 = "test*";
 * CApplication::PreparePath_s(path1);
 * // path1 might become "test1;test2;testing"
 *
 * // Scan current directory and all subdirectories recursively
 * std::string path2 = "** /test*";
 * CApplication::PreparePath_s(path2);
 * // path2 might become "test1;subdir/test2;deep/path/testing"
 *
 * // Scan specific directory
 * std::string path3 = "/some/path/test*";
 * CApplication::PreparePath_s(path3);
 * // path3 becomes matching folders in /some/path/
 * ```
 **/
unsigned CApplication::PreparePath_s(std::string& stringPath)
{
   char iSplitCharacter = ';'; // default split character
   if( stringPath.empty() == true || stringPath == "." )
   {
      std::filesystem::path pathFile = std::filesystem::current_path();
      stringPath = pathFile.string();
      return 1;
   }

   // Handle special cases for wildcards
   if( stringPath == "*" || stringPath == "**" )
   {
      std::filesystem::path pathFile = std::filesystem::current_path();
      stringPath = pathFile.string();
      return 1;
   }

   auto uPosition = stringPath.find_first_of(";,");
   if( uPosition != std::string::npos )
   {
      iSplitCharacter = stringPath[uPosition];
   }
   else
   {
      iSplitCharacter = 0;
   }

   return PreparePath_s( stringPath, iSplitCharacter );
}

unsigned CApplication::PreparePath_s( std::string& stringPath, char iSplitCharacter )
{
   unsigned uPathCount = 0; // number of paths processed
   if( iSplitCharacter != 0 )
   {
      std::string stringNewPath;
      auto vectorPath = Split_s(stringPath, iSplitCharacter);
      for( const auto& it : vectorPath )
      {
         if( it.empty() == false )
         {
            if( stringNewPath.empty() == false ) { stringNewPath += iSplitCharacter; } // Add split character if not the first path
            std::string stringCheck = it;
            uPathCount += PreparePath_s(stringCheck, 0);
            stringNewPath += stringCheck;
         }
      }
      stringPath = stringNewPath;
   }
   else
   {
      if( stringPath.empty() == true || stringPath == "." )
      {
         std::filesystem::path pathFile = std::filesystem::current_path();
         stringPath = pathFile.string();
         uPathCount = 1;
      }
      else
      {
         // Check if path contains wildcards
         if(stringPath.find('*') != std::string::npos || stringPath.find('?') != std::string::npos)
         {
            std::vector<std::string> vectorMatches;
            bool bRecursive = false;
            std::string stringProcessPath = stringPath;

            // Check for recursive pattern "**/"
            if(stringPath.find("**/") == 0)
            {
               bRecursive = true;
               stringProcessPath = stringPath.substr(3);                      // Remove "**/" prefix
            }
            else if(stringPath.find("**\\") == 0)
            {
               bRecursive = true;
               stringProcessPath = stringPath.substr(3);                      // Remove "**\" prefix
            }

            // Make path absolute first if it's relative
            std::filesystem::path pathInput(stringProcessPath);
            std::string stringAbsolutePattern;

            if(pathInput.is_absolute() == false)
            {
               std::filesystem::path pathParent = pathInput.parent_path();
               if(pathParent.empty() == true)
               {
                  // No parent path - scan from current directory
                  pathParent = std::filesystem::current_path();
               }
               else
               {
                  // Relative parent path - make it absolute
                  pathParent = std::filesystem::absolute(pathParent);
               }
               stringAbsolutePattern = (pathParent / pathInput.filename()).string();
            }
            else
            {
               stringAbsolutePattern = stringProcessPath;
            }

            // Expand wildcards
            uPathCount = ExpandWildcardPath_s(stringAbsolutePattern, vectorMatches, bRecursive);

            if(uPathCount > 0)
            {
               // Join all matches with semicolon
               stringPath.clear();
               for(size_t i = 0; i < vectorMatches.size(); ++i)
               {
                  if(i > 0) stringPath += ";";
                  stringPath += vectorMatches[i];
               }
            }
            else
            {
               // No matches found, keep original pattern
               stringPath = stringAbsolutePattern;
               uPathCount = 0;
            }
         }
         else
         {
            // No wildcards, handle as before
            std::filesystem::path pathFile(stringPath);
            if( pathFile.is_absolute() == false )
            {
               stringPath = std::filesystem::absolute(pathFile).string();
            }
            uPathCount = 1;
         }
      }
   }
   return uPathCount;
}

/** ---------------------------------------------------------------------------
 * @brief Helper function to expand wildcard patterns in paths
 *
 * This function scans directories for folders matching the wildcard pattern.
 * It can scan:
 * - The parent directory if a path is specified (e.g., "/some/path/test*")
 * - The current directory if just a pattern is given (e.g., "test*")
 * - Child directories recursively if specified
 *
 * @param stringPath The path containing wildcard pattern
 * @param vectorResult Vector to store all matching absolute paths
 * @param bRecursive If true, scan child directories recursively
 * @return Number of matching paths found
 **/
unsigned CApplication::ExpandWildcardPath_s(const std::string& stringPath, std::vector<std::string>& vectorResult, bool bRecursive)
{
   std::filesystem::path pathInput(stringPath); // Input path with potential wildcards
   std::string stringPattern = pathInput.filename().string(); // Extract the pattern (last part of the path)

   // Check if pattern contains wildcards
   if(stringPattern.find('*') == std::string::npos && stringPattern.find('?') == std::string::npos)
   {
      // No wildcards, return the path as-is if it exists
      if(std::filesystem::exists(pathInput))
      {
         vectorResult.push_back(std::filesystem::absolute(pathInput).string());
         return 1;
      }
      return 0;
   }

   // Determine the directory to scan
   std::filesystem::path pathToScan;

   // Check if we have a parent path specified
   std::filesystem::path pathParent = pathInput.parent_path();

   if(pathParent.empty() == true ) { pathToScan = std::filesystem::current_path(); } // No parent path specified (e.g., just "test*"). Use current working directory
   else
   {
      // Parent path specified (e.g., "/some/path/test*" or "relative/test*")
      if(pathParent.is_absolute() == false ) { pathParent = std::filesystem::absolute(pathParent); }
      pathToScan = pathParent;
   }

   // Check if directory exists
   if(std::filesystem::exists(pathToScan) == false || std::filesystem::is_directory(pathToScan) == false) { return 0; }

   unsigned uMatchCount = 0;

   try
   {
      if(bRecursive == true)
      {
         // Recursive scan - search in current directory and all subdirectories
         for(const auto& entry : std::filesystem::recursive_directory_iterator(pathToScan))
         {
            if(entry.is_directory() == true )
            {
               std::string stringFolderName = entry.path().filename().string();

               // ## If match the add to result
               bool bMatch = gd::ascii::strcmp( stringFolderName.c_str(), stringFolderName.length(), stringPattern.c_str(), stringPattern.length(), gd::utf8::tag_wildcard() );
               if(bMatch == true)
               {
                  vectorResult.push_back(entry.path().string());
                  uMatchCount++;
               }
            }
         }
      }
      else
      {
         // Non-recursive - only scan immediate children
         for(const auto& entry : std::filesystem::directory_iterator(pathToScan))
         {
            if(entry.is_directory() == true )
            {
               std::string stringFolderName = entry.path().filename().string();

               // ## If match the add to result
               bool bMatch = gd::ascii::strcmp( stringFolderName.c_str(), stringFolderName.length(), stringPattern.c_str(), stringPattern.length(), gd::utf8::tag_wildcard() );
               if(bMatch == true)
               {
                  vectorResult.push_back(entry.path().string());
                  uMatchCount++;
               }
            }
         }
      }
   }
   catch(const std::filesystem::filesystem_error&)
   {                                                                                               assert(false);
      // Handle filesystem errors (permission denied, etc.)
      return 0;
   }

   return uMatchCount;
}


/** ---------------------------------------------------------------------------
 * @brief Prompts the user for input values for specified command-line options.
 *
 * This method checks if the "prompt" option is active in the provided command-line options.
 * If active, it retrieves the list of options that require user input, prompts the user for each,
 * and sets the corresponding values in the active options.
 *
 * @param poptionsApplication Pointer to the command-line options object.
 * @return std::pair<bool, std::string> Returns a pair where the first element is true on success and false on failure,
 *         and the second element contains an error message if applicable.
 *
 * Example usage:
 * @code
 * gd::cli::options optionsApplication;
 * // ... (initialize optionsApplication)
 * auto result = CApplication::CliPrompt_s(&optionsApplication);
 * if(!result.first) {
 *     std::cerr << "Error: " << result.second << std::endl;
 * }
 * @endcode
 */
std::pair<bool, std::string> CApplication::CliPrompt_s(gd::cli::options* poptionsApplication)
{                                                                                                  assert(poptionsApplication != nullptr);
   if( poptionsApplication->exists("prompt", gd::types::tag_state_active{}) == true )
   {
      // If prompt option is active, we can use it
      auto stringOptions = poptionsApplication->get_variant_view("prompt", gd::types::tag_state_active{}).as_string();
      if( stringOptions.empty() == false )
      {
         gd::cli::options* poptionsActive = poptionsApplication->find_active();
         auto vector_ = gd::utf8::split(stringOptions, ';');                  // Split prompt values by ; and add them to options

         // ## prompt for each argument but first print information to user that user input is needed
         papplication_g->PrintMessage( "Please provide values for the following options (leave empty to skip)" );

         for( const auto& argument_ : vector_ )
         {
            std::cout << "Set " << argument_ << ": ";
            std::string stringValue;
            std::getline(std::cin, stringValue);                              // Get user input

            if( stringValue.empty() == true ) continue;                       // Skip empty values
            poptionsActive->set_value(argument_, stringValue);                // Set value to active options
         }
      }
   }

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Sets a variable's value based on rules defined in arguments.
 * @param arguments_ The arguments containing the rules.
 * @param pvariantValue The variant to set the value on.
 * @return A pair indicating success or failure, along with an error message if applicable.
 */
std::pair<bool, std::string> CApplication::CliSetVariable_s(const gd::argument::arguments& arguments_, gd::variant* pvariantValue)
{
   if( arguments_["ask"].is_true() == true )
   {
      std::string stringDescription = arguments_["description"].as_string();
      if( stringDescription.empty() == false )
      {
         std::cout << stringDescription << std::endl;                         // Print description if provided
      }

      std::string stringName = arguments_["name"].as_string();
      if( stringName.empty() == false )
      {
         stringName = std::string().append("(").append(stringName).append("): ");
         std::cout << stringName;
      }

      std::string stringValue;
      std::getline(std::cin, stringValue);                                    // Get user input

      if( pvariantValue != nullptr ) { *pvariantValue = stringValue; }        // Set value to variant

      if( stringValue.empty() == true && arguments_["required"].is_true() == true )
      {
         return { false, "No value provided for variable" };
      }
   }

   return { true, "" };
}

bool CApplication::CliLogging_s(gd::cli::options* poptionsApplication)
{
   bool bSetLogging = false;
#ifdef GD_LOG_SIMPLE
   if( poptionsApplication->exists("logging-severity", gd::types::tag_state_active{}) == true ) // if logging severity is set
   {
      std::string stringSeverity = poptionsApplication->get_variant_view("logging-severity", gd::types::tag_state_active{}).as_string();
      if( stringSeverity.empty() == false )
      {
         auto eSeverityNumber = gd::log::severity_get_type_number_g(stringSeverity);
         if( eSeverityNumber != gd::log::enumSeverityNumber::eSeverityNumberNone )
         {
            gd::log::logger<0>* plogger = gd::log::get_s();
            plogger->set_severity( eSeverityNumber );                                              LOG_INFORMATION_RAW("== Set logging severity to: " & stringSeverity);
            bSetLogging = true;                                                // set logging is set
         }
      }
   }
#endif
   return bSetLogging;
}

/** --------------------------------------------------------------------------- @API [tag: folder, application ] [summary: Get folder to home directory]
 * @brief Retrieves the home directory path for the application.
 *
 * This static method determines the home directory path based on the operating system.
 * It sets the provided `stringHomePath` to the appropriate path and checks if the directory exists.
 *
 * @param stringHomePath A reference to a string where the home path will be stored.
 * @return std::pair<bool, std::string> Returns a pair where the first element is true on success and false on failure,
 *         and the second element contains an error message if applicable.
 *
 * Example usage:
 * @@code @code
 * std::string stringHomePath;
 * auto result = CApplication::FolderGetHome_s(stringHomePath);
 * if(!result.first) { std::cerr << "Error: " << result.second << std::endl; }
 * @endcode
 */
std::pair<bool, std::string> CApplication::FolderGetHome_s(std::string& stringHomePath)
{
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

   if( std::filesystem::exists(stringPath) == false ) { return { false, "User configuration directory does not exist: " + stringPath }; }

   stringHomePath = stringPath; // Set the home path

   return { true, "" };
}

/** --------------------------------------------------------------------------- @API [tag: ignore, application ] [summary: Read ignore patterns]
 * @brief Reads ignore patterns from a specified file or directory and populates a vector of ignore rules.
 *
 * This static method attempts to read ignore patterns (such as those found in .gitignore or other ignore files)
 * from the provided file or directory path. If a directory is specified, it looks for a .gitignore file or other
 * files containing "ignore" in their name within the directory. The method parses the file(s), extracts ignore
 * patterns, and fills the provided vector with ignore rules, handling folder and wildcard patterns as needed.
 *
 * @param stringForderOrFile The path to a file or directory to search for ignore patterns.
 * @param vectorIgnore Reference to a vector that will be populated with parsed ignore rules.
 * @return std::pair<bool, std::string> Returns a pair where the first element is true on success and false on failure,
 *         and the second element contains an error message if applicable.
 *
 * Example usage:
 * @code
 * std::vector<ignore> vectorPattern;
 * auto result = CApplication::ReadIgnoreFile_s("/path/to/project", vectorPattern);
 * if(!result.first) {
 *     std::cerr << "Error: " << result.second << std::endl;
 * }
 * @endcode
 */
std::pair<bool, std::string> CApplication::ReadIgnoreFile_s(const std::string_view& stringForderOrFile, std::vector<ignore>& vectorIgnore )
{
   using gd::expression::parse::state;

   std::vector<std::filesystem::path> vectorFile;
   std::filesystem::path pathForderOrFile(stringForderOrFile);

   if( std::filesystem::is_directory( pathForderOrFile ) == true)
   {
      // ## first, check for gitignore file in the directory
      std::filesystem::path pathGitIgnore = pathForderOrFile / ".gitignore"; // Check for .gitignore file
      if( std::filesystem::exists(pathGitIgnore) && std::filesystem::is_regular_file(pathGitIgnore)) { vectorFile.push_back(pathGitIgnore); }
      else
      {
         // ## if no .gitignore file we try to find some file that have the ignore pattern in folder

         unsigned uMax = 20; // Limit the number of files to read, to avoid too many files
         for( const auto& entry_ : std::filesystem::directory_iterator(pathForderOrFile) )
         {
            if( entry_.is_regular_file() && entry_.path().extension().string().find( "ignore" ) != std::string::npos ) // Check for .ignore files
            {
               vectorFile.push_back(entry_);
            }

            if( --uMax == 0 ) break;                                          // Limit the number of files to read
         }
      }
   }
   else
   {
      vectorFile.push_back( pathForderOrFile );
   }

   if( vectorFile.empty() == true ) return { true, "" };

   // ## read ignore file, if it exists
   std::filesystem::path pathInput = vectorFile.front();                      // Take the first file from the vector

   std::ifstream file_(pathInput, std::ios::binary);
   if( file_.is_open() == false ) { return { false, "Failed to open ignore file: " + pathInput.string() }; } // failed to open file

   gd::expression::parse::state state_;
   state_.add(std::string_view("LINECOMMENT"), "#", "\n");
   gd::parse::window::line lineBuffer(1024, gd::types::tag_create{});         // create line buffer 64 * 16 = 1024 bytes = 16 cache lines


   file_.read((char*)lineBuffer.buffer(), lineBuffer.available());
   auto uReadSize = file_.gcount();                                           // get number of valid bytes read
   lineBuffer.update(uReadSize);                                              // Update valid size in line buffer

   // ## Process the file
   while(lineBuffer.eof() == false)
   {
      std::string_view stringLine;
      while( lineBuffer.getline( stringLine ) == true )
      {
         // ## Process the line, filter comments

         auto [iRule, piPosition] = state_.find_first(stringLine);            // find first rule in the line or return -1 if no rule and if whe have a position then process it

         // ### If we have don't have a rule, but a pointer then process it (this is code)

         if( iRule == -1 && piPosition != nullptr )
         {
            auto [ iRule, stringValue ] = state_.read_first( stringLine );    // get line value

            // #### Check if we have a folder or file to ignore

            auto position_ = stringValue.find('.');                           // find . character
            if( position_ == 0 || position_  == std::string::npos )           // a bit brutal but skip everything with . unless folder starts with .
            {
               stringValue = gd::utf8::trim(stringValue, gd::types::tag_view{});// trim whitespace from the start and end of the string
               unsigned uType = 0;                                          // type of ignore
               if( stringValue[0] == '/' )                                  // if starts with / then it is a folder
               {
                  uType = unsigned(ignore::eTypeRoot|ignore::eTypeFolder);
                  stringValue = stringValue.substr(1);                      // remove the first character
               }
               else if( stringValue.back() == '/' )
               {
                  uType = unsigned(ignore::eTypeFolder);
                  stringValue = stringValue.substr(0, stringValue.length() - 1); // remove the last character
               }
               else if( stringValue.find_first_of("*?") == std::string::npos )
               {
                  uType = unsigned(ignore::eTypeFolder);
               }

               if( uType != 0 )
               {
                  if( stringValue.find_first_of("*?") != std::string_view::npos ) { uType |= unsigned(ignore::eTypeWildcard); } // if we have a wildcard then set the type to wildcard
                  std::string string_( stringValue );
                  std::replace(string_.begin(), string_.end(), '\\', '/');
                  vectorIgnore.push_back( { uType, string_ } );
               }
            }
         }
      }

      lineBuffer.rotate();
      file_.read((char*)lineBuffer.buffer(), lineBuffer.available());
      uReadSize = file_.gcount();                                             // get number of valid bytes read
      lineBuffer.update(uReadSize);
   }

   return {true, ""};
}

/** --------------------------------------------------------------------------- @API [tag: parse, configuration, application ] [summary: Reads configuration from an XML file]
 * @brief Reads configuration from an XML file and populates the application state.
 * @param stringFile The path to the XML file to read.
 * @param tag_xml Unused parameter, kept for compatibility with the function signature.
 *
 * @verbatim
<templates>
   <template name="template-name" description="optional description">
      <command name="command name" description="optional description"><![CDATA[ raw command line string {option name needed to be filled in} ]]></command>
      <metadata>
         <autor></autor>
         <version></version>
         <application></application>
      </metadata>
      <configuration>
         <options>
            <option name="option-name" type="boolean|integer|decimal|string" required="true" default="value" description="optional description"></option>
            <option name="option-name" type="boolean|integer|decimal|string" required="true" default="value" description="optional description"></option>
         </options>
      </configuration>
   </template>
</templates>
 * @endverbatim
 *
 * @return A pair containing:
 *         - `bool`: `true` if the operation was successful, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 */
std::pair<bool, std::string> CApplication::SettingsRead_s(const std::string_view stringFile, gd::types::tag_xml)
{

   pugi::xml_document xmldocument; // Create an XML document object to hold the parsed XML data
   CONFIGURATION::CSettings settings_; // Create a settings object to hold configuration data

   // Load the XML file
   pugi::xml_parse_result result = xmldocument.load_file(stringFile.data());
   if(!result) { return { false, std::string("Failed to load XML file: ") + result.description() }; }

   // Get the root templates node
   pugi::xml_node xmlnodeTemplates = xmldocument.child("templates");
   if(!xmlnodeTemplates) { return { false, "No 'templates' root node found in XML" }; }

   // ## Iterate through each template
   for(pugi::xml_node xmlnodeTemplate = xmlnodeTemplates.child("template"); xmlnodeTemplate; xmlnodeTemplate = xmlnodeTemplate.next_sibling("template"))
   {
      // ## Read template attributes
      std::string stringTemplateName = xmlnodeTemplate.attribute("name").value();
      std::string stringTemplateDescription = xmlnodeTemplate.attribute("description").value();

      // ## `metadata` element
      pugi::xml_node xmlnodeMetadata = xmlnodeTemplate.child("metadata");
      if(xmlnodeMetadata)
      {
         std::string stringAutor = xmlnodeMetadata.child("autor").text().get();
         std::string stringVersion = xmlnodeMetadata.child("version").text().get();
         std::string stringApplication = xmlnodeMetadata.child("application").text().get();
      }

      CONFIGURATION::CSettings::settings* psettingsAdd;
      // Read command node
      pugi::xml_node xmlnodeCommand = xmlnodeTemplate.child("command");
      if((bool)xmlnodeCommand == true )
      {
         std::string stringCommandName = xmlnodeCommand.attribute("name").value();
         std::string stringCommandDescription = xmlnodeCommand.attribute("description").value();
         std::string stringCommandData = xmlnodeCommand.text().get();

         if( stringTemplateName.empty() == true ) stringTemplateName = stringCommandName; // Use command name as template name if not specified

         psettingsAdd = settings_.Add(stringTemplateName, stringCommandData, stringCommandDescription); // Add command to settings
      }
      else
      {
         psettingsAdd = settings_.Add(stringTemplateName, stringTemplateDescription); // Add command to settings
      }


      // Read configuration node
      pugi::xml_node xmlnodeConfiguration = xmlnodeTemplate.child("configuration");
      if(xmlnodeConfiguration)
      {
         pugi::xml_node xmlnodeOptions = xmlnodeConfiguration.child("options");
         if(xmlnodeOptions)
         {
            // Iterate through all options
            for(pugi::xml_node xmlnodeOption = xmlnodeOptions.child("option"); xmlnodeOption; xmlnodeOption = xmlnodeOption.next_sibling("option"))
            {
               std::string stringOptionName = xmlnodeOption.attribute("name").value();
               std::string stringOptionType = xmlnodeOption.attribute("type").value();
               std::string stringOptionRequired = xmlnodeOption.attribute("required").value();
               std::string stringOptionDefault = xmlnodeOption.attribute("default").value();
               std::string stringOptionDesc = xmlnodeOption.attribute("description").value();

               if(stringOptionName.empty() == true) { return { false, "Option missing required 'name' attribute" }; }

               // ## Validate option type
               if(stringOptionType.empty() == false &&
                  stringOptionType != "boolean" &&
                  stringOptionType != "integer" &&
                  stringOptionType != "decimal" &&
                  stringOptionType != "string") { return { false, "Invalid option type: " + stringOptionType }; }

               // Here you would typically store the parsed data in your configuration structure
               // For example: m_vectorOptions.push_back({stringOptionName, stringOptionType, ...});
            }
         }
      }
   }

   return { true, "" };
}

#if 0
void CApplication::Read_s(const gd::database::record* precord, gd::table::table_column_buffer* ptablecolumnbuffer )
{
   for( unsigned u = 0, uMax = (unsigned)precord->size(); u < uMax; u++ )
   {
      auto pcolumn = precord->get_column( u );
      std::string_view stringName = precord->name_get( u );
      unsigned uType = pcolumn->type();
#ifndef NDEBUG
      auto pbszTypeName_d = gd::types::type_name_g( uType );
#endif // !NDEBUG

      unsigned uSize = 0;
      if( pcolumn->is_fixed() == false ) { uType |= gd::types::eTypeDetailReference; }
      else                               { uSize = pcolumn->size_buffer(); }

      ptablecolumnbuffer->column_add( uType, uSize, stringName );
   }
}

void CApplication::Read_s( gd::database::cursor_i* pcursorSelect, gd::table::table_column_buffer* ptablecolumnbuffer )
{
   const auto* precord = pcursorSelect->get_record();

   if( ptablecolumnbuffer->empty() == true )
   {
      if( ptablecolumnbuffer->get_reserved_row_count() == 0 ) ptablecolumnbuffer->set_reserved_row_count( 10 ); //pre allocate data to hold 10 rows
      ptablecolumnbuffer->set_flags( gd::table::tag_full_meta{});
      Read_s( precord, ptablecolumnbuffer );
      ptablecolumnbuffer->prepare();

      while( pcursorSelect->is_valid_row() == true )
      {
         auto vectorValue = precord->get_variant_view();
         ptablecolumnbuffer->row_add( vectorValue );
         pcursorSelect->next();
      }
   }
   else
   {
      // ## table contains columns, match against tables in result to know what to add
      auto vectorTableName = ptablecolumnbuffer->column_get_name();
      auto vectorResultName = precord->name_get();
      // Match column names, only fill in columns with matching name in table and result
      auto vectorMatch = gd::table::table_column_buffer::column_match_s( vectorTableName, vectorResultName );

      if( vectorMatch.empty() == false )
      {
         std::vector<unsigned> vectorWriteTable;
         std::vector<unsigned> vectorReadResult;

         for( const auto& it : vectorMatch )
         {
            vectorWriteTable.push_back( it.first );
            vectorReadResult.push_back( it.second );
         }

         while( pcursorSelect->is_valid_row() == true )
         {
            auto vectorValue = precord->get_variant_view( vectorReadResult );
            ptablecolumnbuffer->row_add( vectorValue, vectorWriteTable, gd::table::tag_convert{} );
            pcursorSelect->next();
         }
      }
      else
      {
         while( pcursorSelect->is_valid_row() == true )
         {
            auto vectorValue = precord->get_variant_view();
            ptablecolumnbuffer->row_add( vectorValue, gd::table::tag_convert{} );
            pcursorSelect->next();
         }
      }
   }
}
#endif // 0


/** ---------------------------------------------------------------------------
 * @brief Finds the configuration file in the current directory or its parent directories.
 *
 * This static method searches for a configuration file named "cleaner-configuration.json" in the current directory
 * and up to a specified number of parent directories. If found, it sets the provided pathLocation to the file's path.
 *
 * @param pathLocation A reference to a filesystem::path where the found configuration file's path will be stored.
 * @param uDirectoryLevels The number of parent directories to search upwards.
 * @return std::pair<bool, std::string> Returns a pair where the first element is true if the file was found,
 *         false if not found, and the second element contains an error message if applicable.
 */
std::pair<bool, std::string> CApplication::ConfigurationFindFile_s(std::filesystem::path& pathLocation, uint32_t uDirectoryLevels )
{
   constexpr std::string_view stringConfigurationName( ".cleaner-configuration.json" );
   std::filesystem::path pathConfigurationCurrent = std::filesystem::current_path(); // Default configuration location in current directory
   for( uint32_t u = 0; u <= uDirectoryLevels; ++u )
   {
      std::filesystem::path pathConfigurationTemp = pathConfigurationCurrent / stringConfigurationName; // Create the path to the configuration file in the current directory
      if( std::filesystem::exists(pathConfigurationTemp) == true )
      {
         pathLocation = pathConfigurationTemp;                                // Set the location to the configuration file path
         return { true, "" };                                                 // Found the configuration file
      }

      if(pathConfigurationCurrent == pathConfigurationCurrent.root_path() )   // If we reached the root directory, stop searching
      {
         return { true, "Unable to find " + std::string(stringConfigurationName) };
      }
      pathConfigurationCurrent = pathConfigurationCurrent.parent_path(); // Go one directory up and check again
   }
   return { true, "" };
}

/** --------------------------------------------------------------------------- @API [tag: history, file, application ] [summary: Find local history file]  
 * @brief Finds the local history file
 *
 * This static method searches for a history file named ".cleaner-history.xml" in the current directory
 * and up to a specified number of parent directories. If found, it sets the provided pathLocation to the file's path.
 *
 * @param pathLocation A reference to a filesystem::path where the found history file's path will be stored.
 * @return std::pair<bool, std::string> Returns a pair where the first element is true if the file was found,
 *         false if not found, and the second element contains an error message if applicable.
 */
std::pair<bool, std::string> CApplication::HistoryFindLocal_s(std::filesystem::path& pathLocation )
{
   uint64_t uHistoryLevels = papplication_g->PROPERTY_Get("history-levels");

   const std::string stringHistoryName =  ".cleaner-history.xml";
   std::filesystem::path pathHistoryCurrent = std::filesystem::current_path(); // Default history location in current directory

   for( uint64_t u = 0; u <= uHistoryLevels; ++u )
   {
      std::filesystem::path pathHistoryTemp = pathHistoryCurrent / stringHistoryName; // Create the path to the history file in the current directory

      if( std::filesystem::exists(pathHistoryTemp) == true )
      {
         pathLocation = pathHistoryTemp; // Set the location to the history file path
         return { true, "" };                                                 // Found the history file
      }

      if(pathHistoryCurrent == pathHistoryCurrent.root_path() )               // If we reached the root directory, stop searching
      {
         return { false, "" };
      }
      pathHistoryCurrent = pathHistoryCurrent.parent_path();                  // Go one directory up and check again
   }

   return { false, "" };
}

std::pair<bool, std::string> CApplication::HistorySave_s(const std::string_view& stringFileName, const gd::table::dto::table* ptable)
{
   pugi::xml_document xmldocument;
   pugi::xml_parse_result result_ = xmldocument.load_file(stringFileName.data());
   //if( !result_ ) { return { false, std::string("Failed to load XML file: ") + stringFileName.data()}; }
   if( !result_ ) { return { false, std::format("Failed to load XML file: {}", stringFileName) }; }

   // Check if entries exist
   pugi::xml_node xmlnodeEntries = xmldocument.child("history").child("entries");
   if( xmlnodeEntries.empty() ) { return { false, std::format("No entries node found in XML file: {}", stringFileName) }; }

   // Create a new entry node
   pugi::xml_node xmlnodeEntry = xmlnodeEntries.append_child("entry");

   auto uRowCount = ptable->size();

   for( unsigned u = 0; u < uRowCount; ++u )
   {
      std::string stringDate = ptable->cell_get_variant_view(u, "date").as_string();
      std::string stringCommand = ptable->cell_get_variant_view(u, "command").as_string();
      std::string stringLine = ptable->cell_get_variant_view(u, "line").as_string();

      xmlnodeEntry.append_child("date").text().set(stringDate.c_str());
      xmlnodeEntry.append_child("command").text().set(stringCommand.c_str());
      xmlnodeEntry.append_child("line").text().set(stringLine.c_str());
   }



   xmldocument.save_file(stringFileName.data(), "  ", pugi::format_default);

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Finds the active history file, either local or in the home directory.
 *
 * This static method first attempts to find a local history file named ".cleaner-history.xml"
 * in the current directory and its parent directories. If not found, it then looks for a
 * history file named ".cleaner-history.xml" in the user's home directory.
 *
 * @param pathLocation A reference to a filesystem::path where the found history file's path will be stored.
 * @return std::pair<bool, std::string> Returns a pair where the first element is true if a history file was found,
 *         false if not found, and the second element contains an error message if applicable.
 */
std::pair<bool, std::string> CApplication::HistoryFindActive_s(std::filesystem::path& pathLocation)
{
   // ## Find local history file first

   auto result_ = HistoryFindLocal_s(pathLocation);                            // Try to find local history file

   // ## Find home history file if no local history file found

   if( result_.first == false )
   {
      std::string stringHomePath;
      result_ = FolderGetHome_s( stringHomePath );
      if( result_.first == false ) { return result_; }
      pathLocation = std::filesystem::path(stringHomePath) / ".cleaner-history.xml"; // Default history location in user home directory
      if( std::filesystem::exists(pathLocation) == false ) { return { false, "No history file found" }; }
   }

   return { true, "" };
}

std::pair<bool, std::string> CApplication::HISTORY_SaveCommand(const std::string_view& stringFileLocation)
{
   std::string stringCommand = PROPERTY_Get("command").as_string();
   std::string stringLine = PROPERTY_Get("arguments").as_string();


   auto* pdocument = DOCUMENT_Get("history", true); // Pointer to the current document

   if( pdocument->CACHE_Exists("history") == false )
   {
      pdocument->CACHE_Prepare("history");
   }
   auto* ptable = pdocument->CACHE_Get("history");

   HISTORY_AddAndSave(stringCommand, stringLine, ptable);

   HistorySave_s(stringFileLocation, ptable); // Save the history table to file

   return { true, "" };
}


/** ---------------------------------------------------------------------------
 * @brief Splits a string into a vector of strings based on the specified delimiter.
 *
 * *Sample code*
 * ```cpp
 * std::string stringText = "apple;banana;cherry";
 * char iDelimiter = ';';
 * std::vector<std::string> result = Split_s(stringText, iDelimiter);
 * // result will contain {"apple", "banana", "cherry"}
 * ```
 *
 * @param stringText The string to split.
 * @param iDelimiter The delimiter character to use for splitting. If 0, it will try to determine the delimiter.
 * @return std::vector<std::string> A vector of strings obtained by splitting the input string.
 */
std::vector<std::string> CApplication::Split_s(const std::string& stringText, char iDelimiter)
{
   std::vector<std::string> vectorResult; // vector to hold the split strings

   char iEffectiveDelimiter = iDelimiter;
   if( iEffectiveDelimiter == 0 )
   {
      // ## Determine the effective delimiter

      auto uSemicolon = stringText.find(";");
      auto uComma = stringText.find(",");

      if( uSemicolon != std::string::npos && uComma != std::string::npos )
      {
         iEffectiveDelimiter = ( uSemicolon < uComma ) ? ';' : ',';           // select the first found
      }
      else if( uSemicolon != std::string::npos )
      {
         iEffectiveDelimiter = ';';
      }
      else if( uComma != std::string::npos )
      {
         iEffectiveDelimiter = ',';
      }
   }

   // ## Split the string using the effective delimiter

   vectorResult = gd::utf8::split( stringText, iEffectiveDelimiter, gd::utf8::tag_escape{});

   return vectorResult;
}

/** ---------------------------------------------------------------------------
 * @brief Splits a string into a vector of strings based on the presence of numbers.
 *
 * This function scans through the input string and extracts substrings that represent numbers.
 * It handles digits, decimal points, and signs (positive/negative) to identify valid number formats.
 *
 * @param stringText The input string to be parsed for numbers.
 * @return std::vector<std::string> A vector containing the extracted number strings.
 */
std::vector<std::string> CApplication::SplitNumber_s(const std::string& stringText)
{
   std::vector<std::string> vectorNumber;
   /// Parse string and try to extract numbers
   std::string stringNumber;
   for( const auto& it : stringText )
   {
      if( std::isdigit(it) || it == '.' || it == '-' || it == '+' ) // Check if character is a digit or a sign
      {
         stringNumber += it;                                                  // Append to current number
      }
      else if( stringNumber.empty() == false )                                // If we hit a non-digit and we have a number, push it to the vector
      {
         vectorNumber.push_back(stringNumber);
         stringNumber.clear();                                                // Clear current number
      }
   }

   if( stringNumber.empty() == false ) vectorNumber.push_back(stringNumber);  // add last string if any

   return vectorNumber;                                                       // Return the vector of numbers
}


// @TASK #user.per [name: keyvalue][summary: ParseKeyValueRule_s is no longer used, remove it?][state: open][date: 2025-08-12]

/** --------------------------------------------------------------------------- @TAG #hack.keyvalue [description: Parse keys and format if specified, this is added to simplify adding key-value pairs to be read]
 * @brief Parses a key-value rule from a string and populates the provided arguments object.
 *
 * This function takes a string rule in the format "key1,ke2,key3@pattern" and parses to extract key-value pairs.
 * It supports multiple keys separated by commas or semicolon and can handle patterns for scoping.
 *
 * @param stringRule The string rule to parse, e.g., "key1,key2@p()=".
 * @param pargumentsKVRule Pointer to an arguments object where the parsed key-value pairs will be stored.
 * @return std::pair<bool, std::string> Returns true if parsing was successful, false with an error message if it failed.
 */
std::pair<bool, std::string> CApplication::ParseKeyValueRule_s(const std::string_view stringRule, gd::argument::arguments* pargumentsKVRule)
{
   char iKeyDelimiter = ':';
   constexpr char iFormatDelimiter = '@';
   enum { eKey = 0, eValue = 1, ePattern = 2, eUnknown = 3 };

   unsigned uState = eKey; // Start with key state

   auto add_ = [pargumentsKVRule, &uState](std::string& stringValue) -> void
   {
      if( stringValue.empty() == true ) return;

      if( uState == eKey )
      {
         if( stringValue.find(',') != std::string::npos ) pargumentsKVRule->append("keys", stringValue); // if ; or , there are multiple keys
         else if( stringValue.find(';') != std::string::npos )
         {
            auto vector_ = gd::utf8::split(stringValue, ';', gd::utf8::tag_escape{}); // split by ; if it is used
            // remove empty strings
            vector_.erase(std::remove_if(vector_.begin(), vector_.end(), [](const std::string& s_) { return s_.empty(); }), vector_.end());
            for( const auto& it : vector_ ) { pargumentsKVRule->append("key", it); }
         }
         else                                                       pargumentsKVRule->append("key", stringValue);
      }
      else if( uState == eValue ) pargumentsKVRule->append("value", stringValue);
      else pargumentsKVRule->append("scope", stringValue);

      stringValue.clear();
   };

   std::string string_;


   // ## Parse value until we hit a delimiter

   for( auto it = std::begin( stringRule ); it != std::end( stringRule ); it++ )
   {
      const char iCharacter = *it;
      if( iCharacter > 'A' ) { string_ += iCharacter; }
      else if( iCharacter == iKeyDelimiter )
      {
         add_( string_ );
         uState++;
      }
      else if( iCharacter == iFormatDelimiter)
      {
         iKeyDelimiter = 0;                                                    // reset key delimiter to avoid further key-value parsing
         add_( string_ );
         uState = ePattern;
      }
      else
      {
         string_ += iCharacter;
      }
   }

   add_( string_ );

#ifndef NDEBUG
   std::string stringArguments_d = gd::argument::debug::print( *pargumentsKVRule );
#endif // NDEBUG

   if( uState != eUnknown ) return { true, "" };

   return { false, std::string("invalid rule: ") + stringRule.data()};
}

/** --------------------------------------------------------------------------- @API [tag: file, application ] [description: Helper method to check if source code text file, this to avoid checking the internal data in file]
 * @brief Checks if the provided file extension is a known text file type.
 *
 * This function checks if the given file extension matches any of the predefined text file extensions.
 * It returns true if the extension is recognized as a text file, otherwise false.
 *
 * @param stringExtension The file extension to check, provided as a string_view.
 * @return bool True if the extension is a known text file type, false otherwise.
 */
bool CApplication::IsTextFile_s(const std::string_view& stringExtension)
{
   // Check if the file extension is one of the known text file types
   static const std::set<std::string_view> setTextFileExtension =
   {
      ".txt", ".md", ".csv", ".json", ".xml", ".html", ".htm", ".css", ".js", ".ts",
      ".jsx", ".tsx", ".py", ".java", ".c", ".cpp", ".cxx", ".h", ".hpp", ".ipp", ".go",
      ".cs", ".fs", ".kt", ".swift", ".rs", ".lua", ".php", ".rb",
      ".pl", ".pm", ".sh", ".bash", ".yaml", ".yml", ".toml",
      ".dart", ".clj", ".vim", ".bat", ".cmd", ".ps1",
      ".mak", ".ninja", ".makefile", ".ini", ".zig"
   };

   return setTextFileExtension.find(stringExtension) != setTextFileExtension.end();
}




#ifdef _WIN32

std::pair<bool, std::string> CApplication::PrepareWindows_s()
{
   // Initialize COM library
   HRESULT iResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
   if( FAILED(iResult) ) { return {false, "Failed to initialize COM library. HRESULT: " + std::to_string(iResult)}; }

   // Additional Windows-specific preparation can go here
   // For example, setting up security, initializing other Windows APIs, etc.

   // Example: Set COM security levels
   /*
   hr = CoInitializeSecurity(
      nullptr,                        // Security descriptor
      -1,                             // COM negotiates authentication services
      nullptr,                        // Authentication services
      nullptr,                        // Reserved
      RPC_C_AUTHN_LEVEL_DEFAULT,      // Default authentication level
      RPC_C_IMP_LEVEL_IMPERSONATE,    // Default impersonation level
      nullptr,                        // Authentication info
      EOAC_NONE,                      // Additional capabilities
      nullptr                         // Reserved
   );

   if(FAILED(hr))
   {
      CoUninitialize(); // Clean up COM if security initialization fails
      return {false, "Failed to initialize COM security. HRESULT: " + std::to_string(hr)};
   }
   */

   papplication_g->PROPERTY_Add("WINDOWS", true );

   // If everything succeeds
   return {true, ""};
}

std::pair<bool, std::string> CApplication::ExitWindows_s()
{
   // Uninitialize the COM library
   CoUninitialize();

   papplication_g->PROPERTY_Add("WINDOWS", false );

   return {true, ""};
}


#endif


/*
@@code @code
// ## core application properties
CApplication::PROPERTY_Add("WINDOWS", true|false); // if windows
CApplication::PROPERTY_Add("os", "windows|linux|wsl|mac"); // os running on
CApplication::PROPERTY_Add("folder-current", "current active folder path");
CApplication::PROPERTY_Add("folder-home", "user home director for cleaner");
@endcode
*/
