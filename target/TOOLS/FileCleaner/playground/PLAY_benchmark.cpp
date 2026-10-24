#include <cassert>
#include <iostream>
#include <filesystem>

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

TEST_CASE("[benchmark] string_transport", "[.benchmark]")
{
   std::array<std::byte, 150> arrayBuffer;

   const auto resultTransport01 = gd::benchmark::run_s(
      [&arrayBuffer]()
      {
         gd::string_transport stringText(arrayBuffer);
         stringText = "0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         return stringText;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.

   const auto resultTransport02 = gd::benchmark::run_s(
      []()
      {
         gd::string_transport stringText;
         stringText = "0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         return stringText;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.

   const auto resultTransport03 = gd::benchmark::run_s(
      []()
      {
         std::string stringText;
         stringText = "0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         return stringText;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.



   std::cout << "Benchmark: string_transport using buffer\n";
   std::cout << "Mean: " << resultTransport01.mean() << " ns\n";
   std::cout << "Minimum: " << resultTransport01.min_time() << " ns\n";
   std::cout << "Maximum: " << resultTransport01.max_time() << " ns\n";

   std::cout << "\nBenchmark: string_transport with no buffer\n";
   std::cout << "Mean: " << resultTransport02.mean() << " ns\n";
   std::cout << "Minimum: " << resultTransport02.min_time() << " ns\n";
   std::cout << "Maximum: " << resultTransport02.max_time() << " ns\n";

   std::cout << "\nBenchmark: std::string\n";
   std::cout << "Mean: " << resultTransport03.mean() << " ns\n";
   std::cout << "Minimum: " << resultTransport03.min_time() << " ns\n";
   std::cout << "Maximum: " << resultTransport03.max_time() << " ns\n";
}

TEST_CASE("[benchmark] table dto", "[benchmark]")
{
   constexpr unsigned uRunIterations = 1000;
   constexpr unsigned uWarmupCount = 10;
   constexpr unsigned uBatchCount = 20;

   using namespace gd::table::dto;
   table tableDto((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", "C1" },{ "uint64", "C2" },{ "uint64", "C3" },{ "uint64", "SUM" } }, gd::table::tag_prepare{});

   // Add 10000 rows to table
   for(uint64_t u = 0; u < 10000; ++ u)
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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.

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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.

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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.

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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.


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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.

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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.

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

      }, uRunIterations, uWarmupCount, uBatchCount);    // Measured calls and Warm-up calls and number of batched calls.



   
   std::cout << "\nBenchmark: resultSum01\n";
   std::cout << "Mean: " << resultSum01.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum01.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum01.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum01.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSum02\n";
   std::cout << "Mean: " << resultSum02.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum02.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum02.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum02.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSum03 no null check\n";
   std::cout << "Mean: " << resultSum03.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum03.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum03.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum03.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSumSpecial no null check\n";
   std::cout << "Mean: " << resultSumSpecial.mean() << " ns\n";
   std::cout << "Minimum: " << resultSumSpecial.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSumSpecial.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSumSpecial.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSum04 struct three values\n";
   std::cout << "Mean: " << resultSum04.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum04.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum04.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum04.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSum05 struct three values with checksum\n";
   std::cout << "Mean: " << resultSum05.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum05.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum05.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum05.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSum06 struct + unroll 4 for OoO\n";
   std::cout << "Mean: " << resultSum06.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum06.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum06.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum06.total_duration_ns() / 1'000'000.0) << " ms\n";

}


TEST_CASE("[benchmark] string_transport02", "[.benchmark]")
{
   /*const auto resultTransport03 = gd::benchmark::run_s(
      []()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         std::string stringText;
         stringText = "0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         stringText += " 0123456789!";
         return stringText;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.

   const auto resultVector = gd::benchmark::run_s(
      []()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         std::vector<char> vectorChar;
         vectorChar.push_back('0');
         vectorChar.push_back('1');
         vectorChar.push_back('2');
         vectorChar.push_back('3');
         vectorChar.push_back('4');
         vectorChar.push_back('5');
         return vectorChar;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.

   const auto resultVector2 = gd::benchmark::run_s(
      []()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         std::vector<std::string> vectorString;
         vectorString.push_back("0123456789!");
         vectorString.push_back("0123456789!");
         vectorString.push_back("0123456789!");
         vectorString.push_back("0123456789!");
         vectorString.push_back("0123456789!");
         vectorString.push_back("0123456789!");
         return vectorString;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.
  

   const auto resultVector3 = gd::benchmark::run_s(
      []()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         std::vector<gd::string_transport> vectorString;
         vectorString.emplace_back("0123456789!");
         vectorString.emplace_back("0123456789!");
         vectorString.emplace_back("0123456789!");
         vectorString.emplace_back("0123456789!");
         vectorString.emplace_back("0123456789!");
         vectorString.emplace_back("0123456789!");
         return vectorString;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.  */

   const auto resultArgument = gd::benchmark::run_s(
      []()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         gd::argument::arguments arguments;
         arguments.append("one","0123456789!");
         arguments.append("two","0123456789!");
         arguments.append("three","0123456789!");
         arguments.append("four","0123456789!");
         arguments.append("five","0123456789!");
         arguments.append("six","0123456789!");
         //arguments["seven"] = "0123456789!";
         return arguments;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.

   std::array<std::byte, 280> arrayBuffer;

   const auto resultArgument2 = gd::benchmark::run_s(
      [&arrayBuffer]()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         gd::argument::arguments arguments(arrayBuffer);
         arguments.append("one", "0123456789!");
         arguments.append("two", "0123456789!");
         arguments.append("three", "0123456789!");
         arguments.append("four", "0123456789!");
         arguments.append("five", "0123456789!");
         arguments.append("six", "0123456789!");
         //arguments["seven"] = "0123456789!";
         return arguments;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.

   const auto resultArgument3 = gd::benchmark::run_s(
      [&arrayBuffer]()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
         gd::argument::arguments arguments;
         arguments["one"] = "0123456789!";
         arguments["two"] = "0123456789!";
         arguments["three"] = "0123456789!";
         arguments["four"] = "0123456789!";
         arguments["five"] = "0123456789!";
         arguments["six"] = "0123456789!";
         arguments["seven"] = "0123456789!";
         arguments["eight"] = "0123456789!";
         //arguments["seven"] = "0123456789!";
         return arguments;
      }, 100000, 100, 100);    // Measured calls and Warm-up calls.


  /* std::cout << "\nBenchmark: std::string\n";
   std::cout << "Mean: " << resultTransport03.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultTransport03.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultTransport03.m_dMaxTime << " ns\n";

   std::cout << "\nBenchmark: std::vector<char>\n";
   std::cout << "Mean: " << resultVector.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultVector.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultVector.m_dMaxTime << " ns\n";

   std::cout << "\nBenchmark: std::vector<std::string>\n";
   std::cout << "Mean: " << resultVector2.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultVector2.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultVector2.m_dMaxTime << " ns\n";

   std::cout << "\nBenchmark: std::vector<gd::string_transport>\n";
   std::cout << "Mean: " << resultVector3.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultVector3.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultVector3.m_dMaxTime << " ns\n";  */

   std::cout << "\nBenchmark: gd::argument::arguments with no buffer\n";
   std::cout << "Mean: " << resultArgument.mean() << " ns\n";
   std::cout << "Minimum: " << resultArgument.min_time() << " ns\n";
   std::cout << "Maximum: " << resultArgument.max_time() << " ns\n";
   
   std::cout << "\nBenchmark: gd::argument::arguments with buffer\n";
   std::cout << "Mean: " << resultArgument2.mean() << " ns\n";
   std::cout << "Minimum: " << resultArgument2.min_time() << " ns\n";
   std::cout << "Maximum: " << resultArgument2.max_time() << " ns\n";

   std::cout << "\nBenchmark: assign with []\n";
   std::cout << "Mean: " << resultArgument3.mean() << " ns\n";
   std::cout << "Minimum: " << resultArgument3.min_time() << " ns\n";
   std::cout << "Maximum: " << resultArgument3.max_time() << " ns\n";
}


// @FILE [tag: benchmark, table] [description: isolates why unrolling helps in the table dto sum benchmark]
//
// Add to the same translation unit as the existing "[benchmark] table dto" test
// (same includes: Catch2, gd::table::dto, gd::benchmark).
//
// Question: why is Sum06 (unroll 4, four accumulators) faster than Sum05?
// Three suspects, isolated one at a time:
//   (A) cell_get() called per row   vs.   base pointer + stride hoisted out of the loop
//   (B) unroll by 4                 vs.   no unroll
//   (C) four accumulators           vs.   one accumulator
//
//   Variant             cell_get      unroll   accumulators
//   Sum05 (reference)   per row       1        1
//   Sum06 (reference)   per row       4        4
//   Sum07               per row       4        1       <- tests (C)
//   Sum08               hoisted       1        1       <- tests (A)
//   Sum09               hoisted       4        1       <- tests (B) once (A) is removed
//   Sum10               hoisted       4        4       <- everything together
//
// Every variant is verified OUTSIDE the timed region with REQUIRE (assert() is compiled
// out in release builds, so the old check never ran). The expected total is computed
// from a formula, not read back through the table API.

namespace
{
   // Written at the end of every measured function so the total stays observable.
   volatile uint64_t g_uBenchmarkSink = 0;
}

TEST_CASE("[benchmark] table dto unroll", "[benchmark]")
{
   using namespace gd::table::dto;

   constexpr unsigned uWarmupCount = 10;
   constexpr unsigned uBatchCount = 20;

   // same overlay as Sum05/Sum06 in the original test
   struct row
   {
      uint64_t uC1;
      uint64_t uC2;
      uint64_t uC3;
      uint64_t uSum;
   };

   // 10 000 rows fits in cache, 1 000 000 rows does not. If all variants converge to
   // the same ns/row at 1M, the loop is memory bound and unrolling cannot help.
   for(const uint64_t uRowCount : { uint64_t(10000), uint64_t(1000000) })
   {
      const unsigned uRunIterations = (uRowCount <= 10000) ? 1000 : 50;

      table tableDto((table::eTableFlagNull32 | table::eTableFlagRowStatus), { { "uint64", "C1" },{ "uint64", "C2" },{ "uint64", "C3" },{ "uint64", "SUM" } }, gd::table::tag_prepare{});

      uint64_t uCheckSum = 0;
      for(uint64_t u = 0; u < uRowCount; ++u)
      {
         auto uRow = tableDto.row_add_one();
         tableDto.cell_set(uRow, 0, u);
         tableDto.cell_set(uRow, 1, u * 2);
         tableDto.cell_set(uRow, 2, u * 3);
         uCheckSum += u + (u * 2) + (u * 3);                                   // expected total, independent of the table API
      }

      // ## Row layout used by the hoisted variants.
      // Assumes rows are contiguous with a constant stride. Verified here, not assumed.
      char* pbBase = (char*)tableDto.cell_get(0, 0);
      const size_t uStride = (size_t)((char*)tableDto.cell_get(1, 0) - pbBase);
      REQUIRE(uStride >= sizeof(row));
      REQUIRE((char*)tableDto.cell_get(uRowCount - 1, 0) == pbBase + (uRowCount - 1) * uStride);

      // ----------------------------------------------------------------------
      // Sum05 (reference): cell_get per row, no unroll, one accumulator
      auto callSum05 = [&tableDto, uRowCount]() -> uint64_t
         {
            uint64_t uTotalSum = 0;
            for(uint64_t uRow = 0; uRow < uRowCount; ++uRow)
            {
               auto* prow = (row*)tableDto.cell_get(uRow, 0);
               const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
               prow->uSum = uSum;
               uTotalSum += uSum;
            }
            g_uBenchmarkSink = uTotalSum;
            return uTotalSum;
         };

      // ----------------------------------------------------------------------
      // Sum06 (reference): cell_get per row, unroll 4, four accumulators
      auto callSum06 = [&tableDto, uRowCount]() -> uint64_t
         {
            uint64_t uTotalSum0 = 0, uTotalSum1 = 0, uTotalSum2 = 0, uTotalSum3 = 0;
            uint64_t uRow = 0;
            for(; uRow + 3 < uRowCount; uRow += 4)
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
            for(; uRow < uRowCount; ++uRow)
            {
               auto* prow = (row*)tableDto.cell_get(uRow, 0);
               const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
               prow->uSum = uSum;
               uTotalSum += uSum;
            }
            g_uBenchmarkSink = uTotalSum;
            return uTotalSum;
         };

      // ----------------------------------------------------------------------
      // Sum07: cell_get per row, unroll 4, ONE accumulator   (isolates C)
      auto callSum07 = [&tableDto, uRowCount]() -> uint64_t
         {
            uint64_t uTotalSum = 0;
            uint64_t uRow = 0;
            for(; uRow + 3 < uRowCount; uRow += 4)
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

               uTotalSum += uSum0;
               uTotalSum += uSum1;
               uTotalSum += uSum2;
               uTotalSum += uSum3;
            }
            for(; uRow < uRowCount; ++uRow)
            {
               auto* prow = (row*)tableDto.cell_get(uRow, 0);
               const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
               prow->uSum = uSum;
               uTotalSum += uSum;
            }
            g_uBenchmarkSink = uTotalSum;
            return uTotalSum;
         };

      // ----------------------------------------------------------------------
      // Sum08: hoisted base + stride, no unroll, one accumulator   (isolates A)
      auto callSum08 = [pbBase, uStride, uRowCount]() -> uint64_t
         {
            uint64_t uTotalSum = 0;
            char* pb = pbBase;
            for(uint64_t uRow = 0; uRow < uRowCount; ++uRow, pb += uStride)
            {
               auto* prow = (row*)pb;
               const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
               prow->uSum = uSum;
               uTotalSum += uSum;
            }
            g_uBenchmarkSink = uTotalSum;
            return uTotalSum;
         };

      // ----------------------------------------------------------------------
      // Sum09: hoisted, unroll 4, ONE accumulator   (does unrolling still matter without cell_get?)
      auto callSum09 = [pbBase, uStride, uRowCount]() -> uint64_t
         {
            uint64_t uTotalSum = 0;
            char* pb = pbBase;
            uint64_t uRow = 0;
            for(; uRow + 3 < uRowCount; uRow += 4, pb += 4 * uStride)
            {
               auto* prow0 = (row*)(pb);
               auto* prow1 = (row*)(pb + uStride);
               auto* prow2 = (row*)(pb + 2 * uStride);
               auto* prow3 = (row*)(pb + 3 * uStride);

               const uint64_t uSum0 = prow0->uC1 + prow0->uC2 + prow0->uC3;
               const uint64_t uSum1 = prow1->uC1 + prow1->uC2 + prow1->uC3;
               const uint64_t uSum2 = prow2->uC1 + prow2->uC2 + prow2->uC3;
               const uint64_t uSum3 = prow3->uC1 + prow3->uC2 + prow3->uC3;

               prow0->uSum = uSum0;
               prow1->uSum = uSum1;
               prow2->uSum = uSum2;
               prow3->uSum = uSum3;

               uTotalSum += uSum0;
               uTotalSum += uSum1;
               uTotalSum += uSum2;
               uTotalSum += uSum3;
            }
            for(; uRow < uRowCount; ++uRow, pb += uStride)
            {
               auto* prow = (row*)pb;
               const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
               prow->uSum = uSum;
               uTotalSum += uSum;
            }
            g_uBenchmarkSink = uTotalSum;
            return uTotalSum;
         };

      // ----------------------------------------------------------------------
      // Sum10: hoisted, unroll 4, FOUR accumulators   (everything together)
      auto callSum10 = [pbBase, uStride, uRowCount]() -> uint64_t
         {
            uint64_t uTotalSum0 = 0, uTotalSum1 = 0, uTotalSum2 = 0, uTotalSum3 = 0;
            char* pb = pbBase;
            uint64_t uRow = 0;
            for(; uRow + 3 < uRowCount; uRow += 4, pb += 4 * uStride)
            {
               auto* prow0 = (row*)(pb);
               auto* prow1 = (row*)(pb + uStride);
               auto* prow2 = (row*)(pb + 2 * uStride);
               auto* prow3 = (row*)(pb + 3 * uStride);

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
            for(; uRow < uRowCount; ++uRow, pb += uStride)
            {
               auto* prow = (row*)pb;
               const uint64_t uSum = prow->uC1 + prow->uC2 + prow->uC3;
               prow->uSum = uSum;
               uTotalSum += uSum;
            }
            g_uBenchmarkSink = uTotalSum;
            return uTotalSum;
         };

      // ## Verify every variant OUTSIDE the timed region (works in release builds).
      // Column 3 is cleared first so a variant that forgets to write it is caught.
      auto verify = [&](auto&& call)
         {
            for(uint64_t u = 0; u < uRowCount; ++u) { ((row*)(pbBase + u * uStride))->uSum = 0; }
            REQUIRE(call() == uCheckSum);
            bool bOk = true;
            for(uint64_t u = 0; u < uRowCount; ++u) { bOk = bOk && (((row*)(pbBase + u * uStride))->uSum == u * 6); }
            REQUIRE(bOk == true);
         };

      verify(callSum05);
      verify(callSum06);
      verify(callSum07);
      verify(callSum08);
      verify(callSum09);
      verify(callSum10);

      // ## Measure
      const auto resultSum05 = gd::benchmark::run_s(callSum05, uRunIterations, uWarmupCount, uBatchCount);
      const auto resultSum06 = gd::benchmark::run_s(callSum06, uRunIterations, uWarmupCount, uBatchCount);
      const auto resultSum07 = gd::benchmark::run_s(callSum07, uRunIterations, uWarmupCount, uBatchCount);
      const auto resultSum08 = gd::benchmark::run_s(callSum08, uRunIterations, uWarmupCount, uBatchCount);
      const auto resultSum09 = gd::benchmark::run_s(callSum09, uRunIterations, uWarmupCount, uBatchCount);
      const auto resultSum10 = gd::benchmark::run_s(callSum10, uRunIterations, uWarmupCount, uBatchCount);

      // ## Report in ns per row so different row counts can be compared
      auto print = [uRowCount](const char* pbszName, const auto& result)
         {
            const double dMean = static_cast<double>(result.mean());
            const double dMin = static_cast<double>(result.min_time());
            std::cout << std::left << std::setw(44) << pbszName
               << std::right << std::fixed << std::setprecision(0) << std::setw(12) << dMean << " ns"
               << std::setprecision(3) << std::setw(10) << (dMean / static_cast<double>(uRowCount)) << " ns/row"
               << std::setw(10) << (dMin / static_cast<double>(uRowCount)) << " min ns/row\n";
         };

      std::cout << "\n=== rows: " << uRowCount << "  stride: " << uStride << " bytes ===\n";
      print("Sum05 cell_get, no unroll, 1 acc (ref)", resultSum05);
      print("Sum06 cell_get, unroll 4, 4 acc  (ref)", resultSum06);
      print("Sum07 cell_get, unroll 4, 1 acc", resultSum07);
      print("Sum08 hoisted,  no unroll, 1 acc", resultSum08);
      print("Sum09 hoisted,  unroll 4, 1 acc", resultSum09);
      print("Sum10 hoisted,  unroll 4, 4 acc", resultSum10);
   }
}
