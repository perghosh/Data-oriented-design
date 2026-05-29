// @FILE [tag: arguments_index] [description: index class that stores buffer offset positions into an arguments object to enable O(1) value access without name scanning] [type: header]
 

/**
 * @file gd_arguments_index.h
 * 
 * | Area                | Methods (Examples)                                                              | Description                                                                                    |
 * |---------------------|---------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------|
 * | Construction        | arguments_index(...), operator=(...)                                            | Constructors, copy/move, and assignment for creating index instances.                          |
 * | Building            | build(...), add(...), build_s(...)                                              | Populating the index from an arguments buffer or incrementally adding slots.                   |
 * | Retrieval           | get_argument(...), get_slot(...), get_position(...), operator[], find_s(...)    | Looking up values, slots, and buffer positions by name or index.                               |
 * | Utility/Meta        | clear(), empty(), size(), contains(...), reserve(...), slots()                  | Querying index state, checking membership, and managing slot capacity.                        |
 *
 * @brief Store buffer offset positions into an `arguments` object to enable fast O(1) value
 *        access by positional index or name, without scanning the byte buffer on every call.
 *
 * `arguments_index<ARGUMENTS>` is a thin companion to any arguments class that exposes the
 * common buffer interface (`next`, `is_name_s`, `get_name_s`, `get_argument_s`,
 * `buffer_offset`, `buffer_size`).  It works with both `gd::argument::arguments` and
 * `gd::argument::shared::arguments` without any code duplication.
 *
 * Build the index once after the arguments object is fully populated (or after the last
 * append), then use `get_argument` or `operator[]` to retrieve values directly.
 * The index is invalidated if the arguments buffer is reallocated or if any element
 * before an indexed position is removed or resized; call `build()` again after any such
 * structural change.
 *
 * ### Key Features
 * - Works with any arguments type that satisfies the buffer interface (template parameter).
 * - Each `slot` stores a `size_t` byte offset and a `string_view` into the buffer (zero-copy).
 * - `build()` iterates the buffer once using `next()`; subsequent access is O(1) by position
 *   or O(n-slots) by name, where each slot comparison is a cheap string_view equality check.
 * - `add()` keeps the index live as values are appended one at a time.
 *
 * ### Example Usage
 * \code
 * gd::argument::arguments args;
 * args.append("width",  1920);
 * args.append("height", 1080);
 * args.append("depth",  32);
 *
 * gd::argument::arguments_index<gd::argument::arguments> idx(args);
 *
 * int width  = idx.get_argument(args, "width").as_int();   // O(1) buffer access
 * int height = idx.get_argument(args, 1u).as_int();         // positional slot
 * auto depth = idx[2u].get(args).as_int();                  // slot operator[]
 * \endcode
 *
 * Works identically with `gd::argument::shared::arguments`:
 * \code
 * gd::argument::shared::arguments sharedArgs;
 * sharedArgs.append("x", 10);
 * sharedArgs.append("y", 20);
 *
 * gd::argument::arguments_index<gd::argument::shared::arguments> idx(sharedArgs);
 * int x = idx.get_argument(sharedArgs, "x").as_int();
 * \endcode
 */

/**
 * ### 0TAG0 File navigation, mark and jump to common parts
 * - `0TAG0slot.arguments_index`       - The `slot` struct: one offset entry in the index.
 * - `0TAG0construct.arguments_index`  - Constructors and destructor.
 * - `0TAG0build.arguments_index`      - Methods to build or incrementally populate the index.
 * - `0TAG0get.arguments_index`        - Methods to retrieve values through the index.
 * - `0TAG0operator.arguments_index`   - Overloaded operators.
 * - `0TAG0utility.arguments_index`    - Utility / meta methods (size, empty, contains, ...).
 */


#pragma once
#include <cassert>
#include <string_view>
#include <vector>

#include "gd_arguments.h"
#include "gd_arguments_shared.h"


