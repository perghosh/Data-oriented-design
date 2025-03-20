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


std::pair<bool, std::string> CDocument::Save(const std::string_view& stringPath)
{
   std::ofstream ofstreamFile(stringPath.data(), std::ios::binary);
   if(!ofstreamFile) { return {false, std::format("Failed to open file: {}", stringPath )}; }

   ofstreamFile.write(reinterpret_cast<const char*>(m_vectorData.data()), m_vectorData.size());
   if( !ofstreamFile ) { return {false, std::format("Failed to write to file: {}", stringPath ) }; }

   return {true, ""};
}

size_t CDocument::Count(uint8_t uCharacter) const
{
   auto uCount = std::count(m_vectorData.begin(), m_vectorData.end(), uCharacter);
   return uCount;
}