#include "LuaStatePool.h"

LUA_BEGIN

// ----------------------------------------------------------------------------
// --------------------------------------------------------------- LuaStatePool
// ----------------------------------------------------------------------------

LuaStatePool::LuaStatePool(size_t uMaxPoolSize, size_t uHardConcurrencyLimit )
   : m_uMaxPoolSize{ uMaxPoolSize > 0 ? uMaxPoolSize : std::thread::hardware_concurrency() }
   , m_uHardConcurrencyLimit{ uHardConcurrencyLimit }
{
   m_vectorStates.reserve( m_uMaxPoolSize );
}


// ## public interface ---------------------------------------------------------

LuaStatePool::state& LuaStatePool::emplace( std::string_view stringName, std::function<void(sol::state&)> callbackRegister )
{
   auto psolstateLua = create_state( callbackRegister );

   std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };

   auto& pstate_ = m_vectorStates.emplace_back( std::make_unique<state>( next_id(), std::string{ stringName }, std::move( psolstateLua ) ) );

   return *pstate_;
}

void LuaStatePool::emplace( std::string_view stringName, size_t uCount, std::function<void(sol::state&)> callbackRegister )
{
   for( size_t u = 0; u < uCount; ++u )
   {
      emplace( stringName, callbackRegister );
   }
}

LuaStatePool::borrow LuaStatePool::acquire( std::string_view stringName )
{
   while( true )
   {
      {
         std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };

         for( auto& pstate_ : m_vectorStates )
         {
            if( pstate_->m_stringName != stringName ) { continue; }

            bool bExpectedIdle = false;
            if( pstate_->m_bInUse.compare_exchange_strong(
                   bExpectedIdle, true,
                   std::memory_order_acquire,
                   std::memory_order_relaxed ) )
            {
               return borrow{ *pstate_ };
            }
         }

         // ## all matching states busy — check hard limit before blocking
         if( m_uHardConcurrencyLimit == 0 )
         {
            // unbounded: no matching idle state found and we cannot create more here
            // (creation requires a register callback; fall through to wait)
         }
      }

      // ## block until any state is released, then retry
      std::unique_lock<std::mutex> lockWait{ m_mutexPool };
      m_conditionStateAvailable.wait( lockWait, [&]
      {
         for( auto& pstate_ : m_vectorStates )
         {
            if( pstate_->m_stringName == stringName && !pstate_->m_bInUse.load( std::memory_order_relaxed ) )
               return true;
         }
         return false;
      });
   }
}

bool LuaStatePool::erase( uint64_t uId )
{
   // ## wait outside the lock until the target state is idle, then erase
   while( true )
   {
      std::unique_lock<std::mutex> lockguardPool{ m_mutexPool };

      auto it = std::find_if( m_vectorStates.begin(), m_vectorStates.end(),
         [uId]( const std::unique_ptr<state>& pstate_ ) { return pstate_->m_uId == uId; }
      );

      if( it == m_vectorStates.end() ) { return false; }                        // id not found

      state& stateTarget = **it;

      bool bExpectedInUse = false;
      if( stateTarget.m_bInUse.compare_exchange_strong(
             bExpectedInUse, true,                                               // claim it so no new borrow can sneak in
             std::memory_order_acquire,
             std::memory_order_relaxed ) )
      {
         reset_state( *stateTarget.m_psolstateLua );
         m_vectorStates.erase( it );                                             // unique_ptr destructor destroys state + sol::state
         m_conditionStateAvailable.notify_all();
         return true;
      }

      // ## state is in use — wait for it to be released, then retry
      m_conditionStateAvailable.wait( lockguardPool, [&stateTarget]
      {
         return !stateTarget.m_bInUse.load( std::memory_order_relaxed );
      });
   }
}

size_t LuaStatePool::size() const
{
   std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };
   return m_vectorStates.size();
}

bool LuaStatePool::empty() const
{
   std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };
   return m_vectorStates.empty();
}

void LuaStatePool::set_reset_callback( std::function<void(sol::state&)> callbackReset )
{
   std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };
   m_callbackReset = std::move( callbackReset );
}


// ## internal helpers ---------------------------------------------------------

std::unique_ptr<sol::state> LuaStatePool::create_state( const std::function<void(sol::state&)>& callbackRegister )
{
   auto psolstateLua = std::make_unique<sol::state>();

   psolstateLua->open_libraries(
      sol::lib::base, sol::lib::coroutine, sol::lib::string,
      sol::lib::table, sol::lib::math, sol::lib::io
   );

   psolstateLua->set_exception_handler( &OnError );

   callbackRegister( *psolstateLua );                                            // caller controls exactly what gets registered

   return psolstateLua;
}

void LuaStatePool::reset_state( sol::state& solstateLua )
{
   if( m_callbackReset )
   {
      m_callbackReset( solstateLua );
      return;
   }

   // ## default: clear known per-request globals
   solstateLua["app"]     = sol::lua_nil;
   solstateLua["user"]    = sol::lua_nil;
   solstateLua["request"] = sol::lua_nil;

   solstateLua.gc();                                                             // reclaim memory held by previous script's temporaries
}

uint64_t LuaStatePool::next_id() { return m_uNextStateId.fetch_add( 1, std::memory_order_relaxed ); }


LUA_END
