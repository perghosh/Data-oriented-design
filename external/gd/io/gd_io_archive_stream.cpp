
#include "gd_io_archive_stream.h"

_GD_IO_STREAM_BEGIN


archive& archive::read( std::string& string_ )
{
   uint64_t uSize;
   read_size(uSize);
   string_.resize(uSize);
   read(string_.data(), uSize);
   return *this;
}

archive& archive::write( const std::string_view& stringData )
{
   write_size((uint64_t)stringData.size());
   write(stringData.data(), stringData.size());
   return *this;
}

archive& archive::write( const std::string& stringData )
{
   write_size((uint64_t)stringData.size());
   write(stringData.data(), stringData.size());
   return *this;
}


/** ---------------------------------------------------------------------------
 * @brief open file stream
 * @param fstreamFile file stream object to open
 * @param stringPath path to file
 * @param mode_ in which mode to open file
 * @return true if file is opened, false if failed to open file
 */
std::pair<bool, std::string> archive::open_s(std::fstream& fstreamFile, const std::string_view& stringPath, std::ios_base::openmode mode_) 
{
   fstreamFile.open( stringPath.data(), mode_ );
   if( fstreamFile.is_open() == false ) 
   {
      return {false, std::string( "Failed to open file: " ) + stringPath.data() };
   }

   return { true, "" };
}

_GD_IO_STREAM_END