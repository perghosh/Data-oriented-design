#pragma once

#include <cassert>
#include <cstdint>                                                            // Standard fixed-width integer types  
#include <cstring>                                                            // For memcpy, memmove, memset  
#include <iterator>                                                           // For std::iterator  
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "gd_variant.h"
#include "gd_variant_view.h"
#include "gd_debug.h"
#include "gd_types.h"


#ifndef _GD_BEGIN
#  define _GD_BEGIN namespace gd {
#  define _GD_END } 
_GD_BEGIN
#else
_GD_BEGIN
#endif

namespace strings {

   /**
    * @brief A forward iterator for navigating through strings stored in a custom container.
    * 
    * This iterator is designed to work with a container (`STRINGS`) that manages string data. 
    * It allows sequential access to the strings in the container, moving forward through 
    * the internas STRINGS storage.
    * 
    * @tparam STRINGS The type of the container managing the strings, which must provide 
    *                 methods like `advance`, `to_string_view_s`, and `get_position`.
    */
   template <typename STRINGS>
   class iterator
   {  
   public:
      using value_type = std::string_view;  
      using iterator_category = std::forward_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using pointer = const std::string_view*;
      using reference = const std::string_view&;

      iterator() : m_pstrings(nullptr), m_uOffset(0) {}

      /// Initializes the iterator with a current pointer and an end pointer for the buffer.  
      iterator( STRINGS* pstrings, uint64_t uOffset ) : m_uOffset(uOffset), m_pstrings(pstrings) {}  

      /// Copies the iterator from another iterator instance.  
      iterator(const iterator& o) : m_uOffset(o.m_uOffset), m_pstrings(o.m_pstrings) {}  
      iterator(iterator&& o) noexcept : m_uOffset(o.m_uOffset), m_pstrings(o.m_pstrings) { o.m_pstrings = nullptr; o.m_uOffset = 0; }  

      /// Assigns the iterator state from another iterator instance.  
      iterator& operator=(const iterator& o) {  
         m_pstrings = o.m_pstrings; m_uOffset = o.m_uOffset; return *this;  
      }  

      bool operator==(const iterator& o) const { assert( m_pstrings == o.m_pstrings ); return m_uOffset == o.m_uOffset; }  
      bool operator!=(const iterator& o) const { return !(o == *this); }

      uint64_t offset() const { return m_uOffset; }
      STRINGS* get() { return m_pstrings; }
      const STRINGS* get() const { return m_pstrings; }

      /// get std::string_view from active position for iterator
      std::string_view operator*() const { return as_string_view(); }  

      std::string_view as_string_view() const { return STRINGS::to_string_view_s(m_pstrings->buffer(), m_uOffset); }
      std::string as_string() const { return STRINGS::to_string_s(m_pstrings->buffer(), m_uOffset); }

      /// Advances the iterator to the next string block.  
      iterator& operator++()  
      {
         auto uOffset = m_pstrings->advance( m_uOffset );
         m_uOffset = uOffset;
         return *this;                                                        // Return the updated iterator  
      }  

      /// move to next string
      iterator operator++(int)  
      {  
         iterator it = *this; 
         ++(*this);
         return it;
      }  

      /// Compound assignment operator to advance the iterator by a specified number of string blocks.
      iterator& operator+=(size_t uCount)
      {
         for(size_t i = 0; i < uCount; ++i) {
            ++(*this); // Use existing increment operator
         }
         return *this; // Return a reference to this iterator
      }

      /// Advance the iterator by a specified number of string blocks.
      iterator operator+(size_t uCount) const
      {
         iterator it = *this;
         for( size_t i = 0; i < uCount; ++i ) ++it;
         return it;
      }

      uint64_t m_uOffset;  ///< Offset of the current string in the buffer
      STRINGS* m_pstrings; ///< Pointer to the strings container
   };  
}



/**
 * @class strings32
 * @brief A custom string container class that manages a buffer of strings, where each string's length is stored as a 32-bit unsigned integer (uint32_t).
 * 
 * This class provides:
 * - Efficient storage and manipulation of strings in a contiguous memory block.
 * - Support for dynamic buffer sizing, appending, erasing, and replacing strings.
 * - Custom iterators for traversing the string collection.
 * - Methods for accessing buffer positions and converting buffer data back into string views or C-style strings.
 * 
 * Key features include:
 * - Memory management for the string buffer.
 * - Constructor overloads for different initialization methods.
 * - Overloaded operators for string concatenation and indexed access.
 * - Buffer management functions to reserve space and clear content.
 * 
 * Note: 
 * - The class assumes that each string's length precedes its content in the buffer.
 * - Manual memory handling is employed, which requires careful buffer management to prevent leaks or overflows.
 * - This class is not thread-safe by default; consider thread safety for multi-threaded applications.
 */
