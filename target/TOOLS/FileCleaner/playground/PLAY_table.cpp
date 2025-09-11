#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdio>


#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif


#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "gd/expression/gd_expression_value.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_runtime.h"


#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"

static std::string CreateTemporaryFile_s();

/*
@TASK [project: serialize-table][status: open][created: 250905] [assigned: per]
[title: select method names for read and write] [description: names needed to read and write data in table class]
[sample: "suggestions
- serialize( void* pBuffer, bool bSave, tag_columns )
- write( void* pBuffer, tag_columns ); read( void* pBuffer, tag_columns );
- write( void* pBuffer, tag_full ); read( void* pBuffer, tag_full );
"]

@TASK [project: serialize-table][status: ongoing][created: 250905] [assigned: per]
[title: size methods] [description: methods needed to calculate needed memory size for parts in table]
[sample: "suggestions
- serialize_size( tag_columns ); serialize_size( tag_full ); serialize_size( tag_body );
- write_size( void* pBuffer, tag_columns ); read_size( void* pBuffer, tag_columns );
"]

@TASK [project: serialize-table][status: ongoing][created: 250905] [assigned: per]
[title: method to calculate needed size for columns]
[sample: "suggestions
- serialize_size( tag_columns ); serialize_size( tag_full ); serialize_size( tag_body );
- write_size( void* pBuffer, tag_columns ); read_size( void* pBuffer, tag_columns );
"]

@TASK [project: serialize-table][status: ongoing][created: 250905] [assigned: per]
[title: method to calculate needed size for body]
[sample: "suggestions
- serialize_size( tag_body ); 
"]

@TASK [project: serialize-table][status: ongoing][created: 250905] [assigned: per]
[title: read and write table body data]



*/

TEST_CASE("[table] expression", "[table]") {
   using namespace gd::expression;

   { auto value_ = token::calculate_s("1 + 1 * 2"); std::cout << value_.as_string() << std::endl; }
   { auto value_ = token::calculate_s("1 + 1 == 2 + 2"); std::cout << value_.as_string() << std::endl; }
   { auto value_ = token::calculate_s("2 * 3 + 3 * 2 - 4 * 2 + 20"); std::cout << value_.as_string() << std::endl; }
   { auto value_ = token::calculate_s("2 == 1 || 3 == 2"); std::cout << value_.as_string() << std::endl; }

   // [1, 1, +, 2, 2, +, ==]


   //auto b = value1.as_bool();
}

TEST_CASE("[table] tests to serialize parts of table", "[table]") {
   using namespace gd::table::dto;

   const std::string stringCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   constexpr unsigned uTableDuplicate = ( table::eTableFlagNull32 | table::eTableFlagRowStatus | table::eTableFlagDuplicateStrings );
   gd::table::dto::table tableTest01(uTableDuplicate, { { "int64", 0, "KeyK"}, { "string", 100, "name"}, { "string", 100, "text"} }, gd::table::tag_prepare{});
   gd::table::dto::table tableTestR01(uTableDuplicate, { { "int64", 0, "KeyK"}, { "rstring", 0, "name"}, { "rstring", 0, "text"} }, gd::table::tag_prepare{});

   std::mt19937 mt19937;

   // Generate 100 random strings using stl
   std::vector<std::string> vectorRandomStrings;
   for( int i = 0; i < 100; ++i )
   {
      std::string string_;
      // Generate random number for length of string
      int iLength = mt19937() % 30 + 1; // Random length between 1 and 30
      for( int j = 0; j < iLength; ++j )
      {
         string_ += stringCharset[mt19937() % stringCharset.size()];
      }
      vectorRandomStrings.push_back(string_);
   }

   unsigned uCount = 10;
   for( const auto& string_ : vectorRandomStrings )
   {
      auto uRow = tableTest01.row_add_one();
      tableTest01.row_set(uRow, { {"KeyK", (int64_t)uRow}, {"name", string_}, {"text", string_} });
      if( uCount > 0 )
      {
         uCount--;
         tableTestR01.row_add_one();
         tableTestR01.row_set(uRow, { {"KeyK", (int64_t)uRow}, {"name", string_}, {"text", string_} });
      }
   }

   {  // --- test serialize references
      auto uReferencesSize = tableTestR01.serialize_size(gd::table::tag_reference{});

      std::vector<uint8_t> vectorBuffer;
      vectorBuffer.resize(uReferencesSize);
      auto pPosition = tableTestR01.serialize(reinterpret_cast<std::byte*>( vectorBuffer.data() ), true, gd::table::tag_reference{}); // write

      // read back
      pPosition = tableTestR01.serialize(reinterpret_cast<std::byte*>( vectorBuffer.data() ), false, gd::table::tag_reference{}); // read

      std::string s_ = gd::table::to_string(tableTestR01, gd::table::tag_io_cli{});
      //std::cout << s_ << std::endl;
   }


   std::string stringTable_ = gd::table::to_string(tableTest01, gd::table::tag_io_cli{});

   // read all names and texts
   for( uint64_t uRow = 0; uRow < tableTest01.size(); ++uRow )
   {
      auto stringName = tableTest01.cell_get_variant_view(uRow, "name").as_string_view();
      auto stringText = tableTest01.cell_get_variant_view(uRow, "text").as_string_view();
   }

   uint64_t uTableSize = tableTest01.serialize_size(gd::table::tag_columns{});
   uTableSize += tableTest01.serialize_size(gd::table::tag_body{});

   std::vector<uint8_t> vectorBuffer;
   vectorBuffer.resize(uTableSize);

   auto pPosition = tableTest01.serialize(reinterpret_cast<std::byte*>( vectorBuffer.data() ), true, gd::table::tag_columns{});
   tableTest01.serialize(pPosition, true, gd::table::tag_body{});

   gd::table::dto::table tableTest02;
   pPosition = tableTest02.serialize(reinterpret_cast<std::byte*>( vectorBuffer.data() ), false, gd::table::tag_columns{});
   pPosition = tableTest02.serialize(pPosition, false, gd::table::tag_body{});

   bool bSame = tableTest02.m_uRowGrowBy == tableTest01.m_uRowGrowBy;
   bSame = tableTest02.m_uRowCount == tableTest01.m_uRowCount;

   std::string stringTable = gd::table::to_string(tableTest02, gd::table::tag_io_cli{});
   std::cout << stringTable << std::endl;
   stringTable = gd::table::to_string(tableTest01, gd::table::tag_io_cli{});
   //std::cout << stringTable << std::endl;
}

