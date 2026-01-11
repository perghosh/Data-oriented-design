// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <array>
#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"
#include "gd/gd_uuid.h"

#include "gd/parse/gd_parse_uri.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[uri] test with plain old data POD", "[uri]" ) {
   
   // ## POD STRUCT APPROACH - Fixed structure, rigid
   struct UriPOD {
      std::string scheme;
      std::string host;
      int port = 0;
      std::string path;
      std::string query;
      std::string fragment;
      std::string user;
      std::string password;
   };

   // Example 1: Using POD struct - must know all fields upfront
   UriPOD uriPod;
   uriPod.scheme = "http";
   uriPod.host = "example.com";
   uriPod.port = 8080;
   uriPod.path = "/api/users";
   uriPod.query = "limit=10&offset=20";
   uriPod.fragment = "section1";
   uriPod.user = "admin";
   uriPod.password = "secret123";

   std::cout << "## POD Struct Approach:\n";
   std::cout << "   Scheme: " << uriPod.scheme << "\n";
   std::cout << "   Host: " << uriPod.host << "\n";
   std::cout << "   Port: " << uriPod.port << "\n";
   std::cout << "   Path: " << uriPod.path << "\n";
   
   // POD limitations:
   // - Fixed structure, can't add new fields at runtime
   // - All fields always allocated even if not used
   // - Need to recompile to change structure
   // - Can't iterate over fields
   // - Can't serialize/deserialize generically

   // ## ARGUMENTS APPROACH - Dynamic, flexible
   std::array<std::byte, 512> buffer;
   gd::argument::arguments args( buffer );

   // Example 2: Using arguments - can add fields dynamically
   args["scheme"] = "http";
   args["host"] = "example.com";
   args["port"] = 8080;
   args["path"] = "/api/users";
   args["query"] = "limit=10&offset=20";
   args["fragment"] = "section1";
   args["user"] = "admin";
   args["password"] = "secret123";

   // Can add extra fields that POD struct doesn't have
   args["timeout"] = 5000;
   args["retry_count"] = 3;
   args["secure"] = true;

   std::cout << "\n## Arguments Approach:\n";
   for( auto [key, value] : args.named() ) {
      std::cout << "   " << key << ": " << value.as_string() << "\n";
   }

   // Arguments advantages:
   // - Dynamic structure, add/remove fields at runtime
   // - Type-safe access (as_string(), as_int(), as_bool(), etc.)
   // - Memory efficient (only allocated fields use space)
   // - Can iterate over all fields
   // - Built-in serialization support
   // - Can store any type (int, string, bool, pointers, etc.)

   // Comparison of accessing data:
   std::cout << "\n## Access Comparison:\n";
   
   // POD - direct member access (compile-time, fast)
   std::string podHost = uriPod.host;
   std::cout << "   POD host: " << podHost << "\n";
   
   // Arguments - key-based access (runtime, flexible)
   std::string argsHost = args["host"].as_string();
   std::cout << "   Arguments host: " << argsHost << "\n";

   REQUIRE( uriPod.host == "example.com" );
   REQUIRE( args["host"].as_string() == "example.com" );
   REQUIRE( args["port"].as_int() == 8080 );
   REQUIRE( args["secure"].as_bool() == true );
}

