#pragma once

#include <cassert>
#include <fstream>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>

#include "gd_io_archive.h"

#ifndef _GD_IO_STREAM_BEGIN
#define _GD_IO_STREAM_BEGIN namespace gd { namespace io { namespace stream {
#define _GD_IO_STREAM_END } } }
#endif

_GD_IO_STREAM_BEGIN

/**
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class repository
{
public:
   /// @brief Entry flags that marks the state of the file that entry holds information about.
   enum enumEntryFlag
   {
      eEntryFlagNone       = 0x0000,   ///< No flags
      eEntryFlagValid      = 0x0001,   ///< Entry is valid
      eEntryFlagDeleted    = 0x0002,   ///< Entry is deleted
      eEntryFlagRemove     = 0x0004,   ///< Entry is marked for removal
   };
public:

   /**
    * @brief 
    */
   struct header
   {
      header() = default;

      void add_entry() { m_uEntryCount++; assert( m_uEntryCount < m_uEntrySize ); }

      uint64_t size() const { return m_uEntrySize; }

      uint64_t m_uMagic = repository::get_magic_number_s(); ///< Magic number
      uint64_t m_uVersion = 1;    ///< Version number
      uint64_t m_uEntrySize = 10; ///< Size of entry in bytes
      uint64_t m_uEntryCount = 0; ///< Number of entries
   };

   struct entry
   {
      entry(): m_uOffset(0), m_uSize(0), m_uFlags(0) { std::memset(this, 0, sizeof(entry)); }

      entry(const std::string& name, uint64_t uOffset, uint64_t uSize, unsigned uFlags): m_uOffset(uOffset), m_uSize(uSize), m_uFlags(uFlags) {
         std::strncpy(m_piszName, name.c_str(), sizeof(m_piszName) - 1);
         m_piszName[sizeof(m_piszName) - 1] = '\0'; // Ensure null-termination
      }
      entry(const entry& o) { copy(o); }

      entry& operator=(const entry& o) { if( this != &o ) { copy(o); } return *this; }

      bool operator==(const entry& o) const { return std::strncmp(m_piszName, o.m_piszName, sizeof(m_piszName)) == 0; }

      void copy(const entry& o) {
         std::strncpy(m_piszName, o.m_piszName, sizeof(m_piszName) - 1);
         m_piszName[sizeof(m_piszName) - 1] = '\0'; // Ensure null-termination
         m_uOffset = o.m_uOffset; m_uSize = o.m_uSize; m_uFlags = o.m_uFlags;
      }

      std::string_view get_name() const { return std::string_view(m_piszName); }

      void set_offset(uint64_t uOffset) { m_uOffset = uOffset; }

      bool is_valid() const { return ( m_uFlags & eEntryFlagValid ) != 0; }
      bool is_deleted() const { return ( m_uFlags & eEntryFlagDeleted ) != 0; }
      bool is_remove() const { return ( m_uFlags & eEntryFlagRemove ) != 0; }

      void set_valid() { m_uFlags |= eEntryFlagValid; }
      void set_deleted() { m_uFlags |= eEntryFlagDeleted; }
      void set_remove() { m_uFlags |= eEntryFlagRemove; }

      uint64_t offset() const { return m_uOffset; }
      uint64_t size() const { return m_uSize; }
      uint64_t offset_end() const { return offset() + size(); }

      char m_piszName[256];      ///< File name
      uint64_t m_uOffset;        ///< Offset in archive file
      uint64_t m_uSize;          ///< Size of file content
      unsigned m_uFlags;         ///< Entry validity flag
   };

