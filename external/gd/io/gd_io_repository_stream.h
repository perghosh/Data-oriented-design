/**
* @file gd_io_repository_stream.h
* @brief Defines the repository class for managing file-based repositories.
*
* This file contains the definition of the `repository` class, which provides
* functionality for creating, opening, reading, writing, and managing file-based
* repositories. The repository class supports operations such as adding data,
* listing entries, removing entries, and flushing changes to the file.
*
* The repository class uses a header to store metadata about the repository,
* including a magic number, version number, entry size, and entry count. Each
* entry in the repository is represented by the `entry` struct, which contains
* information about the file name, offset, size, and flags indicating the state
* of the entry.
*
* The repository class provides methods for:
* - Opening and creating repository files.
* - Adding data or files to the repository.
* - Reading data from the repository.
* - Listing and finding entries in the repository.
* - Removing entries from the repository.
* - Flushing changes to the file.
*
* The repository class also includes static utility functions for writing data
* to files, calculating offsets, and copying entries between repositories.
*
*/


#pragma once

#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "../gd_types.h"
#include "gd_io_archive.h"

#ifndef _GD_IO_STREAM_BEGIN
#define _GD_IO_STREAM_BEGIN namespace gd { namespace io { namespace stream {
#define _GD_IO_STREAM_END } } }
#endif

_GD_IO_STREAM_BEGIN

constexpr uint64_t uMaxFileNameLength_g = 260; ///< Maximum length of a file name in bytes

