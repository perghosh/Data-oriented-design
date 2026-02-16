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
 * @brief A simple arena allocator with fixed capacity
 * 
 * The arena manages a single memory block with a fixed capacity that can be:
 * - Borrowed from external storage (std::array, std::vector, C-array)
 * - Allocated on heap at construction time
 * 
 * The arena uses a simple bump allocator pattern and NEVER grows beyond its
 * initial capacity. When storage is exhausted, allocate() returns nullptr.
 * 
 * **Memory Ownership Tracking**:
 * The highest bit in `m_uCapacity` indicates borrowed storage (BORROW_BIT).
 * - BORROW_BIT set: Arena borrows memory, will NOT deallocate on destruction
 * - BORROW_BIT clear: Arena owns memory, WILL deallocate on destruction
 * 
 * @par Example - Borrowed Storage
 * @code{.cpp}
 * std::array<std::byte, 1024> buffer_;
 * gd::arena::borrow::arena arenaLocal( buffer_ );
 * 
 * void* pMemory = arenaLocal.allocate( 64 );
 * assert( arenaLocal.is_borrowed() == true );  // using borrowed storage
 * assert( arenaLocal.owner() == false );       // does not own the storage
 * @endcode
 * 
 * @par Example - Owned Storage
 * @code{.cpp}
 * gd::arena::borrow::arena arenaLocal( nullptr, 1024 );  // allocates 1024 bytes on heap
 * 
 * void* pMemory = arenaLocal.allocate( 64 );
 * assert( arenaLocal.is_borrowed() == false ); // using owned storage
 * assert( arenaLocal.owner() == true );        // owns the storage
 * // Storage is deallocated when arenaLocal is destroyed
 * @endcode
 */
class arena
{
public:
   using size_type = std::size_t;                                             // type for size measurements
   using byte_type = std::byte;                                               // type for raw memory
   
   static constexpr size_type BORROW_BIT = size_type(1) << (sizeof(size_type) * 8 - 1); // bit flag indicating borrowed storage

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

   [[nodiscard]] void* allocate(size_type uBytes, size_type uAlignment = alignof(std::max_align_t)) noexcept;
   template<typename OBJECT>
   [[nodiscard]] OBJECT* allocate_objects(size_type uCount);
   template<typename OBJECT>
   [[nodiscard]] std::span<OBJECT> allocate_span(size_type uCount);
   void deallocate(void* pMemory, size_type uBytes) noexcept;                 // no-op for simple arena
   void reset() noexcept;                                                     // reset allocation pointer to beginning

   /// ## @API [tag: capacity] [summary: Capacity queries]  

   [[nodiscard]] size_type capacity() const noexcept { return m_uCapacity & ~BORROW_BIT; }
   [[nodiscard]] size_type used() const noexcept { return m_uUsed; }
   [[nodiscard]] size_type available() const noexcept { return capacity() - m_uUsed; }
   [[nodiscard]] bool is_borrowed() const noexcept { return (m_uCapacity & BORROW_BIT) != 0; }
   [[nodiscard]] bool owner() const noexcept { return (m_uCapacity & BORROW_BIT) == 0 && m_pBuffer != nullptr; }
   
   /// ## @API [tag: query] [summary: Memory queries]
   
