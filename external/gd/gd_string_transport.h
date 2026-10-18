// @FILE [tag: string, transport] [description: ] [name: gd_string_transport.h] [type: header] 

#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>   

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gd {
#define _GD_END }
#endif

_GD_BEGIN

/** @CLASS [tag: string, transport] [summary: string used for transport and minimal memory allocation]
 * @brief Mutable character string that can use caller-provided storage.
 *
 * Builds and edits text using either a borrowed writable buffer or owned
 * heap storage. Borrowing a buffer avoids allocating destination storage
 * while the text, including its terminating null character, fits.
 * Operations such as insert() and replace() may use temporary storage.
 *
 * @par Borrowed storage
 * Construct from a writable char buffer, std::span<char>, or
 * std::span<std::byte>. Compatible std::array and std::vector objects can
 * be passed through these span constructors.
 *
 * The buffer must contain between 1 and 65535 bytes, including space for
 * the terminating null character. Construction initializes an empty string
 * and overwrites the first byte; existing buffer contents are not imported.
 *
 * The caller owns the borrowed buffer. It must remain valid and at the same
 * address while this object uses it. In particular, do not resize a backing
 * vector if doing so could invalidate its data pointer.
 *
 * When growth exceeds the borrowed buffer's usable capacity, the object
 * switches to owned heap storage. The borrowed buffer is never deleted.
 *
 * @par Length and storage size
 * length() returns the number of stored characters, excluding the null
 * terminator. Unlike std::string, size() returns the entire buffer size
 * when borrowing, including the space reserved for the null terminator;
 * for owned storage, size() returns the stored character count.
 *
 * @par Character access
 * operator[] provides unchecked access; use an index less than length().
 * at() throws std::out_of_range for an invalid character index.
 * front(), back(), and pop_back() require a nonempty string.
 *
 * @par Ownership and lifetime
 * Copying creates an independent owned copy. Moving transfers the pointer
 * and storage state; moving a borrowed string does not extend the lifetime
 * of its external buffer. Operations that allocate can invalidate pointers
 * and references previously obtained from this object.
 *
 * @note A default-constructed or moved-from object has length zero and may
 * return nullptr from c_str(). Do not treat it as a non-null empty C-string.
 *
 * @par Example: build text in a local buffer
 * @code{.cpp}
 * std::array<std::byte, 150> arrayBuffer;
 * gd::string_transport stringText(arrayBuffer);
 *
 * stringText = "Hello";
 * stringText += ", world";
 * stringText.push_back('!');
 *
 * // Text: "Hello, world!"
 * // length() == 13; size() == 150; is_borrowed() == true.
 * @endcode
 *
 * @par Example: edit and access characters
 * @code{.cpp}
 * gd::string_transport stringText("Hello");
 * stringText.insert(5, ", world");    // "Hello, world"
 * stringText.replace(0, 5, "Hi");     // "Hi, world"
 * stringText.erase(2, 2);             // "Hiworld"
 * stringText.push_back('!');          // "Hiworld!"
 *
 * char iFirstCharacter = stringText.front();
 * char iLastCharacter = stringText.back();
 * stringText.at(0) = 'h';            // "hiworld!"
 * stringText.pop_back();             // "hiworld"
 * stringText.clear();                // length() == 0
 * @endcode
 *
 * @par Example: append strings and views
 * @code{.cpp}
 * gd::string_transport stringText("Path: ");
 * std::string stringDirectory = "data";
 * stringText.append(stringDirectory);
 * stringText.append(std::string_view("/input.txt"));
 *
 * // Text: "Path: data/input.txt"
 * @endcode
 */
class string_transport
{
public:
   enum class enumConstant : uint64_t
   {
      eFlagBorrowed     = 1ull << 61,
      eFlagLiteral      = 1ull << 60,

      eClearFlag        = 0x0FFFFFFF'FFFFFFFFull,

      eBorrowSizeMask   = 0x00000000'FFFF0000ull,
      eBorrowLengthMask = 0x00000000'0000FFFFull,

      eSizeMask         = 0x0000FFFF'FFFFFFFFull,
   };

public:
   using size_type = std::size_t;
   static constexpr size_type npos = static_cast<size_type>(-1);

// @API [tag: construction]
public:
   string_transport() : m_uStateAndSize(0), m_piText(nullptr) {}

   explicit string_transport( const char* piText ) : m_uStateAndSize(0), m_piText(nullptr) { append(piText); }
   // Borrow an external writable buffer (no allocation).
   string_transport(char* piBuffer, std::size_t uBufferSize);

   explicit string_transport(std::span<std::byte> spanBuffer)                  // Flexible adapter: accepts std::array<std::byte, N>, std::vector<std::byte>, etc.
      : string_transport(reinterpret_cast<char*>(spanBuffer.data()), spanBuffer.size()) {}

   explicit string_transport(std::span<char> spanBuffer)                       // Optional convenience for std::array<char, N> / char buffers.
      : string_transport(spanBuffer.data(), spanBuffer.size()) {}

   // copy
   string_transport(const string_transport& o): m_uStateAndSize(0), m_piText(nullptr) { common_construct(o); }
   string_transport(string_transport&& o) noexcept : m_uStateAndSize(0), m_piText(nullptr) { common_construct(std::move(o)); }

   // assign
   string_transport& operator=(const string_transport& o)
   {
      if (this != &o) { common_construct(o); }
      return *this;
   }
   string_transport& operator=(string_transport&& o) noexcept
   {
      if (this != &o) { common_construct(std::move(o)); }
      return *this;
   }

   ~string_transport() { release(); }
private:
   void release() noexcept;
   void common_construct(const string_transport& o);
   void common_construct(string_transport&& o) noexcept;

// @API [tag: operator]
public:
   string_transport& operator=(const char* piText);
   string_transport& operator=(std::string_view stringText);
   string_transport& operator+=(const char* piText) { append(piText); return *this; }

   char& operator[](size_type uIndex) noexcept { return m_piText[uIndex]; }
   const char& operator[](size_type uIndex) const noexcept { return m_piText[uIndex]; }

   char& at(size_type uIndex);
   const char& at(size_type uIndex) const;

   char& front() noexcept { assert(length() > 0); return m_piText[0]; }
   const char& front() const noexcept { assert(length() > 0); return m_piText[0]; }

   char& back() noexcept { assert(length() > 0); return m_piText[length() - 1]; }
   const char& back() const noexcept { assert(length() > 0); return m_piText[length() - 1]; }

// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// ## state
   bool is_borrowed() const noexcept { return m_uStateAndSize & static_cast<uint64_t>(enumConstant::eFlagBorrowed); }
   bool is_literal()  const noexcept { return m_uStateAndSize & static_cast<uint64_t>(enumConstant::eFlagLiteral); }
   bool is_owned()    const noexcept { return !(m_uStateAndSize & (static_cast<uint64_t>(enumConstant::eFlagBorrowed) | static_cast<uint64_t>(enumConstant::eFlagLiteral))); }

   void clear_flags() { m_uStateAndSize &= static_cast<uint64_t>(enumConstant::eClearFlag); }

   void set_length(std::size_t uLength);


// @API [tag: operation]


   void append(const char* piText);
   void append(const char* piText, const std::size_t uLength);
   void append(std::string_view stringText) { append(stringText.data(), stringText.size()); }
   void append(std::span<std::byte> spanBuffer) { append(reinterpret_cast<const char*>(spanBuffer.data()), spanBuffer.size()); }

   void clear();
   void push_back(char iCharacter);
   void pop_back();

   string_transport& assign(std::string_view stringText);
   string_transport& insert(size_type uIndex, std::string_view stringText);
   string_transport& erase(size_type uIndex = 0, size_type uCount = npos);
   string_transport& replace(size_type uIndex, size_type uCount, std::string_view stringText);

   void allocate(std::size_t uSize);

   const char* c_str() const noexcept { return m_piText; } ///< returns the pointer to the string data
   char* data() const noexcept { return m_piText; }   ///< returns the pointer to the string data
   std::size_t size() const noexcept;   ///< buffer length or when not borrowed, the actual string length
   std::size_t length() const noexcept; ///< the actual string length

protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   uint64_t m_uStateAndSize;
   char* m_piText;


// @API [tag: free-functions]
public:

};

/// Construct string with borrowed buffer, the buffer must be writable and the size must be less than 65536 bytes (64KB)
inline string_transport::string_transport(char* piBuffer, std::size_t uBufferSize) : m_uStateAndSize(0), m_piText(piBuffer)
{                                                                                                   assert(piBuffer != nullptr);
                                                                                                    assert(uBufferSize > 0);
                                                                                                    assert(uBufferSize <= 0xFFFFu); // borrow-size field is 16-bit
   // ## Set the borrowed flag and store the buffer size in the upper 16 bits of m_uStateAndSize
   m_uStateAndSize = static_cast<uint64_t>(enumConstant::eFlagBorrowed);
   m_uStateAndSize |= (static_cast<uint64_t>(uBufferSize) << 16) & static_cast<uint64_t>(enumConstant::eBorrowSizeMask);

   set_length(0);
   m_piText[0] = '\0';
}

/// Assignment operator for string_transport, appends the given C-string to the current string_transport buffer.
inline string_transport& string_transport::operator=(const char* piText)
{ 
   assign(piText);
   return *this;
}

/// Assignment operator for string_transport, appends the given std::string_view to the current string_transport buffer.
inline string_transport& string_transport::operator=(std::string_view stringText)
{
   assign(stringText);
   return *this;
}

/// Returns a reference to the character at the specified index in the string_transport buffer, with bounds checking.
inline char& string_transport::at(size_type uIndex)
{
   if( uIndex >= length() ) { throw std::out_of_range("string_transport::at"); }
   return m_piText[uIndex];
}

/// Returns a const reference to the character at the specified index in the string_transport buffer, with bounds checking.
inline const char& string_transport::at(size_type uIndex) const
{
   if( uIndex >= length() ) { throw std::out_of_range("string_transport::at"); }
   return m_piText[uIndex];
}

/// clear text (does not reallocate)
inline void string_transport::clear()
{
   if( m_piText != nullptr ) { m_piText[0] = '\0'; }
   set_length(0);
}

/// 
inline void string_transport::push_back(char iCharacter)
{
   char piCharacter[2] = { iCharacter, '\0' };
   append(piCharacter, 1);
}

/// 
inline void string_transport::pop_back()
{
   auto uLength = length();
   assert(uLength > 0);
   m_piText[uLength - 1] = '\0';
   set_length(uLength - 1);
}

/// Appends the given C-string to the current string_transport buffer. --------
inline void string_transport::append(const char* piText)
{
   auto uLength = std::strlen(piText);
   append(piText, uLength);
}

/// Returns the length of the string in string_transport buffer ---------------
inline std::size_t string_transport::length() const noexcept
{
   std::size_t uSize;
   if(is_borrowed() == true)
   {
      uSize = static_cast<std::size_t>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eBorrowLengthMask));
   }
   else
   {
      uSize = static_cast<std::size_t>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eSizeMask));
   }

   return uSize;
}