class strings32  
{  
public:
   using iterator               = strings::iterator<strings32>;
   using const_iterator         = strings::iterator<const strings32>;

// ## construction -------------------------------------------------------------
public:
   strings32() : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) {}
   explicit strings32( const char* pisz_ ) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append( std::string_view( pisz_ ) ); }
   explicit strings32( const std::string_view& string_ ) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append( string_ ); }
   explicit strings32( const std::string& string_ ) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append( std::string_view( string_ ) ); }
   strings32( const std::initializer_list<std::string_view>& listString ) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append( listString ); }
   template<typename CONTAINER>
   strings32(const CONTAINER& container) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append(container); }
   // copy
   strings32(const strings32& o) { common_construct(o); }
   strings32(strings32&& o) noexcept { common_construct(std::move(o)); }
   // assign
   strings32& operator=(const strings32& o) { common_construct(o); return *this; }
   strings32& operator=(strings32&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~strings32()  {  delete[] m_puBuffer;  }  

private:
   // common copy
   void common_construct(const strings32& o);
   void common_construct(strings32&& o) noexcept;

// ## operator -----------------------------------------------------------------
public:
   /// Provides indexed access to the stored strings.  
   std::string_view operator[](size_t uIndex) const;
   
   // ## += operator to add strings to the buffer


   strings32& operator+=(const std::string_view& stringAppend) { append(stringAppend); return *this; }
   strings32& operator+=(const gd::strings32& strings_) { append(strings_, gd::types::tag_internal{}); return *this; }
   template<typename VALUE>
   strings32& operator+=(const VALUE& value_) { append_any(value_); return *this; }

   /// appends different types that is convertible to string to the buffer.
   template<typename VALUE>
   strings32& operator<<(const VALUE& value_) { append_any(value_); return *this; }

// ## iterator -----------------------------------------------------------------
public:

   iterator begin() { return strings::iterator<strings32>(this, 0); }
   iterator end() { return strings::iterator<strings32>(this, m_uSize); }
   const_iterator begin() const { return const_iterator(this, 0); }
   const_iterator end() const { return const_iterator(this, m_uSize); }
   const_iterator cbegin() const { return const_iterator(this, 0); }
   const_iterator cend() const { return const_iterator(this, m_uSize); }

/** \name GET/SET
*///@{
   /// get position in buffer for offset position
   uint8_t* get_position(uint64_t uOffset);
   const uint8_t* get_position(uint64_t uOffset) const;

   /// ## get std::string_view from buffer at offset position
   std::string_view at(uint64_t uOffset) const;
   std::string_view at(iterator itPosition) const { return at(itPosition.offset()); }
   std::string_view at(const_iterator itPosition) const { return at(itPosition.offset()); }
//@}

/** \name OPERATION
*///@{
   /// appends a new string to the buffer
   void append(const std::string_view& stringAppend);
   strings32& append(const std::initializer_list<std::string_view>& listString);
   strings32& append(const std::vector<std::string>& vectorString);
   strings32& append(const std::vector<std::string_view>& vectorString);
   strings32& append_any(const std::initializer_list<gd::variant_view>& listValue);
   strings32& append_any(const std::vector<gd::variant_view>& vectorValue);

   template<typename TYPE, typename ALLOC, typename = std::enable_if_t<gd::types::is_vector<std::vector<TYPE, ALLOC>>::value>>
   strings32& append(const std::vector<TYPE, ALLOC>& vector_) { for( const auto& it : vector_ ) { append( it ); } return *this; }

   template<typename TYPE, typename ALLOC, typename = std::enable_if_t<gd::types::is_list<std::list<TYPE, ALLOC>>::value>>
   strings32& append(const std::list<TYPE, ALLOC>& list_) { for( const auto& it : list_ ) { append( it ); } return *this; }

   /// Appends types that is convertible to string to the buffer.
   strings32& append(const gd::variant_view& variant_view, gd::types::tag_view ) { append(variant_view.as_string()); return *this; }

   // ## Append special values
   strings32& append(const gd::strings32& strings_, gd::types::tag_internal);
   strings32& append(const std::nullptr_t, gd::types::tag_internal) { append(std::string_view("")); return *this; }

   /// Appends any number of strings to the buffer using variadic templates.
   template<typename... ARGUMENTS>
   strings32& add(ARGUMENTS&&... args);
   template<typename VALUE>
   strings32& append_any(const VALUE& value);


   /// Erases the string at the specified iterator position from the buffer.
   iterator erase( const iterator& it );
   /// Erases the string at the specified iterator position from the buffer.
   const_iterator erase( const const_iterator& it );

   /// Replaces the string at the specified iterator position with a new string.  
   void replace( uint8_t* puPosition, const std::string_view& stringReplace);
   void replace(iterator itPosition, const std::string_view& stringReplace) { replace(buffer() + itPosition.offset(), stringReplace); }
   void replace(const_iterator itPosition, const std::string_view& stringReplace) { replace(buffer() + itPosition.offset(), stringReplace); }


   /// Returns the number of strings stored in the buffer.  
   size_t count() const;
   /// Returns the number of strings stored in the buffer.  
   size_t size() const { return count(); }

   /// Removes all strings from the buffer by resetting the used size to zero.  
   void clear() { m_uSize = 0; }
   /// Reserve additional space in the buffer.
   void reserve_add(uint64_t uAdd) { reserve(m_uSize + uAdd); }
   /// Reserve space in the buffer.
   void reserve( uint64_t uSize );

   /// Advance offset in buffer and return offset to next string
   uint64_t advance(uint64_t uOffset) const;

   /// Searches for a specific string within a buffer. Returnes true if string exists.
   bool exists(const std::string_view& stringFind) const;

   /// ## find string in buffer

   iterator find( const std::string_view& stringFind );
   iterator find( const std::string_view& stringFind, const iterator& itOffset );
   iterator find( const std::string_view& stringFind, const iterator& itBegin, const iterator& itEnd );
   const_iterator find( const std::string_view& stringFind ) const;
   const_iterator find( const std::string_view& stringFind, const const_iterator& itOffset ) const;
   const_iterator find( const std::string_view& stringFind, const const_iterator& itBegin, const const_iterator& itEnd ) const;

   // const uint8_t* find( const std::string_view& stringFind, uint64_t uOffset, uint64_t uSize );

   // ## join internal string values to one string
   // 
   std::string join() const { return join_s(begin(), end(), ""); }
   std::string join( const std::string_view& stringSeparator ) const { return join_s(begin(), end(), stringSeparator); }
//@}

/** \name BUFFER
*///@{
   uint8_t* buffer() { return m_puBuffer; }
   const uint8_t* buffer() const { return m_puBuffer; }
   uint8_t* buffer_end() { return m_puBuffer + m_uSize; }
   const uint8_t* buffer_end() const { return m_puBuffer + m_uSize; }
   uint64_t buffer_size() const { return m_uSize; }
   uint64_t buffer_capacity() const { return m_uBufferSize; } 
//@}

// ## attributes ----------------------------------------------------------------
public:
   uint8_t* m_puBuffer;    // m_puBuffer: Pointer to the buffer where strings are stored  
   uint64_t m_uSize;       // m_uSize: Number of bytes currently used in the buffer  
   uint64_t m_uBufferSize; // m_uBufferSize: Total allocated size of the buffer in bytes  


// ## free functions ------------------------------------------------------------
public:
   /// get const char* pointer from buffer at offset position
   static const char* c_str_s(const uint8_t* puBuffer, uint64_t uPosition) { return reinterpret_cast<const char*>( puBuffer + sizeof(uint32_t) ); }
   /// return const char*, note that string values within buffer are NOT null terminated
   static const char* c_str_s(const uint8_t* puPosition) { return reinterpret_cast<const char*>( puPosition + sizeof(uint32_t) ); }
   /// get length of string from buffer at offset position
   static uint32_t length_s(const uint8_t* puPosition)  { return *reinterpret_cast<const uint32_t*>( puPosition ); }
   /// move to next string
   static const uint8_t* next_s( const uint8_t* puPosition );
   /// get std::string_view from buffer at offset position
   static std::string_view to_string_view_s(const uint8_t* puBuffer, uint64_t uPosition);
   /// get std::string from buffer at offset position
   static std::string to_string_s(const uint8_t* puBuffer, uint64_t uPosition);

   static const uint8_t* find_s( const uint8_t* puBuffer, const std::string_view& stringFind, uint64_t uOffset, uint64_t uSize );

   /// join strings with a separator using iterator range
   template<typename ITERATOR>
   static std::string join_s(ITERATOR itBegin, ITERATOR itEnd, const std::string_view& stringSeparator);

   template<typename ITERATOR>
   static std::string join_s(ITERATOR itBegin, ITERATOR itEnd, const std::string_view& stringSeparator, std::function< bool(std::string&, const std::string_view&, unsigned)> callback_ );

};  