// ## construction -------------------------------------------------------------
public:
   repository(): m_pFile(nullptr) {}
   // copy
   repository(const repository& o) { common_construct(o); }
   repository(repository&& o) noexcept { common_construct(std::move(o)); }
   // assign
   repository& operator=(const repository& o) { common_construct(o); return *this; }
   repository& operator=(repository&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~repository() { close(); }
private:
   // common copy
   void common_construct(const repository& o) {}
   void common_construct(repository&& o) noexcept {}

// ## operator -----------------------------------------------------------------
public:


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{

//@}

/** \name OPERATION
*///@{
   // ## open file

   std::pair<bool, std::string> open(const std::string_view& stringPath);
   std::pair<bool, std::string> open(const std::string_view& stringPath, const std::string_view& stringMode );

   std::pair<bool, std::string> create(const std::string_view& stringPath);
   std::pair<bool, std::string> create();

   // ## add data or files to repository
   
   std::pair<bool, std::string> add(const std::string_view& stringName, const void* pdata, uint64_t uSize);
   std::pair<bool, std::string> add(const std::string_view& stringFile);

   // ## updates internal data in repository to file

   std::pair<bool, std::string> flush();

   // ## read data from repository

   std::pair<bool, std::string> read(const std::string_view& stringName, void* pdata, uint64_t uSize) const;
   std::pair<bool, std::string> read_to_file(const std::string_view& stringName, const std::string_view& stringPath) const;

   // ## information about repository

   std::vector<std::string> list() const;
   int64_t find(const std::string_view& stringName) const;
   bool exists(const std::string_view& stringName) const { return find(stringName) != -1; }
   size_t size() const { return m_vectorEntry.size(); }
   size_t size_reserved() const { return m_header.size(); }
   bool empty() const { return m_vectorEntry.empty(); }

   // ## remove data from repository

   std::pair<bool, std::string> remove( const std::string_view& stringName );
   void remove( std::size_t uIndex );

   std::pair<bool, std::string> remove_entry_from_file( const std::vector<uint64_t>& vectorIndexes );

   // ## close repository

   void close();

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
   header m_header;               ///< Header of repository
   FILE* m_pFile;                 ///< File handle for archive
   std::string m_stringRepositoryPath;///< Path to archive file
   std::vector<entry> m_vectorEntry;///< Index of files


// ## free functions ------------------------------------------------------------
public:
   /// @brief calculate position of entry block in repository file
   static uint64_t calculte_entry_offset_s() { return (uint64_t)sizeof( header ); }
   /// @brief calculate position of entry block in repository file
   static uint64_t calculte_file_offset_s( const repository& repository_ ) { return calculte_entry_offset_s() + repository_.size_reserved() * sizeof( entry ); }
   static std::pair<bool, std::string> write_header_s(FILE* pfile, const header& header_);
   /// @brief write entry block to file
   static std::pair<bool, std::string> write_entry_block_s(FILE* pfile, const void* pdata, uint64_t uSize, uint64_t uOffset);
   /// @brief fill block with fill value
   static std::pair<bool, std::string> write_block_s(FILE* pfile, uint8_t uFillValue, uint64_t uSize, uint64_t uOffset);
   /// @brief calculate the first position of content in repository file
   static uint64_t calculate_first_content_position_s(const repository& repository_);
   /// @brief calculate the first position of content in repository file
   static uint64_t calculate_first_free_content_position_s(const repository& repository_);
   /// @brief get size of entry buffer
   static uint64_t size_entry_buffer_s(const repository& repository_) { return repository_.m_vectorEntry.size() * sizeof(entry); }
   /// get size of reserved buffer for entry
   static uint64_t size_entry_reserved_buffer_s(const repository& repository_) { return repository_.m_header.size() * sizeof( entry ); }
   /// @brief get magic number used to identify repository file
   static constexpr uint64_t get_magic_number_s();

};

/// create repository file
inline std::pair<bool, std::string> repository::create(const std::string_view& stringPath) {
   close();
   m_stringRepositoryPath = stringPath;
   return create();
}

inline uint64_t repository::calculate_first_content_position_s(const repository& repository_) { 
   uint64_t uOffset = calculte_file_offset_s( repository_ );
   return uOffset; 
}


/// calculate first free content position in repository file
inline uint64_t repository::calculate_first_free_content_position_s(const repository& repository_) { 
   uint64_t uOffset = calculte_file_offset_s( repository_ );
   if( repository_.empty() == false ) uOffset += repository_.m_vectorEntry.back().offset_end();
   return uOffset; 
}

/// returns the magic number used to identify repository file
inline constexpr uint64_t repository::get_magic_number_s() {
   return 0x2e2d2e2d2e2d2e2d; // .-.-.-.-
}




_GD_IO_STREAM_END