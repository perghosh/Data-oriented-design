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
 * ----------------------------------------------------------------------------
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
 * ----------------------------------------------------------------------------
 * @par Example: convert a result into named arguments
 * @code{.cpp}
 * // Uses resultSort from the preceding example.
 * gd::benchmark benchmarkSort("Copy and sort");
 * auto argumentsResult = benchmarkSort.to_arguments_s(resultSort);
 *
 * std::cout << benchmarkSort.m_stringName << ": "
 *           << argumentsResult["Mean (ns)"].as_double() << " ns\n";
 * @endcode
 * 
 * ----------------------------------------------------------------------------
 * @par Example: measure a cold run after flushing the cache
 * @code{.cpp}
 * #include <algorithm>
 * #include <iostream>
 * #include <vector>
 * #include "gd_benchmark.h"
 *
 * const std::vector<int> vectorInput( 100'000, 0 );   // Large enough to be evicted from L1/L2.
 *
 * // Flush up to and including L2 before every sample. The flush is not measured.
 * gd::benchmark::flusher flusherCold( gd::benchmark::flush_settings::l2_s() );
 *
 * const auto resultCold = gd::benchmark::run_cold_s(
 *    [&vectorInput]()
 *    {
 *       auto vectorSorted = vectorInput;
 *       std::sort(vectorSorted.begin(), vectorSorted.end());
 *       return vectorSorted;
 *    },
 *    flusherCold,
 *    200,    // Measured calls, each one preceded by a flush.
 *    2);     // Warm-up calls, flushed too. Removes first-call costs such as lazy init.
 *
 * // Cold timings are noisy, compare the mean and maximum rather than the minimum.
 * std::cout << "Mean: " << resultCold.mean() << " ns\n";
 * std::cout << "Maximum: " << resultCold.max_time() << " ns\n";
 * @endcode
 *
 * ----------------------------------------------------------------------------
 * @par Example: flush only the data under test, or part of a cache level
 * @code{.cpp}
 * // Evict just the input buffer with clflush (x86 only). Everything else stays warm.
 * flusherCold.settings() = gd::benchmark::flush_settings::range_s(
 *    vectorInput.data(), vectorInput.size() * sizeof(int) );
 * const auto resultRange = gd::benchmark::run_cold_s( [&vectorInput]() { return vectorInput.size(); }, flusherCold, 200 );
 *
 * // Walk a quarter of L3, and flush the input buffer as well.
 * flusherCold.settings() = gd::benchmark::flush_settings::partial_s( gd::benchmark::enumCacheLevel::l3, 0.25 );
 * flusherCold.settings().m_vectorRange.push_back( { vectorInput.data(), vectorInput.size() * sizeof(int) } );
 * const auto resultPartial = gd::benchmark::run_cold_s( [&vectorInput]() { return vectorInput.size(); }, flusherCold, 200 );
 * @endcode
 *
 * @note The copy in the first example is measured together with the sort, as in the warm sample.
 *       Each sample is a single call, so the same flusher can be reused with different settings.
 *       Pin the thread to one core so the flush and the measured call hit
 *
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
      double total_duration() const noexcept { return m_dTotalDurationNs; }

      std::size_t m_uIterations{ 0 };     ///< The number of iterations executed in the benchmark run.
      double m_dMean{ 0.0 };              ///< The mean execution time in nanoseconds.
      double m_dMinTime{ 0.0 };           ///< The minimum execution time in nanoseconds.
      double m_dMaxTime{ 0.0 };           ///< The maximum execution time in nanoseconds.
      double m_dStandardDeviation{ 0.0 }; ///< The standard deviation of the execution time in nanoseconds.
      double m_dTotalDurationNs{ 0.0 };   ///< The total duration of the benchmark run in nanoseconds.
   };

