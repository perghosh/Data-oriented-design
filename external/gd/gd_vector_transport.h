// @FILE [tag: vector, transport] [description: template vector with optional borrowed storage and a 16 byte footprint] [name: gd_vector_transport.h] [type: header] 

#pragma once

#include <algorithm>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility> 

/*
 * # vector_transport
 *
 * Jump board
 *
 * append__          Append elements to the vector_transport
 * attributes__      Member attributes and state flags for vector_transport
 * capacity__        Size, capacity and reservation of the vector_transport
 * construct__       Construct a vector_transport object with optional borrowed buffer
 * iterator__        Iterators for traversing elements in the vector_transport
 * modify__          Insert, erase, resize and assign elements
 */

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

/** @CLASS [tag: vector, transport] [summary: vector used for transport and minimal memory allocation]
 * @brief Mutable sequence of elements that can use caller-provided storage.
 *
 * Template counterpart of string_transport. Stores elements in either a
 * borrowed writable buffer or owned heap storage and keeps the same 16 byte
 * footprint (one state word and one pointer). The API follows std::vector.
 *
 * @par Borrowed storage
 * Construct from a writable T pointer and capacity, std::span<T>, or
 * std::span<std::byte>. Compatible std::array and std::vector objects can
 * be passed through these span constructors.
 *
 * The buffer must have room for between 1 and 65535 elements and be aligned
 * for T. Construction initializes an empty vector; existing buffer contents
 * are not imported. The buffer is treated as raw storage: elements are
 * created in it with placement new and destroyed when removed, the buffer
 * itself is never deleted.
 *
 * The caller owns the borrowed buffer. It must remain valid and at the same
 * address while this object uses it.
 *
 * When growth exceeds the borrowed capacity, the elements are moved to owned
 * heap storage and the object stays owned from then on.
 *
 * @par Size and capacity
 * size() returns the number of elements and capacity() the number of elements
 * the current storage can hold, exactly as in std::vector. Owned storage
 * grows geometrically (capacity doubles) so push_back is amortized constant.
 * Limits: borrowed buffers hold at most 65535 elements, owned storage
 * at most 1073741823 elements (max_size()).
 *
 * @par State word
 * @code
 * bit 61        borrowed flag
 * bit 60        literal flag (reserved)
 * borrowed:     bits 16..31 capacity, bits 0..15 size
 * owned:        bits 30..59 capacity, bits 0..29 size
 * @endcode
 *
 * @par Ownership and lifetime
 * Copying creates an independent owned copy. Moving transfers the pointer
 * and storage state; moving a borrowed vector does not extend the lifetime
 * of its external buffer. swap() exchanges storage including borrowed buffers.
 * Operations that allocate invalidate pointers, references and iterators.
 *
 * @note A default-constructed or moved-from object has size zero and
 * data() returns nullptr.
 *
 * @par Differences from std::vector
 * No allocator parameter (std::allocator is used), no specialization for bool.
 *
 * @par Example: build values in a local buffer
 * @code{.cpp}
 * std::array<int, 8> arrayBuffer;
 * gd::vector_transport<int> vectorValue(arrayBuffer);
 *
 * vectorValue.push_back(10);
 * vectorValue.push_back(20);
 * vectorValue.insert(vectorValue.begin() + 1, 15);
 *
 * // Values: 10, 15, 20
 * // size() == 3; capacity() == 8; is_borrowed() == true.
 * @endcode
 *
 * @par Example: owned storage with non-trivial elements
 * @code{.cpp}
 * gd::vector_transport<std::string> vectorName = { "alpha", "beta" };
 * vectorName.emplace_back("gamma");
 * vectorName.erase(vectorName.begin());
 * @endcode
 */
   template<typename T>
class vector_transport
{
public:
   enum class enumConstant : uint64_t
   {
      eFlagBorrowed = 1ull << 61,
      eFlagLiteral = 1ull << 60,

      eClearFlag = 0x0FFFFFFF'FFFFFFFFull,