/// copy string32
inline void strings32::common_construct(const strings32& o) {
   m_uSize = o.m_uSize;
   m_uBufferSize = o.m_uBufferSize;
   m_puBuffer = new uint8_t[m_uBufferSize];
   memcpy(m_puBuffer, o.m_puBuffer, m_uSize);
}

/// move string32
inline void strings32::common_construct(strings32&& o) noexcept {
   m_uSize = o.m_uSize; m_uBufferSize = o.m_uBufferSize; m_puBuffer = o.m_puBuffer;
   o.m_puBuffer = nullptr; o.m_uSize = 0; o.m_uBufferSize = 0;
}

/// Get string at index, this is slow because it need to jump through all strings
inline std::string_view strings32::operator[](size_t uIndex) const 
{  
   auto it = begin();
   for( size_t i = 0; i < uIndex; ++i ) ++it;
   return it.as_string_view();
}  


/// Appends strings to internal buffer.
inline strings32& strings32::append(const std::initializer_list<std::string_view>& listString)
{
   for(const auto& string_ : listString) { append(string_);  }
   return *this;
}

/// Appends strings to internal buffer.
inline strings32& strings32::append(const std::vector<std::string>& vectorString)
{
   for(const auto& string_ : vectorString) { append(string_);  }
   return *this;
}

