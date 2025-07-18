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

TEST_CASE("[table] custom columns", "[table]") 
{
   gd::table::arguments::table table_( gd::table::tag_full_meta{} );
}