/**
 * @class repository
 * @brief Manages file-based repositories.
 *
 * The repository class provides functionality for creating, opening, reading, writing, and managing file-based repositories.
 * It supports operations such as adding data, listing entries, removing entries, and flushing changes to the file.
 *
 * The repository class uses a header to store metadata about the repository, including a magic number, version number, entry size, and entry count.
 * Each entry in the repository is represented by the `entry` struct, which contains information about the file name, offset, size, and flags indicating the state of the entry.
 *
 * The repository class provides methods for:
 * - Opening and creating repository files.
 * - Adding data or files to the repository.
 * - Reading data from the repository.
 * - Listing and finding entries in the repository.
 * - Removing entries from the repository.
 * - Flushing changes to the file.
 *
 * The repository class also includes static utility functions for writing data to files, calculating offsets, and copying entries between repositories.
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
   struct entry;
   // Standard type definitions
   using value_type              = entry;
   using pointer                 = value_type*;
   using const_pointer           = const value_type*;
   using reference               = value_type&;
   using const_reference         = const value_type&;
   using size_type               = std::size_t;
   using difference_type         = std::ptrdiff_t;
   using iterator                = std::vector<entry>::iterator;
   using const_iterator          = std::vector<entry>::const_iterator;
   using reverse_iterator        = std::vector<entry>::reverse_iterator;
   using const_reverse_iterator  = std::vector<entry>::const_reverse_iterator;


public:

   /**
    * @brief Represents the header of the repository.
    *
    * The header contains metadata about the repository, including a magic number,
    * version number, entry size, and entry count. It provides methods to add entries
    * and retrieve the size of the entry.
    */
   struct header
   {
      header() = default;
      /// @brief Construct a header with the specified maximum entry size.
      explicit header(uint64_t uMaxEntrySize) : m_uMaxEntryCount(uMaxEntrySize) {}

      /// @brief set maximum entry size
      void set_max_size(uint64_t uMaxEntrySize) { m_uMaxEntryCount = uMaxEntrySize; }
      /// @brief Increment the entry count, this is the number of used entries.
      void add_entry() { assert( m_uEntryCount < m_uMaxEntryCount ); m_uEntryCount++; }

      /// @brief Get the size of files that entry block is able to store.
      /// @return The size of the entry in number of max number of files.
      uint64_t size() const { return m_uMaxEntryCount; }
      /// @brief Get the number of free entries in the repository. There are limited number of entries.
      uint64_t size_free() const { return m_uMaxEntryCount - m_uEntryCount; }

      /// @brief Get the number of entries in the repository.
      uint64_t count() const { return m_uEntryCount; }

      /// @brief Clear the header, setting the entry count to zero.
      void clear() { m_uEntryCount = 0; }

      /// @brief get the offset to where content starts in the repository file
      uint64_t margin() const { return m_uMarginToContent; }
      /// @brief sets the offset to where content starts in the repository file
      void set_margin(uint64_t uMargin) { m_uMarginToContent = uMargin; }

   // ## attributes -----------------------------------------------------------
      uint64_t m_uMagic = repository::get_magic_number_s(); ///< Magic number, used to identify the repository
      uint64_t m_uVersion = 1;    ///< Version number, used to track changes to the repository format
      uint64_t m_uMaxEntryCount = 10; ///< Maximum number of entries that can be stored
      uint64_t m_uEntryCount = 0; ///< Number of entries used in the repository
      uint64_t m_uMarginToContent = 0; ///< Margin to content, used for padding and can be used as space for more entries
   };

   /**
    * @brief Represents an entry in the repository.
    *
    * Each entry holds information about a file, including the file name, offset,
    * size, and flags indicating the state of the entry (valid, deleted, or removed).
    */
   struct entry
   {
      entry(): m_uOffset(0), m_uSize(0), m_uFlags(0) { std::memset(this, 0, sizeof(entry)); }

      entry(const std::string& name, uint64_t uOffset, uint64_t uSize, unsigned uFlags): m_uOffset(uOffset), m_uSize(uSize), m_uFlags(uFlags), m_dTimeCreate(0.0), m_dTimeAccess(0.0) {
         std::strncpy(m_piszName, name.c_str(), sizeof(m_piszName) - 1);
         m_piszName[sizeof(m_piszName) - 1] = '\0'; // Ensure null-termination
      }
      entry(const entry& o) { copy(o); }

      entry& operator=(const entry& o) { if( this != &o ) { copy(o); } return *this; }

      bool operator==(const entry& o) const { return std::strncmp(m_piszName, o.m_piszName, sizeof(m_piszName)) == 0; }

      void copy(const entry& o) {
         std::strncpy(m_piszName, o.m_piszName, sizeof(m_piszName) - 1);
         m_piszName[sizeof(m_piszName) - 1] = '\0'; // Ensure null-termination
         m_uOffset = o.m_uOffset; m_uSize = o.m_uSize; m_uFlags = o.m_uFlags; m_dTimeCreate = o.m_dTimeCreate; m_dTimeAccess = o.m_dTimeAccess;
      }

      std::string_view get_name() const { return std::string_view(m_piszName); }
      void set_name(const std::string_view& stringName) { 
         assert( stringName.length() < uMaxFileNameLength_g ); std::strncpy(m_piszName, stringName.data(), sizeof(m_piszName) - 1); m_piszName[uMaxFileNameLength_g - 1] = '\0'; }

      void set_offset(uint64_t uOffset) { m_uOffset = uOffset; }

      bool is_valid() const   { return ( m_uFlags & eEntryFlagValid ) != 0; }
      bool is_deleted() const { return ( m_uFlags & eEntryFlagDeleted ) != 0; }
      bool is_remove() const  { return ( m_uFlags & eEntryFlagRemove ) != 0; }

      void set_valid()        { m_uFlags |= eEntryFlagValid; }
      void set_deleted()      { m_uFlags |= eEntryFlagDeleted; }
      void set_remove()       { m_uFlags |= eEntryFlagRemove; }

      uint64_t offset() const { return m_uOffset; }
      uint64_t size() const { return m_uSize; }
      uint64_t offset_end() const { return offset() + size(); }

   // ## attributes -----------------------------------------------------------
      char m_piszName[uMaxFileNameLength_g]; ///< File name
      uint64_t m_uOffset;        ///< Offset in archive file, where the file content starts and zero is the entries end, not the file start
      uint64_t m_uSize;          ///< Size of file content
      double   m_dTimeCreate;    ///< Time of creation
      double   m_dTimeAccess;    ///< Time of last access
      unsigned m_uFlags;         ///< Entry validity flag, each entry can be valid, deleted or marked for removal
   };

