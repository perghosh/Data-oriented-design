// @FILE [tag: vector] [description: Vector variants with special storage features]] [type: header] [name: gd_vector.h]


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

namespace stack {

// ============================================================================
// ## Base class - contains all size-independent logic
// ============================================================================

template<typename VALUE>
class vector_base
{
public:
   using value_type = VALUE;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   using reference = value_type&;
   using const_reference = const value_type&;
   using pointer = value_type*;
   using const_pointer = const value_type*;
   using iterator = value_type*;
   using const_iterator = const value_type*;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

protected:
   vector_base() noexcept = default;
   ~vector_base() noexcept { destroy(); }

   // Non-copyable, non-movable (derived class handles this)
   vector_base(const vector_base&) = delete;
   vector_base& operator=(const vector_base&) = delete;
   vector_base(vector_base&&) = delete;
   vector_base& operator=(vector_base&&) = delete;

   /// Initialize base with inline buffer
   void init_base(VALUE* pInlineBuffer, size_type uInlineCapacity) noexcept
   {
      m_pBuffer = pInlineBuffer;
      m_uInlineCapacity = uInlineCapacity;
      m_uCapacity = uInlineCapacity;
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
   [[nodiscard]] size_type inline_capacity() const noexcept { return m_uInlineCapacity; }

   void reserve(size_type uNewCapacity)
   {
      if( uNewCapacity > m_uCapacity ) { allocate(uNewCapacity); }
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

   void resize(size_type uNewSize)
   {
      if( uNewSize > m_uSize )
      {
         if( uNewSize > m_uCapacity ) { allocate(uNewSize); }
         std::uninitialized_value_construct_n(m_pBuffer + m_uSize, uNewSize - m_uSize);
      }
      else if( uNewSize < m_uSize )
      {
         std::destroy_n(m_pBuffer + uNewSize, m_uSize - uNewSize);
      }
      m_uSize = uNewSize;
   }

   void resize(size_type uNewSize, const VALUE& value)
   {
      if( uNewSize > m_uSize )
      {
         if( uNewSize > m_uCapacity ) { allocate(uNewSize); }
         std::uninitialized_fill_n(m_pBuffer + m_uSize, uNewSize - m_uSize, value);
      }
      else if( uNewSize < m_uSize )
      {
         std::destroy_n(m_pBuffer + uNewSize, m_uSize - uNewSize);
      }
      m_uSize = uNewSize;
   }

   /// ## Comparison operators (C++20)
   [[nodiscard]] bool operator==(const vector_base& o) const {
      return m_uSize == o.m_uSize && std::equal(begin(), end(), o.begin());
   }

   [[nodiscard]] auto operator<=>(const vector_base& o) const requires std::three_way_comparable<VALUE> {
      return std::lexicographical_compare_three_way(begin(), end(), o.begin(), o.end());
   }

protected:
   void allocate(size_type uNeededCapacity)
   {
      if( uNeededCapacity <= m_uCapacity ) { return; }
      
      size_type uGrowCapacity = std::max(uNeededCapacity, m_uCapacity + m_uCapacity / 2);
      
      // allocate new storage
      VALUE* pNew = std::allocator<VALUE>().allocate(uGrowCapacity);

      // ### Move or copy elements to new storage
      if constexpr( std::is_nothrow_move_constructible_v<VALUE> )
      {
         std::uninitialized_move_n(m_pBuffer, m_uSize, pNew);
      }
      else
      {
         try
         {
            std::uninitialized_copy_n(m_pBuffer, m_uSize, pNew);
         }
         catch( ... )
         {
            std::allocator<VALUE>().deallocate(pNew, uGrowCapacity);
            throw;
         }
      }

      // ## destroy old elements and free old storage
      std::destroy_n(m_pBuffer, m_uSize);
      if( is_external() ) { std::allocator<VALUE>().deallocate(m_pHeap, m_uCapacity); }

      // ## update to new storage
      m_pBuffer = pNew;
      m_pHeap = pNew;
      m_uCapacity = uGrowCapacity;
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
   using base = vector_base<VALUE>;

public:
   using typename base::value_type;
   using typename base::size_type;
   using typename base::difference_type;
   using typename base::reference;
   using typename base::const_reference;
   using typename base::pointer;
   using typename base::const_pointer;
   using typename base::iterator;
   using typename base::const_iterator;
   using typename base::reverse_iterator;
   using typename base::const_reverse_iterator;

public:
   /// ## Constructors and destructor
   vector() noexcept { this->init_base(get_inline_buffer(), uCapacityStack); }
   vector(const vector& o);
   vector(vector&& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>);
   vector(std::initializer_list<VALUE> list_);
   explicit vector(size_type uCount, const VALUE& value = VALUE());

   template<std::input_iterator iterator_> vector(iterator_ itFirst, iterator_ itLast);

   ~vector() noexcept = default;

   /// ## Copy assignment operator
   vector& operator=(const vector& o)
   {
      if( this != &o )
      {
         if( o.m_uSize <= this->m_uCapacity )
         {
            // ### Reuse existing storage
            size_type uMinSize = std::min(this->m_uSize, o.m_uSize);
            std::copy_n(o.m_pBuffer, uMinSize, this->m_pBuffer);
            
            if( o.m_uSize > this->m_uSize )
            {
               std::uninitialized_copy_n(o.m_pBuffer + this->m_uSize, o.m_uSize - this->m_uSize, this->m_pBuffer + this->m_uSize);
            }
            else
            {
               std::destroy_n(this->m_pBuffer + o.m_uSize, this->m_uSize - o.m_uSize);
            }
            this->m_uSize = o.m_uSize;
         }
         else
         {
            // ### Need to allocate new storage
            vector temp(o);
            swap(temp);
         }
      }
      return *this;
   }

   /// ## Move assignment operator
   vector& operator=(vector&& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>)
   {
      if( this != &o )
      {
         this->destroy();
         
         if( o.is_external() )
         {
            this->m_pHeap = o.m_pHeap;
            this->m_pBuffer = o.m_pHeap;
            this->m_uCapacity = o.m_uCapacity;
            
            o.m_pHeap = nullptr;
            o.m_pBuffer = o.get_inline_buffer();
            o.m_uCapacity = uCapacityStack;
         }
         else
         {
            this->m_pBuffer = get_inline_buffer();
            std::uninitialized_move_n(o.m_pBuffer, o.m_uSize, this->m_pBuffer);
         }
         
         this->m_uSize = o.m_uSize;
         o.m_uSize = 0;
      }
      return *this;
   }

   /// ## Static capacity query
   [[nodiscard]] static constexpr size_type inline_capacity() noexcept { return uCapacityStack; }
   
   /// Swap contents with another vector
   void swap(vector& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>);

private:
   alignas(VALUE) std::byte m_buffer[sizeof(VALUE) * uCapacityStack]; // inline storage buffer

   [[nodiscard]] VALUE* get_inline_buffer() noexcept { return reinterpret_cast<VALUE*>(m_buffer); }
   [[nodiscard]] const VALUE* get_inline_buffer() const noexcept { return reinterpret_cast<const VALUE*>(m_buffer); }

};

/// ## Deduction guides
template<typename T, typename... U>
vector(T, U...) -> vector<T, 1 + sizeof...(U)>;

// ============================================================================
// ## Constructors and destructor defines outside class
// ============================================================================

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

   std::uninitialized_copy_n(o.m_pBuffer, o.m_uSize, this->m_pBuffer);
   this->m_uSize = o.m_uSize;
}


template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(vector&& o) noexcept(std::is_nothrow_move_constructible_v<VALUE>)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( o.is_external() )
   {
      // ### Take ownership of heap storage
      this->m_pHeap = o.m_pHeap;
      this->m_pBuffer = o.m_pHeap;
      this->m_uCapacity = o.m_uCapacity;
         
      o.m_pHeap = nullptr;
      o.m_pBuffer = o.get_inline_buffer();
      o.m_uCapacity = uCapacityStack;
   }
   else
   {
      // ### Move inline elements
      std::uninitialized_move_n(o.m_pBuffer, o.m_uSize, this->m_pBuffer);
   }
      
   this->m_uSize = o.m_uSize;
   o.m_uSize = 0;
}

template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(std::initializer_list<VALUE> list_)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( list_.size() > uCapacityStack )
   {
      this->m_pHeap = std::allocator<VALUE>().allocate(list_.size());
      this->m_pBuffer = this->m_pHeap;
      this->m_uCapacity = list_.size();
   }

