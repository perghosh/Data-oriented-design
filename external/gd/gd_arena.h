// @FILE [tag: arena] [description: Arena allocator with block chaining and alignment] [type: header] [name: gd_arena.h]

/**
 * \file gd_arena.h
 *
 * \brief Arena allocator with block chaining and 32-bit alignment
 *
 * Arena is a memory allocator that allocates memory in blocks. Each block is aligned to 32-bit boundaries
 * for optimal cache performance. When a block is full, a new block is automatically created and linked.
 * The arena supports iteration over all blocks and allocations within blocks for debugging and diagnostics.
 *
 | Area                | Methods (Examples)                                                                                      | Description                                                                                   |
 |---------------------|--------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `arena()`, `arena(size_type)`, `arena(const arena&)`, `arena(arena&&)`                                | Constructors for creating arena instances with specified block sizes.                        |
 | Assignment          | `operator=(const arena&)`, `operator=(arena&&)`, `swap(...)`                                           | Methods for assigning or moving arena contents, including efficient swap operations.         |
 | Allocation          | `allocate(...)`, `allocate_aligned(...)`, `deallocate(...)`                                           | Methods for allocating and deallocating memory from the arena.                               |
 | Capacity            | `block_size()`, `block_count()`, `total_allocated()`, `total_capacity()`, `fragmentation()`           | Methods for querying arena capacity, usage, and fragmentation statistics.                    |
 | Iteration           | `begin_blocks()`, `end_blocks()`, `begin_allocations(...)`, `end_allocations(...)`                    | Iterator methods for traversing blocks and allocations within blocks.                        |
 | Diagnostics         | `dump_blocks()`, `dump_allocations()`, `validate()`                                                   | Methods for debugging and validating arena state.                                            |
 | Management          | `clear()`, `reset()`, `shrink_to_fit()`                                                               | Methods for clearing allocations and managing arena memory.                                  |
 */

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

/**
 * \brief Namespace for arena memory allocators
 */
namespace arena {

// ============================================================================
// ## Forward declarations
// ============================================================================

template<typename ALLOCATOR = std::allocator<std::byte>>                      // @NOTE [tag: template, default] [description: default template parameter for allocator type used in arena]
class arena;

// ============================================================================
// ## Constants and types
// ============================================================================

/// Magic number for block header validation
constexpr std::uint32_t BLOCK_MAGIC = 0x424C4F4Bu;  // 'BLOK' in ASCII

/// Magic number for allocation header validation
constexpr std::uint32_t ALLOC_MAGIC = 0x414C4F43u;  // 'ALOC' in ASCII

/// Default alignment boundary (32 bits = 4 bytes)
constexpr std::size_t DEFAULT_ALIGNMENT = 4;

/// Default block size (64KB)
constexpr std::size_t DEFAULT_BLOCK_SIZE = 65536;

// ============================================================================
// ## Block header structure
// ============================================================================

/**
 * \brief Header for each memory block in the arena
 * 
 * Each block contains a magic number for validation, size information,
 * usage tracking, and a link to the next block.
 */
struct block_header
{
   std::uint32_t m_uMagic;          ///< Magic number for validation (BLOCK_MAGIC)
   std::uint32_t m_uBlockSize;      ///< Total size of this block including header
   std::uint32_t m_uUsedSize;       ///< Current used bytes in this block
   std::uint32_t m_uAllocCount;     ///< Number of allocations in this block
   block_header* m_pblockheaderNext;///< Pointer to next block in chain
   std::byte*    m_pData;           ///< Pointer to usable data area

   block_header() noexcept : m_uMagic(BLOCK_MAGIC), m_uBlockSize(0), m_uUsedSize(0), m_uAllocCount(0), m_pblockheaderNext(nullptr), m_pData(nullptr) {}
   
   [[nodiscard]] bool is_valid() const noexcept { return m_uMagic == BLOCK_MAGIC; } ///< Check if the block header is valid
   [[nodiscard]] std::size_t available() const noexcept { return m_uBlockSize - m_uUsedSize; } ///< Available bytes in this block
   [[nodiscard]] std::byte* current_position() noexcept { return m_pData + m_uUsedSize; } ///< Current position in this block (pointer to first position in block that is free)
   [[nodiscard]] block_header* next_block() const noexcept { return m_pblockheaderNext; } ///< Pointer to next block in chain
   void next_block( block_header* pNext ) noexcept { m_pblockheaderNext = pNext; } ///< Pointer to next block in chain
};

// ============================================================================
// ## Allocation header structure
// ============================================================================

/**
 * \brief Header for each allocation within a block
 * 
 * Each allocation is prefixed with this header for tracking and validation.
 */
struct allocation_header
{
   std::uint32_t m_uMagic;          ///< Magic number for validation (ALLOC_MAGIC)
   std::uint32_t m_uSize;           ///< Size of allocation (excluding header)
   std::uint32_t m_uAlignment;      ///< Alignment used for this allocation
   std::uint32_t m_uPadding;        ///< Reserved for future use

   allocation_header() noexcept : m_uMagic(ALLOC_MAGIC), m_uSize(0), m_uAlignment(0), m_uPadding(0) {}
   
   [[nodiscard]] bool is_valid() const noexcept { return m_uMagic == ALLOC_MAGIC; }
};

// ============================================================================
// ## Block iterator
// ============================================================================

/**
 * \brief Iterator for traversing blocks in the arena
 */
class block_iterator
{
public:
   using iterator_category = std::forward_iterator_tag;
   using value_type        = block_header;
   using difference_type   = std::ptrdiff_t;
   using pointer           = block_header*;
   using reference         = block_header&;

   block_iterator() noexcept : m_pBlock(nullptr) {}
   explicit block_iterator(block_header* pBlock) noexcept : m_pBlock(pBlock) {}

   [[nodiscard]] reference operator*() const noexcept { assert(m_pBlock != nullptr); return *m_pBlock; }
   [[nodiscard]] pointer operator->() const noexcept { assert(m_pBlock != nullptr); return m_pBlock; }

   block_iterator& operator++() noexcept { if(m_pBlock) { m_pBlock = m_pBlock->next_block(); } return *this; }
   block_iterator operator++(int) noexcept { block_iterator tmp = *this; ++(*this); return tmp; }