TEST_CASE( "[uri] arguments", "[uri]" ) {
   {
      std::array<std::byte, 256> buffer;
      gd::argument::arguments args( buffer );  // uses external buffer      
      args["age"] = 31;                     // overwrites if exists
      args.set( "level", 10 );                // same as append if not found
      args["old"] = 80;
      for( auto [key, value] : args.named() ) {
         std::cout << key << ": " << value.as_string() << "\n";
      }

      for( auto value : args.values() ) {
         std::cout <<  value.as_string() << "\n";
      }
   }
   {
      std::string* pstring1 = new std::string( "sample on how to use pointers" );
      gd::argument::arguments arguments_;
      arguments_["string"] = pstring1;
      std::string* pstring2 = arguments_["string"].get_pointer<std::string>();
      REQUIRE( *pstring2 == "sample on how to use pointers" );
      delete pstring2;
   }

   {
      std::string* pstring1 = new std::string( "sample on how to use pointers" );
      gd::argument::shared::arguments arguments_;
      arguments_["string"] = pstring1;
      std::string* pstring2 = arguments_["string"].get_pointer<std::string>();
      REQUIRE( *pstring2 == "sample on how to use pointers" );
      delete pstring2;
   }


   {
      std::vector<std::byte> vector_; // vector to hold data for arguments
      vector_.resize( 256 );
      gd::argument::arguments arguments_( vector_ );
   }
   {
      std::array<std::byte, 128> array_; // array to hold data for arguments
      gd::argument::arguments arguments_( array_ );
      arguments_["name"] = "value";
      arguments_["number"] = 42;
      std::string stringDebug = gd::argument::debug::print( arguments_ );
      std::cout << "Arguments:\n" << stringDebug << std::endl;
      REQUIRE( arguments_["name"].as_string() == "value" );
      REQUIRE( arguments_["number"].as_int() == 42 );
   }
   {
      gd::argument::shared::arguments arguments_;
      arguments_["path"] = "/some/path/to/resource";
      arguments_["enabled"] = true;
      std::string stringDebug = gd::argument::shared::debug::print( arguments_ );
      std::cout << "Shared Arguments:\n" << stringDebug << std::endl;
      REQUIRE( arguments_["path"].as_string() == "/some/path/to/resource" );
      REQUIRE( arguments_["enabled"].as_bool() == true );
   }
}

TEST_CASE( "[uri] binary", "[uri]" ) {
   {
      std::array<uint8_t, 100> binaryData;
      gd::binary::write_le binaryWriter( binaryData );
      //gd::binary::write_le binaryWriter( binaryData.data(), binaryData.size() );
      //gd::binary::write_le binaryWriter( binaryData );

      int iValue = 42;

      binaryWriter << iValue << 100 << 1000;

      gd::binary::read_le binaryReader( binaryData.data(), binaryData.size() );

      binaryReader >> iValue;
      binaryReader >> iValue;
      binaryReader >> iValue;

      gd::uuid uuid(gd::uuid::tag_null{});

      binaryWriter.write_bytes( uuid, 16 );

      gd::uuid uuidRead;
      binaryReader.read_bytes( uuidRead.data(), 16);

      assert( uuid.compare( uuidRead ) );
   }

   {
      using namespace gd;
      std::string stringHex20 = "00000001445566778899AABBCCDDEEFF00112233";
      auto result_ = binary_validate_hex_g( stringHex20 ); REQUIRE( result_.first == true );
      std::vector<uint8_t> vectorBuffer;
      // set vector size to hold 20 bytes
      vectorBuffer.resize( stringHex20.length() / 2 );
      binary_copy_hex_g( vectorBuffer.data(), stringHex20); REQUIRE(vectorBuffer.size() == 20);

      gd::binary::read_be binaryReader( vectorBuffer );
      uint32_t uValue1 = binaryReader.read<uint32_t>(); REQUIRE( uValue1 == 0x00000001 );
      gd::uuid uuidRead;
      binaryReader.read_bytes( uuidRead.data(), 16 );
      gd::uuid uuidExpected( (const uint8_t*)"\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC\xDD\xEE\xFF\x00\x11\x22\x33" );
      REQUIRE( uuidRead.compare( uuidExpected ) );
   }

   {
      using namespace gd;
      std::array<uint8_t, 100> buffer_;

      binary::write_be write_( buffer_ );
      std::string string_("0123456789");
      write_.write_container( string_ );

      binary::read_be read_( buffer_ );
      std::string stringRead;
      read_.read_container( stringRead );
      REQUIRE( stringRead == string_ );
   }

}

