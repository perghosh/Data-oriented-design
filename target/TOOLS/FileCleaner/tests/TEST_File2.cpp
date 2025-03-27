#include <fstream>


#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"
#include "History.h"

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

// struct variabler skriva in och sedan läsas ut

struct Data
{
   int m_iNumber = 5;
   int m_iNumber2 = 3;

   void write(gd::io::stream::archive& archive_)
   {
      archive_ << m_iNumber;
      archive_ << m_iNumber2;
   }

   void read(gd::io::stream::archive& archive_)
   {
      archive_ >> m_iNumber;
      archive_ >> m_iNumber2;
   }

};

TEST_CASE("[file] serialize4", "[file]")
{
   using namespace gd::io::stream;

   Data dataWrite;

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive5.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});
   dataWrite.write( archiveStream );
   //archiveStream.write_all(dataWrite.m_iNumber, dataWrite.m_iNumber2);

   archiveStream.close();

   Data dataRead;

   archiveStream.open(pathFile, gd::io::tag_io_read{});
   archiveStream.read_all(dataRead.m_iNumber, dataRead.m_iNumber2);

   archiveStream.close();

}

// struct sekvenser vector

TEST_CASE("[file] serialize5", "[file]")
{
   using namespace gd::io::stream;

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive6.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   std::vector<Data> vectorDataWrite[3];
   int iCount = 0;

   for( int i = 0; i < 3; i++ )
   {
      Data dataTemp;
      vectorDataWrite->push_back(dataTemp);
      iCount++;
   }

   archiveStream.write(iCount);

   for( int i = 0; i < vectorDataWrite->size(); i++ )
   {
      archiveStream.write(vectorDataWrite[i]);
   }

   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});
   std::vector<Data> vectorDataRead;
   int iCountRead;

   archiveStream.read(iCountRead);

   for( int i = 0; i < iCountRead; i++ )
   {
      Data dataTemp;
      archiveStream.read(dataTemp);
      vectorDataRead.push_back(dataTemp);
   }

   archiveStream.close();

}

TEST_CASE("[file] serialize6", "[file]")
{
   using namespace gd::io::stream;

   std::string stringDataFolder = GetDataFolder();
   gd::file::path pathFile(stringDataFolder + "/archive7.bin");
   if( std::filesystem::exists(pathFile) == true ) std::filesystem::remove(pathFile);

   archive archiveStream(pathFile, gd::io::tag_io_write{});

   CHistory history;
   history.Add("HHH");
   history.Add("BBB");
   history.Add("AAA");

   history.Write(archiveStream);

   archiveStream.close();

   archiveStream.open(pathFile, gd::io::tag_io_read{});

   history.Read(archiveStream);

   archiveStream.close();
 
}