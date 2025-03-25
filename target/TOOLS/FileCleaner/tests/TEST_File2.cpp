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

   const int iSize = 5;
   int piNumbers[iSize];
   int iCount = 0;

   for( int i = 0; i < iSize; i++ )
   {
      piNumbers[i] = i;
      iCount++;
   }

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive2.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   std::string stringValue = "Hello";
   archiveStream.write(stringValue);
   archiveStream.write(iCount);

   for( int i = 0; i < iSize; i++ )
   {
      archiveStream.write(piNumbers[i]);
   }

   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});
   std::string stringValueRead = " ";
   archiveStream.read(stringValueRead);

   std::vector<int> vectorNumbersRead;

   int iCountRead;
   archiveStream.read(iCountRead);

   for( int i = 0; i < iCountRead; i++ )
   {
      int iTemp;

      archiveStream.read(iTemp);
      vectorNumbersRead.push_back(iTemp);
   }

   std::cout << "Values: " << stringValueRead << std::endl;

   archiveStream.close();

}

// int, double, int64

TEST_CASE( "[file] serialize2", "[file]" )
{
   using namespace gd::io::stream;
   const int iSize = 3;
   int piNumbers[iSize];
   double pdNumbers[iSize];
   int64_t piNumbers2[iSize];
   int iCount = 0;
   int iCount2 = 0;
   int iCount3 = 0;
   std::string stringValue = "DDD";

   for( int i = 0; i < iSize; i++ )
   {
      piNumbers[i] = i;
      pdNumbers[i] = i;
      piNumbers2[i] = i;
      iCount++;
      iCount2++;
      iCount3++;
   }

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive3.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   archiveStream.write(stringValue);
   archiveStream.write(iCount);
   archiveStream.write(iCount2);
   archiveStream.write(iCount3);

   for( int i = 0; i < iSize; i++ )
   {
      archiveStream.write(piNumbers[i]);
   }

   for( int i = 0; i < iSize; i++ )
   {
      archiveStream.write(pdNumbers[i]);
   }

   for( int i = 0; i < iSize; i++ )
   {
      archiveStream.write(piNumbers2[i]);
   }

   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});

   std::vector<int> vectorNumbersRead;
   std::vector<double> vectorNumbersRead2;
   std::vector<int64_t> vectorNumbersRead3;

   int iCountRead;
   int iCountRead2;
   int iCountRead3;
   std::string stringValueRead = " ";
   /*archiveStream.read(iCountRead);
   archiveStream.read(iCountRead2);
   archiveStream.read(iCountRead3);*/

   std::cout << "Values: ";

   archiveStream.read(stringValueRead);
   std::cout << stringValueRead;

   archiveStream.read_all(iCountRead, iCountRead2, iCountRead3);


   for( int i = 0; i < iCountRead; i++ )
   {
      int iTemp;

      archiveStream.read(iTemp);
      vectorNumbersRead.push_back(iTemp);
      std::cout << vectorNumbersRead[i];
   }

   for( int i = 0; i < iCountRead2; i++ )
   {
      int iTemp;

      archiveStream.read(iTemp);
      vectorNumbersRead2.push_back(iTemp);
      std::cout << vectorNumbersRead2[i];
   }

   for( int i = 0; i < iCountRead3; i++ )
   {
      int iTemp;

      archiveStream.read(iTemp);
      vectorNumbersRead3.push_back(iTemp);
      std::cout << vectorNumbersRead3[i];
   }

   std::cout << std::endl;

   archiveStream.close();

}

TEST_CASE("[file] serialize3", "[file]")
{
   using namespace gd::io::stream;
   const int iSize = 5;
   char iCharacters[iSize] = { 'a', 'b', 'a', 'd', 'l' };
   int iCount = iSize;

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive4.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   archiveStream.write(iCount);

   for( int i = 0; i < iSize; i++ )
   {
      archiveStream.write(iCharacters[i]);
   }

   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});

   std::vector<char> vectorCharactersRead;
   int iCountRead;

   archiveStream.read(iCountRead);

   std::cout << "Values: ";

   for( int i = 0; i < iCount; i++ )
   {
      char iTemp;
      archiveStream.read(iTemp);
      vectorCharactersRead.push_back(iTemp);
      std::cout << vectorCharactersRead[i];
   }

   std::cout << std::endl;

   archiveStream.close();


}