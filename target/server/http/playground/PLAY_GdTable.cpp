#include <array>
#include <filesystem>

#include "gd/gd_binary.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"
#include "gd/gd_uuid.h"

#include "gd/gd_sql_query.h"
#include "gd/gd_sql_query_builder.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("[gd-table] custom columns", "[gd-table]")
{
   {
      gd::table::arguments::table tableFiles(gd::table::tag_full_meta{});
      tableFiles.column_prepare();
      tableFiles.column_add("rstring", 0, "path");
      tableFiles.column_add("rstring", 0, "name");
      tableFiles.column_add("uint64", 0, "size");
      tableFiles.prepare();

      for(unsigned uRowIndex = 0; uRowIndex < 100; ++uRowIndex)
      {
         const auto uRow = tableFiles.row_add_one();
         tableFiles.row_set(uRow, { { "path", "C:\\data\\files\\entry.bin" }, { "name", "entry.bin" }, { "size", 1000 + uRowIndex } }, gd::table::tag_convert{});
         tableFiles.cell_set(uRow, "custom_file_category", gd::variant_view((uRowIndex % 2) == 0 ? "binary" : "text"));
         tableFiles.cell_set(uRow, "custom_file_region", gd::variant_view((uRowIndex % 3) == 0 ? "north" : "south"));
      }

      for(unsigned uRowIndex = 0; uRowIndex < 100; ++uRowIndex)
      {
         const auto bCategoryOk = tableFiles.cell_get_variant_view(uRowIndex, "custom_file_category").as_string_view() == ((uRowIndex % 2) == 0 ? "binary" : "text"); REQUIRE(bCategoryOk);
         const auto bRegionOk = tableFiles.cell_get_variant_view(uRowIndex, "custom_file_region").as_string_view() == ((uRowIndex % 3) == 0 ? "north" : "south"); REQUIRE(bRegionOk);
      }
   }

   {
      gd::table::arguments::table tableUsers(gd::table::tag_full_meta{});
      tableUsers.column_prepare();
      tableUsers.column_add({ { "uint64", 0, "user_id" }, { "rstring", 0, "username" }, { "string", 20, "email" } }, gd::table::tag_type_name{});
      tableUsers.prepare();

      for(unsigned uRowIndex = 0; uRowIndex < 100; ++uRowIndex)
      {
         const auto uRow = tableUsers.row_add_one();
         tableUsers.row_set(uRow, { { "user_id", 10000 + uRowIndex }, { "username", "demo-user" }, { "email", "demo@example.com" } }, gd::table::tag_convert{});
         tableUsers.cell_set(uRow, "custom_user_tier", gd::variant_view((uRowIndex % 4) == 0 ? "gold" : "standard"));
         tableUsers.cell_set(uRow, "custom_user_channel", gd::variant_view((uRowIndex % 5) == 0 ? "partner" : "direct"));
      }

      for(unsigned uRowIndex = 0; uRowIndex < 100; ++uRowIndex)
      {
         const auto bTierOk = tableUsers.cell_get_variant_view(uRowIndex, "custom_user_tier").as_string_view() == ((uRowIndex % 4) == 0 ? "gold" : "standard"); REQUIRE(bTierOk);
         const auto bChannelOk = tableUsers.cell_get_variant_view(uRowIndex, "custom_user_channel").as_string_view() == ((uRowIndex % 5) == 0 ? "partner" : "direct"); REQUIRE(bChannelOk);
      }
   }

   {
      using namespace gd::table::arguments;
      unsigned uFlags = table::eTableFlagArguments | table::eTableFlagNull64 | table::eTableFlagRowStatus;
      gd::table::arguments::table tableMetrics(uFlags, { {"string", 5, "service"}, {"uint64", 0, "epoch"}, {"uint64", 0, "requests"}}, gd::table::tag_prepare{});

      for(unsigned uRowIndex = 0; uRowIndex < 100; ++uRowIndex)
      {
         const auto uRow = tableMetrics.row_add_one();
         tableMetrics.row_set(uRow, { { "epoch", 1700000000 + uRowIndex }, { "requests", 500 + uRowIndex } }, gd::table::tag_convert{});
         tableMetrics.cell_set(uRow, "service", gd::variant_view("api-service"), gd::table::tag_adjust{});
         tableMetrics.cell_set(uRow, "custom_metric_bucket", gd::variant_view((uRowIndex % 10) < 5 ? "low" : "high"));
         tableMetrics.cell_set(uRow, "custom_metric_window", gd::variant_view((uRowIndex % 2) == 0 ? "day" : "night"));
      }

      for(unsigned uRowIndex = 0; uRowIndex < 100; ++uRowIndex)
      {
         const auto bBucketOk = tableMetrics.cell_get_variant_view(uRowIndex, "custom_metric_bucket").as_string_view() == ((uRowIndex % 10) < 5 ? "low" : "high"); REQUIRE(bBucketOk);
         const auto bWindowOk = tableMetrics.cell_get_variant_view(uRowIndex, "custom_metric_window").as_string_view() == ((uRowIndex % 2) == 0 ? "day" : "night"); REQUIRE(bWindowOk);
      }

      std::string string_d = gd::table::arguments::debug::print(tableMetrics);
      std::cout << string_d << std::endl;
   }
}