/// Appends strings to internal buffer.
inline strings32& strings32::append(const std::vector<std::string_view>& vectorString)
{
   for(const auto& string_ : vectorString) { append(string_);  }
   return *this;
}

/// Appends values from initializer list to internal buffer.
inline strings32& strings32::append_any(const std::initializer_list<gd::variant_view>& listValue)
{
   for( const auto& v_ : listValue ) { append(v_, gd::types::tag_view{}); }
   return *this;
}

/// Appends values from vector to internal buffer.
inline strings32& strings32::append_any(const std::vector<gd::variant_view>& vectorValue)
{
   for( const auto& v_ : vectorValue ) { append(v_, gd::types::tag_view{}); }
   return *this;
}

template<typename... ARGUMENTS>
strings32& strings32::add(ARGUMENTS&&... arguments_) {
   (append_any(std::forward<ARGUMENTS>(arguments_)), ...);
   return *this;
}

/// Append and convert any type that is convertible to some of the supported types
template<typename VALUE>
strings32& strings32::append_any(const VALUE& value) {
   if constexpr ( std::is_same<VALUE, std::string_view>::value ) append(value);
   else if constexpr ( std::is_same<VALUE, std::string>::value ) append(value);
   else if constexpr ( std::is_same<VALUE, const char*>::value ) append(value);
   else if constexpr ( std::is_convertible<VALUE, gd::variant_view>::value ) append(value, gd::types::tag_view{});
   else if constexpr ( std::is_convertible<VALUE, gd::strings32>::value ) append(value, gd::types::tag_internal{});
   else static_assert(gdd::always_false<VALUE>::value, "unsupported type");
   return *this;
}


/// Return position in internal buffer for offset
inline uint8_t* strings32::get_position(uint64_t uOffset) {                                        assert( uOffset < m_uSize ); assert( (uOffset % sizeof( uint32_t )) == 0 );
   return m_puBuffer + uOffset;
}
/// Return position in internal buffer for offset
inline const uint8_t* strings32::get_position(uint64_t uOffset) const {                            assert( uOffset < m_uSize ); assert( (uOffset % sizeof( uint32_t )) == 0 );
   return m_puBuffer + uOffset;
}

/// get std::string_view from buffer at offset position
inline std::string_view strings32::to_string_view_s(const uint8_t* puBuffer, uint64_t uPosition) 
{
   uint32_t uLength = *reinterpret_cast<const uint32_t*>( puBuffer + uPosition );
   const char* pi_ = reinterpret_cast<const char*>( puBuffer + uPosition + sizeof(uint32_t) );
   return std::string_view( pi_, uLength );
}

