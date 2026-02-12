// @FILE [tag: vector] [description: Vector variants with special storage features]] [type: header] [name: gd_vector.h]

/**
 * \file gd_vector.h
 *
 * \brief Small vector optimization (SVO) container with inline storage capacity
 *
 * Vector is a hybrid container that stores elements inline up to a specified capacity (uCapacityStack),
 * then dynamically allocates additional storage when needed. This is similar to std::vector but with
 * optimized storage for small containers to avoid heap allocation when possible.
 *
 | Area                | Methods (Examples)                                                                                      | Description                                                                                   |
 |---------------------|--------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `vector()`, `vector(const vector&)`, `vector(vector&&)`, `vector(initializer_list)`, `vector(size_type, const VALUE&)` | Constructors for creating, copying, and moving vector instances with various initialization strategies. |
 | Assignment          | `operator=(const vector&)`, `operator=(vector&&)`, `swap(...)`                                         | Methods for assigning or moving vector contents, including efficient swap operations.        |
 | Element Access      | `operator[](...)`, `at(...)`, `front()`, `back()`, `data()`, `get_buffer()`, `get_inline_buffer()`     | Methods for accessing elements in the container with bounds checking or direct access.       |
 | Iterators           | `begin()`, `end()`, `cbegin()`, `cend()`, `rbegin()`, `rend()`, `crbegin()`, `crend()`                | Iterator methods for forward, const, and reverse traversal of container elements.             |
 | Capacity            | `empty()`, `size()`, `capacity()`, `capacity_inline()`, `reserve(...)`, `is_external()`               | Methods for querying size, capacity, and storage mode (inline vs. heap).                       |
 | Modifiers           | `push_back(...)`, `emplace_back(...)`, `pop_back()`, `resize(...)`, `clear()`                          | Methods for adding, removing, and modifying elements in the container.                       |
 | Comparison          | `operator==(...)`, `operator<=>(...)`                                                                  | Comparison operators for lexicographical comparison between vectors.                          |
 | Memory Management   | `allocate(...)`, `destroy()`, `init_base(...)`                                                          | Internal methods for memory allocation, deallocation, and buffer initialization.              |
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "gd_compiler.h"

#if GD_COMPILER_HAS_CPP20_SUPPORT

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

/// Namespace for stack based vector, that stores elements inline up to a specified capacity, then dynamically allocates when needed
namespace stack {

// ============================================================================
// ## Base class - contains all size-independent logic to avoid to much code bloat
// ============================================================================

template<typename VALUE>
class vector_base
{
public:
   using value_type      = VALUE;                                             // Type of values stored in the container
   using size_type       = std::size_t;                                       // Type for size measurements
   using difference_type = std::ptrdiff_t;                                    // Type for differences between iterators
   using reference       = value_type&;                                       // Reference to stored value type
   using const_reference = const value_type&;                                 // Const reference to stored value type
   using pointer         = value_type*;                                       // Pointer to stored value type
   using const_pointer   = const value_type*;                                 // Const pointer to stored value type
   using iterator        = value_type*;                                       // Iterator for traversing container elements
   using const_iterator  = const value_type*;                                 // Const iterator for traversing container elements
   using reverse_iterator       = std::reverse_iterator<iterator>;            // Iterator for reverse traversal
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;      // Const iterator for reverse traversal

protected:
   vector_base() noexcept = default;
   ~vector_base() noexcept { destroy(); }
   
   // ## Non-copyable, non-movable (derived class handles this) .............

   vector_base(const vector_base&) = delete;
   vector_base& operator=(const vector_base&) = delete;
   vector_base(vector_base&&) = delete;
   vector_base& operator=(vector_base&&) = delete;
   
   /// ## Comparison operators (C++20)
   [[nodiscard]] bool operator==(const vector_base& o) const {
      return m_uSize == o.m_uSize && std::equal(begin(), end(), o.begin());
   }
   
   [[nodiscard]] auto operator<=>(const vector_base& o) const requires std::three_way_comparable<VALUE> {
      return std::lexicographical_compare_three_way(begin(), end(), o.begin(), o.end());
   }
   

   /// Initialize base with inline buffer
   void init_base(VALUE* pInlineBuffer, size_type uInlineCapacity) noexcept {                      assert( uInlineCapacity < 0x10'00'00 ); assert( uInlineCapacity > 0 ? (pInlineBuffer != nullptr) : true ); // realistic !
      m_pBuffer = pInlineBuffer; m_uInlineCapacity = uInlineCapacity; m_uCapacity = uInlineCapacity;
   }

   /// Check if using heap storage
   [[nodiscard]] bool is_external() const noexcept { return m_pHeap != nullptr; }

   /// Get pointer to current storage (inline or heap)
   [[nodiscard]] VALUE* get_buffer() noexcept { return m_pBuffer; }
   [[nodiscard]] const VALUE* get_buffer() const noexcept { return m_pBuffer; }

public:
   /// ## Element access
   [[nodiscard]] reference operator[](size_type uIndex) noexcept {                                 assert(uIndex < m_uSize);
      return m_pBuffer[uIndex];
   }

   [[nodiscard]] const_reference operator[](size_type uIndex) const noexcept { assert(uIndex < m_uSize); return m_pBuffer[uIndex]; }
   [[nodiscard]] reference at(size_type uIndex) { if( uIndex >= m_uSize ) { throw std::out_of_range("vector::at"); }
      return m_pBuffer[uIndex];
   }
   [[nodiscard]] const_reference at(size_type uIndex) const { if( uIndex >= m_uSize ) { throw std::out_of_range("vector::at"); }
      return m_pBuffer[uIndex];
   }

   [[nodiscard]] reference front() noexcept { assert(m_uSize > 0); return m_pBuffer[0]; }
   [[nodiscard]] const_reference front() const noexcept { assert(m_uSize > 0); return m_pBuffer[0]; }
   [[nodiscard]] reference back() noexcept { assert(m_uSize > 0); return m_pBuffer[m_uSize - 1]; }
   [[nodiscard]] const_reference back() const noexcept { assert(m_uSize > 0); return m_pBuffer[m_uSize - 1]; }
   [[nodiscard]] VALUE* data() noexcept { return m_pBuffer; }
   [[nodiscard]] const VALUE* data() const noexcept { return m_pBuffer; }

   /// ## Iterators
   [[nodiscard]] iterator begin() noexcept { return m_pBuffer; }
   [[nodiscard]] iterator end() noexcept { return m_pBuffer + m_uSize; }
   [[nodiscard]] const_iterator begin() const noexcept { return m_pBuffer; }
   [[nodiscard]] const_iterator end() const noexcept { return m_pBuffer + m_uSize; }
   [[nodiscard]] const_iterator cbegin() const noexcept { return m_pBuffer; }
   [[nodiscard]] const_iterator cend() const noexcept { return m_pBuffer + m_uSize; }

   [[nodiscard]] reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
   [[nodiscard]] reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
   [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
   [[nodiscard]] const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
   [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
   [[nodiscard]] const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

   /// ## Capacity
   [[nodiscard]] bool empty() const noexcept { return m_uSize == 0; }
   [[nodiscard]] size_type size() const noexcept { return m_uSize; }
   [[nodiscard]] size_type capacity() const noexcept { return m_uCapacity; }
   [[nodiscard]] size_type capacity_inline() const noexcept { return m_uInlineCapacity; }

   void reserve(size_type uNeededCapacity) {
      if( uNeededCapacity > m_uCapacity ) { allocate(uNeededCapacity); }
   }

   /// ## Modifiers
   void clear() noexcept
   {
      std::destroy_n(m_pBuffer, m_uSize);
      m_uSize = 0;
   }

   void push_back(const VALUE& value)
   {
      if( m_uSize >= m_uCapacity ) { allocate(m_uSize + 1); }
      std::construct_at(m_pBuffer + m_uSize, value);
      ++m_uSize;
   }

   void push_back(VALUE&& value)
   {
      if( m_uSize >= m_uCapacity ) { allocate(m_uSize + 1); }
      std::construct_at(m_pBuffer + m_uSize, std::move(value));
      ++m_uSize;
   }

   template<typename... Args>
   reference emplace_back(Args&&... args)
   {
      if( m_uSize >= m_uCapacity ) { allocate(m_uSize + 1); }
      std::construct_at(m_pBuffer + m_uSize, std::forward<Args>(args)...);
      return m_pBuffer[m_uSize++];
   }

   void pop_back() noexcept  {                                                                     assert(m_uSize > 0);
      --m_uSize;
      std::destroy_at(m_pBuffer + m_uSize);
   }

   void resize(size_type uNeededSize)
   {
      if( uNeededSize > m_uSize )
      {
         if( uNeededSize > m_uCapacity ) { allocate(uNeededSize); }
         std::uninitialized_value_construct_n(m_pBuffer + m_uSize, uNeededSize - m_uSize);
      }
      else if( uNeededSize < m_uSize )
      {
         std::destroy_n(m_pBuffer + uNeededSize, m_uSize - uNeededSize);
      }
      m_uSize = uNeededSize;
   }

   void resize(size_type uNeededSize, const VALUE& value)
   {
      if( uNeededSize > m_uSize )
      {
         if( uNeededSize > m_uCapacity ) { allocate(uNeededSize); }
         std::uninitialized_fill_n(m_pBuffer + m_uSize, uNeededSize - m_uSize, value);
      }
      else if( uNeededSize < m_uSize ) { std::destroy_n(m_pBuffer + uNeededSize, m_uSize - uNeededSize); }
      m_uSize = uNeededSize;
   }

protected:
   void allocate(size_type uNeededCapacity)
   {
      if( uNeededCapacity <= m_uCapacity ) { return; }
      
      size_type uAllocateCapacity = std::max(uNeededCapacity, m_uCapacity + m_uCapacity / 2); // increase by 1.5x
      
      VALUE* pvalueNew = std::allocator<VALUE>().allocate(uAllocateCapacity);    // allocate new storage

      // ### Move or copy elements to new storage ............................

      if constexpr( std::is_nothrow_move_constructible_v<VALUE> )            // Ask if type can be moved without throwing
      {
         std::uninitialized_move_n(m_pBuffer, m_uSize, pvalueNew);           // raw copy
      }
      else
      {
         try { std::uninitialized_copy_n(m_pBuffer, m_uSize, pvalueNew); }
         catch( ... ) { std::allocator<VALUE>().deallocate(pvalueNew, uAllocateCapacity); throw; }
      }

      // ## destroy old elements and free old storage
      std::destroy_n(m_pBuffer, m_uSize);
      if( is_external() == true ) { std::allocator<VALUE>().deallocate(m_pHeap, m_uCapacity); }

      m_pBuffer   = pvalueNew;
      m_pHeap     = pvalueNew;
      m_uCapacity = uAllocateCapacity;
   }

   void destroy() noexcept
   {
      std::destroy_n(m_pBuffer, m_uSize);
      if( is_external() )
      {
         std::allocator<VALUE>().deallocate(m_pHeap, m_uCapacity);
         m_pHeap = nullptr;
      }
      m_uSize = 0;
   }

protected:
   VALUE* m_pBuffer = nullptr;      // pointer to current storage (inline or heap)
   VALUE* m_pHeap = nullptr;        // heap storage pointer (null if using inline)
   size_type m_uSize = 0;           // current number of elements
   size_type m_uCapacity = 0;       // current capacity
   size_type m_uInlineCapacity = 0; // inline capacity (from derived class)
};


// ============================================================================
// ## Derived class - only handles storage and forwarding
// ============================================================================

/// ## Small vector class template
/// Hybrid container that stores elements inline up to uCapacityStack, then dynamically allocates
template<typename VALUE, std::size_t uCapacityStack = 1>
class vector : public vector_base<VALUE>
{
   using base = vector_base<VALUE>; // alias for base

public:
   /**
    * @name Type aliases
    * @brief Re-exposes the most common STL-compatible typedefs from `vector_base<VALUE>`.
    *
    * The derived `gd::stack::vector` inherits all core storage and element-management behavior
    * from `vector_base`. These `using` declarations "publish" the base type aliases in the
    * derived class so users can refer to them as `gd::stack::vector<...>::size_type`,
    * `::iterator`, etc., matching the conventions of `std::vector`.
    *
    * This also helps keep the derived template minimal: the derived class focuses on inline
    * storage/forwarding while the base class centralizes container logic.
    * @{
    */
   using typename base::value_type;              ///< Element type stored in the vector (`VALUE`).
   using typename base::size_type;               ///< Unsigned size type used for sizes/capacities (typically `std::size_t`).
   using typename base::difference_type;         ///< Signed difference type for iterator distances (typically `std::ptrdiff_t`).
   using typename base::reference;               ///< Mutable lvalue reference to an element (`value_type&`).
   using typename base::const_reference;         ///< Const lvalue reference to an element (`const value_type&`).
   using typename base::pointer;                 ///< Pointer to element (`value_type*`).
   using typename base::const_pointer;           ///< Pointer to const element (`const value_type*`).
   using typename base::iterator;                ///< Iterator over elements (implemented as `value_type*`).
   using typename base::const_iterator;          ///< Const iterator over elements (implemented as `const value_type*`).
   using typename base::reverse_iterator;        ///< Reverse iterator over elements.
   using typename base::const_reverse_iterator;  ///< Const reverse iterator over elements.
   /** @} */

