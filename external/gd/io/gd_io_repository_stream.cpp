/**
* @file gd_io_repository_stream.cpp
* @brief Implements the repository class for managing file-based repositories.
*
*/

#include <cstdio>
#include <algorithm>
#include <filesystem>

#include "gd_io_repository_stream.h"

#ifdef _WIN32
    #define fseek_64_ _fseeki64
    #define offset_t int64_t
#else
    #define fseek_64_ fseeko
    #define offset_t off_t
#endif

_GD_IO_STREAM_BEGIN


/** ---------------------------------------------------------------------------
 * @brief Opens a file at the specified path with a default mode of "w+b".
 *
 * This is a convenience overload that calls the two-parameter version of open()
 * with the default mode "w+b" (write and read, binary mode, creating the file if it doesn't exist).
 *
 * @param stringPath The path to the file to be opened, provided as a string view.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 */
std::pair<bool, std::string> repository::open(const std::string_view& stringPath) 
{
   if( std::filesystem::exists(stringPath) == false ) return create(stringPath);

   return open(stringPath, "r+b");
}

/** ---------------------------------------------------------------------------
 * @brief Opens a file at the specified path with a given mode.
 *
 * Attempts to open a file using the provided path and mode. If successful, the file pointer
 * (m_pFile) is set, and the repository path (m_stringRepositoryPath) is updated. If the file
 * cannot be opened, an error message is returned.
 *
 * @param stringPath The path to the file to be opened, provided as a string view.
 * @param stringMode The mode in which to open the file (e.g., "r", "w", "w+b"), provided as a string view.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 * @retval {true, ""} If the file was successfully opened.
 * @retval {false, "Failed to open file: <path>"} If the file could not be opened.
 */
std::pair<bool, std::string> repository::open(const std::string_view& stringPath, const std::string_view& stringMode) 
{
   close();
   m_pFile = fopen(stringPath.data(), stringMode.data());
   if( m_pFile == nullptr ) { return {false, std::string( "Failed to open file: " ) + stringPath.data()}; }


   m_stringRepositoryPath = stringPath;

   // ## Read the header
   auto result_ = read_header_s(m_pFile, m_header);
   if( result_.first == false ) { close(); return result_; }

   // ## Read the entry block
   m_vectorEntry.clear();
   m_vectorEntry.reserve(m_header.count());
   result_ = read_entry_block_s(m_pFile, m_vectorEntry, m_header.count() * sizeof( entry ), sizeof( m_header ) );
   if( result_.first == false ) { close(); return result_; }

   return {true, ""};
}

std::pair<bool, std::string> repository::open()
{
   return open(m_stringRepositoryPath);
}

/** ---------------------------------------------------------------------------
 * @brief Creates a new repository file and initializes it with a header and reserved entry space.
 *
 * This method attempts to create a new repository file at the path specified by `m_stringRepositoryPath`.
 * It performs the following steps:
 * 1. Closes any existing file associated with the repository.
 * 2. Opens a new file for writing in binary mode.
 * 3. Writes the repository header to the file.
 * 4. Reserves space for the entry block by writing a block of zeros.
 *
 * If any of these steps fail, the method returns a pair with `false` and an error message.
 * If all steps succeed, it returns `true` and an empty string.
 *
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 */
std::pair<bool, std::string> repository::create() 
{
   close();                                                                   // Close any existing file

   // Open the file for writing
   m_pFile = std::fopen(m_stringRepositoryPath.c_str(), "w+b");
   if (!m_pFile) { return {false, "Failed to create file: " + m_stringRepositoryPath}; }

   auto result_ = write_header_s(m_pFile, m_header);                           // write the header to the file, place it at the beginning
   if( result_.first == false ) { close(); return result_; }

   uint64_t uEntryOffset = calculte_entry_offset_s();                          // Calculate the offset for the entry block
   uint64_t uEntrySize = size_entry_reserved_buffer_s(*this);                  // Calculate the size of the entry block 
   result_ = write_block_s(m_pFile, 0, uEntrySize, uEntryOffset);              // fill the entry block with zeros
   if( result_.first == false ) { close(); return result_; }

   return {true, ""};
}



