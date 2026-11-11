#include <chrono>
#include <fstream>

#include "gd/gd_benchmark.h"
#include "gd/gd_utf8.h"
#include "gd/gd_arguments.h"
#include "gd/gd_cli_options.h"
#include "gd/gd_file.h"
#include "gd/gd_variant.h"
#include "gd/gd_table_column-buffer.h"
#include "gd/io/gd_io_archive_stream.h"
#include "gd/io/gd_io_repository_stream.h"
#include "gd/expression/gd_expression_token.h"
#include "gd/expression/gd_expression_code.h"
#include "gd/expression/gd_expression_method_01.h"
#include "gd/expression/gd_expression_method_02.h"

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[benchmark] expression code execution", "[benchmark][expression]") {

std::string stringCode = R"(
x = 1

if x < 2
begin
    x += 1;
end
else
begin
    x += 2;
end

function add_to_x( delta )
begin
   x = x + delta;
end

add_to_x( 1 )

for i = 1, i < 10, i = i + 1
begin
   x = x + 1;
end
)";

   gd::expression::runtime runtime_;
   runtime_.add({ (unsigned)gd::expression::uMethodDefaultSize_g, gd::expression::pmethodDefault_g, "" });
   runtime_.add({ (unsigned)gd::expression::uMethodStringSize_g, gd::expression::pmethodString_g, "str" });
   runtime_.add("x", int64_t(0));
   gd::expression::code code_;
   auto [bOk, stringError] = code_.compile(stringCode, runtime_);

    INFO(stringError);
    REQUIRE(bOk);

     auto print_result_ = [](const char* pbszName, const gd::benchmark::result& result_)
     {
        std::cout << "\nBenchmark: " << pbszName << "\n"
                  << gd::benchmark::to_string_s(result_, "ns")
                  << "Median: " << result_.median() << " ns\n"
                  << "P95: " << result_.p95() << " ns\n";
     };

    constexpr std::size_t uCompileIterations = 5000;
    constexpr std::size_t uCompileWarmupCount = 100;
    std::string stringCompileError;
    bool bCompilationOk = true;
    auto compile_ = [&]()
    {
       gd::expression::code codeCompiled;
       auto [bCompiled, stringErrorCompile] = codeCompiled.compile(stringCode, runtime_);
       if( !bCompiled ) { stringCompileError = stringErrorCompile; }
       bCompilationOk = bCompilationOk && bCompiled;
       return bCompiled;
    };

    const auto resultCompile = gd::benchmark::run_s( compile_, uCompileIterations, uCompileWarmupCount);

    INFO(stringCompileError);
    CHECK(bCompilationOk);
     print_result_("expression code compilation", resultCompile);

    constexpr std::size_t uRunIterations = 1000;
    constexpr std::size_t uWarmupCount = 100;
    auto setup_ = [&runtime_]() { runtime_.set_variable("x", int64_t(0)); };
    bool bExecutionOk = true;
    auto execute_ = [&]()
    {
       gd::expression::value result_;
       auto error_ = code_.execute(runtime_, &result_);
       bExecutionOk = bExecutionOk && error_.first;
       return error_.first;
    };

    // ## Run benchmark
    const auto resultBenchmark = gd::benchmark::run_prepared_s( setup_, execute_, nullptr, uRunIterations, uWarmupCount);

    CHECK(bExecutionOk);
    gd::expression::value x_(runtime_.get_variable("x"));
    CHECK(x_.as_string() == "11");
     print_result_("expression code execution", resultBenchmark);

   //std::string stringCodeDump = code_.dump();
   //std::cout << "Code dump:\n" << stringCodeDump << std::endl;

}