public:
   /// ## Constructors and destructor
   vector() noexcept { this->init_base(get_inline_buffer(), uCapacityStack); }
   vector(const vector& o);
   vector(vector&& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>);
   vector(std::initializer_list<VALUE> list_);
   explicit vector(size_type uCount, const VALUE& value = VALUE());

   template<std::input_iterator iterator_> vector(iterator_ itFirst, iterator_ itLast);

   ~vector() noexcept = default;

   /// Copy assignment operator
   vector& operator=(const vector& o)
   {
      if( this != &o )
      {
         if( o.m_uSize <= this->m_uCapacity )                                 // if vector to copy is within capacity
         {
            size_type uMinSize = std::min(this->m_uSize, o.m_uSize);         // number of elements to assign with copy operator
            std::copy_n(o.m_pBuffer, uMinSize, this->m_pBuffer);             // assign existing elements instead of assigning where they overlap, this is an optimization an may save allocations
            
            if( o.m_uSize > this->m_uSize ) { std::uninitialized_copy_n(o.m_pBuffer + this->m_uSize, o.m_uSize - this->m_uSize, this->m_pBuffer + this->m_uSize); } // raw copy new elements
            else { std::destroy_n(this->m_pBuffer + o.m_uSize, this->m_uSize - o.m_uSize); }   // destroy excess elements in destination
            this->m_uSize = o.m_uSize;
         }
         else
         {
            vector temp_(o);                                                 // Need to allocate new storage into temporay vector
            swap(temp_);                                                     // Swap this temp_ with current into current
         }
      }
      return *this;
   }

   /// Move assignment operator
   vector& operator=(vector&& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>)
   {
      if( this != &o )
      {
         this->destroy();                                                     // clean up
         
         if( o.is_external() == true ) {
            this->m_pHeap = o.m_pHeap; this->m_pBuffer = o.m_pHeap; this->m_uCapacity = o.m_uCapacity;// move            
            o.m_pHeap = nullptr; o.m_pBuffer = o.get_inline_buffer(); o.m_uCapacity = uCapacityStack; // clear
         }
         else { this->m_pBuffer = get_inline_buffer(); std::uninitialized_move_n(o.m_pBuffer, o.m_uSize, this->m_pBuffer); } // copy
         
         this->m_uSize = o.m_uSize; o.m_uSize = 0;                            // transfer size
      }
      return *this;
   }

   /// Static capacity query
   [[nodiscard]] static constexpr size_type capacity_inline() noexcept { return uCapacityStack; }
   
   /// Swap contents with another vector
   void swap(vector& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>);

