// @FILE [tag: string] [description: String class with borrow capability] [type: header] [name: gd_string.h]

/**
 * \file gd_string.h
 *
 * \brief String class that can own or borrow memory, mimicking std::string interface
 *
 * This string class provides std::string-like functionality with the ability to borrow
 * external memory without taking ownership. The high bit of the capacity field tracks
 * ownership: when set, the string borrows memory; when clear, it owns the memory.
 *
 | Area                | Methods (Examples)                                                                                      | Description                                                                                   |
 |---------------------|--------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------|
 | Construction        | `string()`, `string(const char*)`, `string(const string&)`, `string(string&&)`, `borrow(...)`         | Constructors for creating, copying, moving strings, and borrowing external memory.            |
 | Assignment          | `operator=(const string&)`, `operator=(string&&)`, `operator=(const char*)`, `swap(...)`              | Methods for assigning or moving string contents, including efficient swap operations.         |
 | Element Access      | `operator[](...)`, `at(...)`, `front()`, `back()`, `data()`, `c_str()`                                | Methods for accessing characters in the string with bounds checking or direct access.         |
 | Iterators           | `begin()`, `end()`, `cbegin()`, `cend()`, `rbegin()`, `rend()`, `crbegin()`, `crend()`                | Iterator methods for forward, const, and reverse traversal of string characters.              |
 | Capacity            | `empty()`, `size()`, `length()`, `capacity()`, `reserve(...)`, `is_borrowed()`                        | Methods for querying size, capacity, and memory ownership status.                             |
 | Modifiers           | `push_back(...)`, `append(...)`, `operator+=(...)`, `insert(...)`, `erase(...)`, `clear()`            | Methods for adding, removing, and modifying characters in the string.                         |
 | String Operations   | `substr(...)`, `compare(...)`, `find(...)`, `rfind(...)`, `starts_with(...)`, `ends_with(...)`        | String manipulation and search operations.                                                    |
 | Comparison          | `operator==(...)`, `operator<=>(...)`                                                                  | Comparison operators for lexicographical comparison between strings.                          |
 | Memory Management   | `allocate(...)`, `destroy()`, `borrow(...)`, `owner()`                                                 | Internal methods for memory allocation, deallocation, borrowing, and ownership tracking.      |
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
#include <string_view>
#include <type_traits>
#include <utility>

#include "gd_compiler.h"

#if GD_COMPILER_HAS_CPP20_SUPPORT

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

/// Namespace for borrowable string that can own or reference external memory
namespace borrow {

// ============================================================================
// ## string - A string class that can own or borrow memory
// ============================================================================

/**
 * @brief String class with memory borrowing capability
 * 
 * This class provides std::string-like functionality with the ability to borrow
 * external memory without taking ownership. Uses the high bit (bit 63) of the
 * capacity field to track ownership:
 * - Bit 63 = 0: String owns the memory (must deallocate)
 * - Bit 63 = 1: String borrows memory (must not deallocate)
 * 
 * Bits 0-62 store the actual capacity (max ~9 exabytes, practically unlimited)
 */
class string
{
public:
   using value_type      = char;                                              // Type of characters stored
   using size_type       = std::size_t;                                       // Type for size measurements
   using difference_type = std::ptrdiff_t;                                    // Type for differences between iterators
   using reference       = char&;                                             // Reference to character
   using const_reference = const char&;                                       // Const reference to character
   using pointer         = char*;                                             // Pointer to character
   using const_pointer   = const char*;                                       // Const pointer to character
   using iterator        = char*;                                             // Iterator for traversing characters
   using const_iterator  = const char*;                                       // Const iterator for traversing characters
   using reverse_iterator       = std::reverse_iterator<iterator>;            // Iterator for reverse traversal
   using const_reverse_iterator = std::reverse_iterator<const_iterator>;      // Const iterator for reverse traversal

   static constexpr size_type npos = static_cast<size_type>(-1);              // Invalid position constant

   // ## Constructors and destructor .........................................

   string() noexcept;
   string(const char* psz);
   string(const char* psz, size_type uLength);
   string(size_type uCount, char ch);
   string(std::string_view sv);
   
   // ## Borrow constructors .................................................
   
   explicit string(char* pBuffer, size_type uCapacity) noexcept;             // borrow buffer with capacity
   
   template<std::size_t uN>
   explicit string(char (&array_)[uN]) noexcept;                             // borrow C-array
   
   template<typename CONTAINER>
   requires requires(CONTAINER& c_) {
      { c_.data() } -> std::convertible_to<char*>;                            // verify container has .data() returning pointer to char
      { c_.size() } -> std::convertible_to<std::size_t>;                      // verify container has .size() returning size type
   }
   && ( !std::is_same_v<std::remove_cvref_t<CONTAINER>, string> )
   explicit string(CONTAINER& container_) noexcept;                           // borrow from container
   