#if defined( __clang__ )
   #pragma clang diagnostic push
   #pragma clang diagnostic ignored "-Wunused-variable"
#elif defined( __GNUC__ )
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wunused-variable"
#elif defined( _MSC_VER )
   #pragma warning(push)
   #pragma warning( disable : 4100 4189 )
#endif

// @AI [tag: gd, arguments_index] [llm: core]

_GD_ARGUMENT_BEGIN


// ================================================================================================
// ============================================================================= arguments_index
// ================================================================================================

/**
 * \brief Index of buffer-offset positions into an `ARGUMENTS` object for fast value access.
 *
 * `ARGUMENTS` must provide the common buffer interface shared by `gd::argument::arguments`
 * and `gd::argument::shared::arguments`:
 *  - `next() -> const_pointer`
 *  - `next(const_pointer) -> const_pointer`
 *  - `static is_name_s(const_pointer) -> bool`
 *  - `static get_name_s(const_pointer) -> string_view`
 *  - `static get_argument_s(const_pointer) -> argument`
 *  - `buffer_offset(const_pointer) -> size_t`
 *  - `buffer_offset(size_t) -> const_pointer`
 *  - `buffer_size() -> (integral)`
 *
 \code
 \endcode

*Build index and access values with O(1) buffer lookup*
 \code
gd::argument::arguments args;
args.append("x", 10);
args.append("y", 20);
args.append("label", "hello");

gd::argument::arguments_index<gd::argument::arguments> idx;
idx.build(args);

int x     = idx.get_argument(args, "x").as_int();      // name lookup via slot scan
int y     = idx.get_argument(args, 1u).as_int();        // direct positional slot
auto lbl  = idx[2u].get(args).as_string();              // slot then direct buffer read
 \endcode
 */
template<typename ARGUMENTS>
class arguments_index
{
// ## typedefs -----------------------------------------------------------------
public:
   using argument_type = typename ARGUMENTS::argument;
   using const_pointer = typename ARGUMENTS::const_pointer;
   using offset_type   = size_t;                    ///< byte offset into the arguments buffer
   using size_type     = size_t;
   using self          = arguments_index<ARGUMENTS>;

   struct tag_is_arguments_index {};                ///< tag dispatcher to detect arguments_index in templates

   /** ======================================================================= slot
    * @brief One entry in the index: the byte offset of an argument plus a view of its name.
    *
    * `m_stringName` is a zero-copy `string_view` into the `ARGUMENTS` buffer.
    * `m_uOffset` is the byte distance from `buffer_data()` to the start of the entry,
    * matching the value returned by `ARGUMENTS::buffer_offset(const_pointer)`.
    *
    * 0TAG0slot.arguments_index
    */
   struct slot
   {
      using self = slot;

      slot() : m_uOffset(0) {}
      slot(std::string_view stringName, offset_type uOffset) : m_stringName(stringName), m_uOffset(uOffset) {}
      slot(const slot&) = default;
      slot& operator=(const slot&) = default;

      /// Retrieve the argument value this slot points to
      [[nodiscard]] argument_type get(const ARGUMENTS& argumentsSource) const;

      /// Return the raw buffer pointer for this slot
      [[nodiscard]] const_pointer position(const ARGUMENTS& argumentsSource) const;

      /// Name for the argument at this slot (empty for unnamed arguments)
      [[nodiscard]] std::string_view name() const { return m_stringName; }

      /// Byte offset stored in this slot
      [[nodiscard]] offset_type offset() const { return m_uOffset; }

      /// True when the slot has not been populated
      [[nodiscard]] bool empty() const { return m_uOffset == 0 && m_stringName.empty(); }

      bool operator==(const self& o) const { return m_uOffset == o.m_uOffset; }
      bool operator!=(const self& o) const { return !(*this == o); }