// ## attributes -------------------------------------------------------------
private:
   alignas(VALUE) std::byte m_buffer[sizeof(VALUE) * uCapacityStack]; // inline storage buffer @CRITICAL [tag: stack, memory] [description: inline storage buffer]

   [[nodiscard]] VALUE* get_inline_buffer() noexcept { return reinterpret_cast<VALUE*>(m_buffer); }
   [[nodiscard]] const VALUE* get_inline_buffer() const noexcept { return reinterpret_cast<const VALUE*>(m_buffer); }

};

/// ## Deduction guides
template<typename T, typename... U>
vector(T, U...) -> vector<T, 1 + sizeof...(U)>;

// ============================================================================
// ## Constructors and destructor defines outside class
// ============================================================================

/// Copy constructor
template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(const vector& o)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( o.m_uSize > uCapacityStack )
   {
      this->m_pHeap = std::allocator<VALUE>().allocate(o.m_uSize);
      this->m_pBuffer = this->m_pHeap;
      this->m_uCapacity = o.m_uSize;
   }

   std::uninitialized_copy_n( o.m_pBuffer, o.m_uSize, this->m_pBuffer );       // raw copy, uninitialized_copy_n to avoid default construction first
   this->m_uSize = o.m_uSize;
}

/// Move constructor
template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(vector&& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( o.is_external() == true )
   {
      // ### Take ownership of heap storage ..................................
      this->m_pHeap = o.m_pHeap;
      this->m_pBuffer = o.m_pHeap;
      this->m_uCapacity = o.m_uCapacity;
         
      o.m_pHeap = nullptr;
      o.m_pBuffer = o.get_inline_buffer();
      o.m_uCapacity = uCapacityStack;
   }
   else
   {
      std::uninitialized_move_n( o.m_pBuffer, o.m_uSize, this->m_pBuffer );    // raw copy, uninitialized_move_n to avoid default construction first
   }
      
   this->m_uSize = o.m_uSize;
   o.m_uSize = 0;
}

/// Constructor with initializer list
template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(std::initializer_list<VALUE> list_)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( list_.size() > uCapacityStack )                                       // need heap storage ?
   {
      this->m_pHeap = std::allocator<VALUE>().allocate(list_.size());
      this->m_pBuffer = this->m_pHeap;
      this->m_uCapacity = list_.size();
   }

   std::uninitialized_copy(list_.begin(), list_.end(), this->m_pBuffer);
   this->m_uSize = list_.size();
}

/// Constructor with count and value
template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(size_type uCount, const VALUE& value)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( uCount > uCapacityStack )                                             // need heap storage ?
   {
      this->m_pHeap = std::allocator<VALUE>().allocate(uCount);
      this->m_pBuffer = this->m_pHeap;
      this->m_uCapacity = uCount;
   }

   std::uninitialized_fill_n( this->m_pBuffer, uCount, value );               // fill with value, use uninitialized_fill_n to avoid default construction first
   this->m_uSize = uCount;
}

/// Constructor from iterator range
template<typename VALUE, std::size_t uCapacityStack>
template<std::input_iterator iterator_>
vector<VALUE,uCapacityStack>::vector(iterator_ itFirst, iterator_ itLast)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if constexpr( std::forward_iterator<iterator_> )
   {
      auto uDistance = static_cast<size_t>(std::distance(itFirst, itLast));
      if( uDistance > uCapacityStack )
      {
         this->m_pHeap = std::allocator<VALUE>().allocate(uDistance);
         this->m_pBuffer = this->m_pHeap;
         this->m_uCapacity = uDistance;
      }
      std::uninitialized_copy(itFirst, itLast, this->m_pBuffer);
      this->m_uSize = uDistance;
   }
   else
   {
      for( ; itFirst != itLast; ++itFirst ) { this->push_back(*itFirst); }
   }
}

// ============================================================================
// ## Methods defined outside class
// ============================================================================

