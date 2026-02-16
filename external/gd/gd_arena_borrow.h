#pragma once

#include "gd_compiler.h"
#if GD_COMPILER_HAS_CPP20_SUPPORT

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif


_GD_BEGIN


namespace arena {
namespace borrow {

/** ==========================================================================
 * @brief A simple arena allocator that may borrow its initial storage or allocate on heap
 * 
 * The arena manages a single memory block that can either be:
 * - Borrowed from external storage (std::array, std::vector, C-array, or raw buffer)
 * - Allocated on the heap when borrowed storage is exhausted
 * 
 * The arena tracks allocations using a simple bump allocator pattern. When the 
 * borrowed storage is full, it allocates a new block on the heap and continues 
 * allocating from there.
 * 
 * **Memory Ownership Tracking**:
 * The highest bit in `m_uCapacity` indicates borrowed storage (BORROW_BIT).
 * When set, the arena does not own the memory and will not deallocate it.
 * When allocating on heap, this bit is cleared and the arena takes ownership.
 * 
 * @par Example
 * @code{.cpp}
 * std::array<std::byte, 1024> buffer_;
 * gd::borrow::arena arenaLocal( buffer_ );
 * 
 * // ## Allocate from borrowed storage ......................................
 * void* pMemory1 = arenaLocal.allocate( 64 );
 * assert( arenaLocal.is_borrowed() == true ); // still using borrowed storage
 * 
 * // ## Force heap allocation when borrowed storage is full ..................
 * void* pMemory2 = arenaLocal.allocate( 2048 ); // exceeds borrowed capacity
 * assert( arenaLocal.is_borrowed() == false ); // now owner heap storage
 * @endcode
 * 
 * @par Usage with STL Allocators
 * @code{.cpp}
 * std::array<std::byte, 512> buffer_;
 * gd::borrow::arena arenaLocal( buffer_ );
 * 
 * // ## Use with custom allocator for std::string ............................
 * using string_type = std::basic_string<char, std::char_traits<char>, arena_allocator<char>>;
 * string_type stringLocal( arena_allocator<char>( &arenaLocal ) );
 * @endcode
 */
class arena
{
public:
   using size_type = std::size_t;                                             // type for size measurements
   using byte_type = std::byte;                                               // type for raw memory
   
   static constexpr size_type BORROW_BIT = size_type(1) << (sizeof(size_type) * 8 - 1); // bit flag indicating borrowed storage
   static constexpr size_type DEFAULT_HEAP_SIZE = 4096;                       // default heap allocation size in bytes

   /// ## Constructors
   
   arena() noexcept;
   arena(void* pBuffer, size_type uCapacity) noexcept;
   
   // ## Borrow from container with .data() and .size() - works with std::vector, std::array, etc.
   template<typename CONTAINER>
   requires requires(CONTAINER& c_) {
      { c_.data() } -> std::convertible_to<void*>;
      { c_.size() } -> std::convertible_to<std::size_t>; 
   }
   && ( !std::is_same_v<std::remove_cvref_t<CONTAINER>, arena> )
   explicit arena(CONTAINER& container_) noexcept;
   
   template<size_type uN>
   explicit arena(std::byte (&array_)[uN]) noexcept;                          // C-array specialization
   
   arena(const arena&) = delete;                                              // arenas should not be copied
   arena(arena&& o) noexcept;
   
   ~arena() noexcept;

   /// ## @API [tag: assign] [summary: Assignment operators] 

   arena& operator=(const arena&) = delete;                                   // arenas should not be copied
   arena& operator=(arena&& o) noexcept;

   /// ## @API [tag: allocate] [summary: Memory allocation]

   [[nodiscard]] void* allocate(size_type uBytes, size_type uAlignment = alignof(std::max_align_t));
   void deallocate(void* pMemory, size_type uBytes) noexcept;                 // no-op for simple arena
   void reset() noexcept;                                                     // reset allocation pointer to beginning

   /// ## @API [tag: capacity] [summary: Capacity queries]  