   // ## Copy and move constructors ..........................................
   
   string(const string& o);
   string(string&& o) noexcept;
   ~string() noexcept;

   // ## Assignment operators ................................................

   string& operator=(const string& o);
   string& operator=(string&& o) noexcept;
   string& operator=(const char* psz);
   string& operator=(std::string_view sv);

   // ## Borrow memory from external source ..................................

   void borrow(const char* psz) noexcept;
   void borrow(const char* psz, size_type uLength) noexcept;

   // ## Element access ......................................................

   [[nodiscard]] reference operator[](size_type uIndex) noexcept;
   [[nodiscard]] const_reference operator[](size_type uIndex) const noexcept;
   [[nodiscard]] reference at(size_type uIndex);
   [[nodiscard]] const_reference at(size_type uIndex) const;
   [[nodiscard]] reference front() noexcept;
   [[nodiscard]] const_reference front() const noexcept;
   [[nodiscard]] reference back() noexcept;
   [[nodiscard]] const_reference back() const noexcept;
   [[nodiscard]] char* data() noexcept;
   [[nodiscard]] const char* data() const noexcept;
   [[nodiscard]] const char* c_str() const noexcept;

   // ## Iterators ...........................................................

   [[nodiscard]] iterator begin() noexcept;
   [[nodiscard]] iterator end() noexcept;
   [[nodiscard]] const_iterator begin() const noexcept;
   [[nodiscard]] const_iterator end() const noexcept;
   [[nodiscard]] const_iterator cbegin() const noexcept;
   [[nodiscard]] const_iterator cend() const noexcept;
   [[nodiscard]] reverse_iterator rbegin() noexcept;
   [[nodiscard]] reverse_iterator rend() noexcept;
   [[nodiscard]] const_reverse_iterator rbegin() const noexcept;
   [[nodiscard]] const_reverse_iterator rend() const noexcept;
   [[nodiscard]] const_reverse_iterator crbegin() const noexcept;
   [[nodiscard]] const_reverse_iterator crend() const noexcept;

   // ## Capacity ............................................................

   [[nodiscard]] bool empty() const noexcept;
   [[nodiscard]] size_type size() const noexcept;
   [[nodiscard]] size_type length() const noexcept;
   [[nodiscard]] size_type capacity() const noexcept;
   [[nodiscard]] bool is_borrowed() const noexcept;
   void reserve(size_type uNewCapacity);
   void shrink_to_fit();

   // ## Modifiers ...........................................................

   void clear() noexcept;
   void push_back(char ch);
   void pop_back() noexcept;
   string& append(const char* psz);
   string& append(const char* psz, size_type uLength);
   string& append(const string& str);
   string& append(std::string_view sv);
   string& append(size_type uCount, char ch);
   string& operator+=(char ch);
   string& operator+=(const char* psz);
   string& operator+=(const string& str);
   string& operator+=(std::string_view sv);
   iterator insert(size_type uPos, size_type uCount, char ch);
   iterator insert(size_type uPos, const char* psz);
   iterator insert(size_type uPos, const char* psz, size_type uLength);
   string& erase(size_type uPos = 0, size_type uCount = npos);
   iterator erase(const_iterator itPos);
   iterator erase(const_iterator itFirst, const_iterator itLast);
   void resize(size_type uCount);
   void resize(size_type uCount, char ch);
   void swap(string& o) noexcept;

   // ## String operations ...................................................

   [[nodiscard]] string substr(size_type uPos = 0, size_type uCount = npos) const;
   [[nodiscard]] int compare(const string& str) const noexcept;
   [[nodiscard]] int compare(const char* psz) const noexcept;
   [[nodiscard]] int compare(size_type uPos, size_type uCount, const char* psz, size_type uLength) const noexcept;
   [[nodiscard]] bool starts_with(std::string_view sv) const noexcept;
   [[nodiscard]] bool starts_with(char ch) const noexcept;
   [[nodiscard]] bool starts_with(const char* psz) const noexcept;
   [[nodiscard]] bool ends_with(std::string_view sv) const noexcept;
   [[nodiscard]] bool ends_with(char ch) const noexcept;
   [[nodiscard]] bool ends_with(const char* psz) const noexcept;
   [[nodiscard]] bool contains(std::string_view sv) const noexcept;
   [[nodiscard]] bool contains(char ch) const noexcept;
   [[nodiscard]] bool contains(const char* psz) const noexcept;
   [[nodiscard]] size_type find(const char* psz, size_type uPos = 0) const noexcept;
   [[nodiscard]] size_type find(const char* psz, size_type uPos, size_type uCount) const noexcept;
   [[nodiscard]] size_type find(std::string_view sv, size_type uPos = 0) const noexcept;
   [[nodiscard]] size_type find(char ch, size_type uPos = 0) const noexcept;
   [[nodiscard]] size_type rfind(const char* psz, size_type uPos = npos) const noexcept;
   [[nodiscard]] size_type rfind(const char* psz, size_type uPos, size_type uCount) const noexcept;
   [[nodiscard]] size_type rfind(std::string_view sv, size_type uPos = npos) const noexcept;
   [[nodiscard]] size_type rfind(char ch, size_type uPos = npos) const noexcept;