TEST_CASE("[table] save table to disk", "[table]") {
   using namespace gd::table::dto;
   const std::string stringCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   constexpr unsigned uTableDuplicate = (table::eTableFlagNull32|table::eTableFlagRowStatus|table::eTableFlagDuplicateStrings);

   gd::table::dto::table tableSerialize( uTableDuplicate, { { "int64", 0, "KeyK"}, { "rstring", 0, "name"}, { "rstring", 0, "text"} }, gd::table::tag_prepare{} );

   std::mt19937 mt19937;

   // Generate 100 random strings using stl
   std::vector<std::string> vectorRandomStrings;
   for( int i = 0; i < 100; ++i )
   {
      std::string string_;
      // Generate random number for length of string
      int iLength = mt19937() % 30 + 1; // Random length between 1 and 30
      for( int j = 0; j < iLength; ++j )
      {
         string_ += stringCharset[mt19937() % stringCharset.size()];
      }
      vectorRandomStrings.push_back(string_);
   }
   
   for( const auto& string_ : vectorRandomStrings )
   {
      auto uRow = tableSerialize.row_add_one();
      tableSerialize.row_set(uRow, { {"KeyK", (int64_t)uRow}, {"name", string_}, {"text", string_} });
   }

   std::string stringTemporaryFile = CreateTemporaryFile_s();

   // delete temporary file if exists
   if( std::filesystem::exists( stringTemporaryFile ) == true )
   {
      std::filesystem::remove( stringTemporaryFile );
   }

   uint64_t uTableSize = tableSerialize.serialize_size(gd::table::tag_columns{});


   /*
   // ## Serialize to disk
   {
      // Generate

      std::string stringFile = Application::get_temp_path() + "table_test.gdt";
      gd::file::file file( stringFile, gd::file::eOpenWriteCreate );
      if( file.is_open() == true )
      {
         auto uSize = tableSerialize.serialize_size( gd::table::tag_full{} );
         std::vector<uint8_t> vectorBuffer;
         vectorBuffer.resize( uSize );
         tableSerialize.serialize( reinterpret_cast<std::byte*>( vectorBuffer.data() ), true, gd::table::tag_full{} );
         file.write( vectorBuffer.data(), (unsigned)vectorBuffer.size() );
         file.close();
      }
   }
   */
}