   [[nodiscard]] bool operator==(const block_iterator& o) const noexcept { return m_pBlock == o.m_pBlock; }
   [[nodiscard]] bool operator!=(const block_iterator& o) const noexcept { return m_pBlock != o.m_pBlock; }

private:
   block_header* m_pBlock;
};

// ============================================================================
// ## Allocation iterator
// ============================================================================

/**
 * \brief Iterator for traversing allocations within a block
 */
class allocation_iterator
{
public:
   using iterator_category = std::forward_iterator_tag;
   using value_type        = allocation_header;
   using difference_type   = std::ptrdiff_t;
   using pointer           = allocation_header*;
   using reference         = allocation_header&;

   allocation_iterator() noexcept : m_pAlloc(nullptr), m_pEnd(nullptr) {}
   allocation_iterator(std::byte* pStart, std::byte* pEnd) noexcept : m_pAlloc(reinterpret_cast<allocation_header*>(pStart)), m_pEnd(pEnd) { if(m_pAlloc && !is_valid_position()) { m_pAlloc = nullptr; } }

   [[nodiscard]] reference operator*() const noexcept { assert(m_pAlloc != nullptr); return *m_pAlloc; }
   [[nodiscard]] pointer operator->() const noexcept { assert(m_pAlloc != nullptr); return m_pAlloc; }
   [[nodiscard]] void* data() const noexcept { assert(m_pAlloc != nullptr); return reinterpret_cast<std::byte*>(m_pAlloc) + sizeof(allocation_header); }

   allocation_iterator& operator++() noexcept
   {
      if(m_pAlloc)
      {
         std::byte* pNext = reinterpret_cast<std::byte*>(m_pAlloc) + sizeof(allocation_header) + m_pAlloc->m_uSize;
         std::size_t uAlignment = m_pAlloc->m_uAlignment > 0 ? m_pAlloc->m_uAlignment : DEFAULT_ALIGNMENT;
         std::size_t uMisalignment = reinterpret_cast<std::uintptr_t>(pNext) % uAlignment;
         if(uMisalignment != 0) { pNext += (uAlignment - uMisalignment); }
         
         m_pAlloc = reinterpret_cast<allocation_header*>(pNext);
         if(!is_valid_position()) { m_pAlloc = nullptr; }
      }
      return *this;
   }
   
   allocation_iterator operator++(int) noexcept { allocation_iterator tmp = *this; ++(*this); return tmp; }

   [[nodiscard]] bool operator==(const allocation_iterator& o) const noexcept { return m_pAlloc == o.m_pAlloc; }
   [[nodiscard]] bool operator!=(const allocation_iterator& o) const noexcept { return m_pAlloc != o.m_pAlloc; }

private:
   [[nodiscard]] bool is_valid_position() const noexcept
   {
      if(!m_pAlloc) { return false; }
      std::byte* pPos = reinterpret_cast<std::byte*>(m_pAlloc);
      if(pPos >= m_pEnd) { return false; }
      if(pPos + sizeof(allocation_header) > m_pEnd) { return false; }
      return m_pAlloc->is_valid();
   }

   allocation_header* m_pAlloc;
   std::byte* m_pEnd;
};

// ============================================================================
// ## Arena allocator class
// ============================================================================

/**
 * \brief Arena allocator with block chaining and 32-bit alignment
 * 
 * The arena allocates memory in blocks. When a block is full, a new block
 * is created and linked to the previous block. All allocations are aligned
 * to 32-bit boundaries (or custom alignment) for optimal cache performance.
 * 
 * \par Example
 * \code{.cpp}
 * gd::arena::arena<> myArena(8192);  // Create arena with 8KB blocks
 * void* p1 = myArena.allocate(100);  // Allocate 100 bytes
 * void* p2 = myArena.allocate(200);  // Allocate 200 bytes
 * 
 * // Iterate over all blocks
 * for (auto it = myArena.begin_blocks(); it != myArena.end_blocks(); ++it)
 * {
 *    std::cout << "Block size: " << it->m_uBlockSize << ", Used: " << it->m_uUsedSize << std::endl;
 * }
 * 
 * myArena.clear();  // Reset arena (keeps first block)
 * \endcode
 * 
 * \tparam ALLOCATOR Allocator type for allocating blocks (default: std::allocator<std::byte>)
 */
template<typename ALLOCATOR>
class arena
{
public:
   using allocator_type  = ALLOCATOR;
   using size_type       = std::size_t;
   using difference_type = std::ptrdiff_t;
   using pointer         = void*;
   using const_pointer   = const void*;

   /// ## Constructors

   arena() noexcept(noexcept(ALLOCATOR()));
   explicit arena(size_type uBlockSize) noexcept(noexcept(ALLOCATOR()));
   explicit arena(size_type uBlockSize, const ALLOCATOR& allocator_) noexcept(noexcept(ALLOCATOR(allocator_)));
   
   arena(const arena& o);
   arena(arena&& o) noexcept;

   ~arena() noexcept;

   /// ## @API [tag: assign] [summary: Assignment operators]

   arena& operator=(const arena& o);
   arena& operator=(arena&& o) noexcept;

   /// ## @API [tag: allocate] [summary: Memory allocation]

   [[nodiscard]] void* allocate(size_type uSize);
   [[nodiscard]] void* allocate_aligned(size_type uSize, size_type uAlignment);
   template<typename OBJECT>
   [[nodiscard]] OBJECT* allocate_objects(size_type uCount);
   template<typename OBJECT>
   [[nodiscard]] std::span<OBJECT> allocate_span(size_type uCount);
   void deallocate(void* pPtr, size_type uSize) noexcept;  // No-op for arena allocators

   /// ## @API [tag: capacity] [summary: Capacity and statistics]

   [[nodiscard]] size_type block_size() const noexcept { return m_uBlockSize; }
   [[nodiscard]] size_type block_count() const noexcept;
   [[nodiscard]] size_type total_allocated() const noexcept;
   [[nodiscard]] size_type total_capacity() const noexcept;
   [[nodiscard]] double fragmentation() const noexcept;

   /// ## @API [tag: iterator] [summary: Block and allocation iteration]