   // ## Conversion ..........................................................

   [[nodiscard]] std::string_view view() const noexcept;
   [[nodiscard]] operator std::string_view() const noexcept;

   // ## Comparison operators ................................................

   [[nodiscard]] bool operator==(const string& o) const noexcept;
   [[nodiscard]] bool operator==(const char* psz) const noexcept;
   [[nodiscard]] bool operator==(std::string_view sv) const noexcept;
   [[nodiscard]] auto operator<=>(const string& o) const noexcept;
   [[nodiscard]] auto operator<=>(const char* psz) const noexcept;
   [[nodiscard]] auto operator<=>(std::string_view sv) const noexcept;

private:
   // ## Memory ownership tracking using high bit ............................

   [[nodiscard]] bool owner() const noexcept;
   void set_borrow_bit(size_type uCapacity) noexcept;
   void clear_borrow_bit() noexcept;

   // ## Internal helper methods .............................................

   void copy_from(const string& o);
   void allocate(size_type uMinCapacity);
   void destroy() noexcept;

   // ## Member variables ....................................................

   char*     m_pData;                                                         // Pointer to string data (owned or borrowed)
   size_type m_uSize;                                                         // Current size (number of characters, excluding null)
   size_type m_uCapacity;                                                     // Bits 0-62: capacity; Bit 63: borrow flag (1 = borrowed, 0 = owned)
};

// ============================================================================
// ## Constructors and destructor
// ============================================================================

inline string::string() noexcept 
   : m_pData(nullptr), m_uSize(0), m_uCapacity(0) 
{
}

inline string::string(const char* psz) 
   : m_pData(nullptr), m_uSize(0), m_uCapacity(0) 
{
   assert(psz != nullptr);
   size_type uLength = std::strlen(psz);
   allocate(uLength + 1);                                                     // +1 for null terminator
   std::memcpy(m_pData, psz, uLength);
   m_pData[uLength] = '\0';
   m_uSize = uLength;
}

inline string::string(const char* psz, size_type uLength) 
   : m_pData(nullptr), m_uSize(0), m_uCapacity(0) 
{
   assert(psz != nullptr);
   allocate(uLength + 1);                                                     // +1 for null terminator
   std::memcpy(m_pData, psz, uLength);
   m_pData[uLength] = '\0';
   m_uSize = uLength;
}

inline string::string(size_type uCount, char ch) 
   : m_pData(nullptr), m_uSize(0), m_uCapacity(0) 
{
   allocate(uCount + 1);                                                      // +1 for null terminator
   std::memset(m_pData, ch, uCount);
   m_pData[uCount] = '\0';
   m_uSize = uCount;
}

inline string::string(std::string_view sv) 
   : m_pData(nullptr), m_uSize(0), m_uCapacity(0) 
{
   allocate(sv.size() + 1);                                                   // +1 for null terminator
   std::memcpy(m_pData, sv.data(), sv.size());
   m_pData[sv.size()] = '\0';
   m_uSize = sv.size();
}

inline string::string(const string& o) 
   : m_pData(nullptr), m_uSize(0), m_uCapacity(0) 
{
   copy_from(o);
}

inline string::string(string&& o) noexcept 
   : m_pData(o.m_pData), m_uSize(o.m_uSize), m_uCapacity(o.m_uCapacity) 
{
   o.m_pData = nullptr;
   o.m_uSize = 0;
   o.m_uCapacity = 0;
}

inline string::~string() noexcept 
{
   destroy();
}

// ============================================================================
// ## Borrow constructors
// ============================================================================

/** -------------------------------------------------------------------------- Borrow buffer constructor
 * @brief Creates a string that borrows external storage
 * 
 * The string will use the provided buffer without taking ownership. The caller
 * is responsible for ensuring the buffer remains valid for the lifetime of
 * the string or until the string is reassigned/destroyed.
 * 
 * @param pBuffer Pointer to external character buffer
 * @param uCapacity Number of characters the external buffer can hold (including null terminator)
 */