/** ---------------------------------------------------------------------------
 * @brief Adds data to the repository with the specified name.
 *
 * Writes the provided data to the end of the file and records an entry in the repository.
 * The name length must be less than 260 characters, and the file must be open.
 *
 * @param stringName The name to associate with the data, provided as a string view.
 * @param pdata_ Pointer to the data to be written.
 * @param uSize Size of the data in bytes.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 * @retval {true, ""} If the data was successfully added.
 * @retval {false, "Invalid file or name too long: <name>"} If the file is not open or the name exceeds the maximum length.
 */
std::pair<bool, std::string> repository::add(const std::string_view& stringName, const void* pdata_, uint64_t uSize)
{                                                                                                  assert( stringName.length() < 260 ); assert( m_pFile != nullptr );
   if(!m_pFile || stringName.length() >= sizeof(entry::m_piszName)) { return {false, std::string("Invalid file or name too long: ") + stringName.data()}; }

   auto uStartOffset = calculate_first_free_content_position_s(*this);          // Calculate the start offset for the new data (end of the file)

   fseek_64_(m_pFile, 0, SEEK_END);                                            // Move to the end of the file
   uint64_t uOffset = ftell(m_pFile);                                          // Get the current file position as the offset
                                                                                                   assert( uOffset == uStartOffset );

   auto uBytesWritten = fwrite(pdata_, 1, uSize, m_pFile);
   if( uBytesWritten != uSize ) { return { false, "Failed to write data to file" }; }

   // ## Subtract the first position of content in repository file with the size of reposytory header and entry block
   auto uFirstPosition = calculate_first_content_position_s(*this);
   uStartOffset -= uFirstPosition;

   // Create a new entry and add it to the repository
   entry entry_( stringName.data(), uStartOffset, uSize, eEntryFlagValid );
   m_vectorEntry.push_back(entry_);
   m_header.add_entry();

   return {true, ""};
}

/** --
 * @brief Adds a file to the repository by reading its contents from the filesystem.
 *
 * This method opens the specified file in binary mode, reads its contents into a buffer,
 * extracts the filename from the path, and delegates to the `add` overload that accepts
 * a name, data pointer, and size. If any step fails (e.g., file opening or reading),
 * an error message is returned.
 *
 * @param stringFile The path to the file to be added (as a string_view).
 * @param stringName The name to associate with the file (as a string_view). if empty, the filename is used.
 * @return A pair containing:
 *         - `bool`: `true` if the file was successfully added, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @note The file is opened in binary mode to preserve exact contents.
 * @note The filename is extracted using `std::filesystem::path` for portability.
 *
 * @par Example:
 * @code
 * repository repo;
 * auto result = repo.add("example.txt");
 * if (result.first) {
 *     std::cout << "File added successfully\n";
 * } else {
 *     std::cout << "Error: " << result.second << "\n";
 * }
 * @endcode
 */
std::pair<bool, std::string> repository::add(const std::string_view& stringFile, const std::string_view& stringName)
{
   std::string stringName_ = stringName.empty() ? std::filesystem::path(stringFile).filename().string() : stringName.data();

   std::ifstream ifstreamFile(stringFile.data(), std::ios::binary | std::ios::ate);
   if (!ifstreamFile) { return {false, "Failed to open input file"}; }

   std::streamsize uSize = ifstreamFile.tellg();
   ifstreamFile.seekg(0, std::ios::beg);

   std::vector<char> vectorBuffer(uSize);
   if( !ifstreamFile.read(vectorBuffer.data(), uSize) ) { return {false, std::string("Failed to read input file") + stringFile.data()}; }

   return add(stringName_, vectorBuffer.data(), static_cast<uint64_t>(uSize));
}

/// Adds a file to the repository by reading its contents from the and using the filename as the name.
/// Check `add(const std::string_view&, const std::string_view&)` for more details.
std::pair<bool, std::string> repository::add(const std::string_view& stringFile)
{
   return add(stringFile, std::string_view());
}