   [[nodiscard]] block_iterator begin_blocks() noexcept { return block_iterator(m_pblockheaderFirst); }
   [[nodiscard]] block_iterator end_blocks() noexcept { return block_iterator(nullptr); }
   [[nodiscard]] allocation_iterator begin_allocations(block_header* pBlock) noexcept;
   [[nodiscard]] allocation_iterator end_allocations(block_header* pBlock) noexcept;

   /// ## @API [tag: diagnostics] [summary: Debugging and validation]

   void dump_blocks() const noexcept;
   void dump_allocations(const block_header* pBlock) const noexcept;
   [[nodiscard]] bool validate() const noexcept;

   /// ## @API [tag: management] [summary: Arena management]

   void clear() noexcept;
   void reset() noexcept;
   void shrink_to_fit() noexcept;
   void swap(arena& o) noexcept;

private:
   [[nodiscard]] block_header* create_block(size_type uSize);
   void destroy() noexcept; ///< Deallocates all blocks and resets arena state, arena becomes empty and not usable until new blocks are allocated
   [[nodiscard]] void* allocate_from_block(block_header* pBlock, size_type uSize, size_type uAlignment);
   [[nodiscard]] static size_type align_size(size_type uSize, size_type uAlignment) noexcept;

private:
   block_header* m_pblockheaderFirst;///< Pointer to first block in chain
   block_header* m_pblockheaderCurrent;///< Pointer to current block for allocation
   size_type     m_uBlockSize;      ///< Size of each block
   ALLOCATOR     m_allocator;       ///< Allocator for block memory
};

// ============================================================================
// ## Out-of-line constructor definitions
// ============================================================================

/** -------------------------------------------------------------------------- Default constructor
 * @brief Creates an empty arena with default block size
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>::arena() noexcept(noexcept(ALLOCATOR())) 
   : m_pblockheaderFirst(nullptr), m_pblockheaderCurrent(nullptr), m_uBlockSize(DEFAULT_BLOCK_SIZE), m_allocator()
{
}

/** -------------------------------------------------------------------------- Block size constructor
 * @brief Creates an arena with specified block size
 * 
 * @param uBlockSize Size of each block in bytes
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>::arena(size_type uBlockSize) noexcept(noexcept(ALLOCATOR())) 
   : m_pblockheaderFirst(nullptr), m_pblockheaderCurrent(nullptr), m_uBlockSize(uBlockSize > 0 ? uBlockSize : DEFAULT_BLOCK_SIZE), m_allocator()
{
}

/** -------------------------------------------------------------------------- Block size and allocator constructor
 * @brief Creates an arena with specified block size and allocator
 * 
 * @param uBlockSize Size of each block in bytes
 * @param allocator_ Allocator to use for block allocation
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>::arena(size_type uBlockSize, const ALLOCATOR& allocator_) noexcept(noexcept(ALLOCATOR(allocator_))) 
   : m_pblockheaderFirst(nullptr), m_pblockheaderCurrent(nullptr), m_uBlockSize(uBlockSize > 0 ? uBlockSize : DEFAULT_BLOCK_SIZE), m_allocator(allocator_)
{
}

/** -------------------------------------------------------------------------- Copy constructor
 * @brief Creates a copy of another arena
 * 
 * Note: This performs a deep copy of all blocks and allocations.
 * 
 * @param o Arena to copy from
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>::arena(const arena& o) 
   : m_pblockheaderFirst(nullptr), m_pblockheaderCurrent(nullptr), m_uBlockSize(o.m_uBlockSize), m_allocator(o.m_allocator)
{
   // Deep copy all blocks
   block_header* pSourceBlock = o.m_pblockheaderFirst;
   block_header** ppDestBlock = &m_pblockheaderFirst;
   
   while (pSourceBlock)
   {
      // Allocate memory for new block
      size_type uTotalSize = sizeof(block_header) + pSourceBlock->m_uBlockSize;
      std::byte* pMemory = m_allocator.allocate(uTotalSize);
      
      // Copy block header
      block_header* pNewBlock = new (pMemory) block_header();
      pNewBlock->m_uBlockSize = pSourceBlock->m_uBlockSize;
      pNewBlock->m_uUsedSize = pSourceBlock->m_uUsedSize;
      pNewBlock->m_uAllocCount = pSourceBlock->m_uAllocCount;
      pNewBlock->m_pData = pMemory + sizeof(block_header);
      
      // Copy block data
      std::memcpy(pNewBlock->m_pData, pSourceBlock->m_pData, pSourceBlock->m_uUsedSize);
      
      // Link block
      *ppDestBlock = pNewBlock;
      ppDestBlock = &pNewBlock->m_pblockheaderNext;
      
      // Update current block pointer
      if(pSourceBlock == o.m_pblockheaderCurrent) { m_pblockheaderCurrent = pNewBlock; }
      
      pSourceBlock = pSourceBlock->next_block();
   }
}

/** -------------------------------------------------------------------------- Move constructor
 * @brief Moves another arena's contents
 * 
 * @param o Arena to move from
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>::arena(arena&& o) noexcept 
   : m_pblockheaderFirst(o.m_pblockheaderFirst), m_pblockheaderCurrent(o.m_pblockheaderCurrent), m_uBlockSize(o.m_uBlockSize), m_allocator(std::move(o.m_allocator))
{
   o.m_pblockheaderFirst = nullptr;
   o.m_pblockheaderCurrent = nullptr;
}

/** -------------------------------------------------------------------------- Destructor
 * @brief Destroys arena and deallocates all blocks
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>::~arena() noexcept
{
   destroy();
}

// ============================================================================
// ## Assignment operators
// ============================================================================

/** -------------------------------------------------------------------------- Copy assignment
 * @brief Assigns a copy of another arena
 * 
 * @param o Arena to copy from
 * @return Reference to this arena
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>& arena<ALLOCATOR>::operator=(const arena& o)
{
   if(this != &o)
   {
      destroy();
      m_uBlockSize = o.m_uBlockSize;
      m_allocator = o.m_allocator;
      
      // Deep copy all blocks (same logic as copy constructor)
      block_header* pSourceBlock = o.m_pblockheaderFirst;
      block_header** ppDestBlock = &m_pblockheaderFirst;
      
      while (pSourceBlock)
      {
         size_type uTotalSize = sizeof(block_header) + pSourceBlock->m_uBlockSize;
         std::byte* pMemory = m_allocator.allocate(uTotalSize);
         
         block_header* pNewBlock = new (pMemory) block_header();
         pNewBlock->m_uBlockSize = pSourceBlock->m_uBlockSize;
         pNewBlock->m_uUsedSize = pSourceBlock->m_uUsedSize;
         pNewBlock->m_uAllocCount = pSourceBlock->m_uAllocCount;
         pNewBlock->m_pData = pMemory + sizeof(block_header);
         
         std::memcpy(pNewBlock->m_pData, pSourceBlock->m_pData, pSourceBlock->m_uUsedSize);
         
         *ppDestBlock = pNewBlock;
         ppDestBlock = &pNewBlock->m_pblockheaderNext;
         
         if(pSourceBlock == o.m_pblockheaderCurrent) { m_pblockheaderCurrent = pNewBlock; }
         
         pSourceBlock = pSourceBlock->next_block();
      }
   }
   return *this;
}

/** -------------------------------------------------------------------------- Move assignment
 * @brief Moves another arena's contents
 * 
 * @param o Arena to move from
 * @return Reference to this arena
 */
template<typename ALLOCATOR>
arena<ALLOCATOR>& arena<ALLOCATOR>::operator=(arena&& o) noexcept
{
   if(this != &o)
   {
      destroy();
      m_pblockheaderFirst = o.m_pblockheaderFirst;
      m_pblockheaderCurrent = o.m_pblockheaderCurrent;
      m_uBlockSize = o.m_uBlockSize;
      m_allocator = std::move(o.m_allocator);
      
      o.m_pblockheaderFirst = nullptr;
      o.m_pblockheaderCurrent = nullptr;
   }
   return *this;
}

// ============================================================================
// ## Allocation methods
// ============================================================================

/** -------------------------------------------------------------------------- allocate
 * @brief Allocate memory with default alignment
 * 
 * @param uSize Number of bytes to allocate
 * @return Pointer to allocated memory
 */
template<typename ALLOCATOR>
void* arena<ALLOCATOR>::allocate(size_type uSize)
{
   return allocate_aligned(uSize, DEFAULT_ALIGNMENT);
}

/** -------------------------------------------------------------------------- allocate_objects
 * @brief Allocate memory for an array of objects with correct type alignment
 * * @param uCount Number of objects to allocate
 * @return Pointer to allocated memory
 */
template<typename ALLOCATOR>
template<typename OBJECT>
OBJECT* arena<ALLOCATOR>::allocate_objects(std::size_t uCount)
{
   return static_cast<OBJECT*>( allocate_aligned(sizeof(OBJECT) * uCount, alignof(OBJECT)) );
}

/** -------------------------------------------------------------------------- allocate_span
 * @brief Allocate memory for an array of objects and return as a span
 * * @param uSize Number of objects to allocate
 * @return Span of allocated objects
 */
template<typename ALLOCATOR>
template<typename OBJECT>
std::span<OBJECT> arena<ALLOCATOR>::allocate_span(size_type uSize)
{
   // This now correctly inherits the alignment from the updated allocate_objects.
   OBJECT* pobject_ = allocate_objects<OBJECT>(uSize);
   return std::span<OBJECT>(pobject_, uSize);
}

/** -------------------------------------------------------------------------- allocate_aligned
 * @brief Allocate memory with specified alignment
 * 
 * @param uSize Number of bytes to allocate
 * @param uAlignment Alignment boundary (must be power of 2)
 * @return Pointer to allocated memory
 */
template<typename ALLOCATOR>
void* arena<ALLOCATOR>::allocate_aligned(size_type uSize, size_type uAlignment)
{                                                                                                  assert(uSize <= 0xFFFFFFFFu && "Allocation size exceeds 4GB limit of uint32_t headers");
                                                                                                   assert(uAlignment > 0 && (uAlignment & (uAlignment - 1)) == 0);  // verify alignment is power of 2
                                                                                                   assert(uSize > 0);  // verify size is non-zero
   // ## Try to allocate from current block ..................................
   if(m_pblockheaderCurrent != nullptr)
   {
      void* pResult = allocate_from_block(m_pblockheaderCurrent, uSize, uAlignment);
      if(pResult) { return pResult; }
   }
   
   // ## Need new block - calculate required size ............................
   size_type uRequiredSize = sizeof(allocation_header) + uSize + uAlignment;
   size_type uNewBlockSize = std::max(m_uBlockSize, uRequiredSize);
   
   block_header* pNewBlock = create_block(uNewBlockSize);                     // Create new block
   
   // ## Link to chain .......................................................
   if(m_pblockheaderCurrent) { m_pblockheaderCurrent->next_block(pNewBlock); }
   else { m_pblockheaderFirst = pNewBlock; }
   
   m_pblockheaderCurrent = pNewBlock;
   
   // ## Allocate from new block .............................................
   void* pResult = allocate_from_block(pNewBlock, uSize, uAlignment);                              assert(pResult != nullptr);  // verify allocation succeeded in new block
   return pResult;
}

/** -------------------------------------------------------------------------- deallocate
 * @brief Deallocate memory (no-op for arena allocators)
 * 
 * Arena allocators do not support individual deallocation. Use clear() or
 * reset() to reclaim all memory at once.
 * 
 * @param p_ Pointer to memory (ignored)
 * @param uSize Size of memory (ignored)
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::deallocate(void* p_, size_type uSize) noexcept
{
   // Arena allocators do not support individual deallocation
   (void)p_;
   (void)uSize;
}

// ============================================================================
// ## Capacity methods
// ============================================================================

/** -------------------------------------------------------------------------- block_count
 * @brief Get total number of blocks in the arena
 * 
 * @return Number of blocks
 */
template<typename ALLOCATOR>
typename arena<ALLOCATOR>::size_type arena<ALLOCATOR>::block_count() const noexcept
{
   size_type uCount = 0;
   for (block_header* pBlock = m_pblockheaderFirst; pBlock != nullptr; pBlock = pBlock->next_block()) { ++uCount; }
   return uCount;
}

/** -------------------------------------------------------------------------- total_allocated
 * @brief Get total bytes allocated across all blocks
 * 
 * @return Total allocated bytes
 */
template<typename ALLOCATOR>
typename arena<ALLOCATOR>::size_type arena<ALLOCATOR>::total_allocated() const noexcept
{
   size_type uTotal = 0;
   for (block_header* pBlock = m_pblockheaderFirst; pBlock != nullptr; pBlock = pBlock->next_block()) { uTotal += pBlock->m_uUsedSize; }
   return uTotal;
}

/** -------------------------------------------------------------------------- total_capacity
 * @brief Get total capacity across all blocks
 * 
 * @return Total capacity in bytes
 */
template<typename ALLOCATOR>
typename arena<ALLOCATOR>::size_type arena<ALLOCATOR>::total_capacity() const noexcept
{
   size_type uTotal = 0;
   for (block_header* pBlock = m_pblockheaderFirst; pBlock != nullptr; pBlock = pBlock->next_block()) { uTotal += pBlock->m_uBlockSize; }
   return uTotal;
}

/** -------------------------------------------------------------------------- fragmentation
 * @brief Calculate fragmentation ratio
 * 
 * Returns ratio of wasted space to total capacity. Lower values indicate
 * better memory utilization.
 * 
 * @return Fragmentation ratio (0.0 = no waste, 1.0 = all waste)
 */
template<typename ALLOCATOR>
double arena<ALLOCATOR>::fragmentation() const noexcept
{
   size_type uCapacity = total_capacity();
   if(uCapacity == 0) { return 0.0; }
   size_type uAllocated = total_allocated();
   return static_cast<double>(uCapacity - uAllocated) / static_cast<double>(uCapacity);
}

// ============================================================================
// ## Iterator methods
// ============================================================================

/** -------------------------------------------------------------------------- begin_allocations
 * @brief Get iterator to first allocation in block
 * 
 * @param pBlock Block to iterate
 * @return Iterator to first allocation
 */
template<typename ALLOCATOR>
allocation_iterator arena<ALLOCATOR>::begin_allocations(block_header* pBlock) noexcept
{
   if(!pBlock || pBlock->m_uUsedSize == 0) { return allocation_iterator(); }
   return allocation_iterator(pBlock->m_pData, pBlock->m_pData + pBlock->m_uUsedSize);
}

/** -------------------------------------------------------------------------- end_allocations
 * @brief Get iterator past last allocation in block
 * 
 * @param pBlock Block to iterate
 * @return Iterator past last allocation
 */
template<typename ALLOCATOR>
allocation_iterator arena<ALLOCATOR>::end_allocations(block_header* pBlock) noexcept
{
   (void)pBlock;
   return allocation_iterator();
}

// ============================================================================
// ## Diagnostic methods
// ============================================================================

/** -------------------------------------------------------------------------- dump_blocks
 * @brief Print information about all blocks
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::dump_blocks() const noexcept
{
   size_type uBlockNum = 0;
   for (block_header* pBlock = m_pblockheaderFirst; pBlock != nullptr; pBlock = pBlock->next_block())
   {
      printf("Block %zu: Size=%u, Used=%u, Available=%zu, Allocations=%u, Magic=%08X\n", uBlockNum++, pBlock->m_uBlockSize, pBlock->m_uUsedSize, pBlock->available(), pBlock->m_uAllocCount, pBlock->m_uMagic);
   }
}

/** -------------------------------------------------------------------------- dump_allocations
 * @brief Print information about all allocations in a block
 * 
 * @param pBlock Block to dump allocations from
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::dump_allocations(const block_header* pBlock) const noexcept
{
   if(!pBlock) { return; }
   
   size_type uAllocNum = 0;
   std::byte* pCurrent = pBlock->m_pData;
   std::byte* pEnd = pBlock->m_pData + pBlock->m_uUsedSize;
   
   while (pCurrent < pEnd)
   {
      allocation_header* pAlloc = reinterpret_cast<allocation_header*>(pCurrent);
      if(!pAlloc->is_valid()) { break; }
      
      void* pData = pCurrent + sizeof(allocation_header);
      printf("  Allocation %zu: Size=%u, Alignment=%u, Address=%p, Magic=%08X\n", uAllocNum++, pAlloc->m_uSize, pAlloc->m_uAlignment, pData, pAlloc->m_uMagic);
      
      pCurrent += sizeof(allocation_header) + pAlloc->m_uSize;
      size_type uAlignment = pAlloc->m_uAlignment > 0 ? pAlloc->m_uAlignment : DEFAULT_ALIGNMENT;
      size_type uMisalignment = reinterpret_cast<std::uintptr_t>(pCurrent) % uAlignment;
      if(uMisalignment != 0) { pCurrent += (uAlignment - uMisalignment); }
   }
}

/** -------------------------------------------------------------------------- validate
 * @brief Validate arena integrity
 * 
 * Checks all block and allocation headers for corruption.
 * 
 * @return true if arena is valid, false otherwise
 */
template<typename ALLOCATOR>
bool arena<ALLOCATOR>::validate() const noexcept
{
   for (block_header* pBlock = m_pblockheaderFirst; pBlock != nullptr; pBlock = pBlock->next_block())
   {
      if(!pBlock->is_valid()) { return false; }
      if(pBlock->m_uUsedSize > pBlock->m_uBlockSize) { return false; }
      
      // Validate allocations within block
      std::byte* pCurrent = pBlock->m_pData;
      std::byte* pEnd = pBlock->m_pData + pBlock->m_uUsedSize;
      size_type uAllocCount = 0;
      
      while (pCurrent < pEnd)
      {
         allocation_header* pAlloc = reinterpret_cast<allocation_header*>(pCurrent);
         if(!pAlloc->is_valid()) { return false; }
         
         pCurrent += sizeof(allocation_header) + pAlloc->m_uSize;
         if(pCurrent > pEnd) { return false; }
         
         size_type uAlignment = pAlloc->m_uAlignment > 0 ? pAlloc->m_uAlignment : DEFAULT_ALIGNMENT;
         size_type uMisalignment = reinterpret_cast<std::uintptr_t>(pCurrent) % uAlignment;
         if(uMisalignment != 0) { pCurrent += (uAlignment - uMisalignment); }
         
         ++uAllocCount;
      }
      
      if(uAllocCount != pBlock->m_uAllocCount) { return false; }
   }
   
   return true;
}

// ============================================================================
// ## Management methods
// ============================================================================

/** -------------------------------------------------------------------------- clear
 * @brief Clear all allocations but keep first block
 * 
 * Resets all blocks to empty state but keeps allocated block memory.
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::clear() noexcept
{
   for (block_header* pBlock = m_pblockheaderFirst; pBlock != nullptr; pBlock = pBlock->next_block())
   {
      pBlock->m_uUsedSize = 0;
      pBlock->m_uAllocCount = 0;
   }
   m_pblockheaderCurrent = m_pblockheaderFirst;
}

/** -------------------------------------------------------------------------- reset
 * @brief Deallocate all blocks except the first one
 * 
 * Keeps the first block and deallocates all others, then clears the first block.
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::reset() noexcept
{
   if(!m_pblockheaderFirst) { return; }
   
   // Destroy all blocks except first
   block_header* pBlock = m_pblockheaderFirst->next_block();
   while (pBlock)
   {
      block_header* pNext = pBlock->next_block();
      size_type uTotalSize = sizeof(block_header) + pBlock->m_uBlockSize;
      pBlock->~block_header();
      m_allocator.deallocate(reinterpret_cast<std::byte*>(pBlock), uTotalSize);
      pBlock = pNext;
   }
   
   // Clear first block
   m_pblockheaderFirst->next_block(nullptr);
   m_pblockheaderFirst->m_uUsedSize = 0;
   m_pblockheaderFirst->m_uAllocCount = 0;
   m_pblockheaderCurrent = m_pblockheaderFirst;
}

/** -------------------------------------------------------------------------- shrink_to_fit
 * @brief Remove all empty blocks except the first
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::shrink_to_fit() noexcept
{
   if(!m_pblockheaderFirst) { return; }
   
   block_header* pblockheaderPrevious = nullptr;
   block_header* pblockheader_ = m_pblockheaderFirst; // Start from first block
   
   while( pblockheader_ )                                                     // Iterate through blocks
   {
      block_header* pblockheaderNext = pblockheader_->next_block(); // Cache next block before potential deallocation
      
      // ## Keep first block and non-empty blocks ............................
      if(pblockheader_ == m_pblockheaderFirst || pblockheader_->m_uUsedSize > 0)
      {
         pblockheaderPrevious = pblockheader_;
         pblockheader_ = pblockheaderNext;
         continue;
      }
      
      // ## Deallocate empty block ...........................................
      size_type uTotalSize = sizeof(block_header) + pblockheader_->m_uBlockSize;
      pblockheader_->~block_header();                                         // Explicitly call destructor for block header
      m_allocator.deallocate( reinterpret_cast<std::byte*>( pblockheader_ ), uTotalSize ); // Deallocate current block memory
      
      // ## Update chain .....................................................
      if(pblockheaderPrevious) { pblockheaderPrevious->next_block(pblockheaderNext); }
      if(m_pblockheaderCurrent == pblockheader_) { m_pblockheaderCurrent = pblockheaderPrevious ? pblockheaderPrevious : m_pblockheaderFirst; }
      
      pblockheader_ = pblockheaderNext;
   }
}


/** -------------------------------------------------------------------------- swap
 * @brief Swap contents with another arena
 * 
 * @param o Arena to swap with
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::swap(arena& o) noexcept
{
   std::swap(m_pblockheaderFirst, o.m_pblockheaderFirst);
   std::swap(m_pblockheaderCurrent, o.m_pblockheaderCurrent);
   std::swap(m_uBlockSize, o.m_uBlockSize);
   std::swap(m_allocator, o.m_allocator);
}

// ============================================================================
// ## Private helper methods
// ============================================================================

/** -------------------------------------------------------------------------- create_block
 * @brief Create a new block with specified size
 * 
 * @param uSize Size of block data area (excluding header)
 * @return Pointer to new block header
 */
template<typename ALLOCATOR>
block_header* arena<ALLOCATOR>::create_block(size_type uSize)
{
   size_type uTotalSize = sizeof(block_header) + uSize;
   std::byte* pMemory = m_allocator.allocate(uTotalSize);
   
   block_header* pBlock = new (pMemory) block_header();
   pBlock->m_uBlockSize = static_cast<std::uint32_t>(uSize);
   pBlock->m_pData = pMemory + sizeof(block_header);
   
   return pBlock;
}

/** -------------------------------------------------------------------------- destroy
 * @brief Destroy all blocks and deallocate memory
 */
template<typename ALLOCATOR>
void arena<ALLOCATOR>::destroy() noexcept
{
   block_header* pBlock = m_pblockheaderFirst;
   while (pBlock)
   {
      block_header* pNext = pBlock->next_block();
      size_type uTotalSize = sizeof(block_header) + pBlock->m_uBlockSize;
      pBlock->~block_header();
      m_allocator.deallocate(reinterpret_cast<std::byte*>(pBlock), uTotalSize);
      pBlock = pNext;
   }
   m_pblockheaderFirst = nullptr;
   m_pblockheaderCurrent = nullptr;
}

/** -------------------------------------------------------------------------- allocate_from_block
 * @brief Allocate memory from a specific block
 * 
 * @param pBlock Block to allocate from
 * @param uSize Number of bytes to allocate
 * @param uAlignment Alignment boundary
 * @return Pointer to allocated memory, or nullptr if insufficient space
 */
template<typename ALLOCATOR>
void* arena<ALLOCATOR>::allocate_from_block(block_header* pBlock, size_type uSize, size_type uAlignment)
{
   std::byte* pCurrent = pBlock->current_position();
   std::uintptr_t uCurrentAddress = reinterpret_cast<std::uintptr_t>(pCurrent);
   
   // ## The header must precede the data. 
   //     We find the earliest address for pData that satisfies uAlignment 
   //     AND leaves enough room f½or the header.
   const size_type uHeaderSize = sizeof(allocation_header);
   std::uintptr_t uDataAddress = ( uCurrentAddress + uHeaderSize + ( uAlignment - 1 ) ) & ~( uAlignment - 1 ); // Calculate aligned data address that leaves room for header
   
   std::byte* pData = reinterpret_cast<std::byte*>( uDataAddress );           // pointer to start of data area for this allocation
   allocation_header* pallocationheader_ = reinterpret_cast<allocation_header*>( pData - uHeaderSize );// pointer to allocation header, which is located immediately before data area

   // ## Calculate where the NEXT allocation would start to determine total space used.
   //    We align the end of this allocation to the same boundary to keep the block 'clean'.
   std::byte* pNext = pData + uSize;
   std::uintptr_t uNextAddress = reinterpret_cast<std::uintptr_t>(pNext);
   uNextAddress = (uNextAddress + (uAlignment - 1)) & ~(uAlignment - 1);      // align
   pNext = reinterpret_cast<std::byte*>(uNextAddress);                        // start of next allocation would be immediately after this data

   size_type uTotalNeeded = static_cast<size_type>( pNext - pCurrent ); // Total space needed from current position to end of this allocation (including header and alignment)

   if(uTotalNeeded > pBlock->available()) { return nullptr; }                 // Check if block has enough space

   // Initialize allocation header
   new ( pallocationheader_ ) allocation_header();                            // Placement new to construct header in place (sets magic number)
   pallocationheader_->m_uSize = static_cast<std::uint32_t>(uSize);
   pallocationheader_->m_uAlignment = static_cast<std::uint32_t>(uAlignment);

   // ## Update block state ..................................................
   pBlock->m_uUsedSize += static_cast<std::uint32_t>( uTotalNeeded );         // Update used size
   pBlock->m_uAllocCount += 1;                                                // Allocate successful, update block allocation count

   return pData;
}

/** -------------------------------------------------------------------------- align_size
 * @brief Align size to specified boundary
 * 
 * @param uSize Size to align
 * @param uAlignment Alignment boundary
 * @return Aligned size
 */
template<typename ALLOCATOR>
typename arena<ALLOCATOR>::size_type arena<ALLOCATOR>::align_size(size_type uSize, size_type uAlignment) noexcept
{
   size_type uMisalignment = uSize % uAlignment;
   if(uMisalignment == 0) { return uSize; }
   return uSize + (uAlignment - uMisalignment);
}

} // namespace arena

