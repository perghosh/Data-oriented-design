#include <array>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>

#include "gd/gd_binary.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_arguments_shared.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/gd_table_simd.h"
#include "gd/gd_table_arguments.h"
#include "gd/gd_table_io.h"
#include "gd/gd_sql_value.h"
#include "gd/gd_parse.h"
#include "gd/gd_uuid.h"

#include "main.h"
#include "catch2/catch_amalgamated.hpp"

// ============================================================================
// CONFIGURATION - Global benchmark parameters
// ============================================================================

namespace bench_config
{
   // Number of iterations per benchmark run (higher = more accurate timing)
   static unsigned g_iterations = 1000;

   // Total bytes to process in each benchmark iteration (default: ~64KB)
   static std::size_t g_total_bytes = 64 * 1024;

   // Pattern density: how often to insert "--" sequences (0-100%)
   // Lower = less comment overhead, higher = more comment scanning
   static unsigned g_comment_density = 5;

   // Number of benchmark passes to average
   static unsigned g_benchmark_passes = 5;
}

// Set benchmark configuration externally
inline void set_benchmark_iterations(unsigned iterations) { bench_config::g_iterations = iterations; }
inline void set_benchmark_bytes(std::size_t bytes) { bench_config::g_total_bytes = bytes; }
inline void set_benchmark_density(unsigned density) { bench_config::g_comment_density = density; }

// ============================================================================
// TEST DATA GENERATION
// ============================================================================

// Generate realistic source code with comments for benchmarking
std::string generate_benchmark_source(std::size_t target_size, unsigned comment_density)
{
   std::string result;
   result.reserve(target_size);

   static const char* keywords[] = {
      "local ", "function ", "return ", "if ", "then ", "else ", "end ",
      "for ", "while ", "do ", "in ", "nil ", "true ", "false ", "and ", "or "
   };

   static const char* identifiers[] = {
      "x", "y", "z", "i", "j", "k", "count", "value", "result", "data",
      "config", "settings", "player", "enemy", "level", "score", "health"
   };

   static const char* operators[] = {
      "=", "==", "~=", "<", ">", "<=", ">=", "+", "-", "*", "/", "%", "^"
   };

   std::mt19937 rng(42); // Fixed seed for reproducibility
   std::uniform_int_distribution<> kw_dist(0, 15);
   std::uniform_int_distribution<> id_dist(0, 16);
   std::uniform_int_distribution<> op_dist(0, 13);
   std::uniform_int_distribution<> byte_dist(32, 126);

   auto rand_keyword = [&]() { return keywords[kw_dist(rng)]; };
   auto rand_identifier = [&]() { return identifiers[id_dist(rng)]; };
   auto rand_operator = [&]() { return operators[op_dist(rng)]; };
   auto rand_char = [&]() { return static_cast<char>(byte_dist(rng)); };

   bool expect_newline = false;

   while(result.size() < target_size)
   {
      // Decide what to add next
      unsigned decision = byte_dist(rng) % 100;

      if(!expect_newline && decision < comment_density)
      {
         // Add comment
         result += "--";

         // Comment body: mix of letters and spaces
         std::size_t comment_len = 20 + (byte_dist(rng) % 60);
         for(std::size_t i = 0; i < comment_len && result.size() < target_size; ++i)
         {
            char c = byte_dist(rng) % 20 == 0 ? '\n' : (byte_dist(rng) % 2 == 0 ? ' ' : 'a' + (byte_dist(rng) % 26));
            result += c;
            if(c == '\n') { expect_newline = false; break; }
         }
      }
      else if(expect_newline || byte_dist(rng) % 30 == 0)
      {
         // Newline
         result += '\n';
         expect_newline = false;
      }
      else if(byte_dist(rng) % 10 == 0)
      {
         // Whitespace
         result += ' ';
      }
      else
      {
         // Random code token
         unsigned token_type = byte_dist(rng) % 4;
         switch(token_type)
         {
         case 0: result += rand_keyword(); break;
         case 1: result += rand_identifier(); break;
         case 2: result += rand_operator(); break;
         case 3: result += rand_char(); break;
         }
      }
   }

   return result;
}

// ============================================================================
// IMPLEMENTATION 1: POSITION-BASED (your original test case)
// ============================================================================

struct benchmark_result
{
   std::string output;
   std::chrono::nanoseconds elapsed;
   std::size_t bytes_processed;
   double mb_per_sec;
};