/// Swap contents with another vector
template<typename VALUE, std::size_t uCapacityStack>
void vector<VALUE,uCapacityStack>::swap(vector& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>)
{
   if( this->is_external() && o.is_external() )
   {
      // ### Both using heap storage - just swap pointers
      std::swap(this->m_pHeap, o.m_pHeap);
      std::swap(this->m_pBuffer, o.m_pBuffer);
      std::swap(this->m_uCapacity, o.m_uCapacity);
      std::swap(this->m_uSize, o.m_uSize);
   }
   else if( this->is_external() == false && o.is_external() == false )        // no external storage
   {
      vector* pvectorSmaller = (this->m_uSize < o.m_uSize) ? this : &o;
      vector* pvectorLarger = (this->m_uSize < o.m_uSize) ? &o : this;
         
      size_type uMinSize = pvectorSmaller->m_uSize;
      for( size_type u = 0; u < uMinSize; ++u ) { std::swap((*pvectorSmaller)[u], (*pvectorLarger)[u]); }
         
      // move remaining elements from larger to smaller
      std::uninitialized_move_n(pvectorLarger->m_pBuffer + uMinSize, pvectorLarger->m_uSize - uMinSize, pvectorSmaller->m_pBuffer + uMinSize);
      std::destroy_n(pvectorLarger->m_pBuffer + uMinSize, pvectorLarger->m_uSize - uMinSize);
         
      std::swap(pvectorSmaller->m_uSize, pvectorLarger->m_uSize);
   }
   else
   {
      // ### One heap, one inline - need to swap carefully
      vector* pHeap = this->is_external() ? this : &o;
      vector* pInline = this->is_external() ? &o : this;
         
      VALUE* pHeapStorage = pHeap->m_pHeap;
      size_type uHeapCapacity = pHeap->m_uCapacity;
         
      // move inline elements to heap's inline storage
      std::uninitialized_move_n(pInline->m_pBuffer, pInline->m_uSize, pHeap->get_inline_buffer()); // raw copy
      std::destroy_n(pInline->m_pBuffer, pInline->m_uSize);
         
      // give heap storage to inline vector
      pInline->m_pHeap = pHeapStorage;
      pInline->m_pBuffer = pHeapStorage;
      pInline->m_uCapacity = uHeapCapacity;
         
      pHeap->m_pHeap = nullptr;
      pHeap->m_pBuffer = pHeap->get_inline_buffer();
      pHeap->m_uCapacity = uCapacityStack;
         
      std::swap(this->m_uSize, o.m_uSize);
   }
}


} // stack

_GD_END

/// ## std::swap specialization
template<typename VALUE, std::size_t uCapacity>
void swap(gd::stack::vector<VALUE, uCapacity>& lhs, gd::stack::vector<VALUE, uCapacity>& rhs)
   noexcept(noexcept(lhs.swap(rhs)))
{
   lhs.swap(rhs);
}

_GD_BEGIN

/** 
 * \brief Namespace for vector that use borrowed storage
 * 
 * Storage that is not owned by the vector until it needs to grow beyond the borrowed capacity, 
 * at which point it allocates its own storage and copies the elements. This is useful for cases 
 * where you want to avoid heap allocation for small vectors but also want to be able to use 
 * externally provided storage without copying until necessary.
 */
namespace borrow {


/** ==========================================================================
 * @brief A dynamic array that may not own its storage initially, but can grow and allocate its own storage if needed.
 * 
 * The vector have three members to manage its storage:
 * `m_pBuffer`: A pointer to the current storage buffer, which may point to either the borrowed storage or the owned heap storage.
 * `m_uSize`: The current number of elements in the vector.
 * `m_uCapacity`: The current capacity of the vector, indicating the maximum number of elements it can hold without reallocating.
 * 
 * What is special about this vector is that if the highest bit in `m_uCapacity` is set that means 
 * that the storage is borrowed and not owned by the vector. When the vector needs to grow beyond its 
 * current capacity, this bit is cleared and new heap storage is allocated, and the elements are copied over.
 * After that, the vector owns its storage and manages it like a regular dynamic array.
 * 
 * @par Example
 * @code{.cpp}
 * std::array<int, 5> buffer_;
 * gd::borrow::vector<int> vec( buffer_ );
 * vec.push_back( 1 ); // This will cause the vector store value in its borrowed storage
 * assert( vec.owner() == false ); // vector do not own its storage
 * vec.push_back({ 1,2,3,4,5 }); // This will fill up the borrowed storage and cause the vector to allocate its own storage
 * assert( vec.owner() == true ); // vector now owns its storage
 * @endcode
 * 
 * 
 * @tparam VALUE The type of elements stored in the vector.
 */
template<typename VALUE>
class vector
{
public:
   using value_type      = VALUE;                                             // type of values stored in the container
   using size_type       = std::size_t;                                       // type for size measurements
   using difference_type = std::ptrdiff_t;                                    // type for differences between iterators
   using reference       = value_type&;                                       // reference to stored value type
   using const_reference = const value_type&;                                 // const reference to stored value type
   using pointer         = value_type*;                                       // pointer to stored value type
   using const_pointer   = const value_type*;                                 // const pointer to stored value type
   using iterator        = value_type*;                                       // iterator for traversing container elements
   using const_iterator  = const value_type*;                                 // const iterator for traversing container elements
   using reverse_iterator       = std::reverse_iterator<iterator>;            // iterator for reverse traversal
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;      // const iterator for reverse traversal

   static constexpr size_type BORROW_BIT = size_type(1) << (sizeof(size_type) * 8 - 1); // bit flag indicating borrowed storage

   /// ## Constructors
   
   vector() noexcept;
   vector(VALUE* pBuffer, size_type uCapacity) noexcept;
   
   // ## Borrow from container with .data() and .size() - works with std::vector, std::array, C-arrays, etc.
   template<typename CONTAINER>
   requires requires(CONTAINER& c_) {
      { c_.data() } -> std::convertible_to<VALUE*>;
      { c_.size() } -> std::convertible_to<std::size_t>; 
   }
   && ( !std::is_same_v<std::remove_cvref_t<CONTAINER>, vector<VALUE>> )
   explicit vector(CONTAINER& container_) noexcept;
   
   template<size_type uN>
   explicit vector(VALUE (&array)[uN]) noexcept;                             // C-array specialization (no .data()/.size())
   
   vector(const vector& o);
   vector(vector&& o) noexcept;
   vector(std::initializer_list<VALUE> list_);
   vector(size_type uCount, const VALUE& value);
   
   template<std::input_iterator iterator_>
   vector(iterator_ itFirst, iterator_ itLast);

   ~vector() noexcept;

   /// ## @API [tag: assign] [summary: Assign operators] 

   vector& operator=(const vector& o);
   vector& operator=(vector&& o) noexcept;
   vector& operator=(std::initializer_list<VALUE> list_);
   
   /// ## @API [tag: operator] [summary: Comparison operators]

   [[nodiscard]] bool operator==(const vector& o) const;
   [[nodiscard]] auto operator<=>(const vector& o) const requires std::three_way_comparable<VALUE>;

   /// ## @API [tag: access] [summary: Element access]

   [[nodiscard]] reference operator[](size_type uIndex) noexcept;
   [[nodiscard]] const_reference operator[](size_type uIndex) const noexcept;
   [[nodiscard]] reference at(size_type uIndex);
   [[nodiscard]] const_reference at(size_type uIndex) const;
   [[nodiscard]] reference front() noexcept;
   [[nodiscard]] const_reference front() const noexcept;
   [[nodiscard]] reference back() noexcept;
   [[nodiscard]] const_reference back() const noexcept;
   [[nodiscard]] VALUE* data() noexcept { return m_pBuffer; }
   [[nodiscard]] const VALUE* data() const noexcept { return m_pBuffer; }