// ## construction -------------------------------------------------------------
public:
   repository(): m_pFile(nullptr) {}
   explicit repository(std::size_t uMaxFileCount): m_pFile(nullptr), m_header(uMaxFileCount) {}
   repository(const std::string_view& stringPath): m_stringRepositoryPath( file_make_preffered_s( std::string(stringPath) ) ), m_pFile(nullptr) {}
   repository(const std::string_view& stringPath, std::size_t uMaxFileCount): m_stringRepositoryPath( file_make_preffered_s( std::string(stringPath) ) ), m_pFile(nullptr), m_header(uMaxFileCount) {}
   // copy
   repository(const header& o): m_header(o), m_pFile(nullptr) {}
   repository(const repository& o) { common_construct(o); }
   repository(repository&& o) noexcept { common_construct(std::move(o)); }
   // assign
   repository& operator=(const repository& o) { common_construct(o); return *this; }
   repository& operator=(repository&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~repository() { close(); }
private:
   // common copy
   void common_construct(const repository& o) {
      m_header = o.m_header;
      m_pFile = nullptr;      // do not copy file handle
      m_stringRepositoryPath = o.m_stringRepositoryPath;
      m_vectorEntry = o.m_vectorEntry;
   }
   void common_construct(repository&& o) noexcept {
      m_header = std::move(o.m_header);
      m_pFile = o.m_pFile;
      m_stringRepositoryPath = std::move(o.m_stringRepositoryPath);
      m_vectorEntry = std::move(o.m_vectorEntry);
      o.m_pFile = nullptr;
   }

// ## operator -----------------------------------------------------------------
public:
   repository::entry& operator[](std::size_t uIndex) { return m_vectorEntry[uIndex]; }
   const repository::entry& operator[](std::size_t uIndex) const { return m_vectorEntry[uIndex]; }
   repository::entry& operator[](const std::string_view& stringName) { assert(find_entry(stringName) != nullptr); return *find_entry(stringName); }


// ## methods ------------------------------------------------------------------
public:
/** \name GET/SET
*///@{
   /// @brief Check if the repository file is open.
   bool is_open() const { return m_pFile != nullptr; }
   /// @brief Get the path to the repository file.
   const std::string& get_path() const { return m_stringRepositoryPath; }
   /// @brief Get the path to the repository file.
   const std::string& get_temporary_path() const { return m_stringTemporaryPath.empty() == false ? m_stringRepositoryPath : m_stringRepositoryPath; }
   /// @brief Get the header of the repository.
   const header& get_header() const { return m_header; }
   /// @brief Get the margin space between the the end of entries and the start of content.
   uint64_t margin() const { return m_header.margin(); }
//@}

/** \name OPERATION
*///@{
   // ## open file

   std::pair<bool, std::string> open();
   std::pair<bool, std::string> open(const std::string_view& stringPath);
   std::pair<bool, std::string> open(const std::string_view& stringPath, const std::string_view& stringMode );

   // ## create repository file

   std::pair<bool, std::string> create(const std::string_view& stringPath);
   std::pair<bool, std::string> create();

   // ## add data or files to repository
   
   std::pair<bool, std::string> add(const std::string_view& stringName, const void* pdata, uint64_t uSize);
   std::pair<bool, std::string> add(const std::string_view& stringFile);
   std::pair<bool, std::string> add(const std::string_view& stringFile, const std::string_view& stringName);
   /// @brief Add an entry to the internal vector and update header repository. No data is written to the file.
   void add_entry(const entry& entry_) { m_vectorEntry.push_back(entry_); m_header.add_entry(); }

   // ## updates internal data in repository to file

   std::pair<bool, std::string> flush();

   /// @brief Expand the repository to hold more entries, one file is one entry.
   std::pair<bool, std::string> expand( uint64_t uCount, uint64_t uBuffer );
   /// @brief Expand the repository to hold more entries and use temporary file to store data
   std::pair<bool, std::string> expand(uint64_t uCount) { return expand(uCount, 0); }
   /// @brief Expand the repository to hold more entries by rotating file content. The lowest part is moved to top and entries are recalculated.
   //std::pair<bool, std::string> expand_rotate(uint64_t uCount);

   // ## read data from repository

   std::pair<bool, std::string> read( const std::string_view& stringName, void* pdata, uint64_t uSize, uint64_t* puReadSize) const;
   std::pair<bool, std::string> read( const std::string_view& stringName, std::vector<uint8_t>& vectorContent ) const;
   std::pair<bool, std::string> read( const std::string_view& stringName, std::string& stringContent) const;
   std::vector<uint8_t> read( const std::variant<size_t,std::string_view>& index_, gd::types::tag_vector ) const;
   std::pair<bool, std::string> read( size_t uIndex, void* pdata, uint64_t uSize, uint64_t* puReadSize) const;
   std::pair<bool, std::string> read( size_t uIndex, std::vector<uint8_t>& vectorContent ) const;
   std::pair<bool, std::string> read( size_t uIndex, std::string& stringContent ) const;
   std::string read( const std::variant<size_t,std::string_view>& index_, gd::types::tag_string ) const;
   std::pair<bool, std::string> read_to_file(const std::string_view& stringName, const std::string_view& stringPath) const;

   // ## information about repository

   int64_t find(const std::string_view& stringName) const;
   bool exists(const std::string_view& stringName) const { return find(stringName) != -1; }
   entry* find_entry(const std::string_view& stringName);
   const entry* find_entry(const std::string_view& stringName) const;

   std::vector<std::string> list() const;
   size_t size() const { return m_vectorEntry.size(); }
   size_t size_reserved() const { return m_header.size(); }
   bool empty() const { return m_vectorEntry.empty(); }

   // ## remove data from repository

   std::pair<bool, std::string> remove( const std::string_view& stringName );
   void remove( std::size_t uIndex );

   void remove_entry() { m_vectorEntry.clear(); }
   std::pair<bool, std::string> remove_entry_from_file();
   std::pair<bool, std::string> remove_entry_from_file( const std::vector<uint64_t>& vectorIndexes );

   // ## close repository

   void close();

   // ## iterator methods

   std::vector<entry>::iterator begin() { return m_vectorEntry.begin(); }
   std::vector<entry>::iterator end() { return m_vectorEntry.end(); }
   std::vector<entry>::const_iterator begin() const { return m_vectorEntry.begin(); }
   std::vector<entry>::const_iterator end() const { return m_vectorEntry.end(); }
   std::vector<entry>::const_iterator cbegin() const { return m_vectorEntry.cbegin(); }
   std::vector<entry>::const_iterator cend() const { return m_vectorEntry.cend(); }

//@}

protected:
/** \name INTERNAL
*///@{

//@}

public:
/** \name DEBUG
*///@{
   /// @brief Dump the repository to a string for debugging purposes.
   std::string dump() const;
//@}


// ## attributes ----------------------------------------------------------------
public:
   header m_header;                    ///< Header of repository
   FILE* m_pFile;                      ///< File handle for archive
   std::string m_stringRepositoryPath; ///< Path to archive file
   std::string m_stringTemporaryPath;  ///< Path to folder where temporary files are generated, if not set same as repository file
   std::vector<entry> m_vectorEntry;   ///< Index of files

   inline static std::string m_stringRepositoryExtension_s = "repo"; ///< Default extension for repository file
   inline static std::string m_stringTemporaryExtension_s = "tmp"; ///< Default extension for temporary file


// ## free functions ------------------------------------------------------------
public:
   // ## file operations
   /// @brief prepare string path for current platform
   static std::string file_make_preffered_s(const std::string& stringPath);
   /// @brief create a new temporary file
   static std::pair<bool, std::string> file_new_tempoary_s(const repository& repository_, std::string& stringTemporaryFile, bool bOpen );

   // ## file extension
   /// @brief set the repository extension, this extension is used to identify the repository file
   static void extension_set_repository_s( const std::string_view& stringExstension ) { m_stringRepositoryExtension_s = stringExstension; }
   /// @brief set the temporary extension
   static void extension_set_temporary_s( const std::string_view& stringExstension ) { m_stringTemporaryExtension_s = stringExstension; }

   // ## calculate position
   /// @brief calculate position of entry block in repository file
   static uint64_t calculte_entry_offset_s() { return (uint64_t)sizeof( header ); }
   /// @brief calculate position of content block in repository file, this is the first position wher file content is stored
   /// `calculte_file_offset_s` is same as `calculte_content_offset_s`
   static uint64_t calculte_file_offset_s( const repository& repository_ );
   static uint64_t calculte_content_offset_s(const repository& repository_) { return calculte_file_offset_s(repository_); }
   /// @brief calculate the first position of content in repository file
   static uint64_t calculate_first_content_position_s(const repository& repository_);
   /// @brief calculate the first position of content in repository file
   static uint64_t calculate_first_free_content_position_s(const repository& repository_);

   // ## read
   /// @brief read repository file, internal data in repository is updated
   static std::pair<bool, std::string> read_s(repository& repository_);
   /// @brief read repository header from file
   static std::pair<bool, std::string> read_header_s(FILE* pfile, header& header_);
   /// @brief read entry block from file
   static std::pair<bool, std::string> read_entry_block_s(FILE* pfile, std::vector<entry>& vectorEntry, uint64_t uSize, uint64_t uOffset);
   /// @brief read content from file
   static std::pair<bool, std::string> read_content_from_file_s(repository& repository_, const std::string_view& stringInputPath);
   /// @brief read content from buffer and write to repository file
   static std::pair<bool, std::string> read_content_from_buffer_s(repository& repository_, const void* pBuffer, uint64_t uSize );
   static std::pair<bool, std::string> read_content_from_buffer_s(repository& repository_, const std::vector<uint8_t>& vectorBuffer) { return read_content_from_buffer_s(repository_, vectorBuffer.data(), vectorBuffer.size()); }

   // ## write
   /// @brief write data to file, like a wrapper for c `fwrite` command
   static size_t write_s(repository& repository_, const void* pdata_, size_t uCount);
   /// @brief write header to file
   static std::pair<bool, std::string> write_header_s(FILE* pfile, const header& header_);
   /// @brief write entry block to file
   static std::pair<bool, std::string> write_entry_block_s(FILE* pfile, const void* pdata, uint64_t uSize, uint64_t uOffset);
   /// @brief fill block with fill value
   static std::pair<bool, std::string> write_block_s(FILE* pfile, uint8_t uFillValue, uint64_t uSize, uint64_t uOffset);
   /// @brief write content to file
   static std::pair<bool, std::string> write_content_to_file_s(const repository& repository_, const std::string_view& stringOutputPath);
   static std::pair<bool, std::string> write_content_to_file_s(const repository& repository_) { return write_content_to_file_s(repository_, ""); }
   /// @brief write content data from the repository in to a buffer
   static std::pair<bool, std::string> write_content_to_buffer_s(const repository& repository_, std::vector<uint8_t>& vectorBuffer);

   // ## size
   /// @brief get size of header in repository file
   static uint64_t size_header_s() { return sizeof(header); }
   /// @brief get size of entry buffer, this is the size of all entries in the repository
   static uint64_t size_entry_buffer_s(const repository& repository_) { return repository_.m_vectorEntry.size() * sizeof(entry); }
   /// get size of reserved buffer for entry, thish is the maximum buffer size of entries that can be stored
   static uint64_t size_entry_reserved_buffer_s(const repository& repository_) { return repository_.m_header.size() * sizeof( entry ); }
   /// get size of all data in repository, header, entry buffer and content
   static uint64_t size_all_s(const repository& repository_) { return calculate_first_free_content_position_s(repository_); }
   /// @brief get magic number used to identify repository file
   static constexpr uint64_t get_magic_number_s();

   /// @brief copy entries from one repository to another
   static void copy_entries_s(repository& repositoryTo, const repository& repositoryFrom);

};