      eBorrowCapacityShift = 16,
      eBorrowCapacityMask = 0x00000000'FFFF0000ull,
      eBorrowSizeMask = 0x00000000'0000FFFFull,

      eOwnedCapacityShift = 30,
      eOwnedCapacityMask = 0x0FFFFFFF'C0000000ull,
      eOwnedSizeMask = 0x00000000'3FFFFFFFull,
   };

public:
   using value_type = T;
   using allocator_type = std::allocator<T>;
   using size_type = std::size_t;
   using difference_type = std::ptrdiff_t;
   using reference = value_type&;
   using const_reference = const value_type&;
   using pointer = value_type*;
   using const_pointer = const value_type*;

   using iterator = pointer;
   using const_iterator = const_pointer;
   using reverse_iterator = std::reverse_iterator<iterator>;
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;

   // @API [tag: construction] [summary: construction and assignment] [jump: construct__]
public:
   vector_transport() noexcept : m_uStateAndSize(0), m_pValue(nullptr) {}

   // Constructors that can throw delegate to the default constructor, once it has finished the destructor runs and frees the storage if a later step throws.
   explicit vector_transport(size_type uCount) : vector_transport() { resize(uCount); }
   vector_transport(size_type uCount, const T& valueElement) : vector_transport() { resize(uCount, valueElement); }

   template<std::input_iterator InputIt>
   vector_transport(InputIt itFirst, InputIt itLast) : vector_transport() { append(itFirst, itLast); }

   vector_transport(std::initializer_list<T> initializerlistValue) : vector_transport() { append(initializerlistValue); }

   // Borrow an external writable buffer (no allocation).
   vector_transport(T* pBuffer, size_type uBufferCapacity);

   explicit vector_transport(std::span<T> spanBuffer)                          // Flexible adapter: accepts std::array<T, N>, std::vector<T>, etc.
      : vector_transport(spanBuffer.data(), spanBuffer.size()) {
   }

   explicit vector_transport(std::span<std::byte> spanBuffer)                  // Raw storage: the capacity is the number of whole elements that fit.
      requires (std::is_same_v<T, std::byte> == false)
   : vector_transport(reinterpret_cast<T*>(spanBuffer.data()), spanBuffer.size() / sizeof(T)) {
   }

   vector_transport(std::span<const T> spanValue, T* pBuffer, size_type uBufferCapacity)
      : vector_transport(pBuffer, uBufferCapacity) {
      append(spanValue);
   }

   vector_transport(std::span<const T> spanValue, std::span<T> spanBuffer)
      : vector_transport(spanValue, spanBuffer.data(), spanBuffer.size()) {
   }

   // copy
   vector_transport(const vector_transport& o) : vector_transport() { common_construct(o); }
   vector_transport(vector_transport&& o) noexcept : m_uStateAndSize(0), m_pValue(nullptr) { common_construct(std::move(o)); }

   // assign
   vector_transport& operator=(const vector_transport& o)
   {
      if(this != &o) { common_construct(o); }
      return *this;
   }
   vector_transport& operator=(vector_transport&& o) noexcept
   {
      if(this != &o) { common_construct(std::move(o)); }
      return *this;
   }
   vector_transport& operator=(std::initializer_list<T> initializerlistValue) { assign(initializerlistValue); return *this; }

   ~vector_transport() { release(); }
private:
   void release() noexcept;
   void common_construct(const vector_transport& o);
   void common_construct(vector_transport&& o) noexcept;

   // @API [tag: operator]
public:
   reference operator[](size_type uIndex) noexcept { assert(uIndex < size()); return m_pValue[uIndex]; }
   const_reference operator[](size_type uIndex) const noexcept { assert(uIndex < size()); return m_pValue[uIndex]; }

   reference at(size_type uIndex);
   const_reference at(size_type uIndex) const;

   reference front() noexcept { assert(size() > 0); return m_pValue[0]; }
   const_reference front() const noexcept { assert(size() > 0); return m_pValue[0]; }