benchmark_result benchmark_position_based(const std::string& source,
   gd::table::simd::table_8_8& tableCode)
{
   auto start = std::chrono::high_resolution_clock::now();

   tableCode.row_clear();
   std::span<const char> span_(source.data(), source.size());
   tableCode.pack_plant_span<char>(span_, 0, '\0');

   std::string stringCodeCleaned;
   stringCodeCleaned.reserve(source.size());

   using namespace gd::table::simd;
   table_8_8::position positionEnd;
   positionEnd.advance((unsigned)source.size());

   // ### Pass 1: find every "-- ... \n" comment as a [begin, end) range
   std::vector<table_8_8::range> vectorComment;
   {
      table_8_8::position position_;
      while(position_ < positionEnd)
      {
         uint8_t uByte = tableCode.get_uint8(position_);

         table_8_8::position positionNext = position_;
         positionNext.advance(1);

         if(uByte == '-' && positionNext < positionEnd && tableCode.get_uint8(positionNext) == '-')
         {
            table_8_8::position positionStart = position_;
            table_8_8::position positionScan = position_;
            positionScan.advance(2);

            while(positionScan < positionEnd && tableCode.get_uint8(positionScan) != '\n') { positionScan.advance(1); }
            if(positionScan < positionEnd) { positionScan.advance(1); }

            vectorComment.push_back(table_8_8::range{ positionStart, positionScan });
            position_ = positionScan;
            continue;
         }

         position_.advance(1);
      }
   }

   // ### Pass 2: copy everything that isn't covered by a comment range
   {
      table_8_8::position position_;
      auto itComment = vectorComment.begin();
      while(position_ < positionEnd)
      {
         if(itComment != vectorComment.end() && position_ == itComment->first)
         {
            position_ = itComment->second;
            ++itComment;
            continue;
         }

         stringCodeCleaned.push_back((char)tableCode.get_uint8(position_));
         position_.advance(1);
      }
   }

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

   double mb_per_sec = (source.size() / 1e6) / (duration.count() / 1e9);

   return { std::move(stringCodeCleaned), duration, source.size(), mb_per_sec };
}

// ============================================================================
// IMPLEMENTATION 2: HYBRID (your original test case)
// ============================================================================

benchmark_result benchmark_hybrid(const std::string& source,
   gd::table::simd::table_8_8& tableCode)
{
   auto start = std::chrono::high_resolution_clock::now();

   tableCode.row_clear();
   std::span<const char> span_(source.data(), source.size());
   tableCode.pack_plant_span<char>(span_, 0, '\0');

   std::string stringCodeCleaned;
   stringCodeCleaned.reserve(source.size());

   bool bInComment = false;
   uint64_t uTotalBytes = source.size();

   using namespace gd::table::simd;
   table_8_8::position positionEnd;
   positionEnd.advance((unsigned)uTotalBytes);

   constexpr unsigned uPackBytes = table_8_8::size_pack_s();

   uint64_t uFullPackCount = uTotalBytes / uPackBytes;
   unsigned uTailByteCount = (unsigned)(uTotalBytes % uPackBytes);

   auto ScanSimple = [&](table_8_8::position position_, table_8_8::position positionRangeEnd)
      {
         while(position_ < positionRangeEnd)
         {
            uint8_t uByte = tableCode.get_uint8(position_);

            if(bInComment)
            {
               position_.advance(1);
               if(uByte == '\n') { bInComment = false; }
               continue;
            }

            if(uByte == '-')
            {
               table_8_8::position positionNext = position_;
               positionNext.advance(1);
               if(positionNext < positionEnd && tableCode.get_uint8(positionNext) == '-')
               {
                  bInComment = true;
                  position_ = positionNext;
                  position_.advance(1);
                  continue;
               }
            }

            stringCodeCleaned.push_back((char)uByte);
            position_.advance(1);
         }
      };

   // ## FAST PATH: one mask over the whole (guaranteed-full) pack decides everything.
   for(uint64_t uPack = 0; uPack < uFullPackCount; ++uPack)
   {
      if(bInComment)
      {
         uint64_t uMaskNewline = tableCode.pack_find_value<char>(uPack, 0, '\n');
         if(uMaskNewline == 0) { continue; }
      }
      else
      {
         uint64_t uMaskDash = tableCode.pack_find_value<char>(uPack, 0, '-');
         if(uMaskDash == 0)
         {
            auto spanPack = tableCode.pack_harvest_span<char>(uPack, 0);
            stringCodeCleaned.append(spanPack.data(), spanPack.size());
            continue;
         }
      }

      table_8_8::position positionPackStart;
      positionPackStart.m_uRow = uPack * table_8_8::count_pack_s();
      table_8_8::position positionPackEnd = positionPackStart;
      positionPackEnd.advance(uPackBytes);

      ScanSimple(positionPackStart, positionPackEnd);
   }

   // ## TAIL: the one possibly-partial pack, if any.
   if(uTailByteCount > 0)
   {
      table_8_8::position positionTailStart;
      positionTailStart.m_uRow = uFullPackCount * table_8_8::count_pack_s();
      table_8_8::position positionTailEnd = positionTailStart;
      positionTailEnd.advance(uTailByteCount);

      ScanSimple(positionTailStart, positionTailEnd);
   }

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

   double mb_per_sec = (source.size() / 1e6) / (duration.count() / 1e9);

   return { std::move(stringCodeCleaned), duration, source.size(), mb_per_sec };
}

