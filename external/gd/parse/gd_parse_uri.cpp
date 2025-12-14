#include "gd_parse_uri.h"
#include "../gd_utf8.h"

_GD_PARSE_URI_BEGIN


/**
 * \brief Parse a URI string into its components.
 * 
 https://john.doe:password123@www.example.com:8080/path/to/resource?search=query&sort=desc#section2
 ├──┬──┘├────┬────┘├─────┬────┘├──┬──┘├─────┬─────┘├────┬────┘├─────┬─────┘├──┬──┘
 scheme  user   pass    host    port   path       query params  fragment * 
 * **sample uri to parse:**
 * - gd://example.com/path/to/resource
 * - http://www.example.com/path/to/resource
 * - https://www.example.com:8080/path/to/resource?query=param#fragment
 * - db/create?name=testdb&user=admin
 * - db/column/create?table=TUser&name=FColumnName&type=int32
 * - db/column/create?table=TUser&name=FColumnName&type=int32&key=1
 * - db/update?table=TUser&sql=UPDATE%20TUser%20SET%20FColumnName%3D100
 * 
 * Parts in html that have names like "www.example.com" are named to uri-* in argumentsUr
 */
 /*
std::pair<bool, std::string> parse( std::string_view stringUri, gd::argument::arguments& argumentsUri )
{
   // Clear any existing arguments 
   argumentsUri.clear();
   
   // Check if the URI is empty
   if( stringUri.empty() ) { return { false, "No URI provided" }; }

   return { true, "" };
}
*/

/**
 * \brief Parse a URI string into its components.
 * 
 https://john.doe:password123@www.example.com:8080/path/to/resource?search=query&sort=desc#section2
 ├──┬──┘├────┬────┘├─────┬────┘├──┬──┘├─────┬─────┘├────┬────┘├─────┬─────┘├──┬──┘
 scheme  user   pass    host    port   path       query params  fragment
 * 
 * **sample uri to parse:**
 * - gd://example.com/path/to/resource
 * - http://www.example.com/path/to/resource
 * - https://www.example.com:8080/path/to/resource?query=param#fragment
 * - db/create?name=testdb&user=admin
 * - db/column/create?table=TUser&name=FColumnName&type=int32
 * - db/column/create?table=TUser&name=FColumnName&type=int32&key=1
 * - db/update?table=TUser&sql=UPDATE%20TUser%20SET%20FColumnName%3D100
 * 
 * Parts in html that have names like "www.example.com" are named to uri-* in argumentsUri
 */
