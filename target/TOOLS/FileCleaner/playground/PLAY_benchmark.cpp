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

#include "catch2/catch_amalgamated.hpp"

TEST_CASE("[benchmark] hot and cold", "[benchmark]")
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

   gd::benchmark::flusher flusherCold(gd::benchmark::flush_settings::l2_s());
   const std::vector<int> vectorInput(100'000, 0);
   for(unsigned uSortColumn = 0; uSortColumn < 7; ++uSortColumn)
   {
      const auto resultSort01 = gd::benchmark::run_s(
         [&]()
         {
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_key{});
            return tableTarget.size();
         }, uRunIterations, uWarmupCount);

      

      const auto resultSort02 = gd::benchmark::run_cold_s(
         [&]()
         {
            tableTarget.row_clear();
            tableTarget.append(tableSource);
            tableTarget.sort(uSortColumn, true, gd::table::tag_sort_key{});
            return tableTarget.size();
         }, flusherCold, uRunIterations, uWarmupCount );



      std::cout << "\n\nColumn: " << uSortColumn << " =======================";

      print_result("sort table 01", resultSort01);
      print_result("sort table 02 (cold)", resultSort02);
   }

}

TEST_CASE("[benchmark] table dto", "[.benchmark]")
{
   constexpr unsigned uRunIterations = 50;
   constexpr unsigned uWarmupCount = 1;

   using namespace gd::table::dto;
   table tableDto((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", "C1" },{ "uint64", "C2" },{ "uint64", "C3" },{ "uint64", "SUM" } }, gd::table::tag_prepare{});

   // Add 10000000 rows to table
   for(uint64_t u = 0; u < 10'000'000; ++ u)
   {
      auto uRow = tableDto.row_add_one();
      tableDto.cell_set(uRow, 0, u);
      tableDto.cell_set(uRow, 1, u * 2);
      tableDto.cell_set(uRow, 2, u * 3);
      //tableDto.cell_set(uRow, 3, u + (u * 2) + (u * 3));
   }

   uint64_t uCheckSum = 0;

   // calculate the checksum to verify the results are correct
   for(auto itRow : tableDto)
   {
      for(unsigned uCol = 0; uCol < 3; ++uCol)
      {
         auto value = itRow.cell_get_variant_view(uCol);
         uCheckSum += value.get_uint64();
      }
   }


   const auto resultSum01 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         uint64_t uTotalSum = 0;
         // sum columns into column 3 and also do a total and return the total
         for(auto itRow = tableDto.begin(); itRow != tableDto.end(); ++itRow)
         {
            auto uRow = *itRow;
            uint64_t uSum = 0;
            for(unsigned uCol = 0; uCol < 3; ++uCol)
            {
               auto value = itRow.cell_get_variant_view(uCol);
               if(value.is_null() == false)
               {
                  uSum += value.get_uint64();
               }
            }
            itRow.cell_set(3, uSum);
            uTotalSum += uSum;
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.

   const auto resultSum02 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         uint64_t uTotalSum = 0;
         // sum columns into column 3 and also do a total and return the total
         for(uint64_t uRow = 0; uRow < tableDto.size(); ++uRow)
         {
            uint64_t uSum = 0;
            for(unsigned uCol = 0; uCol < 3; ++uCol)
            {
               auto value = tableDto.cell_get_variant_view(uRow, uCol);
               if(value.is_null() == false)
               {
                  uSum += value.get_uint64();
               }
            }
            tableDto.cell_set(uRow, 3, uSum);
            uTotalSum += uSum;
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.

   const auto resultSum03 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         uint64_t uTotalSum = 0;
         // sum columns into column 3 and also do a total and return the total
         for(uint64_t uRow = 0; uRow < tableDto.size(); ++uRow)
         {
            uint64_t uSum = 0;
            for(unsigned uCol = 0; uCol < 3; ++uCol)
            {
               auto value = tableDto.cell_get_variant_view(uRow, uCol);
               uSum += value.get_uint64();
            }
            tableDto.cell_set(uRow, 3, uSum);
            uTotalSum += uSum;
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.

   const auto resultSumSpecial = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         uint64_t uTotalSum = 0;
         // sum columns into column 3 and also do a total and return the total
         for (uint64_t uRow = 0; uRow < tableDto.size(); ++uRow)
         {
            uint64_t uSum = 0;
            for (unsigned uCol = 0; uCol < 3; ++uCol)
            {
               auto value = tableDto.cell_get_variant_view(uRow, uCol);
               uSum += (uint64_t)value;
            }
            tableDto.cell_set(uRow, 3, uSum);
            uTotalSum += uSum;
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.


   const auto resultSum04 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         struct row
         {
            uint64_t uC1;
            uint64_t uC2;
            uint64_t uC3;
         };

         uint64_t uTotalSum = 0;
         // sum columns into column 3 and also do a total and return the total
         for(uint64_t uRow = 0; uRow < tableDto.size(); ++uRow)
         {
            uint64_t uSum = 0;
            auto* prow = (row*)tableDto.cell_get(uRow, 0);
            uSum = prow->uC1 + prow->uC2 + prow->uC3;
            uTotalSum += uSum;
            tableDto.cell_set(uRow, 3, uSum);
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.

   const auto resultSum05 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         struct row
         {
            uint64_t uC1;
            uint64_t uC2;
            uint64_t uC3;
            uint64_t uSum;
         };

         uint64_t uTotalSum = 0;
         // sum columns into column 3 and also do a total and return the total
         for(uint64_t uRow = 0; uRow < tableDto.size(); ++uRow)
         {
            uint64_t uSum = 0;
            auto* prow = (row*)tableDto.cell_get(uRow, 0);
            uSum = prow->uC1 + prow->uC2 + prow->uC3;
            uTotalSum += uSum;
            prow->uSum = uSum;
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.

   const auto resultSum06 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         struct row
         {
            uint64_t uC1;
            uint64_t uC2;
            uint64_t uC3;
            uint64_t uSum;
         };

         constexpr uint64_t uUnroll = 4;
         const uint64_t uRowCount = tableDto.size();
         uint64_t uTotalSum0 = 0;
         uint64_t uTotalSum1 = 0;
         uint64_t uTotalSum2 = 0;
         uint64_t uTotalSum3 = 0;

         // Unroll by 4 rows so independent sums can run in parallel on OoO CPUs.
         uint64_t uRow = 0;
         for(; (uRow + (uUnroll - 1)) < uRowCount; uRow += uUnroll)
         {
            auto* prow0 = (row*)tableDto.cell_get(uRow + 0, 0);
            auto* prow1 = (row*)tableDto.cell_get(uRow + 1, 0);
            auto* prow2 = (row*)tableDto.cell_get(uRow + 2, 0);
            auto* prow3 = (row*)tableDto.cell_get(uRow + 3, 0);

            const uint64_t uSum0 = prow0->uC1 + prow0->uC2 + prow0->uC3;
            const uint64_t uSum1 = prow1->uC1 + prow1->uC2 + prow1->uC3;
            const uint64_t uSum2 = prow2->uC1 + prow2->uC2 + prow2->uC3;
            const uint64_t uSum3 = prow3->uC1 + prow3->uC2 + prow3->uC3;

            prow0->uSum = uSum0;
            prow1->uSum = uSum1;
            prow2->uSum = uSum2;
            prow3->uSum = uSum3;

            uTotalSum0 += uSum0;
            uTotalSum1 += uSum1;
            uTotalSum2 += uSum2;
            uTotalSum3 += uSum3;
         }

         uint64_t uTotalSum = uTotalSum0 + uTotalSum1 + uTotalSum2 + uTotalSum3;

         // Handle remaining rows when row count is not divisible by 4.
         for(; uRow < uRowCount; ++uRow)
         {
            auto* prow = (row*)tableDto.cell_get(uRow, 0);
            const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
            prow->uSum = uSum;
            uTotalSum += uSum;
         }

         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);    // Measured calls and Warm-up calls.


   const auto resultSum07 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         struct row
         {
            uint64_t uC1;
            uint64_t uC2;
            uint64_t uC3;
            uint64_t uSum;
         };

         constexpr uint64_t uUnroll = 4;
         const uint64_t uRowCount = tableDto.size();
         uint64_t uTotalSum0 = 0;
         uint64_t uTotalSum1 = 0;
         uint64_t uTotalSum2 = 0;
         uint64_t uTotalSum3 = 0;

         // Hämta bas-pekaren EN gång
         auto* __restrict pBase = (row*)tableDto.cell_get(0, 0);

         uint64_t uRow = 0;
         for(; (uRow + (uUnroll - 1)) < uRowCount; uRow += uUnroll)
         {
            auto* __restrict prow0 = pBase + (uRow + 0);
            auto* __restrict prow1 = pBase + (uRow + 1);
            auto* __restrict prow2 = pBase + (uRow + 2);
            auto* __restrict prow3 = pBase + (uRow + 3);

            const uint64_t uSum0 = prow0->uC1 + prow0->uC2 + prow0->uC3;
            const uint64_t uSum1 = prow1->uC1 + prow1->uC2 + prow1->uC3;
            const uint64_t uSum2 = prow2->uC1 + prow2->uC2 + prow2->uC3;
            const uint64_t uSum3 = prow3->uC1 + prow3->uC2 + prow3->uC3;

            prow0->uSum = uSum0;
            prow1->uSum = uSum1;
            prow2->uSum = uSum2;
            prow3->uSum = uSum3;

            uTotalSum0 += uSum0;
            uTotalSum1 += uSum1;
            uTotalSum2 += uSum2;
            uTotalSum3 += uSum3;
         }

         uint64_t uTotalSum = uTotalSum0 + uTotalSum1 + uTotalSum2 + uTotalSum3;

         for(; uRow < uRowCount; ++uRow)
         {
            auto* __restrict prow = pBase + uRow;
            const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
            prow->uSum = uSum;
            uTotalSum += uSum;
         }
         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);


   const auto resultSum08 = gd::benchmark::run_s(
      [&tableDto, uCheckSum]()
      {
         struct row
         {
            uint64_t uC1;
            uint64_t uC2;
            uint64_t uC3;
            uint64_t uSum;
         };

         const uint64_t uRowCount = tableDto.size();
         uint64_t uTotalSum = 0;

         // Hämta bas-pekaren EN gång utanför loopen
         auto* __restrict pBase = (row*)tableDto.cell_get(0, 0);

         // En helt vanlig loop, precis som Sum05, men med konstant pekarsteg
         for(uint64_t uRow = 0; uRow < uRowCount; ++uRow)
         {
            auto* __restrict prow = pBase + uRow;

            const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
            prow->uSum = uSum;
            uTotalSum += uSum;
         }

         assert(uTotalSum == uCheckSum);
         return uTotalSum;

      }, uRunIterations, uWarmupCount);
   
   auto print_result_ = [](std::string_view stringBenchmarkName, const auto& result_)
   {
      std::cout << "\nBenchmark: " << stringBenchmarkName << "\n";
      std::cout << "Mean: " << result_.mean() << " ns\n";
      std::cout << "Minimum: " << result_.min_time() << " ns\n";
      std::cout << "Maximum: " << result_.max_time() << " ns\n";
      std::cout << "Total in milliseconds: " << (result_.total_duration() / 1'000'000.0) << " ms\n";
   };

   print_result_("resultSum01", resultSum01);
   print_result_("resultSum02", resultSum02);
   print_result_("resultSum03 no null check", resultSum03);
   print_result_("resultSumSpecial no null check", resultSumSpecial);
   print_result_("resultSum04 struct three values", resultSum04);
   print_result_("resultSum05 struct three values with checksum", resultSum05);
   print_result_("resultSum06 struct + unroll 4 for OoO", resultSum06);
   print_result_("resultSum07 struct + unroll 4 for OoO + restrict", resultSum07);
   print_result_("resultSum08 struct + restrict", resultSum08);
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
