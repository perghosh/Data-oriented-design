// @FILE [tag: benchmark] [description: Benchmarking functions for GD library] [name: gd_benchmark.h] [type: header]

#pragma once

#include <chrono>
#include <concepts>
#include <print>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>      // std::addressof
#include <atomic>      // std::atomic_signal_fence
#if defined(_MSC_VER) && !defined(__clang__)
#  include <intrin.h>
#endif

#include "gd_arguments.h"

/**
 * @file gd_benchmark.h
 *
 * @brief Provides a benchmarking utility for measuring the performance of functions or code blocks.
 * 
 * Sections — double-click a marker, then Ctrl+F3 / F3 to jump. Repeated markers step through multiple locations.
 *
 *   result__                      - The result of a benchmark run, containing statistics about the execution time.
 *   run_s__                       - Runs a performance benchmark on a given function or lambda statically.
 *   s_pSink__                     - Address sink used by do_not_optimize_s on compilers without inline asm.
 *   do_not_optimize_s__           - Prevents the compiler from optimizing away a value.
 */


#ifndef GD_COMPILER_HAS_CPP26_SUPPORT
//#  error "This file is not being compiled in C++26 mode"
#endif

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gdwin {
#define _GD_END }
#endif

_GD_BEGIN



/** @CLASS [tag: benchmark, timing, performance] [summary: measures callable execution time and reports statistics]
 * @brief Measures execution time for a function or lambda over repeated calls.
 *
 * run_s() first executes untimed warm-up calls, then measures each requested
 * iteration using std::chrono::high_resolution_clock. It returns timing
 * statistics without retaining the result in a benchmark object.
 *
 * The callable must accept no arguments. Capture any required inputs in a
 * lambda. Both void-returning and value-returning callables are supported.
 * Returned values are passed to do_not_optimize_s(); they are not included
 * in the benchmark result.
 *
 * @par Iterations and warm-up
 * The default run performs 100 warm-up calls followed by 1000 measured calls.
 * Supply at least one measured iteration; zero iterations does not produce
 * meaningful minimum and maximum values.
 *
 * Warm-up calls invoke the same callable and can change captured state.
 * Exceptions from the callable propagate to the caller.
 *
 * @par Result fields
 * All duration values are expressed in nanoseconds:
 * - m_uIterations: number of measured calls, excluding warm-up calls.
 * - m_dMean: average measured execution time.
 * - m_dMinTime: fastest measured iteration.
 * - m_dMaxTime: slowest measured iteration.
 * - m_dStandardDeviation: sample standard deviation; zero for one iteration.
 * - m_dTotalDurationNs: elapsed time for the complete measurement phase,
 *   including timer and statistics overhead, but excluding warm-up.
 *
 * @par Reporting
 * run_s() is static and does not require a benchmark object.
 * A benchmark object optionally stores a name in m_stringName.
 * The name is not automatically attached to results or printed.
 *
 * to_arguments_s() and to_arguments_print_s() are instance methods that
 * convert a result into gd::argument::arguments. Both currently produce
 * the same descriptive keys, such as "Iterations" and "Mean (ns)".
 *
 * @par Measurement considerations
 * Use an optimized build and compare runs under similar conditions.
 * Very short operations may be dominated by timer overhead; consider doing
 * several operations inside each measured call.
 *
 * do_not_optimize_s() is a best-effort optimization barrier. Its effectiveness
 * depends on the compiler; the current MSVC implementation does not provide
 * a strong compiler barrier. Inspect optimized code for critical measurements.
 *
 * @par Example: measure copying and sorting a small collection
 * @code{.cpp}
 * #include <algorithm>
 * #include <iostream>
 * #include <vector>
 * #include "gd_benchmark.h"
 *
 * const std::vector<int> vectorInput{ 9, 3, 7, 1, 5 };
 *
 * const auto resultSort = gd::benchmark::run_s(
 *    [&vectorInput]()
 *    {
 *       auto vectorSorted = vectorInput;
 *       std::sort(vectorSorted.begin(), vectorSorted.end());
 *       return vectorSorted;
 *    },
 *    1000,   // Measured calls.
 *    100);   // Warm-up calls.
 *
 * // This measures copying, allocation, and sorting together.
 * std::cout << "Mean: " << resultSort.m_dMean << " ns\n";
 * std::cout << "Minimum: " << resultSort.m_dMinTime << " ns\n";
 * std::cout << "Maximum: " << resultSort.m_dMaxTime << " ns\n";
 * @endcode
 *
 * @par Example: convert a result into named arguments
 * @code{.cpp}
 * // Uses resultSort from the preceding example.
 * gd::benchmark benchmarkSort("Copy and sort");
 * auto argumentsResult = benchmarkSort.to_arguments_s(resultSort);
 *
 * std::cout << benchmarkSort.m_stringName << ": "
 *           << argumentsResult["Mean (ns)"].as_double() << " ns\n";
 * @endcode
 */
class benchmark
{
public:
   /// @API [tag: types] [description: The result of a benchmark run, containing statistics about the execution time.] [jump: result__]
   /// \brief The result of a benchmark run, containing statistics about the execution time.
   struct result {
      std::size_t iterations() const noexcept { return m_uIterations; }
      double mean() const noexcept { return m_dMean; }
      double min_time() const noexcept { return m_dMinTime; }
      double max_time() const noexcept { return m_dMaxTime; }
      double standard_deviation() const noexcept { return m_dStandardDeviation; }
      double total_duration_ns() const noexcept { return m_dTotalDurationNs; }

      std::size_t m_uIterations{ 0 };     ///< The number of iterations executed in the benchmark run.
      double m_dMean{ 0.0 };              ///< The mean execution time in nanoseconds.
      double m_dMinTime{ 0.0 };           ///< The minimum execution time in nanoseconds.
      double m_dMaxTime{ 0.0 };           ///< The maximum execution time in nanoseconds.
      double m_dStandardDeviation{ 0.0 }; ///< The standard deviation of the execution time in nanoseconds.
      double m_dTotalDurationNs{ 0.0 };   ///< The total duration of the benchmark run in nanoseconds.
   };

   // @API [tag: construction]
public:
   benchmark() {}
   benchmark(const std::string& stringName) : m_stringName(stringName) {}
   // copy
   benchmark(const benchmark& o) { common_construct(o); }
   benchmark(benchmark&& o) noexcept { common_construct(std::move(o)); }
   // assign
   benchmark& operator=(const benchmark& o) { common_construct(o); return *this; }
   benchmark& operator=(benchmark&& o) noexcept { common_construct(std::move(o)); return *this; }

   ~benchmark() {}
private:
   // common copy
   void common_construct(const benchmark& o) { 
      m_stringName = o.m_stringName; 
   }
   void common_construct(benchmark&& o) noexcept {
      m_stringName = std::move(o.m_stringName);
   }

   // @API [tag: operator]
public:


// ## methods ------------------------------------------------------------------
public:
// @API [tag: get, set]

// @API [tag: operation]


protected:
// @API [tag: internal]

public:
// @API [tag: debug]

// ## attributes ----------------------------------------------------------------
public:
   std::string m_stringName; ///< The name of the benchmark, used for identification and reporting.


// @API [tag: free-functions]
public:

   /// @API [tag: run] [description: Runs a performance benchmark on a given function or lambda statically.] [jump: run_s__]
   /// \brief Runs a performance benchmark on a given function or lambda statically.
   /// \param function_ Callable that takes no arguments.
   /// \param uIterations Number of measured samples.
   /// \param uWarmups Number of untimed warm-up calls.
   /// \param uBatch Number of calls timed together in one sample. Reported times are per call.
   template <typename FUNCTION>
      requires std::invocable<FUNCTION>
   static result run_s(FUNCTION&& function_, std::size_t uIterations = 1000, std::size_t uWarmups = 100, std::size_t uBatch = 1) {
      constexpr bool bIsVoid = requires { { function_() } -> std::same_as<void>; };
      if (uBatch == 0) uBatch = 1;

      // ## Warm-up phase
      for (std::size_t uI = 0; uI < uWarmups; ++uI) {
         if constexpr (bIsVoid) { function_(); }
         else { do_not_optimize_s(function_()); }
      }

      // ## Measurement phase & Statistics (Welford's algorithm)
      double dMinTime = std::numeric_limits<double>::max();    // Initialize to maximum possible value
      double dMaxTime = std::numeric_limits<double>::lowest(); // Initialize to minimum possible value
      double dMean = 0.0;                                      // Mean execution time
      double dM2 = 0.0;                                        // Sum of squares of differences from the current mean

      auto total_start_ = std::chrono::steady_clock::now();    // Start total duration timer

      for (std::size_t u = 1; u <= uIterations; ++u) {
         auto start_ = std::chrono::steady_clock::now();       // Start sample timer

         for (std::size_t uCall = 0; uCall < uBatch; ++uCall) {
            if constexpr (bIsVoid) { function_(); }            // Void: just call it
            else { do_not_optimize_s(function_()); }           // Value: prevent optimization
         }

         auto end_ = std::chrono::steady_clock::now();         // End sample timer
         std::chrono::duration<double, std::nano> elapsed_ = end_ - start_;
         double dDuration = elapsed_.count() / uBatch;         // Duration per call in nanoseconds

         if (dDuration < dMinTime) dMinTime = dDuration;        // Update minimum time if current duration is less
         if (dDuration > dMaxTime) dMaxTime = dDuration;        // Update maximum time if current duration is more

         double dDelta = dDuration - dMean;
         dMean += dDelta / u;
         dM2 += dDelta * (dDuration - dMean);
      }

      auto total_end_ = std::chrono::steady_clock::now();      // End total duration timer
      std::chrono::duration<double, std::nano> total_elapsed_ = total_end_ - total_start_;

      double dVariance = (uIterations > 1) ? (dM2 / (uIterations - 1)) : 0.0;
      double dStdDev = std::sqrt(dVariance);

      return result{
         .m_uIterations = uIterations,
         .m_dMean = dMean,
         .m_dMinTime = dMinTime,
         .m_dMaxTime = dMaxTime,
         .m_dStandardDeviation = dStdDev,
         .m_dTotalDurationNs = total_elapsed_.count()
      };
   }

   /// Converts a benchmark result into an argument::arguments object for easier access and manipulation of the result data. 
   argument::arguments to_arguments_s(const result& result_);

   /// Converts a benchmark result into an argument::arguments object with more descriptive keys for easier access and manipulation of the result data.
   argument::arguments to_arguments_print_s(const result& result_);


   // inside the class (or at namespace scope)
   inline static const volatile void* volatile s_pSink = nullptr;

   template <typename TYPE>
   static void do_not_optimize_s(TYPE && value_) {
#if defined(__GNUC__) || defined(__clang__)
      __asm__ volatile("" : : "g"(std::addressof(value_)) : "memory");
#else
      s_pSink = std::addressof(value_);                    // address escapes through a volatile store
      std::atomic_signal_fence(std::memory_order_seq_cst); // compiler-only barrier, no CPU fence
#endif
   }

};

_GD_END