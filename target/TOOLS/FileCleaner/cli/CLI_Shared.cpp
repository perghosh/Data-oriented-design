/**
 * @file CLI_Shared.cpp
 * 
 * @brief Implementation file for shared CLI operations.
 * 
 * This file contains the implementation of functions shared across CLI tools.
 */

#include <boost/regex.hpp>

#include "gd/gd_file.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"

#include "CLI_Shared.h"

NAMESPACE_CLI_BEGIN

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


/**
 * @brief Converts SQL-like filter syntax to internal expression format
 * 
 * This function transforms SQL WHERE clause syntax into an internal expression format
 * that uses source::get_argument() calls. It handles automatic value quoting, operator
 * conversion, and preserves expressions already in internal format.
 * 
 * @param stringSql Input SQL-like filter expression (read-only string view)
 * @param stringExpression Output parameter that receives the converted expression
 * 
 * @return std::pair<bool, std::string> where:
 *         - first: true if conversion succeeded, false otherwise
 *         - second: error message (empty string on success)
 * 
 * @details
 * The function performs the following transformations:
 * 1. Detects expressions already in internal format (containing "source::")
 * 2. Auto-quotes unquoted values after = and <> operators
 * 3. Converts SQL logical operators (AND/OR) to C++ operators (&&/||)
 * 4. Wraps column references in source::get_argument(args, 'column') calls
 * 5. Converts SQL comparison operators (= becomes ==, <> becomes !=)
 * 
 * Supported SQL operators:
 * - = (equality, converted to ==)
 * - <> (not equal, converted to !=)  
 * - AND/and (converted to &&)
 * - OR/or (converted to ||)
 * - Parentheses for grouping (preserved)
 * 
 * @note The function automatically quotes unquoted values but preserves
 *       already quoted strings. Column names must follow identifier rules
 *       (alphanumeric plus underscore, starting with letter or underscore).
 * 
 * @example
 * // Simple equality
 * Input:  "assigned_to = 'per'"
 * Output: "(source::get_argument(args,'assigned_to') == 'per')"
 * 
 * @example  
 * // Complex expression with logical operators
 * Input:  "(assigned_to = 'per' OR assigned_to = 'kevin') AND status = 'open'"
 * Output: "((source::get_argument(args,'assigned_to') == 'per') || (source::get_argument(args,'assigned_to') == 'kevin')) && (source::get_argument(args,'status') == 'open')"
 * 
 * @example
 * // Not equal operator
 * Input:  "status <> 'open' and assigned_to = 'per'" 
 * Output: "(source::get_argument(args,'status') != 'open') && (source::get_argument(args,'assigned_to') == 'per')"
 * 
 * @example
 * // Auto-quoting unquoted values
 * Input:  "assigned_to = per"
 * Output: "(source::get_argument(args,'assigned_to') == 'per')"
 * 
 * @example
 * // Already in internal format (passthrough)
 * Input:  "(source::get_argument(args,'status') == 'active')"
 * Output: "(source::get_argument(args,'status') == 'active')"
 * 
 * @example
 * // Multiple conditions with mixed case operators
 * Input:  "priority = high AND status <> closed OR assigned_to = admin"
 * Output: "(source::get_argument(args,'priority') == 'high') && (source::get_argument(args,'status') != 'closed') || (source::get_argument(args,'assigned_to') == 'admin')"
 */
std::pair<bool, std::string> SHARED_SqlToExpression_g(std::string_view stringSql, std::string& stringExpression)
{
   // ## Check for markers for internal raw expression format
   if( stringSql.find("source::") != std::string_view::npos ) { stringExpression = stringSql; return { true, "" };  } // if source:: is found, then it is already in internal format

     // ## Convert SQL-like syntax to internal expression format
     // sample 1 "assigned_to = 'per'" -> "(source::get_argument(args,'assigned_to') == 'per')"
     // sample 2 "(assigned_to = 'per' OR assigned_to = 'kevin') AND status = 'open'" -> "((source::get_argument(args,'assigned_to') == 'per') || (source::get_argument(args,'assigned_to') == 'kevin')) && (source::get_argument(args,'status') == 'open')"
     // sample 3 "status <> 'open' and assigned_to = 'per'" -> "(source::get_argument(args,'status') != 'open') && (source::get_argument(args,'assigned_to') == 'per')"
     // sample 4 "assigned_to = per" -> "(source::get_argument(args,'assigned_to') == 'per')" (auto-quote unquoted values)

   std::string stringResult;
   // Convert SQL-like syntax to internal expression format
   stringResult = stringSql;

   // ## Auto-quote unquoted values after = and <> operators
   // This works for multiple expressions in the same string by using global replace

   // Match: column_name = unquoted_value (handles multiple occurrences)
   stringResult = boost::regex_replace(stringResult, 
      boost::regex("([a-zA-Z_][a-zA-Z0-9_]*)\\s*=\\s*([^'\"\\s\\)\\(&&||]+)(?!['\"])"), 
      "$1 = '$2'");

   // Match: column_name <> unquoted_value (handles multiple occurrences)
   stringResult = boost::regex_replace(stringResult, 
      boost::regex("([a-zA-Z_][a-zA-Z0-9_]*)\\s*<>\\s*([^'\"\\s\\)\\(&&||]+)(?!['\"])"), 
      "$1 <> '$2'");

   // ## convert SQL operators to internal operators
   // ### Convert operators first (before column conversions)
   stringResult = boost::regex_replace(stringResult, boost::regex("\\b(OR|or)\\b"), "||");
   stringResult = boost::regex_replace(stringResult, boost::regex("\\b(AND|and)\\b"), "&&");

   // ### Convert column comparisons (now all values should be quoted)
   stringResult = boost::regex_replace(stringResult, boost::regex("([a-zA-Z_][a-zA-Z0-9_]*) = '([^']*)'"), "(source::get_argument(args,'$1') == '$2')");
   stringResult = boost::regex_replace(stringResult, boost::regex("([a-zA-Z_][a-zA-Z0-9_]*) <> '([^']*)'"), "(source::get_argument(args,'$1') != '$2')");

   stringExpression = stringResult;                         
   return { true, "" };
}


NAMESPACE_CLI_END // namespace CLI