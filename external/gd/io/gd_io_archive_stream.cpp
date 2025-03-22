
#include "gd_io_archive_stream.h"

_GD_IO_STREAM_BEGIN

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