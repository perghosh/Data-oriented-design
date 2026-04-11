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
      state( uint64_t uId, std::string stringName, std::unique_ptr<sol::state> psolstateLua )
         : m_uId{ uId }, m_stringName{ std::move( stringName ) }, m_psolstateLua{ std::move( psolstateLua ) }, m_bInUse{ false } {}

      state( const state& ) = delete;
      state& operator=( const state& ) = delete;
      state( state&& ) = delete;
      state& operator=( state&& ) = delete;

   // ## operators -------------------------------------------------------------
      sol::state& operator*()  const { return *m_psolstateLua; }
      sol::state* operator->() const { return m_psolstateLua.get(); }

   // ## attributes ------------------------------------------------------------
      const uint64_t        m_uId;         ///< unique id assigned at construction, never changes
      const std::string     m_stringName;  ///< non-unique name used to match acquire requests
      std::atomic<bool>     m_bInUse;      ///< true while borrowed by a caller

   private:
      friend class LuaStatePool;
      std::unique_ptr<sol::state> m_psolstateLua; ///< owning; only pool touches this directly
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
   explicit LuaStatePool(
      size_t uMaxPoolSize          = 0,
      size_t uHardConcurrencyLimit = 0
   );

   ~LuaStatePool() = default;

   LuaStatePool( const LuaStatePool& ) = delete;
   LuaStatePool& operator=( const LuaStatePool& ) = delete;
   LuaStatePool( LuaStatePool&& ) = delete;
   LuaStatePool& operator=( LuaStatePool&& ) = delete;


// ## public interface ---------------------------------------------------------
public:

   /**  -------------------------------------------------------------------------- emplace
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
   state& emplace( std::string_view stringName, std::function<void(sol::state&)> callbackRegister );

   /**  -------------------------------------------------------------------------- emplace
    * @brief Convenience overload: create `uCount` states that all share the
    *        same name and the same registration callback.
    *
    * @param stringName         Shared name for all created states.
    * @param uCount             Number of states to create.
    * @param callbackRegister   Registration callback applied to each new state.
    */
   void emplace( std::string_view stringName, size_t uCount, std::function<void(sol::state&)> callbackRegister );

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
   [[nodiscard]] borrow acquire( std::string_view stringName );

   /**  -------------------------------------------------------------------------- erase
    * @brief Remove the state with the given `uId` from the pool.
    *
    *        If the state is currently in use the call **blocks** until it is
    *        released.  After this call the associated `state` object is
    *        destroyed and the id is never reused.
    *
    * @param uId   Unique id of the state to remove.
    * @return      `true` if a state with that id was found and erased,
    *              `false` if no such id exists.
    */
   bool erase( uint64_t uId );

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

   /**  -------------------------------------------------------------------------- create_state
    * @brief Allocate a new `sol::state`, open libraries, install the exception
    *        handler, and invoke `callbackRegister`.
    *
    * @param callbackRegister   Registration callback for this specific state.
    * @return                   Fully initialised, ready-to-use lua state.
    */
   std::unique_ptr<sol::state> create_state( const std::function<void(sol::state&)>& callbackRegister );

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
