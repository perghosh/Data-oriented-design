// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#include <psapi.h>
#endif
#include "catch2/catch_amalgamated.hpp"

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_io.h"

#include "../Session.h"
#include "../Document.h"
#include "../Application.h"

#include "main.h"

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif



TEST_CASE("[session] test uri logic", "[session]") 
{
   using namespace gd::log;
   gd::log::logger<0>* plogger = gd::log::get_s();                           // get pointer to logger 0

   // Take a memory snapshot at the start
   #ifdef _WIN32
   _CrtMemState memStateStart, memStateEnd, memStateDiff;
   _CrtMemCheckpoint(&memStateStart);

   //_CrtSetBreakAlloc(1223);
   //_CrtSetBreakAlloc(1213);
   #endif

   {
      std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
      papplication_g = papplication_.get();

      std::string stringFolder = FOLDER_GetRoot_g( "target/server/http/playground/data" );
      papplication_g->PROPERTY_Add("folder-application", stringFolder );

      {
         auto [bOk, stringError] = papplication_g->Initialize();                                      REQUIRE( bOk == true );

         std::string stringConfigurationFile = stringFolder + "/configuration.xml";                   REQUIRE( std::filesystem::exists( stringConfigurationFile ) == true );

         gd::argument::arguments argumentsDatabase = papplication_g->PROPERTY_Get({"database-meta-tables", "database-meta-columns", "database-open"}, gd::types::tag_argument{});

         std::string stringArguments_d = gd::argument::debug::print( argumentsDatabase );
         std::tie(bOk, stringError) = papplication_g->DATABASE_Connect(argumentsDatabase);            REQUIRE( bOk == true );
      }
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