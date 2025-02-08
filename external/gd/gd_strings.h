#pragma once

#include <cassert>
#include <cstdint>                                                            // Standard fixed-width integer types  
#include <cstring>                                                            // For memcpy, memmove, memset  
#include <iterator>                                                           // For std::iterator  
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "gd_variant_view.h"


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
      iterator( STRINGS* pstrings, uint64_t uOffset ) : m_pstrings(pstrings), m_uOffset(uOffset) {}  

      /// Copies the iterator from another iterator instance.  
      iterator(const iterator& o) : m_pstrings(o.m_pstrings), m_uOffset(o.m_uOffset) {}  
      iterator(iterator&& o) noexcept : m_pstrings(o.m_pstrings), m_uOffset(o.m_uOffset) { o.m_pstrings = nullptr; o.m_uOffset = 0; }  

      /// Assigns the iterator state from another iterator instance.  
      iterator& operator=(const iterator& o) {  
         m_pstrings = o.m_pstrings; m_uOffset = o.m_uOffset; return *this;  
      }  

      bool operator==(const iterator& o) const { assert( m_pstrings == o.m_pstrings ); return m_uOffset == o.m_uOffset; }  
      bool operator!=(const iterator& o) const { return !(o == *this); }

      uint64_t offset() const { return m_uOffset; }

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
         for (size_t i = 0; i < uCount; ++i) {
            ++(*this); // Use existing increment operator
         }
         return *this; // Return a reference to this iterator
      }

      /// Advance the iterator by a specified number of string blocks.
      iterator operator+(size_t uCount) const
      {
         iterator it = *this;
         for ( size_t i = 0; i < uCount; ++i ) ++it;
         return it;
      }

      /// Get the current position in the buffer
      uint8_t* get_position() const { return m_pstrings->get_position( m_uOffset ); }


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
   strings32( const std::string_view& string_ ) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append( string_ ); }
   strings32( const std::initializer_list<std::string_view>& listString ) : m_puBuffer(nullptr), m_uSize(0), m_uBufferSize(0) { append( listString ); }
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
   /// Appends a new string to the buffer.
   strings32& operator+=(const std::string_view& stringAppend) { append(stringAppend); return *this; }
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

   /// Appends types that is convertible to string to the buffer.
   strings32& append(const gd::variant_view& variant_view, gd::types::tag_view ) { append(variant_view.as_string()); return *this; }

   /// Appends any number of strings to the buffer using variadic templates.
   template<typename... ARGUMENTS>
   strings32& add(ARGUMENTS&&... args);
   template<typename VALUE>
   strings32& append_any(const VALUE& value);


   /// Erases the string at the specified iterator position from the buffer.
   void erase( iterator itPosition );

   /// Replaces the string at the specified iterator position with a new string.  
   void replace( uint8_t* puPosition, const std::string_view& stringReplace);
   void replace(iterator itPosition, const std::string_view& stringReplace) { replace(itPosition.get_position(), stringReplace); }
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
   /// get std::string_view from buffer at offset position
   static std::string_view to_string_view_s(const uint8_t* puBuffer, uint64_t uPosition);
   /// get std::string from buffer at offset position
   static std::string to_string_s(const uint8_t* puBuffer, uint64_t uPosition);

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
   for ( size_t i = 0; i < uIndex; ++i ) ++it;
   return it.as_string_view();
}  

/// Appends strings to internal buffer.
inline strings32& strings32::append(const std::initializer_list<std::string_view>& listString)
{
   for (const auto& string_ : listString) { append(string_);  }
   return *this;
}

/// Appends strings to internal buffer.
inline strings32& strings32::append(const std::vector<std::string>& vectorString)
{
   for (const auto& string_ : vectorString) { append(string_);  }
   return *this;
}

/// Appends strings to internal buffer.
inline strings32& strings32::append(const std::vector<std::string_view>& vectorString)
{
   for (const auto& string_ : vectorString) { append(string_);  }
   return *this;
}

/// Appends values from initializer list to internal buffer.
inline strings32& strings32::append_any(const std::initializer_list<gd::variant_view>& listValue)
{
   for ( const auto& v_ : listValue ) { append(v_, gd::types::tag_view{}); }
   return *this;
}

/// Appends values from vector to internal buffer.
inline strings32& strings32::append_any(const std::vector<gd::variant_view>& vectorValue)
{
   for ( const auto& v_ : vectorValue ) { append(v_, gd::types::tag_view{}); }
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
   else static_assert( false, "Invalid type" );
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
                    for (const char* p = start; p != read; ++p) {
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