   [[nodiscard]] bool contains(const void* pMemory) const noexcept;           // check if pointer is within arena bounds

private:
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
{
}

/** -------------------------------------------------------------------------- Borrow buffer constructor
 * @brief Creates an arena that borrows or owns storage
 * 
 * If `pBuffer` is not nullptr, the arena borrows that external storage.
 * If `pBuffer` is nullptr and `uCapacity` > 0, the arena allocates owned storage on heap.
 * 
 * @param pBuffer Pointer to external storage buffer (nullptr to allocate on heap)
 * @param uCapacity Number of bytes the buffer can hold (or to allocate if pBuffer is nullptr)
 */
inline arena::arena(void* pBuffer, size_type uCapacity) noexcept 
{
   if( pBuffer != nullptr )
   {
      // ## Borrow external storage .............................................
      m_pBuffer = static_cast<byte_type*>(pBuffer);
      m_uUsed = 0;
      m_uCapacity = uCapacity | BORROW_BIT;                                   // set BORROW_BIT to indicate borrowed storage
   }
   else if( uCapacity > 0 )
   {
      // ## Allocate owned storage on heap ......................................
      m_pBuffer = static_cast<byte_type*>(::operator new(uCapacity, std::align_val_t(alignof(std::max_align_t))));
      m_uUsed = 0;
      m_uCapacity = uCapacity;                                                // no BORROW_BIT - we own this memory
   }
   else
   {
      // ## Empty arena .........................................................
      m_pBuffer = nullptr;
      m_uUsed = 0;
      m_uCapacity = 0;
   }
}

/** -------------------------------------------------------------------------- Borrow from C-array constructor
 * @brief Creates an arena that borrows storage from a C-array
 * 
 * @tparam uN Size of the array in bytes
 * @param array_ Reference to byte array to borrow storage from
 */
template<std::size_t uN>
inline arena::arena(std::byte (&array_)[uN]) noexcept 
   : m_pBuffer(array_), 
     m_uUsed(0), 
     m_uCapacity(uN | BORROW_BIT)                                             // note the BORROW_BIT is set in capacity to indicate borrowed storage
{
}

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
   : m_pBuffer(o.m_pBuffer), 
     m_uUsed(o.m_uUsed), 
     m_uCapacity(o.m_uCapacity) 
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
 * Allocates `uBytes` bytes with specified alignment. Returns nullptr if 
 * insufficient space is available. The arena NEVER grows beyond its initial capacity.
 * 
 * @param uBytes Number of bytes to allocate
 * @param uAlignment Alignment requirement (default: alignof(std::max_align_t))
 * @return Pointer to allocated memory, or nullptr if insufficient space
 */
inline void* arena::allocate(size_type uBytes, size_type uAlignment) noexcept
{
   // ## Align current position ..............................................
   size_type uAlignedUsed = align_up(m_uUsed, uAlignment);
   size_type uNewUsed = uAlignedUsed + uBytes;
   
   // ## Check if allocation fits in current buffer ..........................
   if( uNewUsed > capacity() ) { return nullptr; }                            // return nullptr if insufficient space
   
   void* pResult = m_pBuffer + uAlignedUsed;
   m_uUsed = uNewUsed;
   return pResult;
}

/** -------------------------------------------------------------------------- allocate_objects
 * @brief Allocate memory for an array of objects with correct type alignment
 * @param uCount Number of objects to allocate
 * @return Pointer to allocated memory
 */
template<typename OBJECT>
OBJECT* arena::allocate_objects(std::size_t uCount)
{
   return static_cast<OBJECT*>( allocate(sizeof(OBJECT) * uCount, alignof(OBJECT)) );
}

/** -------------------------------------------------------------------------- allocate_span
 * @brief Allocate memory for an array of objects and return as a span
 * @param uSize Number of objects to allocate
 * @return Span of allocated objects
 */
template<typename OBJECT>
std::span<OBJECT> arena::allocate_span(size_type uSize)
{
   // This now correctly inherits the alignment from the updated allocate_objects.
   OBJECT* pobject_ = allocate_objects<OBJECT>(uSize);
   return std::span<OBJECT>(pobject_, uSize);
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
 * reclaiming all allocated memory.
 */
inline void arena::reset() noexcept 
{
   m_uUsed = 0;
}

/** -------------------------------------------------------------------------- contains
 * @brief Check if a pointer is within arena bounds
 * 
 * @param pMemory Pointer to check
 * @return true if pointer is within arena's memory range
 */
inline bool arena::contains(const void* pMemory) const noexcept
{
   if( m_pBuffer == nullptr || pMemory == nullptr ) { return false; }
   
   const byte_type* pByte = static_cast<const byte_type*>(pMemory);
   return pByte >= m_pBuffer && pByte < (m_pBuffer + capacity());
}

// ============================================================================
// ## Private helper methods
// ============================================================================

/** -------------------------------------------------------------------------- destroy
 * @brief Deallocate owned storage
 * 
 * Only deallocates if the arena owns its storage (BORROW_BIT is not set).
 * Borrowed storage is never deallocated.
 */
inline void arena::destroy() noexcept 
{
   if( owner() == true )                                                      // only deallocate if we own the storage
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
// ## STL-compatible allocator adapter with heap fallback
// ============================================================================

/** ==========================================================================
 * @brief STL-compatible allocator that uses arena with heap fallback
 * 
 * This allocator tries to allocate from the arena first. If the arena is full
 * or the allocation is too large, it falls back to heap allocation.
 * 
 * **Key feature**: Tracks whether each allocation came from arena or heap,
 * so heap allocations are properly deallocated while arena allocations are not.
 * 
 * **Memory Tracking Strategy**:
 * - Arena allocations: Identified by checking if pointer is within arena bounds
 * - Heap allocations: Uses a small header before the user data to store size
 * 
 * @par Example - Borrowed Storage
 * @code{.cpp}
 * std::array<std::byte, 256> buffer_;
 * gd::arena::borrow::arena arenaLocal( buffer_ );
 * 
 * using string_type = std::basic_string<char, std::char_traits<char>, 
 *                                        arena_allocator<char>>;
 * string_type stringLocal( arena_allocator<char>(&arenaLocal) );
 * 
 * stringLocal = "Small";  // allocated from borrowed arena
 * stringLocal = "This is a very long string...";  // automatically uses heap when arena is full
 * @endcode
 * 
 * @par Example - Owned Storage
 * @code{.cpp}
 * gd::arena::borrow::arena arenaLocal( nullptr, 512 );  // arena owns 512 bytes on heap
 * 
 * std::vector<int, arena_allocator<int>> vectorLocal( arena_allocator<int>(&arenaLocal) );
 * 
 * for( int i = 0; i < 100; ++i )
 * {
 *    vectorLocal.push_back(i);  // uses arena first, then heap fallback
 * }
 * // When vectorLocal is destroyed, heap allocations are freed
 * // When arenaLocal is destroyed, its owned 512 bytes are freed
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
   
   /// ## Allocation header for heap allocations
   struct allocation_header
   {
      size_type uSize;          ///< size of allocation in bytes (not including header)
   };
   
   /// ## Constructors
   
   arena_allocator() noexcept : m_parena(nullptr) {}
   explicit arena_allocator(arena* parena_) noexcept : m_parena(parena_) {}
   explicit arena_allocator(arena& arena_) noexcept : m_parena(&arena_) {}
   
   template<typename U>
   arena_allocator(const arena_allocator<U>& o) noexcept : m_parena(o.m_parena) {}

   /// ## @API [tag: allocate] [summary: Allocation interface]

   [[nodiscard]] T* allocate(size_type uCount) 
   {
      const size_type uBytes = uCount * sizeof(T);
      
      // ## Try arena allocation first ..........................................
      if( m_parena != nullptr )
      {
         void* pMemory = m_parena->allocate(uBytes, alignof(T));
         if( pMemory != nullptr )
         {
            return static_cast<T*>(pMemory);                                  // arena allocation succeeded
         }
      }
      
      // ## Fallback to heap allocation with header .............................
      const size_type uTotalBytes = sizeof(allocation_header) + uBytes;
      void* pRawMemory = ::operator new(uTotalBytes, std::align_val_t(alignof(allocation_header)));
      
      // ## Write header ........................................................
      allocation_header* pheader = static_cast<allocation_header*>(pRawMemory);
      pheader->uSize = uBytes;
      
      // ## Return pointer after header .........................................
      return reinterpret_cast<T*>(pheader + 1);
   }
   
   void deallocate(T* pMemory, size_type uCount) noexcept 
   {
      if( pMemory == nullptr ) { return; }
      
      // ## Check if allocation is from arena ...................................
      if( m_parena != nullptr && m_parena->contains(pMemory) )
      {
         // ## Arena allocation - no-op (arena handles bulk deallocation) .......
         return;
      }
      
      // ## Heap allocation - read header and deallocate ........................
      allocation_header* pheader = reinterpret_cast<allocation_header*>(pMemory) - 1;
      const size_type uTotalBytes = sizeof(allocation_header) + pheader->uSize;
      ::operator delete(pheader, uTotalBytes, std::align_val_t(alignof(allocation_header)));
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

#endif // GD_COMPILER_HAS_CPP20_SUPPORT
