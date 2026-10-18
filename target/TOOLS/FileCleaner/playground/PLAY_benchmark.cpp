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

/*TEST_CASE("[benchmark] string_transport", "[benchmark]")
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

}   */


TEST_CASE("[benchmark] string_transport02", "[benchmark]")
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
   std::cout << "Mean: " << resultArgument.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultArgument.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultArgument.m_dMaxTime << " ns\n";
   
   std::cout << "\nBenchmark: gd::argument::arguments with buffer\n";
   std::cout << "Mean: " << resultArgument2.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultArgument2.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultArgument2.m_dMaxTime << " ns\n";

   std::cout << "\nBenchmark: assign with []\n";
   std::cout << "Mean: " << resultArgument3.m_dMean << " ns\n";
   std::cout << "Minimum: " << resultArgument3.m_dMinTime << " ns\n";
   std::cout << "Maximum: " << resultArgument3.m_dMaxTime << " ns\n";
}
