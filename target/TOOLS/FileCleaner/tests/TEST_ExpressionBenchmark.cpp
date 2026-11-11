#include <chrono>
#include <fstream>
#include <memory>

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

extern "C"
{
#include "lua/lua.h"
#include "lua/lauxlib.h"
}

#include "main.h"

#include "catch2/catch_amalgamated.hpp"


TEST_CASE("[benchmark] gd-lua code execution", "[benchmark]") 
{
std::string stringCode = R"(
x = 1

if x < 2
    x += 1;
else
    x += 2;
end

function add_to_x( delta )
   x = x + delta;
end

add_to_x( 1 )

for i = 1, i < 10, 1
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

    constexpr std::size_t uRunIterations = 5000;
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

TEST_CASE("[benchmark] lua code execution", "[benchmark]")
{
   const std::string stringCode = R"(
x = 1

if x < 2 then
   x = x + 1
else
   x = x + 2
end

function add_to_x( delta )
   x = x + delta
end

add_to_x( 1 )

for i = 1, 9 do
   x = x + 1
end
)";

   std::unique_ptr<lua_State, decltype(&lua_close)> pState(luaL_newstate(), &lua_close);
   REQUIRE(pState != nullptr);
   lua_State* pLua = pState.get();

   const int iLoadStatus = luaL_loadbuffer(pLua, stringCode.data(), stringCode.size(), "benchmark");
   if( iLoadStatus != LUA_OK )
   {
      FAIL(lua_tostring(pLua, -1));
   }

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
      const int iStatus = luaL_loadbuffer(pLua, stringCode.data(), stringCode.size(), "benchmark");
      const bool bCompiled = iStatus == LUA_OK;
      if( !bCompiled ) { stringCompileError = lua_tostring(pLua, -1); }
      bCompilationOk = bCompilationOk && bCompiled;
      lua_pop(pLua, 1);
      return bCompiled;
   };

   // Loading parses and compiles without executing; the original chunk stays at stack index 1.
   const auto resultCompile = gd::benchmark::run_s(compile_, uCompileIterations, uCompileWarmupCount);

   INFO(stringCompileError);
   REQUIRE(bCompilationOk);
   CHECK(lua_gettop(pLua) == 1);
   print_result_("lua code compilation", resultCompile);

   constexpr std::size_t uRunIterations = 5000;
   constexpr std::size_t uWarmupCount = 100;
   auto setup_ = [&]()
   {
      lua_pushinteger(pLua, 0);
      lua_setglobal(pLua, "x");
      // lua_pcall consumes its chunk, so copy the retained compiled chunk before timing.
      lua_pushvalue(pLua, 1);
   };
   std::string stringExecutionError;
   bool bExecutionOk = true;
   auto execute_ = [&]()
   {
      const int iStatus = lua_pcall(pLua, 0, 0, 0);
      const bool bExecuted = iStatus == LUA_OK;
      if( !bExecuted )
      {
         stringExecutionError = lua_tostring(pLua, -1);
         lua_pop(pLua, 1);
      }
      bExecutionOk = bExecutionOk && bExecuted;
      return bExecuted;
   };

   const auto resultBenchmark = gd::benchmark::run_prepared_s(setup_, execute_, nullptr,
                                                            uRunIterations, uWarmupCount);

   INFO(stringExecutionError);
   REQUIRE(bExecutionOk);
   CHECK(lua_gettop(pLua) == 1);
   lua_getglobal(pLua, "x");
   REQUIRE(lua_isinteger(pLua, -1));
   CHECK(lua_tointeger(pLua, -1) == 12);
   lua_pop(pLua, 1);
   print_result_("lua code execution", resultBenchmark);
}