   reference back() noexcept { assert(size() > 0); return m_pValue[size() - 1]; }
   const_reference back() const noexcept { assert(size() > 0); return m_pValue[size() - 1]; }

   // ## methods ------------------------------------------------------------------
public:
   // @API [tag: get, set]

   // ## state
   bool is_borrowed() const noexcept { return (m_uStateAndSize & static_cast<uint64_t>(enumConstant::eFlagBorrowed)) != 0; }
   bool is_literal()  const noexcept { return (m_uStateAndSize & static_cast<uint64_t>(enumConstant::eFlagLiteral)) != 0; }
   bool is_owned()    const noexcept { return (m_uStateAndSize & (static_cast<uint64_t>(enumConstant::eFlagBorrowed) | static_cast<uint64_t>(enumConstant::eFlagLiteral))) == 0; }

   void clear_flags() { m_uStateAndSize &= static_cast<uint64_t>(enumConstant::eClearFlag); }

   allocator_type get_allocator() const noexcept { return allocator_type(); }

   // @API [tag: capacity] [summary: size, capacity and reservation] [jump: capacity__]
   bool empty() const noexcept { return size() == 0; }
   size_type size() const noexcept;                ///< number of elements
   size_type max_size() const noexcept { return (std::min)(static_cast<size_type>(enumConstant::eOwnedSizeMask), static_cast<size_type>((std::numeric_limits<difference_type>::max)()) / sizeof(T)); }
   size_type capacity() const noexcept;            ///< number of elements the current storage (borrowed buffer or owned allocation) can hold
   void reserve(size_type uCapacity);
   void shrink_to_fit();

   // @API [tag: operation]

   // @API [tag: append] [summary: append elements to the vector] [jump: append__]
   void append(const T* pValue, size_type uCount) { append(pValue, pValue + uCount); }
   void append(std::span<const T> spanValue) { append(spanValue.data(), spanValue.size()); }
   void append(std::initializer_list<T> initializerlistValue) { append(initializerlistValue.begin(), initializerlistValue.end()); }

   template<std::input_iterator InputIt>
   void append(InputIt itFirst, InputIt itLast)
   {
      if constexpr(std::forward_iterator<InputIt>)
      {
         const auto uCount = static_cast<size_type>(std::distance(itFirst, itLast));
         append_with(uCount, [&](pointer pDestination) { std::uninitialized_copy(itFirst, itLast, pDestination); });
      }
      else
      {
         for(; itFirst != itLast; ++itFirst) { emplace_back(*itFirst); }     // single pass iterators, one element at a time
      }
   }

   void push_back(const T& valueElement) { emplace_back(valueElement); }
   void push_back(T&& valueElement) { emplace_back(std::move(valueElement)); }
   template<typename... Args>
   reference emplace_back(Args&&... arguments);
   void pop_back();

   // @API [tag: modify] [summary: insert, erase, resize and assign elements] [jump: modify__]
   void assign(size_type uCount, const T& valueElement);
   void assign(std::initializer_list<T> initializerlistValue) { assign(initializerlistValue.begin(), initializerlistValue.end()); }
   template<std::input_iterator InputIt>
   void assign(InputIt itFirst, InputIt itLast) { clear(); append(itFirst, itLast); }

   iterator insert(const_iterator itPosition, const T& valueElement) { return emplace(itPosition, valueElement); }
   iterator insert(const_iterator itPosition, T&& valueElement) { return emplace(itPosition, std::move(valueElement)); }
   iterator insert(const_iterator itPosition, size_type uCount, const T& valueElement);
   iterator insert(const_iterator itPosition, std::initializer_list<T> initializerlistValue) { return insert(itPosition, initializerlistValue.begin(), initializerlistValue.end()); }