   /// ## @API [tag: iterator] [summary: Iterators] 

   [[nodiscard]] iterator begin() noexcept { return m_pBuffer; }
   [[nodiscard]] iterator end() noexcept { return m_pBuffer + m_uSize; }
   [[nodiscard]] const_iterator begin() const noexcept { return m_pBuffer; }
   [[nodiscard]] const_iterator end() const noexcept { return m_pBuffer + m_uSize; }
   [[nodiscard]] const_iterator cbegin() const noexcept { return m_pBuffer; }
   [[nodiscard]] const_iterator cend() const noexcept { return m_pBuffer + m_uSize; }

   [[nodiscard]] reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
   [[nodiscard]] reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
   [[nodiscard]] const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
   [[nodiscard]] const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
   [[nodiscard]] const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(end()); }
   [[nodiscard]] const_reverse_iterator crend() const noexcept { return const_reverse_iterator(begin()); }

   /// ## @API [tag: capacity] [summary: Capacity]  

   [[nodiscard]] bool empty() const noexcept { return m_uSize == 0; }
   [[nodiscard]] size_type size() const noexcept { return m_uSize; }
   [[nodiscard]] size_type max_size() const noexcept { return std::numeric_limits<size_type>::max() / sizeof(VALUE); }
   [[nodiscard]] size_type capacity() const noexcept { return m_uCapacity & ~BORROW_BIT; }
   [[nodiscard]] bool owner() const noexcept { return (m_uCapacity & BORROW_BIT) == 0 && m_pBuffer != nullptr; }
   [[nodiscard]] bool is_borrowed() const noexcept { return (m_uCapacity & BORROW_BIT) != 0; }

   void reserve(size_type uNewCapacity);
   void shrink_to_fit();

   /// ## @API [tag: modify] [summary: Modifiers] 

   void assign(std::initializer_list<VALUE> list_);
   void clear() noexcept;
   void push_back(const VALUE& value);
   void push_back(VALUE&& value);
   template<typename... ARGUMENTS>
      requires (sizeof...(ARGUMENTS) == 1) || (!(std::same_as<std::remove_cvref_t<ARGUMENTS>, VALUE> && ...))
   reference emplace_back(ARGUMENTS&&... arguments);

   template<typename... ARGUMENTS>
      requires (sizeof...(ARGUMENTS) > 1) && (std::same_as<std::remove_cvref_t<ARGUMENTS>, VALUE> && ...)
   reference emplace_back(ARGUMENTS&&... arguments);
   
   void pop_back() noexcept;
   void resize(size_type uNewSize);
   void resize(size_type uNewSize, const VALUE& value);
   
   iterator insert(const_iterator itPosition, const VALUE& value);
   iterator insert(const_iterator itPosition, VALUE&& value);
   template<class... Args>
   iterator emplace(const_iterator position, Args&&... args);
   iterator erase(const_iterator itPosition);
   iterator erase(const_iterator itFirst, const_iterator itLast);
   
   void swap(vector& o) noexcept;

private:
   void copy_from(const vector& o);
   void allocate(size_type uMinCapacity);
   void destroy() noexcept;

private:
   VALUE* m_pBuffer;       ///< pointer to current storage (borrowed or owned)
   size_type m_uSize;      ///< current number of elements
   size_type m_uCapacity;  ///< capacity with high bit indicating borrowed storage
};

// ============================================================================
// ## Out-of-line constructor definitions
// ============================================================================

/** -------------------------------------------------------------------------- Default constructor
 * @brief Creates an empty vector with no storage
 */
template<typename VALUE>
vector<VALUE>::vector() noexcept 
   : m_pBuffer(nullptr), m_uSize(0), m_uCapacity(0) 
{
}

/** -------------------------------------------------------------------------- Borrow buffer constructor
 * @brief Creates a vector that borrows external storage
 * 
 * @param pBuffer Pointer to external storage buffer
 * @param uCapacity Number of elements the external buffer can hold
 */
template<typename VALUE>
vector<VALUE>::vector(VALUE* pBuffer, size_type uCapacity) noexcept 
   : m_pBuffer(pBuffer), m_uSize(0), m_uCapacity(uCapacity | BORROW_BIT) 
{                                                                                                  assert(pBuffer != nullptr || uCapacity == 0); // verify buffer is valid or capacity is zero
}

/** -------------------------------------------------------------------------- Borrow from C-array constructor
 * @brief Creates a vector that borrows storage from a C-array
 * 
 * @tparam uN Size of the array
 * @param array Reference to array to borrow storage from
 */
template<typename VALUE>
template<std::size_t uSize>
vector<VALUE>::vector(VALUE (&array_)[uSize]) noexcept 
   : m_pBuffer( array_ ), m_uSize( 0 ), m_uCapacity( uSize | BORROW_BIT )     // note the BORROW_BIT is set in capacity to indicate borrowed storage
{
}

/** -------------------------------------------------------------------------- Borrow from container constructor
 * @brief Creates a vector that borrows storage from a container with .data() and .size()
 * 
 * Works with std::vector, std::array, and other containers providing .data() and .size() methods.
 * 
 * @tparam CONTAINER Container type with .data() and .size() methods
 * @param container_ Reference to container to borrow storage from
 */
template<typename VALUE>
template<typename CONTAINER>
requires requires(CONTAINER& c_) {
   { c_.data() } -> std::convertible_to<VALUE*>;                              // verify container has .data() returning pointer to VALUE
   { c_.size() } -> std::convertible_to<std::size_t>;                         // verify container has .size() returning size type
}
&& ( !std::is_same_v<std::remove_cvref_t<CONTAINER>, vector<VALUE>> )
vector<VALUE>::vector(CONTAINER& container_) noexcept 
   : m_pBuffer( container_.data() ), m_uSize( 0 ), m_uCapacity( container_.size() | BORROW_BIT ) // note the BORROW_BIT is set in capacity to indicate borrowed storage
{
}

/** -------------------------------------------------------------------------- Copy constructor
 * @brief Creates a copy of another vector
 * 
 * @param o Vector to copy from
 */
template<typename VALUE>
vector<VALUE>::vector(const vector& o) 
   : m_pBuffer(nullptr), m_uSize(0), m_uCapacity(0) 
{
   copy_from(o);
}

/** -------------------------------------------------------------------------- Move constructor
 * @brief Moves another vector's contents
 * 
 * @param o Vector to move from
 */
template<typename VALUE>
vector<VALUE>::vector(vector&& o) noexcept 
   : m_pBuffer(o.m_pBuffer), m_uSize(o.m_uSize), m_uCapacity(o.m_uCapacity) 
{
   o.m_pBuffer = nullptr;
   o.m_uSize = 0;
   o.m_uCapacity = 0;
}