_GD_END

/// ## std::swap specialization for arena::arena
template<typename ALLOCATOR>
void swap(gd::arena::arena<ALLOCATOR>& lhs, gd::arena::arena<ALLOCATOR>& rhs) noexcept
{
   lhs.swap(rhs);
}

_GD_BEGIN

namespace arena {

// ============================================================================
// ## arena_allocator class
// ============================================================================

/**
 * \brief STL-compatible allocator that uses arena for memory allocation
 * 
 * This allocator wraps an arena instance and provides the standard allocator
 * interface required by STL containers. All allocations are delegated to the
 * underlying arena, which means:
 * - Individual deallocations are no-ops (arena doesn't support individual deallocation)
 * - Memory is reclaimed when the arena is cleared or destroyed
 * - Allocations are fast and cache-friendly due to arena's block-based design
 * 
 * \par Example
 * \code{.cpp}
 * gd::arena::arena<> myArena(8192);
 * gd::arena::arena_allocator<int> allocator(myArena);
 * 
 * std::vector<int, gd::arena::arena_allocator<int>> vec(allocator);
 * vec.push_back(42);
 * vec.push_back(100);
 * 
 * std::basic_string<char, std::char_traits<char>, gd::arena::arena_allocator<char>> str(allocator);
 * str = "Hello from arena!";
 * \endcode
 * 
 * \tparam T Type of objects to allocate
 * \tparam ALLOCATOR Underlying allocator used by the arena (default: std::allocator<std::byte>)
 */
template<typename TYPE, typename ALLOCATOR = std::allocator<std::byte>>
class arena_allocator
{
public:
   /// ## Type definitions required by std::allocator_traits