inline string::string(char* pBuffer, size_type uCapacity) noexcept 
   : m_pData(pBuffer), m_uSize(0), m_uCapacity(uCapacity | 0x8000'0000'0000'0000) 
{
   assert(pBuffer != nullptr || uCapacity == 0);                              // verify buffer is valid or capacity is zero
   if( pBuffer && uCapacity > 0 ) {
      pBuffer[0] = '\0';                                                      // ensure empty borrowed string is null-terminated
   }
}

/** -------------------------------------------------------------------------- Borrow from C-array constructor
 * @brief Creates a string that borrows storage from a C-array
 * 
 * @tparam uN Size of the array
 * @param array_ Reference to array to borrow storage from
 */
template<std::size_t uN>
inline string::string(char (&array_)[uN]) noexcept 
   : m_pData(array_), m_uSize(0), m_uCapacity(uN | 0x8000'0000'0000'0000)     // note the borrow bit is set in capacity
{
   if( uN > 0 ) {
      array_[0] = '\0';                                                       // ensure empty borrowed string is null-terminated
   }
}

/** -------------------------------------------------------------------------- Borrow from container constructor
 * @brief Creates a string that borrows storage from a container with .data() and .size()
 * 
 * Works with std::vector<char>, std::array<char, N>, and other containers providing .data() and .size() methods.
 * 
 * @tparam CONTAINER Container type with .data() and .size() methods
 * @param container_ Reference to container to borrow storage from
 */
template<typename CONTAINER>
requires requires(CONTAINER& c_) {
   { c_.data() } -> std::convertible_to<char*>;                               // verify container has .data() returning pointer to char
   { c_.size() } -> std::convertible_to<std::size_t>;                         // verify container has .size() returning size type
}
&& ( !std::is_same_v<std::remove_cvref_t<CONTAINER>, string> )
inline string::string(CONTAINER& container_) noexcept 
   : m_pData(container_.data()), m_uSize(0), m_uCapacity(container_.size() | 0x8000'0000'0000'0000) // note the borrow bit is set in capacity
{
   if( container_.size() > 0 ) {
      container_.data()[0] = '\0';                                            // ensure empty borrowed string is null-terminated
   }
}

// ============================================================================
// ## Assignment operators
// ============================================================================

inline string& string::operator=(const string& o) 
{
   if( this != &o ) {
      destroy();
      copy_from(o);
   }
   return *this;
}

inline string& string::operator=(string&& o) noexcept 
{
   if( this != &o ) {
      destroy();
      m_pData = o.m_pData;
      m_uSize = o.m_uSize;
      m_uCapacity = o.m_uCapacity;
      o.m_pData = nullptr;
      o.m_uSize = 0;
      o.m_uCapacity = 0;
   }
   return *this;
}

inline string& string::operator=(const char* psz) 
{
   assert(psz != nullptr);
   destroy();
   size_type uLength = std::strlen(psz);
   allocate(uLength + 1);                                                     // +1 for null terminator
   std::memcpy(m_pData, psz, uLength);
   m_pData[uLength] = '\0';
   m_uSize = uLength;
   return *this;
}

inline string& string::operator=(std::string_view sv) 
{
   destroy();
   allocate(sv.size() + 1);                                                   // +1 for null terminator
   std::memcpy(m_pData, sv.data(), sv.size());
   m_pData[sv.size()] = '\0';
   m_uSize = sv.size();
   return *this;
}

// ============================================================================
// ## Borrow memory from external source
// ============================================================================

/** -------------------------------------------------------------------------- borrow
 * @brief Borrow memory without taking ownership
 * 
 * Sets the string to reference external memory. The high bit (bit 63) of
 * m_uCapacity is set to indicate borrowed memory. The string will NOT
 * deallocate this memory on destruction.
 * 
 * @param psz Pointer to null-terminated string to borrow
 */
inline void string::borrow(const char* psz) noexcept 
{
   assert(psz != nullptr);
   destroy();
   size_type uLength = std::strlen(psz);
   m_pData = const_cast<char*>(psz);
   m_uSize = uLength;
   set_borrow_bit(uLength);                                                   // set capacity with borrow bit
}

/** -------------------------------------------------------------------------- borrow
 * @brief Borrow memory with explicit length without taking ownership
 * 
 * @param psz Pointer to string data to borrow
 * @param uLength Length of the string (excluding null terminator)
 */
inline void string::borrow(const char* psz, size_type uLength) noexcept 
{
   assert(psz != nullptr);
   destroy();
   m_pData = const_cast<char*>(psz);
   m_uSize = uLength;
   set_borrow_bit(uLength);                                                   // set capacity with borrow bit
}

// ============================================================================
// ## Element access
// ============================================================================

inline string::reference string::operator[](size_type uIndex) noexcept 
{
   assert(uIndex < m_uSize);
   return m_pData[uIndex];
}

inline string::const_reference string::operator[](size_type uIndex) const noexcept 
{
   assert(uIndex < m_uSize);
   return m_pData[uIndex];
}

inline string::reference string::at(size_type uIndex) 
{
   if( uIndex >= m_uSize ) { throw std::out_of_range("string::at"); }
   return m_pData[uIndex];
}

inline string::const_reference string::at(size_type uIndex) const 
{
   if( uIndex >= m_uSize ) { throw std::out_of_range("string::at"); }
   return m_pData[uIndex];
}

inline string::reference string::front() noexcept 
{
   assert(m_uSize > 0);
   return m_pData[0];
}

inline string::const_reference string::front() const noexcept 
{
   assert(m_uSize > 0);
   return m_pData[0];
}

inline string::reference string::back() noexcept 
{
   assert(m_uSize > 0);
   return m_pData[m_uSize - 1];
}

inline string::const_reference string::back() const noexcept 
{
   assert(m_uSize > 0);
   return m_pData[m_uSize - 1];
}

inline char* string::data() noexcept { return m_pData; }

inline const char* string::data() const noexcept { return m_pData; }

inline const char* string::c_str() const noexcept 
{
   return m_pData ? m_pData : "";
}

// ============================================================================
// ## Iterators
// ============================================================================

inline string::iterator string::begin() noexcept { return m_pData; }
inline string::iterator string::end() noexcept { return m_pData + m_uSize; }
inline string::const_iterator string::begin() const noexcept { return m_pData; }
inline string::const_iterator string::end() const noexcept { return m_pData + m_uSize; }
inline string::const_iterator string::cbegin() const noexcept { return m_pData; }
inline string::const_iterator string::cend() const noexcept { return m_pData + m_uSize; }
inline string::reverse_iterator string::rbegin() noexcept { return reverse_iterator(end()); }
inline string::reverse_iterator string::rend() noexcept { return reverse_iterator(begin()); }
inline string::const_reverse_iterator string::rbegin() const noexcept { return const_reverse_iterator(end()); }

inline string::const_reverse_iterator string::rend() const noexcept { return const_reverse_iterator(begin()); }

inline string::const_reverse_iterator string::crbegin() const noexcept { return const_reverse_iterator(end()); }

inline string::const_reverse_iterator string::crend() const noexcept { return const_reverse_iterator(begin()); }

// ============================================================================
// ## Capacity
// ============================================================================

inline bool string::empty() const noexcept { return m_uSize == 0; }

inline string::size_type string::size() const noexcept { return m_uSize; }

inline string::size_type string::length() const noexcept { return m_uSize; }

inline string::size_type string::capacity() const noexcept { return m_uCapacity & 0x7FFF'FFFF'FFFF'FFFF; } // mask out borrow bit

inline bool string::is_borrowed() const noexcept { return !owner(); }

inline void string::reserve(size_type uNewCapacity) { if( uNewCapacity > capacity() ) { allocate(uNewCapacity); } }

inline void string::shrink_to_fit() 
{
   if( owner() && capacity() > m_uSize + 1 ) {
      size_type uNewCapacity = m_uSize + 1;                                   // +1 for null terminator
      char* pNewData = std::allocator<char>().allocate(uNewCapacity);
      std::memcpy(pNewData, m_pData, m_uSize);
      pNewData[m_uSize] = '\0';
      std::allocator<char>().deallocate(m_pData, capacity());
      m_pData = pNewData;
      m_uCapacity = uNewCapacity;
   }
}

// ============================================================================
// ## Modifiers
// ============================================================================

inline void string::clear() noexcept 
{
   if( owner() && m_pData ) {
      m_pData[0] = '\0';
   }
   m_uSize = 0;
}

inline void string::push_back(char ch) 
{
   if( m_uSize + 1 >= capacity() ) { allocate(m_uSize + 2); }                // +1 for char, +1 for null
   m_pData[m_uSize] = ch;
   m_pData[m_uSize + 1] = '\0';
   ++m_uSize;
}

inline void string::pop_back() noexcept 
{
   assert(m_uSize > 0);
   --m_uSize;
   if( owner() && m_pData ) {
      m_pData[m_uSize] = '\0';
   }
}

inline string& string::append(const char* psz) 
{
   assert(psz != nullptr);
   size_type uLength = std::strlen(psz);
   return append(psz, uLength);
}

inline string& string::append(const char* psz, size_type uLength) 
{
   assert(psz != nullptr);
   if( uLength == 0 ) { return *this; }
   size_type uNewSize = m_uSize + uLength;
   if( uNewSize + 1 > capacity() ) { allocate(uNewSize + 1); }                // +1 for null terminator
   std::memcpy(m_pData + m_uSize, psz, uLength);
   m_uSize = uNewSize;
   m_pData[m_uSize] = '\0';
   return *this;
}

inline string& string::append(const string& str) 
{
   return append(str.data(), str.size());
}

inline string& string::append(std::string_view sv) 
{
   return append(sv.data(), sv.size());
}

inline string& string::append(size_type uCount, char ch) 
{
   if( uCount == 0 ) { return *this; }
   size_type uNewSize = m_uSize + uCount;
   if( uNewSize + 1 > capacity() ) { allocate(uNewSize + 1); }                // +1 for null terminator
   std::memset(m_pData + m_uSize, ch, uCount);
   m_uSize = uNewSize;
   m_pData[m_uSize] = '\0';
   return *this;
}

inline string& string::operator+=(char ch) 
{
   push_back(ch);
   return *this;
}

inline string& string::operator+=(const char* psz) 
{
   return append(psz);
}

inline string& string::operator+=(const string& str) 
{
   return append(str);
}

inline string& string::operator+=(std::string_view sv) 
{
   return append(sv);
}

inline string::iterator string::insert(size_type uPos, size_type uCount, char ch) 
{
   assert(uPos <= m_uSize);
   if( uCount == 0 ) { return begin() + uPos; }
   size_type uNewSize = m_uSize + uCount;
   if( uNewSize + 1 > capacity() ) { allocate(uNewSize + 1); }                // +1 for null terminator
   
   // ## Shift characters to make room .......................................
   std::memmove(m_pData + uPos + uCount, m_pData + uPos, m_uSize - uPos);
   std::memset(m_pData + uPos, ch, uCount);
   m_uSize = uNewSize;
   m_pData[m_uSize] = '\0';
   return begin() + uPos;
}

inline string::iterator string::insert(size_type uPos, const char* psz) 
{
   assert(psz != nullptr);
   assert(uPos <= m_uSize);
   return insert(uPos, psz, std::strlen(psz));
}

inline string::iterator string::insert(size_type uPos, const char* psz, size_type uLength) 
{
   assert(psz != nullptr);
   assert(uPos <= m_uSize);
   if( uLength == 0 ) { return begin() + uPos; }
   size_type uNewSize = m_uSize + uLength;
   if( uNewSize + 1 > capacity() ) { allocate(uNewSize + 1); }                // +1 for null terminator
   
   // ## Shift characters to make room .......................................
   std::memmove(m_pData + uPos + uLength, m_pData + uPos, m_uSize - uPos);
   std::memcpy(m_pData + uPos, psz, uLength);
   m_uSize = uNewSize;
   m_pData[m_uSize] = '\0';
   return begin() + uPos;
}

inline string& string::erase(size_type uPos, size_type uCount) 
{
   assert(uPos <= m_uSize);
   if( uCount == npos || uPos + uCount > m_uSize ) {
      uCount = m_uSize - uPos;
   }
   if( uCount == 0 ) { return *this; }
   
   // ## Shift characters ...................................................
   std::memmove(m_pData + uPos, m_pData + uPos + uCount, m_uSize - uPos - uCount);
   m_uSize -= uCount;
   if( owner() && m_pData ) {
      m_pData[m_uSize] = '\0';
   }
   return *this;
}

inline string::iterator string::erase(const_iterator itPos) 
{
   size_type uPos = itPos - begin();
   assert(uPos < m_uSize);
   erase(uPos, 1);
   return begin() + uPos;
}

inline string::iterator string::erase(const_iterator itFirst, const_iterator itLast) 
{
   size_type uPos = itFirst - begin();
   size_type uCount = itLast - itFirst;
   assert(uPos <= m_uSize && uPos + uCount <= m_uSize);
   erase(uPos, uCount);
   return begin() + uPos;
}

inline void string::resize(size_type uCount) 
{
   resize(uCount, '\0');
}

inline void string::resize(size_type uCount, char ch) 
{
   if( uCount > m_uSize ) {
      append(uCount - m_uSize, ch);
   }
   else if( uCount < m_uSize ) {
      m_uSize = uCount;
      if( owner() && m_pData ) {
         m_pData[m_uSize] = '\0';
      }
   }
}

inline void string::swap(string& o) noexcept 
{
   std::swap(m_pData, o.m_pData);
   std::swap(m_uSize, o.m_uSize);
   std::swap(m_uCapacity, o.m_uCapacity);
}

// ============================================================================
// ## String operations
// ============================================================================

inline string string::substr(size_type uPos, size_type uCount) const 
{
   if( uPos > m_uSize ) { throw std::out_of_range("string::substr"); }
   if( uCount == npos || uPos + uCount > m_uSize ) {
      uCount = m_uSize - uPos;
   }
   return string(data() + uPos, uCount);  // data() returns const char* in const context
}

inline int string::compare(const string& str) const noexcept 
{
   return compare(0, m_uSize, str.data(), str.size());
}

inline int string::compare(const char* psz) const noexcept 
{
   assert(psz != nullptr);
   return compare(0, m_uSize, psz, std::strlen(psz));
}

inline int string::compare(size_type uPos, size_type uCount, const char* psz, size_type uLength) const noexcept 
{
   assert(uPos <= m_uSize);
   if( uPos + uCount > m_uSize ) { uCount = m_uSize - uPos; }
   size_type uMinLen = std::min(uCount, uLength);
   int iResult = std::memcmp(m_pData + uPos, psz, uMinLen);
   if( iResult == 0 ) {
      if( uCount < uLength ) { return -1; }
      if( uCount > uLength ) { return 1; }
   }
   return iResult;
}

inline bool string::starts_with(std::string_view sv) const noexcept 
{
   return m_uSize >= sv.size() && std::memcmp(m_pData, sv.data(), sv.size()) == 0;
}

inline bool string::starts_with(char ch) const noexcept 
{
   return m_uSize > 0 && m_pData[0] == ch;
}

inline bool string::starts_with(const char* psz) const noexcept 
{
   assert(psz != nullptr);
   return starts_with(std::string_view(psz));
}

inline bool string::ends_with(std::string_view sv) const noexcept 
{
   return m_uSize >= sv.size() && std::memcmp(m_pData + m_uSize - sv.size(), sv.data(), sv.size()) == 0;
}

inline bool string::ends_with(char ch) const noexcept 
{
   return m_uSize > 0 && m_pData[m_uSize - 1] == ch;
}

inline bool string::ends_with(const char* psz) const noexcept 
{
   assert(psz != nullptr);
   return ends_with(std::string_view(psz));
}

inline bool string::contains(std::string_view sv) const noexcept 
{
   return find(sv) != npos;
}

inline bool string::contains(char ch) const noexcept 
{
   return find(ch) != npos;
}

inline bool string::contains(const char* psz) const noexcept 
{
   assert(psz != nullptr);
   return find(psz) != npos;
}

inline string::size_type string::find(const char* psz, size_type uPos) const noexcept 
{
   assert(psz != nullptr);
   return find(psz, uPos, std::strlen(psz));
}

inline string::size_type string::find(const char* psz, size_type uPos, size_type uCount) const noexcept 
{
   if( uCount == 0 ) { return uPos <= m_uSize ? uPos : npos; }
   if( uPos + uCount > m_uSize ) { return npos; }
   
   const char* pResult = std::search(m_pData + uPos, m_pData + m_uSize, psz, psz + uCount);
   return pResult != m_pData + m_uSize ? pResult - m_pData : npos;
}

inline string::size_type string::find(std::string_view sv, size_type uPos) const noexcept 
{
   return find(sv.data(), uPos, sv.size());
}

inline string::size_type string::find(char ch, size_type uPos) const noexcept 
{
   if( uPos >= m_uSize ) { return npos; }
   const char* pResult = static_cast<const char*>(std::memchr(m_pData + uPos, ch, m_uSize - uPos));
   return pResult ? pResult - m_pData : npos;
}

inline string::size_type string::rfind(const char* psz, size_type uPos) const noexcept 
{
   assert(psz != nullptr);
   return rfind(psz, uPos, std::strlen(psz));
}

inline string::size_type string::rfind(const char* psz, size_type uPos, size_type uCount) const noexcept 
{
   if( uCount == 0 ) { return std::min(uPos, m_uSize); }
   if( uCount > m_uSize ) { return npos; }
   
   uPos = std::min(uPos, m_uSize - uCount);
   for( size_type i = uPos + 1; i-- > 0; ) {
      if( std::memcmp(m_pData + i, psz, uCount) == 0 ) {
         return i;
      }
   }
   return npos;
}

inline string::size_type string::rfind(std::string_view sv, size_type uPos) const noexcept 
{
   return rfind(sv.data(), uPos, sv.size());
}

inline string::size_type string::rfind(char ch, size_type uPos) const noexcept 
{
   if( m_uSize == 0 ) { return npos; }
   uPos = std::min(uPos, m_uSize - 1);
   for( size_type i = uPos + 1; i-- > 0; ) {
      if( m_pData[i] == ch ) { return i; }
   }
   return npos;
}

// ============================================================================
// ## Conversion
// ============================================================================

inline std::string_view string::view() const noexcept { return std::string_view(m_pData, m_uSize); }

inline string::operator std::string_view() const noexcept { return view(); }

// ============================================================================
// ## Comparison operators
// ============================================================================

inline bool string::operator==(const string& o) const noexcept { return m_uSize == o.m_uSize && (m_pData == o.m_pData || std::memcmp(m_pData, o.m_pData, m_uSize) == 0); }

inline bool string::operator==(const char* psz) const noexcept 
{
   assert(psz != nullptr);
   return compare(psz) == 0;
}

inline bool string::operator==(std::string_view sv) const noexcept { return m_uSize == sv.size() && std::memcmp(m_pData, sv.data(), m_uSize) == 0; }

inline auto string::operator<=>(const string& o) const noexcept 
{
   int iResult = compare(o);
   return iResult <=> 0;
}

inline auto string::operator<=>(const char* psz) const noexcept 
{
   assert(psz != nullptr);
   int iResult = compare(psz);
   return iResult <=> 0;
}

inline auto string::operator<=>(std::string_view sv) const noexcept 
{
   int iResult = compare(0, m_uSize, sv.data(), sv.size());
   return iResult <=> 0;
}

// ============================================================================
// ## Private helper methods
// ============================================================================

/** -------------------------------------------------------------------------- owner
 * @brief Check if we own the memory
 * 
 * Returns true if bit 63 of m_uCapacity is NOT set (we own the memory).
 * Returns false if bit 63 is set (we're borrowing the memory).
 */
inline bool string::owner() const noexcept { return (m_uCapacity & 0x8000'0000'0000'0000) == 0; }

/** -------------------------------------------------------------------------- set_borrow_bit
 * @brief Set the borrow bit to indicate borrowed memory
 * 
 * Sets bit 63 of m_uCapacity while preserving the capacity value in bits 0-62.
 * After this call, owner() will return false.
 */
inline void string::set_borrow_bit(size_type uCapacity) noexcept 
{
   m_uCapacity = uCapacity | 0x8000'0000'0000'0000;
}

/** -------------------------------------------------------------------------- clear_borrow_bit
 * @brief Clear the borrow bit to indicate owned memory
 * 
 * Clears bit 63 of m_uCapacity while preserving the capacity value in bits 0-62.
 * After this call, owner() will return true.
 */
inline void string::clear_borrow_bit() noexcept 
{
   m_uCapacity = m_uCapacity & 0x7FFF'FFFF'FFFF'FFFF;
}

/** -------------------------------------------------------------------------- copy_from
 * @brief Internal helper to copy from another string
 * 
 * Creates an owned copy of the source string's data, even if the source
 * is borrowing its memory.
 */
inline void string::copy_from(const string& o) 
{
   if( o.m_uSize > 0 ) {
      allocate(o.m_uSize + 1);                                                // +1 for null terminator
      std::memcpy(m_pData, o.m_pData, o.m_uSize);
      m_pData[o.m_uSize] = '\0';
      m_uSize = o.m_uSize;
   }
}

/** -------------------------------------------------------------------------- allocate
 * @brief Allocate new owned storage and transfer ownership
 * 
 * This method allocates new heap storage, copies existing data to it,
 * and takes ownership. If storage was borrowed, the borrow bit is cleared.
 * 
 * @param uMinCapacity Minimum capacity required (including null terminator)
 */
inline void string::allocate(size_type uMinCapacity) 
{
   size_type uOldCapacity = capacity();
   size_type uNewCapacity = std::max(uOldCapacity * 2, uMinCapacity);         // standard growth factor of 2
   
   char* pNewData = std::allocator<char>().allocate(uNewCapacity);
   
   // ## Copy existing data if any ............................................
   if( m_pData && m_uSize > 0 ) {
      std::memcpy(pNewData, m_pData, m_uSize);
      pNewData[m_uSize] = '\0';
   }
   
   // ## Deallocate old buffer only if we owned it ...........................
   if( owner() && m_pData ) {
      std::allocator<char>().deallocate(m_pData, uOldCapacity);
   }
   
   m_pData = pNewData;
   m_uCapacity = uNewCapacity;                                                // clear borrow bit by setting without it
}

/** -------------------------------------------------------------------------- destroy
 * @brief Destroy string data and deallocate owned storage
 */
inline void string::destroy() noexcept 
{
   if( owner() && m_pData ) {
      std::allocator<char>().deallocate(m_pData, capacity());
   }
   m_pData = nullptr;
   m_uSize = 0;
   m_uCapacity = 0;
}

// ============================================================================
// ## Non-member functions
// ============================================================================

/// Concatenation operator
[[nodiscard]] inline string operator+(const string& lhs, const string& rhs) 
{
   string result;
   result.reserve(lhs.size() + rhs.size() + 1);
   result.append(lhs);
   result.append(rhs);
   return result;
}

[[nodiscard]] inline string operator+(const string& lhs, const char* rhs) 
{
   string result;
   size_t uRhsLen = std::strlen(rhs);
   result.reserve(lhs.size() + uRhsLen + 1);
   result.append(lhs);
   result.append(rhs, uRhsLen);
   return result;
}

[[nodiscard]] inline string operator+(const char* lhs, const string& rhs) 
{
   string result;
   size_t uLhsLen = std::strlen(lhs);
   result.reserve(uLhsLen + rhs.size() + 1);
   result.append(lhs, uLhsLen);
   result.append(rhs);
   return result;
}

[[nodiscard]] inline string operator+(const string& lhs, char rhs) 
{
   string result;
   result.reserve(lhs.size() + 2);
   result.append(lhs);
   result.push_back(rhs);
   return result;
}

[[nodiscard]] inline string operator+(char lhs, const string& rhs) 
{
   string result;
   result.reserve(rhs.size() + 2);
   result.push_back(lhs);
   result.append(rhs);
   return result;
}

/// std::swap specialization
inline void swap(string& lhs, string& rhs) noexcept 
{
   lhs.swap(rhs);
}

} // namespace borrow

_GD_END

#endif // GD_COMPILER_HAS_CPP20_SUPPORT
