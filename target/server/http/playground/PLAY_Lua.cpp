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

	std::unique_ptr<LUA::Application> papplication = std::make_unique<LUA::Application>( papplication_.get() );
	lua_.get_luastate()["app"] = std::move( papplication );											 // set global variable named "app" to prepared application

   lua_->script( R"(
   local iPropertyCount = app:GetPropertyCount()
   print( "Property count: " .. iPropertyCount )
   )" );
}