   template<std::input_iterator InputIt>
   iterator insert(const_iterator itPosition, InputIt itFirst, InputIt itLast)
   {
      const auto uIndex = static_cast<size_type>(itPosition - cbegin());
      const auto uSize = size();
      if(uIndex > uSize) { throw std::out_of_range("vector_transport::insert"); }

      append(itFirst, itLast);                                                 // append at the end, then rotate the new elements into place
      std::rotate(begin() + uIndex, begin() + uSize, end());
      return begin() + uIndex;
   }

   template<typename... Args>
   iterator emplace(const_iterator itPosition, Args&&... arguments);

   iterator erase(const_iterator itPosition) { return erase(itPosition, itPosition + 1); }
   iterator erase(const_iterator itFirst, const_iterator itLast);

   void resize(size_type uCount);
   void resize(size_type uCount, const T& valueElement);

   void clear() noexcept;
   void swap(vector_transport& o) noexcept { std::swap(m_uStateAndSize, o.m_uStateAndSize); std::swap(m_pValue, o.m_pValue); }

   // @API [tag: iterator] [summary: iterate over elements] [jump: iterator__]
   iterator begin() noexcept { return m_pValue; }
   const_iterator begin() const noexcept { return m_pValue; }
   const_iterator cbegin() const noexcept { return begin(); }

   iterator end() noexcept { return m_pValue != nullptr ? m_pValue + size() : nullptr; }
   const_iterator end() const noexcept { return m_pValue != nullptr ? m_pValue + size() : nullptr; }
   const_iterator cend() const noexcept { return end(); }

   reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
   const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
   const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }

   reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
   const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
   const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

   pointer data() noexcept { return m_pValue; }
   const_pointer data() const noexcept { return m_pValue; }

protected:
   // @API [tag: internal]
   void set_size(size_type uSize) noexcept;
   void set_owned(pointer pValue, size_type uSize, size_type uCapacity) noexcept;
   void commit_storage(pointer pValueNew, size_type uSize, size_type uCapacity) noexcept;
   void truncate(size_type uSize) noexcept;
   void reallocate(size_type uCapacity);
   size_type grow_capacity(size_type uMinimum) const;

   template<typename Construct>
   void append_with(size_type uCount, Construct&& lambdaConstruct);

   static void relocate(pointer pFrom, size_type uCount, pointer pTo);
   static pointer allocate_storage(size_type uCapacity) { return allocator_type().allocate(uCapacity); }
   static void deallocate_storage(pointer pValue, size_type uCapacity) noexcept { allocator_type().deallocate(pValue, uCapacity); }

public:
   // @API [tag: debug]

   // ## attributes ---------------------------------------------------------------- [jump: attributes__]
public:
   uint64_t m_uStateAndSize;  ///< The state, size and capacity of the vector_transport object, including flags for borrowed or literal storage.
   T* m_pValue;               ///< Element buffer. It may point to a borrowed buffer or owned heap storage.


   // @API [tag: free-functions]
public:
   friend void swap(vector_transport& vectorLeft, vector_transport& vectorRight) noexcept { vectorLeft.swap(vectorRight); }

   friend bool operator==(const vector_transport& vectorLeft, const vector_transport& vectorRight)
   {
      return vectorLeft.size() == vectorRight.size() && std::equal(vectorLeft.begin(), vectorLeft.end(), vectorRight.begin());
   }

   friend auto operator<=>(const vector_transport& vectorLeft, const vector_transport& vectorRight)
   {
      return std::lexicographical_compare_three_way(vectorLeft.begin(), vectorLeft.end(), vectorRight.begin(), vectorRight.end());
   }
};

