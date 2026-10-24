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
