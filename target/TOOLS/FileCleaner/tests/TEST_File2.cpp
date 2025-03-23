#include <fstream>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"

#include "main.h"

#include "../Document.h"
#include "../Application.h"

#include "catch2/catch_amalgamated.hpp"

std::string GetDataFolder()
{
   return FOLDER_GetRoot_g("target/TOOLS/FileCleaner/tests/data");
}

TEST_CASE( "[file] serialize", "[file]" )
{
   using namespace gd::io::stream;

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive2.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   std::string stringValue = "Hello";
   archiveStream.write(stringValue);
   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});
   std::string stringValueRead = " ";
   archiveStream.read(stringValueRead);

   std::cout << "Values: " << stringValueRead << std::endl;
   archiveStream.close();

}