/// Construct vector with borrowed buffer, the buffer must be writable, aligned for T and the capacity must be less than 65536 elements (16-bit field)
template<typename T>
vector_transport<T>::vector_transport(T* pBuffer, size_type uBufferCapacity) : m_uStateAndSize(0), m_pValue(pBuffer)
{
   assert(pBuffer != nullptr);
   assert(uBufferCapacity > 0);
   assert(uBufferCapacity <= 0xFFFFu); // borrow-capacity field is 16-bit
   assert(reinterpret_cast<std::uintptr_t>(pBuffer) % alignof(T) == 0);
   // ## Set the borrowed flag and store the buffer capacity in bits 16..31 of m_uStateAndSize, the size is zero
   const auto uCapacity = (std::min)(uBufferCapacity, static_cast<size_type>(0xFFFFu));
   m_uStateAndSize = static_cast<uint64_t>(enumConstant::eFlagBorrowed);
   m_uStateAndSize |= (static_cast<uint64_t>(uCapacity) << static_cast<unsigned>(enumConstant::eBorrowCapacityShift)) & static_cast<uint64_t>(enumConstant::eBorrowCapacityMask);
}

/// Returns a reference to the element at the specified index, with bounds checking.
template<typename T>
typename vector_transport<T>::reference vector_transport<T>::at(size_type uIndex)
{
   if(uIndex >= size()) { throw std::out_of_range("vector_transport::at"); }
   return m_pValue[uIndex];
}

/// Returns a const reference to the element at the specified index, with bounds checking.
template<typename T>
typename vector_transport<T>::const_reference vector_transport<T>::at(size_type uIndex) const
{
   if(uIndex >= size()) { throw std::out_of_range("vector_transport::at"); }
   return m_pValue[uIndex];
}

/// Returns the number of elements in the vector_transport ---------------------
template<typename T>
typename vector_transport<T>::size_type vector_transport<T>::size() const noexcept
{
   size_type uSize;
   if(is_borrowed() == true)
   {
      uSize = static_cast<size_type>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eBorrowSizeMask));
   }
   else
   {
      uSize = static_cast<size_type>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eOwnedSizeMask));
   }

   return uSize;
}

/// Returns the number of elements the storage can hold, if borrowed, it returns the capacity of the buffer that it borrows, if owner then it returns the capacity of the allocation that it owns.
template<typename T>
typename vector_transport<T>::size_type vector_transport<T>::capacity() const noexcept
{
   size_type uCapacity;
   if(is_borrowed() == true)
   {
      uCapacity = static_cast<size_type>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eBorrowCapacityMask));
      uCapacity >>= static_cast<unsigned>(enumConstant::eBorrowCapacityShift);
   }
   else
   {
      uCapacity = static_cast<size_type>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eOwnedCapacityMask));
      uCapacity >>= static_cast<unsigned>(enumConstant::eOwnedCapacityShift);
   }

   return uCapacity;
}

/** ---------------------------------------------------------------------------
 * @brief Makes sure the vector can hold at least the requested number of elements without allocating again.
 * @param uCapacity The wanted capacity in elements. If it fits in the current storage, nothing happens; a borrowed buffer that is too small is replaced by owned storage.
 */
template<typename T>
void vector_transport<T>::reserve(size_type uCapacity)
{
   if(uCapacity <= capacity()) { return; }
   if(uCapacity > max_size()) { throw std::length_error("vector_transport::reserve"); }
   reallocate(uCapacity);
}

/// Reduces the owned allocation to the number of stored elements, a borrowed buffer is left as it is.
template<typename T>
void vector_transport<T>::shrink_to_fit()
{
   if(is_owned() == false) { return; }                                       // a borrowed buffer cannot be shrunk
   const auto uSize = size();
   if(uSize == capacity()) { return; }
   if(uSize == 0) { release(); return; }
   reallocate(uSize);
}

/** ---------------------------------------------------------------------------
 * @brief Constructs a new element at the end of the vector and returns a reference to it.
 * @param arguments Arguments forwarded to the constructor of the element. They may refer to an element of this vector.
 * @return A reference to the new element.
 */
template<typename T>
template<typename... Args>
typename vector_transport<T>::reference vector_transport<T>::emplace_back(Args&&... arguments)
{
   append_with(1, [&](pointer pDestination) { std::construct_at(pDestination, std::forward<Args>(arguments)...); });
   return m_pValue[size() - 1];
}

