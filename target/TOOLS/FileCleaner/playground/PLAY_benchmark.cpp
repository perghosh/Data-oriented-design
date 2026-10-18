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

TEST_CASE("[benchmark] string_transport", "[benchmark]")
{
   std::array<std::byte, 150> arrayBuffer;

   const auto resultTransport01 = gd::benchmark::run_s(
      [&arrayBuffer]()
      {
         int i;
         gd::benchmark::do_not_optimize_s(i);
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
         int i;
         gd::benchmark::do_not_optimize_s(i);
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



   std::cout << "Benchmark: string_transport using buffer\n";
   std::cout << "Mean: " << resultTransport01.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultTransport01.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultTransport01.m_dMaxTime << " ns\n";

   std::cout << "\nBenchmark: string_transport with no buffer\n";
   std::cout << "Mean: " << resultTransport02.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultTransport02.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultTransport02.m_dMaxTime << " ns\n";

   std::cout << "\nBenchmark: std::string\n";
   std::cout << "Mean: " << resultTransport03.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultTransport03.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultTransport03.m_dMaxTime << " ns\n";

}


TEST_CASE("[benchmark] string_transport02", "[benchmark]")
{
   const auto resultTransport03 = gd::benchmark::run_s(
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


   std::cout << "\nBenchmark: std::string\n";
   std::cout << "Mean: " << resultTransport03.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultTransport03.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultTransport03.m_dMaxTime << " ns\n";
}
