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
   
   const char* piAtSign = nullptr; // position of '@' if user info is present
   const char* piHostStart = piPosition; // start of host (after user info if present)
   
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
         std::string_view stringUser( piPosition, piColon - piPosition );
         argumentsUri.push_back( { "user", stringUser } );
         
         std::string_view stringPassword( piColon + 1, piAtSign - (piColon + 1) );
         argumentsUri.push_back( { "password", stringPassword } );
      }
      else
      {
         std::string_view stringUser( piPosition, piAtSign - piPosition );
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
      std::string_view stringHost( piHostStart, piHostEnd - piHostStart );
      argumentsUri.push_back( { "host", stringHost } );
   }
   
   if( piPortStart != nullptr )
   {
      const char* piPortEnd = piPosition;
      std::string_view stringPort( piPortStart, piPortEnd - piPortStart );
      argumentsUri.push_back( { "port", stringPort } );
   }
   
   // ## Parse path ..........................................................

   if( piPosition < piEnd && *piPosition == '/' )
   {
      piPartStart = piPosition;
      while( piPosition < piEnd && *piPosition != '?' && *piPosition != '#' ) { piPosition++; }
      
      if( piPosition > piPartStart )
      {
         std::string_view stringPath( piPartStart, piPosition - piPartStart );
         argumentsUri.push_back( { "path", stringPath } );
      }
   }
   
   // ## Parse query parameters ..............................................

   if( piPosition < piEnd && *piPosition == '?' )
   {
      piPosition++;                                                           // skip '?'
      piPartStart = piPosition;
      
      while( piPosition < piEnd && *piPosition != '#' ) { piPosition++; }
      
      std::string_view stringQuery( piPartStart, piPosition - piPartStart );
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

/** --------------------------------------------------------------------------
 * \brief Parse URI path into segments
 * 
 * Splits a URI path like "/one/two/three/four" into individual segments.
 * Leading slash is skipped, empty segments are ignored.
 * 
 * @param stringPath The path string to parse (e.g., "/one/two/three")
 * @param vectorSegments Output vector to store path segments
 * @return Pair of success flag and error message
 * 
 * Example:
 *   "/one/two/three/four" -> ["one", "two", "three", "four"]
 *   "/path" -> ["path"]
 *   "/" -> []
 */
std::pair<bool, std::string> parse_path( std::string_view stringPath, std::vector<std::string_view>& vectorSegments )
{
   if( stringPath.empty() ) return { true, "" };
   
   const char* piStart = stringPath.data(); // start of path string
   const char* piEnd = stringPath.data() + stringPath.size(); // end of path string
   const char* piPosition = piStart; // current position in path string
   
   // Skip leading slash
   if( piPosition < piEnd && *piPosition == '/' ) { piPosition++; }
   
   const char* piSegmentStart = piPosition;
   
   while( piPosition < piEnd )
   {
      if( *piPosition == '/' )
      {
         // Found segment separator, note that two segment separators that follows will add empty string
         if( piPosition >= piSegmentStart ) { vectorSegments.emplace_back( piSegmentStart, piPosition - piSegmentStart ); }
         piPosition++;
         piSegmentStart = piPosition;
      }
      else { piPosition++; }
   }
   
   // Add final segment if exists
   if( piPosition > piSegmentStart )
   {
      vectorSegments.emplace_back( piSegmentStart, piPosition - piSegmentStart );
   }
   
   return { true, "" };
}

/** --------------------------------------------------------------------------
 * \brief Parse URI query string into arguments (template implementation)
 * 
 * Parses query parameters like "arg=value&arg1=value&name=test" into key-value pairs.
 * Both keys and values are stored. Handles URL-encoded values.
 * 
 * @param stringQuery The query string to parse (without leading '?')
 * @param argumentsQuery Output arguments object to store parsed parameters
 * @return Pair of success flag and error message
 * 
 * Example:
 *   "arg=value&arg1=value2" -> {{"arg", "value"}, {"arg1", "value2"}}
 *   "name=test&id=123" -> {{"name", "test"}, {"id", "123"}}
 */
template<typename ARGUMENTS>
std::pair<bool, std::string> parse_query_implementation( std::string_view stringQuery, ARGUMENTS& argumentsQuery )
{
   if( stringQuery.empty() ) return { true, "" };
   
   const char* piStart = stringQuery.data();
   const char* piEnd = stringQuery.data() + stringQuery.size();
   const char* piPosition = piStart;
   
   while( piPosition < piEnd )
   {
      // ## Find key .........................................................
      const char* piKeyStart = piPosition; // start of key
      const char* piKeyEnd = piPosition;   // end of key
      
      while( piPosition < piEnd && *piPosition != '=' && *piPosition != '&' ) { piPosition++; }
      piKeyEnd = piPosition;
      
      if( piKeyEnd > piKeyStart )
      {
         std::string_view stringKey( piKeyStart, piKeyEnd - piKeyStart );
         
         // Check if we have a value
         if( piPosition < piEnd && *piPosition == '=' )
         {
            piPosition++;                                                     // skip '='
            
            const char* piValueStart = piPosition; // start of value
            const char* piValueEnd = piPosition;   // end of value
            
            while( piPosition < piEnd && *piPosition != '&' ) { piPosition++; } // read until '&' or end
            piValueEnd = piPosition;
            
            // ## Check value for uri encoded characters ....................
            std::u8string_view stringValue( reinterpret_cast<const char8_t*>( piValueStart ), piValueEnd - piValueStart );
            if( gd::utf8::uri::next_sequence( stringValue ) == nullptr ) argumentsQuery.push_back( { stringKey, stringValue } );
            else
            {
               std::string stringDecodedValue = gd::utf8::uri::convert_uri_to_uf8( stringValue );
               argumentsQuery.push_back( { stringKey, stringDecodedValue } );
            }
         }
         else { argumentsQuery.push_back( { stringKey, std::string_view{""} } ); } // Key without value
      }
      
      // Skip '&' separator
      if( piPosition < piEnd && *piPosition == '&' ) { piPosition++; }
   }
   
   return { true, "" };
}

// Stub implementations for parse_query
std::pair<bool, std::string> parse_query( std::string_view stringQuery, gd::argument::arguments& argumentsQuery )
{
   return parse_query_implementation( stringQuery, argumentsQuery );
}

std::pair<bool, std::string> parse_query( std::string_view stringQuery, gd::argument::shared::arguments& argumentsQuery )
{
   return parse_query_implementation( stringQuery, argumentsQuery );
}


/** --------------------------------------------------------------------------
 * \brief Parse path and query string into segments and arguments
 * 
 * Parses a combined path and query string like "/one/two?arg=value&name=test"
 * into path segments and query arguments.
 * 
 * @param stringPathAndQuery The path and query string (e.g., "/path/to/resource?arg=value")
 * @return Pair containing vector of path segments and arguments object with query parameters
 * 
 * Example:
 *   "/one/two?arg=value&name=test" -> {["one", "two"], {{"arg", "value"}, {"name", "test"}}}
 *   "/path" -> {["path"], {}}
 *   "?arg=value" -> {[], {{"arg", "value"}}}
 */
std::pair<std::vector<std::string_view>, gd::argument::arguments> parse_path_and_query( std::string_view stringPathAndQuery )
{
   std::vector<std::string_view> vectorSegments;
   gd::argument::arguments argumentsQuery;
   
   if( stringPathAndQuery.empty() == true ) { return { vectorSegments, argumentsQuery }; }
   
   const char* piStart = stringPathAndQuery.data(); // start of path and query string
   const char* piEnd = stringPathAndQuery.data() + stringPathAndQuery.size(); // end of path and query string
   const char* piPosition = piStart; // current position in string
   
   // ## Find the query separator '?' .......................................

   const char* piQueryStart = nullptr;
   for( const char* pi = piStart; pi < piEnd; pi++ )
   {
      if( *pi == '?' )
      {
         piQueryStart = pi;
         break;
      }
   }
   
   // ## Parse path section .................................................

   if( piQueryStart != nullptr )
   {
      // Path exists before '?'
      std::string_view stringPath( piStart, piQueryStart - piStart );
      if( !stringPath.empty() )
      {
         parse_path( stringPath, vectorSegments );
      }
      
      // Parse query section after '?'
      if( piQueryStart + 1 < piEnd )
      {
         std::string_view stringQuery( piQueryStart + 1, piEnd - (piQueryStart + 1) );
         parse_query_implementation( stringQuery, argumentsQuery );
      }
   }
   else { parse_path( stringPathAndQuery, vectorSegments ); }
   
   return { vectorSegments, argumentsQuery };
}

_GD_PARSE_URI_END
