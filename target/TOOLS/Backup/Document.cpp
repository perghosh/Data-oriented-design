#include <format>
#include <fstream>

#include "Document.h"

void CDocument::common_construct(const CDocument& o)
{
   m_arguments = o.m_arguments;
   m_vectorData = o.m_vectorData;
}

void CDocument::common_construct(CDocument&& o) noexcept
{
   m_arguments = std::move(o.m_arguments);
   m_vectorData = std::move(o.m_vectorData);
}

/** ---------------------------------------------------------------------------
 * @brief Loads a file into the document from the specified path.
 *
 * This method attempts to open and read the contents of a file specified by the given path into the document's internal data structure (`m_vectorData`).
 * The file is read in binary mode, and its size is determined to pre-allocate memory for efficiency. If successful, the path is stored in the document.
 *
 * @param stringPath A string view representing the path to the file to be loaded.
 * @return A pair containing:
 *         - `bool`: `true` if the file was successfully loaded, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure (e.g., "Failed to open file: <path>").
 */
std::pair<bool, std::string> CDocument::Load(const std::string_view& stringPath)
{
   std::ifstream ifstreamFile(stringPath.data(), std::ios::binary | std::ios::ate);
   if( ifstreamFile.is_open() == false ) { return { false, std::format("Failed to open file: {}", stringPath) }; }

   // ## Get file size from position at end
   std::streamsize uSize = ifstreamFile.tellg();
   ifstreamFile.seekg(0, std::ios::beg);

   m_vectorData.reserve( static_cast<size_t>(uSize) );

   // Read entire file in one go
   m_vectorData.assign( std::istreambuf_iterator<char>(ifstreamFile), std::istreambuf_iterator<char>() );

   Set("path", stringPath );

   return {true, ""};
}


/** ---------------------------------------------------------------------------
 * @brief Saves the document's data to a file at the specified path.
 *
 * This method writes the contents of the document's internal data structure (`m_vectorData`) to a file in binary mode.
 * If the file cannot be opened or written to, an error message is returned.
 *
 * @param stringPath A string view representing the path where the file should be saved.
 * @return A pair containing:
 *         - `bool`: `true` if the file was successfully saved, `false` otherwise.
 *         - `std::string`: An empty string on success, or an error message on failure (e.g., "Failed to write to file: <path>").
 */
std::pair<bool, std::string> CDocument::Save(const std::string_view& stringPath)
{
   std::ofstream ofstreamFile(stringPath.data(), std::ios::binary);
   if(!ofstreamFile) { return {false, std::format("Failed to open file: {}", stringPath )}; }

   ofstreamFile.write(reinterpret_cast<const char*>(m_vectorData.data()), m_vectorData.size());
   if( !ofstreamFile ) { return {false, std::format("Failed to write to file: {}", stringPath ) }; }

   return {true, ""};
}

/** ---------------------------------------------------------------------------
 * @brief Counts the occurrences of a specific character in the document's data.
 *
 * This method scans the document's internal data (`m_vectorData`) and returns the number of times a given character appears.
 *
 * @param uCharacter The 8-bit unsigned integer representing the character to count.
 * @return The number of occurrences of the specified character in the document's data.
 */
size_t CDocument::Count(uint8_t uCharacter) const
{
   auto uCount = std::count(m_vectorData.begin(), m_vectorData.end(), uCharacter);
   return uCount;
}