/** -------------------------------------------------------------------------- Initializer list constructor
 * @brief Creates a vector from an initializer list
 * 
 * @param list_ Initializer list of values
 */
template<typename VALUE>
vector<VALUE>::vector(std::initializer_list<VALUE> list_) 
   : vector() 
{
   reserve(list_.size());
   std::uninitialized_copy(list_.begin(), list_.end(), m_pBuffer);
   m_uSize = list_.size();
}

/** -------------------------------------------------------------------------- Fill constructor
 * @brief Creates a vector with uCount copies of value
 * 
 * @param uCount Number of elements to create
 * @param value Value to copy
 */
template<typename VALUE>
vector<VALUE>::vector(size_type uCount, const VALUE& value) 
   : vector() 
{
   reserve(uCount);
   std::uninitialized_fill_n(m_pBuffer, uCount, value);
   m_uSize = uCount;
}

/** -------------------------------------------------------------------------- Range constructor
 * @brief Creates a vector from an iterator range
 * 
 * @tparam iterator_ Iterator type
 * @param itFirst Beginning of range
 * @param itLast End of range
 */
template<typename VALUE>
template<std::input_iterator iterator_>
vector<VALUE>::vector(iterator_ itFirst, iterator_ itLast) 
   : vector() 
{
   if constexpr( std::forward_iterator<iterator_> )
   {
      auto uDistance = static_cast<size_t>(std::distance(itFirst, itLast));
      reserve(uDistance);
      std::uninitialized_copy(itFirst, itLast, m_pBuffer);
      m_uSize = uDistance;
   }
   else
   {
      for( ; itFirst != itLast; ++itFirst ) { push_back(*itFirst); }
   }
}

/** -------------------------------------------------------------------------- Destructor
 * @brief Destroys all elements and deallocates owned storage
 */
template<typename VALUE>
vector<VALUE>::~vector() noexcept 
{
   destroy();
}

// ============================================================================
// ## Assignment operators
// ============================================================================

/** -------------------------------------------------------------------------- Copy assignment
 * @brief Assigns a copy of another vector
 * 
 * @param o Vector to copy from
 * @return Reference to this vector
 */
template<typename VALUE>
vector<VALUE>& vector<VALUE>::operator=(const vector& o) 
{
   if( this != &o )
   {
      clear();
      copy_from(o);
   }
   return *this;
}

/** -------------------------------------------------------------------------- Move assignment
 * @brief Moves another vector's contents
 * 
 * @param o Vector to move from
 * @return Reference to this vector
 */
template<typename VALUE>
vector<VALUE>& vector<VALUE>::operator=(vector&& o) noexcept 
{
   if( this != &o )
   {
      destroy();
      m_pBuffer = o.m_pBuffer; m_uSize = o.m_uSize;   m_uCapacity = o.m_uCapacity; // take ownership of storage
      o.m_pBuffer = nullptr;   o.m_uSize = 0;         o.m_uCapacity = 0;      // clear source vector
   }
   return *this;
}

/** -------------------------------------------------------------------------- Initializer list assignment
 * @brief Assigns from an initializer list
 * 
 * @param list_ Initializer list of values
 * @return Reference to this vector
 */
template<typename VALUE>
vector<VALUE>& vector<VALUE>::operator=(std::initializer_list<VALUE> list_) 
{
   clear();
   reserve(list_.size());
   std::uninitialized_copy(list_.begin(), list_.end(), m_pBuffer);
   m_uSize = list_.size();
   return *this;
}

/** -------------------------------------------------------------------------- assign
 * @brief Assigns elements from an initializer list
 * 
 * @param list_ Initializer list of values to assign
 */
template<typename VALUE>
void vector<VALUE>::assign(std::initializer_list<VALUE> list_) 
{
   clear();
   reserve(list_.size());
   std::uninitialized_copy(list_.begin(), list_.end(), m_pBuffer);
   m_uSize = list_.size();
}

// ============================================================================
// ## Element access methods
// ============================================================================

/** -------------------------------------------------------------------------- operator[]
 * @brief Access element at index (no bounds checking)
 * 
 * @param uIndex Index of element
 * @return Reference to element
 */
template<typename VALUE>
typename vector<VALUE>::reference vector<VALUE>::operator[](size_type uIndex) noexcept 
{                                                                                                  assert(uIndex < m_uSize); // verify index is within bounds
   return m_pBuffer[uIndex];
}

template<typename VALUE>
typename vector<VALUE>::const_reference vector<VALUE>::operator[](size_type uIndex) const noexcept 
{                                                                                                  assert(uIndex < m_uSize); // verify index is within bounds
   return m_pBuffer[uIndex];
}

/** -------------------------------------------------------------------------- at
 * @brief Access element at index with bounds checking
 * 
 * @param uIndex Index of element
 * @return Reference to element
 * @throws std::out_of_range if index is out of bounds
 */
template<typename VALUE>
typename vector<VALUE>::reference vector<VALUE>::at(size_type uIndex) 
{
   if( uIndex >= m_uSize ) { throw std::out_of_range("borrow::vector::at"); }
   return m_pBuffer[uIndex];
}

template<typename VALUE>
typename vector<VALUE>::const_reference vector<VALUE>::at(size_type uIndex) const 
{
   if( uIndex >= m_uSize ) { throw std::out_of_range("borrow::vector::at"); }
   return m_pBuffer[uIndex];
}

/** -------------------------------------------------------------------------- front
 * @brief Access first element
 * 
 * @return Reference to first element
 */
template<typename VALUE>
typename vector<VALUE>::reference vector<VALUE>::front() noexcept 
{                                                                                                  assert(m_uSize > 0); // verify vector is not empty
   return m_pBuffer[0];
}

template<typename VALUE>
typename vector<VALUE>::const_reference vector<VALUE>::front() const noexcept 
{                                                                                                  assert(m_uSize > 0); // verify vector is not empty
   return m_pBuffer[0];
}

/** -------------------------------------------------------------------------- back
 * @brief Access last element
 * 
 * @return Reference to last element
 */
template<typename VALUE>
typename vector<VALUE>::reference vector<VALUE>::back() noexcept 
{                                                                                                  assert(m_uSize > 0); // verify vector is not empty
   return m_pBuffer[m_uSize - 1];
}

template<typename VALUE>
typename vector<VALUE>::const_reference vector<VALUE>::back() const noexcept 
{                                                                                                  assert(m_uSize > 0); // verify vector is not empty
   return m_pBuffer[m_uSize - 1];
}

// ============================================================================
// ## Capacity methods
// ============================================================================

/** -------------------------------------------------------------------------- reserve
 * @brief Reserve storage for at least uNewCapacity elements
 * 
 * If the requested capacity is greater than current capacity, allocates new storage
 * and takes ownership. If storage was borrowed, elements are copied to owned storage.
 * 
 * @param uNewCapacity Minimum capacity required
 */
