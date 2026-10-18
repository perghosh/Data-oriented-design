#include "gd_string_transport.h"

_GD_BEGIN

/// helper function to handle common construction logic for copy and move operations.
void string_transport::common_construct(const string_transport& o)
{
   release();                                                                  // release any existing resources and clear the state of the current string_transport object

   if( o.m_piText == nullptr) { return; }

   const auto uLength = o.length();
   allocate(uLength);
   if( uLength > 0) { std::memcpy(m_piText, o.m_piText, uLength); }
   m_piText[uLength] = '\0';
   set_length(uLength);
}

/// helper function to handle common construction logic for move operations. --
void string_transport::common_construct(string_transport&& o) noexcept
{
   release();

   m_uStateAndSize = o.m_uStateAndSize;
   m_piText = o.m_piText;

   o.m_uStateAndSize = 0;
   o.m_piText = nullptr;
}

/** ---------------------------------------------------------------------------
 * @brief Appends a character buffer to the end of the current string content.
 * @param piText Pointer to the source character data to append.
 * @param uLength Number of characters from piText to append; if 0, no change is made.
 */
void string_transport::append(const char* piText, const std::size_t uLength)
{
   if( uLength == 0 ) { return; }

   // ## allocate new memory for the combined string ..........................
   auto uOldLlength = length();
   auto uNewLength = uOldLlength + uLength;
   allocate(uNewLength);                                                       // allocate
   std::memcpy(m_piText + uOldLlength, piText, static_cast<size_t>(uLength));
   m_piText[uNewLength] = '\0';
   set_length(uNewLength);
}

/** ---------------------------------------------------------------------------
 * @brief Assigns text from a string view into this transport object, copying the characters and updating its stored length.  
 * @param stringText The source text to copy into the object.
 * @return A reference to this object after the assignment.
 */
string_transport& string_transport::assign(std::string_view stringText)
{
   const auto uLength = stringText.size();
   allocate(uLength);
   if( uLength > 0) { std::memmove(m_piText, stringText.data(), uLength); }
   m_piText[uLength] = '\0';
   set_length(uLength);
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Inserts text at a specified position in the string transport buffer, shifting existing content to the right.  
 * @param uIndex The zero-based insertion position. Must be less than or equal to the current length.
 * @param stringText The text to insert. If empty, no changes are made.
 * @return A reference to the current `string_transport` object (`*this`) to support chaining.
 */
string_transport& string_transport::insert(size_type uIndex, std::string_view stringText)
{
   const auto uOldLength = length();
   if( uIndex > uOldLength ) { throw std::out_of_range("string_transport::insert"); }
   if( stringText.empty() == true ) { return *this; }

   std::string stringCopy(stringText);
   const auto uInsertLength = stringCopy.size();
   const auto uNewLength = uOldLength + uInsertLength;

   allocate(uNewLength);
   std::memmove(m_piText + uIndex + uInsertLength, m_piText + uIndex, uOldLength - uIndex + 1);
   std::memcpy(m_piText + uIndex, stringCopy.data(), uInsertLength);
   set_length(uNewLength);
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Erases a range of characters from the string starting at the specified index
 * @param uIndex Zero-based position where erasing begins.
 * @param uCount Number of characters to erase; if equal to npos or extending past the end, erases to the end of the string.
 * @return A reference to this string_transport after the erase operation.
 */
string_transport& string_transport::erase(size_type uIndex, size_type uCount)
{                                                                                                  assert( length() >= uIndex );
   const auto uLength = length();
   if( uIndex > uLength ) { throw std::out_of_range("string_transport::erase"); }

   auto uEraseLength = uCount;
   if( (uEraseLength == npos ) || (uIndex + uEraseLength > uLength)) { uEraseLength = uLength - uIndex; }
   if( uEraseLength == 0 ) { return *this; }

   std::memmove(m_piText + uIndex, m_piText + uIndex + uEraseLength, uLength - uIndex - uEraseLength + 1);
   set_length(uLength - uEraseLength);
   return *this;
}

/** ---------------------------------------------------------------------------
 * @brief Replaces a range of characters in the string with the provided text and returns the current object.
 * @param uIndex The starting index of the range to replace.
 * @param uCount The number of characters to remove starting at uIndex.
 * @param stringText The text to insert at uIndex after removal.
 * @return A reference to this string_transport object after the replacement.
 */
string_transport& string_transport::replace(size_type uIndex, size_type uCount, std::string_view stringText)
{
   std::string stringCopy(stringText);
   erase(uIndex, uCount);
   insert(uIndex, stringCopy);
   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief Allocates or replaces the internal text buffer to the requested size, preserving existing content when possible.
 * @param uSize The new buffer size (excluding the null terminator). Must be at least the current string length.
 */
void string_transport::allocate(std::size_t uSize)
{                                                                                                  assert(uSize >= length());
   // ## Check if borrowed and if the new size is less than the current size, then return without allocating new memory
   auto uCurrentSize = size();                                                 // get the current size of the buffer, simpler to debug
   if( (is_borrowed() == true) && (uSize < uCurrentSize) ) { return; }

   char* piText_ = new char[uSize + 1];                                        // allocate new memory for the string_transport buffer, +1 for null terminator
   if( m_piText != nullptr )
   {
      std::memcpy(piText_, m_piText, static_cast<size_t>(uCurrentSize + 1));
   }
   piText_[uSize] = '\0';                                                      // null terminate the new buffer

   if( is_owned()) { delete[] m_piText; }
   m_piText = piText_;
   m_uStateAndSize = static_cast<uint64_t>(uSize);
}

_GD_END