   std::uninitialized_copy(list_.begin(), list_.end(), this->m_pBuffer);
   this->m_uSize = list_.size();
}

template<typename VALUE, std::size_t uCapacityStack>
vector<VALUE,uCapacityStack>::vector(size_type uCount, const VALUE& value)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if( uCount > uCapacityStack )
   {
      this->m_pHeap = std::allocator<VALUE>().allocate(uCount);
      this->m_pBuffer = this->m_pHeap;
      this->m_uCapacity = uCount;
   }

   std::uninitialized_fill_n(this->m_pBuffer, uCount, value);
   this->m_uSize = uCount;
}


template<typename VALUE, std::size_t uCapacityStack>
template<std::input_iterator iterator_>
vector<VALUE,uCapacityStack>::vector(iterator_ itFirst, iterator_ itLast)
{
   this->init_base(get_inline_buffer(), uCapacityStack);
      
   if constexpr( std::forward_iterator<iterator_> )
   {
      auto uDistance = static_cast<size_type>(std::distance(itFirst, itLast));
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
   else if( !this->is_external() && !o.is_external() )
   {
      // ### Both using inline storage - swap elements
      vector* pSmaller = (this->m_uSize < o.m_uSize) ? this : &o;
      vector* pLarger = (this->m_uSize < o.m_uSize) ? &o : this;
         
      size_type uMinSize = pSmaller->m_uSize;
      for( size_type i = 0; i < uMinSize; ++i )
      {
         std::swap((*pSmaller)[i], (*pLarger)[i]);
      }
         
      // move remaining elements from larger to smaller
      std::uninitialized_move_n(pLarger->m_pBuffer + uMinSize, pLarger->m_uSize - uMinSize, pSmaller->m_pBuffer + uMinSize);
      std::destroy_n(pLarger->m_pBuffer + uMinSize, pLarger->m_uSize - uMinSize);
         
      std::swap(pSmaller->m_uSize, pLarger->m_uSize);
   }
   else
   {
      // ### One heap, one inline - need to swap carefully
      vector* pHeap = this->is_external() ? this : &o;
      vector* pInline = this->is_external() ? &o : this;
         
      VALUE* pHeapStorage = pHeap->m_pHeap;
      size_type uHeapCapacity = pHeap->m_uCapacity;
         
      // move inline elements to heap's inline storage
      std::uninitialized_move_n(pInline->m_pBuffer, pInline->m_uSize, pHeap->get_inline_buffer());
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


#endif // GD_COMPILER_HAS_CPP20_SUPPORT