#include <algorithm>
#include <atomic>
#include <chrono>
#include <cassert>
#include <iostream>
#include <filesystem>
#include <random>
#include <latch>
#include <map>
#include <mutex>
#include <numeric>
#include <thread>
#include <random>

#include "gd/gd_file.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_benchmark.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_string_transport.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_table_io.h"

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("[benchmark] arguments", "[benchmark]")
{
   using namespace gd::argument::shared;

   std::array<std::byte, 64> array_;
   arguments arguments_(array_);

   arguments_.append("0123456789", 42);
   arguments_.append("012345678901234567890", 42);


}

TEST_CASE("[benchmark] hot and cold", "[.benchmark]")
{
   constexpr unsigned uRunIterations = 100;
   constexpr unsigned uWarmupCount = 3;
   constexpr unsigned uCreateRowCount = 10000;

   using namespace gd::table::dto;

   // Generate 100000 rows of data for the table with random values
   std::mt19937_64 generatorRandom(42);                                     // fixed seed gives repeatable data
   std::uniform_int_distribution<uint64_t> distributionInteger(0, 1'000'000);
   std::uniform_real_distribution<double> distributionDouble(0.0, 1'000.0);
   std::uniform_int_distribution<int> distributionCharacter('a', 'z');

   table tableSource((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" },{ "int32", 0, "C5" },{ "uint32", 0, "C6" },{ "rstring", 50, "C7" } }, gd::table::tag_prepare{});

   for(uint64_t u = 0; u < uCreateRowCount; ++u)
   {
      auto uRow = tableSource.row_add_one();

      std::string stringValue(8, ' ');
      std::string stringValue2(25, ' ');
      for(auto& iCharacter : stringValue) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }
      for(auto& iCharacter : stringValue2) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }

      std::string stringValue6 = "01234567890123456789";
      stringValue6 += stringValue2;

      tableSource.cell_set(uRow, 0u, distributionInteger(generatorRandom));
      tableSource.cell_set(uRow, 1u, std::string_view(stringValue));
      tableSource.cell_set(uRow, 2u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 3u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 4u, static_cast<int32_t>(distributionInteger(generatorRandom) % 1000));
      tableSource.cell_set(uRow, 5u, static_cast<uint32_t>(distributionInteger(generatorRandom) % 1000));
      tableSource.cell_set(uRow, 6u, std::string_view(stringValue6));
   }

   REQUIRE(tableSource.size() == uCreateRowCount);

   uint64_t uRowCount = tableSource.size();
   table tableTarget((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" },{ "int32", 0, "C5" },{ "uint32", 0, "C6" },{ "rstring", 50, "C7" } }, gd::table::tag_prepare{});
   tableTarget.reserve(uRowCount);

   auto print_result = [](const char* pbszName, const auto& result)
      {
         std::cout << "\n\n#######\nBenchmark: " << pbszName << "\n";
         std::cout << "Mean: " << (result.mean() / 1'000.0) << " us\n";
         std::cout << "Minimum: " << (result.min_time() / 1'000.0) << " us\n";
         std::cout << "Maximum: " << (result.max_time() / 1'000.0) << " us\n";
         std::cout << "Total in milliseconds: " << (result.total_duration() / 1'000'000.0) << " ms\n";
      };

   //gd::benchmark::flusher flusherCold(gd::benchmark::flush_settings::l2_s());
   //const std::vector<int> vectorInput(100'000, 0);
   gd::benchmark::flusher flusherCold(gd::benchmark::flush_settings::all_s());

   auto setup_ = [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
      };

   for(unsigned uSortColumn = 0; uSortColumn < 7; ++uSortColumn)
   {
      auto sort_ = [&tableTarget, uSortColumn]()
         {
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_key{});
            return tableTarget.size();
         };

      const auto resultSort01 = gd::benchmark::run_prepared_s(setup_, sort_, nullptr, uRunIterations, uWarmupCount);
      const auto resultSort02 = gd::benchmark::run_prepared_s(setup_, sort_, &flusherCold, uRunIterations, uWarmupCount);

      std::cout << "\n\nColumn: " << uSortColumn << " =======================";
      print_result("sort table 01 (warm)", resultSort01);
      print_result("sort table 02 (cold)", resultSort02);
   }
}


TEST_CASE("[benchmark] edit", "[.benchmark]")
{
   using namespace gd::table::dto;

   table tableGet((table::eTableFlagNull32 | table::eTableFlagRowStatus),
      { { "string", 32, "C1" },{ "string", 32, "C2" },{ "string", 32, "C3" } }, gd::table::tag_prepare{});

   tableGet.row_add(5);
   for(uint64_t uRow = 0; uRow < tableGet.size(); ++uRow)
   {
      tableGet[uRow, "C1"] = { "100", "100", "100" };
   }

   table tableSet((table::eTableFlagNull32 | table::eTableFlagRowStatus), 
   { { "uint64", 0, "C1" },{ "uint64", 0, "C2" },{ "uint64", 0, "C3" } }, gd::table::tag_prepare{});


   tableSet.row_add(5);
   for( uint64_t uRow = 0; uRow < tableSet.size(); ++uRow)
   {
      tableSet[uRow,"C1"] = {"1", "1", "1"};
   }

   // ## Set values from get table
   auto resultSet = gd::benchmark::run_s(
      [&tableGet, &tableSet]()
      {
         for(uint64_t uRow = 0; uRow < tableGet.size(); ++uRow)
         {
            tableSet[uRow, "C1"] = tableGet[uRow, "C1"];
            tableSet[uRow, "C2"] = tableGet[uRow, "C2"];

            auto cellSet3 = tableSet[uRow, "C3"];
            auto cellGet3 = tableGet[uRow, "C3"];
            cellSet3 = cellGet3;
         }
         return tableSet.size();
      }, 100000, 10);

   std::cout << "\n\nBenchmark: set values from get table\n";
   std::cout << gd::benchmark::to_string_s(resultSet, "ns");
}

TEST_CASE("[demo] flexibility", "[demo]")
{
   using namespace gd::table::dto;

   // ## 1. The proxy way (what you already have) ------------------------------
   //    operator[] returns a cell<> proxy; assignment goes through
   //    cell_set( row, "C1", value ) — with implicit conversion via variant.
   table t1((table::eTableFlagNull32 | table::eTableFlagRowStatus),
            { { "uint64", 0, "C1" }, { "uint64", 0, "C3" }, { "uint64", 0, "C4" } },
            gd::table::tag_prepare{});

   t1.row_add(10);
   for( uint64_t uRow = 0; uRow < 10; ++uRow )
      t1[uRow, "C1"] = "1";                                // string -> uint64 conversion

   // ## 2. The bulk-fill way --------------------------------------------------
   //    Same column, no loop at all. column_fill writes every row in one call.
   table t2((table::eTableFlagNull32 | table::eTableFlagRowStatus),
            { { "uint64", 0, "C1" }, { "uint64", 0, "C3" }, { "uint64", 0, "C4" } },
            gd::table::tag_prepare{});
   t2.row_add(10);
   t2.column_fill("C1", gd::variant_view(uint64_t{1}));    // all 10 rows = 1

   // ## 3. The row-oriented way ----------------------------------------------
   //    Add and populate a row in a single call, by name, with conversion.
   table t3((table::eTableFlagNull32 | table::eTableFlagRowStatus),
            { { "uint64", 0, "C1" }, { "uint64", 0, "C3" }, { "uint64", 0, "C4" } },
            gd::table::tag_prepare{});
   for( int i = 0; i < 10; ++i )
      t3.row_add({ { "1" } }, gd::table::tag_convert{});

   // ## 4. The "plant from a plain vector" way -------------------------------
   //    Plant takes a std::vector<T> straight into a column.
   table t4((table::eTableFlagNull32 | table::eTableFlagRowStatus),
            { { "uint64", 0, "C1" }, { "uint64", 0, "C3" }, { "uint64", 0, "C4" } },
            gd::table::tag_prepare{});
   t4.row_add(10);
   std::vector<uint64_t> vOnes(10, 1);
   t4.plant("C1", vOnes);                                  // template plant<u64>

   // ## 5. The scripted way --------------------------------------------------
   //    Build the column definitions from a string, then fill by name.
   table t5((table::eTableFlagNull32 | table::eTableFlagRowStatus),
            "uint64,C1;uint64,C3;uint64,C4", gd::table::tag_parse{}, gd::table::tag_prepare{});
   t5.row_add(10);
   t5.column_fill(0, gd::variant_view(uint64_t{1}));

   // ## All five produce identical data ------------------------------------
   for( const table* p : { &t1, &t2, &t3, &t4, &t5 } )
   {
      REQUIRE( p->get_row_count() == 10 );
      REQUIRE( p->cell_get_variant_view(0, "C1") == gd::variant_view(uint64_t{1}) );
      REQUIRE( p->cell_get_variant_view(9, "C1") == gd::variant_view(uint64_t{1}) );
   }
}


/**
 * Example table with 10 rows and 4 columns, printed two ways:
 *  - tag_io_cli: grid made to look good in a terminal
 *  - tag_io_sql: write_insert_g, one multi-row INSERT statement
 */
TEST_CASE("[table_io] example cli and sql", "[.table_io]")
{
   using namespace gd::table::dto;

   // ## create table with 4 columns ........................................
   table tableCustomer((table::eTableFlagNull32 | table::eTableFlagRowStatus),
      { { "int64", 0, "id" },{ "string", 32, "name" },{ "string", 32, "city" },{ "bool", 0, "active" } },
      gd::table::tag_prepare{});

   // ## add 10 rows and fill them ..........................................
   const char* apbszName[10] = { "Anna", "Bjorn", "Cecilia", "David", "Eva", "Filip", "Greta", "Henrik", "Ingrid", "O'Brien" };
   const char* apbszCity[10] = { "Gothenburg", "Stockholm", "Malmo", "Uppsala", "Lund", "Boras", "Umea", "Kiruna", "Visby", "Dublin" };

   tableCustomer.row_add(10);
   for(uint64_t uRow = 0; uRow < tableCustomer.size(); ++uRow)
   {
      // initializer list sets "id" and the columns to the right of it
      tableCustomer[uRow, "id"] = { int64_t(uRow + 1), apbszName[uRow], apbszCity[uRow], (uRow % 2) == 0 };
   }

   tableCustomer[3, "city"].set_null();   // one null value to see how it is printed

   REQUIRE(tableCustomer.size() == 10);
   REQUIRE(tableCustomer.get_column_count() == 4);

   // ## cli: grid for terminal ..............................................
   std::cout << "\n\nCLI, default grid\n";
   std::cout << gd::table::to_string(tableCustomer, gd::table::tag_io_cli{}) << "\n";

   std::cout << "\nCLI, max column width 6\n";
   std::cout << gd::table::to_string(tableCustomer, uint32_t(6), gd::table::tag_io_cli{}) << "\n";

   std::cout << "\nCLI, header only\n";
   std::cout << gd::table::to_string(tableCustomer, gd::table::tag_io_header{}, gd::table::tag_io_cli{}) << "\n";

   // ## sql: insert queries .................................................
   std::string stringInsert;
   gd::table::write_insert_g("TCustomer", tableCustomer, stringInsert, gd::table::tag_io_sql{});
   std::cout << "\nSQL, all rows in one insert\n" << stringInsert << ";\n";

   // only some columns (id and name) and only rows 2 to 4
   std::string stringInsertPart;
   gd::table::write_insert_g("TCustomer", tableCustomer, 2, 3, std::vector<unsigned>{ 0, 1 }, stringInsertPart, gd::table::tag_io_sql{});
   std::cout << "\nSQL, rows 2-4, columns id and name\n" << stringInsertPart << ";\n";

   REQUIRE(stringInsert.starts_with("INSERT INTO TCustomer"));
}


TEST_CASE("[benchmark] sort table", "[.benchmark]")
{
   constexpr unsigned uRunIterations = 50;
   constexpr unsigned uWarmupCount = 3;
   constexpr unsigned uCreateRowCount = 1000;

   using namespace gd::table::dto;

   // Generate 100000 rows of data for the table with random values
   std::mt19937_64 generatorRandom(42);                                     // fixed seed gives repeatable data
   std::uniform_int_distribution<uint64_t> distributionInteger(0, 1'000'000);
   std::uniform_real_distribution<double> distributionDouble(0.0, 1'000.0);
   std::uniform_int_distribution<int> distributionCharacter('a', 'z');

   table tableSource((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" }, { "int32", 0, "C5" },{ "uint32", 0, "C6" }, { "string", 50, "C7" } }, gd::table::tag_prepare{});

   for (uint64_t u = 0; u < uCreateRowCount; ++u)
   {
      auto uRow = tableSource.row_add_one();

      std::string stringValue(8, ' ');
      std::string stringValue2(25, ' ');
      for (auto& iCharacter : stringValue) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }
      for (auto& iCharacter : stringValue2) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }

      std::string stringValue6 = "01234567890123456789";
      stringValue6 += stringValue2;

      tableSource.cell_set(uRow, 0u, distributionInteger(generatorRandom));
      tableSource.cell_set(uRow, 1u, std::string_view(stringValue));
      tableSource.cell_set(uRow, 2u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 6u, std::string_view(stringValue6));
   }

   REQUIRE(tableSource.size() == uCreateRowCount);

   uint64_t uRowCount = tableSource.size();
   table tableTarget((table::eTableFlagNull32 | table::eTableFlagRowStatus),{ { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" }, { "int32", 0, "C5" },{ "uint32", 0, "C6" }, { "string", 50, "C7" } }, gd::table::tag_prepare{});
   tableTarget.reserve(uRowCount);

   const auto resultSort01 = gd::benchmark::run_s(
      [ &tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(0u, true);
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   // check if table is sorted correctly
   for (uint64_t uRow = 1; uRow < tableTarget.size(); ++uRow)
   {
      auto valuePrev = tableTarget.cell_get_variant_view(uRow - 1, 0u);
      auto valueCurr = tableTarget.cell_get_variant_view(uRow, 0u);
      REQUIRE(valuePrev.get_uint64() <= valueCurr.get_uint64());
   }

   const auto resultSort02 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(1u, true);
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   const auto resultSort03 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(6u, true);
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);


   const auto resultSort04 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(0u, true, gd::table::tag_sort_index{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   const auto resultSort05 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(1u, true, gd::table::tag_sort_index{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   const auto resultSort06 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(6u, true, gd::table::tag_sort_index{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   const auto resultSort07 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(0u, true, gd::table::tag_sort_key{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   // check if table is sorted correctly after key sort
   for (uint64_t uRow = 1; uRow < tableTarget.size(); ++uRow)
   {
      auto valuePrev = tableTarget.cell_get_variant_view(uRow - 1, 0u);
      auto valueCurr = tableTarget.cell_get_variant_view(uRow, 0u);
      REQUIRE(valuePrev.get_uint64() <= valueCurr.get_uint64());
   }

   const auto resultSort08 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(1u, true, gd::table::tag_sort_key{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);

   const auto resultSort09 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(6u, true, gd::table::tag_sort_key{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount);


   // check if table is sorted correctly
   /*
   for (uint64_t uRow = 1; uRow < tableTarget.size(); ++uRow)
   {
      auto valuePrev = tableTarget.cell_get_variant_view(uRow - 1, 1u);
      auto valueCurr = tableTarget.cell_get_variant_view(uRow, 1u);
      REQUIRE(valuePrev.get_uint64() <= valueCurr.get_uint64());
   }
   */


   auto print_result = [](const char* pbszName, const auto& result)
      {
         std::cout << "\nBenchmark: " << pbszName << "\n";
         std::cout << "Mean: " << result.mean() << " ns\n";
         std::cout << "Minimum: " << result.min_time() << " ns\n";
         std::cout << "Maximum: " << result.max_time() << " ns\n";
         std::cout << "Total in milliseconds: " << (result.total_duration() / 1'000'000.0) << " ms\n";
      };

   print_result("sort table 01", resultSort01);
   print_result("sort table 02", resultSort02);
   print_result("sort table 03", resultSort03);
   print_result("sort table 04", resultSort04);
   print_result("sort table 05", resultSort05);
   print_result("sort table 06", resultSort06);
   print_result("sort table 07", resultSort07);
   print_result("sort table 08", resultSort08);
   print_result("sort table 09", resultSort09);

}




TEST_CASE("[benchmark] three sort methods for table", "[.benchmark]")
{
   constexpr unsigned uRunIterations = 10;
   constexpr unsigned uWarmupCount = 1;
   constexpr unsigned uCreateRowCount = 10000;

   using namespace gd::table::dto;

   // Generate 100000 rows of data for the table with random values
   std::mt19937_64 generatorRandom(42);                                     // fixed seed gives repeatable data
   std::uniform_int_distribution<uint64_t> distributionInteger(0, 1'000'000);
   std::uniform_real_distribution<double> distributionDouble(0.0, 1'000.0);
   std::uniform_int_distribution<int> distributionCharacter('a', 'z');

   table tableSource((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" },{ "int32", 0, "C5" },{ "uint32", 0, "C6" },{ "string", 50, "C7" } }, gd::table::tag_prepare{});

   for (uint64_t u = 0; u < uCreateRowCount; ++u)
   {
      auto uRow = tableSource.row_add_one();

      std::string stringValue(8, ' ');
      std::string stringValue2(25, ' ');
      for (auto& iCharacter : stringValue) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }
      for (auto& iCharacter : stringValue2) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }

      std::string stringValue6 = "01234567890123456789";
      stringValue6 += stringValue2;

      tableSource.cell_set(uRow, 0u, distributionInteger(generatorRandom));
      tableSource.cell_set(uRow, 1u, std::string_view(stringValue));
      tableSource.cell_set(uRow, 2u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 3u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 4u, static_cast<int32_t>(distributionInteger(generatorRandom) % 1000));
      tableSource.cell_set(uRow, 5u, static_cast<uint32_t>(distributionInteger(generatorRandom) % 1000)); 
      tableSource.cell_set(uRow, 6u, std::string_view(stringValue6));
   }

   REQUIRE(tableSource.size() == uCreateRowCount);

   uint64_t uRowCount = tableSource.size();
   table tableTarget((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" },{ "int32", 0, "C5" },{ "uint32", 0, "C6" },{ "string", 50, "C7" } }, gd::table::tag_prepare{});
   tableTarget.reserve(uRowCount);

   auto print_result = [](const char* pbszName, const auto& result)
      {
         std::cout << "\nBenchmark: " << pbszName << "\n";
         std::cout << "Mean: " << (result.mean() / 1'000.0) << " us\n";
         std::cout << "Minimum: " << (result.min_time() / 1'000.0) << " us\n";
         std::cout << "Maximum: " << (result.max_time() / 1'000.0) << " us\n";
         std::cout << "Total in milliseconds: " << (result.total_duration() / 1'000'000.0) << " ms\n";
      };

   for(unsigned uSortColumn = 0; uSortColumn < 7; ++uSortColumn)
   {

      const auto resultSort01 = gd::benchmark::run_s(
         [&]()
         {
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true);
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);

      const auto resultSort02 = gd::benchmark::run_s(
         [&]()
         {
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_index{});
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);

      const auto resultSort03 = gd::benchmark::run_s(
         [&]()
         {
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_key{});
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);


      std::cout << "\n\nColumn: " << uSortColumn << " =======================";

      print_result("sort table 01", resultSort01);
      print_result("sort table 02", resultSort02);
      print_result("sort table 03", resultSort03);
   }

}

inline void cache_evict_all()
{
   static std::vector<uint8_t> vectorTrash(128 * 1024 * 1024);
   for (size_t u = 0; u < vectorTrash.size(); u += 64) { vectorTrash[u]++; }
   std::atomic_signal_fence(std::memory_order_seq_cst);                      // stop compiler from removing the loop
}


TEST_CASE("[benchmark] three sort methods for table using rstring", "[.benchmark]")
{
   constexpr unsigned uRunIterations = 10;
   constexpr unsigned uWarmupCount = 0;
   constexpr unsigned uCreateRowCount = 10000;

   using namespace gd::table::dto;

   // Generate 100000 rows of data for the table with random values
   std::mt19937_64 generatorRandom(42);                                     // fixed seed gives repeatable data
   std::uniform_int_distribution<uint64_t> distributionInteger(0, 1'000'000);
   std::uniform_real_distribution<double> distributionDouble(0.0, 1'000.0);
   std::uniform_int_distribution<int> distributionCharacter('a', 'z');

   table tableSource((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" },{ "int32", 0, "C5" },{ "uint32", 0, "C6" },{ "rstring", 50, "C7" } }, gd::table::tag_prepare{});

   for (uint64_t u = 0; u < uCreateRowCount; ++u)
   {
      auto uRow = tableSource.row_add_one();

      std::string stringValue(8, ' ');
      std::string stringValue2(25, ' ');
      for (auto& iCharacter : stringValue) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }
      for (auto& iCharacter : stringValue2) { iCharacter = static_cast<char>(distributionCharacter(generatorRandom)); }

      std::string stringValue6 = "01234567890123456789";
      stringValue6 += stringValue2;

      tableSource.cell_set(uRow, 0u, distributionInteger(generatorRandom));
      tableSource.cell_set(uRow, 1u, std::string_view(stringValue));
      tableSource.cell_set(uRow, 2u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 3u, distributionDouble(generatorRandom));
      tableSource.cell_set(uRow, 4u, static_cast<int32_t>(distributionInteger(generatorRandom) % 1000));
      tableSource.cell_set(uRow, 5u, static_cast<uint32_t>(distributionInteger(generatorRandom) % 1000));
      tableSource.cell_set(uRow, 6u, std::string_view(stringValue6));
   }

   REQUIRE(tableSource.size() == uCreateRowCount);

   uint64_t uRowCount = tableSource.size();
   table tableTarget((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", 0, "C1" },{ "string", 8, "C2" },{ "double", 0, "C3" },{ "double", 0, "C4" },{ "int32", 0, "C5" },{ "uint32", 0, "C6" },{ "rstring", 50, "C7" } }, gd::table::tag_prepare{});
   tableTarget.reserve(uRowCount);

   auto print_result = [](const char* pbszName, const auto& result)
      {
         std::cout << "\nBenchmark: " << pbszName << "\n";
         std::cout << "Mean: " << (result.mean() / 1'000.0) << " us\n";
         std::cout << "Minimum: " << (result.min_time() / 1'000.0) << " us\n";
         std::cout << "Maximum: " << (result.max_time() / 1'000.0) << " us\n";
         std::cout << "Total in milliseconds: " << (result.total_duration() / 1'000'000.0) << " ms\n";
      };

   for (unsigned uSortColumn = 0; uSortColumn < 7; ++uSortColumn)
   {

      const auto resultSort01 = gd::benchmark::run_s(
         [&]()
         {
            // cache_evict_all();
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true);
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);

      const auto resultSort02 = gd::benchmark::run_s(
         [&]()
         {
            // cache_evict_all();
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_index{});
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);

      const auto resultSort03 = gd::benchmark::run_s(
         [&]()
         {
            // cache_evict_all();
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_key{});
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);


      std::cout << "\n\nColumn: " << uSortColumn << " =======================";

      print_result("sort table 01", resultSort01);
      print_result("sort table 02", resultSort02);
      print_result("sort table 03", resultSort03);
   }

}