/// get std::string from buffer at offset position
inline std::string strings32::to_string_s(const uint8_t* puBuffer, uint64_t uPosition) 
{
   uint32_t uLength = *reinterpret_cast<const uint32_t*>( puBuffer + uPosition );
   const char* pi_ = reinterpret_cast<const char*>( puBuffer + uPosition + sizeof(uint32_t) );
   return std::string( pi_, uLength );
}



/// join strings with a separator using iterator range
template<typename ITERATOR>
std::string strings32::join_s(ITERATOR itBegin, ITERATOR itEnd, const std::string_view& stringSeparator)
{
   std::string stringResult;
   stringResult.reserve(64);                                                   // Reserve space (cache line) for the result string

   // ## Append the first string, this to avoid if statement in loop
   ITERATOR it = itBegin;
   if( it != itEnd ) stringResult.append( it.as_string_view() );
   it++;

   for( ; it != itEnd; ++it ) 
   {
      stringResult.append(stringSeparator.data(), stringSeparator.size());
      auto string_ = it.as_string_view();                                      // Get the string at the current iterator position (compiler will optimize this, easier for debug)
      stringResult.append(string_.data(), string_.size());
   }
   return stringResult;
}

/**
 * Joins a range of strings using a specified separator, with an optional callback function.
 * 
 * @tparam ITERATOR The iterator type for the range of strings to join
 * @param itBegin Iterator pointing to the start of the range
 * @param itEnd Iterator pointing to the end of the range
 * @param stringSeparator The separator to insert between strings
 * @param callback_ Optional callback function that can modify the result.
 *        Takes the current result string and next string value as parameters.
 *        Returns false to append the string normally, true to skip appending.
 *        Default is an empty function that always returns false.
 * @return The joined string containing all strings from the range,
 *         separated by the specified separator
 */
template<typename ITERATOR>
std::string strings32::join_s(ITERATOR itBegin, ITERATOR itEnd, const std::string_view& stringSeparator, std::function< bool(std::string& stringResult, const std::string_view& stringValue, unsigned uIndex)> callback_ )
{
   unsigned uIndex = 0;      // Index of the current string in the range
   std::string stringResult; // Result string to store the joined strings
   stringResult.reserve(64);                                                   // Reserve space (cache line) for the result string

   // ## Append the first string, this to avoid if statement in loop
   ITERATOR it = itBegin;
   if( it != itEnd )
   {
      auto s_ = it.as_string_view();
      if( callback_(stringResult, s_, uIndex) == false ) stringResult.append( s_ );
   }
   it++;
   uIndex++;

   for( ; it != itEnd; ++it, ++uIndex ) 
   {
      stringResult.append(stringSeparator.data(), stringSeparator.size());
      auto string_ = it.as_string_view();                                      // Get the string at the current iterator position (compiler will optimize this, easier for debug)
      if( callback_(stringResult, string_, uIndex) == false ) stringResult.append( string_ );
   }
   return stringResult;
}

/**
 * @brief Converts a custom string container to different types at compile time.
 * @tparam TYPE The type to convert to; supported types include std::string, std::vector<std::string_view>, 
 *              std::vector<std::string>, std::list<std::string_view>, and std::list<std::string>.
 * @param strings_ The input container of type strings32 to be converted.
 * @return TYPE An instance of TYPE filled with elements from strings_.
 * ```cpp
 * strings32 strings32Test; // Assume this is initialized with some string data
 * 
 * // Convert to std::string
 * std::string result1 = get<std::string>(strings32Test);
 * 
 * // Convert to std::vector<std::string_view>
 * std::vector<std::string_view> result2 = get<std::vector<std::string_view>>(strings32Test);
 * 
 * // Convert to std::list<std::string>
 * std::list<std::string> result3 = get<std::list<std::string>>(strings32Test);
 * ``` 
 */