/// create repository file
inline std::pair<bool, std::string> repository::create(const std::string_view& stringPath) {
   close();
   m_stringRepositoryPath = stringPath;
   return create();
}

/// read repository file into vector
inline std::pair<bool, std::string> repository::read(const std::string_view& stringName, std::string& stringContent) const {
   auto iIndex = find(stringName);
   if( iIndex == -1 ) return { false, std::string( "file not found: " ) + stringName.data() };
   auto uSize = m_vectorEntry[iIndex].size();                                                      assert(uSize != 0);
   stringContent.resize(uSize);
   return read(iIndex, stringContent.data(), uSize, nullptr);
}

/// read repository file into vector
inline std::pair<bool, std::string> repository::read(const std::string_view& stringName, std::vector<uint8_t>& vectorContent) const {
   auto iIndex = find(stringName);
   if( iIndex == -1 ) return { false, std::string( "file not found: " ) + stringName.data() };
   auto uSize = m_vectorEntry[iIndex].size();                                                      assert(uSize != 0);
   vectorContent.resize(uSize);
   return read(iIndex, vectorContent.data(), uSize, nullptr);
}

/// read repository file into vector
inline std::vector<uint8_t> repository::read(const std::variant<size_t, std::string_view>& index_, gd::types::tag_vector) const {
   std::vector<uint8_t> vectorContent;
   if( index_.index() == 0 ) {
      auto uIndex = std::get<size_t>(index_);
      read( uIndex, vectorContent );
   }
   else if( index_.index() == 1 ) {
      auto stringName = std::get<std::string_view>(index_);
      read( stringName, vectorContent );
   }
   return vectorContent;
}

