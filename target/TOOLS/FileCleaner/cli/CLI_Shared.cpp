/**
 * @file CLI_Shared.cpp
 * 
 * @brief Implementation file for shared CLI operations.
 * 
 * This file contains the implementation of functions shared across CLI tools.
 */

#ifdef _WIN32
 // Windows-specific includes or code
#else

#include <sys/ioctl.h>
#include <unistd.h>

#endif


#include <boost/regex.hpp>

#include "gd/gd_file.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "CLI_Shared.h"

NAMESPACE_CLI_BEGIN

std::pair<bool, std::string> SHARED_ReadHarvestSetting_g( const gd::cli::options& options_, gd::argument::shared::arguments& arguments_, CDocument* pdocument)
{                                                                                                  assert( pdocument != nullptr );

   // ## Source option .......................................................

   std::string stringSource = options_["source"].as_string();
   CApplication::PreparePath_s(stringSource);                                                      LOG_DEBUG_RAW_IF( stringSource.empty() == false, "Source: " & stringSource);

   // ## Ignore option .......................................................

   std::string stringIgnore = options_["ignore"].as_string();                                      LOG_DEBUG_RAW_IF( stringIgnore.empty() == false, "Ignore: " & stringIgnore);
   if( stringIgnore.empty() == false ) 
   { 
      auto vectorIgnore = CApplication::Split_s(stringIgnore);
      pdocument->GetApplication()->IGNORE_Add(vectorIgnore);                  // add ignore patterns to the application
   }

   // ## Recursive option ....................................................

   unsigned uRecursive = 0; // default recursive value (current folder)
   if( options_.exists("recursive") == true ) { uRecursive = options_["recursive"].as_uint(); } // get recursive value from options
   else if( options_.exists("R") == true ) 
   { 
      if(options_["R"].is_bool() == true) uRecursive = 16;                    // set to 16 if R is set, find all files
      else { uRecursive = options_["R"].as_uint(); }
   }

   // ## Filter option .......................................................

   std::string stringFilter = options_["filter"].as_string();
   if( stringFilter.empty() == true || stringFilter == "." )
   {
      stringFilter = "*.*";                                                   // all files
   }
   else if( stringFilter == "*" || stringFilter == "**" || stringFilter == ".." )
   {
      if( uRecursive == 0 ) { uRecursive = 16; }
   }
   else if( stringFilter.empty() == false ) 
   {
      if( options_.get_arguments().count("filter") > 1 )
      {
         auto vectorFilter = options_.get_all("filter");
         std::string stringCombinedFilter;
         for( const auto& filter : vectorFilter )
         {
            if( stringCombinedFilter.empty() == false ) stringCombinedFilter += ';';
            stringCombinedFilter += filter.as_string();
         }
         stringFilter = stringCombinedFilter;
      }
   }

   // ## Set harvest arguments ...............................................

   arguments_.set("source", stringSource);                                    // set source argument
   arguments_.set("ignore", stringIgnore);                                    // set ignore argument
   arguments_.set("depth", uRecursive);                                       // set recursive argument
   arguments_.set("filter", stringFilter);                                    // set filter argument

   pdocument->GetApplication()->UpdateApplicationState();                     // update application state based on new arguments

   return { true, "" };                                                       // return success
}

 /** --------------------------------------------------------------------------
 * @brief Retrieves and prepares a list of source file paths from the provided command-line options.
 * @param options_ The command-line options object containing potential source file arguments.
 * @param options_.source {string} A string or list of strings representing source file paths.
 * @return A vector of strings, each representing a prepared source file path.
 */
