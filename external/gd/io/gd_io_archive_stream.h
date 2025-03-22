#pragma once

#include <fstream>
#include <cstdint>
#include <string>
#include <utility>
#include <stdexcept>

#include "gd_io_archive.h"

#ifndef _GD_IO_STREAM_BEGIN
#define _GD_IO_STREAM_BEGIN namespace gd { namespace io { namespace stream {
#define _GD_IO_STREAM_END } } }
#endif

/*
improve speeed
   void set_buffer_size(size_t size) {
      if (m_buffer.size() != size) {
         m_buffer.resize(size);
         m_fstreamFile.rdbuf()->pubsetbuf(m_buffer.data(), size);
      }
   }



   template<typename T>
   archive& serialize(T& obj, typename std::enable_if<!std::is_fundamental<T>::value>::type* = nullptr) {
      return obj.serialize(*this);
   }
   
   // Example usage in a class:
   /*
   class MyClass {
   public:
      template<typename Archive>
      Archive& serialize(Archive& ar) {
         return ar << member1 >> member2;
      }
   private:
      int member1;
      std::string member2;
   };
   

*/

_GD_IO_STREAM_BEGIN

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class archive
{
   // ## construction -------------------------------------------------------------
public:
   archive() {}
   archive(const std::string_view& stringPath): m_stringPath( stringPath ) { }
   archive(const std::string_view& stringPath, std::ios_base::openmode mode_) { open(stringPath, mode_); } 
   /// open file stream, if open for read (the openmode is std::ios_base::in | std::ios_base::binary)
   archive(const std::string_view& stringPath, gd::io::tag_io_read ): m_stringPath( stringPath ) { open( stringPath, (std::ios::in | std::ios::binary) ); }
   /// open file stream, if open for write (the openmode is std::ios_base::out | std::ios_base::binary)
   archive(const std::string_view& stringPath, gd::io::tag_io_write ): m_stringPath( stringPath ) { open( stringPath, (std::ios::out | std::ios::binary) ); }
   // copy
   archive(const archive& o) { common_construct(o); }
   archive(archive&& o) noexcept { common_construct(std::move(o)); }
   // assign
   archive& operator=(const archive& o) { common_construct(o); return *this; }
   archive& operator=(archive&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~archive() { close(); }
private:
   // common copy
   void common_construct(const archive& o) {}
   void common_construct(archive&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:
   // Stream extraction operator (>>) for reading
   template<typename TYPE>
   archive& operator>>(TYPE& value_) { return read(value_); }

   // Stream insertion operator (<<) for writing
   template<typename TYPE>
   archive& operator<<(const TYPE& value_) { return write(value_); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   /// is file open or not
   bool is_open() const { return m_fstreamFile.is_open(); }
//@}

/** \name OPERATION
*///@{
   /// open file stream, if open for write the openmode should be std::ios_base::out | std::ios_base::binary
   std::pair<bool, std::string> open(std::ios_base::openmode mode_) { return open_s(m_fstreamFile, m_stringPath, mode_); }
   std::pair<bool, std::string> open(const std::string_view& stringPath, std::ios_base::openmode mode_);
   std::pair<bool, std::string> open(const std::string_view& stringPath, gd::io::tag_io_read) { return open(stringPath, ( std::ios::in | std::ios::binary )); }
   void close() { m_fstreamFile.close(); }
//@}


/** \name READ
* Read methods that reads from file
*///@{
   archive& read_size(uint32_t& uSize) { return read(&uSize, sizeof(uSize)); }
   archive& read_size(uint64_t& uSize) { return read(&uSize, sizeof(uSize)); }

   archive& read_block(uint32_t& uSize, void* pdata_);
   archive& read_block(uint64_t& uSize, void* pdata_);

   archive& read(void* pdata_, uint64_t uSize);

   archive& read(std::string& string_);

   template<typename TYPE>
   archive& read(TYPE& value_) { return read(&value_, sizeof(TYPE)); }

   template<typename... VALUES>
   archive& read_all(VALUES&... values_);
//@}

/** \name WRITE
* Write methods that writes to file
*///@{
   archive& write_size(uint32_t uSize) { return write(&uSize, sizeof(uSize)); }
   archive& write_size(uint64_t uSize) { return write(&uSize, sizeof(uSize)); }

   archive& write_block(uint32_t uSize, const void* pdata_);
   archive& write_block(uint64_t uSize, const void* pdata_);

   archive& write(const void* pdata_, uint64_t uSize);

   archive& write(const std::string_view& stringData);
   archive& write(const std::string& stringData);

   template<typename TYPE>
   archive& write(const TYPE& value_ ) { return write( &value_, sizeof(TYPE)); }

   template<typename... VALUES>
   archive& write_all(const VALUES&... values_);

//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{

//@}


// ## attributes ----------------------------------------------------------------
public:
   std::string m_stringPath; ///< path to file
   std::fstream m_fstreamFile; ///< file stream object


// ## free functions ------------------------------------------------------------
public:
   static std::pair<bool, std::string> open_s(std::fstream& fstreamFile,const std::string_view& stringPath, std::ios_base::openmode mode_);
};

/// open file stream
inline std::pair<bool, std::string> archive::open(const std::string_view& stringPath, std::ios_base::openmode mode_) {
   close();
   m_stringPath = stringPath;
   return open( mode_ );
}

/// read block of data from archive, first read size of block and then block of data
inline archive& archive::read_block(uint32_t& uSize, void* pdata_) {
   read_size(uSize);
   return read(pdata_, uSize);
}

/// read block of data from archive, first read size of block and then block of data
inline archive& archive::read_block(uint64_t& uSize, void* pdata_) {
   read_size(uSize);
   return read(pdata_, uSize);
}

/// read data from archive
inline archive& archive::read(void* pdata_, uint64_t uSize) {
   m_fstreamFile.read((char*)pdata_, uSize);
   return *this;
}

template<typename... VALUES>
archive& archive::read_all(VALUES&... values_) {
   (read(values_), ...);
   return *this;
}


/// write block of data to archive, first write size of block and then block of data
inline archive& archive::write_block(uint32_t uSize, const void* pdata_) {
   write_size(uSize);
   return write(pdata_, uSize);
}


/// write block of data to archive, first write size of block and then block of data
inline archive& archive::write_block(uint64_t uSize, const void* pdata_) {
   write_size(uSize);
   return write(pdata_, uSize);
}

/// write data to archive
inline archive& archive::write(const void* pdata_, uint64_t uSize) {
   m_fstreamFile.write((const char*)pdata_, uSize);
   return *this;
}

template<typename... VALUES>
archive& archive::write_all(const VALUES&... values_) {
   (write(values_), ...);
   return *this;
}

_GD_IO_STREAM_END