   // ## attributes
   public:
      std::string_view m_stringName;  ///< view into the arguments buffer for the argument name
      offset_type      m_uOffset;     ///< byte offset from buffer start to this argument entry
   };


// ## @API [tag: construct] [description: construction methods for arguments_index instances]
public: //0TAG0construct.arguments_index

   arguments_index() {}

   /// Construct and immediately build the index from `argumentsSource`
   explicit arguments_index(const ARGUMENTS& argumentsSource);

   /// Construct with pre-reserved slot capacity to avoid early reallocations
   explicit arguments_index(size_type uReserve) { m_vectorSlot.reserve(uReserve); }

   arguments_index(const self&) = default;
   arguments_index(self&&) noexcept = default;

   self& operator=(const self&) = default;
   self& operator=(self&&) noexcept = default;

   ~arguments_index() = default;

   // ## @API [tag: operator] [description: overloaded operators for arguments_index]
public: //0TAG0operator.arguments_index

   /// Return the slot at `uIndex`; the slot can be passed to `slot::get(args)` for the value
   [[nodiscard]] const slot& operator[](size_type uIndex) const;

   /// Return the first slot matching `stringName`; returns an empty sentinel slot if not found
   [[nodiscard]] const slot& operator[](std::string_view stringName) const;


// ## @API [tag: build] [description: methods to build or populate the index from an arguments object]
public: //0TAG0build.arguments_index

   /// Scan `argumentsSource` and record a slot for every argument (replaces existing slots)
   void build(const ARGUMENTS& argumentsSource);

   /// Append one slot for the argument at `pPosition` (use after a single append to keep the index live)
   void add(const ARGUMENTS& argumentsSource, const_pointer pPosition);

   /// Append one slot for the argument at byte offset `uOffset`
   void add(const ARGUMENTS& argumentsSource, offset_type uOffset);

   /// Remove all stored slots without releasing vector capacity
   void clear() noexcept { m_vectorSlot.clear(); }

   /// Build and return an index for `argumentsSource` (static factory)
   [[nodiscard]] static self build_s(const ARGUMENTS& argumentsSource);


// ## @API [tag: get] [description: retrieve values through the index, O(1) buffer access after slot lookup]
public: //0TAG0get.arguments_index

   /// Return the argument value for the slot at positional index `uIndex`
   [[nodiscard]] argument_type get_argument(const ARGUMENTS& argumentsSource, size_type uIndex) const;

   /// Return the argument value for the first slot whose name matches `stringName`
   [[nodiscard]] argument_type get_argument(const ARGUMENTS& argumentsSource, std::string_view stringName) const;

   /// Return the slot at positional index `uIndex`
   [[nodiscard]] const slot& get_slot(size_type uIndex) const;

   /// Return a pointer to the first slot matching `stringName`, or `nullptr` if not found
   [[nodiscard]] const slot* get_slot(std::string_view stringName) const;

   /// Return the raw buffer pointer for the slot at `uIndex`
   [[nodiscard]] const_pointer get_position(const ARGUMENTS& argumentsSource, size_type uIndex) const;

   /// Return the raw buffer pointer for the first slot matching `stringName`, or `nullptr`
   [[nodiscard]] const_pointer get_position(const ARGUMENTS& argumentsSource, std::string_view stringName) const;

   /// Return the index of the first slot matching `stringName`, or -1 if not found
   [[nodiscard]] int64_t get_index(std::string_view stringName) const;

// ## @API [tag: utility] [description: utility and meta methods for arguments_index]
public: //0TAG0utility.arguments_index

   /// Number of slots stored in the index
   [[nodiscard]] size_type size() const noexcept { return m_vectorSlot.size(); }

   /// True when the index has no slots
   [[nodiscard]] bool empty() const noexcept { return m_vectorSlot.empty(); }

   /// True when a slot with the given name exists in the index
   [[nodiscard]] bool exists(std::string_view stringName) const { return find_s(m_vectorSlot, stringName) != nullptr; }

