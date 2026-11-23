#include <algorithm>
#include <atomic>
#include <chrono>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iomanip>
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
#if defined( __clang__ )
   #define TEST_NOINLINE          __attribute__((noinline))
   #define TEST_MEMORY_BARRIER()  asm volatile( "" : : : "memory" )
   #define TEST_NO_VECTOR         _Pragma( "clang loop vectorize(disable) interleave(disable)" )
#elif defined( __GNUC__ )
   #define TEST_NOINLINE          __attribute__((noinline))
   #define TEST_MEMORY_BARRIER()  asm volatile( "" : : : "memory" )
   #define TEST_NO_VECTOR         _Pragma( "GCC novector" )                      // needs GCC 14 or newer
#elif defined( _MSC_VER )
   #include <intrin.h>
   #define TEST_NOINLINE          __declspec(noinline)
   #define TEST_MEMORY_BARRIER()  _ReadWriteBarrier()
   #define TEST_NO_VECTOR         __pragma( loop( no_vector ) )
#endif



// ---------------------------------------------------------------------------
// What does an "if" cost? Five loops that do the same work. Only the data differs.
//
// The data is a long list of numbers that are either 0 or 1. The loop asks "is it 0?" and does
// a small calculation on one of two paths. Scenario 1 has no if at all (the same calculation,
// always on the same path), scenarios 2 to 5 use the very same loop with different data:
//
//   1  no if at all
//   2  if, but every value is 1 (the if never says "zero")
//   3  5% zeros, 95% ones
//   4  25% zeros, 75% ones
//   5  50% zeros, 50% ones (random)
//
// Both paths contain a volatile store. That stops the compiler from replacing the if with a
// branch-free instruction (cmov), so we measure a real if.
// ---------------------------------------------------------------------------
namespace {

   /// No if: the same work as the loop below, always on the same path.
   TEST_NOINLINE uint64_t branch_none_s(const uint8_t* pbyteData, size_t uCount, volatile uint32_t* puSink)
   {
      uint64_t uAccumulator = 1;
      for(size_t u = 0; u < uCount; ++u)
      {
         const uint32_t uValue = pbyteData[u];
         const uint32_t uIndex = static_cast<uint32_t>(u);
         uAccumulator += (uIndex << 1) + uValue; puSink[0] = uIndex;
      }
      return uAccumulator;
   }

   /// With an if: is the value 0 or not? One of two small calculations is done.
   TEST_NOINLINE uint64_t branch_if_s(const uint8_t* pbyteData, size_t uCount, volatile uint32_t* puSink)
   {
      uint64_t uAccumulator = 1;
      for(size_t u = 0; u < uCount; ++u)
      {
         const uint32_t uValue = pbyteData[u];
         const uint32_t uIndex = static_cast<uint32_t>(u);
         if(uValue == 0) { uAccumulator += uIndex << 1; puSink[0] = uIndex; }
         else { uAccumulator ^= uIndex << 2; puSink[1] = uIndex; }
      }
      return uAccumulator;
   }

} // namespace