   using value_type      = TYPE;
   using size_type       = std::size_t;
   using difference_type = std::ptrdiff_t;
   using pointer         = TYPE*;
   using const_pointer   = const TYPE*;
   
   /// Rebind allocator to different type (required for STL containers)
   template<typename U>
   struct rebind
   {
      using other = arena_allocator<U, ALLOCATOR>;
   };

   /// ## Constructors

   /** ----------------------------------------------------------------------- Constructor with arena reference
    * @brief Creates allocator that uses the provided arena
    * 
    * @param arena_ Reference to arena instance to use for allocations
    */
   explicit arena_allocator(arena<ALLOCATOR>& arena_) noexcept 
      : m_parena(&arena_)
   {}

   /** ----------------------------------------------------------------------- Copy constructor
    * @brief Creates allocator from another allocator of same type
    * 
    * @param o Allocator to copy from
    */
   arena_allocator(const arena_allocator& o) noexcept 
      : m_parena(o.m_parena)
   {}

   /** ----------------------------------------------------------------------- Rebind copy constructor
    * @brief Creates allocator from allocator of different type
    * 
    * Allows containers to create allocators for internal types (e.g., node allocators).
    * 
    * @param o Allocator of different type to copy from
    */
   template<typename U>
   arena_allocator(const arena_allocator<U, ALLOCATOR>& o) noexcept 
      : m_parena(o.m_parena)
   {}

