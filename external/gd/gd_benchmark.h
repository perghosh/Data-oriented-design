// @FILE [tag: benchmark] [description: Benchmarking functions for GD library] [name: gd_benchmark.h] [type: header]

#pragma once

#include <chrono>
#include <concepts>
#include <print>
#include <vector>
#include <numeric>
#include <algorithm>

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
 */


#ifndef GD_COMPILER_HAS_CPP26_SUPPORT
#  error "This file is not being compiled in C++26 mode"
#endif

#ifndef _GD_BEGIN
#define _GD_BEGIN namespace gdwin {
#define _GD_END }
#endif

_GD_BEGIN



/** @CLASS [name: benchmark] [description:  ]
 * \brief
 *
 *
 *
 \code
 \endcode
 */
class benchmark
{
public:
   /// @API [tag: types] [description: The result of a benchmark run, containing statistics about the execution time.] [jump: result__]
   /// \brief The result of a benchmark run, containing statistics about the execution time.
   struct result {
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
   template <typename FUNCTION>
      requires std::invocable<FUNCTION>
   static result run_s(FUNCTION&& function_, std::size_t uIterations = 1000, std::size_t uWarmups = 100) {
      // ## Warm-up phase
      for (std::size_t uI = 0; uI < uWarmups; ++uI) {
         if constexpr (requires { { function_() } -> std::same_as<void>; }) { function_(); }
         else { do_not_optimize_s(function_()); }
      }

      // ## Measurement phase & Statistics (Welford's algorithm)
      double dMinTime = std::numeric_limits<double>::max(); // Initialize to maximum possible value
      double dMaxTime = std::numeric_limits<double>::lowest(); // Initialize to minimum possible value
      double dMean = 0.0; // Mean execution time
      double dM2 = 0.0; // Sum of squares of differences from the current mean

      auto total_start_ = std::chrono::high_resolution_clock::now(); // Start total duration timer

      for(std::size_t u = 1; u <= uIterations; ++u) {
         auto start_ = std::chrono::high_resolution_clock::now(); // Start individual iteration timer

         if constexpr (requires { { function_() } -> std::same_as<void>; }) {  // If the function returns void, just call it
            function_();
         }
         else { do_not_optimize_s(function_()); }                              // If the function returns a value, prevent optimization

         auto end_ = std::chrono::high_resolution_clock::now(); // End individual iteration timer
         std::chrono::duration<double, std::nano> elapsed_ = end_ - start_;
         double dDuration = elapsed_.count(); // Duration in nanoseconds

         if(dDuration < dMinTime) dMinTime = dDuration;                        // Update minimum time if current duration is less
         if(dDuration > dMaxTime) dMaxTime = dDuration;                        // Update maximum time if current duration is more

         double dDelta = dDuration - dMean;
         dMean += dDelta / u;
         dM2 += dDelta * (dDuration - dMean);
      }

      auto total_end_ = std::chrono::high_resolution_clock::now();  // End total duration timer
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

   /// \brief Prevents the compiler from optimizing away a value.
   template <typename TYPE>
   static void do_not_optimize_s(TYPE&& value_) {
      if constexpr (!std::is_void_v<TYPE>) {
         volatile auto& ref = value_;
         (void)ref;
      }
#if defined(__GNUC__) || defined(__clang__)
      __asm__ volatile("" : : "g"(&value_) : "memory");
#endif
   }

};

_GD_END