template<typename TYPE> 
TYPE get( const strings32& strings_  ) 
{
   if constexpr ( std::is_same<TYPE, std::string>::value ) 
   { 
      return strings_.join(); 
   }
   else if constexpr ( std::is_same<TYPE, std::vector<std::string_view>>::value ) 
   {
      std::vector<std::string_view> vector_;
      for( const auto& it : strings_ ) { vector_.push_back(it); }
      return vector_;
   }
   else if constexpr ( std::is_same<TYPE, std::vector<std::string>>::value ) 
   {
      std::vector<std::string> vector_;
      for( const auto& it : strings_ ) { vector_.push_back(std::string(it)); }
      return vector_;
   }
   else if constexpr ( std::is_same<TYPE, std::list<std::string_view>>::value ) 
   {
      std::list<std::string_view> list_;
      for( const auto& it : strings_ ) { list_.push_back(it); }
      return list_;
   }
   else if constexpr ( std::is_same<TYPE, std::list<std::string>>::value ) 
   {
      std::list<std::string> list_;
      for( const auto& it : strings_ ) { list_.push_back(std::string(it)); }
      return list_;
   }
   else static_assert( gdd::always_false<TYPE>::value, "unsupported type" );
}


_GD_END


// ----------------------------------------------------------------------------
// ----------------------------------------------------------- pointer::strings
// ----------------------------------------------------------------------------


_GD_BEGIN

namespace pointer {

   /**
    * @class strings
    * @brief Manages a list of C-style strings with ownership awareness.
    *
    * This class encapsulates a collection of C-style string pointers (`const char*`) 
    * with functionality to manage ownership and perform operations on these strings.
    *
    * Key features:
    * - Support for different constructors to initialize from various sources:
    *   - Default and tagged constructors to control ownership.
    *   - From `std::vector` with or without taking ownership.
    *   - From C-style arrays of strings.
    * - Custom copy and move semantics that respect ownership.
    * - Methods to append strings from different formats.
    * - Accessor methods for different views and conversions of strings.
    * - Ownership control: Strings can be owned by this class or referenced.
    * - Memory management: If this class owns the strings, it will free them in the destructor.
    *
    * @note 
    * - When the `m_bOwner` flag is true, the class is responsible for managing the 
    *   memory of the strings, hence they are deleted in the destructor.
    * - `gd::types::tag_owner` is presumably a tag type for ownership semantics.
    *
    * @warning 
    * - Ensure that when strings are not owned, their lifetime exceeds the lifetime of this object.
    * - Copying or moving the object might involve deep copying if the source owns the strings.
    */
   class strings
   {
   public:
      using iterator               = std::vector<const char*>::iterator;
      using const_iterator         = std::vector<const char*>::const_iterator;

   public:
      // Constructor from std::vector
      strings() {}
      strings( gd::types::tag_owner ): m_bOwner(true) {}
      strings(const std::vector<const char*>& vectorText) : m_bOwner(false), m_vectorText(vectorText) {}
      strings(const std::vector<const char*>& vectorText, gd::types::tag_owner) :m_bOwner{true} { clone_s(vectorText, m_vectorText); }

      strings(const char** ppiList, size_t uCount) : m_bOwner(false) {
         for( size_t u = 0; u < uCount; u++ ) { m_vectorText[u] = ppiList[u]; }
      }
      strings(const char** ppiList, size_t uCount, gd::types::tag_owner) : m_bOwner(true) { clone_s(ppiList, uCount, m_vectorText); }

      /// Copy constructor
      strings(const strings& o) { assert(this != &o); m_bOwner = o.m_bOwner; common_construct(o);  }
      /// Copy assignment operator
      strings& operator=(const strings& o) { assert(this != &o); common_construct(o); return *this; }
      /// Move constructor
      strings(strings&& o) noexcept { common_construct( std::move(o) ); }
      /// Move assignment operator
      strings& operator=(strings&& o) noexcept { assert(this != &o); common_construct( std::move(o) ); return *this; }


      ~strings()
      {
         if( m_bOwner == true ) { for( const auto& p_ : m_vectorText) { delete[] p_; } }
      }

   private:
      // common copy
      void common_construct(const strings& o);
      void common_construct(strings&& o) noexcept;

   public:  

   // ## operator -----------------------------------------------------------------
   public:
      /// Provides indexed access to the stored strings.  
      std::string_view operator[](size_t uIndex) const { return get_string_view(uIndex); }
      /// Appends a new string to the buffer.
      strings& operator+=( const char* pitext ) { append(pitext); return *this; }
      strings& operator+=( const strings& strings_ ) { append(strings_); return *this; }
      strings& operator+=( const std::vector<const char*>& vectorText ) { append(vectorText); return *this; }
      strings& operator+=( const std::vector<gd::variant_view>& vectorText ) { append(vectorText); return *this; }
   
      /// returns if strings object is owner of strings
      bool is_owner() const { return m_bOwner; }

      // ## operation ---------------------------------------------------------------
      
      // ## append methods

