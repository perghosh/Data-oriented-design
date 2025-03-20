#include <format>
#include <fstream>

#include "Document.h"

std::pair<bool, std::string> CDocument::Load(const std::string_view& stringPath)
{
   std::ifstream ifstreamFile(stringPath.data(), std::ios::binary | std::ios::ate);
   if( ifstreamFile.is_open() == false ) { return {false, std::format("Failed to open file: {}", stringPath); }

   // ## Get file size from position at end
   std::streamsize uSize = ifstreamFile.tellg();
   ifstreamFile.seekg(0, std::ios::beg);

   m_vectorData.reserve( static_cast<size_t>(uSize) );

   // Read entire file in one go
   m_vectorData.assign( std::istreambuf_iterator<char>(ifstreamFile), std::istreambuf_iterator<char>() );

   Set("path", stringPath );

   return {true, ""};
}