public:
   /// Highest cache level that is evicted by the walk. Evicting level N also clears the levels below it.
   enum class enumCacheLevel { none, l1, l2, l3 };

   /// Memory region that is flushed line by line with clflush (x86 only).
   struct flush_range
   {
      const void* m_pBegin = nullptr;
      std::size_t m_uSize = 0;
   };

   struct flush_settings
   {
      enumCacheLevel m_eLevel = enumCacheLevel::none; ///< Evict by walking a buffer up to this level
      double m_dFraction = 1.0;                       ///< 0..1, how much of the level to walk (partial flush)
      std::size_t m_uPasses = 2;                      ///< Passes over the buffer, 2 is more reliable than 1
      std::size_t m_uL1Size = 0;                      ///< 0 = detect
      std::size_t m_uL2Size = 0;                      ///< 0 = detect
      std::size_t m_uL3Size = 0;                      ///< 0 = detect
      std::vector<flush_range> m_vectorRange;         ///< Specific regions to clflush

      static flush_settings all_s() { return { .m_eLevel = enumCacheLevel::l3 }; } ///< Evict all cache levels, including L3 if present
      static flush_settings l1_s() { return { .m_eLevel = enumCacheLevel::l1 }; } ///< Evict L1 cache only
      static flush_settings l2_s() { return { .m_eLevel = enumCacheLevel::l2 }; } ///< Evict L2 cache only
      static flush_settings l3_s() { return { .m_eLevel = enumCacheLevel::l3 }; } ///< Evict L3 cache only
      /// @brief Create flush settings for a partial cache eviction.
      static flush_settings partial_s(enumCacheLevel eLevel, double dFraction) { return { .m_eLevel = eLevel, .m_dFraction = dFraction }; }
      /// @brief Create flush settings for a specific memory range to be flushed.
      static flush_settings range_s(const void* pBegin, std::size_t uSize) { return { .m_vectorRange = { { pBegin, uSize } } }; }
   };

   /// Performs the flush. Create it once and reuse it, the buffer is allocated on first use.
   class flusher
   {
   public:
      flusher() = default;
      explicit flusher(const flush_settings& settingsFlush) : m_settingsFlush(settingsFlush) {}

      flush_settings& settings() { return m_settingsFlush; }
      void flush();                                  ///< Evict by walk, then clflush ranges
      static void flush_range_s(const void* pBegin, std::size_t uSize);
      static std::size_t cache_size_s(int iLevel);   ///< Detected size in bytes for level 1, 2 or 3

   private:
      void walk(std::size_t uBytes);

      flush_settings m_settingsFlush;
      std::unique_ptr<std::uint8_t[]> m_pBuffer;
      std::size_t m_uBufferSize = 0;
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
   static result run_s(FUNCTION&& function_, std::size_t uIterations = 1000, std::size_t uWarmups = 100) {
      constexpr bool bIsVoid = requires { { function_() } -> std::same_as<void>; };

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

         if constexpr (bIsVoid) { function_(); }               // Void: just call it
         else { do_not_optimize_s(function_()); }              // Value: prevent optimization

         auto end_ = std::chrono::steady_clock::now();         // End sample timer
         std::chrono::duration<double, std::nano> elapsed_ = end_ - start_;
         double dDuration = elapsed_.count();                  // Duration per call in nanoseconds

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

   /// Like run_s, but flushes caches before every sample. The flush is not measured.
   /// Each sample is a single call, because batching would make every call but the first warm.
   template <typename FUNCTION>
      requires std::invocable<FUNCTION>
   static result run_cold_s(FUNCTION&& function_, flusher& flusher_, std::size_t uIterations = 100, std::size_t uWarmups = 0) {
      constexpr bool bIsVoid = requires { { function_() } -> std::same_as<void>; }; // Determine if the callable returns void

      // ### Warm-up phase
      for(std::size_t uI = 0; uI < uWarmups; ++uI)                            // optional: primes code pages, lazy init, etc.
      {
         flusher_.flush();
         if constexpr (bIsVoid) { function_(); }
         else { do_not_optimize_s(function_()); }
      }

      double dMinTime = std::numeric_limits<double>::max(); // Initialize to min to maximum possible value
      double dMaxTime = std::numeric_limits<double>::lowest(); // Initialize max to minimum possible value
      double dMean = 0.0;  // Mean execution time
      double dM2 = 0.0;    // Sum of squares of differences from the current mean
      double dTotal = 0.0; // Total duration of all measured calls

      // ### Measurement phase & Statistics (Welford's algorithm)
      for(std::size_t u = 1; u <= uIterations; ++u) {
         flusher_.flush();                                       // not measured
         std::atomic_thread_fence(std::memory_order_seq_cst);

         auto start_ = std::chrono::steady_clock::now();
         if constexpr (bIsVoid) { function_(); }
         else { do_not_optimize_s(function_()); }
         auto end_ = std::chrono::steady_clock::now();

         double dDuration = std::chrono::duration<double, std::nano>(end_ - start_).count();
         dTotal += dDuration;                                    // sum of measured time only, flush excluded
         if (dDuration < dMinTime) dMinTime = dDuration;
         if (dDuration > dMaxTime) dMaxTime = dDuration;

         double dDelta = dDuration - dMean;
         dMean += dDelta / u;
         dM2 += dDelta * (dDuration - dMean);
      }

      double dVariance = (uIterations > 1) ? (dM2 / (uIterations - 1)) : 0.0; // sample variance
      return result{
         .m_uIterations = uIterations, 
         .m_dMean = dMean, 
         .m_dMinTime = dMinTime,
         .m_dMaxTime = dMaxTime, 
         .m_dStandardDeviation = std::sqrt(dVariance), 
         .m_dTotalDurationNs = dTotal
      };
   }

   /// Converts a benchmark result into an argument::arguments object for easier access and manipulation of the result data. 
   argument::arguments to_arguments_s(const result& result_);

   /// Converts a benchmark result into an argument::arguments object with more descriptive keys for easier access and manipulation of the result data.
   argument::arguments to_arguments_print_s(const result& result_);

   /// Convert to string for printing or logging.
   std::string to_string_s(const result& result_, std::string_view stringMeassurment );


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