/// Destroys the last element, the vector must not be empty.
template<typename T>
void vector_transport<T>::pop_back()
{
   const auto uSize = size();                                                                       assert(uSize > 0);
   std::destroy_at(m_pValue + uSize - 1);
   set_size(uSize - 1);
}

/** ---------------------------------------------------------------------------
 * @brief Replaces the content with a number of copies of a value.
 * @param uCount The number of elements after the call.
 * @param valueElement The value to copy. It may refer to an element of this vector.
 */
template<typename T>
void vector_transport<T>::assign(size_type uCount, const T& valueElement)
{
   const T valueCopy(valueElement);                                            // the value may live in this vector and clear() destroys it
   clear();
   append_with(uCount, [&](pointer pDestination) { std::uninitialized_fill_n(pDestination, uCount, valueCopy); });
}

/** ---------------------------------------------------------------------------
 * @brief Inserts a number of copies of a value at the specified position.
 * @param itPosition Iterator to the element before which the copies are inserted, end() appends.
 * @param uCount The number of copies to insert.
 * @param valueElement The value to copy.
 * @return An iterator to the first inserted element, or itPosition when uCount is zero.
 */
template<typename T>
typename vector_transport<T>::iterator vector_transport<T>::insert(const_iterator itPosition, size_type uCount, const T& valueElement)
{
   const auto uIndex = static_cast<size_type>(itPosition - cbegin());
   const auto uSize = size();
   if(uIndex > uSize) { throw std::out_of_range("vector_transport::insert"); }

   append_with(uCount, [&](pointer pDestination) { std::uninitialized_fill_n(pDestination, uCount, valueElement); });
   std::rotate(begin() + uIndex, begin() + uSize, end());                      // move the appended elements to the insert position
   return begin() + uIndex;
}

/** ---------------------------------------------------------------------------
 * @brief Constructs an element in place at the specified position, shifting the following elements to the right.
 * @param itPosition Iterator to the element before which the new element is created, end() appends.
 * @param arguments Arguments forwarded to the constructor of the element.
 * @return An iterator to the new element.
 */
template<typename T>
template<typename... Args>
typename vector_transport<T>::iterator vector_transport<T>::emplace(const_iterator itPosition, Args&&... arguments)
{
   const auto uIndex = static_cast<size_type>(itPosition - cbegin());
   if(uIndex > size()) { throw std::out_of_range("vector_transport::emplace"); }

   emplace_back(std::forward<Args>(arguments)...);                             // create at the end first, then rotate it into place
   std::rotate(begin() + uIndex, end() - 1, end());
   return begin() + uIndex;
}

/** ---------------------------------------------------------------------------
 * @brief Erases a range of elements, the following elements are moved to the left.
 * @param itFirst First element to erase.
 * @param itLast One past the last element to erase.
 * @return An iterator to the element that followed the erased range.
 */
template<typename T>
typename vector_transport<T>::iterator vector_transport<T>::erase(const_iterator itFirst, const_iterator itLast)
{
   const auto uFirst = static_cast<size_type>(itFirst - cbegin());
   const auto uLast = static_cast<size_type>(itLast - cbegin());
   const auto uSize = size();                                                                       assert(uFirst <= uLast);
   if(uFirst > uLast || uLast > uSize) { throw std::out_of_range("vector_transport::erase"); }
   if(uFirst == uLast) { return begin() + uFirst; }

   std::move(begin() + uLast, end(), begin() + uFirst);
   truncate(uSize - (uLast - uFirst));
   return begin() + uFirst;
}

/// Resizes the vector, new elements are value-initialized, surplus elements are destroyed.
template<typename T>
void vector_transport<T>::resize(size_type uCount)
{
   const auto uSize = size();
   if(uCount < uSize) { truncate(uCount); return; }

   const auto uAddCount = uCount - uSize;
   append_with(uAddCount, [&](pointer pDestination) { std::uninitialized_value_construct_n(pDestination, uAddCount); });
}

