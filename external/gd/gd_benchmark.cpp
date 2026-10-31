// @FILE [tag: benchmark] [description: Benchmarking functions for GD library] [name: gd_benchmark.cpp] [type: source] 

#include "gd_benchmark.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__linux__)
#  include <unistd.h>
#endif
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__)
#  include <emmintrin.h>
#  define GD_BENCHMARK_HAS_CLFLUSH 1
#endif

_GD_BEGIN


argument::arguments benchmark::to_arguments_s(const result& result_)
{
   argument::arguments arguments_;
   arguments_.append("Iterations", result_.m_uIterations);
   arguments_.append("Mean (ns)", result_.m_dMean);
   arguments_.append("Min Time (ns)", result_.m_dMinTime);
   arguments_.append("Max Time (ns)", result_.m_dMaxTime);
   arguments_.append("Standard Deviation (ns)", result_.m_dStandardDeviation);
   arguments_.append("Total Duration (ns)", result_.m_dTotalDurationNs);
   return arguments_;
}


argument::arguments benchmark::to_arguments_print_s(const result& result_) 
{
   argument::arguments arguments_;
   arguments_.append("Iterations", result_.m_uIterations);
   arguments_.append("Mean (ns)", result_.m_dMean);
   arguments_.append("Min Time (ns)", result_.m_dMinTime);
   arguments_.append("Max Time (ns)", result_.m_dMaxTime);
   arguments_.append("Standard Deviation (ns)", result_.m_dStandardDeviation);
   arguments_.append("Total Duration (ns)", result_.m_dTotalDurationNs);
   return arguments_;
}

/** ---------------------------------------------------------------------------
 * @brief Converts a benchmark result into a human-readable string report.
 * @param result_ The benchmark result data to render, including iterations, mean, min/max time, standard deviation, and total duration (in nanoseconds).
 * @param stringMeassurment type of messurement (e.g., "ns", "us", "ms") to include in the report.
 * @return A human-readable report string with benchmark details and numeric metrics formatted as text.
 */
std::string benchmark::to_string_s(const result& result_, std::string_view stringMeassurment)
{
   double dDivisor = 0.0;
   if( stringMeassurment == "ns" ) { dDivisor = 1.0; }
   else if( stringMeassurment == "us" ) { dDivisor = 1000.0; }
   else if( stringMeassurment == "ms" ) { dDivisor = 1'000'000.0; }
   else if(stringMeassurment.empty() == false )
   {
      stringMeassurment = "";
      dDivisor = 1.0; // Default to nanoseconds when an unsupported measurement type is provided
   }
   else
   {
      dDivisor = 1.0; // Default to nanoseconds if no measurement type is provided
   }

   std::ostringstream streamReport;
   streamReport << std::fixed << std::setprecision(3);
   streamReport << "Iterations: " << result_.iterations() << "\n";
   streamReport << "Mean: " << (result_.mean() / dDivisor) << stringMeassurment << "\n";
   streamReport << "Min Time: " << (result_.min_time() / dDivisor) << stringMeassurment << "\n";
   streamReport << "Max Time: " << (result_.max_time() / dDivisor) << stringMeassurment << "\n";
   streamReport << "Standard Deviation: " << (result_.standard_deviation() / dDivisor) << stringMeassurment << "\n";
   streamReport << "Total Duration: " << (result_.total_duration() / dDivisor) << stringMeassurment;
   return streamReport.str();
}



std::size_t benchmark::flusher::cache_size_s(int iLevel)
{
   std::size_t uSize = 0;
#if defined(_WIN32)
   DWORD uLength = 0;
   ::GetLogicalProcessorInformation(nullptr, &uLength);
   std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> vectorInformation(uLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
   if( vectorInformation.empty() == false && ::GetLogicalProcessorInformation(vectorInformation.data(), &uLength) )
   {
      for( const auto& information_ : vectorInformation )
      {
         if( information_.Relationship == RelationCache && information_.Cache.Level == iLevel &&
             information_.Cache.Type != CacheInstruction )
         {
            uSize = (std::max)(uSize, static_cast<std::size_t>(information_.Cache.Size));
         }
      }
   }
#elif defined(__linux__) && defined(_SC_LEVEL1_DCACHE_SIZE)
   long iValue = 0;
   if( iLevel == 1 ) iValue = sysconf(_SC_LEVEL1_DCACHE_SIZE);
   else if( iLevel == 2 ) iValue = sysconf(_SC_LEVEL2_CACHE_SIZE);
   else if( iLevel == 3 ) iValue = sysconf(_SC_LEVEL3_CACHE_SIZE);
   if( iValue > 0 ) uSize = static_cast<std::size_t>(iValue);
#endif
   if( uSize == 0 )   // fallback when detection fails
   {
      constexpr std::size_t uFallback[] = { 0, 32 * 1024, 1024 * 1024, 32 * 1024 * 1024 };
      uSize = uFallback[(iLevel >= 1 && iLevel <= 3) ? iLevel : 0];
   }
   return uSize;
}

void benchmark::flusher::flush_range_s(const void* pBegin, std::size_t uSize)
{
#if defined(GD_BENCHMARK_HAS_CLFLUSH)
   constexpr std::size_t uLine = 64;
   const auto* pPosition = reinterpret_cast<const std::uint8_t*>(reinterpret_cast<std::uintptr_t>(pBegin) & ~(uLine - 1));
   const auto* pEnd = reinterpret_cast<const std::uint8_t*>(pBegin) + uSize;
   for( ; pPosition < pEnd; pPosition += uLine ) { _mm_clflush(pPosition); }
   _mm_mfence();
#else
   (void)pBegin; (void)uSize;   // no portable line flush, use the eviction walk instead
#endif
}

void benchmark::flusher::walk(std::size_t uBytes)
{
   if( uBytes > m_uBufferSize )
   {
      m_pBuffer = std::make_unique<std::uint8_t[]>(uBytes);   // zero-initialized, pages are touched
      m_uBufferSize = uBytes;
   }
   volatile std::uint8_t* pBuffer = m_pBuffer.get();
   for( std::size_t uPass = 0; uPass < m_settingsFlush.m_uPasses; ++uPass )
   {
      for( std::size_t u = 0; u < uBytes; u += 64 ) { pBuffer[u] = static_cast<std::uint8_t>(pBuffer[u] + 1); } // write = dirty lines get written back
   }
}

void benchmark::flusher::flush()
{
   const auto& settings_ = m_settingsFlush;
   if( settings_.m_eLevel != enumCacheLevel::none )
   {
      auto uSize = [&](int iLevel, std::size_t uOverride) { return uOverride != 0 ? uOverride : cache_size_s(iLevel); };
      std::size_t uCache = 0;
      switch( settings_.m_eLevel )
      {
      case enumCacheLevel::l1: uCache = uSize(1, settings_.m_uL1Size); break;
      case enumCacheLevel::l2: uCache = uSize(2, settings_.m_uL2Size); break;
      case enumCacheLevel::l3: uCache = uSize(3, settings_.m_uL3Size); break;
      default: break;
      }
      // 2x the cache size so replacement policy can't keep old lines, scaled by fraction for partial flush
      auto uBytes = static_cast<std::size_t>(static_cast<double>(uCache) * 2.0 * settings_.m_dFraction);
      walk(uBytes);
   }
   for( const auto& range_ : settings_.m_vectorRange ) { flush_range_s(range_.m_pBegin, range_.m_uSize); }
}

_GD_END