TEST_CASE("[benchmark] cost of an if, by how predictable it is", "[benchmark]")
{
   constexpr size_t uDataCount = 1'048'576;                                    // 1 MB of data per scenario, too long for the CPU to memorize
   constexpr unsigned uWarmupCount = 3;
   constexpr unsigned uRunIterations = 40;

   struct scenario
   {
      const char* m_pbszName;
      unsigned m_uZeroPercent;                                                 // how many percent of the values are 0
      bool m_bUseIf;
      std::vector<uint8_t> m_vectorData;
      double m_dNanosecond = 0;                                                // result: time per step
   };

   std::vector<scenario> vectorScenario;
   vectorScenario.push_back({ "1  no if at all",        0, false,{} });
   vectorScenario.push_back({ "2  if, always 1",        0, true,{} });
   vectorScenario.push_back({ "3  if,  5% zeros",       5, true,{} });
   vectorScenario.push_back({ "4  if, 25% zeros",      25, true,{} });
   vectorScenario.push_back({ "5  if, 50% zeros",      50, true,{} });

   // ## Fill every scenario with 0 and 1 in the wanted proportion
   std::mt19937 generatorRandom(12345);
   for(auto& scenario_ : vectorScenario)
   {
      scenario_.m_vectorData.resize(uDataCount);
      size_t uZero = 0;
      for(auto& byte_ : scenario_.m_vectorData)
      {
         const bool bZero = (generatorRandom() % 100) < scenario_.m_uZeroPercent;
         byte_ = bZero ? 0 : 1;
         uZero += bZero ? 1 : 0;
      }
      const double dZeroPercent = 100.0 * static_cast<double>(uZero) / static_cast<double>(uDataCount);
      INFO(scenario_.m_pbszName);
      CHECK(std::abs(dZeroPercent - scenario_.m_uZeroPercent) < 0.5);          // the data must have the proportion we say it has
   }

   volatile uint32_t uSink[4] = {};
   volatile uint64_t uSinkResult = 0;

   auto run_ = [&](const scenario& scenario_)
      {
         auto function_ = [&]()
            {
               TEST_MEMORY_BARRIER();
               const uint8_t* pbyteData = scenario_.m_vectorData.data();
               uSinkResult = scenario_.m_bUseIf ? branch_if_s(pbyteData, uDataCount, uSink) : branch_none_s(pbyteData, uDataCount, uSink);
            };
         return gd::benchmark::run_s(function_, uRunIterations, uWarmupCount);
      };

   CHECK(branch_if_s(vectorScenario[4].m_vectorData.data(), uDataCount, uSink) != 0);

   // ## Run 1,2,3,4,5 and then 5,4,3,2,1 so that a slowly changing clock speed affects all scenarios equally
   std::vector<double> vectorMedianSum(vectorScenario.size(), 0.0);
   for(size_t u = 0; u < vectorScenario.size(); ++u) { vectorMedianSum[u] += static_cast<double>(run_(vectorScenario[u]).median()); }
   for(size_t u = vectorScenario.size(); u-- > 0; ) { vectorMedianSum[u] += static_cast<double>(run_(vectorScenario[u]).median()); }
   for(size_t u = 0; u < vectorScenario.size(); ++u) { vectorScenario[u].m_dNanosecond = vectorMedianSum[u] / 2.0 / static_cast<double>(uDataCount); }

   // ## Print a table that is easy to read
   const double dNone = vectorScenario[0].m_dNanosecond;
   const double dNever = vectorScenario[1].m_dNanosecond;
   double dMax = 0;
   for(const auto& scenario_ : vectorScenario) { dMax = (std::max)(dMax, scenario_.m_dNanosecond); }

   std::cout << "\n==============================================================================\n"
      << " What does an \"if\" cost? Same work every time, " << uDataCount << " steps per run.\n"
      << " The only difference is how predictable the if is.\n"
      << "==============================================================================\n"
      << " scenario             zeros   wrong guesses   time per step   compared to\n"
      << "                              per 100 steps                   no if\n"
      << "------------------------------------------------------------------------------\n";

   for(size_t u = 0; u < vectorScenario.size(); ++u)
   {
      const auto& scenario_ = vectorScenario[u];
      const double dWrongPer100 = scenario_.m_bUseIf ? 100.0 * (std::min)(scenario_.m_uZeroPercent / 100.0, 1.0 - scenario_.m_uZeroPercent / 100.0) : 0.0;
      const size_t uBar = static_cast<size_t>(30.0 * scenario_.m_dNanosecond / dMax + 0.5);

      std::cout << " " << std::left << std::setw(21) << scenario_.m_pbszName << std::right
         << std::setw(4) << (scenario_.m_bUseIf ? std::to_string(scenario_.m_uZeroPercent) + "%" : std::string("-"))
         << std::setw(11) << std::fixed << std::setprecision(0) << dWrongPer100
         << std::setw(15) << std::setprecision(2) << scenario_.m_dNanosecond << " ns"
         << std::setw(10) << std::setprecision(1) << (scenario_.m_dNanosecond / dNone) << " x   "
         << std::string(uBar, '#') << "\n";
   }

   const double dWrongCost = (vectorScenario[4].m_dNanosecond - dNever) / 0.5;  // 50/50: half of the steps are guessed wrong
   std::cout << "------------------------------------------------------------------------------\n"
      << " How to read this:\n"
      << "  * The CPU guesses which way every if will go, and starts working before it knows.\n"
      << "  * A right guess costs nothing. A wrong guess throws the work away and starts over.\n"
      << "  * Scenario 2 has an if but the CPU always guesses right: compare it with scenario 1.\n"
      << "  * The more mixed the data, the more wrong guesses and the slower the loop.\n"
      << "  * The loop body is tiny on purpose. Real code does more work per step, so the same cost matters less.\n"
      << "  * One wrong guess costs roughly " << std::setprecision(1) << dWrongCost << " ns on this computer (from scenario 5).\n"
      << "==============================================================================\n";
}



