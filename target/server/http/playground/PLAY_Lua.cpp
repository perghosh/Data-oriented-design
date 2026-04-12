#include <array>
#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"

#include "main.h"

#include "../lua/LUAStatePool.h"
#include "../lua/LUAObjects.h"
#include "../lua/LUABindings.h"

#include "../Document.h"
#include "../Application.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE( "[lua] first", "[lua]" )
{
   std::unique_ptr< CApplication > papplication_ = std::make_unique<CApplication>();

   auto* ppool_ = papplication_->LUA_GetPool();

   ppool_->Add( "test", []( auto& state_ ) {
      LUA::RegisterDocument( state_ );
      LUA::RegisterApplication( state_ );
   });

   auto lua_ = ppool_->Acquire("test");


   lua_->script( "print('Hello from Lua!')" );
}