   /// True when positional index `uIndex` is within range
   [[nodiscard]] bool exists(size_type uIndex) const { return uIndex < m_vectorSlot.size(); }

   /// Reserve capacity for `uCount` slots
   void reserve(size_type uCount) { m_vectorSlot.reserve(uCount); }

   /// Read-only access to the underlying slot vector
   [[nodiscard]] const std::vector<slot>& slots() const { return m_vectorSlot; }


// ## @API [tag: internal] [description: internal static helpers]
public:

   /// Find the first slot in `vectorSlot` matching `stringName`; returns `nullptr` if not found
   [[nodiscard]] static const slot* find_s(const std::vector<slot>& vectorSlot, std::string_view stringName);


// ## attributes ---------------------------------------------------------------
public:
   std::vector<slot> m_vectorSlot;  ///< ordered list of offset slots, one per argument
};


// ============================================================================
// ============================================= arguments_index implementation
// ============================================================================

// ## slot methods

template<typename ARGUMENTS>
typename arguments_index<ARGUMENTS>::argument_type
arguments_index<ARGUMENTS>::slot::get(const ARGUMENTS& argumentsSource) const
{                                                                                                   assert( m_uOffset < (offset_type)argumentsSource.buffer_size() );
   return ARGUMENTS::get_argument_s( argumentsSource.buffer_offset(m_uOffset) );
}

template<typename ARGUMENTS>
typename arguments_index<ARGUMENTS>::const_pointer
arguments_index<ARGUMENTS>::slot::position(const ARGUMENTS& argumentsSource) const
{                                                                                                   assert( m_uOffset < (offset_type)argumentsSource.buffer_size() );
   return argumentsSource.buffer_offset(m_uOffset);
}

// ## construct

template<typename ARGUMENTS>
arguments_index<ARGUMENTS>::arguments_index(const ARGUMENTS& argumentsSource)
{
   build(argumentsSource);
}

// ## operator

template<typename ARGUMENTS>
const typename arguments_index<ARGUMENTS>::slot&
arguments_index<ARGUMENTS>::operator[](size_type uIndex) const
{
   assert(uIndex < m_vectorSlot.size());
   return m_vectorSlot[uIndex];
}

template<typename ARGUMENTS>
const typename arguments_index<ARGUMENTS>::slot&
arguments_index<ARGUMENTS>::operator[](std::string_view stringName) const
{
   const slot* pSlot = find_s(m_vectorSlot, stringName);
   if(pSlot != nullptr) { return *pSlot; }
   static const slot slotEmpty_;
   return slotEmpty_;
}

// ## build

template<typename ARGUMENTS>
void arguments_index<ARGUMENTS>::build(const ARGUMENTS& argumentsSource)
{
   m_vectorSlot.clear();
   m_vectorSlot.reserve( static_cast<size_type>(argumentsSource.buffer_size()) / 8u + 4u );
   for( auto pPosition = argumentsSource.next(); pPosition != nullptr; pPosition = argumentsSource.next(pPosition) )
   {
      offset_type uOffset = argumentsSource.buffer_offset(pPosition);
      std::string_view stringName;
      if( ARGUMENTS::is_name_s(pPosition) ) { stringName = ARGUMENTS::get_name_s(pPosition); }
      m_vectorSlot.emplace_back(stringName, uOffset);
   }
}

template<typename ARGUMENTS>
void arguments_index<ARGUMENTS>::add(const ARGUMENTS& argumentsSource, const_pointer pPosition)
{                                                                                                   assert( argumentsSource.verify_d(pPosition) );
   offset_type uOffset = argumentsSource.buffer_offset(pPosition);
   std::string_view stringName;
   if( ARGUMENTS::is_name_s(pPosition) ) { stringName = ARGUMENTS::get_name_s(pPosition); }
   m_vectorSlot.emplace_back(stringName, uOffset);
}

