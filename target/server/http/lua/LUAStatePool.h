/**
 * @FILE [tag: lua, pool, web_server] [summary: Pool of reusable sol::state instances for high-throughput web server lua execution]
 *
 * @brief Manages a pool of pre-initialised `sol::state` objects to avoid the
 *        overhead of creating and registering a new lua state per web request.
 *        Each state has a numeric id, a non-unique name used to select a
 *        specific flavour of state, and an in-use flag that is set atomically
 *        when a caller acquires the state.
 *
 * ## Usage
 *
 * Acquire a state by name via `acquire( stringName )`. The pool marks the
 * matching idle state as in-use and wraps it in an RAII `borrow` token.
 * The flag is cleared automatically when the `borrow` goes out of scope.
 *
~~~{.cpp}
auto borrow = lua::LuaStatePool::instance().acquire( "worker" );
borrow->script( "print('hello')" );
// in-use flag cleared here, state remains owned by the pool
~~~
 *
 * - `class LuaStatePool`  - Owns all `state` objects in a contiguous array
 * - `struct state`        - Wraps one `sol::state` with id, name, and in-use flag
 * - `struct borrow`       - RAII guard; clears in-use flag on destruction
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

extern "C" {
   #include "lua/lua.h"
   #include "lua/lualib.h"
   #include "lua/lauxlib.h"
}


#include "lua/sol.hpp"


#ifndef LUA_BEGIN
#  define LUA_BEGIN namespace LUA {
#  define LUA_END }
#endif

LUA_BEGIN

// ----------------------------------------------------------------------------
// --------------------------------------------------------------- LuaStatePool
// ----------------------------------------------------------------------------

/**
 * @CLASS [tag: lua, pool, concurrency] [summary: Thread-safe pool of pre-initialised sol::state objects]
 *
 * @brief The pool owns **all** `state` objects for their entire lifetime.
 *        Callers never take ownership; they receive a `borrow` token that
 *        holds a non-owning pointer and clears the in-use flag on destruction.
 *
 *        States are looked up by `stringName`; multiple states may share the
 *        same name to allow round-robin or first-available selection across
 *        states of the same "type".  Each state also carries a unique numeric
 *        id assigned at construction.
 *
 * **Thread safety**: All public methods are thread-safe.
 * **Blocking behaviour**: `acquire()` blocks when every matching state is
 *   in use and `m_uHardConcurrencyLimit` has been reached.  Set the limit
 *   to 0 to allow the pool to grow without bound.
 * **Deletion**: `erase()` waits until the target state is idle before
 *   removing it from the pool.
 */
class LuaStatePool
{
// ## types --------------------------------------------------------------------
public:
   enum enumLuaFeature
   {
      eLuaFeatureCore = 0,     ///< opens standard lua libraries (base, table, string)
      eLuaFeatureAll,          ///< opens all lua libraries
   };

   // --------------------------------------------------------------------------
   /**
    * @CLASS [tag: lua, pool] [summary: Owned state record — wraps one sol::state with identity and status]
    *
    * @brief Stored by value inside `m_vectorStates`.  Never moves after
    *        insertion (pool reserves capacity up front).  The `m_bInUse` flag
    *        is the only field written concurrently; everything else is
    *        immutable after construction.
    *
    *        Non-copyable, non-movable (the pool holds pointers into this object).
    */
   struct state
   {
   // ## construction ----------------------------------------------------------
      state() = delete;
      state( uint64_t uId, std::string stringName, std::unique_ptr<sol::state> pstateLua )
         : m_uId{ uId }, m_stringName{ std::move( stringName ) }, m_bInUse{ false }, m_pstateLua{ std::move( pstateLua ) } {}

      state( const state& ) = delete;
      state& operator=( const state& ) = delete;
      state( state&& ) = delete;
      state& operator=( state&& ) = delete;

   // ## operators -------------------------------------------------------------
      sol::state& operator*()  const { return *m_pstateLua; }
      sol::state* operator->() const { return m_pstateLua.get(); }

      void script( std::string_view stringScript ) { m_pstateLua->script( stringScript ); }
      void safe_script( std::string_view stringScript ) { m_pstateLua->safe_script( stringScript ); }

   // ## attributes ------------------------------------------------------------
      const uint64_t        m_uId;         ///< unique id assigned at construction, never changes
      const std::string     m_stringName;  ///< non-unique name used to match acquire requests
      std::atomic<bool>     m_bInUse;      ///< true while borrowed by a caller

   private:
      friend class LuaStatePool;
      std::unique_ptr<sol::state> m_pstateLua; ///< owning; only pool touches this directly
   };


   // --------------------------------------------------------------------------
   /**
    * @CLASS [tag: lua, RAII] [summary: Borrow token — clears in-use flag on destruction]
    *
    * @brief Non-owning handle to a `state` inside the pool.  Grants exclusive
    *        access to the underlying `sol::state` for the lifetime of the
    *        `borrow` object.  The pool retains ownership at all times.
    *
    *        Non-copyable, movable.
    */
   struct borrow
   {
   // ## construction ----------------------------------------------------------
      borrow() = delete;
      borrow( state& stateTarget ): m_pstateTarget{ &stateTarget }
      {}

      borrow( borrow&& o ) noexcept : m_pstateTarget{ o.m_pstateTarget } { o.m_pstateTarget = nullptr; }