/// read repository file into vector
inline std::pair<bool, std::string> repository::read(size_t uIndex, std::vector<uint8_t>& vectorContent) const {
   auto uSize = m_vectorEntry[uIndex].size();                                                      assert(uSize != 0);
   vectorContent.resize(uSize);
   return read(uIndex, vectorContent.data(), uSize, nullptr);
}

/// read repository file into vector
inline std::pair<bool, std::string> repository::read(size_t uIndex, std::string& stringContent) const {
   auto uSize = m_vectorEntry[uIndex].size();                                                      assert(uSize != 0);
   stringContent.resize(uSize);
   return read(uIndex, stringContent.data(), uSize, nullptr);
}

/// read repository file into string
inline std::string repository::read( const std::variant<size_t, std::string_view>& index_, gd::types::tag_string ) const {
   std::string stringContent;
   if( index_.index() == 0 ) {
      auto uIndex = std::get<size_t>(index_);
      read( uIndex, stringContent );
   }
   else if( index_.index() == 1 ) {
      auto stringName = std::get<std::string_view>(index_);
      read( stringName, stringContent );
   }
   return stringContent;
}




inline uint64_t repository::calculate_first_content_position_s(const repository& repository_) { 
   uint64_t uOffset = calculte_file_offset_s( repository_ );
   return uOffset; 
}

/// calculate position of content in repository file
/// this is the first position wher file content is stored
/// If `content_offset` is set to 0, it will be calculated as the size of the header and entry block
/// and the size of the reserved buffer for entries
/// if `content_offset` is set to a value, it will be used as the content offset
inline uint64_t repository::calculte_file_offset_s( const repository& repository_ ) { 
   return calculte_entry_offset_s() + repository_.size_reserved() * sizeof( entry ) + repository_.margin(); 
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

/// wrapper for c `fwrite` command
inline size_t repository::write_s(repository& repository_, const void* pdata_, size_t uCount) {
   auto uSize = fwrite(pdata_, 1, uCount, repository_.m_pFile);
   fflush(repository_.m_pFile);
   return uSize;
}




_GD_IO_STREAM_END