template<typename VALUE>
void vector<VALUE>::reserve(size_type uNewCapacity) 
{
   if( uNewCapacity > capacity() )
   {
      allocate(uNewCapacity);
   }
}

/** -------------------------------------------------------------------------- shrink_to_fit
 * @brief Reduce capacity to match size
 * 
 * If the vector owns its storage and size() < capacity(), reallocates to reduce
 * memory usage. Does nothing if the vector is borrowed or if size() == capacity().
 */
template<typename VALUE>
void vector<VALUE>::shrink_to_fit() 
{
   if( is_borrowed() ) { return; }                                             // Do nothing if storage is borrowed - we don't own it
   
   if( m_uSize == capacity() ) { return; }                                     // Do nothing if size already equals capacity
   
   if( m_uSize == 0 ) {  destroy(); return; }                                  // If empty, destroy and deallocate 
   
   // ## Reallocate to exactly match size .....................................
   size_type uOldCapacity = capacity();
   VALUE* pNewBuffer = std::allocator<VALUE>().allocate(m_uSize);
   
   if constexpr( std::is_nothrow_move_constructible_v<VALUE> )
   {
      std::uninitialized_move_n(m_pBuffer, m_uSize, pNewBuffer);
   }
   else { std::uninitialized_copy_n(m_pBuffer, m_uSize, pNewBuffer); }
   
   std::destroy_n(m_pBuffer, m_uSize);
   std::allocator<VALUE>().deallocate(m_pBuffer, uOldCapacity);
   
   m_pBuffer = pNewBuffer;
   m_uCapacity = m_uSize;
}

// ============================================================================
// ## Modifier methods
// ============================================================================

/** -------------------------------------------------------------------------- clear
 * @brief Remove all elements
 * 
 * Destroys all elements but does not deallocate storage
 */
template<typename VALUE>
void vector<VALUE>::clear() noexcept 
{
   std::destroy_n(m_pBuffer, m_uSize);
   m_uSize = 0;
}

/** -------------------------------------------------------------------------- push_back
 * @brief Add element to end of vector
 * 
 * @param value Value to add
 */
template<typename VALUE>
void vector<VALUE>::push_back(const VALUE& value) 
{
   if( m_uSize >= capacity() ) { allocate(m_uSize + 1); }
   std::construct_at(m_pBuffer + m_uSize, value);
   ++m_uSize;
}

template<typename VALUE>
void vector<VALUE>::push_back(VALUE&& value) 
{
   if( m_uSize >= capacity() ) { allocate(m_uSize + 1); }
   std::construct_at(m_pBuffer + m_uSize, std::move(value));
   ++m_uSize;
}

/** -------------------------------------------------------------------------- emplace_back
 * @brief Construct element in-place at end of vector
 * 
 * @tparam ARGUMENTS Types of constructor arguments
 * @param arguments Arguments to forward to element constructor
 * @return Reference to newly constructed element
 */
template<typename VALUE>
template<typename... ARGUMENTS>
   requires (sizeof...(ARGUMENTS) == 1) || (!(std::same_as<std::remove_cvref_t<ARGUMENTS>, VALUE> && ...))
typename vector<VALUE>::reference vector<VALUE>::emplace_back(ARGUMENTS&&... arguments) {
   if( m_uSize >= capacity() ) { allocate(m_uSize + 1); }
   std::construct_at(m_pBuffer + m_uSize, std::forward<ARGUMENTS>(arguments)...);
   return m_pBuffer[m_uSize++];
}

template<typename VALUE>
template<typename... ARGUMENTS>
   requires (sizeof...(ARGUMENTS) > 1) && (std::same_as<std::remove_cvref_t<ARGUMENTS>, VALUE> && ...)
typename vector<VALUE>::reference vector<VALUE>::emplace_back(ARGUMENTS&&... arguments) {
   reserve(m_uSize + sizeof...(ARGUMENTS));
   ([&]<typename T>(T&& arg) { 
      std::construct_at(m_pBuffer + m_uSize++, std::forward<T>(arg)); 
   }(std::forward<ARGUMENTS>(arguments)), ...);
   return back();
}

/** -------------------------------------------------------------------------- pop_back
 * @brief Remove last element
 */
template<typename VALUE>
void vector<VALUE>::pop_back() noexcept 
{
   assert(m_uSize > 0);                                                       // verify that there are elements to pop
   --m_uSize;
   std::destroy_at(m_pBuffer + m_uSize);
}

/** -------------------------------------------------------------------------- resize
 * @brief Change number of elements
 * 
 * @param uNewSize New size of vector
 */
template<typename VALUE>
void vector<VALUE>::resize(size_type uNewSize) 
{
   if( uNewSize > capacity() ) { allocate(uNewSize); }
   
   if( uNewSize > m_uSize )
   {
      std::uninitialized_value_construct_n(m_pBuffer + m_uSize, uNewSize - m_uSize);
   }
   else if( uNewSize < m_uSize )
   {
      std::destroy_n(m_pBuffer + uNewSize, m_uSize - uNewSize);
   }
   m_uSize = uNewSize;
}

template<typename VALUE>
void vector<VALUE>::resize(size_type uNewSize, const VALUE& value) 
{
   if( uNewSize > capacity() ) { allocate(uNewSize); }
   
   if( uNewSize > m_uSize )
   {
      std::uninitialized_fill_n(m_pBuffer + m_uSize, uNewSize - m_uSize, value);
   }
   else if( uNewSize < m_uSize )
   {
      std::destroy_n(m_pBuffer + uNewSize, m_uSize - uNewSize);
   }
   m_uSize = uNewSize;
}

/** -------------------------------------------------------------------------- insert
 * @brief Insert element at position
 * 
 * @param itPosition Iterator to position where element will be inserted
 * @param value Value to insert
 * @return Iterator to inserted element
 */
template<typename VALUE>
typename vector<VALUE>::iterator vector<VALUE>::insert(const_iterator itPosition, const VALUE& value) 
{
   size_type uIndex = itPosition - begin();
   if( m_uSize >= capacity() ) { allocate(m_uSize + 1); }
   
   //
   // ### Shift elements to make room
   if( uIndex < m_uSize )
   {
      std::construct_at(m_pBuffer + m_uSize, std::move(m_pBuffer[m_uSize - 1]));
      std::move_backward(m_pBuffer + uIndex, m_pBuffer + m_uSize - 1, m_pBuffer + m_uSize);
      m_pBuffer[uIndex] = value;
   }
   else
   {
      std::construct_at(m_pBuffer + m_uSize, value);
   }
   ++m_uSize;
   return m_pBuffer + uIndex;
}

/** -------------------------------------------------------------------------- emplace
 * @brief Construct element in-place at position
 * 
 * @tparam Args Types of constructor arguments
 * @param position Iterator to position where element will be constructed
 * @param args Arguments to forward to element constructor
 * @return Iterator to newly constructed element
 */
