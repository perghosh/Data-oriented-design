// @FILE [tag: benchmark] [description: Benchmarking functions for GD library] [name: gd_benchmark.cpp] [type: source] 

#include "gd_benchmark.h"

#include <format>
#include <stdexcept>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#elif defined(__APPLE__)
#  include <sys/types.h>
#  include <sys/sysctl.h>
#elif defined(__linux__)
#  include <unistd.h>
#endif
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_IX86) || defined(__i386__)
#  include <emmintrin.h>
#  define GD_BENCHMARK_HAS_CLFLUSH 1
#elif defined(__aarch64__) && defined(__linux__)
#  define GD_BENCHMARK_HAS_DC_CIVAC 1
#endif

#if defined(_WIN32)
namespace {
   /// Returns a vector of SYSTEM_LOGICAL_PROCESSOR_INFORMATION structures containing cache information for the system.
   std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> cache_information_s()
   {
      DWORD uLength = 0;
      ::GetLogicalProcessorInformation(nullptr, &uLength);
      std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> vectorInformation(uLength / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
      if( vectorInformation.empty() == false && ::GetLogicalProcessorInformation(vectorInformation.data(), &uLength) == FALSE ) { vectorInformation.clear(); }
      return vectorInformation;
   }
}
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

   auto format_value_ = [](double dValue) -> std::string
   {
      std::string stringValue = std::format("{:.2f}", dValue);
      const auto uDot = stringValue.find('.');
      if( uDot != std::string::npos )
      {
         while( stringValue.empty() == false && stringValue.back() == '0' )
         {
            stringValue.pop_back();
         }

         if( stringValue.empty() == false && stringValue.back() == '.' )
         {
            stringValue.pop_back();
         }
      }

      return stringValue;
   };

   return std::format(
      "Iterations: {}\n"
      "Mean: {} {}\n"
      "Min Time: {} {}\n"
      "Max Time: {} {}\n"
      "Standard Deviation: {} {}\n"
      "Total Duration: {} {}\n",
      result_.iterations(),
      format_value_(result_.mean() / dDivisor), stringMeassurment,
      format_value_(result_.min_time() / dDivisor), stringMeassurment,
      format_value_(result_.max_time() / dDivisor), stringMeassurment,
      format_value_(result_.standard_deviation() / dDivisor), stringMeassurment,
      format_value_(result_.total_duration() / dDivisor), stringMeassurment);
}



std::size_t benchmark::flusher::cache_size_s(int iLevel)
{
   std::size_t uSize = 0;
#if defined(_WIN32)
   for( const auto& information_ : cache_information_s() )
   {
      if( information_.Relationship == RelationCache && information_.Cache.Level == iLevel &&
          information_.Cache.Type != CacheInstruction )
      {
         uSize = (std::max)(uSize, static_cast<std::size_t>(information_.Cache.Size));
      }
   }
#elif defined(__APPLE__)
   // Apple Silicon: hw.perflevel0.l2cachesize gives the performance cores, hw.l2cachesize may report the efficiency cores
   const char* pbszName = iLevel == 1 ? "hw.l1dcachesize" : iLevel == 2 ? "hw.l2cachesize" : "hw.l3cachesize";
   std::uint64_t uValue = 0;
   std::size_t uLength = sizeof(uValue);
   if( ::sysctlbyname(pbszName, &uValue, &uLength, nullptr, 0) == 0 ) { uSize = static_cast<std::size_t>(uValue); }
#elif defined(__linux__) && defined(_SC_LEVEL1_DCACHE_SIZE)
   long iValue = 0;
   if( iLevel == 1 ) iValue = sysconf(_SC_LEVEL1_DCACHE_SIZE);
   else if( iLevel == 2 ) iValue = sysconf(_SC_LEVEL2_CACHE_SIZE);
   else if( iLevel == 3 ) iValue = sysconf(_SC_LEVEL3_CACHE_SIZE);
   if( iValue > 0 ) uSize = static_cast<std::size_t>(iValue);
#endif
   if( uSize == 0 )   // fallback when detection fails (also used for a missing L3, which over-evicts but is safe)
   {
      constexpr std::size_t uFallback[] = { 0, 32 * 1024, 1024 * 1024, 32 * 1024 * 1024 };
      uSize = uFallback[(iLevel >= 1 && iLevel <= 3) ? iLevel : 0];
   }
   return uSize;
}

std::size_t benchmark::flusher::line_size_s()
{
   std::size_t uSize = 0;
#if defined(_WIN32)
   for( const auto& information_ : cache_information_s() )
   {
      if( information_.Relationship == RelationCache && information_.Cache.Level == 1 ) { uSize = information_.Cache.LineSize; break; }
   }
#elif defined(__APPLE__)
   std::uint64_t uValue = 0;
   std::size_t uLength = sizeof(uValue);
   if( ::sysctlbyname("hw.cachelinesize", &uValue, &uLength, nullptr, 0) == 0 ) { uSize = static_cast<std::size_t>(uValue); }
#elif defined(__linux__) && defined(_SC_LEVEL1_DCACHE_LINESIZE)
   const long iValue = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
   if( iValue > 0 ) uSize = static_cast<std::size_t>(iValue);
#endif
   if( uSize == 0 || (uSize & (uSize - 1)) != 0 ) { uSize = 64; }   // unknown or not a power of two
   return uSize;
}