std::vector<std::string> SHARED_GetSourcePaths( const gd::cli::options& options_ )
{
   auto vectorSourceToPrepare = options_.get_all("source");                    // get all source arguments, this is used to find files in the source directory

   std::vector<std::string> vectorSource; // vector of source paths

   // ## Source preparation
   //    if source is one then check if it may contain multiple sources that are separated by ':'
   if( vectorSourceToPrepare.size() == 1 )
   {
      std::string stringSource = vectorSourceToPrepare[0].as_string();
      auto uCount = CApplication::PreparePath_s(stringSource, ':');           // if source is empty then set it to current path, otherwise prepare it
      if( uCount == 1 ) vectorSource.push_back(stringSource);                 // if there is only one source then add it to the vector
      else if( uCount > 1 )  vectorSource = CApplication::Split_s(stringSource, ':');// if there are multiple sources then split them by ':'
   }
   else if( vectorSourceToPrepare.size() > 1 )
   {
      for( const auto& source : vectorSourceToPrepare )
      {
         std::string stringSource = source.as_string();
         CApplication::PreparePath_s(stringSource, 0);                        // if source is empty then set it to current path, otherwise prepare it
         vectorSource.push_back(stringSource);                                // add prepared source to vector
      }
   }
   else
   {
      std::string stringSource;
      CApplication::PreparePath_s(stringSource, 0);
      vectorSource.push_back(stringSource);
   }

   return vectorSource;                                                       // return the vector of source paths
}



std::pair<bool, std::string> SHARED_MatchAllPatterns_g(const std::vector<std::string>& vectorPattern, CDocument* pdocument, int iMatchCount )
{                                                                                                  assert( pdocument != nullptr ); assert( vectorPattern.size() > 0 ); // at least one pattern must be specified
   std::vector<uint64_t> vectorRowDelete; // vector of row numbers to delete

   auto ptableLineList = pdocument->CACHE_Get("file-linelist"); // get the file line list from the cache

   if( iMatchCount == -1 ) iMatchCount = (int)vectorPattern.size();                     // if iMatchCount is -1 then set it to the size of the vectorPattern, so we check all patterns

   // ## iterate over all rows in table and check the line text for all patterns
   for( size_t uRow = 0; uRow < ptableLineList->get_row_count(); ++uRow )
   {
      std::string_view stringLineText = ptableLineList->cell_get_variant_view(uRow, "line").as_string_view(); // get the line text
      int iMatch = iMatchCount;                                               // number of patterns to match, if -1 then match all
      
      for( const auto& stringPattern : vectorPattern )                        // check if all patterns match the line text
      {
         if( stringLineText.find(stringPattern) != std::string::npos )        // if pattern is not found in line text
         {
            iMatch--;                                                         // decrement the match count 
            if( iMatch <= 0 ) break;                                          // if we have matched all patterns, break the loop
         }
      }
      
      if( iMatch != 0 ) vectorRowDelete.push_back(uRow);                      // if not all patterns match, add row number to delete vector
   }

   // ## delete all rows that do not match all patterns
   if( vectorRowDelete.empty() == false )
   {
      ptableLineList->erase( vectorRowDelete );
   }

   return { true, "" };
}

std::pair<bool, std::string> SHARED_OpenFile_g(const std::string_view& stringFile)
{
   gd::file::path pathFile(stringFile); // convert string to file path
#ifdef _WIN32
   std::wstring wstringConfigurationFile = gd::utf8::convert_ascii_to_unicode(pathFile.string());
   HINSTANCE hResult = ShellExecuteW(nullptr, L"open", wstringConfigurationFile.c_str(), nullptr, nullptr, SW_SHOWNORMAL); 

   auto bSuccess = ( hResult != nullptr && reinterpret_cast<intptr_t>( hResult ) > 32 );
   if( bSuccess == false )
   {
      DWORD dwError = ::GetLastError();

      return { false, "Failed to open configuration file. Error code: " + std::to_string(dwError) + " and file: " + pathFile.string() };
   }
   return { true, "" };
#else

   // Open the configuration file in the default text editor
   std::string command = "xdg-open " + pathFile.string();
   int result = system(command.c_str());

   return { result == 0, result == 0 ? "" : "Failed to open configuration file." };
#endif
}

/** --------------------------------------------------------------------------
 * @brief Gets the width of the terminal window.
 * @return The width of the terminal in characters.
 */
int SHARED_GetTerminalWidth() 
{
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO CSBI_;
    if(GetConsoleScreenBufferInfo(hConsole, &CSBI_)) 
    {
        return CSBI_.srWindow.Right - CSBI_.srWindow.Left + 1;
    }
    return 80; // Fallback width
#else
    struct winsize winsize_;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize_) == 0) 
    {
        return winsize_.ws_col;
    }
    return 80; // Fallback width
#endif
}


NAMESPACE_CLI_END // namespace CLI