// ============================================================================
// IMPLEMENTATION 3: STANDARD STRING (baseline comparison)
// ============================================================================

benchmark_result benchmark_std_string(const std::string& source)
{
   auto start = std::chrono::high_resolution_clock::now();

   std::string cleaned;
   cleaned.reserve(source.size());

   bool in_comment = false;
   std::size_t n = source.size();

   for(std::size_t i = 0; i < n; ++i)
   {
      if(in_comment)
      {
         if(source[i] == '\n')
         {
            in_comment = false;
            cleaned += '\n';
         }
         // Skip all characters while in comment
      }
      else
      {
         if(i + 1 < n && source[i] == '-' && source[i + 1] == '-')
         {
            in_comment = true;
            ++i; // Skip second '-'
         }
         else
         {
            cleaned += source[i];
         }
      }
   }

   auto end = std::chrono::high_resolution_clock::now();
   auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

   double mb_per_sec = (source.size() / 1e6) / (duration.count() / 1e9);

   return { std::move(cleaned), duration, source.size(), mb_per_sec };
}

// ============================================================================
// VERIFICATION UTILITIES
// ============================================================================

bool verify_outputs_match(const std::string& expected, const std::string& actual)
{
   if(expected.size() != actual.size())
   {
      std::cerr << "Size mismatch: expected=" << expected.size()
         << " actual=" << actual.size() << "\n";
      return false;
   }

   // Find first difference
   for(std::size_t i = 0; i < expected.size(); ++i)
   {
      if(expected[i] != actual[i])
      {
         std::cerr << "Difference at position " << i << ": '"
            << expected[i] << "' vs '" << actual[i] << "'\n";
         // Show context
         std::size_t ctx_start = i > 10 ? i - 10 : 0;
         std::size_t ctx_end = i + 10 < expected.size() ? i + 10 : expected.size();
         std::cerr << "Context: [" << expected.substr(ctx_start, ctx_end - ctx_start) << "]\n";
         return false;
      }
   }

   return true;
}

// ============================================================================
// BENCHMARK RUNNER
// ============================================================================

