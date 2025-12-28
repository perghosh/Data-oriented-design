// @FILE [tag: cli, http] [description: Handle http configuration from terminal] [type: source] [name: CLIHttp.cpp]

#include <cassert>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "gd/gd_utf8.h"
#include "gd/gd_variant_view.h"

#include "CLIHttp.h"


NAMESPACE_CLI_BEGIN

static std::pair<bool, std::string> HttpValidateAndExpand_s( std::vector<std::string>& vectorUuid );

std::pair<bool, std::string> Http_g(const gd::cli::options* poptionsHttp, CDocument* pdocument)
{                                                                                                  assert( poptionsHttp != nullptr ); assert( pdocument != nullptr );
   // ## set http server properties based on options .........................

   std::vector<gd::variant_view> vectorSession_ = poptionsHttp->get_all( "add-session" ); // ensure value is parsed
   std::vector<std::string> vectorSessionNoSplit;
   std::vector<std::string> vectorSession;

   for( const auto& v_ : vectorSession_ )
   { 
      vectorSessionNoSplit.push_back( v_.as_string() );
   }

   for( const auto& s_ : vectorSessionNoSplit ) 
   { 
      const auto v_ = gd::utf8::split( s_, ',' );
      for( const auto& s_ : v_ ) { vectorSession.push_back( std::string( s_ ) ); }
   }

   auto result_ = HttpValidateAndExpand_s( vectorSession );                   // validate and expand uuids

   // ## Add session values to document ......................................

   return { true, "" };
}


/** --------------------------------------------------------------------------
 * @brief Validates and expands a vector of UUID strings, returning a status and an optional error message.
 * @param vectorUuid A reference to a vector of UUID strings to be validated and potentially expanded. Padded UUIDs will be modified in place with zero-padding if necessary.
 * @return A pair consisting of a boolean indicating success or failure, and a string containing an error message if validation fails (empty if successful).
 */
std::pair<bool, std::string> HttpValidateAndExpand_s( std::vector<std::string>& vectorUuid )
{
   for( auto& stringUuid : vectorUuid )
   {
      // Remove any whitespace from the UUID
      stringUuid.erase(std::remove_if(stringUuid.begin(), stringUuid.end(), ::isspace), stringUuid.end());
      
      // ## Check if UUID is empty after removing whitespace
      if(stringUuid.empty() == true ) { return { false, "Empty UUID provided" }; }
      
      // ## Check if the UUID needs padding (UUIDs are typically 32 characters without hyphens)
      if(stringUuid.length() < 32) 
      {
         // Pad with leading zeros
         stringUuid = std::string(32 - stringUuid.length(), '0') + stringUuid;
      }
      else if(stringUuid.length() > 32 && stringUuid.find('-') == std::string::npos) 
      {
         return { false, "Invalid UUID format: too long without hyphens" };
      }
      
      
      // If UUID contains hyphens, remove them for consistent processing
      if(stringUuid.find('-') != std::string::npos) 
      {
         std::string::iterator end_pos = std::remove(stringUuid.begin(), stringUuid.end(), '-');
         stringUuid.erase(end_pos, stringUuid.end());
         
         // Check if it has the correct length after removing hyphens
         if(stringUuid.length() != 32) { return { false, "Invalid UUID format: incorrect length" }; }
      }
      
      // Validate that all characters are hexadecimal
      for (char c : stringUuid) 
      {
         if(!std::isxdigit(c)) { return { false, "Invalid UUID format: contains non-hexadecimal characters" }; }
      }
   }
   return { true, "" };
}


NAMESPACE_CLI_END