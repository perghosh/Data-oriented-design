

#include "gd_io_repository_stream.h"

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
   return open(stringPath, "w+b");
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
   if(!m_pFile) { return {false, std::string( "Failed to open file: " ) + stringPath.data()}; }

   m_stringRepositoryPath = stringPath;

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

   fseek(m_pFile, 0, SEEK_END);
   uint64_t uOffset = ftell(m_pFile);

   fwrite(pdata_, 1, uSize, m_pFile);

   entry entry_( stringName.data(), uOffset, uSize, eEntryFlagValid );
   m_vectorEntry.push_back(entry_);

   return {true, ""};
}


/**
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

   fseek(m_pFile, static_cast<long>(it->m_uOffset), SEEK_SET);
   fread(pdata, 1, it->m_uSize, m_pFile);

   return { true, "" };
}

/** -
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

   std::vector<uint8_t> vectorBuffer( it->size() );
   fseek(m_pFile, static_cast<long>(it->m_uOffset), SEEK_SET);
   fread(vectorBuffer.data(), 1, it->m_uSize, m_pFile);

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
 * @brief Physically removes an entry from the repository file by name.
 *
 * Removes the specified entry from the file by rewriting the repository file, excluding the
 * entry with the given name. This involves creating a temporary file, copying all other valid
 * entries, and then replacing the original file. The in-memory index (m_vectorEntry) is also updated.
 *
 * @param stringName The name of the entry to remove, provided as a string view.
 * @return A pair containing a boolean indicating success (true) or failure (false),
 *         and a string with an error message if the operation failed, or empty if successful.
 * @retval {false, "File not open"} If no file is currently open.
 * @retval {false, "File not found: <name>"} If no matching valid entry is found.
 * @retval {false, "Failed to create temporary file"} If the temporary file could not be created.
 * @retval {false, "Failed to write to temporary file"} If writing to the temporary file failed.
 * @retval {false, "Failed to replace original file"} If renaming the temporary file failed.
 */
std::pair<bool, std::string> repository::remove_entry_from_file(const std::string_view& stringName)
{                                                                                                  assert( m_pFile != nullptr );
   if (!m_pFile) { return {false, "File not open"}; }

   // Find the entry to remove
   auto it = std::find_if(m_vectorEntry.begin(), m_vectorEntry.end(),
      [&stringName](const entry& e) { return e.is_valid() && !e.is_deleted() && e.get_name() == stringName; });

   if (it == m_vectorEntry.end()) { return {false, std::string("File not found: ") + stringName.data()}; }

   // Create a temporary file
   std::string stringPathTemporary = m_stringRepositoryPath + ".tmp";
   FILE* pfileTemporary = fopen(stringPathTemporary.c_str(), "w+b");
   if(pfileTemporary == nullptr) { return {false, "Failed to create temporary file"}; }

   // Buffer for copying data
   std::vector<uint8_t> vectorBuffer(1024 * 1024); // 1MB buffer for efficiency

   // Copy all valid entries except the one to be removed
   uint64_t uNewOffset = 0;
   for( auto& entry_ : m_vectorEntry) {
      if( entry_.is_valid() && !entry_.is_deleted() && entry_.get_name() != stringName) {
         // Read the entry's data from the original file
         fseek(m_pFile, static_cast<long>(entry_.offset()), SEEK_SET);
         uint64_t uBytesRemaining = entry_.size();
         while( uBytesRemaining > 0 ) 
         {
            size_t uBytesToRead = std::min(static_cast<size_t>(uBytesRemaining), vectorBuffer.size());
            size_t uBytesRead = fread(vectorBuffer.data(), 1, uBytesToRead, m_pFile);

            if (uBytesRead != uBytesToRead) 
            {
               fclose(pfileTemporary);
               std::remove(stringPathTemporary.c_str());
               return {false, "Failed to read from original file"};
            }

            size_t uBytesWritten = fwrite(vectorBuffer.data(), 1, uBytesRead, pfileTemporary);
            if (uBytesWritten != uBytesRead) 
            {
               fclose(pfileTemporary);
               std::remove(stringPathTemporary.c_str());
               return {false, "Failed to write to temporary file"};
            }
            uBytesRemaining -= uBytesRead;
         }
         // Update the entry's offset
         entry_.set_offset( uNewOffset );
         uNewOffset += entry_.size();
      }
   }

   // Close both files
   fclose(pfileTemporary);
   close();

   // Replace the original file with the temporary file
   if (std::rename(stringPathTemporary.c_str(), m_stringRepositoryPath.c_str()) != 0) 
   {
      std::remove(stringPathTemporary.c_str());
      return {false, "Failed to replace original file"};
   }

   // Reopen the updated file
   m_pFile = fopen(m_stringRepositoryPath.c_str(), "r+b");
   if( !m_pFile ) { return {false, "Failed to reopen updated file"};  }

   // Remove the entry from the in-memory index
   m_vectorEntry.erase(it);

   return {true, ""};
}

/// @brief Closes the repository file.
void repository::close()
{
   if(m_pFile)
   {
      fclose(m_pFile);
      m_pFile = nullptr;
   }
}



_GD_IO_STREAM_END