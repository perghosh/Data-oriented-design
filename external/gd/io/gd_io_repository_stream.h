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
   enum enumEntryFlag
   {
      eEntryFlagNone       = 0x0000,   ///< No flags
      eEntryFlagValid      = 0x0001,   ///< Entry is valid
      eEntryFlagDeleted    = 0x0002,   ///< Entry is deleted
   };
public:
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

      void set_valid() { m_uFlags |= eEntryFlagValid; }
      void set_deleted() { m_uFlags |= eEntryFlagDeleted; }

      uint64_t offset() const { return m_uOffset; }
      uint64_t size() const { return m_uSize; }

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
   std::pair<bool, std::string> open(const std::string_view& stringPath);
   std::pair<bool, std::string> open(const std::string_view& stringPath, const std::string_view& stringMode );

   std::pair<bool, std::string> add(const std::string_view& stringName, const void* pdata, uint64_t uSize);

   std::pair<bool, std::string> read(const std::string_view& stringName, void* pdata, uint64_t uSize) const;
   std::pair<bool, std::string> read_to_file(const std::string_view& stringName, const std::string_view& stringPath) const;

   std::vector<std::string> list() const;

   std::pair<bool, std::string> remove( const std::string_view& stringName );
   void remove( std::size_t uIndex );

   std::pair<bool, std::string> remove_entry_from_file(const std::string_view& stringName);

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
   FILE* m_pFile;                 ///< File handle for archive
   std::string m_stringRepositoryPath;///< Path to archive file
   std::vector<entry> m_vectorEntry;///< Index of files
   uint64_t m_uEntrySize;             ///< Size of index in bytes


// ## free functions ------------------------------------------------------------
public:

};





_GD_IO_STREAM_END