template<typename ARGUMENTS>
std::pair<bool, std::string> parse_implementation( std::string_view stringUri, ARGUMENTS& argumentsUri )
{
   const char* piPosition = stringUri.data();                                // current position in URI string
   const char* piBegin = stringUri.data();                                   // start of URI string
   const char* piEnd = stringUri.data() + stringUri.size();                  // end of URI string
   const char* piPartStart = piPosition;                                     // start of current part being parsed
   
   // ## Parse scheme (e.g., "http://", "https://", "gd://") .................
   
   const char* piSchemeEnd = nullptr;
   const char* piCheckEnd = piPosition + std::min<std::size_t>(16, piEnd - piPosition); // schemes are typically < 16 chars
   
   for( const char* pi = piPosition; pi < piCheckEnd - 2; pi++ )              // -2 to ensure we can check "://"
   {
      if( pi[0] == ':' )
      {
         if( pi[1] == '/' && pi[2] == '/' )
         {
            piSchemeEnd = pi;
            break;
         }
         else { break; }                                                      // found ':' but not "://", invalid scheme
      }
      
      // ### validate scheme character (alphanumeric, '+', '-', '.')
      // 
      if( !std::isalnum(*pi) && *pi != '+' && *pi != '-' && *pi != '.' )
      {
         break;                                                               // invalid scheme character, no scheme present
      }
   }
   
   if( piSchemeEnd != nullptr )
   {
      std::string stringScheme( piPartStart, piSchemeEnd - piPartStart );
      argumentsUri.push_back( { "scheme", stringScheme } );
      piPosition = piSchemeEnd + 3;                                           // skip "://"
      piPartStart = piPosition;
   }
   
   // ## Parse user info (user:password@) if present .........................
   
   const char* piAtSign = nullptr;
   const char* piHostStart = piPosition;
   
   for( const char* pi = piPosition; pi < piEnd && *pi != '/' && *pi != '?' && *pi != '#'; pi++ )
   {
      if( *pi == '@' ) { piAtSign = pi; break; }
   }
   
   if( piAtSign != nullptr )
   {
      // ### Parse user and password
      const char* piColon = nullptr;
      for( const char* pi = piPosition; pi < piAtSign; pi++ )
      {
         if( *pi == ':' ) { piColon = pi; break; }
      }
      
      if( piColon != nullptr )
      {
         std::string stringUser( piPosition, piColon - piPosition );
         argumentsUri.push_back( { "user", stringUser } );
         
         std::string stringPassword( piColon + 1, piAtSign - (piColon + 1) );
         argumentsUri.push_back( { "password", stringPassword } );
      }
      else
      {
         std::string stringUser( piPosition, piAtSign - piPosition );
         argumentsUri.push_back( { "user", stringUser } );
      }
      
      piPosition = piAtSign + 1;
      piHostStart = piPosition;
   }
   
   // ## Parse host and port .................................................
    
   const char* piHostEnd = piPosition;
   const char* piPortStart = nullptr;
   
   for( const char* pi = piPosition; pi < piEnd; pi++ )
   {
      if( *pi == ':' && piPortStart == nullptr )                              // found port separator
      {
         piHostEnd = pi;
         piPortStart = pi + 1;
      }
      else if( *pi == '/' || *pi == '?' || *pi == '#' )
      {
         if( piPortStart == nullptr ) piHostEnd = pi;
         piPosition = pi;
         break;
      }
      
      if( pi + 1 == piEnd )                                                   // reached end of string
      {
         if( piPortStart == nullptr ) piHostEnd = piEnd;
         piPosition = piEnd;
      }
   }
   
   if( piHostEnd > piHostStart )
   {
      std::string stringHost( piHostStart, piHostEnd - piHostStart );
      argumentsUri.push_back( { "host", stringHost } );
   }
   
   if( piPortStart != nullptr )
   {
      const char* piPortEnd = piPosition;
      std::string stringPort( piPortStart, piPortEnd - piPortStart );
      argumentsUri.push_back( { "port", stringPort } );
   }
   
   // ## Parse path
   if( piPosition < piEnd && *piPosition == '/' )
   {
      piPartStart = piPosition;
      while( piPosition < piEnd && *piPosition != '?' && *piPosition != '#' )
      {
         piPosition++;
      }
      
      if( piPosition > piPartStart )
      {
         std::string stringPath( piPartStart, piPosition - piPartStart );
         argumentsUri.push_back( { "path", stringPath } );
      }
   }
   
   // ## Parse query parameters ..............................................

   if( piPosition < piEnd && *piPosition == '?' )
   {
      piPosition++;                                                           // skip '?'
      piPartStart = piPosition;
      
      while( piPosition < piEnd && *piPosition != '#' )
      {
         piPosition++;
      }
      
      std::string stringQuery( piPartStart, piPosition - piPartStart );
      argumentsUri.push_back( { "query", stringQuery } );
      

      // ### Parse individual query parameters
      /*
      std::vector<std::pair<std::string_view, std::string_view>> vectorParams;
      gd::utf8::split_pair( piPartStart, piPosition, '=', '&', vectorParams );
      
      for( const auto& pairParam : vectorParams )
      {
         std::string stringKey( pairParam.first );
         std::string stringValue( pairParam.second );
         argumentsUri.push_back( { stringKey, stringValue } );
      }
      */
   }
   
   // ## Parse fragment ......................................................

   if( piPosition < piEnd && *piPosition == '#' )
   {
      piPosition++;                                                           // skip '#'
      std::string stringFragment( piPosition, piEnd - piPosition );
      argumentsUri.push_back( { "fragment", stringFragment } );
   }
   
   return { true, "" };
}

// Stub implementations that call the template
std::pair<bool, std::string> parse( std::string_view stringUri, gd::argument::arguments& argumentsUri )
{
   return parse_implementation( stringUri, argumentsUri );
}

std::pair<bool, std::string> parse( std::string_view stringUri, gd::argument::shared::arguments& argumentsUri )
{
   return parse_implementation( stringUri, argumentsUri );
}
_GD_PARSE_URI_END