template<typename ARGUMENTS>
void arguments_index<ARGUMENTS>::add(const ARGUMENTS& argumentsSource, offset_type uOffset)
{
   add(argumentsSource, argumentsSource.buffer_offset(uOffset));
}

template<typename ARGUMENTS>
arguments_index<ARGUMENTS>
arguments_index<ARGUMENTS>::build_s(const ARGUMENTS& argumentsSource)
{
   self index_;
   index_.build(argumentsSource);
   return index_;
}

// ## get

template<typename ARGUMENTS>
typename arguments_index<ARGUMENTS>::argument_type
arguments_index<ARGUMENTS>::get_argument(const ARGUMENTS& argumentsSource, size_type uIndex) const
{                                                                                                   assert( uIndex < m_vectorSlot.size() );
   return m_vectorSlot[uIndex].get(argumentsSource);
}

template<typename ARGUMENTS>
typename arguments_index<ARGUMENTS>::argument_type
arguments_index<ARGUMENTS>::get_argument(const ARGUMENTS& argumentsSource, std::string_view stringName) const
{
   const slot* pSlot = find_s(m_vectorSlot, stringName);
   if( pSlot != nullptr ) { return pSlot->get(argumentsSource); }
   return argument_type();
}

template<typename ARGUMENTS>
const typename arguments_index<ARGUMENTS>::slot&
arguments_index<ARGUMENTS>::get_slot(size_type uIndex) const
{                                                                                                   assert( uIndex < m_vectorSlot.size() );
   return m_vectorSlot[uIndex];
}

template<typename ARGUMENTS>
const typename arguments_index<ARGUMENTS>::slot*
arguments_index<ARGUMENTS>::get_slot(std::string_view stringName) const
{
   return find_s(m_vectorSlot, stringName);
}

template<typename ARGUMENTS>
typename arguments_index<ARGUMENTS>::const_pointer
arguments_index<ARGUMENTS>::get_position(const ARGUMENTS& argumentsSource, size_type uIndex) const
{                                                                                                   assert( uIndex < m_vectorSlot.size() );
   return m_vectorSlot[uIndex].position(argumentsSource);
}

template<typename ARGUMENTS>
typename arguments_index<ARGUMENTS>::const_pointer
arguments_index<ARGUMENTS>::get_position(const ARGUMENTS& argumentsSource, std::string_view stringName) const
{
   const slot* pSlot = find_s(m_vectorSlot, stringName);
   if( pSlot != nullptr ) { return pSlot->position(argumentsSource); }
   return nullptr;
}

/// Return the index of the first slot matching `stringName`, or -1 if not found
template<typename ARGUMENTS>
int64_t arguments_index<ARGUMENTS>::get_index(std::string_view stringName) const
{
   for(size_type u = 0; u < m_vectorSlot.size(); u++)
   {
      if(m_vectorSlot[u].name() == stringName) { return static_cast<int64_t>(u); }
   }
   return -1;
}

// ## internal

template<typename ARGUMENTS>
const typename arguments_index<ARGUMENTS>::slot*
arguments_index<ARGUMENTS>::find_s(const std::vector<slot>& vectorSlot, std::string_view stringName)
{
   for( const auto& slot_ : vectorSlot )
   {
      if( slot_.m_stringName == stringName ) { return &slot_; }
   }
   return nullptr;
}


// ================================================================================================
// ================================================================================ type aliases
// ================================================================================================

/// Convenience alias for `gd::argument::arguments`
using arguments_index_t        = arguments_index<arguments>;

/// Convenience alias for `gd::argument::shared::arguments`
using arguments_index_shared_t = arguments_index<gd::argument::shared::arguments>;


_GD_ARGUMENT_END


#if defined(__clang__)
   #pragma clang diagnostic pop
#elif defined(__GNUC__)
   #pragma GCC diagnostic pop
#elif defined(_MSC_VER)
   #pragma warning(pop)
#endif