      borrow& operator=( borrow&& o ) noexcept
      {
         if( this != &o )
         {
            release();
            m_pstateTarget   = o.m_pstateTarget;
            o.m_pstateTarget = nullptr;
         }
         return *this;
      }

      borrow( const borrow& ) = delete;
      borrow& operator=( const borrow& ) = delete;

      ~borrow() { release(); }

   // ## operators -------------------------------------------------------------
      sol::state& operator*()  const { return **m_pstateTarget; }
      sol::state* operator->() const { return &(**m_pstateTarget); }

   
   // ## interface -------------------------------------------------------------

      sol::state& get_luastate() { return **m_pstateTarget; }

      /**  -------------------------------------------------------------------------- id
       * @brief Id of the borrowed state.
       * @return Unique numeric id.
       */
      uint64_t id() const { return m_pstateTarget->m_uId; }

      /**  -------------------------------------------------------------------------- name
       * @brief Name of the borrowed state.
       * @return Name string (non-unique, identifies the state type/group).
       */
      std::string_view name() const { return m_pstateTarget->m_stringName; }

   // ## attributes ------------------------------------------------------------
   private:
      state* m_pstateTarget;  ///< non-owning; pool is the sole owner

   // ## helpers ---------------------------------------------------------------
   private:
      void release()
      {
         if( m_pstateTarget )
         {
            m_pstateTarget->m_bInUse.store( false, std::memory_order_release );
            m_pstateTarget = nullptr;
         }
      }
   };


// ## construction -------------------------------------------------------------
public:
   /// Construct an empty pool.  Use `emplace()` to add named states, or call
   explicit LuaStatePool( size_t uMaxPoolSize = 0, size_t uHardConcurrencyLimit = 0 );

   ~LuaStatePool() = default;

   LuaStatePool( const LuaStatePool& ) = delete;
   LuaStatePool& operator=( const LuaStatePool& ) = delete;
   LuaStatePool( LuaStatePool&& ) = delete;
   LuaStatePool& operator=( LuaStatePool&& ) = delete;


// ## public interface ---------------------------------------------------------
public:

   /// Add a new state to the pool with the given name and registration callback.
   state& Add( std::string_view stringName, std::function<void(sol::state&)> callbackRegister, enumLuaFeature eLuaFeature = eLuaFeatureCore );
   /// Convenience overload: add `uCount` states with the same name and registration callback.
   void Add( std::string_view stringName, size_t uCount, std::function<void(sol::state&)> callbackRegister, enumLuaFeature eLuaFeature = eLuaFeatureCore );

   /// Borrow an idle state whose name matches `stringName`.  Blocks if all matching states are busy.
   [[nodiscard]] borrow Acquire( std::string_view stringName );

   /// Erase the state with the given id from the pool.  Waits until the target state is idle before erasing.
   bool Erase( uint64_t uId );

   /**  -------------------------------------------------------------------------- size
    * @brief Total number of states currently owned by the pool (idle + in use).
    * @return Snapshot count (approximate under concurrent load).
    */
   size_t size() const;

   /**  -------------------------------------------------------------------------- empty
    * @brief True when the pool owns no states at all.
    * @return `true` if the pool is empty.
    */
   bool empty() const;

   /**  -------------------------------------------------------------------------- set_reset_callback
    * @brief Override the default per-request reset logic with a custom function.
    *
    *        The callback is invoked on a state's `sol::state` **before** its
    *        in-use flag is cleared.  If not set, the built-in `reset_state`
    *        implementation is used (clears known per-request globals and runs GC).
    *
    * @param callbackReset   Callable `void(sol::state&)`.
    */
   void set_reset_callback( std::function<void(sol::state&)> callbackReset );


// ## internal helpers ---------------------------------------------------------
private:

   /// Create a new `sol::state` and register it with the given callback.
   std::unique_ptr<sol::state> Create( const std::function<void(sol::state&)>& callbackRegister, enumLuaFeature eLuaFeature = eLuaFeatureCore );

   /**  -------------------------------------------------------------------------- reset_state
    * @brief Clear per-request globals so a recycled state is safe for the
    *        next request.  Runs the garbage collector after clearing.
    *
    * @param solstateLua   The underlying `sol::state` to reset in-place.
    */
   void reset_state( sol::state& solstateLua );

   /**  -------------------------------------------------------------------------- next_id
    * @brief Atomically generate the next unique state id.
    * @return Monotonically increasing id value.
    */
   uint64_t next_id();


// ## attributes ---------------------------------------------------------------
private:
   size_t                     m_uMaxPoolSize;            ///< upper bound on total owned states
   size_t                     m_uHardConcurrencyLimit;   ///< 0 = unlimited concurrent borrows

   std::vector<std::unique_ptr<state>> m_vectorStates;   ///< sole owner of all state objects; never reallocated after reserve

   mutable std::mutex         m_mutexPool;               ///< guards `m_vectorStates` for structural changes (emplace / erase)
   std::condition_variable    m_conditionStateAvailable; ///< signalled when a borrow is released

   std::function<void(sol::state&)> m_callbackReset;     ///< optional; invoked before in-use flag is cleared

   std::atomic<uint64_t>      m_uNextStateId{ 1 };       ///< monotonic id counter; 0 is reserved as "invalid"
};


LUA_END