void run_single_benchmark(std::size_t bytes, unsigned density, unsigned iterations)
{
   // Generate test data once
   std::string source = generate_benchmark_source(bytes, density);

   std::cout << "\n=========================================================\n";
   std::cout << "BENCHMARK CONFIGURATION\n";
   std::cout << "=========================================================\n";
   std::cout << "Source Size:        " << std::fixed << std::setprecision(2)
      << (bytes / 1024.0) << " KB\n";
   std::cout << "Comment Density:    " << density << "%\n";
   std::cout << "Iterations:         " << iterations << "\n";
   std::cout << "Actual Comments:    " << std::count(source.begin(), source.end(), '-') << " dashes\n";
   std::cout << "=========================================================\n\n";

   // Initialize table once
   using namespace gd::table::simd;
   table_8_8 tableCode(1000u, gd::table::tag_repare_to_add_column{});
   tableCode.column_add("uint64", 0, "code");
   tableCode.prepare();

   // Benchmark results storage
   std::vector<double> position_times(iterations);
   std::vector<double> hybrid_times(iterations);
   std::vector<double> stdstring_times(iterations);

   std::string last_position_output, last_hybrid_output, last_stdstring_output;

   // Run benchmarks
   std::cout << "Running benchmarks...\n";
   for(unsigned iter = 0; iter < iterations; ++iter)
   {
      // Position-based
      {
         auto result = benchmark_position_based(source, tableCode);
         position_times[iter] = result.mb_per_sec;
         last_position_output = result.output;
      }

      // Hybrid
      {
         auto result = benchmark_hybrid(source, tableCode);
         hybrid_times[iter] = result.mb_per_sec;
         last_hybrid_output = result.output;
      }

      // Standard string
      {
         auto result = benchmark_std_string(source);
         stdstring_times[iter] = result.mb_per_sec;
         last_stdstring_output = result.output;
      }

      if(iter % (iterations / 10) == 0 && iter > 0)
      {
         std::cout << "  Progress: " << iter * 100 / iterations << "%\n";
      }
   }

   // Calculate statistics
   auto calc_stats = [](const std::vector<double>& times)
      {
         double mean = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
         double sq_sum = 0.0;
         for(double t : times)
         {
            sq_sum += (t - mean) * (t - mean);
         }
         double stddev = std::sqrt(sq_sum / times.size());
         auto [min_it, max_it] = std::minmax_element(times.begin(), times.end());

         return std::make_tuple(mean, stddev, *min_it, *max_it);
      };

   auto [pos_mean, pos_std, pos_min, pos_max] = calc_stats(position_times);
   auto [hyb_mean, hyb_std, hyb_min, hyb_max] = calc_stats(hybrid_times);
   auto [std_mean, std_std, std_min, std_max] = calc_stats(stdstring_times);

   // Print results
   std::cout << "\n=========================================================\n";
   std::cout << "BENCHMARK RESULTS (MB/s)\n";
   std::cout << "=========================================================\n";
   std::cout << std::fixed << std::setprecision(2);
   std::cout << std::left << std::setw(25) << "Method"
      << std::right << std::setw(12) << "Mean"
      << std::setw(12) << "StdDev"
      << std::setw(12) << "Min"
      << std::setw(12) << "Max" << "\n";
   std::cout << std::string(73, '-') << "\n";
   std::cout << std::left << std::setw(25) << "POSITION-BASED (SIMD)"
      << std::right << std::setw(12) << pos_mean
      << std::setw(12) << pos_std
      << std::setw(12) << pos_min
      << std::setw(12) << pos_max << "\n";
   std::cout << std::left << std::setw(25) << "HYBRID (SIMD+fast-path)"
      << std::right << std::setw(12) << hyb_mean
      << std::setw(12) << hyb_std
      << std::setw(12) << hyb_min
      << std::setw(12) << hyb_max << "\n";
   std::cout << std::left << std::setw(25) << "STANDARD STRING (baseline)"
      << std::right << std::setw(12) << std_mean
      << std::setw(12) << std_std
      << std::setw(12) << std_min
      << std::setw(12) << std_max << "\n";

   // Speedup calculations
   std::cout << "\n=========================================================\n";
   std::cout << "SPEEDUP ANALYSIS (relative to standard string)\n";
   std::cout << "=========================================================\n";
   std::cout << "Position-based:  " << std::setw(6) << (pos_mean / std_mean)
      << "x faster than std::string\n";
   std::cout << "Hybrid:          " << std::setw(6) << (hyb_mean / std_mean)
      << "x faster than std::string\n";
   std::cout << "Hybrid vs Pos:   " << std::setw(6) << (hyb_mean / pos_mean)
      << "x relative to position-based\n";

   // Output verification
   std::cout << "\n=========================================================\n";
   std::cout << "OUTPUT VERIFICATION\n";
   std::cout << "=========================================================\n";

   bool pos_ok = verify_outputs_match(last_stdstring_output, last_position_output);
   bool hyb_ok = verify_outputs_match(last_stdstring_output, last_hybrid_output);

   std::cout << "Position-based output matches std::string: "
      << (pos_ok ? "✓ PASS" : "✗ FAIL") << "\n";
   std::cout << "Hybrid output matches std::string:         "
      << (hyb_ok ? "✓ PASS" : "✗ FAIL") << "\n";

   std::cout << "\n=========================================================\n\n";

   if(!pos_ok || !hyb_ok)
   {
      throw std::runtime_error("Benchmark verification failed!");
   }
}

// ============================================================================
// CATCH2 TEST CASES
// ============================================================================