      void append(const char* pitext);
      void append( const strings& strings_ );
      void append( const std::vector<const char*>& vectorText ) { append(strings(vectorText)); }
      void append( const std::vector<gd::variant_view>& vectorText ) { assert(is_owner() == true); for( auto& v_ : vectorText ) { append(v_.as_string().c_str()); } }

      /// Get the number of names
      size_t size() const { return m_vectorText.size(); }

      /// ## get methods, access to strings in list based on index

      const char* get_text(size_t uIndex) const { assert(uIndex < m_vectorText.size()); return m_vectorText[uIndex]; }
      std::string_view get_string_view(size_t uIndex) const { assert(uIndex < m_vectorText.size()); return std::string_view(m_vectorText[uIndex]); }
      std::string get_string(size_t uIndex) const { assert(uIndex < m_vectorText.size()); return std::string(m_vectorText[uIndex]); }
      gd::variant_view get_variant_view(size_t uIndex) const { assert(uIndex < m_vectorText.size()); return gd::variant_view(m_vectorText[uIndex]); }
      gd::variant get_variant(size_t uIndex) const { assert(uIndex < m_vectorText.size()); return gd::variant(m_vectorText[uIndex]); }

      /// Check if there is no texts in strings
      bool empty() const { return m_vectorText.empty(); }

      /// Check if name exists in list
      bool exists(const char* piname) const;

      // ## iterator methods

      std::vector<const char*>::iterator begin() { return m_vectorText.begin(); }
      std::vector<const char*>::iterator end() { return m_vectorText.end(); }
      std::vector<const char*>::const_iterator begin() const { return m_vectorText.begin(); }
      std::vector<const char*>::const_iterator end() const { return m_vectorText.end(); }
      std::vector<const char*>::const_iterator cbegin() const { return m_vectorText.cbegin(); }
      std::vector<const char*>::const_iterator cend() const { return m_vectorText.cend(); }

      /// Clone the string list, copying strings into new memory and store then in a new list
      static void clone_s( const std::vector<const char*>& vectorFrom, std::vector<const char*>& vectorTo );
      static void clone_s( const char** ppiList, size_t uCount, std::vector<const char*>& vectorTo );

      std::vector<const char*> m_vectorText; ///< List of const char pointers
      bool m_bOwner = false;                 ///< If string pointer is owned or not, when owned they need to be deleted in dextructor

   };

   /// common copy
   inline void strings::common_construct(const strings& o) 
   {
      if( is_owner() == false ) { m_vectorText = o.m_vectorText; }
      else
      {
         clone_s( o.m_vectorText, m_vectorText );
      }
   }

   /// common move
   inline void strings::common_construct(strings&& o) noexcept 
   { 
      m_vectorText = std::move(o.m_vectorText); m_bOwner = o.m_bOwner; o.m_bOwner = false; 
   }

} // end namespace pointer

_GD_END

// ----------------------------------------------------------------------------
// -------------------------------------------------------------- pointer::view
// ----------------------------------------------------------------------------

_GD_BEGIN

namespace view {

   /**
    * @class strings
    * @brief A class to manage a collection of string views for efficient string handling.
    *
    * This class wraps around a std::vector<std::string_view> to provide:
    * - Efficient string storage by using string views.
    * - Various methods for appending, accessing, and manipulating strings.
    * - Support for copy and move semantics for performance.
    * - Iterators for range-based for loops or explicit iteration.
    *
    * Note:
    * - Using string_view means that strings should outlive this object unless 
    *   you manually manage the lifetime or convert to std::string.
    * - No automatic memory management for the strings themselves; they are 
    *   expected to be managed by the caller or be literals.
    *
    * @constructor
    * - Default constructor initializes an empty collection.
    * - Construct from std::vector<std::string_view> (by value or by move).
    * - Construct from a C-style array of char pointers with a count.
    *
    * @method
    * - append: Adds new strings from various sources.
    * - operator[]: Access strings by index.
    * - operator+=: Appends new strings to the existing collection.
    * - size: Returns the number of strings.
    * - get_string_view, get_string: Retrieve strings or their views.
    * - empty: Check if there are no strings in the collection.
    * - exists: Check for the existence of a string in the collection.
    * - begin, end, cbegin, cend: Iterator methods for traversal.
    *
    * @note This class does not manage the lifecycle of the strings referenced 
    *       by string_view objects; ensure the strings remain valid for the 
    *       lifetime of this object or use get_string for ownership.
    */
   class strings
   {
   public:
      // Constructor from std::vector
      strings() = default;
      strings(const std::vector<std::string_view>& vectorText) : m_vectorText(vectorText) {}
      strings(std::vector<std::string_view>&& vectorText) : m_vectorText( std::move(vectorText) ) {}