TEST_CASE("[uri] test uri logic", "[uri]") 
{
    return;
   std::cout << "test uri logic" << std::endl;

   // test with gd::argument::arguments

   {
       
      gd::argument::arguments argumentsUri;
      std::string uri = "http://username:password@hostname:9090/path?arg=value#anchor";
      std::cout << "\n\n## " << uri << "\n" << std::endl;
      auto [ success, error ] = gd::parse::uri::parse( uri, argumentsUri );                        REQUIRE( success == true );

      std::string stringUriResult = gd::argument::debug::print( argumentsUri );
      std::cout << "Parsed URI arguments:\n" << stringUriResult << std::endl;

      REQUIRE( argumentsUri[ "scheme" ].as_string() == "http");
      REQUIRE( argumentsUri[ "user" ].as_string() == "username" );
      REQUIRE( argumentsUri[ "password" ].as_string() == "password" );
      REQUIRE( argumentsUri[ "host" ].as_string() == "hostname" );
      REQUIRE( argumentsUri[ "port" ].as_int() == 9090 );
      REQUIRE( argumentsUri[ "path" ].as_string() == "/path" );
      REQUIRE( argumentsUri[ "query" ].as_string() == "arg=value" );
      REQUIRE( argumentsUri[ "fragment" ].as_string() == "anchor" );
   }
   {
      gd::argument::arguments argumentsUri;
      std::string uri = "https://example.com";
      auto [ success, error ] = gd::parse::uri::parse( uri, argumentsUri );                        REQUIRE( success == true );

      std::string stringUriResult = gd::argument::debug::print( argumentsUri );
      std::cout << "Parsed URI arguments:\n" << stringUriResult << std::endl;

      REQUIRE( argumentsUri[ "scheme" ].as_string() == "https" );
      REQUIRE( argumentsUri[ "host" ].as_string() == "example.com" );
   }

   // test with gd::argument::shared::arguments
   {
      gd::argument::shared::arguments argumentsUri;
      std::string uri = "ftp://ftp.example.com/resource.txt";
      std::cout << "\n\n## " << uri << "\n" << std::endl;
      auto [ success, error ] = gd::parse::uri::parse( uri, argumentsUri );                        REQUIRE( success == true );
      std::string stringUriResult = gd::argument::shared::debug::print( argumentsUri );
      std::cout << "Parsed URI arguments:\n" << stringUriResult << std::endl;
      REQUIRE( argumentsUri[ "scheme" ].as_string() == "ftp" );
      REQUIRE( argumentsUri[ "host" ].as_string() == "ftp.example.com" );
      REQUIRE( argumentsUri[ "path" ].as_string() == "/resource.txt" );
   }


   // ## 10 different URIs to test parsing in loop
   std::vector<std::string> testUris = {
      "ftp://ftp.example.com/resource.txt",
      "mailto:user@example.com",
      "file:///home/user/document.txt",
      "https://secure.example.com:443/path/to/resource",
      "http://localhost:8080/test?arg=value#fragment",
      "http://localhost:8080/one/two/three/four?arg=value&arg=value&arg=value&arg1=value#fragment",
      "ws://websocket.example.com/socket"
   };

   // ## test with both argument and shared::arguments
   for (const auto& uri : testUris) {
      {
         gd::argument::arguments argumentsUri;
         std::cout << "\n\n## " << uri << "\n" << std::endl;
         auto [ success, error ] = gd::parse::uri::parse( uri, argumentsUri );                     REQUIRE( success == true );
         std::string stringUriResult = gd::argument::debug::print( argumentsUri );
         std::cout << "Parsed URI arguments:\n" << stringUriResult << std::endl;
      }
      {
         gd::argument::shared::arguments argumentsUri;
         std::cout << "\n\n## " << uri << "\n" << std::endl;
         auto [ success, error ] = gd::parse::uri::parse( uri, argumentsUri );                     REQUIRE( success == true );
         std::string stringUriResult = gd::argument::shared::debug::print( argumentsUri );
         std::cout << "Parsed URI arguments:\n" << stringUriResult << std::endl;
      }
   }



}