TEST_CASE("[gd-benchmark] position-based comment removal", "[gd-benchmark]")
{
   using namespace bench_config;
   std::cout << "\n=== RUNNING POSITION-BASED BENCHMARK ===\n";
   std::cout << "Target bytes: " << g_total_bytes
      << ", Iterations: " << g_iterations
      << ", Density: " << g_comment_density << "%\n\n";

   // Generate test data
   std::string source = generate_benchmark_source(g_total_bytes, g_comment_density);

   // Run multiple passes and average
   std::vector<double> speeds;

   using namespace gd::table::simd;
   table_8_8 tableCode(10000u, gd::table::tag_repare_to_add_column{});
   tableCode.column_add("uint64", 0, "code");
   tableCode.prepare();

   for(unsigned pass = 0; pass < g_benchmark_passes; ++pass)
   {
      auto result = benchmark_position_based(source, tableCode);
      speeds.push_back(result.mb_per_sec);
   }

   double avg_speed = std::accumulate(speeds.begin(), speeds.end(), 0.0) / speeds.size();

   std::cout << "Average speed: " << std::fixed << std::setprecision(2)
      << avg_speed << " MB/s\n";

   // Basic sanity check - should be reasonably fast
   CHECK(avg_speed > 0.0);
}

TEST_CASE("[gd-benchmark] hybrid comment removal", "[gd-benchmark]")
{
   using namespace bench_config;
   std::cout << "\n=== RUNNING HYBRID BENCHMARK ===\n";
   std::cout << "Target bytes: " << g_total_bytes
      << ", Iterations: " << g_iterations
      << ", Density: " << g_comment_density << "%\n\n";

   std::string source = generate_benchmark_source(g_total_bytes, g_comment_density);

   std::vector<double> speeds;

   using namespace gd::table::simd;
   table_8_8 tableCode(10000u, gd::table::tag_repare_to_add_column{});
   tableCode.column_add("uint64", 0, "code");
   tableCode.prepare();

   for(unsigned pass = 0; pass < g_benchmark_passes; ++pass)
   {
      auto result = benchmark_hybrid(source, tableCode);
      speeds.push_back(result.mb_per_sec);
   }

   double avg_speed = std::accumulate(speeds.begin(), speeds.end(), 0.0) / speeds.size();

   std::cout << "Average speed: " << std::fixed << std::setprecision(2)
      << avg_speed << " MB/s\n";

   CHECK(avg_speed > 0.0);
}

TEST_CASE("[gd-benchmark] full comparative benchmark", "[gd-benchmark][skip]")
{
   // This test runs the FULL comparative benchmark
   // Marked as [skip] by default - run manually with command line filter
   // Or remove [skip] to enable in regular test runs

   using namespace bench_config;

   // Run comprehensive benchmark
   run_single_benchmark(g_total_bytes, g_comment_density, g_iterations);
}

// ============================================================================
// ADDITIONAL HELPER FOR QUICK TESTS
// ============================================================================

// Quick single-run benchmark for development/testing
inline void quick_benchmark()
{
   std::cout << "\n=== QUICK BENCHMARK ===\n";

   // Test various sizes
   std::vector<std::size_t> sizes = {
      1024,           // 1 KB
      10 * 1024,      // 10 KB  
      100 * 1024,     // 100 KB
      1024 * 1024     // 1 MB
   };

   for(std::size_t size : sizes)
   {
      std::string source = generate_benchmark_source(size, 5);

      using namespace gd::table::simd;
      table_8_8 tableCode(10000u, gd::table::tag_repare_to_add_column{});
      tableCode.column_add("uint64", 0, "code");
      tableCode.prepare();

      auto pos_result = benchmark_position_based(source, tableCode);
      auto hyb_result = benchmark_hybrid(source, tableCode);
      auto std_result = benchmark_std_string(source);

      std::cout << std::fixed << std::setprecision(2);
      std::cout << "Size: " << (size / 1024.0) << " KB\n";
      std::cout << "  Position-based: " << std::setw(8) << pos_result.mb_per_sec << " MB/s\n";
      std::cout << "  Hybrid:         " << std::setw(8) << hyb_result.mb_per_sec << " MB/s\n";
      std::cout << "  std::string:    " << std::setw(8) << std_result.mb_per_sec << " MB/s\n";
      std::cout << "  Hybrid gain:    " << std::setw(8) << (hyb_result.mb_per_sec / pos_result.mb_per_sec) << "x\n";
      std::cout << "\n";
   }
}
