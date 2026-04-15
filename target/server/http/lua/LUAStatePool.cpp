#include "LUAStatePool.h"

LUA_BEGIN

// ----------------------------------------------------------------------------
// --------------------------------------------------------------- LuaStatePool
// ----------------------------------------------------------------------------

/**  -------------------------------------------------------------------------- LuaStatePool
 * @brief Construct an empty pool.  Use `emplace()` to add named states,
 *        or call `emplace( stringName, uCount )` to pre-warm a group.
 *
 * @param uMaxPoolSize           Upper bound on the number of states the pool
 *                               may hold simultaneously.  0 = `hardware_concurrency`.
 * @param uHardConcurrencyLimit  Maximum states that may be *in use* at once.
 *                               0 = unbounded.  When the limit is reached,
 *                               `acquire()` blocks until one is released.
 */
LuaStatePool::LuaStatePool(size_t uMaxPoolSize, size_t uHardConcurrencyLimit ): m_uMaxPoolSize{ uMaxPoolSize > 0 ? uMaxPoolSize : std::thread::hardware_concurrency() }, m_uHardConcurrencyLimit{ uHardConcurrencyLimit }
{
   m_vectorStates.reserve( m_uMaxPoolSize );
}


// ## public interface ---------------------------------------------------------

/**  -------------------------------------------------------------------------- Add
 * @brief Create one new state with the given name and register it in the pool.
 *
 *        The caller supplies `callbackRegister` to bind whatever Lua bindings
 *        this state needs.  Registration is intentionally external so that
 *        different named groups can expose different APIs.
 *
 * @param stringName         Name assigned to the new state (non-unique).
 * @param callbackRegister   Called once after `open_libraries` to register
 *                           types and functions into the new state.
 * @return                   Reference to the newly created `state` record.
 */
LuaStatePool::state& LuaStatePool::Add( std::string_view stringName, std::function<void(sol::state&)> callbackRegister, enumLuaFeature eLuaFeature )
{
   std::unique_ptr<sol::state> psolstateLua = Create( callbackRegister, eLuaFeature ); // create the new sol::state first to avoid blocking the pool lock during registration

   std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };

   auto& pstate_ = m_vectorStates.emplace_back( std::make_unique<state>( next_id(), std::string{ stringName }, std::move( psolstateLua ), this ) );

   return *pstate_;
}

/**  -------------------------------------------------------------------------- Add
 * @brief Convenience overload: create `uCount` states that all share the
 *        same name and the same registration callback.
 *
 * @param stringName         Shared name for all created states.
 * @param uCount             Number of states to create.
 * @param callbackRegister   Registration callback applied to each new state.
 * @param eLuaFeature        Determines which standard Lua libraries to open.
 */
void LuaStatePool::Add( std::string_view stringName, size_t uCount, std::function<void(sol::state&)> callbackRegister, enumLuaFeature eLuaFeature )
{
   for( size_t u = 0; u < uCount; ++u )
   {
      Add( stringName, callbackRegister, eLuaFeature );
   }
}

/**  -------------------------------------------------------------------------- acquire
 * @brief Borrow an idle state whose name matches `stringName`.
 *
 *        Scans the pool for the first state with a matching name whose
 *        in-use flag is clear, sets the flag, and returns a `borrow` token.
 *        If all matching states are busy and `m_uHardConcurrencyLimit` has
 *        been reached the call **blocks** until one becomes available.
 *
 * @param stringName   Name of the desired state group.
 * @return             `borrow` RAII token wrapping the acquired state.
 */
LuaStatePool::borrow LuaStatePool::Acquire( std::string_view stringName )
{
   while( true )
   {
      {
         std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };

         for( auto& pstate_ : m_vectorStates )
         {
            if( pstate_->m_stringName != stringName ) { continue; }

            bool bExpectedIdle = false;
            if( pstate_->m_bInUse.compare_exchange_strong( bExpectedIdle, true, std::memory_order_acquire, std::memory_order_relaxed ) )
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

void LuaStatePool::Reset( uint64_t uId, const std::list<std::string_view>& listStringName, bool bCallGC )
{
   std::lock_guard<std::mutex> lockguardPool{ m_mutexPool };
   auto it = std::find_if( m_vectorStates.begin(), m_vectorStates.end(),
                           [uId]( const std::unique_ptr<state>& pstate_ ) { return pstate_->m_uId == uId; }
   );

   if( it == m_vectorStates.end() ) { return; }                        // id not found

   state& stateTarget = **it;
   auto* pstateLua = stateTarget.get_raw_luastate();                                               assert( pstateLua != nullptr && "Lua state is null" );
   for( const auto& stringName : listStringName )
   {
      (*pstateLua)[stringName] = nullptr;
   }

   if( bCallGC ) { pstateLua->collect_garbage(); }
   
}

/** -------------------------------------------------------------------------- Erase
 * @brief Erase the state with the given id from the pool.
 *
 * Waits until the target state is idle, then removes it from the pool.
 *
 * @param uId   Unique id of the state to erase (assigned at construction).
 * @return      `true` if a state was found and erased; `false` if no state
 *              with the given id exists in the pool.
 */
bool LuaStatePool::Erase( uint64_t uId )
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
         reset_state( *stateTarget.m_pstateLua );
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


/**  -------------------------------------------------------------------------- Create
 * @brief Create a new `sol::state` and register it with the given callback.
 *
 *        Opens standard libraries according to `eLuaFeature`, then calls
 *        `callbackRegister` to bind any additional types and functions.
 *
 * @param callbackRegister   Registration callback applied to the new state.
 * @param eLuaFeature        Determines which standard Lua libraries to open.
 * @return                   Unique pointer to the newly created `sol::state`.
 */
std::unique_ptr<sol::state> LuaStatePool::Create( const std::function<void(sol::state&)>& callbackRegister, enumLuaFeature eLuaFeature )
{
   auto pstateLua = std::make_unique<sol::state>();

   if( eLuaFeature == eLuaFeatureCore )
   {
      pstateLua->open_libraries( sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::io );
   }
   else if( eLuaFeature == eLuaFeatureAll )
   {
      pstateLua->open_libraries( sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::io, sol::lib::package, sol::lib::debug );
   }

   //psolstateLua->set_exception_handler( &OnError );

   if( callbackRegister ) callbackRegister( *pstateLua );                     // caller controls exactly what gets registered
   return pstateLua;
}

void LuaStatePool::reset_state( sol::state& stateLua )
{
   if( m_callbackReset )
   {
      m_callbackReset( stateLua );
      return;
   }

   // ## default: clear known per-request globals
   stateLua["app"]     = sol::lua_nil;
   stateLua["doc"]    = sol::lua_nil;
   stateLua["request"] = sol::lua_nil;

   stateLua.collect_garbage();                                                // reclaim memory held by previous script's temporaries
}

uint64_t LuaStatePool::next_id() { return m_uNextStateId.fetch_add( 1, std::memory_order_relaxed ); }


LUA_END