   /** ----------------------------------------------------------------------- Move constructor
    * @brief Moves allocator from another allocator
    * 
    * @param o Allocator to move from
    */
   arena_allocator(arena_allocator&& o) noexcept 
      : m_parena(o.m_parena)
   {
      o.m_parena = nullptr;
   }

   /// ## Assignment operators

   /** ----------------------------------------------------------------------- Copy assignment
    * @brief Assigns allocator from another allocator
    * 
    * @param o Allocator to copy from
    * @return Reference to this allocator
    */
   arena_allocator& operator=(const arena_allocator& o) noexcept
   {
      if(this != &o)
      {
         m_parena = o.m_parena;
      }
      return *this;
   }

   /** ----------------------------------------------------------------------- Move assignment
    * @brief Moves allocator from another allocator
    * 
    * @param o Allocator to move from
    * @return Reference to this allocator
    */
   arena_allocator& operator=(arena_allocator&& o) noexcept
   {
      if(this != &o)
      {
         m_parena = o.m_parena;
         o.m_parena = nullptr;
      }
      return *this;
   }

   /// ## Allocation interface (STL required methods)

   /** ----------------------------------------------------------------------- allocate
    * @brief Allocate memory for n objects of type TYPE
    * 
    * Allocates memory from the underlying arena with proper alignment for type TYPE.
    * 
    * @param uCount Number of objects to allocate space for
    * @return Pointer to allocated memory
    * @throws std::bad_alloc if arena pointer is null
    */
   [[nodiscard]] TYPE* allocate(size_type uCount)
   {
      if(!m_parena) { throw std::bad_alloc(); }
      
      size_type uSize = uCount * sizeof(TYPE);
      size_type uAlignment = alignof(TYPE);
      
      void* pMemory = m_parena->allocate_aligned(uSize, uAlignment);
      return static_cast<TYPE*>(pMemory);
   }