TEST_CASE("[benchmark] int32 aligned vs misaligned", "[.benchmark]")
{
   constexpr size_t uValueCount = 16'777'216;                                 // 64 MB per buffer
   constexpr size_t uOffsetMisaligned = 1;
   constexpr unsigned uWarmupCount = 3;
   constexpr unsigned uRunIterations = 20;

   struct buffer_test
   {
      buffer_test(size_t uByteCount, size_t uOffset) : m_pbufferRaw(new uint8_t[uByteCount + 128])
      {
         uintptr_t uAddress = reinterpret_cast<uintptr_t>(m_pbufferRaw);
         uAddress = (uAddress + 63) & ~uintptr_t(63);
         m_pbufferData = reinterpret_cast<uint8_t*>(uAddress) + uOffset;
      }
      ~buffer_test() { delete[] m_pbufferRaw; }
      buffer_test(const buffer_test&) = delete;
      buffer_test& operator=(const buffer_test&) = delete;

      uint8_t* m_pbufferRaw;
      uint8_t* m_pbufferData;
   };

   buffer_test bufferAligned(uValueCount * sizeof(int32_t), 0);
   buffer_test bufferMisaligned(uValueCount * sizeof(int32_t), uOffsetMisaligned);

   std::mt19937 generatorRandom(12345);
   std::uniform_int_distribution<int32_t> distributionValue(-1'000'000, 1'000'000);
   for(size_t u = 0; u < uValueCount; ++u)
   {
      const int32_t iValue = distributionValue(generatorRandom);
      memcpy(bufferAligned.m_pbufferData + u * sizeof(int32_t), &iValue, sizeof(iValue));
   }
   memcpy(bufferMisaligned.m_pbufferData, bufferAligned.m_pbufferData, uValueCount * sizeof(int32_t));

   CHECK(reinterpret_cast<uintptr_t>(bufferAligned.m_pbufferData) % 4 == 0);
   CHECK(reinterpret_cast<uintptr_t>(bufferMisaligned.m_pbufferData) % 4 == uOffsetMisaligned);

   auto sum_ = [](const uint8_t* pbuffer, size_t uCount)
      {
         int64_t iSum = 0;
         TEST_NO_VECTOR
            for(size_t u = 0; u < uCount; ++u)
            {
               int32_t iValue;
               memcpy(&iValue, pbuffer + u * sizeof(int32_t), sizeof(iValue));
               iSum += iValue;
            }
         return iSum;
      };

   const int64_t iSumAligned = sum_(bufferAligned.m_pbufferData, uValueCount);
   const int64_t iSumMisaligned = sum_(bufferMisaligned.m_pbufferData, uValueCount);
   CHECK(iSumAligned == iSumMisaligned);

   volatile int64_t iSink = 0;
   auto functionAligned = [&]() { TEST_MEMORY_BARRIER(); iSink = sum_(bufferAligned.m_pbufferData, uValueCount); };
   auto functionMisaligned = [&]() { TEST_MEMORY_BARRIER(); iSink = sum_(bufferMisaligned.m_pbufferData, uValueCount); };

   const auto resultAligned1 = gd::benchmark::run_s(functionAligned, uRunIterations, uWarmupCount);
   const auto resultMisaligned1 = gd::benchmark::run_s(functionMisaligned, uRunIterations, uWarmupCount);
   const auto resultMisaligned2 = gd::benchmark::run_s(functionMisaligned, uRunIterations, uWarmupCount);
   const auto resultAligned2 = gd::benchmark::run_s(functionAligned, uRunIterations, uWarmupCount);

   auto print_result = [&](const char* pbszName, const auto& result)
      {
         const double dNanosecondPerValue = result.mean() / static_cast<double>(uValueCount);
         std::cout << pbszName << ": mean " << (result.mean() / 1'000.0) << " us per run, " << dNanosecondPerValue << " ns per value, " << (4.0 / dNanosecondPerValue) << " GB/s\n";
      };

   std::cout << "\n#######\nScenario: int32 aligned vs misaligned, values: " << uValueCount << ", sum: " << iSumAligned << "\n";
   print_result("aligned    (round 1)", resultAligned1);
   print_result("misaligned (round 1)", resultMisaligned1);
   print_result("misaligned (round 2)", resultMisaligned2);
   print_result("aligned    (round 2)", resultAligned2);
}

// ============================================================================
TEST_CASE("[benchmark] int32 aligned vs misaligned (L2)", "[.benchmark]")
{
   constexpr size_t uValueCount = 262'144;
   constexpr size_t uOffsetMisaligned = 1;
   constexpr unsigned uWarmupCount = 3;
   constexpr unsigned uRunIterations = 20'000; 

   struct buffer_test
   {
      buffer_test(size_t uByteCount, size_t uOffset) : m_pbufferRaw(new uint8_t[uByteCount + 128])
      {
         uintptr_t uAddress = reinterpret_cast<uintptr_t>(m_pbufferRaw);
         uAddress = (uAddress + 63) & ~uintptr_t(63);
         m_pbufferData = reinterpret_cast<uint8_t*>(uAddress) + uOffset;
      }
      ~buffer_test() { delete[] m_pbufferRaw; }
      buffer_test(const buffer_test&) = delete;
      buffer_test& operator=(const buffer_test&) = delete;

      uint8_t* m_pbufferRaw;
      uint8_t* m_pbufferData;
   };

   buffer_test bufferAligned(uValueCount * sizeof(int32_t), 0);
   buffer_test bufferMisaligned(uValueCount * sizeof(int32_t), uOffsetMisaligned);

   std::mt19937 generatorRandom(12345);
   std::uniform_int_distribution<int32_t> distributionValue(-1'000'000, 1'000'000);
   for(size_t u = 0; u < uValueCount; ++u)
   {
      const int32_t iValue = distributionValue(generatorRandom);
      memcpy(bufferAligned.m_pbufferData + u * sizeof(int32_t), &iValue, sizeof(iValue));
   }
   memcpy(bufferMisaligned.m_pbufferData, bufferAligned.m_pbufferData, uValueCount * sizeof(int32_t));

   CHECK(reinterpret_cast<uintptr_t>(bufferAligned.m_pbufferData) % 4 == 0);
   CHECK(reinterpret_cast<uintptr_t>(bufferMisaligned.m_pbufferData) % 4 == uOffsetMisaligned);

   auto sum_ = [](const uint8_t* pbuffer, size_t uCount)
      {
         int64_t iSum = 0;
         TEST_NO_VECTOR
            for(size_t u = 0; u < uCount; ++u)
            {
               int32_t iValue;
               memcpy(&iValue, pbuffer + u * sizeof(int32_t), sizeof(iValue));
               iSum += iValue;
            }
         return iSum;
      };

   const int64_t iSumAligned = sum_(bufferAligned.m_pbufferData, uValueCount);
   const int64_t iSumMisaligned = sum_(bufferMisaligned.m_pbufferData, uValueCount);
   CHECK(iSumAligned == iSumMisaligned);

   volatile int64_t iSink = 0;
   auto functionAligned = [&]() { TEST_MEMORY_BARRIER(); iSink = sum_(bufferAligned.m_pbufferData, uValueCount); };
   auto functionMisaligned = [&]() { TEST_MEMORY_BARRIER(); iSink = sum_(bufferMisaligned.m_pbufferData, uValueCount); };

   const auto resultAligned1 = gd::benchmark::run_s(functionAligned, uRunIterations, uWarmupCount);
   const auto resultMisaligned1 = gd::benchmark::run_s(functionMisaligned, uRunIterations, uWarmupCount);
   const auto resultMisaligned2 = gd::benchmark::run_s(functionMisaligned, uRunIterations, uWarmupCount);
   const auto resultAligned2 = gd::benchmark::run_s(functionAligned, uRunIterations, uWarmupCount);

   auto print_result = [&](const char* pbszName, const auto& result)
      {
         const double dNanosecondPerValue = result.mean() / static_cast<double>(uValueCount);
         std::cout << pbszName << ": mean " << (result.mean() / 1'000.0) << " us per run, " << dNanosecondPerValue << " ns per value, " << (4.0 / dNanosecondPerValue) << " GB/s\n";
      };

   std::cout << "\n#######\nScenario: int32 aligned vs misaligned (L2), values: " << uValueCount << ", sum: " << iSumAligned << "\n";
   print_result("aligned    (round 1)", resultAligned1);
   print_result("misaligned (round 1)", resultMisaligned1);
   print_result("misaligned (round 2)", resultMisaligned2);
   print_result("aligned    (round 2)", resultAligned2);
}



TEST_CASE("[benchmark] arguments find", "[.benchmark]")
{
   gd::argument::arguments argumentsCompact;          // compact, no alignment
   gd::argument::shared::arguments argumentsShared;   // shared, each value aligned to 4 bytes

   constexpr unsigned uRunIterations = 10000;
   constexpr unsigned uWarmupCount = 3;
   constexpr unsigned uValueCount = 50;               // try 5000 to see cache effects (lower uRunIterations)
   constexpr unsigned uRepeatCount = 200;             // repeats per sample for the single name scenarios

   // ## prepare names, build all strings first, then take views (SSO strings move when the vector grows)
   std::vector<std::string> vectorString;
   for(unsigned u = 0; u < uValueCount; ++u) { vectorString.push_back("key_" + std::to_string(u)); }
   std::vector<std::string_view> vectorName(vectorString.begin(), vectorString.end());
   const std::string_view stringMissing = "key_missing";

   // ## fill both with the same mix of types
   auto fill_ = [&](auto& arguments_)
      {
         const std::string_view stringValue = "some text value";
         for(unsigned u = 0; u < uValueCount; ++u)
         {
            switch(u % 3)
            {
            case 0:  arguments_.append(vectorName[u], static_cast<int>(u)); break;
            case 1:  arguments_.append(vectorName[u], u * 1.5); break;
            default: arguments_.append(vectorName[u], stringValue); break;
            }
         }
      };
   fill_(argumentsCompact);
   fill_(argumentsShared);

   // ## verify once, a benchmark of wrong results is worthless
   CHECK(argumentsCompact.size() == uValueCount);
   CHECK(argumentsShared.size() == uValueCount);
   for(unsigned u = 0; u < uValueCount; ++u)
   {
      CHECK(argumentsCompact.find(vectorName[u]) != nullptr);
      CHECK(argumentsShared.find(vectorName[u]) != nullptr);
      if(u % 3 == 0)
      {
         CHECK(argumentsCompact.get_argument(vectorName[u]).as_int() == static_cast<int>(u));
         CHECK(argumentsShared.get_argument(vectorName[u]).as_int() == static_cast<int>(u));
      }
   }
   CHECK(argumentsCompact.find(stringMissing) == nullptr);
   CHECK(argumentsShared.find(stringMissing) == nullptr);

   std::cout << "\nBuffer bytes, compact: " << argumentsCompact.buffer_size()
             << ", shared: " << argumentsShared.buffer_size() << "\n";

   // ## benchmark functions
   volatile size_t uSink = 0;                                                  // keeps the compiler from removing the find calls

   auto make_find_all_ = [&uSink, &vectorName](const auto& arguments_)
      {
         return [&uSink, &vectorName, &arguments_]()
            {
               size_t uSum = 0;
               for(const auto& stringName : vectorName) { uSum += reinterpret_cast<uintptr_t>(arguments_.find(stringName)); }
               uSink = uSum;
            };
      };

   auto make_find_repeat_ = [&uSink](const auto& arguments_, std::string_view stringName)
      {
         return [&uSink, &arguments_, stringName]()
            {
               size_t uSum = 0;
               for(unsigned u = 0; u < uRepeatCount; ++u) { uSum += reinterpret_cast<uintptr_t>(arguments_.find(stringName)); }
               uSink = uSum;
            };
      };

   auto print_result = [](const char* pbszName, const auto& result, unsigned uFindCount)
      {
         std::cout << pbszName << ": mean " << (result.mean() / 1'000.0) << " us per run, " << (result.mean() / uFindCount) << " ns per find\n";
      };

   // ## run A B B A to cancel order effects
   auto run_pair_ = [&](const char* pbszScenario, auto functionCompact, auto functionShared, unsigned uFindCount)
      {
         const auto resultCompact1 = gd::benchmark::run_s(functionCompact, uRunIterations, uWarmupCount);
         const auto resultShared1 = gd::benchmark::run_s(functionShared, uRunIterations, uWarmupCount);
         const auto resultShared2 = gd::benchmark::run_s(functionShared, uRunIterations, uWarmupCount);
         const auto resultCompact2 = gd::benchmark::run_s(functionCompact, uRunIterations, uWarmupCount);

         std::cout << "\n#######\nScenario: " << pbszScenario << "\n";
         print_result("compact (round 1)", resultCompact1, uFindCount);
         print_result("shared  (round 1)", resultShared1, uFindCount);
         print_result("shared  (round 2)", resultShared2, uFindCount);
         print_result("compact (round 2)", resultCompact2, uFindCount);
      };

   run_pair_("find all names", make_find_all_(argumentsCompact), make_find_all_(argumentsShared), uValueCount);
   run_pair_("find first name", make_find_repeat_(argumentsCompact, vectorName.front()), make_find_repeat_(argumentsShared, vectorName.front()), uRepeatCount);
   run_pair_("find last name", make_find_repeat_(argumentsCompact, vectorName.back()), make_find_repeat_(argumentsShared, vectorName.back()), uRepeatCount);
   run_pair_("find missing name (full scan)", make_find_repeat_(argumentsCompact, stringMissing), make_find_repeat_(argumentsShared, stringMissing), uRepeatCount);
}

// ============================================================================
TEST_CASE("[benchmark] arguments", "[.benchmark]")
{
   using namespace gd::argument::shared;

   constexpr unsigned uRunIterations = 10000;
   constexpr unsigned uWarmupCount = 3;

   alignas(64) std::array<std::byte, 1024> array_;

   {
      arguments argumentsCheck(array_);
      for (int i = 0; i < 20; ++i) { argumentsCheck.append(std::to_string(i), i); }
      const auto* pbyteData = static_cast<const std::byte*>(argumentsCheck.data());
      CHECK(pbyteData >= array_.data());
      CHECK(pbyteData < array_.data() + array_.size());
      CHECK(argumentsCheck.size() == 20);
   }
   

   gd::benchmark::flusher flusherCold(gd::benchmark::flush_settings::all_s());

   auto appendStack = [&]()
      {
         arguments argumentsStack(array_);
         for (int i = 0; i < 20; ++i)
         {
            std::string stringkey = std::to_string(i);
            argumentsStack.append(stringkey, i);
         }
         bool bFound = argumentsStack.find("19") != nullptr;
         return bFound;
      };

   auto appendHeap = [&]()
      {
         arguments argumentsHeap;
         for(int i = 0; i < 20; ++i)
         {
            std::string stringkey = std::to_string(i);
            argumentsHeap.append(stringkey, i);
         }
         bool bFound = argumentsHeap.find("19") != nullptr;
         return bFound;
      };

   const auto result01 = gd::benchmark::run_s( appendStack, uRunIterations, uWarmupCount);
   const auto result02 = gd::benchmark::run_s( appendHeap, uRunIterations, uWarmupCount);

   auto print_result = [](const char* pbszName, const auto& result)
      {
         std::cout << "\n\n#######\nBenchmark: " << pbszName << "\n";
         std::cout << "Mean: " << (result.mean() / 1'000.0) << " us\n";
         std::cout << "Minimum: " << (result.min_time() / 1'000.0) << " us\n";
         std::cout << "Maximum: " << (result.max_time() / 1'000.0) << " us\n";
         std::cout << "Total in milliseconds: " << (result.total_duration() / 1'000'000.0) << " ms\n";
      };


   print_result("sort table 01 (buffer)", result01);
   print_result("sort table 02 (heap)", result02);

}

// ============================================================================
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

// ============================================================================
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


// ============================================================================
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