/// Resizes the vector, new elements are copies of the value, surplus elements are destroyed.
template<typename T>
void vector_transport<T>::resize(size_type uCount, const T& valueElement)
{
   const auto uSize = size();
   if(uCount < uSize) { truncate(uCount); return; }

   const auto uAddCount = uCount - uSize;
   append_with(uAddCount, [&](pointer pDestination) { std::uninitialized_fill_n(pDestination, uAddCount, valueElement); });
}

/// clear elements (does not reallocate)
template<typename T>
void vector_transport<T>::clear() noexcept
{
   std::destroy_n(m_pValue, size());
   set_size(0);
}

/// Release the vector_transport storage, the elements are destroyed, if the buffer is owned it is freed, if it is borrowed, it will not be freed.
template<typename T>
void vector_transport<T>::release() noexcept
{
   std::destroy_n(m_pValue, size());
   if(is_owned() == true && m_pValue != nullptr)
   {
      deallocate_storage(m_pValue, capacity());
   }
   m_pValue = nullptr;
   m_uStateAndSize = 0;
}

/// helper function to handle common construction logic for copy operations, the result is always owned storage.
template<typename T>
void vector_transport<T>::common_construct(const vector_transport& o)
{
   release();                                                                  // release any existing resources and clear the state of the current vector_transport object

   const auto uSize = o.size();
   if(uSize == 0) { return; }

   pointer pValueNew = allocate_storage(uSize);
   try { std::uninitialized_copy_n(o.m_pValue, uSize, pValueNew); }
   catch(...) { deallocate_storage(pValueNew, uSize); throw; }
   set_owned(pValueNew, uSize, uSize);
}

/// helper function to handle common construction logic for move operations. --
template<typename T>
void vector_transport<T>::common_construct(vector_transport&& o) noexcept
{
   release();

   m_uStateAndSize = o.m_uStateAndSize;
   m_pValue = o.m_pValue;

   o.m_uStateAndSize = 0;
   o.m_pValue = nullptr;
}

/// Set the size for vector_transport, the size must not be larger than the capacity, flags and capacity are preserved.
/// @param uSize The new number of elements, the elements themselves must already be constructed or destroyed.
template<typename T>
void vector_transport<T>::set_size(size_type uSize) noexcept
{
   assert(uSize <= capacity());
   const auto uSizeMask = is_borrowed() == true ? static_cast<uint64_t>(enumConstant::eBorrowSizeMask) : static_cast<uint64_t>(enumConstant::eOwnedSizeMask);
   m_uStateAndSize = (m_uStateAndSize & ~uSizeMask) | static_cast<uint64_t>(uSize);
}

/// Take over a heap allocation as owned storage, the borrowed or literal flags are cleared. Does not release the current storage.
template<typename T>
void vector_transport<T>::set_owned(pointer pValue, size_type uSize, size_type uCapacity) noexcept
{
   assert(uSize <= uCapacity);
   m_pValue = pValue;
   m_uStateAndSize = static_cast<uint64_t>(uSize) & static_cast<uint64_t>(enumConstant::eOwnedSizeMask);
   m_uStateAndSize |= (static_cast<uint64_t>(uCapacity) << static_cast<unsigned>(enumConstant::eOwnedCapacityShift)) & static_cast<uint64_t>(enumConstant::eOwnedCapacityMask);
}

/// Switch to new owned storage whose first elements were constructed already: destroys the elements of the old storage, frees it if it was owned (a borrowed buffer is never freed).
template<typename T>
void vector_transport<T>::commit_storage(pointer pValueNew, size_type uSize, size_type uCapacity) noexcept
{
   std::destroy_n(m_pValue, size());                                           // old elements, size() still reports the old storage
   if(is_owned() == true && m_pValue != nullptr) { deallocate_storage(m_pValue, capacity()); }
   set_owned(pValueNew, uSize, uCapacity);
}

