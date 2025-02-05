#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>

/**
* @class token_view
* @brief A lightweight, non-owning view of a token's character sequence.
*
* This class provides a simple view of a constant character array (representing a token)
* and stores its length as a 32-bit unsigned integer. It is similar in concept to
* std::string_view but explicitly uses uint32_t for the length.
*/
class token_view
{
public:

   /**
   * @brief Default constructor.
   *
   * Creates an empty token_view.
   */
   token_view()
      : m_piAscii(nullptr),
      m_uLength(0)
   {
   }

   /**
   * @brief Constructs a token_view from a C-string and a given length.
   *
   * @param piAscii A pointer to the token's character data.
   * @param uLength The length of the token.
   */
   token_view(const char* piAscii, uint32_t uLength)
      : m_piAscii(piAscii),
      m_uLength(uLength)
   {
   }

   /**
   * @brief Constructs a token_view from a null-terminated C-string.
   *
   * The length is computed using std::strlen.
   *
   * @param piAscii A pointer to a null-terminated character array.
   */
   token_view(const char* piAscii)
      : m_piAscii(piAscii),
      m_uLength(piAscii ? static_cast<uint32_t>(std::strlen(piAscii)) : 0)
   {
   }

   /**
   * @brief Returns the pointer to the token's character data.
   *
   * @return A pointer to the beginning of the token.
   */
   const char* data() const
   {
      return m_piAscii;
   }

   /**
   * @brief Returns the length of the token_view.
   *
   * @return The number of characters in the token.
   */
   uint32_t size() const
   {
      return m_uLength;
   }

   /**
   * @brief Checks if the token_view is empty.
   *
   * @return true if empty, false otherwise.
   */
   bool empty() const
   {
      return m_uLength == 0;
   }

   /**
   * @brief Index operator to access individual characters.
   *
   * @param uIndex The index of the character to access.
   * @return The character at the given index.
   * @throw std::out_of_range if uIndex >= size().
   */
   char operator[](uint32_t uIndex) const
   {
      if (uIndex >= m_uLength)
      {
         throw std::out_of_range("Index out of range in token_view::operator[]");
      }
      return m_piAscii[uIndex];
   }

   /**
   * @brief Returns a substring view from the current token_view.
   *
   * @param uPos The starting position of the substring.
   * @param uCount The number of characters to include in the substring.
   *               If uCount exceeds the available characters, the substring
   *               will extend to the end of the token_view.
   * @return A new token_view representing the substring.
   * @throw std::out_of_range if uPos > size().
   */
   token_view substr(uint32_t uPos, uint32_t uCount) const
   {
      if (uPos > m_uLength)
      {
         throw std::out_of_range("Position out of range in token_view::substr");
      }

      uint32_t uAvailable = m_uLength - uPos;
      uint32_t uNewLength = (uCount < uAvailable) ? uCount : uAvailable;
      return token_view(m_piAscii + uPos, uNewLength);
   }

private:

   const char* m_piAscii;    // Pointer to the token's character data.
   uint32_t    m_uLength;    // Length of the token.
   uint32_t    m_uType;      // Type of the token.
};



/**
* @class MultiStringBufferRaw
* @brief Manages a raw buffer that stores multiple strings.
*
* Each string is stored in the buffer with a preceding 4-byte unsigned integer (uint32_t)
* that indicates its length, followed by the string data. The start of each record and the
* string data are padded to a 4-byte boundary.
*/
class MultiStringBufferRaw
{
public:

   /**
   * @brief Default constructor.
   *
   * Initializes the buffer with an initial capacity.
   */
   MultiStringBufferRaw()
      : m_pBuffer(nullptr),
      m_uCapacity(0),
      m_uSize(0)
   {
      // Start with an initial capacity (e.g., 256 bytes).
      AllocateBuffer(256);
   }

   /**
   * @brief Destructor.
   *
   * Frees the allocated buffer.
   */
   ~MultiStringBufferRaw()
   {
      delete[] m_pBuffer;
      m_pBuffer = nullptr;
   }

   /**
   * @brief Clears the buffer.
   *
   * Resets the used size to 0. The allocated memory is retained.
   */
   void Clear()
   {
      m_uSize = 0;
   }

   /**
   * @brief Adds a string to the buffer.
   *
   * The string is stored with a preceding uint32_t length and padded so that the next
   * record starts at a 4-byte aligned address.
   *
   * @param stringToken The string to add.
   * @throw std::length_error if the string's length exceeds the maximum allowed.
   */
   void AddString(std::string_view stringToken)
   {
      // Check that the string's length fits into a uint32_t.
      if (stringToken.size() > static_cast<size_t>(UINT32_MAX))
      {
         throw std::length_error("String length exceeds maximum allowed length.");
      }

      // Calculate and add padding to align the current size to a 4-byte boundary.
      uint32_t uPadding = GetPadding(m_uSize);
      EnsureCapacity(m_uSize + uPadding);
      for (uint32_t uIndex = 0; uIndex < uPadding; uIndex++)
      {
         m_pBuffer[m_uSize++] = 0;
      }

      // Append the string length (4 bytes, little-endian).
      uint32_t uStrLength = static_cast<uint32_t>(stringToken.size());
      EnsureCapacity(m_uSize + sizeof(uint32_t));
      AppendUint32(uStrLength);

      // Append the string data.
      EnsureCapacity(m_uSize + uStrLength);
      std::memcpy(m_pBuffer + m_uSize, stringToken.data(), uStrLength);
      m_uSize += uStrLength;

      // Append padding to align the string data.
      uint32_t uContentPadding = GetPadding(uStrLength);
      EnsureCapacity(m_uSize + uContentPadding);
      for (uint32_t uIndex = 0; uIndex < uContentPadding; uIndex++)
      {
         m_pBuffer[m_uSize++] = 0;
      }
   }

