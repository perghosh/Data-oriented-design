#pragma once

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
// ## construction -------------------------------------------------------------
public:
   repository() {}
   // copy
   repository(const repository& o) { common_construct(o); }
   repository(repository&& o) noexcept { common_construct(std::move(o)); }
   // assign
   repository& operator=(const repository& o) { common_construct(o); return *this; }
   repository& operator=(repository&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~repository() {}
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


// ## free functions ------------------------------------------------------------
public:



};


/**
 * @class repository
 * @brief Manages multiple files within a single archive file with an index
 */
class repository
{
private:
    /**
     * @struct file_entry
     * @brief Represents a single file entry in the archive index
     */
    struct file_entry
    {
        char m_name[256];          ///< File name
        uint64_t m_offset;         ///< Offset in archive file
        uint64_t m_size;           ///< Size of file content
        bool m_isValid;            ///< Entry validity flag
    };

    FILE* m_pFile;                 ///< File handle for archive
    std::string m_archivePath;     ///< Path to archive file
    std::vector<file_entry> m_entries; ///< Index of files
    uint64_t m_indexSize;          ///< Size of index in bytes

    /**
     * @brief Updates the index in the archive file
     */
    void updateindex()
    {
        if (!m_pFile)
            return;

        fseek(m_pFile, 0, SEEK_SET);
        uint32_t entryCount = static_cast<uint32_t>(m_entries.size());
        fwrite(&entryCount, sizeof(uint32_t), 1, m_pFile);
        
        for (const auto& entry : m_entries)
        {
            fwrite(&entry, sizeof(file_entry), 1, m_pFile);
        }
        
        m_indexSize = sizeof(uint32_t) + (entryCount * sizeof(file_entry));
    }

    /**
     * @brief Loads existing index from archive file
     */
    void loadindex()
    {
        if (!m_pFile)
            return;

        fseek(m_pFile, 0, SEEK_SET);
        uint32_t entryCount;
        fread(&entryCount, sizeof(uint32_t), 1, m_pFile);
        
        m_entries.resize(entryCount);
        fread(m_entries.data(), sizeof(file_entry), entryCount, m_pFile);
        
        m_indexSize = sizeof(uint32_t) + (entryCount * sizeof(file_entry));
    }

public:
    /**
     * @brief Constructor
     * @param path Path to archive file
     */
    repository(const std::string& path)
        : m_pFile(nullptr)
        , m_archivePath(path)
        , m_indexSize(0)
    {
        m_pFile = fopen(path.c_str(), "r+b");
        if (!m_pFile)
        {
            // Create new file if it doesn't exist
            m_pFile = fopen(path.c_str(), "w+b");
            if (m_pFile)
            {
                uint32_t zero = 0;
                fwrite(&zero, sizeof(uint32_t), 1, m_pFile);
            }
        }
        
        if (m_pFile)
        {
            loadindex();
        }
    }

    /**
     * @brief Destructor
     */
    ~repository()
    {
        if (m_pFile)
        {
            fclose(m_pFile);
        }
    }

    /**
     * @brief Adds a file to the archive
     * @param filename Name to store file as
     * @param data File content
     * @param size Size of file content
     * @return Success status
     */
    bool addfile(const std::string& filename, const void* data, uint64_t size)
    {
        if (!m_pFile || filename.length() >= 256)
            return false;

        // Find position at end of archive
        fseek(m_pFile, 0, SEEK_END);
        uint64_t offset = ftell(m_pFile);

        // Write file content
        fwrite(data, 1, size, m_pFile);

        // Add to index
        file_entry entry{};
        strncpy(entry.m_name, filename.c_str(), 255);
        entry.m_offset = offset;
        entry.m_size = size;
        entry.m_isValid = true;
        
        m_entries.push_back(entry);
        updateindex();
        
        return true;
    }

    /**
     * @brief Removes a file from the archive
     * @param filename Name of file to remove
     * @return Success status
     */
    bool removefile(const std::string& filename)
    {
        if (!m_pFile)
            return false;

        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&filename](const file_entry& entry) {
                return entry.m_isValid && filename == entry.m_name;
            });

        if (it == m_entries.end())
            return false;

        it->m_isValid = false;
        updateindex();
        
        return true;
    }

    /**
     * @brief Extracts a file from the archive
     * @param filename Name of file to extract
     * @param buffer Output buffer for file content
     * @param bufferSize Size of output buffer
     * @return Size of extracted file or 0 if failed
     */
    uint64_t extractfile(const std::string& filename, void* buffer, uint64_t bufferSize)
    {
        if (!m_pFile)
            return 0;

        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [&filename](const file_entry& entry) {
                return entry.m_isValid && filename == entry.m_name;
            });

        if (it == m_entries.end() || bufferSize < it->m_size)
            return 0;

        fseek(m_pFile, static_cast<long>(it->m_offset), SEEK_SET);
        fread(buffer, 1, it->m_size, m_pFile);
        
        return it->m_size;
    }

    /**
     * @brief Gets list of files in archive
     * @return Vector of file names
     */
    std::vector<std::string> getfilelist()
    {
        std::vector<std::string> files;
        for (const auto& entry : m_entries)
        {
            if (entry.m_isValid)
            {
                files.push_back(entry.m_name);
            }
        }
        return files;
    }
};


_GD_IO_STREAM_END