/** ---------------------------------------------------------------------------
 * @brief Writes the repository's header and entry block to the underlying file.
 *
 * This method ensures that the in-memory state of the repository (header and entries)
 * is persisted to the file. It writes the header first, followed by the entry block
 * at the calculated offset. If any write operation fails, the file is closed, and
 * an error is returned.
 *
 * @return A pair containing:
 *         - `bool`: `true` if the flush operation succeeded, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure.
 *
 * @par Example:
 * @code
 * repository repo;
 * repo.create("archive.repo"); // Assume this opens m_pFile
 * repo.add("test.txt");        // Add a file
 * auto result = repo.flush();
 * if (result.first) {
 *     std::cout << "Repository flushed successfully\n";
 * } else {
 *     std::cout << "Flush failed: " << result.second << "\n";
 * }
 * @endcode
 */
std::pair<bool, std::string> repository::flush() 
{                                                                                                  assert( m_pFile != nullptr );
   // Write header to file
   auto result_ = write_header_s(m_pFile, m_header);
   if( result_.first == false ) { close(); return result_; }

   uint64_t uEntryOffset = calculte_entry_offset_s();
   uint64_t uEntrySize = size_entry_buffer_s(*this);

   result_ = write_entry_block_s(m_pFile, m_vectorEntry.data(), uEntrySize, uEntryOffset);
   if( result_.first == false ) { close(); return result_; }

   fflush(m_pFile);

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Reads data from the repository associated with the specified name into a buffer.
 *
 * Searches for an entry with the given name and reads its data into the provided buffer.
 * The buffer must be large enough to hold the data.
 *
 * @param stringName The name of the data to read, provided as a string view.
 * @param pdata Pointer to the buffer where the data will be stored.
 * @param uSize Size of the buffer in bytes.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error or success message.
 * @retval {false, "File not found or buffer too small"} If no matching entry is found or the buffer is insufficient.
 */
std::pair<bool, std::string> repository::read(const std::string_view& stringName, void* pdata, uint64_t uSize) const
{                                                                                                  assert( m_pFile != nullptr );
   auto it = std::find_if(m_vectorEntry.begin(), m_vectorEntry.end(),
      [&stringName](const entry& e) { return e.is_valid() && e.get_name() == stringName; });

   if(it == m_vectorEntry.end() || uSize < it->size()) { return {false, "File not found or buffer too small"}; }

   fseek_64_(m_pFile, static_cast<long>(it->m_uOffset), SEEK_SET);
   fread(pdata, 1, it->m_uSize, m_pFile);

   return { true, "" };
}

#include <sys/types.h>

/** ---------------------------------------------------------------------------
 * @brief Reads data from the repository and writes it to a file at the specified path.
 *
 * Searches for an entry with the given name and saves its data to a new file at the provided path.
 *
 * @param stringName The name of the data to read, provided as a string view.
 * @param stringPath The path where the data will be written, provided as a string view.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 * @retval {false, "File not found: <name>"} If no matching entry is found.
 * @retval {false, "Failed to open output file"} If the output file could not be opened.
 * @retval {false, "Failed to write to output file: <path>"} If writing to the file failed.
 */
std::pair<bool, std::string> repository::read_to_file(const std::string_view& stringName, const std::string_view& stringPath) const
{                                                                                                  assert( m_pFile != nullptr );
   auto it = std::find_if(m_vectorEntry.begin(), m_vectorEntry.end(),
      [&stringName](const entry& e) { return e.is_valid() && e.get_name() == stringName; });

   if(it == m_vectorEntry.end()) { return {false, std::string( "File not found: " ) + stringName.data()}; }

   std::ofstream ofstreamFile(stringPath.data(), std::ios::binary);
   if(!ofstreamFile) { return {false, "Failed to open output file"}; }

   // ## Prepare data for read and write
   std::vector<uint8_t> vectorBuffer( it->size() );
   uint64_t uBeginPosition = calculate_first_content_position_s(*this);
   uBeginPosition += it->offset();

   // ### read data from repository
   fseek_64_(m_pFile, uBeginPosition, SEEK_SET);
   fread(vectorBuffer.data(), 1, it->m_uSize, m_pFile);

   // ### write data to file
   ofstreamFile.write((const char*)vectorBuffer.data(), it->size());
   if(!ofstreamFile) { return {false, std::string("Failed to write to output file: ") + stringPath.data()}; }

   return {true, ""};
}


/** ---------------------------------------------------------------------------
 * @brief Retrieves a list of valid entry names in the repository.
 *
 * Returns a vector of strings containing the names of all valid entries currently stored
 * in the repository.
 *
 * @return A vector of strings representing the names of valid entries.
 */
std::vector<std::string> repository::list() const
{
   std::vector<std::string> vectorList;
   for( const auto& entry_ : m_vectorEntry)
   {
      if(entry_.is_valid())
      {
         vectorList.push_back( std::string( entry_.get_name()) );
      }
   }
   return vectorList;
}

/** ---------------------------------------------------------------------------
 * @brief Finds the index of an entry in the repository by name.
 *
 * Searches through the in-memory vector of entries (m_vectorEntry) to find the first entry
 * that matches the specified name and is valid. Returns the index of the found entry or -1
 * if no matching entry is found.
 *
 * @param stringName The name of the entry to find, provided as a string view.
 * @return The index of the found entry as an int64_t, or -1 if no matching valid entry is found.
 */
int64_t repository::find(const std::string_view& stringName) const
{
   auto it = std::find_if(m_vectorEntry.begin(), m_vectorEntry.end(), [&stringName](const entry& e) 
      { 
         return e.is_valid() && e.get_name() == stringName; 
      }
   );

   if( it == m_vectorEntry.end() ) { return -1; }
 
   return std::distance(m_vectorEntry.begin(), it);
}

/// Finds an entry in the repository by name and returns a pointer to it.
repository::entry* repository::find_entry(const std::string_view& stringName) 
{
   for( auto& entry_ : m_vectorEntry ) 
   {
      if( entry_.get_name() == stringName )
      {
         return &entry_;
      }
   }
   return nullptr;
}

/// Finds an entry in the repository by name and returns a pointer to it.
const repository::entry* repository::find_entry(const std::string_view& stringName) const
{
   for( const auto& entry_ : m_vectorEntry ) 
   {
      if( entry_.get_name() == stringName )
      {
         return &entry_;
      }
   }
   return nullptr;
}


/** ---------------------------------------------------------------------------
 * @brief Marks an entry with the specified name as deleted.
 *
 * Searches for an entry with the given name and marks it as deleted if found.
 *
 * @param stringName The name of the entry to remove, provided as a string view.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 * @retval {true, ""} If the entry was successfully marked as deleted.
 * @retval {false, "File not found: <name>"} If no matching entry is found.
 */
std::pair<bool, std::string> repository::remove(const std::string_view& stringName)
{
   auto it = std::find_if(m_vectorEntry.begin(), m_vectorEntry.end(),
      [&stringName](const entry& e) { return e.is_valid() && e.get_name() == stringName; });

   if(it == m_vectorEntry.end()) { return {false, std::string( "File not found: " ) + stringName.data()}; }

   it->set_deleted();

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Marks an entry at the specified index as deleted.
 *
 * Marks the entry at the given index in the repository as deleted, if the index is valid.
 *
 * @param uIndex The index of the entry to remove.
 * @note The index must be less than the size of the entry vector, enforced by an assertion.
 */
void repository::remove(std::size_t uIndex)
{                                                                                                  assert(uIndex < m_vectorEntry.size());                                      
   if(uIndex < m_vectorEntry.size())
   {
      m_vectorEntry[uIndex].set_deleted();
   }
}

/** ---------------------------------------------------------------------------
 * @brief Physically removes all deleted entries from the repository file.
 *
 * This method rewrites the repository file, excluding all entries marked as deleted.
 * It involves creating a temporary file, copying all valid entries, and then replacing
 * the original file. The in-memory index (m_vectorEntry) is also updated.
 *
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 */
std::pair<bool, std::string> repository::remove_entry_from_file()
{                                                                                                  assert( m_pFile != nullptr );
    // Collect indexes of entries marked as deleted
    std::vector<uint64_t> vectorIndexes;
    for( size_t u = 0; u < m_vectorEntry.size(); ++u ) 
    {
        if(m_vectorEntry[u].is_deleted()) 
        {
            vectorIndexes.push_back(u);
        }
    }

    return remove_entry_from_file(vectorIndexes);                               // call remove method to remove selected entries
}

/** ---------------------------------------------------------------------------
 * @brief Physically removes an entry from the repository file by name.
 *
 * Removes the specified entry from the file by rewriting the repository file, excluding the
 * entry with the given name. This involves creating a temporary file, copying all other valid
 * entries, and then replacing the original file. The in-memory index (m_vectorEntry) is also updated.
 *
 * @param vectorIndexes The vector of indexes of entries to remove
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 * @retval {false, "File not open"} If no file is currently open.
 * @retval {false, "File not found: <name>"} If no matching valid entry is found.
 * @retval {false, "Failed to create temporary file"} If the temporary file could not be created.
 * @retval {false, "Failed to write to temporary file"} If writing to the temporary file failed.
 * @retval {false, "Failed to replace original file"} If renaming the temporary file failed.
 */
// std::pair<bool, std::string> repository::remove_entry_from_file(const std::string_view& stringName)
std::pair<bool, std::string> repository::remove_entry_from_file(const std::vector<uint64_t>& vectorIndexes)
{                                                                                                  assert( m_pFile != nullptr );
   if(m_pFile == nullptr) { return {false, "File not open"}; }

   for (const auto& index_ : vectorIndexes) {
      const entry& entry_ = m_vectorEntry[index_];
      if( index_ >= m_header.size() ) 
      {
         return {false, "Invalid index found"};
      }
   }

   header headerCopy(get_header());                                           // Create a copy of the header
   headerCopy.clear();
   repository repositoryCopy( headerCopy );                                   // Create a copy of the repository

   // ## Create a temporary file
   std::filesystem::path pathRepository(m_stringRepositoryPath);
   pathRepository.replace_extension("tmp");
   auto result_ = repositoryCopy.create(pathRepository.string());
   if( result_.first == false ) { return result_; }

   std::vector<uint8_t> vectorBuffer(1024 * 1024);                            // 1MB buffer for efficiency

   // ## Copy all valid entries except the one to be removed

   uint64_t uNewOffset = 0;
   std::vector<uint64_t> vectorRemove(vectorIndexes.begin(), vectorIndexes.end());
   std::sort(vectorRemove.begin(), vectorRemove.end(), std::greater<>());     // Sort in descending order for safe erasure

   auto uFirstContentPosition = calculate_first_content_position_s(*this);

   for (size_t u = 0; u < m_vectorEntry.size(); ++u)
   {
      bool bRemove = std::find(vectorIndexes.begin(), vectorIndexes.end(), u) != vectorIndexes.end();

      if( bRemove == false )
      {
         entry& entry_ = m_vectorEntry[u];

         // ### Read the entry's data from the original file
         auto uOffset = uFirstContentPosition + entry_.offset();
         auto iResult = fseek_64_(m_pFile, uOffset, SEEK_SET);                                     assert(iResult == 0);
         uint64_t uBytesRemaining = entry_.size();
         while( uBytesRemaining > 0 ) 
         {
            size_t uBytesToRead = std::min(static_cast<size_t>(uBytesRemaining), vectorBuffer.size());
            size_t uBytesRead = fread(vectorBuffer.data(), 1, uBytesToRead, m_pFile);              assert(uBytesRead != 0);

            if( uBytesRead != uBytesToRead ) 
            {
               repositoryCopy.close();
               std::remove(repositoryCopy.get_path().c_str());
               return {false, "Failed to read from original file"};
            }

            size_t uBytesWritten = write_s(repositoryCopy, vectorBuffer.data(), uBytesRead );
            if( uBytesWritten != uBytesRead ) 
            {
               repositoryCopy.close();
               std::remove(repositoryCopy.get_path().c_str());
               return {false, "Failed to write to temporary file"};
            }
            uBytesRemaining -= uBytesRead;
         }
         // Update the entry's offset
         entry_.set_offset( uNewOffset );
         uNewOffset += entry_.size();
         repositoryCopy.add_entry( entry_ );
      }
   }

   repositoryCopy.flush();
   repositoryCopy.close();
   close();

   std::remove(m_stringRepositoryPath.c_str());

   // Replace the original file with the temporary file
   if( std::rename(repositoryCopy.get_path().c_str(), m_stringRepositoryPath.c_str()) != 0 )
   {
      std::remove(repositoryCopy.get_path().c_str());
      return {false, "Failed to replace original file"};
   }
                                                                                                   assert( m_pFile == nullptr );
   result_ = open();
   if( result_.first == false ) { return result_; }

   flush();

   return {true, ""};
}

/// @brief Closes the repository file.
void repository::close()
{
   if(m_pFile)
   {
      fflush(m_pFile);
      fclose(m_pFile);
      m_pFile = nullptr;
   }
}

/// @brief Dumps the repository information to a string for debugging purposes.
std::string repository::dump() const 
{
   std::ostringstream oss;

   // Dump header information
   oss << "Repository Header:" << std::endl;
   oss << "Magic Number: " << std::hex << m_header.m_uMagic << std::dec << std::endl;
   oss << "Version: " << m_header.m_uVersion << std::endl;
   oss << "Entry max count: " << m_header.m_uMaxEntryCount << std::endl;
   oss << "Entry Count: " << m_header.m_uEntryCount << std::endl;

   // Dump entries information
   oss << "Entries:" << std::endl;
   for (const auto& entry : m_vectorEntry) {
      oss << "Name: " << entry.get_name() << std::endl;
      oss << "Offset: " << entry.m_uOffset << ", ";
      oss << "Size: " << entry.m_uSize << ", ";
      oss << "Flags: " << entry.m_uFlags << std::endl;
      oss << "Valid: " << ( entry.is_valid() ? "true" : "false" ) << ", ";
      oss << "Deleted: " << (entry.is_deleted() ? "true" : "false") << ", ";
      oss << "Remove: " << (entry.is_remove() ? "true" : "false") << std::endl;
      oss << "------------------------" << std::endl;
   }

   return oss.str();
}

/// @brief Reads the repository header and entry block from the file and updates the in-memory state.
std::pair<bool, std::string> repository::read_s(repository& repository_) 
{                                                                                                  assert(repository_.m_pFile != nullptr);
   // Read the header
   auto result_ = read_header_s(repository_.m_pFile, repository_.m_header); if(result_.first == false) { return result_; }

   // Calculate the size of the entry block
   uint64_t uEntrySize = size_entry_reserved_buffer_s(repository_);
   uint64_t uEntryOffset = calculte_entry_offset_s();

   // Read the entry block
   result_ = read_entry_block_s(repository_.m_pFile, repository_.m_vectorEntry, uEntrySize, uEntryOffset); if(result_.first == false) { return result_; }

   return {true, ""};
}

/// @brief Reads the repository header from the file and updates the in-memory header.
std::pair<bool, std::string> repository::read_header_s(FILE* pfile, header& header_) 
{                                                                                                  assert(pfile != nullptr);
   // Seek to the beginning of the file
   if( fseek_64_(pfile, 0, SEEK_SET) != 0 ) { return {false, "Failed to seek to the beginning of the file"}; }

   // Read the header from the file
   if( std::fread( &header_, sizeof(header), 1, pfile ) != 1) { return {false, "Failed to read header from the file"};  }

   // Validate the magic number
   if (header_.m_uMagic != repository::get_magic_number_s()) { return {false, "Invalid magic number in header"}; }

   return {true, ""};
}

/// @brief Reads a block of entry data from the file at the specified offset.
std::pair<bool, std::string> repository::read_entry_block_s(FILE* pfile, std::vector<entry>& vectorEntry, uint64_t uSize, uint64_t uOffset) 
{                                                                                                  assert(pfile != nullptr); assert(uSize % sizeof(entry) == 0); // Ensure the size is a multiple of entry size
   // Seek to the specified offset in the file
   if( fseek_64_(pfile, static_cast<long>(uOffset), SEEK_SET) != 0) { return {false, "Failed to seek to offset " + std::to_string(uOffset)}; }

   // Calculate the number of entries to read
   size_t uEntryCount = uSize / sizeof(entry);
   vectorEntry.resize(uEntryCount);

   // Read the entries from the file
   if(std::fread(vectorEntry.data(), sizeof(entry), uEntryCount, pfile) != uEntryCount) { return {false, "Failed to read entry block from the file"}; }

   return {true, ""};
}


/// Writes header repository information to file, this writes the header to the beginning of the file.
std::pair<bool, std::string> repository::write_header_s(FILE* pfile, const header& header_)
{                                                                                                  assert( pfile != nullptr );
   // Seek to the beginning of the file
   if( fseek_64_(pfile, 0, SEEK_SET) != 0 ) { return {false, "Failed to seek to the beginning of the file"}; }

   // Write the header to the file
   if (std::fwrite(&header_, sizeof(header), 1, pfile) != 1) { return {false, "Failed to write header to the file"}; }

   // Flush the file to ensure the header is written
   if (std::fflush(pfile) != 0) { return {false, "Failed to flush the file"}; }

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Writes a block of entry data to a file at a specified offset.
 *
 * Writes a block of data, assumed to be composed of repository::entry objects (but can write any data pointer points to),
 * to the given file starting at the specified offset. The size must be a multiple of the entry size.
 *
 * @param pfile Pointer to the file to write to (must not be null).
 * @param pdata Pointer to the data to write (assumed to be repository::entry objects).
 * @param uSize Size of the data in bytes (must be a multiple of sizeof(repository::entry)).
 * @param uOffset Byte offset in the file where writing begins.
 * @return std::pair<bool, std::string> Pair containing success flag (true) and an empty message on success.
 * @pre uSize must be a multiple of sizeof(repository::entry).
 * @pre pfile must not be null.
 */
std::pair<bool, std::string> repository::write_entry_block_s(FILE* pfile, const void* pdata, uint64_t uSize, uint64_t uOffset)
{                                                                                                  assert( uSize % sizeof( repository::entry ) == 0 ); assert( pfile != nullptr );
   fseek(pfile, static_cast<long>( uOffset ), SEEK_SET);                        // seek (move) to offset
   fwrite(pdata, 1, uSize, pfile);                                              // write data to file

   return { true, "" };
}

/** ---------------------------------------------------------------------------
 * @brief Writes a block of repeated byte values to a file at a specified offset.
 *
 * This static function fills a block of the specified size with a single byte value and writes it
 * to the file starting at the given offset. It includes error checking for file operations.
 *
 * @param pfile Pointer to the file to write to (must not be null).
 * @param uFillValue Byte value to fill the block with (e.g., 0x00 for zeros).
 * @param uSize Number of bytes to write.
 * @param uOffset Byte offset in the file where writing begins.
 * @return std::pair<bool, std::string> Pair containing success flag and a message:
 *         - {true, ""} on success.
 *         - {false, error message} on failure (e.g., invalid pointer, seek/write/flush error).
 * @pre pfile must not be null.
 */
std::pair<bool, std::string> repository::write_block_s(FILE* pfile, uint8_t uFillValue, uint64_t uSize, uint64_t uOffset) 
{                                                                                                  assert( pfile != nullptr );
   // Seek to the specified offset in the file
   if( std::fseek(pfile, (unsigned)uOffset, SEEK_SET) != 0 ) { return {false, "Failed to seek to offset " + std::to_string(uOffset)}; }

   // ## Write the block of data filled with uFillValue
   for(uint64_t u = 0; u < uSize; ++u) 
   {
      if( fputc(uFillValue, pfile) == EOF ) { return {false, "Failed to write byte at position " + std::to_string(uOffset + u)}; }
   }

   // Ensure the data is flushed to the file
   if (fflush(pfile) != 0) {  return {false, "Failed to flush data to file"}; }

   return {true, ""};
}

void repository::copy_entries_s(repository& repositoryFrom, const repository& repositoryTo) 
{
   // Clear the destination repository's entries
   repositoryFrom.m_vectorEntry.clear();
   repositoryFrom.m_header.m_uEntryCount = 0;

   // Copy each entry from the source repository to the destination repository
   for( const auto& entry_ : repositoryTo.m_vectorEntry )  
   {
      repositoryFrom.m_vectorEntry.push_back(entry_);
      repositoryFrom.m_header.add_entry();
   }

   // Update the header in the destination repository
   repositoryFrom.m_header.m_uEntryCount = repositoryFrom.m_vectorEntry.size();
}


std::pair<bool, std::string> repository::expand( uint64_t uCount, uint64_t uBuffer ) 
{                                                                                                  assert( m_pFile != nullptr );

   // Calculate new max entry count (e.g., double the current size)
   uint64_t uNewMaxEntryCount = uCount;
   uint64_t uOldEntryBlockSize = m_header.size() * sizeof(entry);
   uint64_t uNewEntryBlockSize = uNewMaxEntryCount * sizeof(entry);
   uint64_t uShiftSize = uNewEntryBlockSize - uOldEntryBlockSize;

   // Calculate the current content start position
   uint64_t uOldContentOffset = calculate_first_content_position_s(*this);

   // Step 1: Read existing content into memory (or use a temp file for large data)
   std::vector<uint8_t> contentBuffer;
   fseek_64_(m_pFile, uOldContentOffset, SEEK_SET);
   uint64_t uContentSize = calculate_first_free_content_position_s(*this) - uOldContentOffset;
   contentBuffer.resize(uContentSize);
   size_t uRead = fread(contentBuffer.data(), 1, uContentSize, m_pFile);
   if (uRead != uContentSize) 
   {
      return {false, "Failed to read content for shifting"};
   }

   // Step 2: Update header with new max entry count
   m_header.m_uMaxEntryCount = uNewMaxEntryCount;

   // Step 3: Write updated header
   auto [headerSuccess, headerError] = write_header_s(m_pFile, m_header);
   if (!headerSuccess) {
      return {false, "Failed to write updated header: " + headerError};
   }

   // Step 4: Shift content by rewriting it at the new offset
   uint64_t uNewContentOffset = uOldContentOffset + uShiftSize;
   fseek_64_(m_pFile, uNewContentOffset, SEEK_SET);
   size_t uWritten = fwrite(contentBuffer.data(), 1, uContentSize, m_pFile);
   if (uWritten != uContentSize) {
      return {false, "Failed to shift content"};
   }

   // Step 5: Update entry offsets
   for (auto& entry : m_vectorEntry) {
      if (entry.m_uOffset >= uOldContentOffset) {
         entry.m_uOffset += uShiftSize;
      }
   }

   // Step 6: Write updated entry block (fill new space with zeros)
   auto [entrySuccess, entryError] = write_entry_block_s(m_pFile, m_vectorEntry.data(), uOldEntryBlockSize, calculte_entry_offset_s());
   if (!entrySuccess) {
      return {false, "Failed to write updated entry block: " + entryError};
   }
   auto [fillSuccess, fillError] = write_block_s(m_pFile, 0, uShiftSize, calculte_entry_offset_s() + uOldEntryBlockSize);
   if (!fillSuccess) {
      return {false, "Failed to fill new entry space: " + fillError};
   }

   fflush(m_pFile);
   return {true, ""};
}

_GD_IO_STREAM_END