TEST_CASE("[table] custom columns", "[table]") 
{
#ifdef _MSC_VER
   // Enable memory leak check at program exit
   int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
   tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
   _CrtSetDbgFlag(tmpFlag);
#endif

   gd::table::arguments::table table_( gd::table::tag_full_meta{} );
   table_.column_prepare();
   table_.column_add("rstring", 0, "path");
   table_.column_add("rstring", 0, "name");
   table_.column_add("uint64", 0, "size");
   table_.prepare();

   auto uRow = table_.row_add_one();
   table_.row_set(uRow, { {"path", "C:\\test\\file.txt"}, {"name", "file.txt"}, {"size", 12345} }, gd::table::tag_convert{});

   { auto b_ = table_.cell_get_variant_view(uRow, "path"). as_string_view() == "C:\\test\\file.txt"; REQUIRE(b_); }

   table_.cell_set(uRow, "path2", gd::variant_view("C:\\test\\file2.txt"));
   { auto b_ = table_.cell_get_variant_view(uRow, "path2"). as_string_view() == "C:\\test\\file2.txt"; REQUIRE(b_); }

   { auto b_ = table_.cell_get_variant_view(uRow, 3).as_string_view() == "C:\\test\\file2.txt"; REQUIRE(b_); }

   { auto arguments_ = table_.row_get_arguments(0); std::cout << gd::argument::debug::print(arguments_); }

   {
      auto vector_ = table_.row_get_variant_view(uRow);
      std::cout << "\n\nRow Variant View: ";
      for (const auto& variant : vector_) {
          std::cout << variant.as_string() << " ";
      }
      std::cout << std::endl;
   }

   {
      std::vector<gd::variant_view> vectorGet;
      vectorGet = table_.row_get_variant_view( uRow, {0, 1, 2, 3 });
      std::cout << "\n\nRow Variant View (new): ";
      for (const auto& variant : vectorGet) {
          std::cout << variant.as_string() << " ";
      }
      std::cout << std::endl;
   }

   table_.cell_set(uRow, "test", gd::variant_view("test"));
   table_.cell_set(uRow, 3, gd::variant_view("1234567890"));
   {
      std::vector<gd::variant_view> vectorGet;
      vectorGet = table_.row_get_variant_view( uRow, {0, 1, 2, 3 });
      std::cout << "\n\nRow Variant View (new): ";
      for (const auto& variant : vectorGet) {
         std::cout << variant.as_string() << " ";
      }
      std::cout << std::endl;
   }

}

TEST_CASE("[table] custom columns 1", "[table]") 
{
   gd::table::arguments::table table_( gd::table::tag_full_meta{} );

   gd::argument::shared::arguments argumentsTest({{ "one", 1 },{ "one", 1 },{ "one", 1 },{ "one", 1 },{ "one", 1 },{ "one", 1 } });

   for( auto it = argumentsTest.named_begin(); it != argumentsTest.named_end(); ++it )
   {
      std::cout << "Name: " << it->first << " Value: " << it->second.as_string() << "\n";
   }
}






namespace fs = std::filesystem;

std::string CreateTemporaryFile_s() 
{
   const std::string& stringPrefix = "temp";
   const std::string& stringSuffix = ".tmp";

   try 
   {
      // Get temp directory
      std::filesystem::path pathTemporaryDirectory = std::filesystem::temp_directory_path();

      // Create unique filename
      std::filesystem::path pathTemporary = pathTemporaryDirectory / (stringPrefix + "XXXXXX" + stringSuffix);

      // Generate unique path
      std::string stringTemplate = pathTemporary.string();

      // For platforms that support mkstemp/mktemp
#ifndef _WIN32
      char* piResult = mkstemps(const_cast<char*>(stringTemplate.c_str()), stringSuffix.length());
      if(piResult == nullptr) { throw std::runtime_error("Failed to create temporary file"); }
      return stringTemplate;
#else
      // Windows fallback - create unique name manually
      char piUniqueName[MAX_PATH];
      if( ::GetTempFileNameA(pathTemporaryDirectory.string().c_str(), stringPrefix.c_str(), 0, piUniqueName) == 0) { throw std::runtime_error("Failed to create temporary file on Windows"); }

      // Rename to include suffix if needed
      if(!stringSuffix.empty() && stringSuffix != ".tmp") 
      {
         std::filesystem::path original(piUniqueName);
         std::filesystem::path new_path = original.parent_path() / (original.stem().string() + stringSuffix);
         std::filesystem::rename(original, new_path);
         return new_path.string();
      }

      return piUniqueName;
#endif
   }
   catch (const std::exception& e) 
   {
      std::cerr << "Error creating temp file: " << e.what() << std::endl;
      return "";
   }
}
