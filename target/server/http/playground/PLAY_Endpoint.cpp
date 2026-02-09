// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_io.h"

#include "../render/RENDERSql.h"

#include "../Session.h"
#include "../Router.h"
#include "../Document.h"
#include "../Application.h"

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#endif


#include "main.h"

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

// @TODO Clean upp static objects related to document, search for "if( m_pcolumnsBody_s == nullptr )"

TEST_CASE("[session] insert", "[session]") 
{
   {
      std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();

      std::string stringFolder = FOLDER_GetRoot_g( "target/server/http/playground/data" );
      papplication_->PROPERTY_Add("folder-application", stringFolder );
      auto [bOk, stringError] = papplication_->Initialize();                                      REQUIRE( bOk == true );

      /// Get property values from application, these are set in configuration.xml
      gd::argument::arguments argumentsDatabase = papplication_->PROPERTY_Get({"database-meta-tables", "database-meta-columns", "database-open"}, gd::types::tag_argument{});

      std::string stringArguments_d = gd::argument::debug::print( argumentsDatabase ); // print informationn harvested from application properties
      std::tie(bOk, stringError) = papplication_->DATABASE_Connect(argumentsDatabase);            REQUIRE( bOk == true );

      CRENDERSql rendersql_( gd::sql::sql_get_dialect_g("sqlite") );
      rendersql_.Initialize();

      std::array< std::byte, 256> buffer_;
      gd::argument::arguments argumentsField( buffer_ );

      argumentsField.set( {{"table", "TPollQuestion"}, {"column", "FName"}, {"value", "name value"}} );
      rendersql_.AddValue( argumentsField );

      argumentsField.set( {{"table", "TPollQuestion"}, {"column", "FDescription"}, {"value", "input value"}} );
      rendersql_.AddValue( argumentsField );

      argumentsField.set( {{"table", "TPollQuestion"}, {"column", "PollK"}, {"type", (uint32_t)gd::types::type_g("binary")}, {"value", "1641AC8D3C4DAEB196655AEEF79F30DA"}});
      rendersql_.AddValue( argumentsField );


      std::string stringInsert;
      std::tie( bOk, stringError) = rendersql_.ToSqlInsert( stringInsert );                       REQUIRE( bOk == true );

      CRENDERSql::Destroy_s();
   }
}



TEST_CASE("[session] test uri logic", "[session]") 
{
   using namespace gd::log;
   gd::log::logger<0>* plogger = gd::log::get_s();                           // get pointer to logger 0

   // Take a memory snapshot at the start
   #ifdef _WIN32
   _CrtMemState memStateStart, memStateEnd, memStateDiff;
   _CrtMemCheckpoint(&memStateStart);

   //_CrtSetBreakAlloc(1223);
   //_CrtSetBreakAlloc(4850);
   #endif

   {
      std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
      papplication_g = papplication_.get();

      std::string stringFolder = FOLDER_GetRoot_g( "target/server/http/playground/data" );
      papplication_g->PROPERTY_Add("folder-application", stringFolder );

      {
         /// Initialize application, tries to set up as much as possible for whats been set to the application
         auto [bOk, stringError] = papplication_g->Initialize();                                   REQUIRE( bOk == true );

         auto* pdocument = papplication_g->GetDocument();
         pdocument->SESSION_Initialize( 12 );

         /// Get property values from application, these are set in configuration.xml
         gd::argument::arguments argumentsDatabase = papplication_g->PROPERTY_Get({"database-meta-tables", "database-meta-columns", "database-open"}, gd::types::tag_argument{});

         std::string stringArguments_d = gd::argument::debug::print( argumentsDatabase ); // print informationn harvested from application properties
         std::tie(bOk, stringError) = papplication_g->DATABASE_Connect(argumentsDatabase);            REQUIRE( bOk == true );
      }

      //std::string stringEnpoint = "!sys/meta/db/fields?table=TUser&field=FAlias,FMail&session=01";
      std::string stringEnpoint = "!sys/session/add//sys/meta/db/fields?table=TUser&field=FAlias,FMail&session=01";
      CRouter router_(papplication_g, stringEnpoint);
      auto result_ = router_.Parse();                                                              REQUIRE( result_.first == true );
      result_ = router_.Run();                                                                     REQUIRE( result_.first == true );
   }

   // Take a memory snapshot at the end and compare
   #ifdef _WIN32
   _CrtMemCheckpoint(&memStateEnd);
   if (_CrtMemDifference(&memStateDiff, &memStateStart, &memStateEnd)) {
       _CrtMemDumpStatistics(&memStateDiff);
       _CrtMemDumpAllObjectsSince(&memStateStart);
   }
   #endif
}