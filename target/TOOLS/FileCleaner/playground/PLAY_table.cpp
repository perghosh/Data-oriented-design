#include <filesystem>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"

#include "../Application.h"

#include "main.h"

#include "../Command.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("[table] multiple strings", "[table]") {
   using namespace gd::table::dto;

   const std::string stringCharset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   constexpr unsigned uTableDuplicate = (table::eTableFlagNull32|table::eTableFlagRowStatus|table::eTableFlagDuplicateStrings);
   gd::table::dto::table tableTest01( uTableDuplicate, { { "int64", "KeyK"}, { "rstring", "name"}, { "rstring", "text"} }, gd::table::tag_prepare{} );

   std::mt19937 mt19937;

   // Generate 100 random strings using stl
   std::vector<std::string> vectorRandomStrings;
   for(int i = 0; i < 100; ++i) 
   {
       std::string string_;
       for(int j = 0; j < 10; ++j) 
       {
          string_ += stringCharset[mt19937() % stringCharset.size()];
       }
       vectorRandomStrings.push_back(string_);
   }

   for( const auto& string_ : vectorRandomStrings ) 
   {
      auto uRow = tableTest01.row_add_one();
      tableTest01.row_set(uRow, { {"KeyK", (int64_t)uRow}, {"name", string_}, {"text", string_} } );
   }

   // read all names and texts
   for( uint64_t uRow = 0; uRow < tableTest01.size(); ++uRow ) 
   {
      auto stringName = tableTest01.cell_get_variant_view(uRow, "name").as_string_view();
      auto stringText = tableTest01.cell_get_variant_view(uRow, "text").as_string_view();
   }

   uint64_t uTableSize = tableTest01.storage_size( gd::table::tag_columns{} );

   std::vector<uint8_t> vectorBuffer;
   vectorBuffer.resize(uTableSize);

   tableTest01.storage_write(reinterpret_cast<std::byte*>(vectorBuffer.data()), gd::table::tag_columns{});

   std::byte* pBuffer = reinterpret_cast<std::byte*>( vectorBuffer.data() );

   gd::table::dto::table tableTestRead;
   uint64_t uSize = tableTestRead.storage_read_size(pBuffer);
   tableTestRead.storage_read( (const std::byte*)vectorBuffer.data(), gd::table::tag_columns{});

   //gd::table::dto::table tableTest02(pBuffer, uTableSize, gd::table::tag_columns{});
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