   [[nodiscard]] size_type capacity() const noexcept { return m_uCapacity & ~BORROW_BIT; }
   [[nodiscard]] size_type used() const noexcept { return m_uUsed; }
   [[nodiscard]] size_type available() const noexcept { return capacity() - m_uUsed; }
   [[nodiscard]] bool is_borrowed() const noexcept { return (m_uCapacity & BORROW_BIT) != 0; }
   [[nodiscard]] bool owner() const noexcept { return (m_uCapacity & BORROW_BIT) == 0 && m_pBuffer != nullptr; }

private:
   void grow(size_type uMinCapacity);
   void destroy() noexcept;
   
   [[nodiscard]] static size_type align_up(size_type uValue, size_type uAlignment) noexcept;

private:
   byte_type* m_pBuffer;    ///< pointer to current storage (borrowed or owned)
   size_type m_uUsed;       ///< current number of bytes used
   size_type m_uCapacity;   ///< capacity with high bit indicating borrowed storage
};

// ============================================================================
// ## Out-of-line constructor definitions
// ============================================================================

/** -------------------------------------------------------------------------- Default constructor
 * @brief Creates an empty arena with no storage
 */
inline arena::arena() noexcept 
   : m_pBuffer(nullptr), m_uUsed(0), m_uCapacity(0) 
{}

/** -------------------------------------------------------------------------- Borrow buffer constructor
 * @brief Creates an arena that borrows external storage
 * 
 * @param pBuffer Pointer to external storage buffer
 * @param uCapacity Number of bytes the external buffer can hold
 */
inline arena::arena(void* pBuffer, size_type uCapacity) noexcept 
   : m_pBuffer(static_cast<byte_type*>(pBuffer)), m_uUsed(0), m_uCapacity(uCapacity | BORROW_BIT) // note the BORROW_BIT is set in capacity to indicate borrowed storage
{                                                                                                  assert(pBuffer != nullptr || uCapacity == 0); // verify buffer is valid or capacity is zero
}

/** -------------------------------------------------------------------------- Borrow from C-array constructor
 * @brief Creates an arena that borrows storage from a C-array
 * 
 * @tparam uN Size of the array in bytes
 * @param array_ Reference to byte array to borrow storage from
 */
template<std::size_t uN>
inline arena::arena(std::byte (&array_)[uN]) noexcept 
   : m_pBuffer(array_), m_uUsed(0), m_uCapacity(uN | BORROW_BIT)               // note the BORROW_BIT is set in capacity to indicate borrowed storage
{}

/** -------------------------------------------------------------------------- Borrow from container constructor
 * @brief Creates an arena that borrows storage from a container with .data() and .size()
 * 
 * Works with std::vector<std::byte>, std::array<std::byte, N>, and other containers 
 * providing .data() and .size() methods.
 * 
 * @tparam CONTAINER Container type with .data() and .size() methods
 * @param container_ Reference to container to borrow storage from
 */
template<typename CONTAINER>
requires requires(CONTAINER& c_) {
   { c_.data() } -> std::convertible_to<void*>;                               // verify container has .data() returning pointer
   { c_.size() } -> std::convertible_to<std::size_t>;                         // verify container has .size() returning size type
}
&& ( !std::is_same_v<std::remove_cvref_t<CONTAINER>, arena> )
inline arena::arena(CONTAINER& container_) noexcept 
   : m_pBuffer(static_cast<byte_type*>(container_.data())), 
     m_uUsed(0), 
     m_uCapacity(container_.size() | BORROW_BIT)                              // note the BORROW_BIT is set in capacity to indicate borrowed storage
{
}

/** -------------------------------------------------------------------------- Move constructor
 * @brief Moves another arena's contents
 * 
 * @param o Arena to move from
 */
inline arena::arena(arena&& o) noexcept 
   : m_pBuffer(o.m_pBuffer), m_uUsed(o.m_uUsed), m_uCapacity(o.m_uCapacity) 
{
   o.m_pBuffer = nullptr;
   o.m_uUsed = 0;
   o.m_uCapacity = 0;
}

/** -------------------------------------------------------------------------- Destructor
 * @brief Destroys arena and deallocates owned storage
 */
inline arena::~arena() noexcept 
{
   destroy();
}

// ============================================================================
// ## Assignment operators
// ============================================================================

/** -------------------------------------------------------------------------- Move assignment
 * @brief Moves another arena's contents
 * 
 * @param o Arena to move from
 * @return Reference to this arena
 */
inline arena& arena::operator=(arena&& o) noexcept 
{
   if( this != &o )
   {
      destroy();
      m_pBuffer = o.m_pBuffer;   m_uUsed = o.m_uUsed;   m_uCapacity = o.m_uCapacity; // take ownership of storage
      o.m_pBuffer = nullptr;     o.m_uUsed = 0;         o.m_uCapacity = 0;    // clear source arena
   }
   return *this;
}

// ============================================================================
// ## Allocation methods
// ============================================================================

/** -------------------------------------------------------------------------- allocate
 * @brief Allocate memory from arena
 * 
 * Allocates `uBytes` bytes with specified alignment. If borrowed storage is 
 * exhausted, allocates new heap storage and transitions to owned mode.
 * 
 * @param uBytes Number of bytes to allocate
 * @param uAlignment Alignment requirement (default: alignof(std::max_align_t))
 * @return Pointer to allocated memory
 */
inline void* arena::allocate(size_type uBytes, size_type uAlignment) 
{
   // ## Align current position ..............................................
   size_type uAlignedUsed = align_up(m_uUsed, uAlignment);
   size_type uNewUsed = uAlignedUsed + uBytes;
   
   // ## Check if allocation fits in current buffer ..........................
   if( uNewUsed > capacity() )
   {
      grow(uNewUsed);
      uAlignedUsed = align_up(m_uUsed, uAlignment);                           // recalculate after grow
      uNewUsed = uAlignedUsed + uBytes;
   }
   
   void* pResult = m_pBuffer + uAlignedUsed;
   m_uUsed = uNewUsed;
   return pResult;
}

/** -------------------------------------------------------------------------- deallocate
 * @brief Deallocate memory (no-op for simple bump allocator)
 * 
 * Simple arenas do not support individual deallocation. Use `reset()` to 
 * reclaim all memory at once.
 * 
 * @param pMemory Pointer to memory (ignored)
 * @param uBytes Number of bytes (ignored)
 */
inline void arena::deallocate(void* pMemory, size_type uBytes) noexcept 
{
   // ## No-op for simple bump allocator .....................................
   // Individual deallocations are not supported, use reset() instead
}

/** -------------------------------------------------------------------------- reset
 * @brief Reset arena to initial state
 * 
 * Resets the allocation pointer to the beginning of the buffer, effectively 
 * reclaiming all allocated memory. Does not deallocate owned storage.
 */
inline void arena::reset() noexcept 
{
   m_uUsed = 0;
}

// ============================================================================
// ## Private helper methods
// ============================================================================

/** -------------------------------------------------------------------------- grow
 * @brief Allocate new heap storage and transfer ownership
 * 
 * This method allocates new heap storage, copies existing data to it,
 * and takes ownership. If storage was borrowed, the borrow bit is cleared.
 * 
 * @param uMinCapacity Minimum capacity required in bytes
 */
inline void arena::grow(size_type uMinCapacity) 
{
   size_type uOldCapacity = capacity();
   
   // ## Calculate new capacity with growth factor ...........................
   size_type uNewCapacity = std::max({
      uOldCapacity * 2,                                                       // standard growth factor of 2
      uMinCapacity,                                                           // minimum requested capacity
      DEFAULT_HEAP_SIZE                                                       // minimum heap allocation size
   });
   
   // ## Allocate new buffer ..................................................
   byte_type* pNewBuffer = static_cast<byte_type*>(::operator new(uNewCapacity, std::align_val_t(alignof(std::max_align_t))));
   
   // ## Copy existing data to new buffer .....................................
   if( m_pBuffer != nullptr && m_uUsed > 0 )
   {
      std::memcpy(pNewBuffer, m_pBuffer, m_uUsed);
   }
   
   // ## Deallocate old buffer only if we owned it ............................
   if( owner() == true )
   {
      ::operator delete(m_pBuffer, uOldCapacity, std::align_val_t(alignof(std::max_align_t)));
   }
   
   m_pBuffer = pNewBuffer;
   m_uCapacity = uNewCapacity;                                                // clear borrow bit by setting without it
}

/** -------------------------------------------------------------------------- destroy
 * @brief Deallocate owned storage
 */
inline void arena::destroy() noexcept 
{
   if( owner() == true )
   {
      ::operator delete(m_pBuffer, capacity(), std::align_val_t(alignof(std::max_align_t)));
   }
   m_pBuffer = nullptr;
   m_uUsed = 0;
   m_uCapacity = 0;
}

/** -------------------------------------------------------------------------- align_up
 * @brief Align value up to specified alignment
 * 
 * @param uValue Value to align
 * @param uAlignment Alignment requirement (must be power of 2)
 * @return Aligned value
 */
inline arena::size_type arena::align_up(size_type uValue, size_type uAlignment) noexcept 
{                                                                                                  assert((uAlignment & (uAlignment - 1)) == 0); // verify alignment is power of 2
   return (uValue + uAlignment - 1) & ~(uAlignment - 1);
}

// ============================================================================
// ## STL-compatible allocator adapter
// ============================================================================

/** ==========================================================================
 * @brief STL-compatible allocator that uses an arena for memory allocation
 * 
 * This allocator allows STL containers to use arena memory. It wraps a pointer 
 * to an arena and forwards allocation requests to it.
 * 
 * @par Example
 * @code{.cpp}
 * std::array<std::byte, 1024> buffer_;
 * gd::borrow::arena arenaLocal( buffer_ );
 * 
 * // ## Use with std::vector .................................................
 * std::vector<int, arena_allocator<int>> vectorLocal( arena_allocator<int>(&arenaLocal) );
 * vectorLocal.push_back(42);
 * 
 * // ## Use with std::string .................................................
 * using string_type = std::basic_string<char, std::char_traits<char>, arena_allocator<char>>;
 * string_type stringLocal( arena_allocator<char>(&arenaLocal) );
 * stringLocal = "Hello, arena!";
 * @endcode
 * 
 * @tparam T Type of objects to allocate
 */
template<typename T>
class arena_allocator
{
public:
   using value_type = T;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   
   /// ## Constructors
   