   /** ----------------------------------------------------------------------- deallocate
    * @brief Deallocate memory (no-op for arena allocators)
    * 
    * Arena allocators do not support individual deallocation. Memory is reclaimed
    * when the arena is cleared or destroyed. This method is required by the STL
    * allocator interface but does nothing.
    * 
    * @param pPtr Pointer to memory (ignored)
    * @param uCount Number of objects (ignored)
    */
   void deallocate(TYPE* pPtr, size_type uCount) noexcept
   {
      if(m_parena) { m_parena->deallocate(pPtr, uCount * sizeof(TYPE)); }
   }

   /// ## Comparison operators

   /** ----------------------------------------------------------------------- Equality comparison
    * @brief Check if two allocators use the same arena
    * 
    * @param o Allocator to compare with
    * @return true if allocators use the same arena
    */
   [[nodiscard]] bool operator==(const arena_allocator& o) const noexcept
   {
      return m_parena == o.m_parena;
   }

   /** ----------------------------------------------------------------------- Inequality comparison
    * @brief Check if two allocators use different arenas
    * 
    * @param o Allocator to compare with
    * @return true if allocators use different arenas
    */
   [[nodiscard]] bool operator!=(const arena_allocator& o) const noexcept
   {
      return m_parena != o.m_parena;
   }