/// Destroys the elements from uSize and up, the capacity is not changed.
template<typename T>
void vector_transport<T>::truncate(size_type uSize) noexcept
{
   assert(uSize <= size());
   std::destroy(m_pValue + uSize, m_pValue + size());
   set_size(uSize);
}

/// Moves the elements to new owned storage with the given capacity.
template<typename T>
void vector_transport<T>::reallocate(size_type uCapacity)
{
   assert(uCapacity >= size());
   const auto uSize = size();
   pointer pValueNew = allocate_storage(uCapacity);                            // allocate new memory for the vector_transport buffer
   try { relocate(m_pValue, uSize, pValueNew); }
   catch(...) { deallocate_storage(pValueNew, uCapacity); throw; }
   commit_storage(pValueNew, uSize, uCapacity);
}

/// Capacity to grow to when at least uMinimum elements are needed, capacity doubles.
template<typename T>
typename vector_transport<T>::size_type vector_transport<T>::grow_capacity(size_type uMinimum) const
{
   const auto uMax = max_size();
   if(uMinimum > uMax) { throw std::length_error("vector_transport::grow_capacity"); }

   const auto uCapacity = capacity();
   if(uCapacity > uMax / 2) { return uMax; }
   return (std::max)(uMinimum, uCapacity * 2);
}

/// Copies or moves elements into uninitialized storage, elements are moved when that cannot throw or when they cannot be copied (the old elements are not destroyed here).
template<typename T>
void vector_transport<T>::relocate(pointer pFrom, size_type uCount, pointer pTo)
{
   if constexpr(std::is_trivially_copyable_v<T>)
   {
      if(uCount > 0) { std::memcpy(static_cast<void*>(pTo), static_cast<const void*>(pFrom), uCount * sizeof(T)); }
   }
   else if constexpr(std::is_nothrow_move_constructible_v<T> || std::is_copy_constructible_v<T> == false)
   {
      std::uninitialized_move_n(pFrom, uCount, pTo);
   }
   else
   {
      std::uninitialized_copy_n(pFrom, uCount, pTo);
   }
}

/** ---------------------------------------------------------------------------
 * @brief Appends uCount elements created by lambdaConstruct to the end of the vector.
 * @param uCount Number of elements to create.
 * @param lambdaConstruct Called with a pointer to uninitialized storage for uCount elements, it must construct all of them or throw without leaving any constructed.
 *
 * When the storage must grow the new elements are created in the new storage before the
 * old ones are moved, so the source of the new elements may live in this vector.
 * If anything throws the vector is left unchanged.
 */
template<typename T>
template<typename Construct>
void vector_transport<T>::append_with(size_type uCount, Construct&& lambdaConstruct)
{
   if(uCount == 0) { return; }

   const auto uSize = size();
   if(uCount > max_size() - uSize) { throw std::length_error("vector_transport::append"); }

   if(uSize + uCount <= capacity())                                          // fits in the current storage (borrowed or owned)
   {
      lambdaConstruct(m_pValue + uSize);
      set_size(uSize + uCount);
      return;
   }

   // ## allocate new memory for the combined sequence ........................
   const auto uCapacityNew = grow_capacity(uSize + uCount);
   pointer pValueNew = allocate_storage(uCapacityNew);
   bool bConstructed = false;
   try
   {
      lambdaConstruct(pValueNew + uSize);
      bConstructed = true;
      relocate(m_pValue, uSize, pValueNew);
   }
   catch(...)
   {
      if(bConstructed == true) { std::destroy_n(pValueNew + uSize, uCount); }
      deallocate_storage(pValueNew, uCapacityNew);
      throw;
   }
   commit_storage(pValueNew, uSize + uCount, uCapacityNew);
}

static_assert(sizeof(vector_transport<char>) == 16, "vector_transport must be 16 bytes");
static_assert(sizeof(vector_transport<int>) == 16, "vector_transport must be 16 bytes");
static_assert(sizeof(vector_transport<double>) == 16, "vector_transport must be 16 bytes");

_GD_END