   arena_allocator() noexcept : m_parena(nullptr) {}
   explicit arena_allocator(arena* parena_) noexcept : m_parena(parena_) {}
   
   template<typename U>
   arena_allocator(const arena_allocator<U>& o) noexcept : m_parena(o.m_parena) {}

   /// ## @API [tag: allocate] [summary: Allocation interface]

   [[nodiscard]] T* allocate(size_type uCount) 
   {                                                                                                  assert(m_parena != nullptr); // verify arena is valid
      return static_cast<T*>(m_parena->allocate(uCount * sizeof(T), alignof(T)));
   }
   
   void deallocate(T* pMemory, size_type uCount) noexcept 
   {
      if( m_parena != nullptr )
      {
         m_parena->deallocate(pMemory, uCount * sizeof(T));
      }
   }

   /// ## @API [tag: operator] [summary: Comparison operators]

   template<typename U>
   [[nodiscard]] bool operator==(const arena_allocator<U>& o) const noexcept 
   {
      return m_parena == o.m_parena;
   }
   
   template<typename U>
   [[nodiscard]] bool operator!=(const arena_allocator<U>& o) const noexcept 
   {
      return m_parena != o.m_parena;
   }

   template<typename U>
   friend class arena_allocator;

private:
   arena* m_parena;  ///< pointer to arena used for allocations
};

} // namespace borrow
} // namespace arena

_GD_END
