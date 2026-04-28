// @FILE [tag: main] [description: main entry point for the server application] [type: source] [name: main.cpp]

#include <iostream>

#include "Application.h"

#ifdef TARGET_COMPILER__LEAKS_CHECK
#if defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>

namespace
{
   // Tip: Use AllocHook_d and values there to set breakpoint  or find where to start debug code.

   /// @brief win32_leak_check_ is a helper class to enable memory leak check in debug mode
   struct win32_leak_check_
   {
      win32_leak_check_()
      {
         // ## Use "_CrtSetDbgFlag" - Windows C Runtime (CRT) method, acts as a master control switch for the debug heap manager
         const int iDebugFlags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );       // get current debug flags
         _CrtSetDbgFlag( iDebugFlags | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // enable debug heap allocations and automatic leak check at program exit
      }
   };

   win32_leak_check_ win32leakcheck_g; // global instance to enable memory leak check in debug mode

   /// @brief AllocHook_d is called for each allocation and deallocation, you can set break point here and inspect call stack to find where allocation is happening, you can also check size of allocation and only break when specific size is allocated, this is useful when you have memory leak dump and you want to find where specific allocation is happening
   static int AllocHook_d(int iAllocType, void* /*pUserData*/, size_t uSize, int /*iBlockType*/, long iRequestNumber, const unsigned char* /*pFilename*/, int /*iLineNumber*/)
   {
       if( iAllocType == _HOOK_ALLOC && uSize == 320 )
       {
          std::cout << "## AllocHook: alloc# " << iRequestNumber << ", size " << uSize << std::endl;
           //__debugbreak();                                                    // attach debugger and inspect call stack here
       }
       return TRUE;
   }
}
#endif // defined(_WIN32) && defined(_MSC_VER) && defined(_DEBUG)
#endif // TARGET_COMPILER__LEAKS_CHECK


/** --------------------------------------------------------------------------- @API [tag: main] [description: start application] [type: method]
 * @brief man is as you know where the application starts
 */
int main( int iArgumentCount, char* ppbszArgument[] )
{
// At the top of main(), before anything else
#if defined(_MSC_VER) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetAllocHook(AllocHook_d);
    //_CrtSetBreakAlloc(12500);   // lowest alloc# set break att specific allocation number you can find this number in memory leak dump
#endif

   // ## Initialize application and configure to get the server running
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();
   papplication_g = papplication_.get();

   //std::cout << "we have a message";

   auto result_ =  papplication_->Main( iArgumentCount, ppbszArgument, nullptr );

   if( result_.first == false )
   {
      std::cout << "## Server exit with error: "  << result_.second << std::endl;
   }

   return 0;
}

