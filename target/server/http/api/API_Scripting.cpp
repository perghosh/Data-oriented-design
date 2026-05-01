// @FILE [tag: script, lua] [description: Script execution logic that may be used in the API system] [type: source] [name: API_Scripting.cpp]


#include "../Application.h"
#include "../lua/LUAObjects.h"
#include "../api/API_Base.h"

#include "API_Scripting.h"


SCRIPT_BEGIN

std::pair<bool, std::string> LuaRequestExecute( std::string_view stringScript, CAPIContext* pcontext_, CRENDERSql* psql, callback_lua_state callback_ )
{
   if( stringScript.empty() == true ) { return { true, "" }; }

   std::vector<gd::variant_view> vectorScript{ stringScript };
   return LuaRequestExecute( vectorScript, pcontext_, psql, callback_ );
}

/** -------------------------------------------------------------------------- LuaRequestExecute
 * @brief Execute request-scoped Lua scripts using a pooled Lua state.
 * 
 * Validates `pcontext_`, borrows the `"core"` Lua state from the pool, binds
 * request-scoped objects (`"app"`, `"doc"`, `"request"`), optionally runs
 * `callback_` to customize the Lua environment, and executes each entry in
 * `vectorScript` with `safe_script`.
 * 
 * On any callback or script error, this method returns `{ false, error }` and
 * explicitly resets bound Lua globals before returning.
 * 
 * @param vectorScript Ordered Lua script chunks to execute; empty chunks are skipped.
 * @param pcontext_ Request context with application/document and optional database.
 * @param callback_ Optional Lua-state setup callback executed before scripts run.
 * @return std::pair<bool, std::string> `first` is success, `second` is error text.
 */
std::pair<bool, std::string> LuaRequestExecute( const std::vector<gd::variant_view>& vectorScript, CAPIContext* pcontext_, CRENDERSql* psql, callback_lua_state callback_ )
{                                                                                                  assert( pcontext_ != nullptr && "LuaRequestExecute requires valid context" );
                                                                                                   assert( pcontext_->GetApplication() != nullptr && pcontext_->GetDocument() != nullptr && "LuaRequestExecute requires valid application and document" );
   CApplication* papplication_ = pcontext_->GetApplication();
   if( papplication_ == nullptr ) { return { false, "application is null in context" }; }

   LUA::LuaStatePool* pluastatepool = papplication_->LUA_GetPool();
   if( pluastatepool == nullptr || pluastatepool->Empty() == true ) { return { false, "lua pool is not initialized" }; }

   LUA::LuaStatePool::borrow borrowLuaState = pluastatepool->Acquire( "core" );// acquire "core" lua state from pool to execute code, released in destructor and cleaned up
   auto& stateLua = borrowLuaState.get_luastate();                            // get reference to sol::state for code execution

   auto* pdatabase_ = pcontext_->GetDatabase();                               // the database connection is prepared for current context, it mau be null if no database connection is needed current situation
   auto* pdocument_ = pcontext_->GetDocument();                               // get document from context, important to use this because how documents work is different and this should be prepared before running method
   std::unique_ptr<LUA::Application> papplication = std::make_unique<LUA::Application>( papplication_, pdatabase_ ); // applicaiton information
   stateLua["app"] = std::move( papplication );    
   std::unique_ptr<LUA::Document> pdocument = std::make_unique<LUA::Document>( pdocument_, pdatabase_ ); // document information, note that the database isn't same as database inside document, this is the global database.
   stateLua["doc"] = std::move( pdocument );
   std::unique_ptr<LUA::Request> prequest = std::make_unique<LUA::Request>( pcontext_, psql ); // request information, holds user data etc for current request to server.
   stateLua["request"] = std::move( prequest );

   // ## Callback is used to modify the lua state before executing the script, this can be used to set up the environment for the script
   if( callback_ )
   {
      auto result_ = callback_( &stateLua, pcontext_ );
      if( result_.first == false ) { return result_; }
   }

   std::pair<bool, std::string> result_{ true, "" };

   for( const auto& stringScript : vectorScript )
   {
      if( stringScript.empty() == true ) { continue; }

      try
      {
         stateLua.safe_script( stringScript );
      }
      catch( const sol::error& errorLua )
      {
         std::string stringError = errorLua.what();
         result_ = { false, stringError };
      }
      catch( const std::exception& exception_ )
      {
         std::string stringError = exception_.what();
         result_ = { false, stringError };
      }

      if( result_.first == false ) 
      { 
         borrowLuaState.reset( { "app", "doc", "request" }, true );
         return result_; 
      }
   }

   borrowLuaState.reset( { "app", "doc", "request" }, true ); 
   return { true, "" };
}

SCRIPT_END