   /// ## Cross-type comparison operators

   /** ----------------------------------------------------------------------- Cross-type equality comparison
    * @brief Check if allocators of different types use the same arena
    * 
    * @param o Allocator of different type to compare with
    * @return true if allocators use the same arena
    */
   template<typename U>
   [[nodiscard]] bool operator==(const arena_allocator<U, ALLOCATOR>& o) const noexcept
   {
      return m_parena == o.m_parena;
   }

   /** ----------------------------------------------------------------------- Cross-type inequality comparison
    * @brief Check if allocators of different types use different arenas
    * 
    * @param o Allocator of different type to compare with
    * @return true if allocators use different arenas
    */
   template<typename U>
   [[nodiscard]] bool operator!=(const arena_allocator<U, ALLOCATOR>& o) const noexcept
   {
      return m_parena != o.m_parena;
   }

   /// ## Access to underlying arena

   /** -------------------------------------------------------------------------- get_arena
    * @brief Get reference to underlying arena
    * 
    * @return Reference to arena
    * @throws std::runtime_error if arena pointer is null
    */
   [[nodiscard]] arena<ALLOCATOR>& get_arena() const
   {
      if(!m_parena) { throw std::runtime_error("arena_allocator: null arena pointer"); }
      return *m_parena;
   }

private:
   arena<ALLOCATOR>* m_parena;  ///< Pointer to arena instance

   // Allow other instantiations to access private members for rebind
   template<typename U, typename A>
   friend class arena_allocator;
};

// ============================================================================
// ## Convenience type aliases
// ============================================================================

/*
/// String type using arena allocator
template<typename ALLOCATOR = std::allocator<std::byte>>
using arena_string = std::basic_string<char, std::char_traits<char>, arena_allocator<char, ALLOCATOR>>;

/// Vector type using arena allocator
template<typename T, typename ALLOCATOR = std::allocator<std::byte>>
using arena_vector = std::vector<T, arena_allocator<T, ALLOCATOR>>;

/// List type using arena allocator
template<typename T, typename ALLOCATOR = std::allocator<std::byte>>
using arena_list = std::list<T, arena_allocator<T, ALLOCATOR>>;

/// Deque type using arena allocator
template<typename T, typename ALLOCATOR = std::allocator<std::byte>>
using arena_deque = std::deque<T, arena_allocator<T, ALLOCATOR>>;

/// Map type using arena allocator
template<typename KEY, typename VALUE, typename COMPARE = std::less<KEY>, typename ALLOCATOR = std::allocator<std::byte>>
using arena_map = std::map<KEY, VALUE, COMPARE, arena_allocator<std::pair<const KEY, VALUE>, ALLOCATOR>>;

/// Set type using arena allocator
template<typename T, typename COMPARE = std::less<T>, typename ALLOCATOR = std::allocator<std::byte>>
using arena_set = std::set<T, COMPARE, arena_allocator<T, ALLOCATOR>>;

/// Unordered map type using arena allocator
template<typename KEY, typename VALUE, typename HASH = std::hash<KEY>, typename EQUAL = std::equal_to<KEY>, typename ALLOCATOR = std::allocator<std::byte>>
using arena_unordered_map = std::unordered_map<KEY, VALUE, HASH, EQUAL, arena_allocator<std::pair<const KEY, VALUE>, ALLOCATOR>>;

/// Unordered set type using arena allocator
template<typename T, typename HASH = std::hash<T>, typename EQUAL = std::equal_to<T>, typename ALLOCATOR = std::allocator<std::byte>>
using arena_unordered_set = std::unordered_set<T, HASH, EQUAL, arena_allocator<T, ALLOCATOR>>;
*/

} // namespace arena

_GD_END

#endif // GD_COMPILER_HAS_CPP20_SUPPORT