   /**
   * @brief Retrieves the string at the specified index.
   *
   * The index is 0-based. The function iterates through the buffer and returns a view into
   * the requested string.
   *
   * @param uIndex The 0-based index of the string.
   * @return A string_view representing the requested string.
   * @throw std::out_of_range if the index is invalid.
   */
   std::string_view GetStringAt(uint32_t uIndex) const
   {
      uint32_t uCurrentIndex = 0;
      uint32_t uOffset = 0;

      while (uOffset < m_uSize)
      {
         // Align the offset to the next 4-byte boundary.
         uint32_t uOffsetPadding = GetPadding(uOffset);
         uOffset += uOffsetPadding;

         // Ensure enough space for the length field.
         if (uOffset + sizeof(uint32_t) > m_uSize)
         {
            throw std::out_of_range("String index out of range");
         }

         // Read the length field.
         uint32_t uStrLength = ReadUint32(m_pBuffer + uOffset);
         uOffset += sizeof(uint32_t);

         // Check if this is the desired string.
         if (uCurrentIndex == uIndex)
         {
            if (uOffset + uStrLength > m_uSize)
            {
               throw std::runtime_error("Corrupted buffer: string data incomplete");
            }
            return std::string_view(reinterpret_cast<const char*>(m_pBuffer + uOffset), uStrLength);
         }

         // Move past the string data.
         uOffset += uStrLength;

         // Skip any padding bytes after the string data.
         uint32_t uDataPadding = GetPadding(uStrLength);
         uOffset += uDataPadding;

         uCurrentIndex++;
      }

      throw std::out_of_range("String index out of range");
   }

   /**
   * @brief Returns the number of strings stored in the buffer.
   *
   * @return The count of stored strings.
   */
   uint32_t GetStringCount() const
   {
      uint32_t uCount = 0;
      uint32_t uOffset = 0;

      while (uOffset < m_uSize)
      {
         uint32_t uOffsetPadding = GetPadding(uOffset);
         uOffset += uOffsetPadding;

         if (uOffset + sizeof(uint32_t) > m_uSize)
         {
            break;
         }

         uint32_t uStrLength = ReadUint32(m_pBuffer + uOffset);
         uOffset += sizeof(uint32_t) + uStrLength;
         uOffset += GetPadding(uStrLength);
         uCount++;
      }

      return uCount;
   }

   /**
   * @brief Returns the internal buffer pointer.
   *
   * @return A pointer to the internal buffer.
   */
   const uint8_t* GetBuffer() const
   {
      return m_pBuffer;
   }

   /**
   * @brief Returns the current size (used bytes) of the buffer.
   *
   * @return The number of bytes used in the buffer.
   */
   uint32_t GetSize() const
   {
      return m_uSize;
   }

private:

   /**
   * @brief Ensures that the buffer has at least uRequiredCapacity bytes available.
   *
   * If the current capacity is insufficient, the buffer is reallocated to a larger size.
   *
   * @param uRequiredCapacity The required capacity in bytes.
   */
   void EnsureCapacity(uint32_t uRequiredCapacity)
   {
      if (uRequiredCapacity > m_uCapacity)
      {
         // Double the capacity until it meets the requirement.
         uint32_t uNewCapacity = std::max(m_uCapacity * 2, uRequiredCapacity);
         ReallocateBuffer(uNewCapacity);
      }
   }

   /**
   * @brief Allocates the buffer with a specified capacity.
   *
   * @param uCapacity The capacity in bytes.
   */
   void AllocateBuffer(uint32_t uCapacity)
   {
      m_pBuffer = new uint8_t[uCapacity];
      m_uCapacity = uCapacity;
      m_uSize = 0;
   }

   /**
   * @brief Reallocates the internal buffer to a new capacity.
   *
   * The existing data is copied to the new buffer.
   *
   * @param uNewCapacity The new capacity in bytes.
   */
   void ReallocateBuffer(uint32_t uNewCapacity)
   {
      uint8_t* pNewBuffer = new uint8_t[uNewCapacity];
      if (m_uSize > 0)
      {
         std::memcpy(pNewBuffer, m_pBuffer, m_uSize);
      }
      delete[] m_pBuffer;
      m_pBuffer = pNewBuffer;
      m_uCapacity = uNewCapacity;
   }

   /**
   * @brief Computes the padding required to align a given size to a 4-byte boundary.
   *
   * @param uSize The current size in bytes.
   * @return The number of padding bytes required.
   */
   static uint32_t GetPadding(uint32_t uSize)
   {
      uint32_t uRemainder = uSize % 4;
      if (uRemainder == 0)
      {
         return 0;
      }
      return 4 - uRemainder;
   }

   /**
   * @brief Appends a uint32_t value to the buffer in little-endian format.
   *
   * @param uValue The value to append.
   */
   void AppendUint32(uint32_t uValue)
   {
      for (int i = 0; i < 4; i++)
      {
         m_pBuffer[m_uSize++] = static_cast<uint8_t>((uValue >> (8 * i)) & 0xFF);
      }
   }

   /**
   * @brief Reads a uint32_t value from the given data pointer in little-endian format.
   *
   * @param pData Pointer to the data.
   * @return The uint32_t value read.
   */
   static uint32_t ReadUint32(const uint8_t* pData)
   {
      uint32_t uValue = 0;
      for (int i = 0; i < 4; i++)
      {
         uValue |= (static_cast<uint32_t>(pData[i]) << (8 * i));
      }
      return uValue;
   }

   uint8_t* m_pBuffer;     // Raw pointer to the buffer.
   uint32_t m_uCapacity;   // Total capacity of the buffer.
   uint32_t m_uSize;       // Current used size in the buffer.
};