template<typename VALUE>
template<class... Args>
typename vector<VALUE>::iterator vector<VALUE>::emplace(const_iterator position, Args&&... args) 
{
   size_type uIndex = position - begin();
   if( m_uSize >= capacity() ) { allocate(m_uSize + 1); }
   
   // ### Shift elements to make room ........................................

   if( uIndex < m_uSize )
   {
      std::construct_at(m_pBuffer + m_uSize, std::move(m_pBuffer[m_uSize - 1])); // Construct new element at end
      std::move_backward(m_pBuffer + uIndex, m_pBuffer + m_uSize - 1, m_pBuffer + m_uSize); // Shift elements to make room
      std::destroy_at(m_pBuffer + uIndex);                                    // Destroy old element at index
      std::construct_at(m_pBuffer + uIndex, std::forward<Args>(args)...);     // Construct new element at index   
   }
   else
   {
      std::construct_at(m_pBuffer + m_uSize, std::forward<Args>(args)...);
   }
   ++m_uSize;
   return m_pBuffer + uIndex;
}

template<typename VALUE>
typename vector<VALUE>::iterator vector<VALUE>::insert(const_iterator itPosition, VALUE&& value) 
{
   size_type uIndex = itPosition - begin();
   if( m_uSize >= capacity() ) { allocate(m_uSize + 1); }
   
   //
   // ### Shift elements to make room
   if( uIndex < m_uSize )
   {
      std::construct_at(m_pBuffer + m_uSize, std::move(m_pBuffer[m_uSize - 1]));
      std::move_backward(m_pBuffer + uIndex, m_pBuffer + m_uSize - 1, m_pBuffer + m_uSize);
      m_pBuffer[uIndex] = std::move(value);
   }
   else
   {
      std::construct_at(m_pBuffer + m_uSize, std::move(value));
   }
   ++m_uSize;
   return m_pBuffer + uIndex;
}

/** -------------------------------------------------------------------------- erase
 * @brief Remove element at position
 * 
 * @param itPosition Iterator to element to remove
 * @return Iterator to element following the removed element
 */
template<typename VALUE>
typename vector<VALUE>::iterator vector<VALUE>::erase(const_iterator itPosition) 
{
   size_type uIndex = itPosition - begin();
   assert(uIndex < m_uSize);                                                  // verify valid position
   
   std::move(m_pBuffer + uIndex + 1, m_pBuffer + m_uSize, m_pBuffer + uIndex);
   --m_uSize;
   std::destroy_at(m_pBuffer + m_uSize);
   return m_pBuffer + uIndex;
}

template<typename VALUE>
typename vector<VALUE>::iterator vector<VALUE>::erase(const_iterator itFirst, const_iterator itLast) 
{
   size_type uFirstIndex = itFirst - begin();
   size_type uLastIndex = itLast - begin();
   size_type uCount = uLastIndex - uFirstIndex;
   
   assert(uFirstIndex <= uLastIndex && uLastIndex <= m_uSize);                // verify valid range
   
   std::move(m_pBuffer + uLastIndex, m_pBuffer + m_uSize, m_pBuffer + uFirstIndex);
   std::destroy_n(m_pBuffer + m_uSize - uCount, uCount);
   m_uSize -= uCount;
   return m_pBuffer + uFirstIndex;
}

/** -------------------------------------------------------------------------- swap
 * @brief Swap contents with another vector
 * 
 * @param o Vector to swap with
 */
template<typename VALUE>
void vector<VALUE>::swap(vector& o) noexcept 
{
   std::swap(m_pBuffer, o.m_pBuffer);
   std::swap(m_uSize, o.m_uSize);
   std::swap(m_uCapacity, o.m_uCapacity);
}

// ============================================================================
// ## Comparison operators
// ============================================================================

template<typename VALUE>
bool vector<VALUE>::operator==(const vector& o) const 
{
   return m_uSize == o.m_uSize && std::equal(begin(), end(), o.begin());
}

template<typename VALUE>
auto vector<VALUE>::operator<=>(const vector& o) const requires std::three_way_comparable<VALUE> 
{
   return std::lexicographical_compare_three_way(begin(), end(), o.begin(), o.end());
}

// ============================================================================
// ## Private helper methods
// ============================================================================

/** -------------------------------------------------------------------------- copy_from
 * @brief Internal helper to copy from another vector
 * 
 * @param o Vector to copy from
 */
template<typename VALUE>
void vector<VALUE>::copy_from(const vector& o) 
{
   reserve(o.m_uSize);
   std::uninitialized_copy_n(o.m_pBuffer, o.m_uSize, m_pBuffer);
   m_uSize = o.m_uSize;
}

/** -------------------------------------------------------------------------- allocate
 * @brief Allocate new owned storage and transfer ownership
 * 
 * This method allocates new heap storage, copies existing elements to it,
 * and takes ownership. If storage was borrowed, the borrow bit is cleared.
 * 
 * @param uMinCapacity Minimum capacity required
 */
template<typename VALUE>
void vector<VALUE>::allocate(size_type uMinCapacity) 
{
   size_type uOldCapacity = capacity();
   size_type uNewCapacity = std::max(uOldCapacity * 2, uMinCapacity);         // standard growth factor of 2
   
   VALUE* pNewBuffer = std::allocator<VALUE>().allocate(uNewCapacity);
   
   // ## Copy or move existing elements ......................................
   if constexpr( std::is_nothrow_move_constructible_v<VALUE> )
   {
      std::uninitialized_move_n(m_pBuffer, m_uSize, pNewBuffer);
   }
   else { std::uninitialized_copy_n(m_pBuffer, m_uSize, pNewBuffer); }
   
   std::destroy_n(m_pBuffer, m_uSize);
   
   // ## Deallocate old buffer only if we owned it ...........................
   if( owner() == true ) { std::allocator<VALUE>().deallocate(m_pBuffer, uOldCapacity); }
   
   m_pBuffer = pNewBuffer;
   m_uCapacity = uNewCapacity;                                                // clear borrow bit by setting without it
}

/** -------------------------------------------------------------------------- destroy
 * @brief Destroy all elements and deallocate owned storage
 */
template<typename VALUE>
void vector<VALUE>::destroy() noexcept 
{
   std::destroy_n(m_pBuffer, m_uSize);
   if( owner() )
   {
      std::allocator<VALUE>().deallocate(m_pBuffer, capacity());
   }
   m_pBuffer = nullptr;
   m_uSize = 0;
   m_uCapacity = 0;
}

} // borrow

_GD_END

/// ## std::swap specialization for borrow::vector
template<typename VALUE>
void swap(gd::borrow::vector<VALUE>& lhs, gd::borrow::vector<VALUE>& rhs) noexcept {
   lhs.swap(rhs);
}
#endif // GD_COMPILER_HAS_CPP20_SUPPORT