/// Returns the size of the string_transport buffer, if borrowed, it returns the size of the buffer that it borrows from, if owner then it returns the size of the buffer that it owns.
inline std::size_t string_transport::size() const noexcept
{
   std::size_t uSize;
   if(is_borrowed() == true )
   {
      uSize = static_cast<std::size_t>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eBorrowSizeMask));
      uSize >>= 16;
   }
   else
   {
      uSize = static_cast<std::size_t>(m_uStateAndSize & static_cast<uint64_t>(enumConstant::eSizeMask));
   }

   return uSize;
}

/// ----------------------------------------------------------------------------
/// Set the length for string_transport, if borrowed, the length must be less than the size of the 
/// string_transport buffer that it borrows from, If owner then it can be any length.
/// @param uLength The new length to set for the string_transport object.
inline void string_transport::set_length(std::size_t uLength) 
{ 
   if(is_borrowed() == true)
   {                                                                                                assert( uLength < size() );
      // ## Set the lower 16 bits of m_uStateAndSize to the new length, preserving the borrowed flag and size in the upper bits.
      const auto uLengthMask = static_cast<uint64_t>(enumConstant::eBorrowLengthMask);
      m_uStateAndSize = (m_uStateAndSize & ~uLengthMask) | static_cast<uint64_t>(uLength);
   }
   else
   {
      m_uStateAndSize = static_cast<uint64_t>(uLength);
   }
}

/// Release the string_transport buffer, if it is owned, it will delete the buffer, if it is borrowed, it will not delete the buffer.
inline void string_transport::release() noexcept
{
   if( is_borrowed() == false )
   {
      delete[] m_piText;
   }
   m_piText = nullptr;
   m_uStateAndSize = 0;    
}

_GD_END