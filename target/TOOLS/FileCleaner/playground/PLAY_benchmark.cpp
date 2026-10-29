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

TEST_CASE("[benchmark] table dto", "[.benchmark]")
{
   constexpr unsigned uRunIterations = 10;
   constexpr unsigned uWarmupCount = 1;
   constexpr unsigned uBatchCount = 5;

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

      }, uRunIterations, uWarmupCount, uBatchCount);


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

      }, uRunIterations, uWarmupCount, uBatchCount);
   
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

   std::cout << "\nBenchmark: resultSum07 struct + unroll 4 for OoO + restrict\n";
   std::cout << "Mean: " << resultSum07.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum07.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum07.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum07.total_duration_ns() / 1'000'000.0) << " ms\n";

   std::cout << "\nBenchmark: resultSum08 struct + restrict\n";
   std::cout << "Mean: " << resultSum08.mean() << " ns\n";
   std::cout << "Minimum: " << resultSum08.min_time() << " ns\n";
   std::cout << "Maximum: " << resultSum08.max_time() << " ns\n";
   std::cout << "Total in milliseconds: " << (resultSum08.total_duration_ns() / 1'000'000.0) << " ms\n";


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




TEST_CASE("[benchmark] sort table", "[benchmark]")
{
   constexpr unsigned uRunIterations = 10;
   constexpr unsigned uWarmupCount = 1;
   constexpr unsigned uBatchCount = 5;
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
      }, uRunIterations, uWarmupCount, uBatchCount);

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
      }, uRunIterations, uWarmupCount, uBatchCount);

   const auto resultSort03 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(6u, true);
         return tableTarget.size();
      }, uRunIterations, uWarmupCount, uBatchCount);


   const auto resultSort04 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(0u, true, gd::table::tag_sort_index{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount, uBatchCount);

   const auto resultSort05 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(1u, true, gd::table::tag_sort_index{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount, uBatchCount);

   const auto resultSort06 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(6u, true, gd::table::tag_sort_index{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount, uBatchCount);

   const auto resultSort07 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(0u, true, gd::table::tag_sort_key{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount, uBatchCount);

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
      }, uRunIterations, uWarmupCount, uBatchCount);

   const auto resultSort09 = gd::benchmark::run_s(
      [&tableSource, &tableTarget]()
      {
         tableTarget.row_clear();
         tableTarget.append(tableSource);
         tableTarget.sort(6u, true, gd::table::tag_sort_key{});
         return tableTarget.size();
      }, uRunIterations, uWarmupCount, uBatchCount);


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
         std::cout << "Total in milliseconds: " << (result.total_duration_ns() / 1'000'000.0) << " ms\n";
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




/**
 * @file test_sort_threads.cpp
 * @brief Threaded test for `tag_sort_index` and `tag_sort_key` in table_column_buffer
 *
 * Each thread owns its own copy of the table, sorts it with both algorithms for three columns and
 * both sort orders, and checks the result against an expected table that was sorted single threaded
 * before any thread started. Timings are collected in one shared map protected with a mutex.
 * Failures are counted in an atomic counter because Catch2 assertions (REQUIRE) are not thread safe
 * and are only used in the main thread after all threads have joined.
 *
 * @note This tests that separate table objects can be sorted at the same time. It does not test
 *       sorting one single table object from several threads, that is not supported.
 */

#include <string>
#include <string_view>



namespace {

   constexpr unsigned ROW_COUNT_TEST = 1000;                                      // rows in table that is sorted
   constexpr unsigned COLUMN_COUNT_TEST = 3;                                      // sorted columns, 0 = uint64, 1 = string, 2 = long string
   constexpr unsigned ALGORITHM_COUNT_TEST = 2;                                   // 0 = index sort, 1 = key cache sort
   constexpr unsigned REPEAT_COUNT_TEST = 5;                                      // number of times each thread repeats every sort

   /// create table with 7 mixed columns and random rows, same seed gives same data
   gd::table::table_column_buffer table_random_s(uint64_t uSeed)
   {
      using namespace gd::table;

      table_column_buffer tableRandom(0u, { { "uint64", 0, "FKey" },{ "string", 20, "FText" },{ "string", 200, "FLong" },{ "int32", 0, "FInteger" },
                                             { "double", 0, "FDouble" },{ "int16", 0, "FShort" },{ "uint32", 0, "FCount" } }, tag_prepare{});

      std::mt19937_64 mt19937Random(uSeed);
      for (unsigned uRow = 0; uRow < ROW_COUNT_TEST; uRow++)
      {
         std::string stringText = std::to_string(mt19937Random() % 1'000'000'000);
         std::string stringLong(100 + (mt19937Random() % 60), 'a');             // max 159 characters
         stringLong += std::to_string(mt19937Random());                         // max 20 digits, 179 in total and below column size 200
         tableRandom.row_add({ (uint64_t)mt19937Random(), std::string_view(stringText), std::string_view(stringLong),
                                (int32_t)mt19937Random(), (double)(mt19937Random() % 100000), (int16_t)mt19937Random(), (uint32_t)uRow }, tag_convert{});
      }

      return tableRandom;
   }

   /// check that rows are ordered in selected column
   bool is_sorted_s(const gd::table::table_column_buffer& table_, unsigned uColumn, bool bAscending)
   {
      for (uint64_t uRow = 1; uRow < table_.get_row_count(); uRow++)
      {
         auto variantviewPrevious = table_.cell_get_variant_view(uRow - 1, uColumn);
         auto variantviewCurrent = table_.cell_get_variant_view(uRow, uColumn);
         if (bAscending == true && variantviewCurrent < variantviewPrevious) return false;
         if (bAscending == false && variantviewPrevious < variantviewCurrent) return false;
      }
      return true;
   }

   /// sort with selected algorithm
   void sort_s(gd::table::table_column_buffer& table_, unsigned uColumn, bool bAscending, unsigned uAlgorithm)
   {
      using namespace gd::table;
      if (uAlgorithm == 0) table_.sort(uColumn, bAscending, 0, ROW_COUNT_TEST, tag_sort_index{});
      else                 table_.sort(uColumn, bAscending, 0, ROW_COUNT_TEST, tag_sort_key{});
   }

   /// name used for algorithm in result
   std::string_view name_s(unsigned uAlgorithm) { return uAlgorithm == 0 ? "index" : "key"; }

   /// index in expected vector for column and sort order
   unsigned expected_index_s(unsigned uColumn, bool bAscending) { return (uColumn * 2) + (bAscending ? 0u : 1u); }

} // namespace


TEST_CASE("[table] sort threads", "[.thread]") {
   using namespace gd::table;

   const unsigned uThreadCount = std::clamp(std::thread::hardware_concurrency(), 2u, 8u);
   const table_column_buffer tableRandom = table_random_s(12345);            // source data, only read by threads

   // ## create expected result single threaded, every thread compares against these
   //    index sort and key sort are both stable so the result is identical for equal keys
   std::vector<table_column_buffer> vectorExpected;
   for (unsigned uColumn = 0; uColumn < COLUMN_COUNT_TEST; uColumn++)
   {
      for (bool bAscending : { true, false })
      {
         table_column_buffer tableExpected(tableRandom);
         tableExpected.sort(uColumn, bAscending, 0, ROW_COUNT_TEST, tag_sort_index{});
         INFO("column " << uColumn << ", ascending " << bAscending);
         REQUIRE(is_sorted_s(tableExpected, uColumn, bAscending) == true);
         vectorExpected.push_back(std::move(tableExpected));
      }
   }

   // ## shared state, threads report to this
   std::mutex mutexResult;                                                     // protects mapResult
   std::map<std::string, std::vector<uint64_t>> mapResult;                     // name of run -> elapsed nanoseconds for each run in each thread
   std::atomic<unsigned> atomicFailureCount{ 0 };                              // number of runs that gave wrong result
   std::latch latchStart(uThreadCount);                                      // all threads start sorting at the same time

   auto sort_thread_ = [&](unsigned uThread)
      {
         [[maybe_unused]] auto uThread_d = uThread;
         latchStart.arrive_and_wait();

         for (unsigned uRepeat = 0; uRepeat < REPEAT_COUNT_TEST; uRepeat++)
         {
            for (unsigned uColumn = 0; uColumn < COLUMN_COUNT_TEST; uColumn++)
            {
               for (bool bAscending : { true, false })
               {
                  for (unsigned uAlgorithm = 0; uAlgorithm < ALGORITHM_COUNT_TEST; uAlgorithm++)
                  {
                     table_column_buffer tableSort(tableRandom);                // each run sorts its own unsorted copy

                     auto timeBegin = std::chrono::steady_clock::now();
                     sort_s(tableSort, uColumn, bAscending, uAlgorithm);
                     auto timeEnd = std::chrono::steady_clock::now();
                     auto uNanoseconds = (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(timeEnd - timeBegin).count();

                     // ### check result, do not use REQUIRE in thread, count failures instead
                     const auto& tableExpected = vectorExpected[expected_index_s(uColumn, bAscending)];
                     bool bOk = is_sorted_s(tableSort, uColumn, bAscending) && tableSort.equal(tableExpected, 0, ROW_COUNT_TEST);
                     if (bOk == false) atomicFailureCount++;

                     // ### report to shared result
                     std::string stringName(name_s(uAlgorithm));
                     stringName += bAscending ? " asc " : " desc ";
                     stringName += std::to_string(uColumn);

                     std::lock_guard<std::mutex> lockResult(mutexResult);
                     mapResult[stringName].push_back(uNanoseconds);
                  }
               }
            }
         }
      };

   // ## start threads and wait for them
   std::vector<std::thread> vectorThread;
   vectorThread.reserve(uThreadCount);
   for (unsigned uThread = 0; uThread < uThreadCount; uThread++) vectorThread.emplace_back(sort_thread_, uThread);
   for (auto& threadJoin : vectorThread) threadJoin.join();

   // ## verify shared result
   REQUIRE(atomicFailureCount.load() == 0);
   REQUIRE(mapResult.size() == (COLUMN_COUNT_TEST * 2 * ALGORITHM_COUNT_TEST));
   for (const auto& itResult : mapResult)
   {
      REQUIRE(itResult.second.size() == (size_t)uThreadCount * REPEAT_COUNT_TEST);
   }

   // ## print summary, nanoseconds are measured while all threads sort at the same time
   std::cout << "threads: " << uThreadCount << ", rows: " << ROW_COUNT_TEST << "\n";
   for (const auto& itResult : mapResult)
   {
      const auto& vectorNanosecond = itResult.second;
      uint64_t uMean = std::accumulate(vectorNanosecond.begin(), vectorNanosecond.end(), uint64_t(0)) / vectorNanosecond.size();
      uint64_t uMinimum = *std::min_element(vectorNanosecond.begin(), vectorNanosecond.end());
      std::cout << itResult.first << ": mean " << uMean << " ns, min " << uMinimum << " ns\n";
   }
}