#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>

class Serializer
{
public:
   Serializer(const std::string& stringFilePath)
      : m_bIsWriting(false)
      , m_pStream(nullptr)
   {
      Open(stringFilePath);
   }

   ~Serializer()
   {
      Close();
   }

   void Open(const std::string& stringFilePath)
   {
      Close(); // Ensure any existing stream is closed

      m_pStream = new std::fstream(stringFilePath, 
         std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);

      if (!m_pStream->is_open())
      {
         delete m_pStream;
         m_pStream = nullptr;
         throw std::runtime_error("Failed to open file: " + stringFilePath);
      }
      m_bIsWriting = true;
   }

   void Close()
   {
      if (m_pStream)
      {
         m_pStream->close();
         delete m_pStream;
         m_pStream = nullptr;
      }
      m_bIsWriting = false;
   }

   // Write functions
   template<typename T>
   void Write(const T& value)
   {                                                                                               static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
      if (!m_bIsWriting || !m_pStream)
      {
         throw std::runtime_error("Serializer not in write mode");
      }
      m_pStream->write(reinterpret_cast<const char*>(&value), sizeof(T));
   }

   void WriteString(const std::string& stringValue)
   {
      if (!m_bIsWriting || !m_pStream)
      {
         throw std::runtime_error("Serializer not in write mode");
      }
      uint32_t uLength = static_cast<uint32_t>(stringValue.length());
      Write(uLength);
      m_pStream->write(stringValue.data(), uLength);
   }

   // Read functions
   template<typename T>
   void Read(T& value)
   {                                                                                               static_assert(std::is_trivially_copyable<T>::value, "Type must be trivially copyable");
      if (m_bIsWriting || !m_pStream)
      {
         throw std::runtime_error("Serializer not in read mode");
      }
      m_pStream->read(reinterpret_cast<char*>(&value), sizeof(T));
   }

   void ReadString(std::string& stringValue)
   {
      if (m_bIsWriting || !m_pStream)
      {
         throw std::runtime_error("Serializer not in read mode");
      }
      uint32_t uLength;
      Read(uLength);
      stringValue.resize(uLength);
      m_pStream->read(stringValue.data(), uLength);
   }

   // Specialized vector writing
   template<typename T>
   void WriteVector(const std::vector<T>& vectorValue)
   {
      static_assert(std::is_trivially_copyable<T>::value, "Vector type must be trivially copyable");
      uint32_t uSize = static_cast<uint32_t>(vectorValue.size());
      Write(uSize);
      if (uSize > 0)
      {
         m_pStream->write(reinterpret_cast<const char*>(vectorValue.data()), 
            uSize * sizeof(T));
      }
   }

   template<typename T>
   void ReadVector(std::vector<T>& vectorValue)
   {
      static_assert(std::is_trivially_copyable<T>::value, "Vector type must be trivially copyable");
      uint32_t uSize;
      Read(uSize);
      vectorValue.resize(uSize);
      if (uSize > 0)
      {
         m_pStream->read(reinterpret_cast<char*>(vectorValue.data()), 
            uSize * sizeof(T));
      }
   }

   void SetReadMode()
   {
      if (m_pStream)
      {
         m_pStream->seekg(0);
         m_bIsWriting = false;
      }
   }

   void SetWriteMode()
   {
      if (m_pStream)
      {
         m_pStream->seekp(0);
         m_bIsWriting = true;
      }
   }

   bool IsEof() const
   {
      return m_pStream && m_pStream->eof();
   }

private:
   bool m_bIsWriting;
   std::fstream* m_pStream;
};

// Usage example:
void ExampleUsage()
{
   try
   {
      Serializer serializer("data.bin");

      // Writing
      int32_t iValue = 42;
      float dFloat = 3.14f;
      std::string stringText = "Hello";
      std::vector<uint8_t> vectorData = {1, 2, 3, 4};

      serializer.Write(iValue);
      serializer.Write(dFloat);
      serializer.WriteString(stringText);
      serializer.WriteVector(vectorData);

      // Switch to reading
      serializer.SetReadMode();

      // Reading
      int32_t iReadValue;
      float dReadFloat;
      std::string stringReadText;
      std::vector<uint8_t> vectorReadData;

      serializer.Read(iReadValue);
      serializer.Read(dReadFloat);
      serializer.ReadString(stringReadText);
      serializer.ReadVector(vectorReadData);
   }
   catch (const std::exception& e)
   {
      // Handle error
   }
}