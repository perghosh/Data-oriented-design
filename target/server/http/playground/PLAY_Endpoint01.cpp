// @FILE [tag: strstr, playground] [description: Key value and finding things in string] [type: playground]

#include <filesystem>
#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <windows.h>
#include <psapi.h>
#endif

#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_io.h"

#include "../Session.h"
#include "../Document.h"
#include "../Application.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[session] test uri logic", "[session]") 
{
   // 1. Take a snapshot of the memory state at the start of the test
   //_CrtMemState crtmemstateStart;
   //_CrtMemCheckpoint(&crtmemstateStart);

   // ────────────────────────────────────────────────
   // Enable CRT memory leak checking + file/line info
   // ────────────────────────────────────────────────
   /*
#ifdef _DEBUG
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

   // Optional: also want file/line in leak report (works best with the macro below)
   // You can also do: _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif

   // ────────────────────────────────────────────────
   // Optional: redefine new/delete so leak reports show real source line
   // Put this in a common header if you want it project-wide
   // ────────────────────────────────────────────────
#ifdef _DEBUG
   #define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
   #define new DEBUG_NEW
#endif

   _CrtSetBreakAlloc(234);
   */

   {
      std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
      papplication_g = papplication_.get();

      std::string stringFolder = FOLDER_GetRoot_g( "target/server/http/playground/data" );
      papplication_g->PROPERTY_Add("folder-application", stringFolder );

      {
         auto [bOk, stringError] = papplication_g->Initialize();                                      REQUIRE( bOk == true );

         /*
         std::string stringConfigurationFile = stringFolder + "/configuration.xml";                   REQUIRE( std::filesystem::exists( stringConfigurationFile ) == true );

         gd::argument::arguments argumentsDatabase = papplication_g->PROPERTY_Get({"database-meta-tables", "database-meta-columns", "database-open"}, gd::types::tag_argument{});

         std::string stringArguments_d = gd::argument::debug::print( argumentsDatabase );
         std::tie(bOk, stringError) = papplication_g->DATABASE_Connect(argumentsDatabase);            REQUIRE( bOk == true );
         */
      }
   }

   // ────────────────────────────────────────────────
   // Optional: explicit leak check right after the scope (useful when you have many exit paths)
   // ────────────────────────────────────────────────
   /*
#ifdef _DEBUG
   if (_CrtDumpMemoryLeaks())
   {
      // You can fail the test if you want strict "no leaks" policy
      // FAIL("Memory leak(s) detected — see Output window!");
      
      // or just warn / log
      WARN("Memory leak(s) detected — check Visual Studio Output window (Debug tab)");
   }
#endif
*/

   /*
   // 2. Take a snapshot of the memory state at the end of the test
   _CrtMemState crtmemstateEnd;
   _CrtMemCheckpoint(&crtmemstateEnd);

   // 3. Calculate the difference between the two states
   _CrtMemState crtmemstateDiff;
   if(_CrtMemDifference(&crtmemstateDiff, &crtmemstateStart, &crtmemstateEnd))
   {
      // 4. If there is a difference, output the details to the Visual Studio Debug Window
      // This prints the number of allocations and their total size.
      _CrtMemDumpStatistics(&crtmemstateDiff);
      
      // OPTIONAL: To see the exact allocation line numbers, you can uncomment the line below.
      // Note: This requires the #define _CRTDBG_MAP_ALLOC at the top of the file.
      _CrtDumpMemoryLeaks(); 

      // 5. Fail the test explicitly so you know leaks occurred
      FAIL("Memory leaks detected in this test case. Check the 'Output' window for details.");
   }
   */
}