bool benchmark::flusher::flush_range_s(const void* pBegin, std::size_t uSize)
{
#if defined(GD_BENCHMARK_HAS_CLFLUSH)
   constexpr std::size_t uLine = 64;
   const auto* pPosition = reinterpret_cast<const std::uint8_t*>(reinterpret_cast<std::uintptr_t>(pBegin) & ~(uLine - 1));
   const auto* pEnd = reinterpret_cast<const std::uint8_t*>(pBegin) + uSize;
   for( ; pPosition < pEnd; pPosition += uLine ) { _mm_clflush(pPosition); }
   _mm_mfence();
   return true;
#elif defined(GD_BENCHMARK_HAS_DC_CIVAC)
   // Clean and invalidate to the point of coherency. Linux allows this at EL0, other OSes may trap, so it is limited to Linux.
   // Stepping by 64 is safe for 64 and 128 byte lines.
   constexpr std::size_t uLine = 64;
   const auto* pPosition = reinterpret_cast<const std::uint8_t*>(reinterpret_cast<std::uintptr_t>(pBegin) & ~(uLine - 1));
   const auto* pEnd = reinterpret_cast<const std::uint8_t*>(pBegin) + uSize;
   for( ; pPosition < pEnd; pPosition += uLine ) { __asm__ volatile("dc civac, %0" : : "r"(pPosition) : "memory"); }
   __asm__ volatile("dsb ish" : : : "memory");
   return true;
#else
   (void)pBegin; (void)uSize;
   return false;
#endif
}

std::size_t benchmark::flusher::walk_bytes(int iLevel, double dFraction)
{
   if( m_uLineSize == 0 ) { m_uLineSize = line_size_s(); }

   const std::size_t arrayOverride[] = { 0, m_settingsFlush.m_uL1Size, m_settingsFlush.m_uL2Size, m_settingsFlush.m_uL3Size };
   std::size_t uTotal = 0;
   for( int i = 1; i <= iLevel && i <= 3; ++i )   // sum of all levels: correct for inclusive, exclusive and non-inclusive designs
   {
      std::size_t uSize = arrayOverride[i];
      if( uSize == 0 )
      {
         if( m_arrayCacheSize[i] == 0 ) { m_arrayCacheSize[i] = cache_size_s(i); }   // detect once
         uSize = m_arrayCacheSize[i];
      }
      uTotal += uSize;
   }
   return static_cast<std::size_t>(static_cast<double>(uTotal) * m_settingsFlush.m_dOvershoot * dFraction);
}

void benchmark::flusher::walk(std::size_t uBytes)
{
   const std::size_t uLine = m_uLineSize;
   const std::size_t uLineCount = uBytes / uLine;
   if( uLineCount == 0 ) { return; }

   if( uBytes + uLine > m_uBufferSize )
   {
      m_pBuffer = std::make_unique<std::uint8_t[]>(uBytes + uLine);   // zero-initialized, so the pages are really mapped
      m_uBufferSize = uBytes + uLine;
   }
   const auto uAddress = reinterpret_cast<std::uintptr_t>(m_pBuffer.get());
   auto* pBuffer = reinterpret_cast<volatile std::uint8_t*>((uAddress + uLine - 1) & ~(uLine - 1));   // line aligned

   // Weyl sequence: a stride that is coprime with the line count visits every line exactly once per pass,
   // in a scattered order that a stream prefetcher can't follow. Addresses don't depend on loaded data,
   // so the CPU can still overlap the misses.
   std::size_t uStride = 1;
   if( m_settingsFlush.m_bScatter && uLineCount > 2 )
   {
      uStride = static_cast<std::size_t>(static_cast<double>(uLineCount) * 0.6180339887) | 1;
      while( std::gcd(uStride, uLineCount) != 1 ) { uStride += 2; }
      if( uStride >= uLineCount ) { uStride = 1; }
   }

   const bool bWrite = m_settingsFlush.m_eMode == enumFlushMode::write;
   std::uint64_t uChecksum = 0;
   std::size_t uPosition = 0;
   for( std::size_t uPass = 0; uPass < m_settingsFlush.m_uPasses; ++uPass )
   {
      for( std::size_t u = 0; u < uLineCount; ++u )
      {
         volatile std::uint8_t* pLine = pBuffer + uPosition * uLine;
         if( bWrite ) { *pLine = static_cast<std::uint8_t>(*pLine + 1); }   // dirty line, written back when evicted
         else { uChecksum += *pLine; }                                      // clean line, cheap to evict

         uPosition += uStride;
         if( uPosition >= uLineCount ) { uPosition -= uLineCount; }
      }
   }
   s_uSink = uChecksum;
   std::atomic_thread_fence(std::memory_order_seq_cst);
}

void benchmark::flusher::flush()
{
   const auto& settings_ = m_settingsFlush;
   if( settings_.m_eLevel != enumCacheLevel::none )
   {
      walk(walk_bytes(static_cast<int>(settings_.m_eLevel), settings_.m_dFraction));
   }

   bool bRangeFlushed = true;
   for( const auto& range_ : settings_.m_vectorRange )
   {
      if( flush_range_s(range_.m_pBegin, range_.m_uSize) == false ) { bRangeFlushed = false; }
   }
   // No line flush on this platform: evict everything instead of silently measuring warm data
   if( bRangeFlushed == false && settings_.m_eLevel != enumCacheLevel::l3 ) { walk(walk_bytes(3, 1.0)); }
}
_GD_END