      strings(const char** ppiList, size_t uCount)
      {
         for(size_t u = 0; u < uCount; u++) { m_vectorText.push_back(std::string_view(ppiList[u])); }
      }

      /// Copy constructor
      strings(const strings& o) = default;
      /// Copy assignment operator
      strings& operator=(const strings& o) = default;
      /// Move constructor
      strings(strings&& o) noexcept = default;
      /// Move assignment operator
      strings& operator=(strings&& o) noexcept = default;

      ~strings() = default;

   public:

      // ## operator -----------------------------------------------------------------
      /// Provides indexed access to the stored strings.
      std::string_view operator[](size_t uIndex) const { assert(uIndex < m_vectorText.size()); return m_vectorText[uIndex]; }

      /// Appends a new string to the buffer.
      strings& operator+=(const std::string_view& stringText) { append(stringText); return *this; }
      strings& operator+=(const strings& strings_) { append(strings_); return *this; }
      strings& operator+=(const std::vector<std::string_view>& vectorText) { append(vectorText); return *this; }
      strings& operator+=(const std::vector<std::string>& vectorText) { append(vectorText); return *this; }

      // ## operation ---------------------------------------------------------------

      // ## append methods

      void append(const std::string_view& stringText) { m_vectorText.push_back(std::string_view(stringText)); }
      void append(const strings& strings_) { for( const auto& s_ : strings_ ) { append(s_); } }
      void append(const std::vector<std::string_view>& vectorText) { for( const auto& s_ : vectorText ) { append(s_); } }
      void append(const std::vector<std::string>& vectorText) { for( const auto& s_ : vectorText ) { append(s_); } }

      /// Get the number of texts
      size_t size() const { return m_vectorText.size(); }

      /// Get methods, access to strings in list based on index
      std::string_view get_string_view(size_t uIndex) const { assert(uIndex < m_vectorText.size()); return m_vectorText[uIndex];  }
      std::string get_string(size_t uIndex) const  { assert(uIndex < m_vectorText.size());   return std::string(m_vectorText[uIndex]); }

      /// Check if there is no texts in strings
      bool empty() const { return m_vectorText.empty(); }

      /// Check if name exists in list
      bool exists(const std::string_view& stringText) const
      {
         for(const auto& s_ : m_vectorText) { if (s_ == stringText) return true; }
         return false;
      }

      // ## iterator methods
      std::vector<std::string_view>::iterator begin() { return m_vectorText.begin(); }
      std::vector<std::string_view>::iterator end() { return m_vectorText.end(); }
      std::vector<std::string_view>::const_iterator begin() const { return m_vectorText.begin(); }
      std::vector<std::string_view>::const_iterator end() const { return m_vectorText.end(); }
      std::vector<std::string_view>::const_iterator cbegin() const { return m_vectorText.cbegin(); }
      std::vector<std::string_view>::const_iterator cend() const { return m_vectorText.cend(); }

      std::vector<std::string_view> m_vectorText;
   };
}

_GD_END

/*

class StringFormatter {
public:
    static std::string format(const char* str, const std::function<const char*(size_t)>& cb) {
        const size_t str_len = std::strlen(str);
        char* result = new char[str_len * 2 + 1]; // worst-case scenario: each char is doubled
        char* write = result;
        const char* read = str;
        
        while (*read) {
            if (*read == '{') {
                const char* start = ++read;
                while (*read && *read != '}') ++read; // find closing brace
                
                if (*read == '}') {
                    size_t index = 0;
                    for(const char* p = start; p != read; ++p) {
                        index = index * 10 + (*p - '0');
                    }
                    
                    const char* replacement = cb(index);
                    size_t repl_len = std::strlen(replacement);
                    std::memcpy(write, replacement, repl_len);
                    write += repl_len;
                    ++read; // Move past the closing brace
                } else {
                    // No closing brace found, copy '{'
                    *write++ = '{';
                    read = start; // Reset read to start of the supposed index
                }
            } else {
                *write++ = *read++;
            }
        }
        *write = '\0'; // Null-terminate the result string

        std::string formatted(result);
        delete[] result;
        return formatted;
    }
};

*/